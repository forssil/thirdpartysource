
#include <math.h>
#include <memory.h>
#include "processingconfig.h"
#include "basemath.h"
#include "SpeechEst.h"
CSpeechEst::CSpeechEst(float fs,int arraylen,int fftlen)
{
	m_nLen=arraylen;
	m_fFs=fs;
	m_nFFTLen=fftlen;
	InitSpeechEst();

}

CSpeechEst::~CSpeechEst(void)
{
	delete m_pfPrioriSNR;
}
void CSpeechEst::InitSpeechEst()
{
	int i;
	float a[]={10.f, 5.f};
	float b[]={3.f, 2.f};
	m_fAlpha_prio=0.7f;
	m_fBeta_prio=0.7f;
	SetGmin(0.0316f); //-30dB
	SetBinUsed(2,m_nLen);
	/*for high bands */
	for (i=0;i<2;i++)
	{
		m_pfPriori_bin_threshold_max[i]=a[i];
		m_pfPriori_bin_threshold_min[i]=b[i];
		m_pfPriori_bin_threshold_dif[i]=m_pfPriori_bin_threshold_max[i]-m_pfPriori_bin_threshold_min[i];		
	}	
	m_pnBands[0]=m_nStartBin;
	m_pnBands[1]=int(500*m_nFFTLen/m_fFs*(1.f+1.5f/4));
	m_pnBands[2]=m_nEndBin;
    if (m_pnBands[1]>m_pnBands[2])
    {

		m_pnBands[1]=m_pnBands[2];
    }
	m_fPriori_band_threshold_max=5.f;
	m_fPriori_band_threshold_min=2.f;
	m_fPriori_band_threshold_dif=m_fPriori_band_threshold_max-m_fPriori_band_threshold_min;
	m_fPriori_frame_threshold_max=6.f;
	m_fPriori_frame_threshold_min=2.f;
	m_fPriori_frame_threshold_dif=m_fPriori_frame_threshold_max-m_fPriori_frame_threshold_min;
	i=6*m_nLen;
	m_pfPrioriSNR=new float[i];
	memset(m_pfPrioriSNR,0,i*sizeof(float));
	m_pfPostSNR=m_pfPrioriSNR+m_nLen;
	m_pfQ=m_pfPostSNR+m_nLen;
	m_pfP=m_pfQ+m_nLen;
	m_pfGain=m_pfP+m_nLen;
	m_pfV=m_pfGain+m_nLen;

	m_fBins=float(m_nEndBin-m_nStartBin);
	m_fAlpha_q=0.7f;
	for (i=0;i<m_nLen;i++)
	{
		m_pfGain[i]=1.f;
		m_pfQ[i]=0.96f;
		m_pfPostSNR[i]=1.f;
	}

	m_fAllbandProb=0.f;

}
void CSpeechEst::UpdateSNR(float *InPwr,float* Noise,float* Trans)
{
	int i;
	float pwr,noise;
	float prisnr,possnr,prisnrest;
	float beta,alpha;
	beta=(1.f-m_fBeta_prio);
	alpha=(1.f-m_fAlpha_prio);
	m_fPriori_frame=0.f;
	Update_allbandProb(InPwr[0],Noise[0]);
	for (i=m_nStartBin;i<m_nEndBin;i++)
	{
		pwr=InPwr[i];
		noise=Noise[i]+Trans[i];
		possnr=pwr/(noise+minvalue);
		possnr=max(minvalue,possnr);
		//*%%*/priori snr update
		prisnrest= m_fAlpha_prio*m_pfGain[i]*m_pfGain[i]*m_pfPostSNR[i]+alpha*max(possnr-1,0.f);
		m_pfPostSNR[i]=possnr;
		prisnr=m_pfPrioriSNR[i];
		prisnr*=m_fBeta_prio;		
		prisnr+=beta*(prisnrest);
		m_pfPrioriSNR[i]=prisnr;
		m_fPriori_frame+=prisnr;
	}
	m_fPriori_frame/=m_fBins;
	
}

