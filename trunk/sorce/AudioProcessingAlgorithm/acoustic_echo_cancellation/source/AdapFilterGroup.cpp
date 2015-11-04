

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include "AdapFilterGroup.h"
CAdapFilterGroup::CAdapFilterGroup(int numbank,int *ntaps)
{
	m_nNumBank=numbank;
    m_npTaps=ntaps;
	AdapfilterIni();

}
CAdapFilterGroup::~CAdapFilterGroup()
{

	if (m_cpReferDelayLine)
	{
		delete m_cpReferDelayLine;
	}
	if (m_npDelaylIndx)
	{
		delete m_npDelaylIndx;
	}
	if (m_pDeltaFreWeight)
	{
		delete m_pDeltaFreWeight;
	}

}

void CAdapFilterGroup::AdapfilterIni()
{

	
	int i;
	////add variable delta weights 20160626
	//#ifdef ADAPTIVE_FILTER_ALGO_AP
	SetMinMaxDelta(5e-8f, 2e-3f);
	//#elif defined ADAPTIVE_FILTER_ALGO_NLMS
	//SetMinMaxDelta(5e-7f, 0.1f);
	//#endif
	m_pDeltaFreWeight=new float[m_nNumBank];
	m_nSumTapsBank=0;
	for (i=0;i<m_nNumBank;i++)
	{
		m_nSumTapsBank+=m_npTaps[i];	
		////add variable delta weights 20160626
		m_pDeltaFreWeight[i]=1.f-float(i)/m_nNumBank;
		if (m_pDeltaFreWeight[i]<2.f/float(m_npTaps[i]))
		{
			m_pDeltaFreWeight[i]=2.f/float(m_npTaps[i]);
		}
		m_pDeltaFreWeight[i]*=m_npTaps[i]*m_fMinDelta;
	}
	m_nSumLenR11    =m_nSumTapsBank+m_nNumBank;
	m_nSumLenDelLine=2*m_nSumLenR11;
	m_nSumLenR12    =2*m_nSumTapsBank;
    //memory assign 
	m_npDelaylIndx=NULL;
	m_npDelaylIndx=new int[m_nNumBank*4+1];
	memset(m_npDelaylIndx,0,sizeof(int)*(m_nNumBank*4+1));
	m_npR11Indx   =m_npDelaylIndx+m_nNumBank+1;
	m_npR12Indx   =m_npR11Indx   +m_nNumBank;
	m_npAttack_length=m_npR12Indx+m_nNumBank;
    
	m_cpReferDelayLine=NULL;
	i=m_nSumLenDelLine*3+2+m_nSumLenR11*2+m_nSumLenR12+m_nNumBank*28;		
	m_cpReferDelayLine=new float[i]; 
	memset(m_cpReferDelayLine,0,sizeof(float)*(i));
    
	
	m_cpAdW       =m_cpReferDelayLine+m_nSumLenDelLine+2;
	m_cpFixW      =m_cpAdW           +m_nSumLenDelLine;

    m_fpAttackTaps=m_cpFixW          +m_nSumLenDelLine;
    m_fpR11       =m_fpAttackTaps    +m_nSumLenR11;

    m_cpR12       =m_fpR11           +m_nSumLenR11;

    m_fpR11sum    =m_cpR12           +m_nSumLenR12;
    m_fpR22sum    =m_fpR11sum        +m_nNumBank;
    m_fpExpDecay  =m_fpR22sum        +m_nNumBank;
    m_fpDen       =m_fpExpDecay      +m_nNumBank;
	m_fpMaxAdp    =m_fpDen           +m_nNumBank;
    m_fpDelta     =m_fpMaxAdp        +m_nNumBank;
	
    
	m_cpR12sum    =m_fpDelta         +m_nNumBank;
    m_cpCorr      = m_cpR12sum       +2*m_nNumBank;
  
	m_cpAdErrPre  =m_cpCorr          +2*m_nNumBank;
    m_cpBeta1     =m_cpAdErrPre      +2*m_nNumBank;
	m_cpFixBeta   =m_cpBeta1         +2*m_nNumBank;
	m_cpBeta2     =m_cpFixBeta       +2*m_nNumBank;
	m_cpBeta12    =m_cpBeta2         +2*m_nNumBank;
	m_cpAdErr     =m_cpBeta12        +2*m_nNumBank;
	m_cpAdEst     =m_cpAdErr         +2*m_nNumBank;
	m_cpFixErr    =m_cpAdEst         +2*m_nNumBank;
	m_cpFixEst    =m_cpFixErr        +2*m_nNumBank;

    //inite 
	m_npDelaylIndx[0]=0;
	m_npR11Indx[0]=0;
    m_npR12Indx[0]=0;
	for (i=1;i<m_nNumBank;i++)
	{
		m_npR11Indx[i]=m_npTaps[i-1]+1+m_npR11Indx[i-1];
		m_npDelaylIndx[i]=m_npR11Indx[i]*2;
		m_npR12Indx[i]=2*m_npTaps[i-1]+m_npR12Indx[i-1];		
	}
    m_npDelaylIndx[i]=m_nSumLenDelLine;
	m_fDeltaTC=0.97f;
	m_fMaxAdpAll=0;
	m_fMu=0.5;

	m_nFlagcnt=0;
	m_cpDes=NULL;
}

