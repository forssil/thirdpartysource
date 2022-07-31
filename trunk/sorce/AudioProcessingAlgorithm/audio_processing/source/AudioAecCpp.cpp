#include	<stdio.h>
#include	<stdlib.h>
//#include	<string.h>
#include    <stdbool.h>
#include    <math.h>
#include    <chrono>
#include    "AudioAecCpp.h"
#include    "AudioProcessingFramework_interface.h"
#include    "agc_new.h"
#include    "rnnoise.h"
#include    "SUBinterface.h"


static AEC_parameter aec_para;

#ifdef __cplusplus
extern "C" {
#endif


void aec_processing_cpp(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out)
{

    audio_pro_share *sharedata = (audio_pro_share *)aec_para.sharedata;
    //AGCSTATE_NEW *agc_new = (AGCSTATE_NEW *)aec_para.pAgc_new;
    //DenoiseState* rnnoise = (DenoiseState*)aec_para.pRnnoise;
    SUBinterface* pSUBThread = (SUBinterface*)aec_para.pSUBThread;

#ifdef AUDIO_WAVE_DEBUG
    int cycle_num = 1;
    int index_tmp = 0;
#else
    int cycle_num = 2;
    int index_tmp = 3;
#endif

#ifdef AUDIO_WAVE_RELEASE
    cycle_num = 1;
    index_tmp = 0;
#endif

    for (int cycle = 0; cycle < cycle_num; cycle++) {
        //int capture[4][480] = { 0 };

        for (int i = 0; i < aec_para.fremaelen; i++)
        {
            for (size_t channel = 0; channel < aec_para.mics_num; channel++)
            {
                sharedata->ppCapture_[channel][i] = float(date_in[index_tmp + channel][i + 480 * cycle]) / 32768.f;
            }
            sharedata->pReffer_[i] = float(ref_spk[i + 480 * cycle]) / 32768.f;
        }

        pSUBThread->sub_process(sharedata, aec_para);
		/*while (1) {
			if (pSUBThread->get_finish_flag()) {
				break;
			}
			else {
				std::chrono::duration<int, std::micro> timespan(50);
				std::this_thread::sleep_for(timespan);
			}
		}*/
		//pSUBThread->task(sharedata);
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
 //   //CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(mics_num, 48000, 2 * fremaelen, fremaelen);
 //   //aec_para.pAPFInterface = (void*)CreateIApfInst_int(aec_para.mics_num, 48000, 2 * aec_para.fremaelen, aec_para.fremaelen);
 //   aec_para.pAPFInterface = (void*)CreateIApfInst_int(aec_para.mics_num, 48000, 1024, aec_para.fremaelen);
 //   //aec_para.pAPFInterface = (void*)CreateIApfInst_int(aec_para.mics_num, 48000, 1536, aec_para.fremaelen);
 //   ((CAudioProcessingFrameworkInterface *)aec_para.pAPFInterface)->Init();
	//((CAudioProcessingFrameworkInterface *)aec_para.pAPFInterface)->SetMainMicIndex(0);
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
    sharedata->bRNNOISEOn_ = false;
    sharedata->bPreRnnOn_ = true;

    sharedata->bRNNOISEVad_ = true;
    sharedata->bRNNOISEVad_enhance_ = true;

    sharedata->RNNCounter_ = 0;
    sharedata->FrameCounter_ = 0;
    sharedata->RnnVad_ = 0.f;
    sharedata->fAGCgain_ = 1.f;

    sharedata->pDesire_ = aec_para.data_in_f;
    sharedata->pReffer_ = aec_para.data_in_f2;
    sharedata->pError_ = aec_para.data_out_f;
    sharedata->pRNNERROR_ = aec_para.data_out_f4;
    sharedata->pRNNPOWER_ = aec_para.data_out_f3;
    sharedata->pRNNBuffer_ = aec_para.data_out_f5; // 480 * 2
    sharedata->pRNNBufferDiff_ = aec_para.data_out_f6; // 480 + 64
    sharedata->RnnGain_ = aec_para.data_out_f7; // 512

    for (int i = 0; i < aec_para.fremaelen; i++) {
        sharedata->RnnGain_[i] = 1.f;
    }

    // buffer
    
    //for (int i = 0; i < 5; i++) {
    //    buffer[i] = new short[960];
    //    memset(buffer[i], 0, sizeof(short) * 960);
    //}

    //create AGC
    //pAgc = agc_create();
    //agc_reset(pAgc);

    // AGCSTATE_NEW *agc_new = agc_new_create();
    // aec_para.pAgc_new = (void*)agc_new;
    // agc_new_reset(agc_new);
    // agc_new_set_NFE_on_off(agc_new, true);

    
    SUBinterface *pSUBThread = new SUBinterface;
    pSUBThread->sub_create();
	pSUBThread->start_sub_thread();
    aec_para.pSUBThread = (void*)pSUBThread;

    //DenoiseState* rnnoise = rnnoise_create(NULL); 
    //aec_para.pRnnoise = (void*)rnnoise;

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
    //agc_new_destroy(agc_new);

    SUBinterface* pSUBThread = (SUBinterface*)aec_para.pSUBThread;
	pSUBThread->stop_sub_thread();
    delete pSUBThread;

    //DenoiseState* rnnoise = (DenoiseState*)aec_para.pRnnoise;
    //rnnoise_destroy(rnnoise);

    memset(&aec_para,0,sizeof(AEC_parameter));

}


#ifdef __cplusplus
};
#endif
