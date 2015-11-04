#ifndef _H_SPEECH_EST_
#define _H_SPEECH_EST_
/*
2011.08.11
The algorithm is based OMLSA: Optimally-modified log-spectral amplitude.

*/

class CSpeechEst
{
public:
	int m_nLen;
	float m_fFs;
	int m_nFFTLen;
	//OMLSA
	float *m_pfPrioriSNR;
	float m_fAlpha_prio;
	float m_fBeta_prio;
	float m_pfPriori_bin_threshold_max[2];
	float m_pfPriori_bin_threshold_min[2];
	float m_pfPriori_bin_threshold_dif[2];//priori_bin_threshold_max-priori_bin_threshold_min;
	int   m_pnBands[3];
	float m_fPriori_band_threshold_max;
	float m_fPriori_band_threshold_min;
	float m_fPriori_band_threshold_dif;//=priori_band_threshold_max-priori_band_threshold_min;
	float m_fPriori_frame_threshold_max;
	float m_fPriori_frame_threshold_min;
	float m_fPriori_frame_threshold_dif;//=priori_frame_threshold_max-priori_frame_threshold_min;
	float m_fPriori_frame;	//%%frame priori snr
	float *m_pfPostSNR;
	
	float *m_pfP;// probability of speech presence 
	float *m_pfQ;// probability of speech absence
	float m_fMenP;
	float m_fAlpha_q;
	int m_nStartBin;
	int m_nEndBin;
	float m_fBins;

	float *m_pfGain;
	float m_fGainMin;
	float *m_pfV;
	//20110926
	float m_fAllbandProb;


public:
	CSpeechEst(float fs,int arraylen, int fftlen);
	~CSpeechEst(void);
	void SetGmin(float f){m_fGainMin=f;};
	void SetBinUsed(int startbin,int endbin){m_nStartBin=startbin;m_nEndBin=endbin;};
	void InitSpeechEst();
	void UpdateSNR(float *InPwr, float* Noise, float* Trans);
	void UpdateSNR(float *InPwr, float* Noise);
	float Update_allbandProb(float InPwr,float Noise);
	void UpdateProb();
	void UpdateGain();
	void Porcess(float *InPwer, float* Noise, float *Trans);
	void Porcess(float *InPwer, float* Noise);
	float* GetGain(){return m_pfGain;};

	float GetProiSNR(){return m_fPriori_frame;};
	float GetSpeechProb();

};
#endif