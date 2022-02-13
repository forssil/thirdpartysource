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
#include <math.h>
#include <memory>
#include "AudioProcessingFramework.h"

extern "C"
{
	CAudioProcessingFrameworkInterface* CreateIApfInst_int(int mic_nums, int Fs, int fftlen, int framlen)
	{

		CAudioProcessingFramework *inst = new CAudioProcessingFramework(mic_nums, Fs, fftlen, framlen);
		if (inst)
			return (CAudioProcessingFrameworkInterface*)inst;
		else
			return NULL;

	};
	int DeleteIAPFInst(CAudioProcessingFrameworkInterface*  IAecInst)
	{
		if (IAecInst)
		{
			delete (CAudioProcessingFramework*)IAecInst;
			return 0;
		}
		else
			return -1;

	};
};


////class CAudioProcessingFramework
CAudioProcessingFramework::CAudioProcessingFramework(int mic_nums, int Fs, int fftlen_sample, int framlen_sample) :
	m_nFFTlen(fftlen_sample)
	, m_nFs(Fs)
	, m_nFramelen(framlen_sample)
	, m_pReferFFT(NULL)
	, m_bResetFlag(false)
	, m_CDTD(NULL)
	, m_CDelayBuf(NULL)
	, m_bInit(false)
	, m_ppCT2FMics(NULL)
	, m_CT2FRef(NULL)
	, m_nDelay(0)
	, m_CF2TErr(NULL)
	, m_pMemAlocat(NULL)
	, m_nTail(0)
	, m_bVad(false)
	, m_fCrossCor(0)
	, m_fGain(1.f)
	, m_nSystemDelay(0)
	, m_pSPest(NULL)
	, m_pVADest(NULL)
	, m_pRefSp(NULL)
#ifdef AUDIO_WAVE_DEBUG
	, m_CF2TErrBeforeNR(NULL)
#endif
	, m_ppCAECMics(NULL)
	, m_pAECDataArray(NULL)
{
	m_fFFTlen_ms = float(m_nFFTlen * 1000) / m_nFs;
	m_fFramelen_ms = float(m_nFramelen * 1000) / m_nFs;

	m_nMicsNum = mic_nums > 1 ? mic_nums : 1;
	memset(m_ppAuidoInBuf, 0, 2 * sizeof(void*));
	memset(m_ppAudioOutBuf, 0, 2 * sizeof(void*));
};

CAudioProcessingFramework::~CAudioProcessingFramework()
{


	if (NULL != m_CDTD)
	{
		delete m_CDTD;
		m_CDTD = NULL;
	}

	if (NULL != m_CDelayBuf)
	{
		delete m_CDelayBuf;
		m_CDelayBuf = NULL;
	}

	if (NULL != m_ppCT2FMics)
	{
		for (int i = 0; i < m_nMicsNum; i++) {
			if (NULL != m_ppCT2FMics[i]) {
				delete m_ppCT2FMics[i];
				m_ppCT2FMics[i] = NULL;
			}
		}
		delete[] m_ppCT2FMics;
		m_ppCT2FMics = NULL;
	}
	if (NULL != m_CT2FRef)
	{
		delete m_CT2FRef;
		m_CT2FRef = NULL;
	}
	if (NULL != m_CF2TErr)
	{
		delete m_CF2TErr;
		m_CF2TErr = NULL;
	}

#ifdef AUDIO_WAVE_DEBUG
	if (NULL != m_CF2TErrBeforeNR)
	{
		delete m_CF2TErrBeforeNR;
		m_CF2TErrBeforeNR = NULL;
	}
#endif

	if (NULL != m_pMemAlocat)
	{
		delete[] m_pMemAlocat;
		m_pMemAlocat = NULL;
	}

	if (NULL != m_pSPest)
	{
		delete m_pSPest;
		m_pSPest = NULL;
	}
	if (NULL != m_pVADest)
	{
		delete m_pVADest;
		m_pVADest = NULL;
	}
	if (NULL != m_ppCAECMics)
	{
		for (int i = 0; i < m_nMicsNum; i++) {
			if (NULL != m_ppCAECMics[i]) {
				delete m_ppCAECMics[i];
				m_ppCAECMics[i] = NULL;
			}
		}
		delete[] m_ppCAECMics;
		m_ppCAECMics = NULL;
	}
	if (NULL != m_pAECDataArray) {
		delete[] m_pAECDataArray;
		m_pAECDataArray = NULL;
	}
}

