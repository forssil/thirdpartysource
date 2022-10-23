#include	<stdio.h>
#include	<stdlib.h>
//#include	<string.h>
#include    <stdbool.h>
#include    <math.h>

#include    "../../audio_processing/include/SUBinterface.h"




//#ifdef __cplusplus
//extern "C" {
//#endif

SUBinterface::SUBinterface() {
	audio_3A_thread_running_ = true;
	task_finished_ = false;
    if (1) {
        if (NULL == fp) {
            fp = fopen("/dev/chn0.pcm", "wb");
        }
        if (NULL == fp1) {
            fp1 = fopen("/dev/chn1.pcm", "wb");
        }
        if (NULL == fp2) {
            fp2 = fopen("/dev/chn2.pcm", "wb");
        }
        if (NULL == fp3) {
            fp3 = fopen("/dev/chn3.pcm", "wb");
        }
        if (NULL == fp_ref) {
            fp_ref = fopen("/dev/chnref.pcm", "wb");
        }
        if (NULL == fp_out) {
            fp_out = fopen("/dev/chnout.pcm", "wb");
        }
    }
	
	dump_idx = 0;
    aec_mic0_buffer_.reset(new SUBBuffer(2)); 
    aec_mic0_buffer_->SetFramePara(48000,480*1,1);
    aec_mic1_buffer_.reset(new SUBBuffer(2));
    aec_mic1_buffer_->SetFramePara(48000, 480 * 1, 1);
    aec_mic2_buffer_.reset(new SUBBuffer(2));
    aec_mic2_buffer_->SetFramePara(48000, 480 * 1, 1);
    aec_mic3_buffer_.reset(new SUBBuffer(2));
    aec_mic3_buffer_->SetFramePara(48000, 480 * 1, 1);
    aec_ref_buffer_.reset(new SUBBuffer(2));
    aec_ref_buffer_->SetFramePara(48000, 480 * 1, 1);
    aec_output_buffer_.reset(new SUBBuffer(2));
    aec_output_buffer_->SetFramePara(48000, 480 * 1, 1);

    //sub_create();


    aec_mic0_buffer_->StartBuffer();
    aec_mic1_buffer_->StartBuffer();
    aec_mic2_buffer_->StartBuffer();
    aec_mic3_buffer_->StartBuffer();
    aec_ref_buffer_->StartBuffer();
    aec_output_buffer_->StartBuffer();
    
    //start_sub_thread();

}

SUBinterface::~SUBinterface() {
    //stop_sub_thread();

    aec_mic0_buffer_->StopBuffer();
    aec_mic1_buffer_->StopBuffer();
    aec_mic2_buffer_->StopBuffer();
    aec_mic3_buffer_->StopBuffer();
    aec_ref_buffer_->StopBuffer();
    aec_output_buffer_->StopBuffer();

    delete data_in_f;
    delete data_in_f2;
    delete share_data_->ppCapture_;
    delete share_data_->ppProcessOut_;
    delete share_data_;

    agc_new_destroy(agc_new_);

    rnnoise_destroy(rnn_noise_);
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
	dump_idx = 0;
}

