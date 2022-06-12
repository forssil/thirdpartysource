#ifndef _H_noise_reduction_
#define _H_noise_reduction_
#include "noiseest.h"
#include "SpeechEst.h"
#include "PSDsmooth.h"
#include "audiotypedef.h"
class CNoiseRedu
{
public:
	//fs, sampling-rate
	//fftlen
	CNoiseRedu(int fs, int fftlen);
	~CNoiseRedu(void);
	void Process(float *input,float *echonoise,audio_pro_share aecdata,float *pfAft,float *pfBef);
	float *GetGain(){return m_pfGainout;};
//	void SetGmin(float f){m_CSpeech->SetGmin(f);};
	void SetGmin(float f){ m_CSpeechStatic->SetGmin(f); };

	/*return noise power in dB after noise reduction*/
	float NoisePwr_allband(float* Noisebef);
	float Window(int n,float delt);
	void ReUpdateGain(float *G);
	float* GetNoisefft(){return m_pfNoiseLine;};
	void transientnois();
	void FullBandCtrl(float *pfAft,float *pfBef,float* gain);

public:
// 	int m_nFs;
	int m_nFFTLen;
	float m_fMinGain;
	int m_nQNum;
	CNoiseEst *m_CNois;
//	CSpeechEst *m_CSpeech;
	CSpeechEst *m_CSpeechStatic;
	CSpeechEst *m_CSpeechTransient;
	CPSDsmooth *m_CPsd;
	CPSDsmooth *m_CPsd_echo;
	float *m_pfNoise;
	float *m_pfTrans;
	float *m_pfTransGain;
	float *m_pfAlpha;
	float *m_pfGaintemp;
	float *m_pfPwr; /*attention: m_pfPwr[0] is all band noise!!!!!!*/
	float *m_pfPwrFd;
	float *m_pfGainout;
	float *m_pfNoiseLine;/*attention: m_pfNoiseLine[0] is all band noise!!!!!!*/
	float *m_pfWinWeight;
    float *m_pfnrin_smooth;
	audio_pro_share m_sAecdata;


};
#endif