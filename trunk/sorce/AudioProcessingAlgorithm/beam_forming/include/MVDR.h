#ifndef MVDR_H_
#define MVDR_H_
#endif

#include <complex>
#include "audiotypedef.h"
#include "processingconfig.h"

class CMVDR {
public:
	CMVDR(int fft_len, int fs, int bins, int channels, float interval, float DOA);
	~CMVDR();
	void init_with_config(int channels, float interval, float DOA);
	void update_noise_matrix(audio_pro_share* sharedata);
	float* get_weight();
	void process(bool VADflag, audio_pro_share* input, audio_pro_share* output);
	void matrix_multiply(float* A, float* B, float *C, int length);
private:
	int m_ncounter;
	int m_nChannels;
	float m_fInterval;
	float* m_pfH;
	float m_fDOA;
	float* m_pfSteerVector;
	float* m_pfNoiseMtx;
	float* m_pfNoiseMtxInv;
	float m_fc;
	int m_nbins;
	int m_nFftLen;
	int m_nFs;
	float *m_pfnum; // for matrix calc
	float* m_pfnum2; 
	float m_fLamda = 1.0;
	// calculate inverse of noise correlation matrix
	float* m_pfx;
	//float noiseInv[50] = {
 //   10130, -38, 130, -38, 130, -38, 130, -38, 130, -38,
 //   130, -38, 10130, -37, 130, -37, 130, -37, 130, -38,
 //   130, -38, 130, -37, 10130, -37, 130, -37, 130, -38,
 //   130, -38, 130, -37, 130, -37, 10130, -37, 130, -38,
 //   130, -38, 130, -38, 130, -38, 130, -38, 10130, -38
	//};
};

class CAdaptiveBeamForming {
public:
	CAdaptiveBeamForming(int fft_len, int fs, int bins, int channels);
	~CAdaptiveBeamForming();
	void init();
	void process(audio_pro_share* input,int input_len, audio_pro_share& output, int main_channel);

private:
	int m_ncounter;
	int m_nChannels;
	int m_nMainChannel;

	float m_fc;
	int m_nbins;
	int m_nFftLen;
	int m_nFs;
	float m_fLamda = .3f;

	float **m_fppAutoCorr;//[channel-1][fftlen/2]
	float **m_fppCrossCorr;//[channel-1][fftlen]
	float **m_fppTransFilter;//[channel-1][fftlen]
	float *m_fpOutPut;//[channel-1][fftlen]
	float m_fActiveThreashold = 0.000001f;
};
