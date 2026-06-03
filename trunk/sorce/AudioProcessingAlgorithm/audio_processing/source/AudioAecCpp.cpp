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

/*
 * AEC processing function
 * h_aec: AEC handle pointer
 * date_in: Input audio data pointer
 * date_in : [(channel+index_tmp)*mic nubmer, cycle*480]
 * mic数据读取规则 date_in[index_tmp + channel][i + 480 * cycle])
 * ref_spk: 扬声器回采数据，没有
 * ref_mic: 参考麦克风数据，可用于远场噪声抑制，没有可以设置为NULL
 * mode: AEC mode，预留接口
 * data_out: Output audio data pointer， 1声道输出
 * cycle_num: Number of cycles， 输入数据为480的倍数
 * index_tmp: Index of the current cycle，每个mic数据所在行数，debug信息放在对应通道之前，没有置0
 *
 */
void aec_processing_cpp(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out, int cycle_num, int index_tmp)
{

    audio_pro_share *sharedata = (audio_pro_share *)aec_para.sharedata;
    SUBinterface* pSUBThread = (SUBinterface*)aec_para.pSUBThread;
// #ifdef AUDIO_WAVE_DEBUG
//     int cycle_num = 1;
//     int index_tmp = 0;
// #else
//     int cycle_num = 2;
//     int index_tmp = 3;
// #endif

// #ifdef AUDIO_WAVE_RELEASE
//     cycle_num = 1;
//     index_tmp = 0;
// #endif
    //printf("[AudioAecCpp] cycle is %d, index_tmp is %d",cycle_num,index_tmp);
    for (int cycle = 0; cycle < cycle_num; cycle++) {
        //int capture[4][480] = { 0 };

        for (int i = 0; i < aec_para.fremaelen; i++)
        {
            for (size_t channel = 0; channel < aec_para.mics_num; channel++)
            {
                sharedata->ppCapture_[channel][i] = float(date_in[index_tmp + channel][i + 480 * cycle]) / 32768.f;
            }
            if (ref_spk != NULL) {
                sharedata->pReffer_[i] = float(ref_spk[i + 480 * cycle]) / 32768.f;
            }
            else {
                sharedata->pReffer_[i] = 0.f;
            }
        }
        //pSUBThread->sub_process(sharedata, aec_para);
        pSUBThread->process_block(sharedata, aec_para);
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

void aec_processing_init_cpp(void  **p_aec, void *config)
{
    Toggle3A *toggle3a = (Toggle3A*)config;
    aec_para.mics_num = toggle3a->mics_num;
    aec_para.fremaelen = 480;
    aec_para.samplerate = 48000;
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
	if (NULL != config) {
		sharedata->bAECOn_ = toggle3a->bAECOn_;
		sharedata->bNROn_ = toggle3a->bNROn_;
		sharedata->bNRCNGOn_ = toggle3a->bNRCNGOn_;
		sharedata->bAGCOn_ = toggle3a->bAGCOn_;
		sharedata->bRNNOISEOn_ =  toggle3a->bRNNOISEOn_;
		sharedata->bPreRnnOn_ = toggle3a->bPreRnnOn_;
	}
	else {
		sharedata->bAECOn_ = true;
		sharedata->bNROn_ = true;
		sharedata->bNRCNGOn_ = false;
		sharedata->bAGCOn_ = true;
		sharedata->bRNNOISEOn_ = false;
		sharedata->bPreRnnOn_ = true;
	}
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
    pSUBThread->sub_create(sharedata);
	pSUBThread->start_sub_thread();
    aec_para.pSUBThread = (void*)pSUBThread;
    if (nullptr != p_aec) {
        *p_aec = (void*)&aec_para;
    }
    
}

void aec_processing_deinit_cpp(void *h_aec)
{
    audio_pro_share *sharedata = (audio_pro_share *)aec_para.sharedata;
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

    memset(&aec_para,0,sizeof(AEC_parameter));
    h_aec = NULL;

}


#ifdef __cplusplus
};
#endif
