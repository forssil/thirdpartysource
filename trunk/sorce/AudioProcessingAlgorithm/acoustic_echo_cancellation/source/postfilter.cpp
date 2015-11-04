#include "postfilter.h"
#include <memory.h>

#include <stdio.h>
#include "processingconfig.h"
#include "randomization_functions.h"
#include "fft.h"
#define n50dB  -50 //1e-5f
#define n75dB  -80 //3.1623e-8f
#define gainspeed 0.7f
#define LOWLEVELTHREASHOLD 0.5*1e-5
float win[]={0.02f,0.96f};

CPostFilter::CPostFilter(int fs,int fftlen)
{
	int i;
	//float gmin=0.0316f;//-30dB
	m_fMinGain=0.05f;
	m_nFs=fs;
	m_nFFTLen=fftlen;
	m_nHalfFFTLen=fftlen/2;
	//m_CNLP=new CNLP(fs,fftlen);
	//envelop tracking
	m_fReferPwrEnvelop = 0;
	m_fReferPwrEnvelopDb = 0;
	m_fReferPwrEnvelopUpdateStep = 5.f / (5 + 16);
	m_fAllBandGain = 1;
	
	
	m_CNoisRedu=new CNoiseRedu(fs,fftlen);
	
	//m_CSpecCtrl=new CSpecCtrl(fftlen,fs,m_fMinGain);
    m_nBufferSize=17*m_nHalfFFTLen;  
	m_pfPwr=new float[m_nBufferSize];
	m_pfEstPwr=m_pfPwr+ m_nHalfFFTLen;
	m_pfAft=m_pfEstPwr+ m_nHalfFFTLen;
	m_pfBef=m_pfAft+ m_nHalfFFTLen;
	m_pfEst=m_pfBef+ m_nHalfFFTLen;
	m_pfGain=m_pfEst+m_nHalfFFTLen;
	m_pfGaintemp=m_pfGain+m_nHalfFFTLen;
	m_pfLimitGain=m_pfGaintemp+m_nHalfFFTLen;
	m_pfWinWeight=m_pfLimitGain+m_nHalfFFTLen;
	m_pfHighFreCtr=m_pfWinWeight+m_nHalfFFTLen;
	m_pfLowpassWin=m_pfHighFreCtr+m_nHalfFFTLen;
	m_pfCNGPwr=m_pfLowpassWin+m_nHalfFFTLen;
	m_pfCNGFFT=m_pfCNGPwr+m_nHalfFFTLen;
	m_pfCNGFFTtemp=m_pfCNGFFT+m_nFFTLen;
	m_pfReferPwr = m_pfCNGFFTtemp + m_nHalfFFTLen;
	//random number for cng
	m_pnRandW16=new int16_t[m_nHalfFFTLen];

	Reset();
	
}