void SUBinterface::sub_create(audio_pro_share *share_data) {
    aec_ = CreateIApfInst_int(mics_num_, 48000, 1024, framelen_);
    //aec_para.pAPFInterface = (void*)CreateIApfInst_int(aec_para.mics_num, 48000, 1536, aec_para.fremaelen);
    aec_->Init();
    aec_->SetMainMicIndex(0);

    //sharedata init
    share_data_ = new audio_pro_share;
    memset(share_data_, 0, sizeof(audio_pro_share));
    share_data_->ppCapture_ = new float*[mics_num_];
    share_data_->nChannelsInCapture_ = mics_num_;
    share_data_->nSamplesPerCaptureChannel_ = framelen_;
    share_data_->ppProcessOut_ = new float*[mics_num_];
    share_data_->nChannelsInProcessOut_ = mics_num_;
    share_data_->nSamplesPerProcessOutChannel_ = framelen_;

    ////float *data_in_f, *data_out_f;
    ////float *data_in_f2, *data_out_f2, *data_out_f3;

    data_in_f = new float[framelen_*(2 + 2 * mics_num_)];
    data_out_f = data_in_f + framelen_;
    memset(data_in_f, 0, (framelen_*(2 + 2 * mics_num_)) * sizeof(float));

    data_in_f2 = new float[framelen_ * 7 + 64 + 512];
    data_out_f2 = data_in_f2 + framelen_;
    data_out_f3 = data_out_f2 + framelen_;
    data_out_f4 = data_out_f3 + framelen_;
    data_out_f5 = data_out_f4 + framelen_;
    data_out_f6 = data_out_f5 + 2 * framelen_;
    data_out_f7 = data_out_f6 + framelen_ + 64;

    memset(data_in_f2, 0, (framelen_ * 7 + 64 + 512) * sizeof(float));

    for (int i = 0; i < mics_num_; i++) {
        share_data_->ppCapture_[i] = data_out_f  + i * framelen_;
        share_data_->ppProcessOut_[i] = data_out_f  + i * framelen_ + mics_num_ * framelen_;
    }
    //for (int i = 0; i < aec_para.mics_num; i++) {
    //    sharedata->ppCapture_[i] = aec_para.data_out_f + i * aec_para.fremaelen;
    //    sharedata->ppProcessOut_[i] = aec_para.data_out_f + i * aec_para.fremaelen + aec_para.mics_num * aec_para.fremaelen;
    //}
    share_data_->pReffer_ = data_in_f2;
    share_data_->nSamplesInReffer_ = framelen_;

    share_data_->bAECOn_ = share_data->bAECOn_;
    share_data_->bNROn_ = share_data->bNROn_;
    share_data_->bNRCNGOn_ = share_data->bNRCNGOn_;
    share_data_->bAGCOn_ = share_data->bAGCOn_;
    share_data_->bRNNOISEOn_ = share_data->bRNNOISEOn_;
    share_data_->bPreRnnOn_ = share_data->bPreRnnOn_;

    share_data_->bRNNOISEVad_ = true;
    share_data_->bRNNOISEVad_enhance_ = true;

    share_data_->RNNCounter_ = 0;
    share_data_->FrameCounter_ = 0;
    share_data_->RnnVad_ = 0.f;
    share_data_->fAGCgain_ = 1.f;

    share_data_->pDesire_ = data_in_f;
    share_data_->pReffer_ = data_in_f2;
    share_data_->pError_ = data_out_f;
    share_data_->pRNNERROR_ = data_out_f4;
    share_data_->pRNNPOWER_ = data_out_f3;
    share_data_->pRNNBuffer_ = data_out_f5; // 480 * 2
    share_data_->pRNNBufferDiff_ = data_out_f6; // 480 + 64
    share_data_->RnnGain_ = data_out_f7; // 512

    for (int i = 0; i < framelen_; i++) {
        share_data_->RnnGain_[i] = 1.f;
    }

    agc_new_ = agc_new_create();
    agc_new_reset(agc_new_);
    agc_new_set_NFE_on_off(agc_new_, true);

    rnn_noise_ = rnnoise_create(NULL);
	//sub_thread_ = new std::thread([this, sharedata, aec_para] {this->task(sharedata); });
    sub_thread_ = new std::thread(&SUBinterface::threadrun,this);

}
void SUBinterface::threadrun() {
    while (audio_3A_thread_running_) {

        //framelen_ = aec_para.fremaelen;
        //counter++;

        //task_finished_ = false;
        {
            std::unique_lock<std::mutex> lock(sub_thread_mutex_);
            task();
        }
        
        //start_sub_thread();
        //sub_thread_->join();

        //if (task_finished_) {
        //    break;
        //}
    }
}
void SUBinterface::sub_process(audio_pro_share * sharedata, AEC_parameter aec_para) {
    {
        std::unique_lock<std::mutex> lock(sub_thread_mutex_);

        //push
        framelen_ = aec_para.fremaelen;

        // push data to sub thread
        std::vector<float> tmp_mic0(480, 0.f);
        memcpy(tmp_mic0.data(), sharedata->ppCapture_[0], sizeof(float) * 480);
        aec_mic0_buffer_->PushOneFrame(tmp_mic0);

        std::vector<float> tmp_mic1(480, 0.f);
        memcpy(tmp_mic1.data(), sharedata->ppCapture_[1], sizeof(float) * 480);
        aec_mic1_buffer_->PushOneFrame(tmp_mic1);

        std::vector<float> tmp_mic2(480, 0.f);
        memcpy(tmp_mic2.data(), sharedata->ppCapture_[2], sizeof(float) * 480);
        aec_mic2_buffer_->PushOneFrame(tmp_mic2);
        std::vector<float> tmp_mic3(480, 0.f);
        memcpy(tmp_mic3.data(), sharedata->ppCapture_[3], sizeof(float) * 480);
        aec_mic3_buffer_->PushOneFrame(tmp_mic3);

        std::vector<float> tmp_ref(480, 0.f);
        memcpy(tmp_ref.data(), sharedata->pReffer_, sizeof(float) * 480);
        aec_ref_buffer_->PushOneFrame(tmp_ref);
        task_finished_ = false;
    }
		//task(sharedata);
		//start_sub_thread();
		//sub_thread_->join();
        int counter_thread = 0;
        while (1) {
            if (get_finish_flag()) {
                break;
            }
            else {
                std::chrono::duration<int, std::micro> timespan(50);
                std::this_thread::sleep_for(timespan);
            }
            counter_thread++;
            if (counter_thread > 150) {
                break;
            }
        }
        {
            std::unique_lock<std::mutex> lock(sub_thread_mutex_);
            // pop data from sub thread
            std::vector<float> tmp_out(480, 0.f);
            int ret_out = aec_output_buffer_->PopOneFrame(tmp_out);
            if (ret_out < 0) {

            }
            for (int i = 0; i < framelen_; i++) {
                sharedata->ppProcessOut_[0][i] = tmp_out[i];
            }
        }

		/*if (task_finished_) {
			break;
		}*/

}
void SUBinterface::start_sub_thread() {
    std::unique_lock<std::mutex> lock(sub_thread_mutex_);
    //audio_3A_thread_running_ = true;
    //sub_thread_ = std::thread([this]() {
    //    this->task();
    //});
    //sub_thread_ = std::thread([this] { this->task(); });
}