//*newRefer size 2*m_nNumBank
void CAdapFilterGroup::UpdateR11_R12(const float *newRefer)
{
	int i,indx,indy;
	////update R11
	memmove(m_fpR11+1,m_fpR11,(m_nSumLenR11-1)*sizeof(float));	

	////update R12		
	memmove(m_cpR12+2,m_cpR12,(m_nSumLenR12-2)*sizeof(float));

	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;
		indy=*(i+m_npDelaylIndx);
		*(m_fpR11+*(m_npR11Indx+i))  =newRefer[indx]*newRefer[indx]+newRefer[indx+1]*newRefer[indx+1];
		*(m_cpR12+*(m_npR12Indx+i))  = *(m_cpReferDelayLine+indy+2)*newRefer[indx]+*(m_cpReferDelayLine+indy+3)*newRefer[indx+1];
        *(m_cpR12+*(m_npR12Indx+i)+1)=-*(m_cpReferDelayLine+indy+3)*newRefer[indx]+*(m_cpReferDelayLine+indy+2)*newRefer[indx+1];
	}
}

void CAdapFilterGroup::UpdateReferEnergy()
{
	float upStep = 0.015;
	float downStep = 0.1;

	for (int i=0; i<m_nNumBank; ++i)
	{
		//float energy = *(m_fpR11+*(m_npR11Indx+i));
		if(m_fpR11sum[i] > m_fpDen[i])
		{
			m_fpDen[i] *=  (1-upStep);
			m_fpDen[i] += upStep * m_fpR11sum[i];
		}
		else
		{
			m_fpDen[i] *=  (1-downStep);
			m_fpDen[i] += downStep * m_fpR11sum[i];
		}	
	}
}

//be called in CSubbandadap
void CAdapFilterGroup::UpdateDecay(int indxbin,int attack_length, float *attackTaps,float subbandDecay,float attackGain)
{
	int i,j;
	float expDecay=1.f;

	float *AttackTaps;
	i=m_npTaps[indxbin];
	AttackTaps=m_fpAttackTaps+m_npR11Indx[indxbin];
	m_npAttack_length[indxbin]=attack_length;
	
	for (j=0;j<attack_length;j++)
	{
		expDecay = attackGain * attackTaps[j];
		AttackTaps[j] = expDecay;
	}
	if (attack_length==0) 
	{
		AttackTaps[0] = expDecay;
		attack_length = 1;
	}
	for (j=attack_length;j<i;j++)
	{
		expDecay *= subbandDecay;
		AttackTaps[j] = expDecay;      
	}

}
//*newRefer size 2*m_nNumBank
void CAdapFilterGroup::UpdateDelayline(const float *newRefer)
{
	int i,indx,indy;
	// update delay line
	memmove(m_cpReferDelayLine+2,m_cpReferDelayLine,(m_nSumLenDelLine-2)*sizeof(float));
	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;
		indy=*(m_npDelaylIndx+i);
		*(m_cpReferDelayLine+indy)  =*(newRefer+indx);
		*(m_cpReferDelayLine+indy+1)=*(newRefer+indx+1);
	}
	UpdateR11_R12(newRefer);//update R11 R12
}

