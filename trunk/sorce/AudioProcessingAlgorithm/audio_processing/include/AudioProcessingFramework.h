/***********************************************************************
 *  Author
;*      Gao Hua
;*      
;*     
;*
;*  History
;*      1/15/2022 Created
;*
;*
;*************************************************************************/
#ifndef _AUDOPROCESSINGFRAMEWORK_
#define _AUDOPROCESSINGFRAMEWORK_
#include "AudioProcessingFramework_interface.h"
#include "delayest.h"
#include "delaybuffer.h"
#include "T2Ftransformer.h"
#include "F2Ttransformer.h"
#include "SubbandAdap.h"
#include "postfilter.h"
#include "SPEst.h"
#include "VAD.h"
#include "AcousticEchoCancellation.h"
#include "MVDR.h"
class CAudioProcessingFramework : public CAudioProcessingFrameworkInterface
				  	              
{
public:
	CAudioProcessingFramework(int mic_nums, int Fs,int fftlen_sample,int framlen_sample);
	~CAudioProcessingFramework();
public:
	///AECInterface API

	int Init();
	virtual int process(audio_pro_share& aShareData);
	// set start frequency bins; m_nOffset is the bin number of start frequency;
	void SetOffset(float fre_start){};	
	void SetAEC_OnOff(bool OnOff){};
	void SetNR_OnOff(bool OnOff){};
	void SetMainMicIndex(int micindx) { 
		m_nMain_mic_index = micindx >= 0 ? micindx : 0;
	};
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

	T2Ftransformer** m_ppCT2FMics;
	T2Ftransformer* m_CT2FRef;
    T2Ftransformer* m_CT2FRNNERROR;
	F2Ttransformer* m_CF2TErr;

#ifdef AUDIO_WAVE_DEBUG
	F2Ttransformer* m_CF2TErrBeforeNR;
	F2Ttransformer** m_ppCF2TMics;
#endif
	CAcousticEchoCancellationInFrequency** m_ppCAECMics;
	audio_pro_share* m_pAECDataArray;
	int m_nadf2_filterbancunm = 0;

	int m_nMicsNum;
	int m_nMain_mic_index;
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
	AUDIO_DATA_TYPE* m_pRefSp_nonsmooth;
	int   m_nDelay; ///delay between mic and ref
	int   m_nSystemDelay;
	float*   m_pReferFFT;
    float*   m_pRNNERRORFFT;
	//////processing buffer
	float*    m_pMemAlocat;
	audio_pro_share   m_APFData;
	audio_pro_share   m_APFData2;

	int m_nTail;
	bool m_bVad;
	float m_fCrossCor;
	float m_fGain;  //full band gain
	CMVDR* m_CBF;
};


#endif //_AUDOPROCESSINGFRAMEWORK_