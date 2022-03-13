#include    "AudioAec.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include    <stdbool.h>

#include <memory>
#include <stdio.h>
#include <time.h>
//#include "timecounter.h"
#ifdef WIN32
#include <windows.h>
#endif

#include "AudioProcessingFramework_interface.h"
//#include "WaveIO.h"
#ifndef _CLOCK_T_DEFINED 
typedef long clock_t;
#define _CLOCK_T_DEFINED 
#endif 

#define framesize 04.f
#define floatfile

char *gs_acallocptr = NULL;
int gs_s32LicState = 1;

unsigned int getAudioAngle()
{
    return 230;
}

void getEncryptedPlaintex(unsigned char* plaintexId)
{
    if (plaintexId) {
        printf("%s-%d: creat plaintex ok \n", __func__, __LINE__);
        memcpy(plaintexId, "123456789abcdefghijklmnopqrstuvw", 32);
        gs_s32LicState = 2;
    }
}

void checkLicense(const unsigned char*dstLic)
{
    if (dstLic) {
        printf("%s-%d: license ok \n", __func__, __LINE__);
        gs_s32LicState = 4;
    }
}

int getLicState()
{
    return gs_s32LicState;
}

void aec_processing(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out)
{
    static FILE   *fp = NULL;
    static FILE   *fp1 = NULL;
    static FILE   *fp2 = NULL;
    static FILE   *fp3 = NULL;
    static FILE   *fp_ref = NULL;
    static FILE   *fp_out = NULL;
    static int	 index = 0;
    static int	 logidx = 0;
    static int    size = 0;
#define	MAX_RECORD_TIMES	20000

    if (logidx++ > 500) {
        logidx = 0;
        printf("%s-%d: =============== aduio size =%d ========== \n", __func__, __LINE__, size);
    }
    if (gs_acallocptr == NULL) {
        printf("%s-%d: malloc fail \n", __func__, __LINE__);
        return;
    }
    //printf("%s-%d: mode =%d %p %p \n",__func__,__LINE__,mode,date_in[3],&date_in[3][0]);   
    //memcpy((void *)data_out, (void *)(&date_in[3][0]), 960 * 2);
    for (int i = 0; i < fremaelen; i++)
    {
        for (size_t channel = 0; channel < mics_num; channel++)
        {
            sharedata.ppCapture_[channel][i] = float(date_in[3 + channel][i]) / 32768.f;
        }
        sharedata.pReffer_[i] = float(ref_spk[i]) / 32768.f;
    }

    pAPFInterface->process(sharedata);
    
    memcpy(data_out_f2, data_in_f, fremaelen * sizeof(float));
    /////////////
    for (int i = 0; i < (fremaelen); i++)
    {
        //for (size_t channel = 0; channel < mics_num; channel++)
        size_t channel = 0;
        {
            sharedata.ppProcessOut_[channel][i] *= 32767.f;
            if (data_out_f[i] > 32767.f)
            {
                data_out[i + channel] = 32767;
            }
            else if (data_out_f[i] < -32768.f)
            {
                data_out[i + channel] = -32768;
            }
            else {
                data_out[i + channel] = short(sharedata.ppProcessOut_[channel][i]);//*32768.f
            }
        }

        //data_out[i*writewavhead.NChannels + mics_num] = (data_in_s[i*writewavhead.NChannels + mics_num]);

    }
    bool valid = (fp == NULL) && (fp1 == NULL) && (fp2 == NULL) && (fp3 == NULL) && (fp_ref == NULL) && (fp_out == NULL);
    if (valid && (index < MAX_RECORD_TIMES)) {
        fp = fopen("/dev/chn0.pcm", "wb");
        fp1 = fopen("/dev/chn1.pcm", "wb");
        fp2 = fopen("/dev/chn2.pcm", "wb");
        fp3 = fopen("/dev/chn3.pcm", "wb");
        fp_ref = fopen("/dev/chnref.pcm", "wb");
        fp_out = fopen("/dev/chnout.pcm", "wb");
    }
    valid = (fp != NULL) && (fp1 != NULL) && (fp2 != NULL) && (fp3 != NULL) && (fp_ref != NULL) && (fp_out != NULL);
    if (valid && (index < MAX_RECORD_TIMES)) {
        fwrite((char *)(&date_in[3][0]), 960 * 2, 1, fp);
        fwrite((char *)(&date_in[4][0]), 960 * 2, 1, fp1);
        fwrite((char *)(&date_in[5][0]), 960 * 2, 1, fp2);
        fwrite((char *)(&date_in[6][0]), 960 * 2, 1, fp3);
        fwrite((char *)(&ref_spk[0]), 960 * 2, 1, fp_ref);
        fwrite((char *)(&data_out[0]), 960 * 2, 1, fp_out);
        index++;
        size = ftell(fp);
        if (index >= MAX_RECORD_TIMES) {
            fclose(fp);
            fclose(fp1);
            fclose(fp2);
            fclose(fp3);
            fclose(fp_ref);
            fclose(fp_out);
            fp = NULL;
            fp1 = NULL;
            fp2 = NULL;
            fp3 = NULL;
            fp_ref = NULL;
            fp_out = NULL;
            printf("%s-%d: ================== stop save =================== \n", __func__, __LINE__);
        }
    }
    if (access("/oem/aserver/debug", F_OK) == 0) {
        printf("%s-%d: ================== start save =================== \n", __func__, __LINE__);
        if (fp != NULL) {
            fclose(fp);
        }
        fp = NULL;
        if (fp1 != NULL) {
            fclose(fp1);
        }
        fp1 = NULL;
        if (fp2 != NULL) {
            fclose(fp2);
        }
        fp2 = NULL;
        if (fp3 != NULL) {
            fclose(fp3);
        }
        fp3 = NULL;
        if (fp_ref != NULL) {
            fclose(fp_ref);
        }
        fp_ref = NULL;
        if (fp_out != NULL) {
            fclose(fp_out);
        }
        fp_out = NULL;

        index = 0;
        size = 0;
        logidx = 0;
        system("rm /oem/aserver/debug -f");
    }
}

