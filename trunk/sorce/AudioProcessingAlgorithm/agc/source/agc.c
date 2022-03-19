#include "agc.h"
#include "ttminmax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define AGC_K 8
#define AGC_LA 0.176788f          /* 5793/32768   */
#define AGC_LF 0.743835f          /* 24374/32768  */
/* Target level matched to -26dBFS, verified with p.56.
 * The agc works on absolute levels and not power levels,
 * so set target to 1-2 dB higher than the power target to compensate for this */
#define AGC_LALFDK ((1841.6f / 32768) / AGC_K) /* 1841.6/32768 = 20*log10f(-25/20) */
#define AGC_MINSA_MIC 0.00943f    /* 309/32768;        La*(K-MaxGn)/(K*MaxGn), MaxGn = 4 */
#define AGC_MSPEED 0.25f          /* 1/4; Maintenance speed               */
#define AGC_SETTLEPERIOD 64           /* 64   */
#define AGC_SETTLETIME   48           /* 48   */

#define AGC_SFMIN    1.192092895507e-7f   /* 2^(-23)    */
#define AGC_NMIN_MIC 3.1e-5f      /* 2^(-15)        */
#define AGC_SFDECAY  0.03125f     /* 1/32         */
#define AGC_MAXDIFF  9.77e-4f     /* 1/1024         */
#define AGC_NATTACK  1.953e-3f    /* 1/512          */
#define AGC_MAXSTEP  2e-6f        /* 1/(1024*512)   */
#define AGC_NOISELIMIT 8
#define AGC_TALKLIMIT  16
#define AGC_NOISEOVEREST 4        /* 2.0f-4.0f */

// robustmode
#define AGC_NUMBINS 10
#define AGC_BINSIZE 100

struct AGCSTATE {
  int AgcOn;
  float Sf;  /*- Measured audio level, short time constant*/
  float Sa;  /*- Measured audio level, long  time constant*/
  float Noiselevel;   /*- Measured noise level*/
  float Gn;  /*- Agc gain*/

  int Cnt;   /*- Agc attack counter*/
  float SpTab[9];     /* Constants in struct. Assigned in agc_reset, [0.2214 0.1283 0.0875 0.0614 0.0424 0.0274 0.0151 0.0047 0] */
  bool debug;

  /* Sneeze suppression variables */
  float SfPeaks[AGC_NUMBINS]; /* history of Sf maxima within 1-second bins */
  float SaPeaks[AGC_NUMBINS]; /* history of Sa maxima within 1-second bins */
  float PeakCount;
  float PeakBinCount;
  float SaTarget[2];
  int SaTimer[2];
  bool robust;
  float Nf;
};

struct AGCSTATE *agc_create(void)
{
    struct AGCSTATE *p;

    p = malloc(sizeof *p);
    return p;
}

void agc_destroy(struct AGCSTATE *self)
{
    free(self);
}

void agc_reset(struct AGCSTATE *self)
{
    self->AgcOn = 1;
    self->Sf = 0.0f;
    self->Sa = (309.0f / 32768);   /* MinSa_MIC */
    self->Noiselevel = 0.0f;
    self->Gn = 1.0f;

    self->debug = false;

    self->Cnt = 0;
    self->SpTab[0] = 0.2214f;
    self->SpTab[1] = 0.1283f;
    self->SpTab[2] = 0.0875f;
    self->SpTab[3] = 0.0614f;
    self->SpTab[4] = 0.0424f;
    self->SpTab[5] = 0.0274f;
    self->SpTab[6] = 0.0151f;
    self->SpTab[7] = 0.0047f;
    self->SpTab[8] = 0.0000f;

    /* Mic AGC Sneeze suppression */
    self->SaTimer[0] = 0;
    self->SaTimer[1] = 0;
    self->PeakCount = 0;
    {
        int i;
        for (i = 0; i < AGC_NUMBINS; i++) {
            self->SfPeaks[i] = 0;
            self->SaPeaks[i] = 0;
        }
    }
    self->PeakBinCount = 0;
    self->SaTarget[0] = 0;
    self->SaTarget[1] = 0;
    self->SaTimer[0] = 0;
    self->SaTimer[1] = 0;
    self->robust = true;   //this makes it default...
    self->Nf = 1.0f;
}