void CAudioProcessingFramework::Reset()
{
	m_bResetFlag = true;
}

//thread safe 
int CAudioProcessingFramework::ResetAll()
{
	int ret = 0;
	if (m_bResetFlag)
	{
		///reset module

		if (ret == 0)
		{
			m_bResetFlag = false;
			return ret;
		}
	}
	else
		return ret;

}

int CAudioProcessingFramework::Init()
{
	m_nMain_mic_index = 0;
	////
	memset(&m_APFData, 0, sizeof(audio_pro_share));

	int total_size = (m_nMicsNum+6 ) * m_nFFTlen;
	m_pMemAlocat = new float[total_size];
	memset(m_pMemAlocat, 0, total_size * sizeof(float));

	//
	m_APFData.ppCapureFFT_ = new float*[m_nMicsNum];
	m_APFData.nChannelsInCaptureFFT_ = m_nMicsNum;
	m_APFData.nLengthFFT_ = m_nFFTlen;
	for (int i = 0; i < m_nFFTlen; i++) {
		m_APFData.ppCapureFFT_[i] = m_pMemAlocat + i * m_nFFTlen;
	}
	m_pReferFFT = m_APFData.ppCapureFFT_[m_nMicsNum-1] + m_nFFTlen;
	m_APFData.pEstimationFFT_=m_pReferFFT+m_nFFTlen;
	m_APFData.pErrorFFT_= m_APFData.pEstimationFFT_+m_nFFTlen;
	m_APFData.pError_= m_APFData.pErrorFFT_+m_nFFTlen;
	m_APFData.pErrorSpectrumPower_= m_APFData.pError_+m_nFFTlen;
	m_pRefSp= m_APFData.pErrorSpectrumPower_ + m_nFFTlen;  //2*fftlen
	m_APFData.nLengthFFT_=m_nFFTlen;
	m_APFData.bAECOn_= true;

	////////NR
	m_APFData.pNRInput_= m_APFData.pErrorFFT_;
	m_APFData.pNRDynamicRefer_= m_APFData.pEstimationFFT_;
	m_APFData.pNRInputRefer_= m_APFData.pDesireFFT_;
	m_APFData.bNROn_=true;
	//CDTDetector(int Fs = 16000, int Winlen = 15, int StarBin = 4, int EndBin = 60, int MaxDely = 5, float framelen_timeUms = 10.f, float updateCoeff=1.f, bool isExponentialCorrelation = true);
	m_CDTD = new CDTDetector(m_nFs, 25, 8, 90, 5, m_fFramelen_ms, 1, false);

	//init delay buffer
	m_CDelayBuf = new CDelayBuffer(m_nFFTlen, 20);
	m_CDelayBuf->Init();
	////init T2F for mic
	m_ppCT2FMics = new T2Ftransformer*[m_nMicsNum];
	m_pAECDataArray = new audio_pro_share[m_nMicsNum];
	m_ppCAECMics = new CAcousticEchoCancellationInFrequency*[m_nMicsNum];
	for (int i = 0; i < m_nMicsNum; i++) {
		m_ppCT2FMics[i] = new T2Ftransformer();
		m_ppCT2FMics[i]->InitFDanaly(m_nFramelen);
		memset(&m_pAECDataArray[i],0, sizeof(audio_pro_share));
		m_ppCAECMics[i] =  new CAcousticEchoCancellationInFrequency(m_nFs, m_nFFTlen, m_nFramelen);
		m_ppCAECMics[i]->Init();
	}

	m_CT2FRef = new T2Ftransformer();
	m_CT2FRef->InitFDanaly(m_nFramelen);
	//init F2T
	m_CF2TErr = new F2Ttransformer();
	m_CF2TErr->InitFDanaly(m_nFramelen);

#ifdef AUDIO_WAVE_DEBUG
	m_CF2TErrBeforeNR = new F2Ttransformer();
	m_CF2TErrBeforeNR->InitFDanaly(m_nFramelen);
#endif

	m_bInit = true;
	return 0;
}
void CAudioProcessingFramework::ProBufferCopy(float *fp, float* fpnew)
{
	int size = m_nFFTlen / 2;
	float *tp = fp + size;
	memcpy(tp, fpnew, sizeof(float)*size);

}
///move buffer
void CAudioProcessingFramework::UpdateProBuffer(float *fp)
{
	if (fp)
	{
		int size = m_nFFTlen / 2;
		memmove(fp, fp + size, sizeof(float)*size);
	}
}


