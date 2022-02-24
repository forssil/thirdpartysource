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
#include "AcousticEchoCancellation.h"

extern "C"
{
	CEchoCancellationInterface* CreateIAECInst(int Fs,float fftlen_ms,float framlen_ms)
	{
		CAcousticEchoCancellation *inst=new CAcousticEchoCancellation( Fs, fftlen_ms, framlen_ms);
		if(inst)
			return (CEchoCancellationInterface*)inst;
		else
			return NULL;

	};
	CEchoCancellationInterface* CreateIAECInst_int(int Fs,int fftlen,int framlen)
	{
		
		CAcousticEchoCancellation *inst=new CAcousticEchoCancellation( Fs, fftlen, framlen);
		if(inst)
			return (CEchoCancellationInterface*)inst;
		else
			return NULL;

	};
	int DeleteIAECInst (CEchoCancellationInterface*  IAecInst)	
	{
		if(IAecInst)
		{
			delete (CAcousticEchoCancellation*)IAecInst;
			return 0;
		}
		else
			return -1;

	};
};


////class CAcousticEchoCancellation
CAcousticEchoCancellation::CAcousticEchoCancellation(int Fs,float fftlen_ms,float framlen_ms):
m_fFFTlen_ms(fftlen_ms)
,m_nFs(Fs)
,m_fFramelen_ms(framlen_ms)
,m_pReferFFT(NULL)
,m_bResetFlag(false)
,m_CDTD(NULL)
,m_CDelayBuf(NULL)
,m_bInit(false)
,m_CT2FMic(NULL)
,m_CT2FRef(NULL)
,m_nDelay(0)
,m_CF2TErr(NULL)
,m_pMemAlocat(NULL)
,m_nTail(0)
,m_bVad(false)
,m_fCrossCor(0)
,m_fGain(1.f) 
,m_pMemArray(NULL)
,m_pSubBandAdap(NULL)
,m_pSubBandAdap2(NULL)
,m_nSystemDelay(0)
,m_pPostFil(NULL)

#ifdef AUDIO_WAVE_DEBUG
, m_CF2TErrBeforeNR(NULL)
#endif

{
	m_nFFTlen=int(m_fFFTlen_ms*m_nFs/1000);
	m_nFramelen=int(m_fFramelen_ms*m_nFs/1000);


	memset(m_ppAuidoInBuf, 0, 2 * sizeof(void*));
	memset(m_ppAudioOutBuf, 0, 2 * sizeof(void*));
};
CAcousticEchoCancellation::CAcousticEchoCancellation(int Fs,int fftlen_sample,int framlen_sample):
	m_nFFTlen(fftlen_sample)
	,m_nFs(Fs)
	,m_nFramelen(framlen_sample)
	,m_pReferFFT(NULL)
	,m_bResetFlag(false)
	,m_CDTD(NULL)
	,m_CDelayBuf(NULL)
	,m_bInit(false)
	,m_CT2FMic(NULL)
	,m_CT2FRef(NULL)
	,m_nDelay(0)
	,m_CF2TErr(NULL)
	,m_pMemAlocat(NULL)
	,m_nTail(0)
	,m_bVad(false)
	,m_fCrossCor(0)
	,m_fGain(1.f) 
	,m_pMemArray(NULL)
	,m_pSubBandAdap(NULL)
	,m_pSubBandAdap2(NULL)
	,m_nSystemDelay(0)
	,m_pPostFil(NULL)
	,m_pSPest(NULL)
	,m_pVADest(NULL)
	,m_pRefSp(NULL)

#ifdef AUDIO_WAVE_DEBUG
	, m_CF2TErrBeforeNR(NULL)
#endif

{
	m_fFFTlen_ms=float (m_nFFTlen*1000)/m_nFs;
	m_fFramelen_ms =float(m_nFramelen*1000)/m_nFs;


	memset(m_ppAuidoInBuf, 0, 2 * sizeof(void*));
	memset(m_ppAudioOutBuf, 0, 2 * sizeof(void*));
};