CPostFilter::~CPostFilter(void)
{
	if(NULL != m_CNoisRedu) 
	   delete m_CNoisRedu;
	//delete m_CNLP;
	if(NULL != m_pfPwr) 
	   delete m_pfPwr;
	if(NULL != m_pnRandW16) 
       delete m_pnRandW16;
}
void CPostFilter::SetGainMin(float noisepwr)
{
	float temp;
	if (noisepwr<m_nHalfFFTLen)
	{
		m_fNoisePwr*=0.75f;
		m_fNoisePwr+=0.25f*noisepwr;
	}
	
    //temp=0.0316f*(n50dB-n75dB)/(10.f*log10(m_fNoisePwr)-n75dB+minvalue);
    /*20111220*/
	temp=(10.f*log10(m_fNoisePwr+minvalue)-n75dB)/(n50dB-n75dB)/0.0316f;
	
	temp=temp>1.f?(1.f/temp):1.f;
    //
	temp=temp>1.f?1.f:temp;
	temp=temp<0.1f?0.1f:temp;

	m_fMinGain*=0.5f;
	m_fMinGain+=temp*0.5f;

	//m_CNLP->SetMinG(m_fMinGain);
	m_CNoisRedu->SetGmin(m_fMinGain);
}
void CPostFilter::Reset()
{
	int i,maxloop;
	float delt;
	m_fShellgain=1.f;
	m_fNoisePwr=1e-7f;
	SetGainMin(m_fNoisePwr);
	memset(m_pfPwr,0,m_nBufferSize*sizeof(float));
	memset(m_pnRandW16,0,m_nHalfFFTLen*sizeof(int16_t));
	ResetGain(1.f);
	m_pfWin=win;
	m_nOffset=0;
	m_fNR=-100.f;//for extra nr;
	m_fExtraG=1.f;
	m_fSp=0.f;
	delt= 1.f/float(m_nHalfFFTLen);
	for (i=0;i<m_nHalfFFTLen;i++)
	{
		m_pfWinWeight[i]=Window_Depreenhance(i,delt);
		m_pfGain[i] = 1.f;
		m_pfGaintemp[i] =1.f;
		m_pfLimitGain[i]=1.f;
		m_pfEstPwr[i]=m_fNoisePwr;
	}
	m_fAfRatio=1.f;
	m_fBefadfPwr=0.f;
	m_fAftadfPwr=0.f;
	m_fFarEndPwr=0.f;
	m_nInd65k=80;//index of 5kHz
	m_nInd80k=127;
	maxloop=m_nInd80k>m_nHalfFFTLen?m_nHalfFFTLen:m_nInd80k;
	if (m_nInd65k<maxloop)
	{
	for (i=0;i<m_nInd65k;i++)
	{
		m_pfHighFreCtr[i]=1.f;
	}
	for (;i<maxloop;i++)
	{
		m_pfHighFreCtr[i]=1.f-0.9f*cosf(((i-m_nInd65k)/float(m_nInd80k-m_nInd65k)+1.f)*3.1415926f*0.5f);//(0.6f+0.5f*cosf((i-m_nInd65k)/float(m_nInd80k-m_nInd65k)*3.1415926))/1.1f;
	}
	for (;i<m_nHalfFFTLen;i++)
	{
		m_pfHighFreCtr[i]=0.1f;
	}
	}
	else
	{
		for (i=0;i<maxloop;i++)
		{
			m_pfHighFreCtr[i]=1.f;
		}
	}

	////////////////////20150713 lowpass 
	for (i=0;i<m_nHalfFFTLen;i++)
	{
		if (i<m_nHalfFFTLen/2)
		{
			m_pfLowpassWin[i]=1.f;
		}
		else if (i<m_nHalfFFTLen*3/4)
		{
			m_pfLowpassWin[i]=m_pfLowpassWin[i-1]*0.94f;
		}
		else
		{
			m_pfLowpassWin[i]=0.0003f;
		}
		
	}
	
	m_fLowLevelThreashold=LOWLEVELTHREASHOLD;
	//cng
	m_nSeed=777;
	
}
void CPostFilter::Process(audio_pro_share *Aec)
{
	int i;
	float* fp1=m_CNoisRedu->GetGain();
	m_nOffset=Aec->nOffsetBin_/2;
	UpdatePwr( Aec, 0.3f);
	///envelop tracking
	UpdateReferPwr(Aec);
	m_fReferPwrEnvelop = TrackEnvelop(m_pfReferPwr[0], m_fReferPwrEnvelop, m_fReferPwrEnvelopUpdateStep);
	UpdateAllBandGain();
	m_fReferPwrEnvelopDb = 10 * log10f(m_fReferPwrEnvelop) + 6; // add power for the sync part
	
	m_CNoisRedu->Process(m_pfPwr,m_pfEstPwr,*Aec,m_pfAft,m_pfBef);
	{
		for (i=0;i<m_nHalfFFTLen;i++)
		{
			m_pfGaintemp[i]=fp1[i];
		}
	}

	
	UpdateGain();	
	m_CNoisRedu->ReUpdateGain(m_pfGaintemp);

	/*for extra gain*/
//	m_fSp=m_CNoisRedu->m_CSpeech->GetSpeechProb();
	//close this feature 2015.07.15
	/*
	m_fSp = m_CNoisRedu->m_CSpeechStatic->GetSpeechProb();
	m_fNR=m_CNoisRedu->NoisePwr_allband(&m_fNR_bef);

	//update min gain
	SetGainMin(m_fNR_bef);
	*/

	//m_fShellgain*=m_fExtraG;
	//2015.07.15 for add cng	
	Spe_Limiter(Aec);
	if (Aec->bNRCNGOn_)
	{
		AddCNG(Aec);
		//memcpy(Aec->pNRCNGBuffer_,m_pfCNGFFT,m_nFFTLen*sizeof(float));
		InvFFT(m_pfCNGFFT,m_pfCNGFFTtemp,m_nFFTLen);
		memcpy(Aec->pNRCNGBuffer_,m_pfCNGFFT,m_nFFTLen*sizeof(float));
	}
	
	//////processing 
	for (int i=0;i<Aec->nLengthFFT_/2;i++)
 	{
		Aec->pErrorFFT_[2*i]*=Aec->pGain_[i];		
 	    Aec->pErrorFFT_[2*i+1]*=Aec->pGain_[i];	
		///Why cannot add in frequency domain?
// 		Aec->pErrorFFT_[2*i]+=m_pfCNGFFT[2*i];	
// 		Aec->pErrorFFT_[2*i+1]+=m_pfCNGFFT[2*i+1];
    }
	


}
void CPostFilter::UpdatePwr(audio_pro_share *Aec,float speed)
{
	int i;
	float* fp1,*fp2,*fp3;
	float re,im,temp;
	float sum=0.f;
	float slowness = 1.0f - speed;
	float speed_4overpi = speed;
	fp1=Aec->pNRInput_;
	fp2=Aec->pNRDynamicRefer_;
	fp3=Aec->pNRInputRefer_;
	for (i=0;i<m_nHalfFFTLen;i++)
	{
		re=*fp1++;
		im=*fp1++;
		temp=re*re+im*im;
		m_pfPwr[i]=temp;
		sum+=temp;
		m_pfAft[i]*=slowness;		
		m_pfAft[i]+=speed_4overpi*temp;

		if(NULL!=Aec->pEstimationFFT_)
		{
			re=*fp2++;
			im=*fp2++;
			temp=re*re+im*im;
			m_pfEstPwr[i]=temp;
			m_pfEst[i]*=slowness;
			m_pfEst[i]+=speed_4overpi*temp;

		}
		if(NULL != Aec->pDesireFFT_)
		{
			re=*fp3++;
			im=*fp3++;
			temp=re*re+im*im;
			m_pfBef[i]*=slowness;
			m_pfBef[i]+=speed_4overpi*temp;
		}
	}
	sum-=(m_pfPwr[0]+m_pfPwr[1]);
	m_pfPwr[0]=sum;

}
//envelop tracking
void CPostFilter::UpdateReferPwr(audio_pro_share *Aec)
{
	int i;
	float re, im, temp;
	float sum = 0.f;
	float* fp = Aec->pRefferFFT_;
	for (i = 0; i < m_nHalfFFTLen; i++)
	{
		re = *fp++;
		im = *fp++;
		temp = re*re + im*im;
		m_pfReferPwr[i] = temp;
		sum += temp;
	}
	sum -= (m_pfReferPwr[0] + m_pfReferPwr[1]);
	
	//
	m_pfReferPwr[0] = sum;
}
//envelop tracking
float CPostFilter::TrackEnvelop(float newData, float envelop, float updateStep)
{
	if(newData >= envelop)
	{
		return newData;
	}
	else
	{
		envelop = (1-updateStep) * envelop + updateStep * newData;
	}

	return envelop;
}
//limiter gain
void CPostFilter::UpdateAllBandGain()
{
	float temGain = 0;
	if (m_fReferPwrEnvelopDb < -35)
	{
		temGain = 1;
	}
	else if (m_fReferPwrEnvelopDb < -15)
	{
		temGain = (5.f - m_fReferPwrEnvelopDb) / 40; // -6db
	}
	else
	{
		temGain = 0.355; // -9db
	}

	m_fAllBandGain = 0.9 * m_fAllBandGain + 0.1 * temGain;
}

