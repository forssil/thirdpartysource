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

    for (int cycle = 0; cycle < cycle_num; cycle++) {
        for (int i = 0; i < aec_para.fremaelen; i++)
        {
            for (size_t channel = 0; channel < aec_para.mics_num; channel++)
            {
                sharedata->ppCapture_[channel][i] = float(date_in[index_tmp + channel][i + 480 * cycle]) / 32768.f;               
            }
            sharedata->pReffer_[i] = float(ref_spk[i + 480 * cycle]) / 32768.f;
        }
        size_t channel = 0;
        sharedata->FrameCounter_++;
        if (sharedata->bRNNOISEOn_ && sharedata->bPreRnnOn_) {
            float tmp[480] = { 0 };
            float tmp_in = 0.f;
            float tmp_out = 0.f;
            float tmp_diff = 0.f;
            int channel1 = 0;
            float alpha = 0;
            for (int i = 0; i < aec_para.fremaelen; i++) {
                sharedata->pRNNBufferDiff_[i + 64] = sharedata->ppCapture_[channel1][i]*32767;
                sharedata->pRNNBuffer_[i + aec_para.fremaelen] = sharedata->pRNNBufferDiff_[i];
                tmp[i] = sharedata->pRNNBuffer_[i];
                tmp_in += tmp[i] * tmp[i];
            }
            float rnn_vad = rnnoise_process_frame(rnnoise, sharedata->pRNNBufferDiff_, sharedata->pRNNBufferDiff_);
            sharedata->RnnVad_ += 0.1*(rnn_vad - sharedata->RnnVad_);
            get_rnn_gain(rnnoise, 1024, sharedata->RnnGain_);

            for (int i = 0; i < aec_para.fremaelen; i++) {
                sharedata->pRNNPOWER_[i] = alpha * sharedata->pRNNPOWER_[i] + (1 - alpha) * sharedata->pRNNBufferDiff_[i] * sharedata->pRNNBufferDiff_[i];
                tmp_out += sharedata->pRNNPOWER_[i];

                sharedata->pRNNERROR_[i] = sharedata->pRNNBuffer_[i] - sharedata->pRNNBufferDiff_[i];
                tmp_diff += sharedata->pRNNERROR_[i] * sharedata->pRNNERROR_[i];
                sharedata->pRNNERROR_[i] /= 32767;
                sharedata->pRNNBuffer_[i] = sharedata->pRNNBuffer_[i + aec_para.fremaelen]; // update one frame
                

                //sharedata->ppCapture_[channel1 - channel1][i] = sharedata->ppCapture_[channel1][i] / 32767; // out
                //sharedata->ppCapture_[channel1 - channel1][i] = sharedata->pRNNBuffer_[i] / 32767; // 
                //sharedata->ppCapture_[channel1][i] = sharedata->pRNNBuffer_[i] / 32767;//  original input  
                //sharedata->ppCapture_[channel1][i] = sharedata->pRNNBufferDiff_[i] / 32767;//  rnn output 
                //sharedata->ppCapture_[channel1 - channel1][i] = sharedata->pRNNERROR_[i] / 32767; // error 

                if (i < 64) {
                    sharedata->pRNNBufferDiff_[i] = sharedata->pRNNBufferDiff_[i + 480];// update 64 points;
                }
            }
            float nrl = 10*log10(tmp_in/(tmp_out+0.000001));
            // to do freq vad and handover vad to protect voice
            //if ((tmp_out > 1*1e4 && tmp_diff < 1e7)|| (tmp_out > 1 * 1e5) ) 
            if (tmp_out < 1 * 1e7 || nrl > 5)
            {  // -47dB
                sharedata->RNNCounter_--;
                sharedata->RNNCounter_ = sharedata->RNNCounter_ < 0 ? 0 : sharedata->RNNCounter_;
                if (sharedata->RNNCounter_ == 0) {
                    sharedata->bRNNOISEVad_ = false;
                }
                if (nrl > 30) {
                    sharedata->bRNNOISEVad_ = false;
                }
            }
            else {
                sharedata->bRNNOISEVad_ = true;
                sharedata->RNNCounter_ = 20;
            }

            if (tmp_out < 1 * 1e6 || nrl > 20) { // -57db
                sharedata->RNNCounter_enhance_--;
                sharedata->RNNCounter_enhance_ = sharedata->RNNCounter_enhance_ < 0 ? 0 : sharedata->RNNCounter_enhance_;
                if (sharedata->RNNCounter_enhance_ == 0) {
                    sharedata->bRNNOISEVad_enhance_ = false;
                }
            }
            else {
                sharedata->bRNNOISEVad_enhance_ = true;
                sharedata->RNNCounter_enhance_ = 20;
            }
        }

        ((CAudioProcessingFrameworkInterface *)aec_para.pAPFInterface)->process(*sharedata);
        
        memcpy(aec_para.data_out_f2, aec_para.data_in_f, aec_para.fremaelen * sizeof(float));

        
        // do rnnoise
        // to do: move rnn before aec
        if (sharedata->bRNNOISEOn_ && !sharedata->bPreRnnOn_) {
            float tmp[480] = { 0 };
            float tmp_in = 0.f;
            float tmp_out = 0.f;
            float tmp_diff = 0.f;
            int channel1 = 1;
            float alpha = 0;
            for (int i = 0; i < aec_para.fremaelen; i++) {
                sharedata->ppProcessOut_[channel1][i] *= 32767;
                sharedata->pRNNBuffer_[i+ aec_para.fremaelen] = sharedata->ppProcessOut_[channel1][i];
                tmp[i] = sharedata->pRNNBuffer_[i];
                tmp_in += tmp[i] * tmp[i];
            }
             rnnoise_process_frame(rnnoise, sharedata->ppProcessOut_[channel1], sharedata->ppProcessOut_[channel1]);

            for (int i = 0; i < aec_para.fremaelen; i++) {
                sharedata->pRNNPOWER_[i] = alpha * sharedata->pRNNPOWER_[i] + (1- alpha) * sharedata->ppProcessOut_[channel1][i] * sharedata->ppProcessOut_[channel1][i];
                tmp_out += sharedata->pRNNPOWER_[i];

                sharedata->pRNNERROR_[i] = sharedata->pRNNBuffer_[i] - sharedata->ppProcessOut_[channel1][i];
                tmp_diff += sharedata->pRNNERROR_[i] * sharedata->pRNNERROR_[i];
                sharedata->pRNNBuffer_[i] = sharedata->pRNNBuffer_[i + aec_para.fremaelen]; // update one frame

                //sharedata->ppProcessOut_[channel1 - channel1][i] = sharedata->ppProcessOut_[channel1][i] / 32767;
                //sharedata->ppProcessOut_[channel1 - channel1][i] = sharedata->pRNNBuffer_[i] / 32767;
                //sharedata->ppProcessOut_[channel1][i] = tmp[i] / 32767;
                //sharedata->ppProcessOut_[channel1 - channel1][i] = sharedata->pRNNERROR_[i] / 32767;
            }

            //if (tmp_in > 1e4 && tmp_out < 1e3 && tmp_diff > 1e3) {  // -50dB -60dB
            //    sharedata->bRNNOISEVad_ = false;
            //}
            //else {
            //    sharedata->bRNNOISEVad_ = true;
            //}
            // to do freq vad and handover vad to protect voice
            //if ((tmp_out > 1*1e4 && tmp_diff < 1e7)|| (tmp_out > 1 * 1e5) ) 
            if (tmp_out > 1 * 1e4)
            {  // -50dB -53dB -57
                sharedata->bRNNOISEVad_ = true;
                sharedata->RNNCounter_ = 5;
            }
            else {
                sharedata->RNNCounter_--;
                sharedata->RNNCounter_ = sharedata->RNNCounter_ < 0 ? 0 : sharedata->RNNCounter_;
                if (sharedata->RNNCounter_ == 0) {
                    sharedata->bRNNOISEVad_ = false;
                }                
            }

            if (tmp_out < 1 * 1e5) {
                sharedata->RNNCounter_enhance_--;
                sharedata->RNNCounter_enhance_ = sharedata->RNNCounter_enhance_ < 0 ? 0 : sharedata->RNNCounter_enhance_;
                if (sharedata->RNNCounter_enhance_ == 0) {
                    sharedata->bRNNOISEVad_enhance_ = false;
                }                
            }
            else {
                sharedata->bRNNOISEVad_enhance_ = true;
                sharedata->RNNCounter_enhance_ = 20;
            }
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
	((CAudioProcessingFrameworkInterface *)aec_para.pAPFInterface)->SetMainMicIndex(0);
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

    aec_para.data_in_f2 = new float[aec_para.fremaelen * 7 + 64 + 512];
    aec_para.data_out_f2 = aec_para.data_in_f2 + aec_para.fremaelen;
    aec_para.data_out_f3 = aec_para.data_out_f2 + aec_para.fremaelen;
    aec_para.data_out_f4 = aec_para.data_out_f3 + aec_para.fremaelen;
    aec_para.data_out_f5 = aec_para.data_out_f4 + aec_para.fremaelen;
    aec_para.data_out_f6 = aec_para.data_out_f5 + 2 * aec_para.fremaelen;
    aec_para.data_out_f7 = aec_para.data_out_f6 + aec_para.fremaelen + 64;

    memset(aec_para.data_in_f2, 0, (aec_para.fremaelen * 7 + 64 + 512) * sizeof(float));

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
    sharedata->bPreRnnOn_ = true;

    sharedata->RNNCounter_ = 0;
    sharedata->FrameCounter_ = 0;
    sharedata->RnnVad_ = 0.f;

    sharedata->pDesire_ = aec_para.data_in_f;
    sharedata->pReffer_ = aec_para.data_in_f2;
    sharedata->pError_ = aec_para.data_out_f;
    sharedata->pRNNERROR_ = aec_para.data_out_f4;
    sharedata->pRNNPOWER_ = aec_para.data_out_f3;
    sharedata->pRNNBuffer_ = aec_para.data_out_f5; // 480 * 2
    sharedata->pRNNBufferDiff_ = aec_para.data_out_f6; // 480 + 64
    sharedata->RnnGain_ = aec_para.data_out_f7; // 512

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
