/*
 * Automatic gain controller implemented by Ole Marius Hoel Rindal -
 *                      orindal@cisco.com or olemarius@olemarius.net
 *
 * A new implementation of an AGC controller based on
 * Knut Olav Skyttemyrs matlab implementasjon (agc_ko).
 *
 * Update summer 2013
 * The agc now uses the absLevel measure as a RMS measure. This saves cpu
 * cycles and fixes a issue on the Saturn platform where the buffer was sent
 * as integers, fixedpoint.
 */

#include "agc_new.h"
#include "audsizes.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE_GUISTRING_PART_1 125
#define MAX_SIZE_GUISTRING_PART_2 100

struct AGC_PREV {
    /* Variables from previous AGC*/
    int counter_prev;
    float smooth_prev;
    float smooth_gain_db_prev;
    int count_nfe_prev; //For noise floor estimation
    float smooth_nfe_prev; //For noise floor estimation
};

struct AGC_PARAM {
    /*AGC Parameters*/
    int N;                    /*Page size*/
    float th_comp;              /*Threshold compressor in dB*/
    float th_exp;               /*Threshold expander in dB*/
    float ratio_comp;         /*Compressor ratio*/
    float ratio_exp;          /*Expander ratio*/
    float attack;             /*Attack time (0,1)*/
    float release;            /*Release time (0,1)*/
    int hold_ms;              /*Hold time in ms*/
    int hold;
    float makeup;               /*Make-up gain in dB*/

    /*Parameters for gain smoothing*/
    float attack_gain;
    float release_gain;

    /*Parameters for NFE (noise floor estimate)*/
    float attack_nfe;
    float release_nfe;
    int hold_nfe_ms;
    int hold_nfe;

    /*Parameters for adaptive update of threshold and makeup gain (NFE) */
    int target_lvl;   /*Target level in dB*/
    int th_comp_lvl;   /*dB above noise floor estimate*/
    int th_exp_lvl;     /*dB below compressor threshold*/
};

struct AGCSTATE_NEW {
    struct AGC_PREV prev;
    struct AGC_PARAM param;

    bool agc_on;    /*AGC on or off*/
    bool nfe_on;    /*NFE on or off*/
    bool gui_on;    /*GUI on or off*/
    int file_size;   /*Keep track of filesize*/
    char guiString[MAX_SIZE_GUISTRING_PART_1 + MAX_SIZE_GUISTRING_PART_2];
};

/*
 * Allocate memory for an AGC
 */

struct AGCSTATE_NEW * agc_new_create(void)
{
    struct AGCSTATE_NEW *agc = malloc(sizeof(struct AGCSTATE_NEW));
    return agc;
}

/*
 * Free the memory of this agc
 */
void agc_new_destroy(struct AGCSTATE_NEW * agc)
{
    free(agc);
}

void agc_new_reset(struct AGCSTATE_NEW * agc)
{
    struct AGC_PREV * prev = &agc->prev;
    struct AGC_PARAM * param = &agc->param;

    /*AGC default on*/
    agc->agc_on = true;
    /*AGC NFE default off*/
    agc->nfe_on = false;
    /*Gui default off*/
    agc->gui_on = false;
    /*File size is 0*/
    agc->file_size = 0;
    /* Variables from previous frame*/
    /* Using same initial value as in agc_ko.m */
    prev->counter_prev = 0;
    prev->smooth_prev = 0.001f; /* Cannot be 0 because of divide by zero*/
    prev->smooth_gain_db_prev = 0;
    /* NFE variables from previous frame*/
    prev->count_nfe_prev = 0;
    prev->smooth_nfe_prev = 0.001f;

    /*Setting values for the AGC parameters*/
    param->N = AUD_INT_BUFSIZE;
    param->th_comp = -25.f;
    param->th_exp = -40.f;
    param->ratio_comp = 0.1f;
    param->ratio_exp = 1.5f;
    param->attack = 0.1f;
    param->release = 0.2f;
    param->hold_ms = 0;
    param->hold = floor(AUD_INT_SAMPLERATE_HZ * param->hold_ms
                                                        / (param->N * 1000));
    param->makeup = 12.f;

    /*Parameters for gain smoothing*/
    param->attack_gain = 0.03f;// 0.05
    param->release_gain = 0.15f;// 0.15


	//param->attack = 1.f - exp(-2.2 / (0.2 * 100));
	//param->release = 1.f - exp(-2.2 / (0.8 * 100));
	//param->attack_gain = 1.f - exp(-2.2 / (0.1 * 100));// 0.05
	//param->release_gain = 1.f - exp(-2.2 / (0.05 * 100));// 0.15
    /*Parameters for NFE (noise floor estimate)*/
    param->attack_nfe = 0.5f;
    param->release_nfe = 0.5f;
    param->hold_nfe_ms = 1000;
    param->hold_nfe = floor(AUD_INT_SAMPLERATE_HZ * param->hold_nfe_ms
                                                        / (param->N*1000));

    param->target_lvl = -13;   /*Target level in dB*/
    param->th_comp_lvl = 30;   /*dB above noise floor estimate*/
    param->th_exp_lvl = 18;     /*dB below compressor threshold*/
}