void CAdapFilterGroup::UpdateDelta(float* fp, float fCorr)
{
	int i,indx;
	
	float DeltaGain = 0.2f;
	float err_powr=0.f;
	float tempf=m_fDeltaTC;
	
	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;
	
#ifdef ADAPTIVE_FILTER_ALGO_AP
		if(NULL==fp)
		{
			err_powr=0.f;
		}
		else
		{
			err_powr= DeltaGain * fp[i];
		}
		err_powr+=DeltaGain * (m_cpAdErr[indx] * m_cpAdErr[indx] + m_cpAdErr[indx+1] * m_cpAdErr[indx+1]);

		m_fpDelta[i] *= m_fDeltaTC;
		m_fpDelta[i] += (1.0f - m_fDeltaTC) * (err_powr);
#elif defined ADAPTIVE_FILTER_ALGO_NLMS
		if(NULL==fp)
		{
			err_powr = 0.f;
		}
		else
		{
			err_powr = fp[i];
		}

		// smooth the error power
		m_cpBeta2[i] *= m_fDeltaTC;
		m_cpBeta2[i] += (1-m_fDeltaTC) * m_cpBeta1[i];

		if(fCorr<0.3)
		{
			DeltaGain = 2;
		}
		else if(fCorr>=0.3 && fCorr<=0.5)
		{
			// 1 <= DeltaGain <= 2
			DeltaGain = 2 - 5 * (fCorr - 0.3);
		}
		else if(fCorr > 0.5)
		{
			// 0.2 <= DeltaGain <= 1
			DeltaGain = 1 - 1.6 * (fCorr - 0.5);
		}
		else
		{
			DeltaGain = 0.2;
		}

		m_fpDelta[i] = (m_cpBeta2[i]+ m_fpDen[i]) * DeltaGain;
		//m_fpDelta[i] += err_powr;
		//m_fpDelta[i] = m_cpBeta2[i] * DeltaGain;

#endif

		//if( m_fpDelta[i] <= 2 * m_npTaps[i] * m_fMinDelta )
		//{
		//	m_fpDelta[i] = 2 * m_npTaps[i] * m_fMinDelta;
		//}
		//if( m_fpDelta[i] > 2 * m_fMaxDelta )
		//{
		//	m_fpDelta[i] = 2 * m_fMaxDelta;
		//}

		if( m_fpDelta[i] <=  m_pDeltaFreWeight[i] )
		{
			m_fpDelta[i] =  m_pDeltaFreWeight[i];
		}
		if( m_fpDelta[i] > m_fMaxDelta )
		{
			m_fpDelta[i] = m_fMaxDelta;
		}

	}
}

void CAdapFilterGroup::SetMinMaxDelta(float min, float max)
{
	m_fMinDelta=min;
	m_fMaxDelta=max;
}

void CAdapFilterGroup::UpdateCorr()
{
	int i,indx;
	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;
		m_cpCorr[indx]   = m_fMu*(m_cpBeta1[indx]*m_cpR12sum[indx]   -	m_cpBeta1[indx+1]*m_cpR12sum[indx+1]);
		m_cpCorr[indx+1] = m_fMu*(m_cpBeta1[indx+1]*m_cpR12sum[indx] +	m_cpBeta1[indx]*m_cpR12sum[indx+1]);
	}
}

