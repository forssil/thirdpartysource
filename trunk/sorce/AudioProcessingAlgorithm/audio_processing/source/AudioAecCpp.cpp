#include	<stdio.h>
#include	<stdlib.h>
//#include	<string.h>
#include    <stdbool.h>
#include    <math.h>

#include    "AudioAecCpp.h"
#include    "AudioProcessingFramework_interface.h"
#include    "agc_new.h"
#include    "rnnoise.h"

static AEC_parameter aec_para;

#ifdef __cplusplus
extern "C" {
#endif


    //struct TagCAudioProcessingFrameworkInterface
    //{
    //    CAudioProcessingFrameworkInterface* pAPFInterface;
    //};

    //struct TagCAudioProcessingFrameworkInterface *GetAudioProcessingFrameworkInterface(void)
    //{
    //    return new struct TagCAudioProcessingFrameworkInterface;
    //}
//char *gs_acallocptr = NULL;
//int gs_s32LicState = 1;
//struct AEC_parameter {
//    float *data_in_f, *data_out_f;
//    float *data_in_f2, *data_out_f2, *data_out_f3;
//    audio_pro_share sharedata;
//    CAudioProcessingFrameworkInterface* pAPFInterface;
//    //CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(4, 48000, 2 * 512, 512);
//    int mics_num;
//    int fremaelen;
//    //short **buffer = new short*[5];
//};

//unsigned int getAudioAngle()
//{
//    return 230;
//}
//
//void getEncryptedPlaintex(unsigned char* plaintexId)
//{
//    if (plaintexId) {
//        printf("%s-%d: creat plaintex ok \n", __func__, __LINE__);
//        memcpy(plaintexId, "123456789abcdefghijklmnopqrstuvw", 32);
//        gs_s32LicState = 2;
//    }
//}

//void checkLicense(const unsigned char*dstLic)
//{
//    if (dstLic) {
//        printf("%s-%d: license ok \n", __func__, __LINE__);
//        gs_s32LicState = 4;
//    }
//}
//
//int getLicState()
//{
//    return gs_s32LicState;
//}

void aec_processing_cpp(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out)
{
    //for (int i = 0; i < 480; i++)
    //{
    //    for (size_t channel = 0; channel < mics_num + 1; channel++)
    //    {
    //        buffer[channel][i] = float(date_in[0 + channel][i]) / 32768.f;
    //    }
    //    sharedata.pReffer_[i] = float(ref_spk[i]) / 32768.f;
    //}
    audio_pro_share *sharedata = (audio_pro_share *)aec_para.sharedata;
    AGCSTATE_NEW *agc_new = (AGCSTATE_NEW *)aec_para.pAgc_new;
    DenoiseState* rnnoise = (DenoiseState*)aec_para.pRnnoise;
#ifdef AUDIO_WAVE_DEBUG
    int cycle_num = 1;
    int index_tmp = 0;
#else
    int cycle_num = 2;
    int index_tmp = 3;
#endif

    for(int cycle = 0; cycle < cycle_num ; cycle++){
        for (int i = 0; i < aec_para.fremaelen; i++)
        {
            for (size_t channel = 0; channel < aec_para.mics_num; channel++)
            {
                sharedata->ppCapture_[channel][i] = float(date_in[index_tmp + channel][i+480*cycle]) / 32768.f;
            }
            sharedata->pReffer_[i] = float(ref_spk[i+480*cycle]) / 32768.f;
        }

        ((CAudioProcessingFrameworkInterface *)aec_para.pAPFInterface)->process(*sharedata);
        
        memcpy(aec_para.data_out_f2, aec_para.data_in_f, aec_para.fremaelen * sizeof(float));

        size_t channel = 0;
        // do rnnoise
        if (sharedata->bRNNOISEOn_) {
            rnnoise_process_frame(rnnoise, sharedata->ppProcessOut_[channel], sharedata->ppProcessOut_[channel]);
        }

        // do agc for every output channel
        if (sharedata->bAGCOn_) {
            //for (size_t channel = 0; channel < mics_num; channel++) 

            {
                float gain = 1, power = 0;
                for (int i = 0; i < aec_para.fremaelen; i++) {
                    power += fabs(sharedata->ppProcessOut_[channel][i]);
                    //power += abs((float)data_out[i] / 32768);
                }
                power /= aec_para.fremaelen;
                //agc_process(pAgc, 1, &power, &gain, 0);
                //agc_new_process(agc_new, 1, &power, &gain, 0);
                agc_new_process(agc_new, 1, &power, &gain, sharedata->IsResEcho_);
                for (int i = 0; i < aec_para.fremaelen; i++) {
                    sharedata->ppProcessOut_[channel][i] *= gain;
                    //data_out[i] *= gain;
                }
            }
        }

        for (int i = 0; i < (aec_para.fremaelen); i++)
        {
            //for (size_t channel = 0; channel < mics_num; channel++)
            size_t channel = 0;
            {
                sharedata->ppProcessOut_[channel][i] *= 32767.f;
                if (sharedata->ppProcessOut_[channel][i] > 32767.f)
                {
                    data_out[i + channel+480*cycle] = 32767;
                }
                else if (sharedata->ppProcessOut_[channel][i] < -32768.f)
                {
                    data_out[i + channel +480*cycle] = -32768;
                }
                else {
                    data_out[i + channel +480*cycle] = short(sharedata->ppProcessOut_[channel][i]);//*32768.f
                }
            }
            //data_out[i*writewavhead.NChannels + mics_num] = (data_in_s[i*writewavhead.NChannels + mics_num]);
        }
    }
    
}

void aec_processing_init_cpp(void  **p_aec)
{
    aec_para.mics_num = 4;
    aec_para.fremaelen = 480;
    audio_pro_share *sharedata = new audio_pro_share;
    memset(sharedata, 0, sizeof(audio_pro_share));
    
    aec_para.sharedata = (void*)sharedata;

    //create AEC
    //CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(mics_num, 48000, 2 * fremaelen, fremaelen);
    //aec_para.pAPFInterface = (void*)CreateIApfInst_int(aec_para.mics_num, 48000, 2 * aec_para.fremaelen, aec_para.fremaelen);
    aec_para.pAPFInterface = (void*)CreateIApfInst_int(aec_para.mics_num, 48000, 1024, aec_para.fremaelen);
    //aec_para.pAPFInterface = (void*)CreateIApfInst_int(aec_para.mics_num, 48000, 1536, aec_para.fremaelen);
    ((CAudioProcessingFrameworkInterface *)aec_para.pAPFInterface)->Init();
    //sharedata init
    sharedata->ppCapture_ = new float*[aec_para.mics_num];
    sharedata->nChannelsInCapture_ = aec_para.mics_num;
    sharedata->nSamplesPerCaptureChannel_ = aec_para.fremaelen;
    sharedata->ppProcessOut_ = new float*[aec_para.mics_num];
    sharedata->nChannelsInProcessOut_ = aec_para.mics_num;
    sharedata->nSamplesPerProcessOutChannel_ = aec_para.fremaelen;

    //float *data_in_f, *data_out_f;
    //float *data_in_f2, *data_out_f2, *data_out_f3;

    aec_para.data_in_f = new float[aec_para.fremaelen*(2 + 2 * aec_para.mics_num)];
    aec_para.data_out_f = aec_para.data_in_f + aec_para.fremaelen;
    memset(aec_para.data_in_f, 0, (aec_para.fremaelen*(2 + 2 * aec_para.mics_num)) * sizeof(float));

    aec_para.data_in_f2 = new float[aec_para.fremaelen * 3];
    aec_para.data_out_f2 = aec_para.data_in_f2 + aec_para.fremaelen;
    aec_para.data_out_f3 = aec_para.data_out_f2 + aec_para.fremaelen;
    memset(aec_para.data_in_f2, 0, (aec_para.fremaelen * 3) * sizeof(float));

    for (int i = 0; i < aec_para.mics_num; i++) {
        sharedata->ppCapture_[i] = aec_para.data_out_f + i * aec_para.fremaelen;
        sharedata->ppProcessOut_[i] = aec_para.data_out_f + i * aec_para.fremaelen + aec_para.mics_num * aec_para.fremaelen;
    }
    sharedata->pReffer_ = aec_para.data_in_f2;
    sharedata->nSamplesInReffer_ = aec_para.fremaelen;

    sharedata->bAECOn_ = true;
    sharedata->bNROn_ = true;
    sharedata->bNRCNGOn_ = false;
    sharedata->bAGCOn_ = true;
    sharedata->bRNNOISEOn_ = true;

    sharedata->pDesire_ = aec_para.data_in_f;
    sharedata->pReffer_ = aec_para.data_in_f2;
    sharedata->pError_ = aec_para.data_out_f;

    // buffer
    
    //for (int i = 0; i < 5; i++) {
    //    buffer[i] = new short[960];
    //    memset(buffer[i], 0, sizeof(short) * 960);
    //}

    //create AGC
    //pAgc = agc_create();
    //agc_reset(pAgc);

    AGCSTATE_NEW *agc_new = agc_new_create();
    aec_para.pAgc_new = (void*)agc_new;
    agc_new_reset(agc_new);
    agc_new_set_NFE_on_off(agc_new, true);
    
    DenoiseState* rnnoise = rnnoise_create(NULL); 
    aec_para.pRnnoise = (void*)rnnoise;
}

void aec_processing_deinit_cpp(void *h_aec)
{
    audio_pro_share *sharedata = (audio_pro_share *)aec_para.sharedata;
    AGCSTATE_NEW *agc_new = (AGCSTATE_NEW *)aec_para.pAgc_new;
    delete aec_para.data_in_f;
    delete aec_para.data_in_f2;
    delete sharedata->ppCapture_;
    delete sharedata->ppProcessOut_;
    delete sharedata;
    

    //if (buffer) {
    //    for (int i = 0; i < 5; i++) {
    //        delete [] buffer[i];
    //    }
    //    delete [] buffer;
    //}
    
    //agc_destroy(pAgc);
    agc_new_destroy(agc_new);
    DenoiseState* rnnoise = (DenoiseState*)aec_para.pRnnoise;
    rnnoise_destroy(rnnoise);
    memset(&aec_para,0,sizeof(AEC_parameter));

}

//unsigned int aec_processing_get_lib_version()
//{
//    return 196;
//}

#ifdef __cplusplus
};
#endif
