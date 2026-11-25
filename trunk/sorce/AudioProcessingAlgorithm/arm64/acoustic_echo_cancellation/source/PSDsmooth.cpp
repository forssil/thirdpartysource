#include "PSDsmooth.h"

#include <arm_neon.h>
#include <memory.h>
#include "processingconfig.h"
#include <stdio.h>
#include "basemath.h"
#include "arm64_optimization.h"

static inline float32x4_t reverse_f32x4(float32x4_t value)
{
	float32x4_t reversed_pairs=vrev64q_f32(value);
	return vextq_f32(reversed_pairs,reversed_pairs,2);
}

static inline float symmetric_window_sum(const float *input,int center,
		const float *window,int window_length)
{
	float result=input[center]*window[0];
	float32x4_t sum4=vdupq_n_f32(0.f);
	int offset=1;
	for (;offset+3<=window_length;offset+=4)
	{
		float32x4_t left=vld1q_f32(input+center-offset-3);
		float32x4_t right=vld1q_f32(input+center+offset);
		float32x4_t weights=vld1q_f32(window+offset);
		left=reverse_f32x4(left);
		sum4=vfmaq_f32(sum4,vaddq_f32(left,right),weights);
	}
	result+=vaddvq_f32(sum4);
	for (;offset<=window_length;offset++)
		result+=window[offset]*(input[center-offset]+input[center+offset]);
	return result;
}
CPSDsmooth::CPSDsmooth(int fftlen,int fs)
{
	m_nHalfFs=fs/2;
	m_nHalfFFTLen=fftlen/2;
	m_fInvDeltf=float(fftlen)/float(fs);
	InitPSDsmooth(CONSTANTPARA);

}

CPSDsmooth::~CPSDsmooth(void)
{
	delete m_pfPsdCQ_Fd;
	delete m_ppfWin[0];
	delete m_ppfWin[1];
	delete m_ppfWin[2];	
}
void CPSDsmooth::InitPSDsmooth(TDMode mode)
{
	int i;
	if (m_nHalfFs>=8000)
	{
		m_nF2n=8000;
		m_nCQBandNum=3;
	}
	else if (m_nHalfFs==4000)
	{
		m_nF2n=4000;
		m_nCQBandNum=2;
	}
	for (i=0;i<4;i++)
	{
		m_pnStep[i]=1<<i;
	}
	m_pnStep[4]=m_pnStep[3];
	m_nLowBand=int(1000*m_fInvDeltf+0.5);
	m_nLowBand += m_nLowBand%2;
	m_nHiBand =int(float(m_nHalfFs-m_nF2n)*m_fInvDeltf/8.f+0.5);
	m_nCQBand =m_nLowBand/2*m_nCQBandNum;
	m_nAllBand=m_nLowBand+m_nHiBand+m_nCQBand;

	m_pnWinLen[0]=InitWindow(m_ppfWin[0],0.5f);
	m_pnWinLen[1]=InitWindow(m_ppfWin[1],0.25f);
	m_pnWinLen[2]=InitWindow(m_ppfWin[2],0.125f);
	i=3*m_nAllBand+m_nHalfFFTLen+1+m_pnWinLen[2];
	m_pfPsdCQ_Fd=new float[i];
	m_pfPsdCQ_Td=m_pfPsdCQ_Fd+m_nAllBand;
	m_pfInBuffer=m_pfPsdCQ_Td+m_nAllBand;
	memset(m_pfPsdCQ_Fd,0,(i)*sizeof(float));

	//TD
	m_pfAlpha=m_pfInBuffer+m_nHalfFFTLen+1+m_pnWinLen[2];
	m_fAlpha_c=1.f;
	m_fAlpha_max=0.96f;
	m_nTDMode=mode;
	SetAlpha(0.3f);
	 
}
//call this function when the fist frame is used to initialize the module.
void CPSDsmooth::SetInitFrame(float *input)
{

	UpdateInBuf(input);
	FDsmooth_process();
	memcpy(m_pfPsdCQ_Td,m_pfPsdCQ_Fd,m_nAllBand*sizeof(float) );
}
int CPSDsmooth::InitWindow(float *&Win,float ratio)
{
	int i,j;
	float fwin[128];
	float x,x1,x2;
	float temp,sum;
	if (ratio<=0.01f)
	{
		return 0;
	}
	i=0;
	sum=fwin[0]=temp=0.5f*ratio;
	
	while ((temp>0.0000001f)&&(i<128))
	{
		i++;
		fwin[i]=temp;
		sum+=2.f*temp;
		x=i*ratio*PI;
		x1=PI-x;
		x2=PI+x;
		temp=0.5f*ratio*(sinc_fun(x)+0.5f*(sinc_fun(x1)+sinc_fun(x2)));
	}
	Win=new float[i];		
	for (j=0;j<i;j++)
	{
		Win[j]=fwin[j]/sum;					
	}
	return i-1;

}
void CPSDsmooth::UpdateInBuf(float *InPwr)
{
	int j;
	int i=m_nHalfFFTLen;
	memcpy(m_pfInBuffer,InPwr,m_nHalfFFTLen*sizeof(float));
	m_pfInBuffer[i]=0.f;
	for (j=1;j<=m_pnWinLen[2];j++)
	{
		m_pfInBuffer[i+j]=m_pfInBuffer[i-j];
	}
}