void CAdapFilterGroup::UpdateError()
{
	int i,indx;
	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;

		// calculate the normalize error
		float den = 1.0f/(m_fpR11sum[i] + m_fpDelta[i]);   // (R_m(1,1)+delta)^-1
		float normalize_error_re =  den * m_cpAdErrPre[indx];   // error_normalized=(R_m(1,1)+delta)\(error_priori)
		float normalize_error_im =  den * m_cpAdErrPre[indx+1];

		// update posteriori error to m_cpAdErr
		float factor = m_fMu * m_fpR11sum[i];
		m_cpAdErr[indx]  = m_cpAdErrPre[indx] - factor * normalize_error_re;
		m_cpAdErr[indx+1] = m_cpAdErrPre[indx+1] - factor * normalize_error_im;

		// update normalize error to m_cpAdErrPre
		m_cpAdErrPre[indx] = normalize_error_re;   
		m_cpAdErrPre[indx+1] = normalize_error_im;

		// constrain the posteriori error
		float err_powr = m_cpAdErr[indx] * m_cpAdErr[indx] + m_cpAdErr[indx+1] * m_cpAdErr[indx+1];
		float des_powr = m_cpDes[indx] * m_cpDes[indx] + m_cpDes[indx+1] * m_cpDes[indx+1];
		if(err_powr>des_powr)
		{
			m_cpAdErr[indx] = m_cpDes[indx];
			m_cpAdErr[indx+1] = m_cpDes[indx+1];
			err_powr = des_powr;
		}
		
		// update posteriori filter output to m_cpAdEst
		m_cpAdEst[indx]  = m_cpDes[indx] - m_cpAdErr[indx];
		m_cpAdEst[indx+1]= m_cpDes[indx+1] - m_cpAdErr[indx+1];

		// smooth the posteriori error power
		//m_fpDen[i] *= m_fDeltaTC;
		//m_fpDen[i] += (1.0f - m_fDeltaTC) * (err_powr);

		// update the current posteriori error power to m_cpBeta1
		m_cpBeta1[i] = err_powr;
	}
}

void CAdapFilterGroup::filter(void)
{
	int i,indx_dely,indx,j,maxtaps;	
	float temp1,temp2,temp3,temp4;
	float w_re,w_im,x_re,x_im;
	memcpy(m_cpAdErrPre,m_cpAdErr,sizeof(float)*(2*m_nNumBank));
    indx_dely=0;
	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;//x[n]
		maxtaps=m_npTaps[i];
		temp1=0.f;
		temp2=0.f;
		temp3=0.f;
		temp4=0.f;
		for (j=0;j<maxtaps;j++,indx_dely+=2)
		{
			x_re =m_cpReferDelayLine[indx_dely];
			x_im =m_cpReferDelayLine[indx_dely+1];
			//adaptive filter
			w_re =m_cpAdW[indx_dely];
			w_im =m_cpAdW[indx_dely+1];
			temp1  +=w_re*x_re   -w_im*x_im;
			temp2  +=w_re*x_im   +w_im*x_re;
			//fix filter
			w_re =m_cpFixW[indx_dely];
			w_im =m_cpFixW[indx_dely+1];
			temp3  +=w_re*x_re   -w_im*x_im;
			temp4  +=w_re*x_im   +w_im*x_re;
		}
		indx_dely+=2;
#ifdef ADAPTIVE_FILTER_ALGO_AP
		m_cpAdEst[indx]  =temp1+m_cpCorr[indx];
		m_cpAdEst[indx+1]=temp2+m_cpCorr[indx+1];
		m_cpAdErr[indx] =m_cpDes[indx]-m_cpAdEst[indx];
		m_cpAdErr[indx+1] =m_cpDes[indx+1]-m_cpAdEst[indx+1];
#elif defined ADAPTIVE_FILTER_ALGO_NLMS

		m_cpAdEst[indx]  =temp1;
		m_cpAdEst[indx+1]=temp2;

		// priori error
		m_cpAdErrPre[indx] =m_cpDes[indx]-temp1;
		m_cpAdErrPre[indx+1] =m_cpDes[indx+1]-temp2;
#endif
		m_cpFixEst[indx]=temp3;
		m_cpFixEst[indx+1]=temp4;
		m_cpFixErr[indx]=m_cpDes[indx]-m_cpFixEst[indx];
		m_cpFixErr[indx+1]=m_cpDes[indx+1]-m_cpFixEst[indx+1];		
	}
}

