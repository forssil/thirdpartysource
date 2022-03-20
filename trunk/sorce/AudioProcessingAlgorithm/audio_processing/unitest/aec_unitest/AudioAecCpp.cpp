#include	<stdio.h>
#include	<stdlib.h>
//#include	<string.h>
#include    <stdbool.h>

#include "AudioAecCpp.h"
#include    "AudioProcessingFramework_interface.h"

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

    for (int i = 0; i < aec_para.fremaelen; i++)
    {
        for (size_t channel = 0; channel < aec_para.mics_num; channel++)
        {
            aec_para.sharedata.ppCapture_[channel][i] = float(date_in[0 + channel][i]) / 32768.f;
        }
        aec_para.sharedata.pReffer_[i] = float(ref_spk[i]) / 32768.f;
    }

    aec_para.pAPFInterface->process(aec_para.sharedata);
    
    memcpy(aec_para.data_out_f2, aec_para.data_in_f, aec_para.fremaelen * sizeof(float));

    for (int i = 0; i < (aec_para.fremaelen); i++)
    {
        //for (size_t channel = 0; channel < mics_num; channel++)
        size_t channel = 0;
        {
            aec_para.sharedata.ppProcessOut_[channel][i] *= 32767.f;
            if (aec_para.data_out_f[i] > 32767.f)
            {
                data_out[i + channel] = 32767;
            }
            else if (aec_para.data_out_f[i] < -32768.f)
            {
                data_out[i + channel] = -32768;
            }
            else {
                data_out[i + channel] = short(aec_para.sharedata.ppProcessOut_[channel][i]);//*32768.f
            }
        }
        //data_out[i*writewavhead.NChannels + mics_num] = (data_in_s[i*writewavhead.NChannels + mics_num]);
    }
}

void aec_processing_init_cpp(void  **p_aec)
{
    aec_para.mics_num = 4;
    aec_para.fremaelen = 480;
    //audio_pro_share sharedata;
    memset(&aec_para.sharedata, 0, sizeof(audio_pro_share));

    //create AEC
    //CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(mics_num, 48000, 2 * fremaelen, fremaelen);
    aec_para.pAPFInterface = CreateIApfInst_int(aec_para.mics_num, 48000, 2 * aec_para.fremaelen, aec_para.fremaelen);
    aec_para.pAPFInterface->Init();
    //sharedata init
    aec_para.sharedata.ppCapture_ = new float*[aec_para.mics_num];
    aec_para.sharedata.nChannelsInCapture_ = aec_para.mics_num;
    aec_para.sharedata.nSamplesPerCaptureChannel_ = aec_para.fremaelen;
    aec_para.sharedata.ppProcessOut_ = new float*[aec_para.mics_num];
    aec_para.sharedata.nChannelsInProcessOut_ = aec_para.mics_num;
    aec_para.sharedata.nSamplesPerProcessOutChannel_ = aec_para.fremaelen;

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
        aec_para.sharedata.ppCapture_[i] = aec_para.data_out_f + i * aec_para.fremaelen;
        aec_para.sharedata.ppProcessOut_[i] = aec_para.data_out_f + i * aec_para.fremaelen + aec_para.mics_num * aec_para.fremaelen;
    }
    aec_para.sharedata.pReffer_ = aec_para.data_in_f2;
    aec_para.sharedata.nSamplesInReffer_ = aec_para.fremaelen;

    aec_para.sharedata.bAECOn_ = true;
    aec_para.sharedata.bNROn_ = true;
    aec_para.sharedata.bNRCNGOn_ = false;

    aec_para.sharedata.pDesire_ = aec_para.data_in_f;
    aec_para.sharedata.pReffer_ = aec_para.data_in_f2;
    aec_para.sharedata.pError_ = aec_para.data_out_f;

    // buffer
    
    //for (int i = 0; i < 5; i++) {
    //    buffer[i] = new short[960];
    //    memset(buffer[i], 0, sizeof(short) * 960);
    //}
}

void aec_processing_deinit_cpp(void *h_aec)
{
    delete aec_para.data_in_f;
    delete aec_para.data_in_f2;
    delete aec_para.sharedata.ppCapture_;
    delete aec_para.sharedata.ppProcessOut_;
    //if (buffer) {
    //    for (int i = 0; i < 5; i++) {
    //        delete [] buffer[i];
    //    }
    //    delete [] buffer;
    //}
}

//unsigned int aec_processing_get_lib_version()
//{
//    return 196;
//}

#ifdef __cplusplus
};
#endif