float CPostFilter::GetAllBandGain()
{
	return m_fAllBandGain;
}

void CPostFilter::UpdateGain()
{
	int i,j,startbin;
	float temp,temp1,temp2;
	temp2=(1.f-gainspeed);
	for (i=0;i<m_nOffset;i++)
	{
		 m_pfGain[i]=m_fMinGain*m_pfLowpassWin[i];
	}
    startbin=max(winlen,m_nOffset);
	if (i<startbin)
	{
		for (;i<startbin;i++)
		{
			m_pfGain[i]*=temp2;
			m_pfGain[i]+=gainspeed*m_pfGaintemp[i];
			/////////////////
			m_pfGain[i]*=m_pfLowpassWin[i];
		}
	}
	for (i=startbin;i<m_nHalfFFTLen-1;i++)
	{
		temp=m_pfGaintemp[i];        
		temp*=m_pfWin[winlen];
		for (j=0;j<winlen;j++)
		{
			temp1=m_pfWin[winlen-j-1];
			temp+=temp1*m_pfGaintemp[i-j-1];
			temp+=temp1*m_pfGaintemp[i+j+1];
		}
		m_pfGain[i]*=temp2;
		m_pfGain[i]+=gainspeed*temp;	
		///////////////////
		/////////////////
		m_pfGain[i]*=m_pfLowpassWin[i];
	}
	m_pfGain[i]=m_fMinGain;
	/////////////////
	m_pfGain[i]*=m_pfLowpassWin[i];
}
void CPostFilter::ResetGain(float g)
{
	int i;
	for (i=0;i<m_nHalfFFTLen;i++)
	{
		m_pfGain[i]=g;
	}

}
void CPostFilter::SelectGain()
{
	int i;
	float *fp1;
	fp1=m_CNoisRedu->GetGain();

	for (i=0;i<m_nHalfFFTLen;i++)
	{
		m_pfGaintemp[i]=(fp1[i]);
	}
}
void CPostFilter::UpdateAECinfo(float *proriSNR,float* noisepwr,float* speechp,float *noisebef)
{
	*noisepwr=m_fNR;
//	*proriSNR=m_CNoisRedu->m_CSpeech->GetProiSNR();
//	*speechp= m_fSp*m_CNoisRedu->m_CSpeech->m_fAllbandProb;

	*proriSNR = m_CNoisRedu->m_CSpeechStatic->GetProiSNR();
	*speechp = m_fSp*m_CNoisRedu->m_CSpeechStatic->m_fAllbandProb;
	
	if (m_fNR_bef>minvalue)
	{
		*noisebef=10.f*log10f(m_fNoisePwr);
	}
	else
		*noisebef=-260;
}
float CPostFilter::Window_Depreenhance(int n,float delt)
{
	
	float temp;
    temp=1.f+0.8125f*0.8125f-2*0.8125f*cosf(n*delt*3.1415926f);
	temp+=minvalue;
	return (1.f/(temp));
}
float CPostFilter::AllbandPwr(float *specpwr,float *gain)
{
	int i;
	float temp,re,im;
	float *fg;
	float *fp1=specpwr+2;
	float temp_bef=0.f;	
	
	if (gain==NULL)
	{
		for (i=1;i<m_nHalfFFTLen;i++)
		{
			re=*fp1++;	
			im=*fp1++;
			temp=re*re+im*im;		
			temp*=m_pfWinWeight[i];
			temp_bef+=temp;
		}	


	}
	else
	{

		fg=gain+1;
		for (i=1;i<m_nHalfFFTLen;i++)
		{
			re=*fp1++;	
			im=*fp1++;
			temp=re*re+im*im;		
			temp*=m_pfWinWeight[i]*(*fg++);
			temp_bef+=temp;
		}	
	}
	
	return temp_bef/8.f;//4 for pre-enhance; 2 for analysis fiter
}
void CPostFilter::Spe_Limiter(audio_pro_share *aeinfo)
{
	
	float tempPwr=0.f;
	float tempgain=1.f;
	for (int i=2;i<m_nHalfFFTLen-1;i++)
	{ 
		tempPwr+=m_pfAft[i]*m_pfGain[i]*m_pfGain[i];
	}
	if (tempPwr<2*m_fLowLevelThreashold)
	{
		tempgain=0.6;
	}
	m_pfGaintemp[0]=1.f;
	m_pfGaintemp[1]=1.f;
	for (int i=2;i<m_nHalfFFTLen-1;i++)
	{ 			
		m_pfGaintemp[i]=m_pfGain[i]*tempgain;
	}
	m_pfGaintemp[m_nHalfFFTLen-1]=m_pfGain[m_nHalfFFTLen-1];
}