void CAdapFilterGroup::UpdateFilterWeight(void)
{
	int i,j,indx,indy;
	//float expDecay[];
	float est_err2_re;
	float den,beta12_re,beta12_im,beta2_re,beta2_im;
	float x_re,x_im;
	float *AttackTaps;

	indy=0;
	AttackTaps=m_fpAttackTaps;
	
	{	
		for (i=0;i<m_nNumBank;i++)	
		{ 
			indx=2*i;

			den = 1.0f/(m_fpR11sum[i]*m_fpR22sum[i] - m_cpR12sum[indx]*m_cpR12sum[indx] - m_cpR12sum[indx+1]*m_cpR12sum[indx+1]);
#ifdef ADAPTIVE_FILTER_ALGO_AP
			beta2_re   = -(m_cpR12sum[indx] * m_cpAdErr[indx] + m_cpR12sum[indx+1] *m_cpAdErr[indx+1]) * den;
			beta2_im   = -(m_cpR12sum[indx] * m_cpAdErr[indx+1] - m_cpR12sum[indx+1] * m_cpAdErr[indx]) * den;


			beta12_re          = m_cpBeta1[indx]   + beta2_re;
			beta12_im          = m_cpBeta1[indx+1] + beta2_im;


			m_cpBeta1[indx]   = m_fpR22sum[i] * m_cpAdErr[indx] * den;
			m_cpBeta1[indx+1] = m_fpR22sum[i] * m_cpAdErr[indx+1] * den;
#elif defined ADAPTIVE_FILTER_ALGO_NLMS
			beta12_re = m_cpAdErrPre[indx];
			beta12_im = m_cpAdErrPre[indx+1];
#endif

			m_cpFixBeta[indx]   = m_fpR22sum[i] * m_cpFixErr[indx] * den;
			m_cpFixBeta[indx+1] = m_fpR22sum[i] * m_cpFixErr[indx+1] * den;
		
		////////update adaptive weights		
	
			for (j=0;j<=m_npTaps[i];j++,indy+=2)
			{			
				est_err2_re=m_fMu*(*AttackTaps++);//m_fpAttackTaps[m_npTaps[i]=0!
#ifdef ADAPTIVE_FILTER_ALGO_AP
				x_re=m_cpReferDelayLine[indy+2];
				x_im=m_cpReferDelayLine[indy+3];
#elif defined ADAPTIVE_FILTER_ALGO_NLMS
				x_re=m_cpReferDelayLine[indy];
				x_im=m_cpReferDelayLine[indy+1];
#endif

				m_cpAdW[indy  ] +=   est_err2_re* (beta12_re * x_re   + beta12_im * x_im);
				m_cpAdW[indy+1] +=   est_err2_re* (beta12_im * x_re   - beta12_re * x_im);
			}
		}
	}
		
}
//endbin is the last used bin +1
void CAdapFilterGroup::Resetfilter(int startbin,int endbin)
{
	int indx;	
    memset(m_cpAdW+m_npDelaylIndx[startbin],0,sizeof(float)*(m_npDelaylIndx[endbin]-m_npDelaylIndx[startbin])) ;
    indx=endbin-startbin;
	indx*=2;
	memset(m_cpBeta1+2*startbin,0,sizeof(float)*indx);
    memset(m_cpAdEst+2*startbin,0,sizeof(float)*indx);
    memcpy(m_cpAdErr+2*startbin,m_cpDes+2*startbin,sizeof(float)*indx);
}
void CAdapFilterGroup::Adap2Fix(int startbin,int endbin)
{
	int i,indx,maxtaps,indy,j;
    float *AttackTaps;
	float tmep,beta_re,beta_im,x_re,x_im;
	indx=m_npDelaylIndx[startbin];

    AttackTaps=m_fpAttackTaps+m_npR11Indx[startbin];
	for (i=startbin;i<endbin;i++)
	{
		indy=2*i;
		maxtaps=m_npTaps[i];
		beta_re=m_cpBeta1[indy];
		beta_im=m_cpBeta1[indy+1];
		for (j=0;j<=maxtaps;j++,indx+=2)
		{
			tmep=m_fMu* (*AttackTaps++);/*m_fpAttackTaps[indy+i]*/	
			x_re=m_cpReferDelayLine[indx+2];
			x_im=m_cpReferDelayLine[indx+3];
			m_cpFixW[indx  ] = m_cpAdW[indx  ] + tmep*(beta_re*x_re + beta_im*x_im);
			m_cpFixW[indx+1] = m_cpAdW[indx+1] + tmep*(beta_im*x_re - beta_re*x_im);
		}
	}
	maxtaps=endbin-startbin;
	maxtaps*=2;
	memcpy(m_cpFixErr+2*startbin,m_cpAdErr+2*startbin,sizeof(float)*maxtaps);
    memcpy(m_cpFixEst+2*startbin,m_cpAdEst+2*startbin,sizeof(float)*maxtaps);
}
void CAdapFilterGroup::Fix2Adap(int startbin,int endbin)
{
	int i,indx,maxtaps,indy,j;
	float *AttackTaps;
	float tmep,beta_re,beta_im,x_re,x_im;
	indx=m_npDelaylIndx[startbin];
	AttackTaps=m_fpAttackTaps+m_npR11Indx[startbin];
	for (i=startbin;i<endbin;i++)
	{
		indy=2*i;
		maxtaps=m_npTaps[i];
		beta_re=m_cpBeta1[indy];
		beta_im=m_cpBeta1[indy+1];
		for (j=0;j<=maxtaps;j++,indx+=2)
		{
			tmep=m_fMu* (*AttackTaps++);/*m_fpAttackTaps[indy+i]*/	
			x_re=m_cpReferDelayLine[indx+2];
			x_im=m_cpReferDelayLine[indx+3];
			m_cpAdW[indx  ] = m_cpFixW[indx  ] + tmep*(beta_re*x_re + beta_im*x_im);
			m_cpAdW[indx+1] = m_cpFixW[indx+1] + tmep*(beta_im*x_re - beta_re*x_im);
		}
	}
	maxtaps=endbin-startbin;
	maxtaps*=2;
	memcpy(m_cpAdErr+2*startbin,m_cpFixErr+2*startbin,sizeof(float)*maxtaps);
	memcpy(m_cpAdEst+2*startbin,m_cpFixEst+2*startbin,sizeof(float)*maxtaps);
    memset(m_cpBeta1+2*startbin,0,sizeof(float)*maxtaps);

}

