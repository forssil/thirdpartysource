#ifndef _H_SUBBAND_
#define _H_SUBBAND_
#include "audiotypedef.h"
// #ifndef Filterbank
// #include "Adaptivfilter.h"
// #else
#include "AdapFilterGroup.h"
// #endif
static float powf_to_n(float x, int n);
 
class CSubbandAdap
{
public:

	//float m_fAdaptMaxTap_subband;
	void process(float * Refer ,float *Des,float *OutErr,float *OutEst,int offset,audio_pro_share aecdata);
	CSubbandAdap(int Fs, float fft_ms);
	CSubbandAdap(int Fs, int fft_samples);
	~CSubbandAdap(void);
	void Subband_init(int fiter_len = 0, int fileter_num = 0,  float mu = 0.5f, float delat_gain = 1.f );
private:
	void Decay_init();
	
public:
	int m_nFs;
	int m_nFFTsize;
	float m_fFFTlen_ms;
	//float m_fFrame_ms;
	float m_fDeltF;
	//about decay
	
    float *m_fpAttackTapsInit;

     ///subband filter
	int m_nNumSubbandGroups;
	int *m_nSubband_groups;
    int *m_npFilterLen;
	int m_nMaxFilLen;
	int m_nSubused;//48kHz 320,16kHz 116; the number of used bins in subband adaptive filter


	int m_nSubbandGroupk;
	int *m_npCopyd;  //m_nNumSubbandGroups

	float* m_fpFxbuf;
	float* m_fpAdbuf;

    CAdapFilterGroup* m_CpAdapG;
	int m_nFarVAD;



};
#endif