#ifndef _H_post_filter_
#define _H_post_filter_
#include "audiotypedef.h"
#include "noisereduction.h"

#define winlen 1 // win length is 2*winlen +1
class CPostFilter
{
public:

	CPostFilter(int fs,int fftlen);
	~CPostFilter(void);
	void Process(audio_pro_share *Aec);
	float* GetGain(){return m_pfGaintemp;};
	//
	void UpdateAECinfo(float *proriSNR,float* noisepwr,float* speechp,float *noisebef);
	/*return est noise*/
	float* GetNoiseEst(){return m_CNoisRedu->GetNoisefft();};
	//
	void UpdateAllBandGain();
	float GetAllBandGain();
private:
	void UpdatePwr(audio_pro_share *Aec,float speed);
	void UpdateReferPwr(audio_pro_share *Aec);
	float TrackEnvelop(float newData, float envelop, float updateStep);
	void UpdateGain();
	void ResetGain(float g);
	void SelectGain();
	/*set min gain. min gain will be updated according to noise power */
	void SetGainMin(float noisepwr);
	void Reset();
	float Window_Depreenhance(int n,float delt);	
	float AllbandPwr(float *specpwr,float *gain);	
	void Spe_Limiter(audio_pro_share *aeinfo);
	void AddCNG(audio_pro_share *aeinfo);

public:
	int m_nFs;
	int m_nFFTLen;
	int m_nHalfFFTLen;
	int m_nOffset;
	float m_fMinGain;

	float *m_pfPwr;
	float *m_pfReferPwr;
	float *m_pfEstPwr;
	float *m_pfAft;
	float *m_pfBef;
	float *m_pfEst;
	float *m_pfGain;
	float *m_pfGaintemp;
	float *m_pfWin;
	//20120306 limit in spectrum
	float *m_pfWinWeight;
	float *m_pfLimitGain;
	float m_fAfRatio;// ratio between after and before adaptive filter
	float m_fBefadfPwr;
	float m_fAftadfPwr;
	float m_fFarEndPwr;
	float *m_pfHighFreCtr;
	int   m_nInd65k;
	int   m_nInd80k;
	//
	int m_nBufferSize;
	//CNLP *m_CNLP;
	CNoiseRedu *m_CNoisRedu;
	//CSpecCtrl *m_CSpecCtrl;

	float m_fNR;//noise level after NR for extra nr; 
	float m_fNR_bef;// noise power befor NR
	float m_fNoisePwr;
	float m_fExtraG;
	float m_fSp;
	float m_fShellgain;
	/////low pass
	float *m_pfLowpassWin;
	///CNG
	float *m_pfCNGPwr;
	float *m_pfCNGFFT;
	float* m_pfCNGFFTtemp;
	int16_t *m_pnRandW16;
	uint32_t m_nSeed;
	//
	float m_fLowLevelThreashold;

	float m_fReferPwrEnvelop;
	float m_fReferPwrEnvelopUpdateStep;
	float m_fReferPwrEnvelopDb;
	float m_fAllBandGain;
};



#endif