float CAdapFilterGroup::GetMaxAdW(void)
{

	int i,j,indx;
	float temp,temp1;
	float *fp=m_cpAdW;
	m_fMaxAdpAll=0;
	for(i=0;i<m_nNumBank;i++)
	//for(i=2;i<m_nNumBank-2;i++)/*for single band devergenc 2012.01.06*/
	{
		temp1=0;
		indx=m_npTaps[i];
		for (j=0;j<=indx;j++)
		{
			temp=*fp*(*fp)+fp[1]*fp[1];
			fp+=2;
			temp1=(temp1>temp)?temp1:temp;
		}
		m_fpMaxAdp[i]=temp1;
        m_fMaxAdpAll=(temp1>m_fMaxAdpAll)?temp1:m_fMaxAdpAll;

	}
	return m_fMaxAdpAll;
}

// processing of complex adaptive filter
//*Refer size 2*m_nNumBank
//*Des size 2*m_nNumBank
//output float *est,float err
void CAdapFilterGroup::process(const float *Refer,const float *Des,float *adest,float *err,int update_flag)
{
	    m_cpDes=Des;
		//UpdateDelta();
		UpdateDelayline(Refer);	
		SumR11_R12();
#ifdef ADAPTIVE_FILTER_ALGO_AP
		UpdateCorr();
#endif
		filter();
#ifdef ADAPTIVE_FILTER_ALGO_NLMS
		UpdateReferEnergy();
		UpdateError();
#endif
		m_nFlagcnt=(update_flag==1)?20:(m_nFlagcnt-1);
		if (m_nFlagcnt>0)
		{
			UpdateFilterWeight();
		}		
		memcpy(adest,m_cpAdEst,2*m_nNumBank*sizeof(float));		
		memcpy(err,m_cpAdErr,2*m_nNumBank*sizeof(float));

}

void CAdapFilterGroup::reset_process(int startbin ,int endbin ,short flag)
{
	if(flag ==-3)/* Adapt set to Zero */
	{
		Resetfilter(startbin,endbin);
		UpdateFilterWeight_band( startbin, endbin);
	}
	else if (flag==-1) /* Fixed copied to adapt */
	{
		Fix2Adap(startbin,endbin);
		UpdateFilterWeight_band( startbin, endbin);
	}
	if (flag==1)/* Adapt copied to fixed */
	{
		Adap2Fix(startbin,endbin);		
	}
}
void CAdapFilterGroup::ResetDelay_Taps(int indx)
{
	int len;
	len=m_npTaps[indx];
	memset(m_cpReferDelayLine+m_npDelaylIndx[indx],0,sizeof(float)*(len+1)*2);
	memset(m_cpAdW+m_npDelaylIndx[indx],0,sizeof(float)*(len+1)*2);
	memset(m_cpFixW+m_npDelaylIndx[indx],0,sizeof(float)*(len+1)*2);
	memset(m_fpR11+m_npR11Indx[indx],0,sizeof(float)*(len+1));
	memset(m_cpR12+m_npR12Indx[indx],0,sizeof(float)*(2*len));
}

