#ifndef _H_Noise_Est_
#define _H_Noise_Est_
/*
2011.08.10
*/
enum NoiseEstMode
{
	MS,  // 
	MRCA,// no implement currently
	SEnv// no implement currently
};
class CNoiseEst
{
	NoiseEstMode m_nMode;
	int m_nLen;
	int m_nBeginFrames;//in the first m_nBeginFrames, noise is not updated.//for bug 40298 
	float m_fFs;

	//Minimum Statistics noise estimator
	int m_nSubWinNum;
	int m_nSubWinLen;
	int m_nWinLen;
	int m_nSubWinCounter;
	float m_fMWin;
	float m_fMSubWin;
	float m_fAlpha_v;
	//float m_pfBeta;
	float *m_pfPwr;
	float *m_pfPwr2;
	//float *m_pfQeqInvGlobe;
	//float *m_pfQeqInvLocal;
	float m_fQeqmean;
	float m_fBc;
	float *m_pfBminGlobe;
	float *m_pfBminLocal;
	float *m_pfDelayLine;// arraylen x m_nSubWinNum
	int *m_pnMinIndx;// pointer to the minimum number in ith row
	int m_nDelIndx;// write pointer to column
	bool *m_pbKmode;
	bool *m_pbMinFlag;
	float *m_pfGlobeMin;
	float *m_pfLocalMin;
	float *m_pfPwrMin;
	float *m_pfNoise;
	float* m_pfTrans;//transient noise;
    //use as static
	float Bmin;
	float Bminsub;

    
public:
	// fs is the sampling-rate of frame; arraylen is the length of input array.
	CNoiseEst(float fs,int arraylen,NoiseEstMode mode=MS);
	~CNoiseEst(void);
	float MD_interp(int N);
	void SetWin(int win,int subwin);
    void InitNosieEst();
	void UpdatePara(float *alpha,float *InPwr);
	void SetDelayline(int bin,float valu);
	void UpdateDelayLine(float* valu);

	void NoiseTrack();
	void Process(float *InPwr,float *alpha);
	float* GetNoise(){return m_pfNoise;};
	float* GetTrans(){return m_pfTrans;};
	void SetInitFrame(float *InPwr);



};
#endif