void SUBinterface::stop_sub_thread() {
  
    if (sub_thread_->joinable()) {
        {
            std::unique_lock<std::mutex> lock(sub_thread_mutex_);
            audio_3A_thread_running_ = false;
        }
        sub_thread_->join();
    }
	
}

void SUBinterface::task() {
    //while (audio_encode_running_) {

    // pop data from main thread
    if (aec_mic0_buffer_->GetRefQueSize() <= 0) {
        return;
    }
    std::vector<std::vector<float>> micin(5,std::vector<float>(480,0.f));
    int ret0 = aec_mic0_buffer_->PopOneFrame(micin[0]);
    int ret1 = aec_mic1_buffer_->PopOneFrame(micin[1]);
    int ret2 = aec_mic2_buffer_->PopOneFrame(micin[2]);
    int ret3 = aec_mic3_buffer_->PopOneFrame(micin[3]);
    int ret4 = aec_ref_buffer_->PopOneFrame(micin[4]);

    if (ret0 + ret1 + ret2 + ret3+ ret4 == 0 && agc_new_ != nullptr && rnn_noise_ != nullptr) {

        for (int i = 0; i < framelen_; i++)
        {
            for (size_t channel = 0; channel < mics_num_; channel++)
            {
                share_data_->ppCapture_[channel][i] = micin[channel][i];
            }
            share_data_->pReffer_[i] = micin[4][i];
        }

        size_t channel = 0;
        share_data_->FrameCounter_++;

        if (share_data_->bRNNOISEOn_ && share_data_->bPreRnnOn_) {
            float tmp[480] = { 0 };
            float tmp_in = 0.f;
            float tmp_out = 0.f;
            float tmp_diff = 0.f;
            int channel1 = 0;
            float alpha = 0;
            for (int i = 0; i < framelen_; i++) {
                share_data_->pRNNBufferDiff_[i + 64] = share_data_->ppCapture_[channel1][i] * 32767;
                share_data_->pRNNBuffer_[i + framelen_] = share_data_->pRNNBufferDiff_[i];
                tmp[i] = share_data_->pRNNBuffer_[i];
                tmp_in += tmp[i] * tmp[i];
            }
            float RNN_vad = rnnoise_process_frame(rnn_noise_, share_data_->pRNNBufferDiff_, share_data_->pRNNBufferDiff_);
            //share_data_->RnnVad_ += 0.1*(RNN_vad - share_data_->RnnVad_);
            share_data_->RnnVad_ = RNN_vad;
            get_rnn_gain(rnn_noise_, 1024, share_data_->RnnGain_);

            for (int i = 0; i < framelen_; i++) {
                share_data_->pRNNPOWER_[i] = alpha * share_data_->pRNNPOWER_[i] + (1 - alpha) * share_data_->pRNNBufferDiff_[i] * share_data_->pRNNBufferDiff_[i];
                tmp_out += share_data_->pRNNPOWER_[i];

                share_data_->pRNNERROR_[i] = share_data_->pRNNBuffer_[i] - share_data_->pRNNBufferDiff_[i];
                //tmp_diff += share_data_->pRNNERROR_[i] * share_data_->pRNNERROR_[i];
                share_data_->pRNNERROR_[i] /= 32767;
                share_data_->pRNNBuffer_[i] = share_data_->pRNNBuffer_[i + framelen_]; // update one frame


                //share_data_->ppCapture_[channel1 - channel1][i] = share_data_->ppCapture_[channel1][i] / 32767; // out
                //share_data_->ppCapture_[channel1 - channel1][i] = share_data_->psubBuffer_[i] / 32767; // 
                //share_data_->ppCapture_[channel1][i] = share_data_->pRNNBuffer_[i] / 32767;//  original input  
                //share_data_->ppCapture_[channel1][i] = share_data_->pRNNBufferDiff_[i] / 32767;//  RNN output 
                //share_data_->ppCapture_[channel1 - channel1][i] = share_data_->pRNNERROR_[i] / 32767; // error 

                if (i < 64) {
                    share_data_->pRNNBufferDiff_[i] = share_data_->pRNNBufferDiff_[i + 480];// update 64 points;
                }
            }
            float nrl = 10 * log10(tmp_in / (tmp_out + 0.000001));
            // to do freq vad and handover vad to protect voice
            //if ((tmp_out > 1*1e4 && tmp_diff < 1e7)|| (tmp_out > 1 * 1e5) ) 
            if (tmp_out < 1 * 1e7 || nrl > 5)
            {  // -47dB
                share_data_->RNNCounter_--;
                share_data_->RNNCounter_ = share_data_->RNNCounter_ < 0 ? 0 : share_data_->RNNCounter_;
                if (share_data_->RNNCounter_ == 0) {
                    share_data_->bRNNOISEVad_ = false;
                }
                if (nrl > 30) {
                    share_data_->bRNNOISEVad_ = false;
                }
            }
            else {
                share_data_->bRNNOISEVad_ = true;
                share_data_->RNNCounter_ = 20;
            }

            if (tmp_out < 1 * 1e6 || nrl > 20) { // -57db
                share_data_->RNNCounter_enhance_--;
                share_data_->RNNCounter_enhance_ = share_data_->RNNCounter_enhance_ < 0 ? 0 : share_data_->RNNCounter_enhance_;
                if (share_data_->RNNCounter_enhance_ == 0) {
                    share_data_->bRNNOISEVad_enhance_ = false;
                }
            }
            else {
                share_data_->bRNNOISEVad_enhance_ = true;
                share_data_->RNNCounter_enhance_ = 20;
            }
        }

        aec_->process(*share_data_);

        //memcpy(aec_para_->data_out_f2, aec_para_->data_in_f, framelen_ * sizeof(float));


        // do suboise
        // to do: move sub before aec
        if (share_data_->bRNNOISEOn_ && !share_data_->bPreRnnOn_) {
            float tmp[480] = { 0 };
            float tmp_in = 0.f;
            float tmp_out = 0.f;
            float tmp_diff = 0.f;
            int channel1 = 1;
            float alpha = 0;
            for (int i = 0; i < framelen_; i++) {
                share_data_->ppProcessOut_[channel1][i] *= 32767;
                share_data_->pRNNBuffer_[i + framelen_] = share_data_->ppProcessOut_[channel1][i];
                tmp[i] = share_data_->pRNNBuffer_[i];
                tmp_in += tmp[i] * tmp[i];
            }
            rnnoise_process_frame(rnn_noise_, share_data_->ppProcessOut_[channel1], share_data_->ppProcessOut_[channel1]);

            for (int i = 0; i < framelen_; i++) {
                share_data_->pRNNPOWER_[i] = alpha * share_data_->pRNNPOWER_[i] + (1 - alpha) * share_data_->ppProcessOut_[channel1][i] * share_data_->ppProcessOut_[channel1][i];
                tmp_out += share_data_->pRNNPOWER_[i];

                share_data_->pRNNERROR_[i] = share_data_->pRNNBuffer_[i] - share_data_->ppProcessOut_[channel1][i];
                tmp_diff += share_data_->pRNNERROR_[i] * share_data_->pRNNERROR_[i];
                share_data_->pRNNBuffer_[i] = share_data_->pRNNBuffer_[i + framelen_]; // update one frame

                //share_data_->ppProcessOut_[channel1 - channel1][i] = share_data_->ppProcessOut_[channel1][i] / 32767;
                //share_data_->ppProcessOut_[channel1 - channel1][i] = share_data_->pRNNBuffer_[i] / 32767;
                //share_data_->ppProcessOut_[channel1][i] = tmp[i] / 32767;
                //share_data_->ppProcessOut_[channel1 - channel1][i] = share_data_->pRNNERROR_[i] / 32767;
            }


            //if ((tmp_out > 1*1e4 && tmp_diff < 1e7)|| (tmp_out > 1 * 1e5) ) 
            if (tmp_out > 1 * 1e4)
            {  // -50dB -53dB -57
                share_data_->bRNNOISEVad_ = true;
                share_data_->RNNCounter_ = 5;
            }
            else {
                share_data_->RNNCounter_--;
                share_data_->RNNCounter_ = share_data_->RNNCounter_ < 0 ? 0 : share_data_->RNNCounter_;
                if (share_data_->RNNCounter_ == 0) {
                    share_data_->bRNNOISEVad_ = false;
                }
            }

            if (tmp_out < 1 * 1e5) {
                share_data_->RNNCounter_enhance_--;
                share_data_->RNNCounter_enhance_ = share_data_->RNNCounter_enhance_ < 0 ? 0 : share_data_->RNNCounter_enhance_;
                if (share_data_->RNNCounter_enhance_ == 0) {
                    share_data_->bRNNOISEVad_enhance_ = false;
                }
            }
            else {
                share_data_->bRNNOISEVad_enhance_ = true;
                share_data_->RNNCounter_enhance_ = 20;
            }
        }

        // do agc for every output channel
        if (share_data_->bAGCOn_) {
            //for (size_t channel = 0; channel < mics_num; channel++) 

            {
                float gain = 1, power = 0;
                for (int i = 0; i < framelen_; i++) {
					power += share_data_->ppProcessOut_[channel][i] * share_data_->ppProcessOut_[channel][i];
                    //power += abs((float)data_out[i] / 32768);
                }
                power /= framelen_;
                //agc_process(pAgc, 1, &power, &gain, 0);
                //agc_new_process(agc_new, 1, &power, &gain, 0);
                agc_new_process(agc_new_, 1, &power, &gain, share_data_->IsResEcho_, share_data_->fNoisePwr_);
				float gain_smth;
                for (int i = 0; i < framelen_; i++) {
					gain_smth = (gain - share_data_->fAGCgain_) * float(i + 1) / float(framelen_) + share_data_->fAGCgain_;
                    share_data_->ppProcessOut_[channel][i] *= gain_smth;
                    //data_out[i] *= gain;
                }
                share_data_->fAGCgain_ = gain;
            }
        }

        // push data from sub to main thread

        std::vector<float> tmp_out(480, 0.f);
        memcpy(tmp_out.data(), share_data_->ppProcessOut_[0], sizeof(float) * 480);
        aec_output_buffer_->PushOneFrame(tmp_out);

		if (NULL != fp && NULL != fp1 && NULL != fp2 && NULL != fp3 && NULL != fp_ref && NULL != fp_out && (dump_idx < MAX_RECORD_TIMES)) {
			fwrite((char *)(&share_data_->ppCapture_[0][0]), sizeof(AUDIO_DATA_TYPE), framelen_, fp);
			fwrite((char *)(&share_data_->ppCapture_[1][0]), sizeof(AUDIO_DATA_TYPE), framelen_, fp1);
			fwrite((char *)(&share_data_->ppCapture_[2][0]), sizeof(AUDIO_DATA_TYPE), framelen_, fp2);
			fwrite((char *)(&share_data_->ppCapture_[3][0]), sizeof(AUDIO_DATA_TYPE), framelen_, fp3);
			fwrite((char *)(&share_data_->pReffer_[0]), sizeof(AUDIO_DATA_TYPE), framelen_, fp_ref);
			fwrite((char *)(&share_data_->ppProcessOut_[0][0]), sizeof(AUDIO_DATA_TYPE), framelen_, fp_out);
			dump_idx++;
			if (dump_idx >= MAX_RECORD_TIMES) {
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
			}
		}
        task_finished_ = true;
    }
    
    //}
}