CAcousticEchoCancellation::~CAcousticEchoCancellation()
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

	if (NULL != m_CT2FMic)
	{
		delete m_CT2FMic;
		m_CT2FMic = NULL;
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

	if(	NULL != m_pSubBandAdap)
	{
		delete m_pSubBandAdap;
		m_pSubBandAdap = NULL;
	}
	if (NULL != m_pSubBandAdap2)
	{
		delete m_pSubBandAdap2;
		m_pSubBandAdap2 = NULL;
	}
	if (NULL != m_pMemAlocat)
	{
		delete[] m_pMemAlocat;
		m_pMemAlocat = NULL;
	}
	if(NULL != m_pMemArray)
	{
		delete m_pMemArray;
		m_pMemArray = NULL;
	}
	if(NULL!=m_pPostFil)
	{
		delete m_pPostFil;
		m_pPostFil = NULL;
	}
	if(NULL != 	m_pSPest)
	{
		delete m_pSPest;
		m_pSPest =NULL;
	}
	if(NULL != m_pVADest)
	{
		delete m_pVADest;
		m_pVADest=NULL;
	}
}

void CAcousticEchoCancellation::Reset()
{
	m_bResetFlag=true;
}

//thread safe 
int CAcousticEchoCancellation::ResetAll()
{
	int ret=0;
	if(m_bResetFlag)	
	{
		///reset module

		if(ret==0)
		{
			m_bResetFlag=false;
			return ret;
		}
	}
	else
		return ret;
	
}

 int CAcousticEchoCancellation::Init()
 {

	 //
	 memset(&m_AECData,0,sizeof(audio_pro_share));
	 m_pMemArray=new float*[4];
	 memset(m_pMemArray,0,sizeof(float*));


	
	 m_pMemAlocat=new float[m_nFFTlen*14];
	 memset(m_pMemAlocat,0,m_nFFTlen*14*sizeof(float));
	 m_AECData.pDesire_=m_pMemAlocat;
	 m_AECData.pReffer_=(m_AECData.pDesire_)   +m_nFFTlen;
	 m_AECData.pDesireFFT_= (m_AECData.pReffer_)+m_nFFTlen;
	 m_pReferFFT =(m_AECData.pDesireFFT_)+m_nFFTlen;

	 m_AECData.pEstimationFFT_=m_pReferFFT+m_nFFTlen;
	 m_AECData.pErrorFFT_= m_AECData.pEstimationFFT_+m_nFFTlen;
	 m_AECData.pError_=m_AECData.pErrorFFT_+m_nFFTlen;
	 m_AECData.pErrorSpectrumPower_=m_AECData.pError_+m_nFFTlen;
	 m_pRefSp= m_AECData.pErrorSpectrumPower_ + m_nFFTlen;  //2*fftlen
	 m_AECData.nLengthFFT_=m_nFFTlen;
	 m_AECData.bAECOn_= true;

	 //////NR
	 m_AECData.pNRInput_=m_AECData.pErrorFFT_;
	 m_AECData.pNRDynamicRefer_=m_AECData.pEstimationFFT_;
	 m_AECData.pNRInputRefer_=m_AECData.pDesireFFT_;
	 m_AECData.bNROn_=true;
	 //CNG
	 m_AECData.bNRCNGOn_=true;
	 m_AECData.pNRCNGBuffer_=m_pReferFFT;///reuse this buffer

	 ////adf2 
	 m_AECData2.pDesire_ = m_AECData.pDesire_;
	 m_AECData2.pReffer_ = m_AECData.pReffer_;
	 m_AECData2.pDesireFFT_ = m_AECData.pErrorFFT_;
	

	 m_AECData2.pEstimationFFT_ = m_pRefSp + m_nFFTlen;
	 m_AECData2.pErrorFFT_ = m_AECData2.pEstimationFFT_ + m_nFFTlen;
	 m_AECData2.pError_ = m_AECData2.pErrorFFT_ + m_nFFTlen;
	 m_AECData2.pErrorSpectrumPower_ = m_AECData2.pError_ + m_nFFTlen;

	 m_AECData2.nLengthFFT_ = m_nFFTlen;
	 m_AECData2.bAECOn_ = true;
	 m_AECData2.nOffsetBin_ = 2;
	 //init DTD
	//fs=22050
	 //CDTDetector(int Fs = 16000, int Winlen = 15, int StarBin = 4, int EndBin = 60, int MaxDely = 5, float framelen_timeUms = 10.f, float updateCoeff=1.f, bool isExponentialCorrelation = true);
	 m_CDTD=new CDTDetector(m_nFs,25,8,90,10,m_fFramelen_ms,1,false);
	
	 //init delay buffer
	 m_CDelayBuf=new CDelayBuffer(m_nFFTlen,200);
	 m_CDelayBuf->Init();
	 ////init T2F
	 m_CT2FMic=new T2Ftransformer();
	 m_CT2FMic->InitFDanaly(m_nFramelen);
	 m_CT2FRef=new T2Ftransformer();
	 m_CT2FRef->InitFDanaly(m_nFramelen);
	 //init F2T
	 m_CF2TErr=new F2Ttransformer();
	 m_CF2TErr->InitFDanaly(m_nFramelen); 

#ifdef AUDIO_WAVE_DEBUG
	 m_CF2TErrBeforeNR=new F2Ttransformer();
	 m_CF2TErrBeforeNR->InitFDanaly(m_nFramelen); 
#endif

	 m_pSubBandAdap=new CSubbandAdap(m_nFs,m_nFFTlen);
	 m_pSubBandAdap->Subband_init();
	 m_pSubBandAdap2 = new CSubbandAdap(m_nFs, m_nFFTlen);
	 m_nadf2_filterbancunm = 2050 * m_nFFTlen / m_nFs;
	 m_pSubBandAdap2->Subband_init(1, m_nadf2_filterbancunm, 0.85f,0.f);
	 m_pPostFil  =new CPostFilter(m_nFs,m_nFFTlen);
	 //SPest
	 m_pSPest=new SPEst();
	 m_pSPest->InitPara(m_nFramelen);
	 //aec vad
	 m_pVADest=new AEC_VAD();
	 m_pVADest->CreateVAD_int(m_nFs,m_nFFTlen,m_nFramelen);

	 m_AECData.pGain_=m_pPostFil->GetGain();
	 m_AECData.pNoiseSPwr=m_pPostFil->GetNoiseEst();

	 m_AECData2.pGain_ = m_pPostFil->GetGain();
	 m_AECData2.pNoiseSPwr = m_pPostFil->GetNoiseEst();

	 m_AECData.nOffsetBin_=2;
	 m_bInit=true;
	 return 0;
 }
 void CAcousticEchoCancellation::ProBufferCopy(float *fp,float* fpnew)
 {
	 int size=m_nFFTlen/2;
	 float *tp=fp+size;
	 memcpy(tp,fpnew,sizeof(float)*size);

 }
 ///move buffer
 void CAcousticEchoCancellation::UpdateProBuffer(float *fp)
 {
	 if(fp)
	 {
		 int size=m_nFFTlen/2;
		 memmove(fp,fp+size,sizeof(float)*size);
	 }
 }


  int CAcousticEchoCancellation::process(audio_pro_share& aShareData)
  {
	  int ret=0;

	  float* fpmic=(aShareData.pDesire_);
	  float* fpref=((aShareData.pReffer_));
	  float* fpRefFft=NULL;
	  float vadband[3]={0};
	  float vadfull=0.f;
	  m_AECData.bAECOn_ = aShareData.bAECOn_;
	  m_AECData.bNROn_  = aShareData.bNROn_ ;
	  m_AECData.bNRCNGOn_ = aShareData.bNRCNGOn_;
	  if(m_bInit)
	  {
		  ////////////time domain to frequency domain
		  m_CT2FMic->T2F(fpmic,m_AECData.pDesireFFT_);
		  if(m_AECData.bAECOn_)
		  {
				m_CT2FRef->T2F(fpref,m_pReferFFT);
	
				 //////far end vad
				 m_pSPest->PwrEnergy(m_pReferFFT, m_pRefSp,m_pRefSp+m_nFFTlen);
				 m_pVADest->GetVAD(m_pRefSp ,m_pRefSp+m_nFFTlen,vadband,vadfull);
				 m_bVad=(vadfull==1);

				 //////delay est
				Audioframe_t audioframe;
				audioframe.fp=m_pReferFFT;
				audioframe.VAD=m_bVad;
				audioframe.VADBand=vadband;
				audioframe.VADBandBuffSize=3;
				m_CDelayBuf->UpdateData(&audioframe);
				m_CDelayBuf->getAudioFrame(m_nSystemDelay,&audioframe);
				float fcorr= m_CDTD->processDelay(audioframe.fp,m_AECData.pDesireFFT_,m_bVad);
				//m_nDelay=(m_CDTD->GetDelay());
		  
				  m_CDelayBuf->getAudioFrame(m_nSystemDelay+m_nDelay,&audioframe);
				  m_AECData.pRefferFFT_=audioframe.fp;
				m_AECData.nFarVAD_   =audioframe.VAD;
				m_AECData.fDTDgain   *=0.8f;
				m_AECData.fDTDgain   +=0.2f*fcorr;

				m_AECData2.pRefferFFT_ = audioframe.fp;
				m_AECData2.nFarVAD_ = audioframe.VAD;
				m_AECData2.fDTDgain = m_AECData.fDTDgain;

			////echo est

				 m_pSubBandAdap->process( m_AECData.pRefferFFT_,m_AECData.pDesireFFT_,m_AECData.pErrorFFT_,m_AECData.pEstimationFFT_, m_AECData.nOffsetBin_,m_AECData);
				 m_pSubBandAdap2->process(m_AECData.pRefferFFT_, m_AECData2.pDesireFFT_, m_AECData2.pErrorFFT_, m_AECData2.pEstimationFFT_, m_AECData2.nOffsetBin_, m_AECData2);
				 for (int i = 0; i < m_nadf2_filterbancunm + m_AECData2.nOffsetBin_; i++) {
					 m_AECData.pErrorFFT_[2 * i] = m_AECData2.pErrorFFT_[2 * i];
					 m_AECData.pErrorFFT_[2 * i + 1] = m_AECData2.pErrorFFT_[2 * i + 1];
					 m_AECData.pEstimationFFT_[2 * i] += m_AECData2.pEstimationFFT_[2 * i];
					 m_AECData.pEstimationFFT_[2 * i + 1 ] += m_AECData2.pEstimationFFT_[2 * i + 1];
				 }

		  }
		  else
		  {
				memcpy( m_AECData.pErrorFFT_,m_AECData.pDesireFFT_,sizeof(float)*m_AECData.nLengthFFT_);  
		  }

#ifdef AUDIO_WAVE_DEBUG
		  // used to save audio data before NR operation
		  if(aShareData.pErrorBeforeNR_)
			  m_CF2TErrBeforeNR->F2T(m_AECData.pErrorFFT_, aShareData.pErrorBeforeNR_);
#endif

		  if(m_AECData.bNROn_)
		  {
			  m_pPostFil->Process(&m_AECData);
		  }

		  m_CF2TErr->F2T(m_AECData.pErrorFFT_, aShareData.pError_);


		  if (m_AECData.bNRCNGOn_)
		  {
			  for (int i=0;i<m_nFFTlen/2;i++)
			  {
				  aShareData.pError_[i] += m_AECData.pNRCNGBuffer_[i];
			  }
		  }

#if 0
		  for(int i=0; i<m_AECData.nLengthFFT_/2; i++)
		  {
		     aShareData.pError_[i] = m_AECData.fDTDgain;
		  }

		  if(m_AECData.fDTDgain>=0.9)
		  {
		  	  m_AECData.fDTDgain *= 1;
		  }
#endif

	  }
	  else
		  ret=-1;	  
	  return ret;
  }


  bool CAcousticEchoCancellation::SetDelay(int nDelay)
  {
	  m_nSystemDelay=nDelay;
	  return true;
  }

  int CAcousticEchoCancellation::ProcessRefferData(audio_pro_share& aShareData)
  {
	   
	  return 0;

  }


  ////class CAcousticEchoCancellationInFrequency
 
  CAcousticEchoCancellationInFrequency::CAcousticEchoCancellationInFrequency(int Fs, int fftlen_sample, int framlen_sample) :
	  m_nFFTlen(fftlen_sample)
	  , m_nFs(Fs)
	  , m_nFramelen(framlen_sample)
	  , m_pReferFFT(NULL)
	  , m_bResetFlag(false)
	  , m_bInit(false)
	  , m_pMemAlocat(NULL)
	  , m_nTail(0)
	  , m_bVad(false)
	  , m_fCrossCor(0)
	  , m_fGain(1.f)
	  , m_pSubBandAdap(NULL)
	  , m_pPostFil(NULL)
	  , m_pSPest(NULL)
	  , m_pRefSp(NULL)
  {
	  m_fFFTlen_ms = float(m_nFFTlen * 1000) / m_nFs;
	  m_fFramelen_ms = float(m_nFramelen * 1000) / m_nFs;


	  memset(m_ppAuidoInBuf, 0, 2 * sizeof(void*));
	  memset(m_ppAudioOutBuf, 0, 2 * sizeof(void*));
  };

  CAcousticEchoCancellationInFrequency::~CAcousticEchoCancellationInFrequency()
  {
	  if (NULL != m_pSubBandAdap)
	  {
		  delete m_pSubBandAdap;
		  m_pSubBandAdap = NULL;
	  }
	  if (NULL != m_pMemAlocat)
	  {
		  delete[] m_pMemAlocat;
		  m_pMemAlocat = NULL;
	  }

	  if (NULL != m_pPostFil)
	  {
		  delete m_pPostFil;
		  m_pPostFil = NULL;
	  }
	  if (NULL != m_pSPest)
	  {
		  delete m_pSPest;
		  m_pSPest = NULL;
	  }
  }

  void CAcousticEchoCancellationInFrequency::Reset()
  {
	  m_bResetFlag = true;
  }

  //thread safe 
  int CAcousticEchoCancellationInFrequency::ResetAll()
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

  int CAcousticEchoCancellationInFrequency::Init()
  {

	  //
	  memset(&m_AECData, 0, sizeof(audio_pro_share));

	  //
	  m_pMemAlocat = new float[m_nFFTlen * 10];
	  memset(m_pMemAlocat, 0, m_nFFTlen * 10 * sizeof(float));
	  //m_AECData.pDesire_ = m_pMemAlocat;
	  //m_AECData.pReffer_ = (m_AECData.pDesire_) + m_nFFTlen;
	  //m_AECData.pDesireFFT_ = (m_AECData.pReffer_) + m_nFFTlen;
	  //m_pReferFFT = (m_AECData.pDesireFFT_) + m_nFFTlen;

	  m_AECData.pEstimationFFT_ = m_pMemAlocat + m_nFFTlen;
	  m_AECData.pErrorFFT_ = m_AECData.pEstimationFFT_ + m_nFFTlen;
	  m_AECData.pErrorSpectrumPower_ = m_AECData.pErrorFFT_ + m_nFFTlen;
	  m_pRefSp = m_AECData.pErrorSpectrumPower_ + m_nFFTlen;  //2*fftlen
	  m_AECData.nLengthFFT_ = m_nFFTlen;
	  m_AECData.bAECOn_ = true;

	  //////NR
	  m_AECData.pNRInput_ = m_AECData.pErrorFFT_;
	  m_AECData.pNRDynamicRefer_ = m_AECData.pEstimationFFT_;
	  m_AECData.pNRInputRefer_ = m_AECData.pDesireFFT_;
	  m_AECData.bNROn_ = true;
	  //CNG
	  m_AECData.bNRCNGOn_ = true;
	  m_AECData.pNRCNGBuffer_ = m_pReferFFT;///reuse this buffer

	  m_pSubBandAdap = new CSubbandAdap(m_nFs, m_nFFTlen);
	  m_pSubBandAdap->Subband_init();
	  m_pPostFil = new CPostFilter(m_nFs, m_nFFTlen);
	  //SPest
	  m_pSPest = new SPEst();
	  m_pSPest->InitPara(m_nFramelen);

	  m_AECData.pGain_ = m_pPostFil->GetGain();
	  m_AECData.pNoiseSPwr = m_pPostFil->GetNoiseEst();

	  m_AECData.nOffsetBin_ = 2;
	  m_bInit = true;
	  return 0;
  }
  void CAcousticEchoCancellationInFrequency::ProBufferCopy(float *fp, float* fpnew)
  {
	  int size = m_nFFTlen / 2;
	  float *tp = fp + size;
	  memcpy(tp, fpnew, sizeof(float)*size);

  }
  ///move buffer
  void CAcousticEchoCancellationInFrequency::UpdateProBuffer(float *fp)
  {
	  if (fp)
	  {
		  int size = m_nFFTlen / 2;
		  memmove(fp, fp + size, sizeof(float)*size);
	  }
  }


  int CAcousticEchoCancellationInFrequency::process(audio_pro_share& aShareData)
  {
	  int ret = 0;

	  float vadband[3] = { 0 };
	  float vadfull = 0.f;
	  m_AECData.bAECOn_ = aShareData.bAECOn_;
	  m_AECData.bNROn_ = aShareData.bNROn_;
	  m_AECData.bNRCNGOn_ = aShareData.bNRCNGOn_;
      m_pReferFFT = aShareData.pRefferFFT_;
      m_AECData.pErrorFFT_ = aShareData.pErrorFFT_;
	  if (m_bInit)
	  {
		  ////////////time domain to frequency domain
		 
		  if (m_AECData.bAECOn_)
		  {
			  //////far end vad
			  m_pSPest->PwrEnergy(m_pReferFFT, m_pRefSp, m_pRefSp + m_nFFTlen);
			  m_bVad = (vadfull == 1);

			  //////delay est
			  Audioframe_t audioframe;
			  audioframe.fp = m_pReferFFT;
			  audioframe.VAD = m_bVad;
			  audioframe.VADBand = vadband;
			  audioframe.VADBandBuffSize = 3;

			  m_AECData.nFarVAD_ = audioframe.VAD;
			  m_AECData.fDTDgain *= 0.8f;
			  m_AECData.fDTDgain += 0.2f*aShareData.fDTDgain;
			  m_AECData.pDesireFFT_ = aShareData.pDesireFFT_;
			  m_AECData.pRefferFFT_ = aShareData.pRefferFFT_;

			  ////echo est
			  m_pSubBandAdap->process(m_AECData.pRefferFFT_, m_AECData.pDesireFFT_, m_AECData.pErrorFFT_, m_AECData.pEstimationFFT_, m_AECData.nOffsetBin_, m_AECData);

		  }
		  else
		  {
              m_AECData.pDesireFFT_ = aShareData.pDesireFFT_;
			  memcpy(m_AECData.pErrorFFT_, m_AECData.pDesireFFT_, sizeof(float)*m_AECData.nLengthFFT_);
		  }

		  if (m_AECData.bNROn_)
		  {
			  m_AECData.pNRInput_ = m_AECData.pErrorFFT_;
			  m_AECData.pNRDynamicRefer_ = m_AECData.pEstimationFFT_;
			  m_AECData.pNRInputRefer_ = m_AECData.pDesireFFT_;
			  m_pPostFil->Process(&m_AECData);
		  }

	  }
	  else
		  ret = -1;
	  return ret;
  }


  bool CAcousticEchoCancellationInFrequency::SetDelay(int nDelay)
  {
	  return true;
  }

  int CAcousticEchoCancellationInFrequency::ProcessRefferData(audio_pro_share& aShareData)
  {

	  return 0;

  }