/*
 * Run the AGC
 */
void agc_new_process(struct AGCSTATE_NEW * agc,
                     const uint32_t channels,
                     const float * const absLevel,
                     float * gain,
                     bool is_res_echo,
					float noisePwr)
{
    /*Initializing variables*/
    struct AGC_PREV * prev = &agc->prev;
    struct AGC_PARAM * param = &agc->param;
    float inputRms = 0, smooth_new = 0, smooth_db_comp = 0, smooth_db = 0;
    float gain_db = 0, smooth_gain_db_new = 0, smooth_nfe = 0,
                                            smooth_nfe_db = 0, target_diff;
    int counter_new = 0, count_nfe = 0;
	float noise_db = 10 * log10f(noisePwr);

    /*  Find RMS of signal
     *  We use absLevel as a rms value
     */
    if (2 == channels) /* stereo: uses average AbsLevel */
    {
        inputRms=0.5f*(absLevel[0] + absLevel[1]);
    }
    else /* mono - default behaviour */
    {
        inputRms=absLevel[0];
    }

    inputRms = inputRms * (*gain); /* in case Gain != 1*/


    if (agc->gui_on)
        agc->guiString[0] = '\0';
    /*If gain or RMS or frame is 0 do nothing*/
    if (noisePwr < 1e-7)
    {
        /*If GUI mode write to gui string*/
        if (agc->gui_on)
        {
            char temp1[MAX_SIZE_GUISTRING_PART_1], temp2[MAX_SIZE_GUISTRING_PART_2];
            if (agc->file_size == 500) /*File has been cleaned in audconnrmixer*/
                agc->file_size = 0;

            strncat(agc->guiString, temp1, sizeof agc->guiString - strlen(agc->guiString) - 1);

            snprintf(temp2, sizeof temp2, "TH_EXP: %.6f END TH_COMP: %.6f END MAKEUP: %.6f END GAIN_NEW_DB: %.6f END ",
                param->th_exp, param->th_comp, param->makeup, smooth_gain_db_new);
            strncat(agc->guiString, temp2, sizeof agc->guiString - strlen(agc->guiString) - 1);

            agc->file_size++;
        }
        return;
    }

    /*Aattack, release, hold for NFE*/
    if (noisePwr < prev->smooth_nfe_prev)
    {
        smooth_nfe = param->attack_nfe*noisePwr + prev->smooth_nfe_prev
                                                * (1-param->attack_nfe);
    }
    else
    {
        smooth_nfe = param->release_nfe*noisePwr +
                            prev->smooth_nfe_prev * (1-param->release_nfe);

    }

    /*convert to dB*/
    smooth_nfe_db = 10 * log10f(smooth_nfe);
    /*AGC NFE*/
    if (agc->nfe_on)
    {
        /*
         * If NFE is on we update threshold and makeup gain adaptively
         */
        /*Threshold xdB above NFE, adjust by trial and error*/
        param->th_comp = smooth_nfe_db + param->th_comp_lvl;
        if (param->th_comp > param->target_lvl)
        {
            /*Turn off expander if above target level*/
            param->th_comp = param->target_lvl;
        }
        param->th_exp = param->th_comp-param->th_exp_lvl;
        target_diff = param->target_lvl - param->th_comp;
        param->makeup = target_diff - (target_diff * param->ratio_comp);
    }

    /*
     * Attack :: Release :: Hold
     */
    if (inputRms > prev->smooth_prev)
    {
        smooth_new = param->attack * inputRms + prev->smooth_prev
                                        * (1 - param->attack); /*Attack*/
        counter_new = 0;
    }
    else
    {
        if (prev->counter_prev >= param->hold)
        {
            smooth_new = param->release * inputRms + prev->smooth_prev
                                        * (1 - param->release); /*Release*/
            counter_new = prev->counter_prev + 1;
        }
        else
        {
            smooth_new = prev->smooth_prev;
            counter_new = prev->counter_prev + 1;
        }
    }

    /*Convert to Db*/
    smooth_db = 10 * log10f(smooth_new);
	
    bool needcompress = false;
	if (smooth_db < -50 && is_res_echo) {
		needcompress = true;
	}
	if (smooth_db >= param->target_lvl) { /*compress above target level*/
		smooth_db_comp = 0.7 * (smooth_db - param->target_lvl) + param->target_lvl;
	}
    else if (smooth_db >= param->th_comp) /*Compress*/
        smooth_db_comp = param->ratio_comp * (smooth_db - param->th_comp)
                                                        + param->th_comp;
    else if (smooth_db < param->th_comp && smooth_db >= param->th_exp)/*Gain=1*/
        smooth_db_comp = smooth_db; /*Gain = 1*/
    else if (smooth_db < param->th_exp) /*Expand*/
    {
        if (!needcompress) {
			smooth_db_comp = (smooth_db - param->th_exp) / (param->th_exp - noise_db) * param->makeup + smooth_db;
            if (smooth_db_comp < smooth_db - param->makeup) {
                /*Expander gain below 1 is set to 1*/
                smooth_db_comp = smooth_db - param->makeup;
            }
        }
        else {
            smooth_db_comp = smooth_db;
        }
    }

    /*Compute gain*/
    if (!agc->agc_on)
        gain_db = 0;
    else
    {
        gain_db = smooth_db_comp - smooth_db; /*Compute the gain, output/input*/
        if (!needcompress) {
            gain_db += param->makeup;             /*Add makeup gain*/
        }
    }
    /*Attack/release for gain*/
    if (gain_db > prev->smooth_gain_db_prev)
        smooth_gain_db_new = param->attack_gain * gain_db
                        + prev->smooth_gain_db_prev * (1-param->attack_gain);
    else
        smooth_gain_db_new = param->release_gain * gain_db
                        + prev->smooth_gain_db_prev * (1-param->release_gain);

    /*Convert from dB and update overall gain*/
    (*gain) *= exp(smooth_gain_db_new / 10.0f * log(10));
    /*Updating variables*/
    prev->counter_prev = counter_new;
    prev->smooth_prev = smooth_new;
    prev->smooth_gain_db_prev = smooth_gain_db_new;
    prev->count_nfe_prev = count_nfe;
    prev->smooth_nfe_prev = smooth_nfe;

    /*If in GUI mode write*/
    if (agc->gui_on)
    {
        char temp1[MAX_SIZE_GUISTRING_PART_1], temp2[MAX_SIZE_GUISTRING_PART_2];

        if (agc->file_size == 500) /*File has been cleaned in audconnrmixer*/
            agc->file_size = 0;

        strncat(agc->guiString, temp1, sizeof agc->guiString - strlen(agc->guiString) - 1);

        snprintf(temp2, sizeof temp2, "TH_EXP: %.6f END TH_COMP: %.6f END MAKEUP: %.6f END GAIN_NEW_DB: %.6f END ",
            param->th_exp, param->th_comp, param->makeup, smooth_gain_db_new);
        strncat(agc->guiString, temp2, sizeof agc->guiString - strlen(agc->guiString) - 1);

        agc->file_size++;
    }
}

