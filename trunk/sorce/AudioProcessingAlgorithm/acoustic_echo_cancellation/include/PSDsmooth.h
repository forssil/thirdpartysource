#ifndef _H_PSDsmooth_
#define _H_PSDsmooth_
/*
2011.08.08
This class calculates a nearly constant-Q power spectral density and smooths this psd in time domain. 
Bins below 1000Hz have the same as periodogram;
Bins over 1000Hz equals convolution of  periodogram and a window.
*/
enum TDMode
{
	CONSTANTPARA,//constant parameters recursive averaging.
	OPTIMALSMOOTH//optimal smoothing and mini mum statistics
	
};

class CPSDsmooth
{
public:
	int m_nHalfFFTLen;
	int m_nHalfFs;
	//int m_nWinsize;
	int m_nF2n;// theoretically it is the closest frequency  to Fs with the number of 2^n;  it is equal 8000Hz for simple
	int m_nLowBand; //bins below 1000Hz in transformed domain
	int m_nHiBand;  //bins over m_nF2n in transformed domain
	int m_nCQBand;  // bins between 1kHz and m_nF2n in transformed domain
	int m_nAllBand;
	int m_pnStep[5];//steps in every band;
	float m_fInvDeltf;

	//float *m_pfWinInv2;
	int m_pnWinLen[3];//  length(m_pfWinInv2)=2*m_nWinInv2len+1
	//float *m_pfWinInv4;
	//int m_nWinInv4len;
	//float *m_pfWinInv8;
	//int m_nWinInv8len;
	float *m_ppfWin[3];
	float* m_pfInBuffer;// size inbuffer is fftlen+max m_pnWinLen
	float* m_pfPsdCQ_Fd;
	float* m_pfPsdCQ_Td;//contain smoothed psd on preframe 
    
	//TD
	float *m_pfAlpha;
	float m_fAlpha_c;
	float m_fAlpha_max;
	TDMode m_nTDMode;//0 for 

	int m_nCQBandNum;

public:
	CPSDsmooth(int fftlen,int fs);
	~CPSDsmooth(void);
	//InPwr  format:[re(0)^2+ im(0)^2, re(1)^2+im(1)^2, ... re(FFTSIZE/2-1)^2+ im(FFTSIZE/2-1)^2] ,size FFTsize/2
	//noisedpsd , 1x m_nAllband;
	//outbuffer, 1x m_nAllband;
	void processing(float* InPwr,float *noisepsd);
	int GetTranfLen(){return m_nAllBand;};
	float* GetAlpha(){return m_pfAlpha;}
	void SetAlpha(float alpha);
	float* GetCQPsd(){return m_pfPsdCQ_Td;};
	float* GetCQPsdFd(){return m_pfPsdCQ_Fd;};
	void CQSpread(float* inbuffer,float *outbuffer);//spead inbuffer from m_nAllband to FFTLen;
	void SetInitFrame(float *input);

private:
    void UpdateInBuf(float *InPwr);
	void InitPSDsmooth(TDMode mode);
    int InitWindow(float *&Win,float ratio);
	////inPwr format:[re(0)^2+ im(0)^2, re(1)^2+im(1)^2, ... re(FFTSIZE/2-1)^2+ im(FFTSIZE/2-1)^2] 
	void FDsmooth_process();
	void UpdateTDpara(float *noisepsd);
	void UpdateTDpara_OSMS(float *noisepsd);
	void TDsmooth_process();
	

	
};
#endif