void CPostFilter::AddCNG(audio_pro_share *aeinfo)
{
	float *fpNoise=m_CNoisRedu->GetNoisefft();
	const float pi2 = 6.28318530717959f;
	float tmp =0.f;
	float noise =0.f;
	int i=0;
	// Generate a uniform random array on [0 1]
	WebRtcSpl_RandUArray(m_pnRandW16, m_nHalfFFTLen, &m_nSeed);
	m_pfCNGFFT[0]=0.f;
	m_pfCNGFFT[1]=0.f;
	for (i = 1; i < m_nHalfFFTLen-1; i++) {
		if ((fpNoise[i]<1.f)&&(fpNoise[i]>minvalue))
		{
			m_pfCNGPwr[i]*=0.999f;
			m_pfCNGPwr[i]+=0.001f*(fpNoise[i-1]+fpNoise[i+1]+fpNoise[i])/75;
		}
		tmp = pi2 *  ((float)m_pnRandW16[i-1]) / 32768;
		noise = sqrtf(m_pfCNGPwr[i]);	
		m_pfCNGFFT[2*i]=noise * cosf(tmp);
		m_pfCNGFFT[2*i+1]=-noise * sinf(tmp);;	
	}	
	m_pfCNGFFT[m_nFFTLen-2]=0.f;
	m_pfCNGFFT[m_nFFTLen-1]=0.f;
	
}