void CPSDsmooth::FDsmooth_process()
{
	int i,k,j,maxloop,step,iCQ;
	int CQbandLen=m_nLowBand;
	float tempmax;
	int maxindex;
	int winlen;
	float *win;
	//low_band porcessing
    maxloop=m_nLowBand;
#if 0
	for (i=0;i<maxloop;i+=m_pnStep[0])
	{
		m_pfPsdCQ_Fd[i]=m_pfInBuffer[i];
	}
#else
	m_pfPsdCQ_Fd[0]=m_pfInBuffer[0];
	for (i=1;i+4<=maxloop;i+=4)
	{
		float32x4_t center=vld1q_f32(m_pfInBuffer+i);
		float32x4_t left=vld1q_f32(m_pfInBuffer+i-1);
		float32x4_t right=vld1q_f32(m_pfInBuffer+i+1);
		float32x4_t result=vmulq_n_f32(center,0.8f);
		result=vfmaq_n_f32(result,left,0.1f);
		result=vfmaq_n_f32(result,right,0.1f);
		vst1q_f32(m_pfPsdCQ_Fd+i,result);
	}
	for (;i<maxloop;i+=m_pnStep[0])
	{
		m_pfPsdCQ_Fd[i]=0.8f*m_pfInBuffer[i]+m_pfInBuffer[i+1]*0.1f+m_pfInBuffer[i-1]*0.1f;
	}
#endif
	iCQ=i;
    //CQ band processing
	for(j=1;j<=m_nCQBandNum;j++)
	{		
		step=m_pnStep[j];
		maxloop+=CQbandLen*(1<<(j-1));
		winlen=m_pnWinLen[j-1];
		win=m_ppfWin[j-1];
		for(;i<maxloop;)
		{
			
			tempmax=0.f;
			maxindex=0;
			for (k=0;k<step;k++,i++)
			{				
				if (tempmax<m_pfInBuffer[i])//min(bins)
				{
					tempmax=m_pfInBuffer[i];
					maxindex=i;
				}
			}
			m_pfPsdCQ_Fd[iCQ]=symmetric_window_sum(m_pfInBuffer,maxindex,win,winlen);
			iCQ++;
		}
	}
	//high band processing
	step=m_pnStep[j];
	maxloop=m_nHalfFFTLen;
	winlen=m_pnWinLen[2];
	win=m_ppfWin[2];
	for(;i<maxloop-step;) 
	{
		
		tempmax=m_pfInBuffer[i];
		maxindex=i;
		for (k=0;k<step;k++,i++)
		{				
			if (tempmax<m_pfInBuffer[i])
			{
				tempmax=m_pfInBuffer[i];
				maxindex=i;
			}
		}
		m_pfPsdCQ_Fd[iCQ]=symmetric_window_sum(m_pfInBuffer,maxindex,win,winlen);
		iCQ++;
	}

	tempmax=m_pfInBuffer[i];
	maxindex=i;
	for(;i<maxloop; ++i) 
	{
		if (tempmax<m_pfInBuffer[i])
		{
			tempmax=m_pfInBuffer[i];
			maxindex=i;
		}
	}

	m_pfPsdCQ_Fd[iCQ]=symmetric_window_sum(m_pfInBuffer,maxindex,win,winlen);
}
void CPSDsmooth::UpdateTDpara(float *noisepsd)
{
	switch (m_nTDMode)
	{
	case OPTIMALSMOOTH:
		UpdateTDpara_OSMS(noisepsd);
		break;	
	case CONSTANTPARA:
		//UpdateTDpara_CONST(); //it's unnecessary to update alpha every frame; just call SetAlpha() after init;
		break;
	default :
		//UpdateTDpara_CONST();
		break;		
	}

}
void CPSDsmooth::SetAlpha(float alpha)
{
	int i;
	float32x4_t alpha4=vdupq_n_f32(alpha);
	for (i=0;i+4<=m_nAllBand;i+=4)
	{
		vst1q_f32(m_pfAlpha+i,alpha4);
	}
	for (;i<m_nAllBand;i++)
	{
		m_pfAlpha[i]=alpha;		
	}

}
void CPSDsmooth::UpdateTDpara_OSMS(float *noisepsd)
{
	float sumPwr,sumPwr2,temp,temp1;
	int i;
    float alpha_cc;
	sumPwr=0.f;
	sumPwr2=0.f;
	for (i=0;i<m_nAllBand;i++)
	{
		sumPwr+=m_pfPsdCQ_Td[i];
		sumPwr2+=m_pfPsdCQ_Fd[i];
	}

	temp=sumPwr/(sumPwr2+minvalue)-1.f;
	temp*=temp;
	alpha_cc=1.f/(1.f+temp);
	alpha_cc=max(0.7f,alpha_cc);
	m_fAlpha_c*=0.7f;
	m_fAlpha_c+=0.3f*alpha_cc;
	temp=m_fAlpha_c*m_fAlpha_max;
	float32x4_t temp4=vdupq_n_f32(temp);
	float32x4_t one4=vdupq_n_f32(1.f);
	float32x4_t min4=vdupq_n_f32(minvalue);
	float32x4_t floor4=vdupq_n_f32(0.3f);
	for (i=0;i+4<=m_nAllBand;i+=4)
	{
		float32x4_t td=vld1q_f32(m_pfPsdCQ_Td+i);
		float32x4_t noise=vld1q_f32(noisepsd+i);
		float32x4_t ratio=vdivq_f32(td,vaddq_f32(noise,min4));
		float32x4_t delta=vsubq_f32(ratio,one4);
		float32x4_t alpha=vdivq_f32(temp4,vfmaq_f32(one4,delta,delta));
		vst1q_f32(m_pfAlpha+i,vmaxq_f32(floor4,alpha));
	}
	for (;i<m_nAllBand;i++)
	{
		temp1=m_pfPsdCQ_Td[i]/(noisepsd[i]+minvalue)-1;
		temp1*=temp1;		
		m_pfAlpha[i]=temp/(1.f+temp1);
		m_pfAlpha[i]=max(0.3f,m_pfAlpha[i]);
	}
}
void CPSDsmooth::TDsmooth_process()
{

	int i;
	float *fpalpha;
	float* fp;
	
	fp=m_pfPsdCQ_Td;
	fpalpha=m_pfAlpha;
	float32x4_t one4=vdupq_n_f32(1.f);
	for(i=0;i+4<=m_nAllBand;i+=4)
	{
		float32x4_t previous=vld1q_f32(fp+i);
		float32x4_t alpha=vld1q_f32(fpalpha+i);
		float32x4_t current=vld1q_f32(m_pfPsdCQ_Fd+i);
		float32x4_t result=vmulq_f32(previous,alpha);
		result=vfmaq_f32(result,vsubq_f32(one4,alpha),current);
		vst1q_f32(fp+i,result);
	}
	for(;i<m_nAllBand;i++)
	{
		fp[i]*=fpalpha[i];
		fp[i]+=(1.f-fpalpha[i])*m_pfPsdCQ_Fd[i];
	}

}
//InPwr  format:[re(0)^2+ im(0)^2, re(1)^2+im(1)^2, ... re(FFTSIZE/2-1)^2+ im(FFTSIZE/2-1)^2] ,size FFTsize/2
//noisedpsd , 1x m_nAllband;
//outbuffer, 1x m_nAllband;
void CPSDsmooth::processing(float* InPwr,float *noisepsd)
{

	UpdateInBuf(InPwr);
	//FD
	FDsmooth_process();
	if (noisepsd!=NULL)
	{
		//TD
		UpdateTDpara(noisepsd);
		
	}
    TDsmooth_process();
    //output
	//memcpy(outbuffer,m_pfPsdCQ_Td,m_nAllBand*siezof(float));

}

void CPSDsmooth::CQSpread(float* inbuffer,float *outbuffer)
{

	int i,k,j,maxloop,step,iCQ;
	int CQbandLen=m_nLowBand;		
	//low_band porcessing
	maxloop=m_nLowBand;
	for (i=0;i<maxloop;i+=m_pnStep[0])
	{
		outbuffer[i]=inbuffer[i];
	}
	iCQ=i;
	//CQ band processing
	for(j=1;j<=m_nCQBandNum;j++)
	{		
		step=m_pnStep[j];
		maxloop+=CQbandLen*j;		
		for(;i<maxloop;)
		{				
			for(k=0;k<step;k++)
			{				
				outbuffer[i++]=inbuffer[iCQ];						
			}			
			iCQ++;
		}
	}
	//high band processing
	step=m_pnStep[j];
	maxloop=m_nHalfFFTLen;	
	for(;i<maxloop-step;)
	{
		k=0;		
		while(k++<step)
		{			
			outbuffer[i++]=inbuffer[iCQ];				
		}		
		iCQ++;
	}

	for(;i<maxloop; ++i) 
	{
		outbuffer[i]=inbuffer[iCQ];	
	}
}