void SUBinterface::sub_processing_deinit()
{

}



SUBBuffer::SUBBuffer(size_t max_size) : max_size_(max_size), is_started_(false) {
    SetFramePara(48000, 480, 1);
}

SUBBuffer::~SUBBuffer() {
    is_started_ = false;
    std::lock_guard<std::mutex> guard(buf_mutex_);
    ba_buffer_queue_.clear();
}

int SUBBuffer::PushOneFrame(std::vector<float> frame) {
    if (is_started_ && frame.data()) {
        std::lock_guard<std::mutex> guard(buf_mutex_);
        if (ba_buffer_queue_.size() >= max_size_) {
            ba_buffer_queue_.erase(ba_buffer_queue_.begin(),
                ba_buffer_queue_.begin() + (ba_buffer_queue_.size() - max_size_ + 1));
        }
        ba_buffer_queue_.push_back(frame);
        return 0;
    }
    return -1;
}

int SUBBuffer::PopOneFrame(std::vector<float> &frame) {
    if (is_started_ && frame.data()) {
        std::lock_guard<std::mutex> guard(buf_mutex_);

        if (ba_buffer_queue_.size() > 0) {
            memcpy(frame.data(), ba_buffer_queue_[0].data(),sizeof(float)*samples_per_channel_);
            //frame->copy_from(ba_buffer_queue_[0]);
            ba_buffer_queue_.erase(ba_buffer_queue_.begin());
        }
        else {
            return -1;
        }
        return 0;
    }
    return -1;
}

void SUBBuffer::SetFramePara(int samplerate, size_t samples_per_frame, size_t num_channel)
{
    samples_per_channel_ = samples_per_frame;
    sample_rate_hz_ = samplerate;
    num_channels_ = num_channel;
};

//#ifdef __cplusplus
//};
//#endif
