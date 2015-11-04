
#include <math.h>
#include <memory.h>
#include "processingconfig.h"
#include "noiseest.h"
#define Bias 2.5f // the PSD of noise est is normally lower
CNoiseEst::CNoiseEst(float fs,int arraylen,NoiseEstMode mode)
{
	m_fFs=fs;
	m_nLen=arraylen;
	m_nMode=mode;
	InitNosieEst();
}

CNoiseEst::~CNoiseEst(void)
{
	delete m_pfPwr;
	delete m_pbKmode;
	delete m_pnMinIndx;
}
float CNoiseEst::MD_interp(int N)
{
	float MD_N;
	int i=0;
	float MD[]={0.f,0.26f,0.48f,0.58f,0.61f,0.668f,0.705f,0.762f,0.8f,0.841f,0.865f,0.89f,0.9f,0.91f };
	float D[] ={1.f,2.f,5.f,8.f, 10.f, 15.f, 20.f ,30.f ,40.f ,60.f, 80.f, 120.f, 140.f, 160.f };
    int len=14;
    MD_N=float(N);
	while (MD_N>D[i]) i++;
    if ((i>0)&&(i<14))
    {
		MD_N-=D[i-1];
		MD_N/=(D[i]-D[i-1]);
		MD_N*=(MD[i]-MD[i-1]);
		MD_N+=MD[i-1];
    }
	else
		MD_N=0.f;
	return MD_N;
}
void CNoiseEst::SetWin(int subwinlen,int subwinnum)
{
	m_nSubWinLen=subwinlen;
	m_nSubWinNum=subwinnum;
	m_nWinLen=m_nSubWinNum*m_nSubWinLen;
	m_fMSubWin=MD_interp( m_nSubWinLen);
	m_fMWin   =MD_interp(m_nWinLen);

}
void CNoiseEst::InitNosieEst()
{
	int i;
	m_nBeginFrames=0x0000000f;//for 96ms analysis filter
	if(m_nMode==MS)
	{
		SetWin(10,25);
		m_fAlpha_v=2.f;
		i=m_nLen*(9+m_nSubWinNum);
		m_pfPwr=new float[i];
		memset(m_pfPwr,0,i*sizeof(float));
		m_pfPwr2=m_pfPwr+m_nLen;
		m_pfLocalMin= m_pfPwr2+m_nLen;
		m_pfGlobeMin= m_pfLocalMin+ m_nLen;
		m_pfBminLocal= m_pfGlobeMin+ m_nLen;
		m_pfBminGlobe= m_pfBminLocal+ m_nLen;
		m_pfPwrMin =  m_pfBminGlobe+ m_nLen;
		m_pfNoise  = m_pfPwrMin   + m_nLen;
		m_pfTrans = m_pfNoise+ m_nLen;
		m_pfDelayLine = m_pfTrans+ m_nLen;
		
		

		i=2*m_nLen;
		m_pbKmode=new bool[i];
		m_pbMinFlag=m_pbKmode+m_nLen;
		memset(m_pbKmode,0,i*sizeof(bool));
		m_pnMinIndx=new int[m_nLen];
		for (i=0;i<m_nLen;i++)
		{
			m_pfLocalMin[i]=maxvalue;
			m_pfGlobeMin[i]=maxvalue;
			m_pfNoise[i]=maxvalue;
			SetDelayline(i,maxvalue);
		}
		m_nDelIndx=0;
		
		m_nSubWinCounter=m_nSubWinLen;

		Bmin=(float(m_nWinLen)-1.f)*2.f;
		Bminsub=(float(m_nSubWinLen)-1.f)*2.f;
	}

	

}
// update the values in a row of delayline
void CNoiseEst::SetDelayline(int bin,float valu)
{
	int i,j;
	j=bin;
	for (i=0;i<m_nSubWinNum;i++)
	{
		m_pfDelayLine[j]=valu;
		j+=m_nLen;
		
	}
	m_pnMinIndx[bin]=0;
	

}
// update the values in a column of delayline
void CNoiseEst::UpdateDelayLine(float* valu)
{
	int i,j,k,m;
	float tempv,tempd;
	j=m_nDelIndx*m_nLen;	
	for (i=0;i<m_nLen;i++)
	{
		tempv=valu[i];
		m_pfDelayLine[j+i]=tempv;
		//find minimum
		k=m_pnMinIndx[i];
		if (k!=m_nDelIndx)
		{
			m_pnMinIndx[i]=tempv>m_pfDelayLine[k*m_nLen+i]?k:m_nDelIndx;
		}
		else
		{
			k=0;
			tempv=m_pfDelayLine[i];
			for (m=1;m<m_nSubWinNum;m++)
			{
				tempd=m_pfDelayLine[m*m_nLen+i];
				if (tempv>tempd)
				{
					k=m;
					tempv=tempd;
				}
			}
			m_pnMinIndx[i]=k;
		}
		
	}
	m_nDelIndx++;
	m_nDelIndx%=m_nSubWinNum;
}