/*
 * Turn agc on or off
 */

void agc_new_set_on_off(struct AGCSTATE_NEW * agc, bool val)
{
    agc->agc_on = val;
    return;
}

/*
 * Check if agc is on or off
 */
bool agc_new_check_on_off(struct AGCSTATE_NEW * agc)
{
    return agc->agc_on;
}

/*
 * Turn NFE on or off
 */
void agc_new_set_NFE_on_off(struct AGCSTATE_NEW * agc, bool val)
{
    agc->nfe_on = val;
}

void agc_new_set_param(struct AGCSTATE_NEW * agc,
                       float attack,
                       float release,
                       int hold_ms,
                       float ratio_comp,
                       float ratio_exp,
                       float attack_gain,
                       float release_gain)
{
    struct AGC_PARAM * param = &agc->param;
    param->attack = attack;
    param->release = release;
    param->hold_ms = hold_ms;
    param->ratio_comp = ratio_comp;
    param->ratio_exp = ratio_exp;
    param->attack_gain = attack_gain;
    param->release_gain = release_gain;

    /*Need to calculate hold depending on hold_ms*/
    param->hold = floor(AUD_INT_SAMPLERATE_HZ
                                        * param->hold_ms / (param->N * 1000));
}

void agc_new_set_gain_char(struct AGCSTATE_NEW *agc,
                            int th_comp,
                            int th_exp,
                            float makeup)
{
    struct AGC_PARAM * param = &agc->param;
    param->th_comp = th_comp;
    param->th_exp = th_exp;
    param->makeup = makeup;
}

void agc_new_set_nfe_param(struct AGCSTATE_NEW *agc,
                           float attack_nfe,
                           float release_nfe,
                           int hold_nfe_ms,
                           int target_lvl,
                           int th_comp_lvl,
                           int th_exp_lvl)
{
    struct AGC_PARAM * param = &agc->param;
    param->attack_nfe = attack_nfe;
    param->release_nfe = release_nfe;
    param->hold_nfe_ms = hold_nfe_ms;
    param->target_lvl = target_lvl;
    param->th_comp_lvl = th_comp_lvl;
    param->th_exp_lvl = th_exp_lvl;

    param->hold_nfe = floor(AUD_INT_SAMPLERATE_HZ
                                    * param->hold_nfe_ms / (param->N * 1000));
}

void agc_new_set_gui_on_off(struct AGCSTATE_NEW *agc, bool val)
{
    agc->gui_on = val;
}

bool agc_new_check_gui_on_off(struct AGCSTATE_NEW *agc)
{
    return agc->gui_on;
}

int agc_new_get_file_size(struct AGCSTATE_NEW *agc)
{
    return agc->file_size;
}


float agc_new_get_gain(struct AGCSTATE_NEW *agc)
{
    return (powf(10.0f, agc->prev.smooth_gain_db_prev / 20.0f));
}

void agc_new_get_gui_string(struct AGCSTATE_NEW *agc, char *guiString, const size_t max_string_size)
{
    strncat(guiString, agc->guiString, max_string_size);
}
