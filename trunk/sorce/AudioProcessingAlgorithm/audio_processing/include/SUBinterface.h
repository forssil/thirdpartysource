#ifndef _INC_AUDIOSUBINTERFACECPP
#define _INC_AUDIOSUBINTERFACECPP
#define	MAX_RECORD_TIMES	20000

#include    <thread>
#include    <mutex>
#include    "AudioAecCpp.h"
#include    "audiotypedef.h"
#include    <vector>
#include    <atomic>

#include    "AudioProcessingFramework_interface.h"
#include    "agc_new.h"
#include    "rnnoise.h"

using namespace std;

class SUBBuffer
{
public:
	SUBBuffer(size_t max_size);
	~SUBBuffer();

public:
	int PushOneFrame(std::vector<float> frame);
	int PopOneFrame(std::vector<float> &frame);
	int GetRefQueSize()
	{
		if (is_started_)
		{
			std::lock_guard<std::mutex> guard(buf_mutex_);
			//size_t size = buffer_queue_.size();
			size_t size = ba_buffer_queue_.size();
			return size;
		}
		return -1;
	};
	int StartBuffer()
	{
		if (is_started_)
			return 0;
		is_started_ = true;
		ClearBuffer();
		return 0;
	};
	int StopBuffer()
	{
		if (is_started_)
			is_started_ = false;
		return 0;
	};
	int ClearBuffer()
	{
		if (is_started_)
		{
			std::lock_guard<std::mutex> guard(buf_mutex_);
			ba_buffer_queue_.clear();
		}
		return 0;
	};
	void SetFramePara(int samplerate, size_t samples_per_frame, size_t num_channel);
	// {
	//     samples_per_channel_ = samples_per_frame;
	//     sample_rate_hz_ = samplerate;
	//     num_channels_ = num_channel;
	// };

private:
	size_t max_size_;
	std::atomic<bool> is_started_;
	std::vector<std::vector<float>> ba_buffer_queue_;
	//std::vector<float *> ba_buffer_queue_;
	std::mutex buf_mutex_;
	size_t samples_per_channel_;
	int sample_rate_hz_;
	size_t num_channels_;

};

class SUBinterface {
public:
	int counter = 0;
	SUBinterface();
	~SUBinterface();
    void threadrun();
	void sub_create(audio_pro_share *share_data);
	void sub_processing_deinit();
	void sub_process(audio_pro_share * sharedata, AEC_parameter aec_para);

	void start_sub_thread();
	void stop_sub_thread();
	void task();
	bool get_finish_flag() { return task_finished_; };
	std::thread* sub_thread_;
	mutable std::mutex sub_thread_mutex_;
	std::atomic_bool audio_3A_thread_running_;
private:

	std::unique_ptr<SUBBuffer> aec_mic0_buffer_;
	std::unique_ptr<SUBBuffer> aec_mic1_buffer_;
	std::unique_ptr<SUBBuffer> aec_mic2_buffer_;
	std::unique_ptr<SUBBuffer> aec_mic3_buffer_;
	std::unique_ptr<SUBBuffer> aec_ref_buffer_;
	std::unique_ptr<SUBBuffer> aec_output_buffer_;

	CAudioProcessingFrameworkInterface* aec_ = nullptr;
	AGCSTATE_NEW *agc_new_ = nullptr;
	DenoiseState* rnn_noise_ = nullptr;
	audio_pro_share * share_data_ = nullptr;
	//AEC_parameter *aec_para_ = nullptr;

	//
	int mics_num_ = 4;
	int framelen_ = 480;
	float *data_in_f, *data_out_f;
	float *data_in_f2, *data_out_f2, *data_out_f3, *data_out_f4, *data_out_f5, *data_out_f6, *data_out_f7;

	bool task_finished_;
	FILE   *fp = NULL;
	FILE   *fp1 = NULL;
	FILE   *fp2 = NULL;
	FILE   *fp3 = NULL;
	FILE   *fp_ref = NULL;
	FILE   *fp_out = NULL;
	int    dump_idx;
};


#endif