void CSpeechEst::UpdateSNR(float *InPwr, float* Noise)
{
	int i;
	float pwr, noise;
	float prisnr, possnr, prisnrest;
	float beta, alpha;
	beta = (1.f - m_fBeta_prio);
	alpha = (1.f - m_fAlpha_prio);
	m_fPriori_frame = 0.f;
	Update_allbandProb(InPwr[0], Noise[0]);
	for (i = m_nStartBin; i < m_nEndBin; i++)
	{
		pwr = InPwr[i];
		noise = Noise[i];
		possnr = pwr / (noise + minvalue);
		possnr = max(minvalue, possnr);
		//*%%*/priori snr update
		prisnrest = m_fAlpha_prio*m_pfGain[i] * m_pfGain[i] * m_pfPostSNR[i] + alpha*max(possnr - 1, 0.f);
		m_pfPostSNR[i] = possnr;
		prisnr = m_pfPrioriSNR[i];
		prisnr *= m_fBeta_prio;
		prisnr += beta*(prisnrest);
		m_pfPrioriSNR[i] = prisnr;
		m_fPriori_frame += prisnr;
	}
	m_fPriori_frame /= m_fBins;
}

float CSpeechEst::Update_allbandProb(float InPwr,float Noise)
{
	 float temp=(InPwr/(Noise+minvalue))/4.f-1.f;//[4,8]->[0,1]
	 temp=temp>1.f?1.f:temp;
	 temp=temp<0.f?0.f:temp;
	 m_fAllbandProb=temp;
	 return m_fAllbandProb;

}
void CSpeechEst::UpdateProb()
{
	float pest,pframeest,alphaq,v,prisnr,temp,q;
	int i,j,startloop,endloop;
    alphaq=1.f-m_fAlpha_q;
	pframeest=m_fPriori_frame-m_fPriori_frame_threshold_min;
	pframeest/=m_fPriori_frame_threshold_dif;
	pframeest=max(pframeest,0.f);
	pframeest=min(pframeest,1.f);
	pframeest*=(-1.f)*m_fAllbandProb;
	for (j=0;j<2;j++)
	{
		startloop=m_pnBands[j];
		endloop=m_pnBands[j+1];
		for (i=startloop;i<endloop;i++)
		{
			prisnr=m_pfPrioriSNR[i];
			pest=prisnr-m_pfPriori_bin_threshold_min[j];
			pest/=m_pfPriori_bin_threshold_dif[j];
			pest=max(pest,0.f);
			pest=min(pest,1.f);
			pest*=pframeest;
			pest+=1.f;
			pest*=0.96f;
			q=m_pfQ[i];
			q*=m_fAlpha_q;
			q+=alphaq*pest;
			m_pfQ[i]=q;

			temp=(1.f+prisnr);
			v=prisnr*m_pfPostSNR[i];
			v/=temp;
			m_pfV[i]=v;
			pest=(1.f-q);
			pest/=(1.f+(temp*expf(-v)-1.f)*q);
			m_pfP[i]=pest;		
		}	

		
	}
	
}
void CSpeechEst::UpdateGain()
{
	int i;
	float prisnr, g, p, lngmin;
	lngmin = logf(m_fGainMin);
	for (i=m_nStartBin;i<m_nEndBin;i++)
	{
		p=m_pfP[i];
		prisnr=m_pfPrioriSNR[i];
		/*g=prisnr/(prisnr+1.f);       		
		g*=expf(0.5f*expint(m_pfV[i]));
		g=min(1.f,g);
		g=max(m_fGainMin,g);
		m_pfGain[i]=powf(g,p);
		m_pfGain[i]*=powf(m_fGainMin,1.f-p);*/
		g = logf(prisnr / (prisnr + 1.f));
		g *= (0.5f*expint(m_pfV[i]));
		g = min(0.f, g);
		g = max(lngmin, g);
		prisnr = p * (g - lngmin) + lngmin;
		m_pfGain[i] = expf(prisnr);
	}
}
void CSpeechEst::Porcess(float *InPwer,float*Noise,float* Trans)
{

	UpdateSNR(InPwer,Noise,Trans);
	UpdateProb();
	UpdateGain();
}

void CSpeechEst::Porcess(float *InPwer, float* Noise)
{
	UpdateSNR(InPwer, Noise);
	UpdateProb();
	UpdateGain();
}

float CSpeechEst::GetSpeechProb()
{
	int i,j;
	float temp;
	float P[5]={0,0,0,0,0};
	for (i=m_nStartBin;i<m_nEndBin;i++)
	{

		temp=m_pfP[i];
		if (temp>P[0])
		{
			P[0]=temp;
			for (j=1;j<5;j++)
			{	
				if (temp>P[j])
				{
					P[j-1]=P[j];
					P[j]=temp;
				}
				else
					break;
			}
		}	
	}
	return P[2];
}