void aec_processing_init(void  **p_aec)
{
    if (gs_acallocptr == NULL) {
        printf("%s-%d: ca test libaec.so date:%s time:%s \n", __func__, __LINE__, __DATE__, __TIME__);
        gs_acallocptr = (char *)malloc(24);
        if (gs_acallocptr != NULL) {
            *p_aec = (void *)gs_acallocptr;
            printf("%s-%d: malloc succeed \n", __func__, __LINE__);
        }
        else {
            printf("%s-%d: malloc fail \n", __func__, __LINE__);
        }
    }

    //audio_pro_share sharedata;
    memset(&sharedata, 0, sizeof(audio_pro_share));

    //create AEC
    //CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(mics_num, readwavhead.SampleRate, 2 * fremaelen, fremaelen);
    pAPFInterface = CreateIApfInst_int(4, 48000, 2 * 480, 480);
    pAPFInterface->Init();
    //sharedata init
    sharedata.ppCapture_ = new float*[mics_num];
    sharedata.nChannelsInCapture_ = mics_num;
    sharedata.nSamplesPerCaptureChannel_ = fremaelen;
    sharedata.ppProcessOut_ = new float*[mics_num];
    sharedata.nChannelsInProcessOut_ = mics_num;
    sharedata.nSamplesPerProcessOutChannel_ = fremaelen;

    //float *data_in_f, *data_out_f;
    //float *data_in_f2, *data_out_f2, *data_out_f3;

    data_in_f = new float[fremaelen*(2 + 2 * mics_num)];
    data_out_f = data_in_f + fremaelen;
    memset(data_in_f, 0, (fremaelen*(2 + 2 * mics_num)) * sizeof(float));

    data_in_f2 = new float[fremaelen * 3];
    data_out_f2 = data_in_f2 + fremaelen;
    data_out_f3 = data_out_f2 + fremaelen;
    memset(data_in_f2, 0, (fremaelen * 3) * sizeof(float));

    for (int i = 0; i < mics_num; i++) {
        sharedata.ppCapture_[i] = data_out_f + i * fremaelen;
        sharedata.ppProcessOut_[i] = data_out_f + i * fremaelen + mics_num * fremaelen;
    }
    sharedata.pReffer_ = data_in_f2;
    sharedata.nSamplesInReffer_ = fremaelen;

    sharedata.bAECOn_ = true;
    sharedata.bNROn_ = true;
    sharedata.bNRCNGOn_ = false;

    sharedata.pDesire_ = data_in_f;
    sharedata.pReffer_ = data_in_f2;
    sharedata.pError_ = data_out_f;
}

void aec_processing_deinit(void *h_aec)
{
    printf("%s-%d: ca test libaec.so date:%s time:%s \n", __func__, __LINE__, __DATE__, __TIME__);
    delete data_in_f;
    delete data_in_f2;
    delete sharedata.ppCapture_;
    delete sharedata.ppProcessOut_;
}

unsigned int aec_processing_get_lib_version()
{
    return 196;
}
