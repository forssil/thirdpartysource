/**************************************************
*         Copyright 2014 Coddy Inc.
*         Author:  Gu Cheng
*         History: 10/15/2014
*         Content: Voice Activity Detector
***************************************************/

#ifndef AECHALFDUPLEX_AEC_VAD_H_
#define AECHALFDUPLEX_AEC_VAD_H_

//window for smooth
//class SilenceDetector;

class AEC_VAD
{
public:
	AEC_VAD(void);
	~AEC_VAD(void);

	/***************************************************
	name:    CreateVAD_int
	para:    fs       (IN)
	         fftlen   (IN)
			 framelen (IN)
	content: initial parameter
	***************************************************/
	void CreateVAD_int(int fs, int fftlen, int framelen);

	/***************************************************
	name:    GetVAD
	para:    segSpecPow  (IN)
	         vad_est     (OUT)
			 vad_full    (OUT)
	content: obtain VAD
	***************************************************/
	void GetVAD(const float* smooth, const float* nonsmooth, float* vad_est, float& vad_full);

	/***************************************************
	name:    GetFlag
	content: get flag
	***************************************************/
	float  GetFlag();

private:
	const float _noise_offset;    //noise offset
	const int m_frame_buffer_count;
	const float* m_segSpecPow;  //input energy power


	/***************************************************
	name:    NoiseEst
	content: noise estimation
	***************************************************/
	void NoiseEst();

	/***************************************************
	name:    VADEst
	content: obtain VADEst
	***************************************************/
	void VADEst();

	/***************************************************
	name:    VADFull
	content: obtain VADFull
	***************************************************/
	void VADFull();

	/***************************************************
	name:    SubbandPwr
	content: obtain sub-band power
	***************************************************/
	void SubbandPwr();

	/***************************************************
	name:    GetNoisePower
	content: get noise power
	***************************************************/
	float GetNoisePower();




	//initial parameter in CreateVAD_int()
	int m_fs;             //sampling rate
	int m_fftlen;         //fft len
	int m_framelen;       //frame len

	//sub-band define
	float m_band_threshold[3][2]; //sub-band threshold
	float m_band_weight[3];       //sub-band weight
	float m_band_freq[4];         //sub-band frequency
	int* m_freq_index;            //index of sub-band frequency
	int m_band_num;               //num of sub-band 

	//VAD value
	float* m_vad_est;     //result of VADEst()
	float m_vad_full;    //result of VADFull()

	int m_i_whole;       //counter of whole time
	int m_i_sub;         //counter of sub time
	int m_sub_time;      //sub time
	int m_whole_time;    //whole time
	int m_bias;          //biases of m_band_power
	float* m_band_power; //sub-band power
	float* m_noisepower; //noise power
	float* m_noise_est;  //noise memory
	float* m_noise_min;  //min noise

	//vad_full parameter
	float alpha_s;
	float alpha_nt;
	float alpha_n;
	float threshold1;
	float threshold2;

	//store the data of pre frame  
	float m_speech_env_pre;
	float m_noise_temp_env_pre;
	float m_noise_env_pre;
	float m_vad_full_pre;
	bool  m_flag;	     //when there is data ,m_flag = false


	float*  m_fFrameBuffer;
	int     m_nFrameBufferCount;
	//SilenceDetector* m_silencedetector;
	bool    m_flagofCurrentSilence;

	float m_fTempout;
};
#endif