void CAdapFilterGroup::MoveTapsForward(int indx,int abs_delay)
{
	int frames_to_move = m_npTaps[indx] * 2 - abs_delay * 2;
	int bytes_to_move = sizeof (float) * frames_to_move;
	
		memmove(m_cpFixW+m_npDelaylIndx[indx] + abs_delay * 2, m_cpFixW+m_npDelaylIndx[indx], bytes_to_move);
		memmove(m_cpAdW +m_npDelaylIndx[indx] + abs_delay * 2, m_cpAdW +m_npDelaylIndx[indx], bytes_to_move);

		memset (m_cpFixW+m_npDelaylIndx[indx], 0, sizeof (float) * abs_delay * 2);
		memset (m_cpAdW +m_npDelaylIndx[indx], 0, sizeof (float) * abs_delay * 2);
	
}
void CAdapFilterGroup::MoveTapsBackward(int indx,int abs_delay)
{
	int frames_to_move = m_npTaps[indx] * 2 - abs_delay * 2;
	int bytes_to_move = sizeof (float) * frames_to_move;
	
	{
		memmove(m_cpFixW+m_npDelaylIndx[indx], m_cpFixW+m_npDelaylIndx[indx] + abs_delay* 2, bytes_to_move);
		memmove(m_cpAdW +m_npDelaylIndx[indx], m_cpAdW +m_npDelaylIndx[indx] + abs_delay* 2, bytes_to_move);

		memset (m_cpFixW+m_npDelaylIndx[indx] +frames_to_move- 1, 0, sizeof (float) * abs_delay * 2);
		memset (m_cpAdW +m_npDelaylIndx[indx] +frames_to_move- 1, 0, sizeof (float) * abs_delay * 2);
	}	
}
void CAdapFilterGroup::UpdateDelaylineInvers(const float *newRefer)
{

	//int maxtaps=2*m_npTaps[];
	int i,indx,indy;
	memmove(m_cpReferDelayLine, m_cpReferDelayLine + 2, (m_nSumLenDelLine-2)*sizeof(float));

	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;
		indy=m_npDelaylIndx[i+1]-2;
		*(m_cpReferDelayLine+indy)  =*(newRefer+indx);
		*(m_cpReferDelayLine+indy+1)=*(newRefer+indx+1);
	}

	UpdateR11_R12Invers(newRefer);//update R11 R12

}
void CAdapFilterGroup::UpdateR11_R12Invers(const float *newRefer)
{
	int indx;
	int i;
	const float *fpr=newRefer;
	////update R11
	memmove(m_fpR11,m_fpR11+1,(m_nSumLenR11-1)*sizeof(float));
 	////update R12	
	memmove(m_cpR12,m_cpR12+2,(m_nSumLenR12-2)*sizeof(float));
	for (i=0;i<m_nNumBank-1;i++)
	{		
		////update R11
		*(m_fpR11+m_npR11Indx[i+1]-1)=fpr[0]*fpr[0]+fpr[1]*fpr[1];
		////update R12	
		indx=m_npDelaylIndx[i+1]-4;
		*(m_cpR12+m_npR12Indx[i+1]-2)=m_cpReferDelayLine[indx]*fpr[0]+m_cpReferDelayLine[indx+1]*fpr[1];
		*(m_cpR12+m_npR12Indx[i+1]-1)=m_cpReferDelayLine[indx+1]*fpr[0]-m_cpReferDelayLine[indx]*fpr[1];
		fpr+=2;
	}
	//fpr+=2;
	*(m_fpR11+m_nSumLenR11-1)=fpr[0]*fpr[0]+fpr[1]*fpr[1];
	indx=m_npDelaylIndx[i+1]-4;
	*(m_cpR12+m_nSumLenR12-2)=m_cpReferDelayLine[indx]*fpr[0]+m_cpReferDelayLine[indx+1]*fpr[1];
	*(m_cpR12+m_nSumLenR12-1)=m_cpReferDelayLine[indx+1]*fpr[0]-m_cpReferDelayLine[indx]*fpr[1];

}
void CAdapFilterGroup::SumR11_R12()
{
	int i,j;
	int indx;
	float r11;
	float r22;
	float r12_re;
	float r12_im;
	float *fpr11;
	float *fpr22;
	float *fpr12;
	float *AttackTaps;
	float temp;

	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;
		r11=0.f;
		r22=0.f;
		r12_re=0.f;
		r12_im=0.f;
		fpr11=m_fpR11+m_npR11Indx[i];
		fpr22=fpr11+1;
		fpr12=m_cpR12+m_npR12Indx[i];
		AttackTaps=m_fpAttackTaps+m_npR11Indx[i];

		for (j=0;j<m_npTaps[i];j++)
		{
			temp=*AttackTaps;
			r11		+= temp * (*(fpr11++));
			r12_re	+= temp * (*(fpr12++));
			r12_im	+= temp * (*(fpr12++));
			r22		+= temp * (*(fpr22++));
			AttackTaps++;
		}

#ifdef ADAPTIVE_FILTER_ALGO_AP
		m_fpR11sum[i]=r11+m_fpDelta[i];
		m_fpR22sum[i]=r22+m_fpDelta[i];
#elif defined ADAPTIVE_FILTER_ALGO_NLMS
		m_fpR11sum[i]=r11;
		m_fpR22sum[i]=r22;
#endif
		m_cpR12sum[indx]  =r12_re;
		m_cpR12sum[indx+1]=r12_im;
	}
}