void  CNoiseEst::UpdatePara(float *alpha,float *InPwr)
{
	int i;
	float beta,temp;
	float *fpalpha=alpha;
	float *infp=InPwr;
	float varP,Qeq;
	//float Bmin,Bminsub;

	
	m_fQeqmean=0.f;
	for (i=0;i<m_nLen;i++)
	{
		beta=*fpalpha*(*fpalpha);
		beta=min(beta,0.8f);
		temp=(1-beta)*(*infp);
		m_pfPwr[i]*=beta;
		m_pfPwr[i]+=temp;
		m_pfPwr2[i]*=beta;
		m_pfPwr2[i]+=temp*(*infp);
		
		temp=(m_pfPwr2[i]-m_pfPwr[i]*m_pfPwr[i]);
		varP=temp>minvalue?temp:minvalue;
		temp=varP/2.f/(m_pfNoise[i]*m_pfNoise[i]+minvalue);

		Qeq=min(0.5f,temp);

		temp=(1.f/(Qeq+minvalue)-2.f*m_fMWin)+minvalue;
		temp=1.f/temp;
		temp*=(1.f-m_fMWin);			
		m_pfBminGlobe[i]=1.f+Bmin*temp;

		temp=(1.f/(Qeq+minvalue)-2.f*m_fMSubWin)+minvalue;
		temp=1.f/temp;
		temp*=(1.f-m_fMSubWin);			
		m_pfBminLocal[i]=1.f+Bminsub*temp;
		m_fQeqmean+=Qeq;

		fpalpha++;
		infp++;
	}
	m_fQeqmean/=float(m_nLen);
	m_fBc=(1.f+m_fAlpha_v*sqrtf(m_fQeqmean));
	
	 infp=InPwr;
	for (i=0;i<m_nLen;i++)
	{
		m_pfBminGlobe[i]*=m_fBc*(*infp)*Bias;
		m_pfBminLocal[i]*=m_fBc*(*infp)*Bias;
		infp++;
	}

}
void CNoiseEst::NoiseTrack()
{
	int i;
	float noise_slope_max,tempmin,tempminsub;
	bool flag;
	float alpa_up;//learn speed for high level noise
	for(i=0;i<m_nLen;i++)
	{
		if (m_pfBminGlobe[i]<m_pfGlobeMin[i])
		{
			m_pfGlobeMin[i]=m_pfBminGlobe[i];
			m_pfLocalMin[i]=m_pfBminLocal[i];
			m_pbKmode[i]=true;
		}
	}
	if (m_nSubWinCounter==m_nSubWinLen)
	{

		//store globe min
		UpdateDelayLine(m_pfGlobeMin);
		
		for (i=0;i<m_nLen;i++)
		{			
			flag=m_pbMinFlag[i]&&(!m_pbKmode[i]);
            tempmin=m_pfDelayLine[i+m_pnMinIndx[i]*m_nLen];

			if (m_fQeqmean<0.03)
				noise_slope_max=2.f;
			else 
			{
				if( m_fQeqmean<0.05) 
					noise_slope_max=1.8f;
				else
				{
					if (m_fQeqmean<0.06) 
						noise_slope_max=1.5f;
					else
						noise_slope_max=1.2f;
				}
			}
			//
			tempminsub=m_pfLocalMin[i];

			if (flag)
			{
				if(tempminsub>tempmin)
					if (tempminsub<(tempmin*noise_slope_max))
					{
						tempmin=tempminsub;
						SetDelayline(i,tempminsub);
					}
			}			
			///update para
			m_pbMinFlag[i]=false;
			m_pfPwrMin[i]=tempmin;			
			m_pfGlobeMin[i]=maxvalue;
			m_pfLocalMin[i]=maxvalue;
		}
		m_nSubWinCounter=1;
	}
	else
	{
		if (m_nSubWinCounter>1)
		{
			for (i=0;i<m_nLen;i++)
			{
				flag=m_pbMinFlag[i]||(m_pbKmode[i]);
				m_pbMinFlag[i]=flag;
				tempmin=m_pfPwrMin[i];
				tempminsub=m_pfBminLocal[i];
				tempmin=min(tempminsub,tempmin);
				
// 				if (tempmin*Bias>m_pfNoise[i])
// 				{
// 					alpa_up=0.5;
// 
// 					m_pfNoise[i]*=alpa_up;
// 					m_pfNoise[i]+=(1-alpa_up)*tempmin*Bias;				
// 				}
// 				else
					m_pfNoise[i]=tempmin;
				m_pfPwrMin[i]=tempmin;
			}
		}
		m_nSubWinCounter++;
	}
}
void CNoiseEst::Process(float *InPwr,float *alpha)
{

	m_nBeginFrames>>=1;
	if (!(m_nBeginFrames&0x00000001))
	{
		UpdatePara(alpha,InPwr);
		NoiseTrack();
	}

	// memcpy(outbuffer,m_pfNoise,m_nLen*sizeof(float));
}
//call this function when the fist frame is used to initialize the module.
void CNoiseEst::SetInitFrame(float *InPwr)
{
	
	int i;
	float temp;
	float *fp=InPwr;
	UpdateDelayLine(fp);
	for (i=0;i<m_nLen;i++)
	{
		temp=*fp;
		m_pfPwr[i]=temp;
		m_pfPwr2[i]=temp*temp;
		m_pfNoise[i]=temp;	
		m_pfGlobeMin[i]=temp;
	}

}