int CAudioProcessingFramework::process(audio_pro_share& aShareData)
{
	int ret = 0;
	float* fpref = ((aShareData.pReffer_));
	float* fpRefFft = NULL;
	float vadband[3] = { 0 };
	float vadfull = 0.f;
	float fcorr = 0.f;

	if (!m_bInit)
		return -1;
	////mic array prepare
	if (aShareData.nChannelsInCapture_ == m_nMicsNum) {
		////////////time domain to frequency domain
		for (int i = 0; i < m_nMicsNum; i++) {
			m_ppCT2FMics[i]->T2F(aShareData.ppCapture_[i], m_APFData.ppCapureFFT_[i]);
			m_pAECDataArray[i].bAECOn_ = aShareData.bAECOn_;
			m_pAECDataArray[i].bNROn_ = aShareData.bNROn_;
			m_pAECDataArray[i].pDesireFFT_ = m_APFData.ppCapureFFT_[i];
	        ///// NR before aec
			////
		}
		if (m_APFData.bAECOn_)
		{
			///t2f ref
			m_CT2FRef->T2F(fpref, m_pReferFFT);

			//////far end vad
			m_pSPest->PwrEnergy(m_pReferFFT, m_pRefSp, m_pRefSp + m_nFFTlen);
			m_pVADest->GetVAD(m_pRefSp, m_pRefSp + m_nFFTlen, vadband, vadfull);
			m_bVad = (vadfull == 1);

			//////delay est
			Audioframe_t audioframe;
			audioframe.fp = m_pReferFFT;
			audioframe.VAD = m_bVad;
			audioframe.VADBand = vadband;
			audioframe.VADBandBuffSize = 3;
			m_CDelayBuf->UpdateData(&audioframe);
			m_CDelayBuf->getAudioFrame(m_nSystemDelay, &audioframe);
		    fcorr = m_CDTD->processDelay(audioframe.fp, m_APFData.ppCapture_[m_nMain_mic_index], m_bVad);
			m_CDelayBuf->getAudioFrame(m_nSystemDelay + m_nDelay, &audioframe);

			////echo est
	
			for (int i = 0; i < m_nMicsNum; i++) {
				m_pAECDataArray[i].pRefferFFT_ = m_pReferFFT;
				m_pAECDataArray[i].nFarVAD_ = audioframe.VAD;
				m_pAECDataArray[i].fDTDgain = fcorr;
				m_ppCAECMics[i]->process(m_pAECDataArray[i]);

			}
			//
			
			//

		}
		else
		{

		}
		//

		//F2T
		m_CF2TErr->F2T(m_APFData.pErrorFFT_, aShareData.pError_);


		if (aShareData.bNRCNGOn_)
		{
			for (int i = 0; i < m_nFFTlen / 2; i++)
			{
				aShareData.pError_[i] += m_APFData.pNRCNGBuffer_[i];
			}
		}
	


	}
	else
		return -1;


	return ret;
}


bool CAudioProcessingFramework::SetDelay(int nDelay)
{
	m_nSystemDelay = nDelay;
	return true;
}

int CAudioProcessingFramework::ProcessRefferData(audio_pro_share& aShareData)
{

	return 0;

}