void agc_process(
    struct AGCSTATE *self,
    uint32_t nchannels,
    const float *const absLevel,
    float *gain,
    uint32_t trace)
{
    int k;
    float framePower, Sp, LimSf, denom1, denom2, tmp;
    float sigPowLimiter = 4.0f;
    float priSilenceGain = 1.0f;

    if (2 == nchannels)
        /* stereo: uses average AbsLevel */
        framePower = 0.5f * (absLevel[0] + absLevel[1]);
    else
        /* mono - default behaviour */
        framePower = absLevel[0];

    framePower = framePower * *gain;    /* in case Gain != 1 */
    if (framePower == 0.0f)
        return;

    /* Update fast audio level Sf, attack time 5ms, decay time 315ms  */
    tmp = framePower - self->Sf;
    if (tmp > 0)
        self->Sf += tmp;
    else
        self->Sf += tmp * AGC_SFDECAY;

    if (self->Sf < AGC_SFMIN)  /* Downwards limit Sf to Min */
        self->Sf = AGC_SFMIN;

    /* Update noiselevel, attack time 5.1sec (further reduced 
     * with limited diff), decay time 5ms */
    tmp = framePower - self->Noiselevel;

    if (self->Noiselevel <= AGC_NMIN_MIC) {
        /* kick start the noise estimate when it's too low */
        self->Noiselevel = framePower; 
    } else {
        if (tmp < 0)
            self->Noiselevel += tmp;   /* Instantaneous release */
        else if (tmp < AGC_MAXDIFF)
            self->Noiselevel += tmp * AGC_NATTACK;
        else
            self->Noiselevel += AGC_MAXSTEP;

        /* Downwards limit Noiselevel to 1/32768 */
        if (self->Noiselevel < AGC_NMIN_MIC)   
            self->Noiselevel = AGC_NMIN_MIC;
    }

    /* perform Gn-calculation */
    /*   Update agc power level Sa
     *   The agc power level is changed by the formula
     *   Sa = Sa * (1 + alpha), if Sf > Sa, and
     *   Sa = Sa * (1 - alpha), if Sf < Sa.
     *   Alpha is dependent on relative difference between
     *   Sa and Sf, the function approximates an alpha tracker
     *   performed on logarithmic values. The function for agc
     *   level update speed is stored in the table self->SpTab.
     *   big signal diff => fast speed
     *   the table is an approximatoin to:
     *   lev[dB] = lev[dB] + a * (meas[dB] - lev[dB])
     *   without actually having to calc log/dB
     *   The alpha value is also dependent on whether the
     *   sound is start of speech/sound (alpha big, fast settling)
     *   or later speech (alpha small, slow maintenance).
     */

    if (self->Sf < AGC_NOISELIMIT * self->Noiselevel) {
        /* this is noise, prepare for next talk sequence */
        self->Cnt = AGC_SETTLEPERIOD;
    } else if (framePower > AGC_TALKLIMIT * self->Noiselevel) {
        /* this is talk */
        if (self->Sf > self->Sa) {
            /* get update speed */
            k = (int) ((8 * self->Sa / self->Sf));
            Sp = self->SpTab[k];

            if (self->Cnt == 0) {
                /* in sequence, use maintenance/reduced speed */
                Sp *= AGC_MSPEED;
            }

            /* increase agc level */
            self->Sa *= (1 + Sp);

            /* never allow Sa above Sf or 1 */
            if (self->Sa > self->Sf)
                self->Sa = self->Sf;
        } else {
            /* get update speed */
            k = (int) ((8 * self->Sf / self->Sa));
            Sp = self->SpTab[k];
            if (self->Cnt == 0 || self->Cnt > AGC_SETTLETIME) {
                /* if in sequence (Cnt==0), use maintenance/reduced speed 
                 * if in start of sequence (Cnt>SettleTime), avoid reduction
                 * of Sa before Sf has settled                  */
                Sp *= AGC_MSPEED;
            }

            /* decrease agc level */
            self->Sa *= (1 - Sp);

            /* never allow Sa below Sf or MinSa (the 
             * latter to reduce attack time) */
            if (self->Sa < AGC_MINSA_MIC)
                self->Sa = AGC_MINSA_MIC;

            if (self->Sa < self->Sf)
                self->Sa = self->Sf;
        }

        /* reduce counter */
        if (self->Cnt > 0)
            self->Cnt -= 1;
    }

    if (self->robust) {
        /* Stuff for sneeze suppression (aka handkerchief mode) */
        self->SfPeaks[AGC_NUMBINS - 1] = max(self->SfPeaks[AGC_NUMBINS - 1], self->Sf);
        self->SaPeaks[AGC_NUMBINS - 1] = max(self->SaPeaks[AGC_NUMBINS - 1], self->Sa);
        self->PeakCount += 1;
        if (self->PeakCount >= AGC_BINSIZE) {
            int i;

            self->PeakCount = 0;
            self->PeakBinCount += 1;

            if (self->PeakBinCount >= AGC_NUMBINS && self->SaTimer[0] == 0) {
                float SaEnvelope[3];
                float SfEnvelope[3];
                float hoveringness, steppyness, peakyness, sig_est, snr;

                /* Initialize */
                for (i = 0; i < 3; i++) {
                    SaEnvelope[i] = 0;
                    SfEnvelope[i] = 0;
                }

                /* Find coarse, 3-element Sa/Sf envelope over past 10 seconds */
                for (i = 0; i < 4; i++) {
                    SaEnvelope[0] = max(SaEnvelope[0], self->SaPeaks[i]);
                    SfEnvelope[0] = max(SfEnvelope[0], self->SfPeaks[i]);
                }

                for (i = 4; i < 6; i++) {
                    SaEnvelope[1] = max(SaEnvelope[1], self->SaPeaks[i]);
                    SfEnvelope[1] = max(SfEnvelope[1], self->SfPeaks[i]);
                }

                for (i = 6; i < AGC_NUMBINS; i++) {
                    //SaEnvelope[2] = max(SaEnvelope[2], self->SaPeaks[i]);
                    SfEnvelope[2] = max(SfEnvelope[2], self->SfPeaks[i]);
                }

                //Use current Sa value instead, makes more sense...
                SaEnvelope[2] = self->Sa;

                /* Convert to dB */
                for (i = 0; i < 3; i++) {
                    SaEnvelope[i] = 20 * log10f(SaEnvelope[i]);
                    SfEnvelope[i] = 20 * log10f(SfEnvelope[i]);
                }

                /* Compare early/middle/late envelope of Sa vs Sf.
                 * We want to identify the case where Sa resemble 
                 * a step-function, while Sf resemble an aligned impulse */
                hoveringness = SaEnvelope[2] - SfEnvelope[2];
                steppyness = SaEnvelope[2] - SaEnvelope[0];
                peakyness = SfEnvelope[1] - max(SfEnvelope[0], SfEnvelope[2]);

                sig_est = 0;
                for (i = 0; i < 4; i++)
                    sig_est += self->SfPeaks[i];

                for (i = 6; i < AGC_NUMBINS; i++)
                    sig_est += self->SfPeaks[i];

                sig_est = sig_est / 8;
                snr = 20 * log10f(sig_est) - 20 * log10f(self->Nf);

                if (peakyness > 3 && steppyness > 3 && hoveringness > 3 && snr < 35) {

                    /* Remember current Sa value, and where we want it to go */
                    self->SaTarget[0] = self->Sa;

                    //return to where we were
                    self->SaTarget[1] = powf(10, SaEnvelope[0] / 20.0f);

                    /* Figure out how much time is needed to 
                     * get there (2.5dB/second) */
                    self->SaTimer[0] 
                        = 40 
                        * 20 
                        * (log10f(self->SaTarget[0]) - log10f(self->SaTarget[1]));

                    self->SaTimer[1] = self->SaTimer[0];
                }
            }

            for (i = 0; i < AGC_NUMBINS - 1; i++) {
                self->SfPeaks[i] = self->SfPeaks[i + 1];
                self->SaPeaks[i] = self->SaPeaks[i + 1];
            }

            self->SfPeaks[AGC_NUMBINS - 1] = 0;
            self->SaPeaks[AGC_NUMBINS - 1] = 0;
        }

        /* Stuff for sneeze suppression (aka handkerchief mode)
         * Distribute correction of Sa over time to make it sound smoother */
        if (self->SaTimer[1] > 0) {
            if (self->SaTimer[1] > 1) {
                float rate;
                float progression 
                    = (float) (self->SaTimer[0] - self->SaTimer[1]) / (float) self->SaTimer[0];

                /* Slower Sa modification for the first and last 1/4 */
                if (progression < 0.25f || progression > 0.75f) {
                    rate = powf(10,
                        (20 * log10f(self->SaTarget[1]) - 20 * log10f(self->SaTarget[0]) + 6) 
                        / (20 * self->SaTimer[1]));
                }
                else {
                    rate = powf(10, (20 * log10f(self->SaTarget[1]) - 20 * log10f(self->SaTarget[0]) - 6) 
                        / (20 * self->SaTimer[1]));
                }
                self->Sa = min(self->Sa, max(rate * self->Sa, self->SaTarget[1]));
            } else {
                /* Done doing smooth ramp. If we are not there, 
                 * jump immediately. Never increase beyond current Sa. */
                self->Sa = min(self->Sa, self->SaTarget[1]);
                self->SaTimer[0] = 0;
            }

            self->SaTimer[1] = max(self->SaTimer[1] - 1, 0);
        }
    }

    /* Reduce agc limit level                 */
    /* The limit level is reduced at start of sentences     */
    /* (during agc power settling time) to avoid very     */
    /* loud signals/amplification if agc level Sa is low.   */
    /* The reduced limit is actually achieved by increasing   */
    /* the sigpowh level (only for limiting part, not noise   */
    /* suppression part)                    */

    if (self->Cnt != 0)
        LimSf = self->Sf * sigPowLimiter;  /* start of sentence, LimSf = Sf * Lf/La; */
    else
        LimSf = self->Sf;  /* not start of sentence */

    /* Calculate new gain */
    denom1 = LimSf * AGC_LA;
    denom2 = self->Sa * AGC_LF + AGC_LALFDK;
    if (denom1 > denom2)
        self->Gn = AGC_K * AGC_LALFDK / denom1;
    else
        self->Gn = AGC_K * AGC_LALFDK / denom2;

    /* If Gain > priSilenceGain, add noise suppression to avoid ampl. of noise
     * Noiselevel is overestimated to avoid gain fluctuations at levels 
     * close to noise */
    if (self->Gn > priSilenceGain) {
        self->Gn *= (self->Sf - AGC_NOISEOVEREST * self->Noiselevel) / self->Sf;

        if (self->Gn < priSilenceGain)
            self->Gn = priSilenceGain;
    }
    /* end Gn-calculation */
    /* ----------------------------------------------------------- */

    /* Set gain to unity if agc not enabled */
    if (self->AgcOn == 0)
        self->Gn = 1.0f;

    /* Finally update overall gain */
    *gain *= self->Gn;

    /* debug */
    if (self->debug) {
        /*
         * 1. agcGain
         * 2. Sf, Measured audio level, short time constant
         * 3. Sa, Measured audio level, long time constant
         * 4. Noiselevel, Measured noise level
         * 5. Gn, based on Sf
         * 6. Gn based on Sa
         * 7. Gn factor from noisegate
         */

        static int counter = 0;
        static float Sum1 = 0.0f, Sum2 = 0.0f, Sum3 = 0.0f, Sum4 = 0.0f,
            Sum5 = 0.0f, Sum6 = 0.0f, Sum7 = 0.0f;

        Sum1 += self->Gn;
        Sum2 += self->Sf;
        Sum3 += self->Sa;
        Sum4 += self->Noiselevel;
        Sum5 += AGC_K * AGC_LALFDK / denom1;
        Sum6 += AGC_K * AGC_LALFDK / denom2;
        Sum7 += (self->Sf - AGC_NOISEOVEREST * self->Noiselevel) / self->Sf;

        if (++counter == 20) {
            Sum1 = Sum2 = Sum3 = Sum4 = Sum5 = Sum6 = Sum7 = 0.0f;
            counter = 0;
        }
    }
}

void agc_set_on_off(struct AGCSTATE *self, bool val)
{
    self->AgcOn = val;
}

bool agc_get_state(struct AGCSTATE *self)
{
    return self->AgcOn;
}

void agc_set_debug(struct AGCSTATE *self, bool onoff)
{
    const char *status[] = { "off", "on" };
    self->debug = onoff;
    return;
}

void agc_set_robust_mode(struct AGCSTATE *self, bool onoff)
{
    const char *status[] = { "off", "on" };
    agc_reset(self);             //reset so that we start from scratch, then change mode
    self->robust = onoff;
    return;
}

void agc_get_gui_string(struct AGCSTATE *self, char *guiString, const size_t max_string_size)
{
    char temp[96];
    snprintf(temp, sizeof temp, "AGC_OLD: SF: %.6f END SA: %.6f END NOISELEVEL: %.6f END GAIN_OLD: %.6f END\n",
        self->Sf, self->Sa, self->Noiselevel, self->Gn);
    strncat(guiString, temp, max_string_size);
}

float agc_get_gain(struct AGCSTATE * self)
{
    return self->Gn;
}
