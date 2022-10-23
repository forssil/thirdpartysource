#ifndef _H_AdapFilterGroup_
#define _H_AdapFilterGroup_

/*
CAdapFilterGroup is a class of a bank of adaptive filters, is instead of CAdapFilter array in CSubbandAdap.
*/
class CAdapFilterGroup
{
	
public:
	CAdapFilterGroup(int numbank,int *ntaps,float mu, float delat_gain);
	~CAdapFilterGroup(void);
	//void UpdateStep(float fCorr);
	void AdapfilterIni();
	void UpdateR11_R12(const float *newRefer);//*newRefer size 2*m_nNumBank
	void UpdateReferEnergy();
	void UpdateDelayline(const float *newRefer);
	void UpdateDelta(float* fp, float fCorr);
	void SetMinMaxDelta(float min, float max);
	void UpdateCorr();
	void UpdateError();
	void filter(void);
	void UpdateFilterWeight(void);
	void UpdateDecay(int indxbin,int attack_length, float *attackTaps,float subbandDecay,float attackGain);
	void Resetfilter(int startbin,int endbin);
	//void Adap2Fix(int startbin,int endbin);
	//void Fix2Adap(int startbin,int endbin);
	//float GetMaxAdW(void);
	float* GetMaxAdWVec(){return m_fpMaxAdp;};
	//void process(const float *Refer,const float *Des);
	//void reset_process(int startbin ,int endbin ,short flag);
	void ResetDelay_Taps(int indx);
	//void MoveTapsForward(int indx,int abs_delay);
	//void MoveTapsBackward(int indx,int abs_delay);
	//void UpdateDelaylineInvers(const float *newRefer);
	//void UpdateR11_R12Invers(const float *newRefer);
	void SumR11_R12();
	void process(const float *Refer,const float *Des,float *est,float* err,int update_flag);
	float* GetFixTaps(int i){return m_cpFixW+m_npDelaylIndx[i];};
	//void UpdateFilterWeight_band(int startbin,int endbin);
	void SetDeltaGain(float gain){m_fDeltaTC=gain;};
private:
	int   m_nNumBank;// the number of filters in bank
	int   m_nSumTapsBank;
	int   *m_npTaps;//must 1*m_nNumBank ,array of filter length, pointer to outside memory
	//float *m_fpSpeBuf;
	float *m_cpReferDelayLine;//sum(2*filtertaps[m]+2)   {X(n),X(n-1)}, X(n)={x(n),x(n-1),...,x(n-taps+1)}T;
    int   m_nSumLenDelLine;
	int   *m_npDelaylIndx;//index of current write position in m_fpReferDelayLine[m]
	float *m_fpR11;//sum(filtertaps[m]+1), it contains r11+r22
	int   m_nSumLenR11;
	int   *m_npR11Indx;//index of current write position in m_fpR11[m]
	float *m_fpR11sum;//m_nNumBank
	float *m_fpR22sum;//m_nNumBank
	float *m_cpR12;//sum(2*filtertaps[m])
	int   m_nSumLenR12;
	int   *m_npR12Indx;//m_nNumBank
	float *m_cpR12sum;//2*m_nNumBank
	float *m_cpAdW;//sum(2*filtertaps[m]+2) as same as length of m_fpReferDelayLine
	float m_fMaxAdpAll;
	float *m_fpMaxAdp;//m_nNumBank
	float *m_cpFixW;//sum(2*filtertaps[m]+2) as same as length of m_fpReferDelayLine
	float *m_fpAttackTaps;//sum(filtertaps[m]+1) as same as half length of m_fpReferDelayLine
	float *m_fpAdEstPwr;//m_nNumBank

	float *m_fpDelta;//m_nNumBank
	float m_fDeltaTC;//constant 
	float m_fMinDelta; //const float m_fMinDelta = 1e-5f;
	float m_fMaxDelta; //const float m_fMaxDelta = 5e-4f
	float *m_cpCorr;//2*m_nNumBank
	const float *m_cpDes;//2*m_nNumBank
	float *m_cpAdErr;//2*m_nNumBank, 
	float *m_cpAdErrPre;//2*m_nNumBank
	float *m_cpAdEst;//2*m_nNumBank, 
	float *m_cpFixErr;//2*m_nNumBank,
	float *m_cpFixEst;//2*m_nNumBank,

	float *m_fpDen;//m_nNumBank
	float m_fMu = 0.f;
	float *m_cpBeta1;//2*m_nNumBank
	float *m_cpFixBeta;//2*m_nNumBank
	float *m_cpBeta2;//2*m_nNumBank
	float *m_cpBeta12;//2*m_nNumBank

	int* m_npAttack_length;
	int m_nFlagcnt;

	//float m_fSubbandDecay;
	//float m_fAttackGain;
	//int m_nMaxDelyFra;
	/*
	int m_nMaxBin;
		int m_nMinBin;
		float m_fMaxFre;
		float m_fMinFre;*/
	////add variable delta weights 20160626
	float* m_pDeltaFreWeight;
	float m_deltagin = 1.f;
#if defined(ADF_DEBUG)
    FILE* ADFW = NULL;
#endif
    int32_t counter_ = 0;
    bool neon_on_ = false;
	
};

#endif