void CAdapFilterGroup::UpdateFilterWeight_band(int startbin,int endbin)
{
	int i,j,indx,indy;
	//float expDecay[];
	float est_err2_re;
	float den,beta12_re,beta12_im,beta2_re,beta2_im;
	float x_re,x_im;
	float *AttackTaps;
	//memset(m_fpExpDecay,1,sizeof(float)*m_nNumBank);
	indy=m_npDelaylIndx[startbin];
	AttackTaps=m_fpAttackTaps+m_npR11Indx[startbin];

	{	
		for (i=startbin;i<endbin;i++)	
		{ 
			indx=2*i;
			den = 1.0f/(m_fpR11sum[i]*m_fpR22sum[i] - m_cpR12sum[indx]*m_cpR12sum[indx] - m_cpR12sum[indx+1]*m_cpR12sum[indx+1]);

			beta2_re   = -(m_cpR12sum[indx] * m_cpAdErr[indx] + m_cpR12sum[indx+1] *m_cpAdErr[indx+1]) * den;
			beta2_im   = -(m_cpR12sum[indx] * m_cpAdErr[indx+1] - m_cpR12sum[indx+1] * m_cpAdErr[indx]) * den;

			beta12_re          = m_cpBeta1[indx]   + beta2_re;
			beta12_im          = m_cpBeta1[indx+1] + beta2_im;


			m_cpBeta1[indx]   = m_fpR22sum[i] * m_cpAdErr[indx] * den;
			m_cpBeta1[indx+1] = m_fpR22sum[i] * m_cpAdErr[indx+1] * den;


			m_cpFixBeta[indx]   = m_fpR22sum[i] * m_cpFixErr[indx] * den;
			m_cpFixBeta[indx+1] = m_fpR22sum[i] * m_cpFixErr[indx+1] * den;

			////////update adaptive weights		

			for (j=0;j<=m_npTaps[i];j++,indy+=2)
			{			
				est_err2_re=(*AttackTaps++);//m_fpAttackTaps[m_npTaps[i]=0!
				x_re=m_cpReferDelayLine[indy+2];
				x_im=m_cpReferDelayLine[indy+3];

				m_cpAdW[indy  ] +=   est_err2_re* (beta12_re * x_re   + beta12_im * x_im);
				m_cpAdW[indy+1] +=   est_err2_re* (beta12_im * x_re   - beta12_re * x_im);
			}
		}
	}

}
void CAdapFilterGroup::UpdateStep(float fCorr)
{
	if(20==m_nFlagcnt)
	{
		float tempf=2*(sqrtf(fCorr)-0.2);
		tempf=tempf>1.f?1.f:tempf;
		tempf=tempf<5e-7f?0.0f:tempf;

		m_fMu*=0.95f;
		m_fMu+=0.05f*tempf;
		
	}
	
	
}