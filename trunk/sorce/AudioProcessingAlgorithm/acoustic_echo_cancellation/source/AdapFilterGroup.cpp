

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#if defined (ARM_NEON)
#include <arm_neon.h>
#endif
#include <vector>
#include "AdapFilterGroup.h"
CAdapFilterGroup::CAdapFilterGroup(int numbank,int *ntaps, float mu, float delat_gain)
{
	m_nNumBank=numbank;
    m_npTaps=ntaps;
	m_fMu =mu > 0.000001f ? mu : 0.8f;
	m_deltagin = delat_gain >= 0.f? delat_gain:0.f;
#if defined (ARM_NEON)
    neon_on_ = true;
#else
    neon_on_ = false;
#endif
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
	SetMinMaxDelta(5e-8f, 0.1);
	//
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
	m_fpAdEstPwr  = m_fpR22sum       +m_nNumBank;
    m_fpDen       = m_fpAdEstPwr     +m_nNumBank;
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
    //memmove(m_cpR12+2,m_cpR12,(m_nSumLenR12-2)*sizeof(float));
    if(neon_on_){
#if defined (ARM_NEON)
        std::vector<float> r11(m_nNumBank);
        for (i=0;i<m_nNumBank/4;i++)
        {
            indx=8*i;

            float32x4x2_t float_ref = vld2q_f32(newRefer+indx);
            const float32x4_t a = vmulq_f32(float_ref.val[0], float_ref.val[0]);
            const float32x4_t e = vmlaq_f32(a, float_ref.val[1], float_ref.val[1]);

            vst1q_f32(&r11[0], e);

            *(m_fpR11+*(m_npR11Indx+4*i)) = r11[0];
            *(m_fpR11+*(m_npR11Indx+4*i + 1)) = r11[1];
            *(m_fpR11+*(m_npR11Indx+4*i + 2)) = r11[2];
            *(m_fpR11+*(m_npR11Indx+4*i + 3)) = r11[3];

            //*(m_fpR11+*(m_npR11Indx+i))  =newRefer[indx]*newRefer[indx]+newRefer[indx+1]*newRefer[indx+1];

        }
#endif
    }else{
        for (i=0;i<m_nNumBank;i++)
        {
            indx=2*i;
            indy=*(i+m_npDelaylIndx);
            *(m_fpR11+*(m_npR11Indx+i))  =newRefer[indx]*newRefer[indx]+newRefer[indx+1]*newRefer[indx+1];
            // *(m_cpR12+*(m_npR12Indx+i))  = *(m_cpReferDelayLine+indy+2)*newRefer[indx]+*(m_cpReferDelayLine+indy+3)*newRefer[indx+1];
            // *(m_cpR12+*(m_npR12Indx+i)+1)=-*(m_cpReferDelayLine+indy+3)*newRefer[indx]+*(m_cpReferDelayLine+indy+2)*newRefer[indx+1];
        }
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
	//float tempf=m_fDeltaTC;
	int ind5k = 102;
	for (i=0;i<m_nNumBank;i++)
	{
		indx=2*i;
	
		if(NULL==fp)
		{
			err_powr = 0.f;
		}
		else
		{
			err_powr = fp[i];
		}

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
		float den_gain = 0.5f;
		if (i > ind5k) den_gain = 0.1;
		m_fpDelta[i] = (m_cpBeta2[i]*3* DeltaGain + den_gain*m_fpDen[i]) * DeltaGain*m_deltagin;


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

		float op_step = 1.f;
		//if (m_cpBeta1[i + m_nNumBank] > 5.f * m_cpBeta2[i])
		//{
		//	op_step = fabs(1 - sqrt(fabs(m_cpBeta1[i + m_nNumBank] - m_fpAdEstPwr[i]) / (m_cpBeta2[i] + 1e-8)));
		//	op_step = op_step > 1.f ? 1.f : op_step;
		//}
		// calculate the normalize error
		float den = op_step /(m_fpR11sum[i] +m_fpDelta[i]);   // (R_m(1,1)+delta)^-1
		float normalize_error_re =  den * m_cpAdErrPre[indx];   // error_normalized=(R_m(1,1)+delta)\(error_priori)
		float normalize_error_im =  den * m_cpAdErrPre[indx+1];

		// update posteriori error to m_cpAdErr
		float factor = m_fMu * m_fpR11sum[i];
		m_cpAdErr[indx]  = m_cpAdErrPre[indx] - factor * normalize_error_re;
		m_cpAdErr[indx+1] = m_cpAdErrPre[indx+1] - factor * normalize_error_im;


		// constrain the posteriori error
		float err_powr = m_cpAdErr[indx] * m_cpAdErr[indx] + m_cpAdErr[indx+1] * m_cpAdErr[indx+1];
		float des_powr = m_cpDes[indx] * m_cpDes[indx] + m_cpDes[indx+1] * m_cpDes[indx+1];
		if(err_powr>des_powr)
		{

			float tmp1= m_cpAdErrPre[indx] - factor/10 * normalize_error_re;
			float tmp2= m_cpAdErrPre[indx + 1] - factor/10 * normalize_error_im;

			// constrain the posteriori error
			float err_powr2 = tmp1 * tmp1 + tmp2 * tmp2;
			if (err_powr2 < des_powr) {
				m_cpAdErr[indx] = tmp1;
				m_cpAdErr[indx + 1] = tmp2;
				err_powr = err_powr2;
			}
			else {
				m_cpAdErr[indx] = m_cpDes[indx];
				m_cpAdErr[indx + 1] = m_cpDes[indx + 1];
				err_powr = des_powr;

			}
		}

		// update normalize error to m_cpAdErrPre

		m_cpAdErrPre[indx] = normalize_error_re ;
		m_cpAdErrPre[indx + 1] = normalize_error_im;

		// update posteriori filter output to m_cpAdEst
		m_cpAdEst[indx]  = m_cpDes[indx] - m_cpAdErr[indx];
		m_cpAdEst[indx+1]= m_cpDes[indx+1] - m_cpAdErr[indx+1];

		// update the current posteriori error power to m_cpBeta1
		m_cpBeta1[i] = err_powr;

		// smooth the error power
		m_cpBeta2[i] *= m_fDeltaTC;
		m_cpBeta2[i] += (1 - m_fDeltaTC) * m_cpBeta1[i];

		// smooth the des power
		m_cpBeta1[i + m_nNumBank] *= m_fDeltaTC;
		m_cpBeta1[i + m_nNumBank] += (1 - m_fDeltaTC) * des_powr;
		// smooth the est power
		m_fpAdEstPwr[i] *= m_fDeltaTC;
		m_fpAdEstPwr[i] += (1 - m_fDeltaTC) * (m_cpAdEst[indx]* m_cpAdEst[indx] + m_cpAdEst[indx+1]* m_cpAdEst[indx+1]);
	}
}

void CAdapFilterGroup::filter(void)
{
    int i,indx_dely,indx,j,maxtaps;	
	float temp1,temp2,temp3,temp4;
	float w_re,w_im,x_re,x_im;
	memcpy(m_cpAdErrPre,m_cpAdErr,sizeof(float)*(2*m_nNumBank));
    indx_dely=0;
    
    if(neon_on_){
#if defined (ARM_NEON)
        //printf("filter using neon! \n");
        std::vector<float> s_re(m_nNumBank);
        std::vector<float> s_im(m_nNumBank);
        
        for (i=0;i<m_nNumBank;i++)
        {
            indx=2*i;//x[n]
            maxtaps=m_npTaps[i];
            temp1=0.f;
            temp2=0.f;
            temp3=0.f;
            temp4=0.f;
            float32x4_t g = { 0.0f, 0.0f, 0.0f, 0.0f };
            float32x4_t h = { 0.0f, 0.0f, 0.0f, 0.0f };
            for (j=0;j<maxtaps/4;j++,indx_dely+=8)
            {
                // const float32x4_t X_re = vld1q_f32(&X->re[k]);
                // const float32x4_t X_im = vld1q_f32(&X->im[k]);
                // const float32x4_t H_re = vld1q_f32(&H_j->re[k]);
                // const float32x4_t H_im = vld1q_f32(&H_j->im[k]);

                float32x4x2_t float_ref = vld2q_f32(m_cpReferDelayLine+indx_dely);
                float32x4x2_t  float_h = vld2q_f32(m_cpAdW+indx_dely);

                const float32x4_t a = vmulq_f32(float_ref.val[0], float_h.val[0]);
                const float32x4_t e = vmlsq_f32(a, float_ref.val[1], float_h.val[1]);
                const float32x4_t c = vmulq_f32(float_ref.val[0], float_h.val[1]);
                const float32x4_t f = vmlaq_f32(c, float_ref.val[1], float_h.val[0]);

                g = vaddq_f32(g, e);
                h = vaddq_f32(h, f);
                // vst1q_f32(&s_re[i], vaddvq_f32(g));
                // vst1q_f32(&s_im[i], vaddvq_f32(h));
            }

            indx_dely+=2;

            vst1q_f32(&s_re[0], g);
            vst1q_f32(&s_im[0], h);
            m_cpAdEst[indx] = s_re[0] + s_re[1] + s_re[2] + s_re[3];
            m_cpAdEst[indx + 1] = s_im[0] + s_im[1] + s_im[2] + s_im[3];

            // priori error
            m_cpAdErrPre[indx] =m_cpDes[indx]-m_cpAdEst[indx];
            m_cpAdErrPre[indx+1] =m_cpDes[indx+1]-m_cpAdEst[indx+1];
        }
#endif
    }else{
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
            /*	w_re =m_cpFixW[indx_dely];
                w_im =m_cpFixW[indx_dely+1];
                temp3  +=w_re*x_re   -w_im*x_im;
                temp4  +=w_re*x_im   +w_im*x_re;*/
            }
            indx_dely+=2;
            m_cpAdEst[indx]  =temp1;
            m_cpAdEst[indx+1]=temp2;

            // priori error
            m_cpAdErrPre[indx] =m_cpDes[indx]-temp1;
            m_cpAdErrPre[indx+1] =m_cpDes[indx+1]-temp2;

        }
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
#if defined(ADF_DEBUG)
    if (ADFW == nullptr) {
        ADFW = fopen("adfw_1016-13.txt","wb");
    }
#endif
    counter_++;

	indy=0;	
    if(neon_on_){
#if defined (ARM_NEON)
        //printf("UpdateFilterWeight using neon! \n");
        std::vector<float> w_re(m_nNumBank);
        std::vector<float> w_im(m_nNumBank);
		for (i=0;i<m_nNumBank/4;i++)	
		{ 
			indx=8*i;
			// beta12_re = m_cpAdErrPre[indx];
			// beta12_im = m_cpAdErrPre[indx+1];

            float32x4x2_t  float_e = vld2q_f32(m_cpAdErrPre+indx);
            float32x4_t g = { 0.0f, 0.0f, 0.0f, 0.0f };
            float32x4_t h = { 0.0f, 0.0f, 0.0f, 0.0f };

		    ////////update adaptive weights		
			for (j=0;j<=m_npTaps[i];j++,indy+=8)
			{			
				// x_re=m_cpReferDelayLine[indy];
				// x_im=m_cpReferDelayLine[indy+1];
                float32x4x2_t float_ref = vld2q_f32(m_cpReferDelayLine+indy);
                const float32x4_t a = vmulq_f32(float_ref.val[0], float_e.val[0]);
                const float32x4_t e = vmlaq_f32(a, float_ref.val[1], float_e.val[1]);
                const float32x4_t c = vmulq_f32(float_ref.val[0], float_e.val[1]);
                const float32x4_t f = vmlsq_f32(c, float_ref.val[1], float_e.val[0]);

                g = vaddq_f32(g, e);
                h = vaddq_f32(h, f);
                vst1q_f32(&w_re[0], g);
                vst1q_f32(&w_im[0], h);
                m_cpAdW[indy] = m_fMu * w_re[0];
                m_cpAdW[indy + 1] = m_fMu * w_im[0];
                m_cpAdW[indy + 2] = m_fMu * w_re[1];
                m_cpAdW[indy + 3] = m_fMu * w_im[1];
                m_cpAdW[indy + 4] = m_fMu * w_re[2];
                m_cpAdW[indy + 5] = m_fMu * w_im[2];
                m_cpAdW[indy + 6] = m_fMu * w_re[3];
                m_cpAdW[indy + 7] = m_fMu * w_im[3];
				// m_cpAdW[indy  ] += m_fMu * (beta12_re * x_re   + beta12_im * x_im);
				// m_cpAdW[indy+1] += m_fMu * (beta12_im * x_re   - beta12_re * x_im);           
			}
		}
#endif
    }else {	
		for (i=0;i<m_nNumBank;i++)	
		{ 
			indx=2*i;
			//den = 1.0f/(m_fpR11sum[i]*m_fpR22sum[i] - m_cpR12sum[indx]*m_cpR12sum[indx] - m_cpR12sum[indx+1]*m_cpR12sum[indx+1]);
			beta12_re = m_cpAdErrPre[indx];
			beta12_im = m_cpAdErrPre[indx+1];
		////////update adaptive weights			
			for (j=0;j<=m_npTaps[i];j++,indy+=2)
			{			
				//est_err2_re = m_fMu;
				x_re=m_cpReferDelayLine[indy];
				x_im=m_cpReferDelayLine[indy+1];

				m_cpAdW[indy  ] += m_fMu * (beta12_re * x_re   + beta12_im * x_im);
				m_cpAdW[indy+1] += m_fMu * (beta12_im * x_re   - beta12_re * x_im);
#if defined(ADF_DEBUG)
                if (counter_ > 920 && counter_ < 1420 && i == 13) {
                    fprintf(ADFW, "%d %d %d %f %f %f %f %f %f %f\n", counter_,i,j, m_cpAdW[indy], m_cpAdW[indy+1], m_fMu, beta12_re, beta12_im, x_re, x_im);
                }
#endif            
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
//void CAdapFilterGroup::Adap2Fix(int startbin,int endbin)
//{
//	int i,indx,maxtaps,indy,j;
//    float *AttackTaps;
//	float tmep,beta_re,beta_im,x_re,x_im;
//	indx=m_npDelaylIndx[startbin];
//
//	for (i=startbin;i<endbin;i++)
//	{
//		indy=2*i;
//		maxtaps=m_npTaps[i];
//		beta_re=m_cpBeta1[indy];
//		beta_im=m_cpBeta1[indy+1];
//		for (j=0;j<=maxtaps;j++,indx+=2)
//		{
//			tmep = m_fMu;
//			x_re=m_cpReferDelayLine[indx+2];
//			x_im=m_cpReferDelayLine[indx+3];
//			m_cpFixW[indx  ] = m_cpAdW[indx  ] + tmep*(beta_re*x_re + beta_im*x_im);
//			m_cpFixW[indx+1] = m_cpAdW[indx+1] + tmep*(beta_im*x_re - beta_re*x_im);
//		}
//	}
//	maxtaps=endbin-startbin;
//	maxtaps*=2;
//	memcpy(m_cpFixErr+2*startbin,m_cpAdErr+2*startbin,sizeof(float)*maxtaps);
//    memcpy(m_cpFixEst+2*startbin,m_cpAdEst+2*startbin,sizeof(float)*maxtaps);
//}
//void CAdapFilterGroup::Fix2Adap(int startbin,int endbin)
//{
//	int i,indx,maxtaps,indy,j;
//	float *AttackTaps;
//	float tmep,beta_re,beta_im,x_re,x_im;
//	indx=m_npDelaylIndx[startbin];
//	for (i=startbin;i<endbin;i++)
//	{
//		indy=2*i;
//		maxtaps=m_npTaps[i];
//		beta_re=m_cpBeta1[indy];
//		beta_im=m_cpBeta1[indy+1];
//		for (j=0;j<=maxtaps;j++,indx+=2)
//		{
//			x_re=m_cpReferDelayLine[indx+2];
//			x_im=m_cpReferDelayLine[indx+3];
//			m_cpAdW[indx  ] = m_cpFixW[indx  ] + m_fMu *(beta_re*x_re + beta_im*x_im);
//			m_cpAdW[indx+1] = m_cpFixW[indx+1] + m_fMu *(beta_im*x_re - beta_re*x_im);
//		}
//	}
//	maxtaps=endbin-startbin;
//	maxtaps*=2;
//	memcpy(m_cpAdErr+2*startbin,m_cpFixErr+2*startbin,sizeof(float)*maxtaps);
//	memcpy(m_cpAdEst+2*startbin,m_cpFixEst+2*startbin,sizeof(float)*maxtaps);
//    memset(m_cpBeta1+2*startbin,0,sizeof(float)*maxtaps);
//
//}

//float CAdapFilterGroup::GetMaxAdW(void)
//{
//
//	int i,j,indx;
//	float temp,temp1;
//	float *fp=m_cpAdW;
//	m_fMaxAdpAll=0;
//	for(i=0;i<m_nNumBank;i++)
//	{
//		temp1=0;
//		indx=m_npTaps[i];
//		for (j=0;j<=indx;j++)
//		{
//			temp=*fp*(*fp)+fp[1]*fp[1];
//			fp+=2;
//			temp1=(temp1>temp)?temp1:temp;
//		}
//		m_fpMaxAdp[i]=temp1;
//        m_fMaxAdpAll=(temp1>m_fMaxAdpAll)?temp1:m_fMaxAdpAll;
//
//	}
//	return m_fMaxAdpAll;
//}

// processing of complex adaptive filter
//*Refer size 2*m_nNumBank
//*Des size 2*m_nNumBank
//output float *est,float err
void CAdapFilterGroup::process(const float *Refer,const float *Des,float *adest,float *err,int update_flag)
{
	    m_cpDes=Des;
		UpdateDelayline(Refer);	
		SumR11_R12();
		filter();
		UpdateReferEnergy();
		UpdateError();
		m_nFlagcnt=(update_flag==1)?20:(m_nFlagcnt-1);
		if (m_nFlagcnt>0)
		{
			UpdateFilterWeight();
		}		
		memcpy(adest,m_cpAdEst,2*m_nNumBank*sizeof(float));		
		memcpy(err,m_cpAdErr,2*m_nNumBank*sizeof(float));

}
//
//void CAdapFilterGroup::reset_process(int startbin ,int endbin ,short flag)
//{
//	if(flag ==-3)/* Adapt set to Zero */
//	{
//		Resetfilter(startbin,endbin);
//		UpdateFilterWeight_band( startbin, endbin);
//	}
//	else if (flag==-1) /* Fixed copied to adapt */
//	{
//		Fix2Adap(startbin,endbin);
//		UpdateFilterWeight_band( startbin, endbin);
//	}
//	if (flag==1)/* Adapt copied to fixed */
//	{
//		Adap2Fix(startbin,endbin);		
//	}
//}
void CAdapFilterGroup::ResetDelay_Taps(int indx)
{
	int len;
	len=m_npTaps[indx];
	memset(m_cpReferDelayLine+m_npDelaylIndx[indx],0,sizeof(float)*(len+1)*2);
	memset(m_cpAdW+m_npDelaylIndx[indx],0,sizeof(float)*(len+1)*2);
	memset(m_cpFixW+m_npDelaylIndx[indx],0,sizeof(float)*(len+1)*2);
	memset(m_fpR11+m_npR11Indx[indx],0,sizeof(float)*(len+1));
	// memset(m_cpR12+m_npR12Indx[indx],0,sizeof(float)*(2*len));
}
//
//void CAdapFilterGroup::MoveTapsForward(int indx,int abs_delay)
//{
//	int frames_to_move = m_npTaps[indx] * 2 - abs_delay * 2;
//	int bytes_to_move = sizeof (float) * frames_to_move;
//	
//		memmove(m_cpFixW+m_npDelaylIndx[indx] + abs_delay * 2, m_cpFixW+m_npDelaylIndx[indx], bytes_to_move);
//		memmove(m_cpAdW +m_npDelaylIndx[indx] + abs_delay * 2, m_cpAdW +m_npDelaylIndx[indx], bytes_to_move);
//
//		memset (m_cpFixW+m_npDelaylIndx[indx], 0, sizeof (float) * abs_delay * 2);
//		memset (m_cpAdW +m_npDelaylIndx[indx], 0, sizeof (float) * abs_delay * 2);
//	
//}
//void CAdapFilterGroup::MoveTapsBackward(int indx,int abs_delay)
//{
//	int frames_to_move = m_npTaps[indx] * 2 - abs_delay * 2;
//	int bytes_to_move = sizeof (float) * frames_to_move;
//	
//	{
//		memmove(m_cpFixW+m_npDelaylIndx[indx], m_cpFixW+m_npDelaylIndx[indx] + abs_delay* 2, bytes_to_move);
//		memmove(m_cpAdW +m_npDelaylIndx[indx], m_cpAdW +m_npDelaylIndx[indx] + abs_delay* 2, bytes_to_move);
//
//		memset (m_cpFixW+m_npDelaylIndx[indx] +frames_to_move- 1, 0, sizeof (float) * abs_delay * 2);
//		memset (m_cpAdW +m_npDelaylIndx[indx] +frames_to_move- 1, 0, sizeof (float) * abs_delay * 2);
//	}	
//}
//void CAdapFilterGroup::UpdateDelaylineInvers(const float *newRefer)
//{
//
//	int i,indx,indy;
//	memmove(m_cpReferDelayLine, m_cpReferDelayLine + 2, (m_nSumLenDelLine-2)*sizeof(float));
//
//	for (i=0;i<m_nNumBank;i++)
//	{
//		indx=2*i;
//		indy=m_npDelaylIndx[i+1]-2;
//		*(m_cpReferDelayLine+indy)  =*(newRefer+indx);
//		*(m_cpReferDelayLine+indy+1)=*(newRefer+indx+1);
//	}
//
//	UpdateR11_R12Invers(newRefer);//update R11 R12
//
//}
//
//void CAdapFilterGroup::UpdateR11_R12Invers(const float *newRefer)
//{
//	int indx;
//	int i;
//	const float *fpr=newRefer;
//	////update R11
//	memmove(m_fpR11,m_fpR11+1,(m_nSumLenR11-1)*sizeof(float));
// 	////update R12	
//	memmove(m_cpR12,m_cpR12+2,(m_nSumLenR12-2)*sizeof(float));
//	for (i=0;i<m_nNumBank-1;i++)
//	{		
//		////update R11
//		*(m_fpR11+m_npR11Indx[i+1]-1)=fpr[0]*fpr[0]+fpr[1]*fpr[1];
//		////update R12	
//		indx=m_npDelaylIndx[i+1]-4;
//		*(m_cpR12+m_npR12Indx[i+1]-2)=m_cpReferDelayLine[indx]*fpr[0]+m_cpReferDelayLine[indx+1]*fpr[1];
//		*(m_cpR12+m_npR12Indx[i+1]-1)=m_cpReferDelayLine[indx+1]*fpr[0]-m_cpReferDelayLine[indx]*fpr[1];
//		fpr+=2;
//	}
//	//fpr+=2;
//	*(m_fpR11+m_nSumLenR11-1)=fpr[0]*fpr[0]+fpr[1]*fpr[1];
//	indx=m_npDelaylIndx[i+1]-4;
//	*(m_cpR12+m_nSumLenR12-2)=m_cpReferDelayLine[indx]*fpr[0]+m_cpReferDelayLine[indx+1]*fpr[1];
//	*(m_cpR12+m_nSumLenR12-1)=m_cpReferDelayLine[indx+1]*fpr[0]-m_cpReferDelayLine[indx]*fpr[1];
//
//}
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
		// r22=0.f;
		// r12_re=0.f;
		// r12_im=0.f;
		fpr11=m_fpR11+m_npR11Indx[i];
		// fpr22=fpr11+1;
		// fpr12=m_cpR12+m_npR12Indx[i];

		for (j=0;j<m_npTaps[i];j++)
		{
			r11		+= (*(fpr11++));
			// r12_re	+=  (*(fpr12++));
			// r12_im	+=  (*(fpr12++));
			// r22		+=  (*(fpr22++));
		}

		m_fpR11sum[i]=r11;
		// m_fpR22sum[i]=r22;

		// m_cpR12sum[indx]  =r12_re;
		// m_cpR12sum[indx+1]=r12_im;
	}
}
//
//void CAdapFilterGroup::UpdateFilterWeight_band(int startbin,int endbin)
//{
//	int i,j,indx,indy;
//	float est_err2_re;
//	float den,beta12_re,beta12_im,beta2_re,beta2_im;
//	float x_re,x_im;
//	float *AttackTaps;
//
//	indy=m_npDelaylIndx[startbin];
//
//	{	
//		for (i=startbin;i<endbin;i++)	
//		{ 
//			indx=2*i;
//			den = 1.0f/(m_fpR11sum[i]*m_fpR22sum[i] - m_cpR12sum[indx]*m_cpR12sum[indx] - m_cpR12sum[indx+1]*m_cpR12sum[indx+1]);
//
//			beta2_re   = -(m_cpR12sum[indx] * m_cpAdErr[indx] + m_cpR12sum[indx+1] *m_cpAdErr[indx+1]) * den;
//			beta2_im   = -(m_cpR12sum[indx] * m_cpAdErr[indx+1] - m_cpR12sum[indx+1] * m_cpAdErr[indx]) * den;
//
//			beta12_re          = m_cpBeta1[indx]   + beta2_re;
//			beta12_im          = m_cpBeta1[indx+1] + beta2_im;
//
//
//			m_cpBeta1[indx]   = m_fpR22sum[i] * m_cpAdErr[indx] * den;
//			m_cpBeta1[indx+1] = m_fpR22sum[i] * m_cpAdErr[indx+1] * den;
//
//
//			m_cpFixBeta[indx]   = m_fpR22sum[i] * m_cpFixErr[indx] * den;
//			m_cpFixBeta[indx+1] = m_fpR22sum[i] * m_cpFixErr[indx+1] * den;
//
//			////////update adaptive weights		
//
//			for (j=0;j<=m_npTaps[i];j++,indy+=2)
//			{			
//				x_re=m_cpReferDelayLine[indy+2];
//				x_im=m_cpReferDelayLine[indy+3];
//
//				m_cpAdW[indy  ] +=  (beta12_re * x_re   + beta12_im * x_im);
//				m_cpAdW[indy+1] +=  (beta12_im * x_re   - beta12_re * x_im);
//			}
//		}
//	}
//
//}

//void CAdapFilterGroup::UpdateStep(float fCorr)
//{
//	if(20==m_nFlagcnt)
//	{
//		float tempf=2*(sqrtf(fCorr)-0.2);
//		tempf=tempf>1.f?1.f:tempf;
//		tempf=tempf<5e-7f?0.0f:tempf;
//
//		m_fMu*=0.95f;
//		m_fMu+=0.05f*tempf;
//		
//	}
//	
//	
//}