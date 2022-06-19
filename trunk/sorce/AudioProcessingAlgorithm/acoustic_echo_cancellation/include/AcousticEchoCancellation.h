/***********************************************************************
 *  Author
;*      Gao Hua
;*      
;*     
;*
;*  History
;*      10/15/2014 Created
;*
;*
;*************************************************************************/
#ifndef _ACOUSTICECHO_
#define _ACOUSTICECHO_
#include "echocancellation_interface.h"
#include "delayest.h"
#include "delaybuffer.h"
#include "T2Ftransformer.h"
#include "F2Ttransformer.h"
#include "SubbandAdap.h"
#include "postfilter.h"
#include "SPEst.h"
#include "VAD.h"
class CAcousticEchoCancellation: CEchoCancellationInterface
				  	              
{
public:
	CAcousticEchoCancellation(int Fs,float fftlen_ms,float framlen_ms);
	CAcousticEchoCancellation(int Fs,int fftlen_sample,int framlen_sample);
	~CAcousticEchoCancellation();
public:
	///AECInterface API

	int Init();
	virtual int process(audio_pro_share& aShareData);
	// set start frequency bins; m_nOffset is the bin number of start frequency;
	void SetOffset(float fre_start){};	
	void SetAEC_OnOff(bool OnOff){};
	void SetNR_OnOff(bool OnOff){};
	void Reset();
	bool SetDelay(int nDelay);
	int ProcessRefferData(audio_pro_share& aShareData);
  
private:
	inline int ResetAll();
	//move 1/2 length data of buffer
	inline void UpdateProBuffer(float *fp);
	//copy new data to processing buffer
	inline void ProBufferCopy(float *fp,float* fpnew);
private:

	SPEst* m_pSPest;
	AEC_VAD* m_pVADest;

	CDTDetector*  m_CDTD;
	CDelayBuffer* m_CDelayBuf;

	T2Ftransformer* m_CT2FMic;
	T2Ftransformer* m_CT2FRef;
	F2Ttransformer* m_CF2TErr;

#ifdef AUDIO_WAVE_DEBUG
	F2Ttransformer* m_CF2TErrBeforeNR;
#endif

	CSubbandAdap   *m_pSubBandAdap;
	CSubbandAdap   *m_pSubBandAdap2;
	int m_nadf2_filterbancunm = 0;
	CPostFilter    *m_pPostFil;
	int m_nFs;//samplerate
	int m_nFFTlen;// fft length in sample
	float m_fFFTlen_ms;// fft length in ms
	int m_nFramelen;// frame length processed in sample
	float m_fFramelen_ms;// frame length processed in ms
	float m_fValu0dB;// value for 0dBov
	bool m_bResetFlag;
	bool m_bInit;
	void* m_ppAuidoInBuf[2];
	void* m_ppAudioOutBuf[2];
	AUDIO_DATA_TYPE* m_pRefSp;
	int   m_nDelay; ///delay between mic and ref
	int   m_nSystemDelay;
	float*   m_pReferFFT;
	//////processing buffer
	float*    m_pMemAlocat;
	float**   m_pMemArray;
	audio_pro_share   m_AECData;
	audio_pro_share   m_AECData2;

	int m_nTail;
	bool m_bVad;
	float m_fCrossCor;
	float m_fGain;  //full band gain
};

class CAcousticEchoCancellationInFrequency : CEchoCancellationInterface

{
public:
	CAcousticEchoCancellationInFrequency(int Fs, int fftlen_sample, int framlen_sample);
	~CAcousticEchoCancellationInFrequency();
public:
	///AECInterface API

	int Init();
	virtual int process(audio_pro_share& aShareData);
	// set start frequency bins; m_nOffset is the bin number of start frequency;
	void SetOffset(float fre_start) {};
	void SetAEC_OnOff(bool OnOff) {};
	void SetNR_OnOff(bool OnOff) {};
	void Reset();
	bool SetDelay(int nDelay);
	int ProcessRefferData(audio_pro_share& aShareData);

private:
	inline int ResetAll();
	//move 1/2 length data of buffer
	inline void UpdateProBuffer(float *fp);
	//copy new data to processing buffer
	inline void ProBufferCopy(float *fp, float* fpnew);
public:

	SPEst* m_pSPest;
	CSubbandAdap   *m_pSubBandAdap;
	CPostFilter    *m_pPostFil;
	int m_nFs;//samplerate
	int m_nFFTlen;// fft length in sample
	float m_fFFTlen_ms;// fft length in ms
	int m_nFramelen;// frame length processed in sample
	float m_fFramelen_ms;// frame length processed in ms
	float m_fValu0dB;// value for 0dBov
	bool m_bResetFlag;
	bool m_bInit;
	void* m_ppAuidoInBuf[2];
	void* m_ppAudioOutBuf[2];
	//AUDIO_DATA_TYPE* m_pRefSp;
	//float*   m_pReferFFT;
	//////processing buffer
	float*    m_pMemAlocat;
	audio_pro_share   m_AECData;
	int m_nTail;
	//bool m_bVad;
	float m_fCrossCor;
	float m_fGain;  //full band gain

    int counter_ = 0;
    int first_render_ = 0;
    int first_render_his_ = 0;
};


#endif //_ACOUSTICECHO_