/***********************************************************************
 *  Author
;*      Gao Hua
;*      
;*     
;*
;*  History
;*      10/16/2014 Created
;*
;*
;*************************************************************************/
#include "delayest.h"

#include <memory.h>
#include <stdio.h>
#include <math.h>

#define DELAY_ESTIMATION_MIN_LSPOWER (3.8147e-6f)
#define DECISION_THRESHOLD (0.21)
#define MIN_POWER  (0.00000002)
#define SEARCH_THRESHOLD (DECISION_THRESHOLD*0.75)
#define LOCAL_WINDOW (5)

CDTDetector::CDTDetector(int Fs,int Winlen,int StarBin,int EndBin,int MaxDely,float framelen_timeUms,float updateCoeff, bool isExponentialCorrelation):
m_fDecisionThreshold(DECISION_THRESHOLD),
m_fDecisionThresholdTemp(DECISION_THRESHOLD),
m_fSerachThreashold(SEARCH_THRESHOLD)

{
	//init
	m_nWinlen=Winlen;         //length of calculate 

	m_nMaxDelay=MaxDely;       //frame number responding to max delay time 
	m_nStarBin=StarBin;        //begin band
	m_nEndBin=EndBin;         //end band
	m_nBins=m_nEndBin-m_nStarBin+1;
	m_nFarBufferLen=MaxDely+Winlen;   //length of far data buffer
	m_nBufCorrLen=int(1000.f/framelen_timeUms);  //1s   //length of buffering correlation
	m_nSortLen=5;       //max 5 values //length of sort buffer
	//m_nIndFarBufer=0;    
	m_nVADTail=00;        //200ms
	m_nOffset=0;
	/////////////////////////////////
	//Memory init
	m_pnCoverTime=new int[m_nSortLen];
	m_nMemfloatsum=(m_nBins*2*m_nFarBufferLen)+MaxDely+(m_nBins*2*m_nWinlen)+m_nSortLen+m_nBufCorrLen;
	m_pfFarPwr  =new float[m_nMemfloatsum];                //power array of far data, length is m_nBufCorrLen
	m_pfcFarBuf =m_pfFarPwr  + MaxDely;                      //bins x m_nFarBufferLen array, 2i real, 2i+1 imag; sort: line1, line2,...
	m_pfcNearBuf=m_pfcFarBuf + (m_nBins*2*m_nFarBufferLen);  //bins x m_nWinlen array, 2i real, 2i+1 imag; sort: line1, line2,...
	m_pfCorr    =m_pfcNearBuf+ (m_nBins*2*m_nWinlen);   
	m_pfCorSort =m_pfCorr    + m_nBufCorrLen;
	// index init

	m_pfCorrInd=m_pfCorr;

	m_fLameta = updateCoeff;
	m_fLametaN = 1;
	for (int i = 0; i < Winlen-1; ++i)
	{
		m_fLametaN *= m_fLameta;
	}
	m_fLametaN = -m_fLametaN;

	m_pfCrossProduct = new ComplexData_t[MaxDely];

	memset(m_pfCrossProduct, 0, sizeof(ComplexData_t)*MaxDely);
	memset(m_pfFarPwr, 0, m_nMemfloatsum*sizeof(float));
	memset(m_pnCoverTime, 0, m_nSortLen*sizeof(int));

	m_fNearPwr = 0;
	m_fNearPwrUpdate = 0;
	m_fFarPwrUpdate = 0;
	m_bIsExponentialCorrelation = isExponentialCorrelation;
	m_nIsVoice = 0;
	// reset buffer
	DTDReset();


}
CDTDetector::~CDTDetector()
{

	if (m_pfFarPwr)
	{
		delete m_pfFarPwr;
	}
	if (m_pnCoverTime)
	{
		delete m_pnCoverTime;
	}
	if (m_pfCrossProduct)
	{
		delete m_pfCrossProduct;
	}
}
int CDTDetector::DTDReset()
{
	m_fDecisionThreshold=(DECISION_THRESHOLD);
	m_fDecisionThresholdTemp=(DECISION_THRESHOLD);
	m_fSerachThreashold=(SEARCH_THRESHOLD);
	m_fNearPwr=0.f;
	m_pFarBufInd=m_pfcFarBuf;
	m_pfNearBufInd=m_pfcNearBuf;
	m_nVADCount=0;
	m_nDelay=m_nOffset;
	if (m_pfFarPwr)
	{
		memset(m_pfFarPwr,0,sizeof(float)*m_nMemfloatsum);
	}
	if (m_pnCoverTime)
	{
		memset(m_pnCoverTime,0,sizeof(int)*m_nSortLen);
	}
	return 0;
}

float CDTDetector::process(float* far_data,float *near_data,int FarVAD)
{
	/**/
	if (FarVAD>0)
	{
		m_nVADTail=1;

	}
	else
	{
		m_nVADTail--;
		m_nVADCount=0;
	}	
	m_nVADTail=m_nVADTail<(-5)?(m_nVADTail+1):m_nVADTail;
	UpdateBuffer(far_data,near_data);
	if (m_nVADTail>0)
	{
		m_nVADCount++;
		
		if (m_nVADCount>0)
		{
			//if ((*(m_pfFarPwr+m_nOffset)>(0.0001*m_nWinlen))&(m_fNearPwr>(0.0001*m_nWinlen)))
			Cal_Correlation();	
		}

	}
	else
	{ 
		if (m_nVADTail==-1) 
		{
			memset(m_pfFarPwr,0,sizeof(float)*(m_nMemfloatsum-m_nSortLen-m_nBufCorrLen));//reset buffer of far data, near data and power.
			m_fNearPwr=0.f;
			m_pFarBufInd=m_pfcFarBuf;
			m_pfNearBufInd=m_pfcNearBuf;
		}		
	}
	return m_pfCorSort[2];

}
float CDTDetector::processDelay(float* far_data,float *near_data,int FarVAD)
{
#if 0
	float tempcor=0.f;
	if (FarVAD>0)
	{
		m_nVADTail=1;

	}
	else
	{
		m_nVADTail--;
		m_nVADCount=0;
	}	
	m_nVADTail=m_nVADTail<(-5)?(m_nVADTail+1):m_nVADTail;
	UpdateBuffer(far_data,near_data);
	if (m_nVADTail>0)
	{
		
		m_nVADCount++;
        //m_nVADCount%=5;
		if (m_nVADCount > 0)
		{
			//if ((*(m_pfFarPwr+m_nOffset)>(0.0001*m_nWinlen))&(m_fNearPwr>(0.0001*m_nWinlen)))
			tempcor=Cal_Correlation();	
		}

	}
	else
	{ 
		if (m_nVADTail==-1) 
		{
			memset(m_pfFarPwr,0,sizeof(float)*(m_nMemfloatsum-m_nSortLen-m_nBufCorrLen));//reset buffer of far data, near data and power.
			m_fNearPwr=0.f;
			m_pFarBufInd=m_pfcFarBuf;
			m_pfNearBufInd=m_pfcNearBuf;
		}		
	}
#else
	m_nIsVoice = FarVAD;
	float tempcor=0.f;
	UpdateBuffer(far_data,near_data);
	tempcor=Cal_Correlation();	
#endif


	return m_pfCorSort[2];

}
int CDTDetector::UpdateBuffer(float* far_data,float *near_data)
{
	int i=0;
	int	j;
	float tmp_pwr_add_f,tmp_pwr_sub_f,tmp_pwr_add_n,tmp_pwr_sub_n;
	float* pf_s_far;

	tmp_pwr_add_f=0.f;
	tmp_pwr_sub_f=0.f;
	tmp_pwr_add_n=0.f;
	tmp_pwr_sub_n=0.f;

	/*move power forward */
	pf_s_far=m_pfcFarBuf;//m_pfcFarBuf =m_pfFarPwr  + MaxDely;   
	for(i=m_nMaxDelay;i>1;i--)
	{
		*(--pf_s_far)=*(pf_s_far-1);
	}
	/*find far-end data removed */
	pf_s_far=m_pFarBufInd-2*m_nBins*(m_nWinlen-1);
	if (pf_s_far<m_pfcFarBuf)
	{
		j=pf_s_far-m_pfcFarBuf;
		pf_s_far=j+m_pfcNearBuf;
	}

	float* pf_s_near = m_pfNearBufInd - 2 * m_nBins*(m_nWinlen - 1);
	if (pf_s_near < m_pfcNearBuf)
	{
		j = pf_s_near - m_pfcNearBuf;
		pf_s_near = j + m_pfCorr;
	}


	/*update powers*/
#if 1
	j = 2 * m_nStarBin;
	float* pNearStart = &near_data[j];
	float* pFarStart = &far_data[j];
	if (true == m_bIsExponentialCorrelation)
	{
		m_fNearPwr = AutoCorrelationProduct(m_fNearPwr, pNearStart, m_fLameta, 1);
		*m_pfFarPwr = AutoCorrelationProduct(*m_pfFarPwr, pFarStart, m_fLameta, 1);
	}
	else
	{
		m_fNearPwr = AutoCorrelationProduct(m_fNearPwrUpdate, pNearStart, m_fLameta, 1);
		*m_pfFarPwr = AutoCorrelationProduct(m_fFarPwrUpdate, pFarStart, m_fLameta, 1);
		/*substract pwr of removed data in advance to next frame*/
		m_fNearPwrUpdate = AutoCorrelationProduct(m_fNearPwr, pf_s_near, 1, m_fLametaN);
		m_fFarPwrUpdate = AutoCorrelationProduct(*m_pfFarPwr, pf_s_far, 1, m_fLametaN);
	}
	/* update buffer */
	j = 2 * m_nBins;
	memcpy(m_pfNearBufInd,  pNearStart, j * sizeof(float));
	memcpy(m_pFarBufInd,  pFarStart, j * sizeof(float));
	m_pfNearBufInd += j;
	m_pFarBufInd += j;
#else
	j=2*m_nStarBin;
	for (i=0;i<m_nBins;i++)
	{	
		tmpf_a=far_data[j];
		tmpf_s=*pf_s_far++;
		*(m_pFarBufInd++)  =tmpf_a;
		tmp_pwr_add_f+=tmpf_a*tmpf_a;
		tmp_pwr_sub_f+=tmpf_s*tmpf_s;

		tmpf_a=far_data[j+1];
		tmpf_s=*pf_s_far++;
		*(m_pFarBufInd++)  =tmpf_a;
		tmp_pwr_add_f+=tmpf_a*tmpf_a;
		tmp_pwr_sub_f+=tmpf_s*tmpf_s;

		tmpf_a=near_data[j++];
		tmpf_s=*m_pfNearBufInd;
		*(m_pfNearBufInd++)=tmpf_a;
		tmp_pwr_add_n+=tmpf_a*tmpf_a;
		tmp_pwr_sub_n+=tmpf_s*tmpf_s;

		tmpf_a=near_data[j++];
		tmpf_s=*m_pfNearBufInd;
		*(m_pfNearBufInd++)=tmpf_a;
		tmp_pwr_add_n+=tmpf_a*tmpf_a;
		tmp_pwr_sub_n+=tmpf_s*tmpf_s;
	}
	*m_pfFarPwr+=tmp_pwr_add_f-tmp_pwr_sub_f;
	m_fNearPwr +=tmp_pwr_add_n-tmp_pwr_sub_n;
#endif


	m_pfNearBufInd=m_pfNearBufInd==m_pfCorr?m_pfcNearBuf:m_pfNearBufInd;//    m_pfCorr=m_pfcNearBuf+ (m_nBins*2*m_nWinlen);  
	m_pFarBufInd  = m_pFarBufInd ==m_pfcNearBuf?m_pfcFarBuf:m_pFarBufInd;//  m_pfcNearBuf=m_pfcFarBuf + (m_nBins*2*m_nFarBufferLen);

	return 0;
}

float CDTDetector::Cal_Correlation()
{
	int i,j,k,m;
	int delay;
	float tmp_corr,tmp_max_corr,pwr;
	float *pf_f_s;
	float* pf_pwr=m_pfFarPwr+m_nOffset;
	int nDynamicMaxDealy=m_nMaxDelay;
	/*calculate current correaltion*/
	tmp_max_corr=0;
	m=m_nOffset*2*m_nBins;
	j=(m_pFarBufInd-m_pfcFarBuf);
	for (i=m_nOffset;i<m_nMaxDelay;i++)
	{
		pwr=*(pf_pwr++);	
		k=(i+m_nWinlen)*2*m_nBins;

		if (j>=(k))
		{
			pf_f_s=m_pFarBufInd-k;
		}
		else
		{
			pf_f_s=m_pfcNearBuf-k+j;

		}
		if (true == m_bIsExponentialCorrelation)
		{
			tmp_corr=ExponentialCorrelation(pf_f_s,m_pfNearBufInd,i);	
		} 
		else
		{
			tmp_corr=SlideWindowCorrelation(pf_f_s,m_pfNearBufInd,i);
		}
		
		if ((pwr>(MIN_POWER*m_nWinlen))&(m_fNearPwr>(MIN_POWER*m_nWinlen)))	
		{					
			tmp_corr/=pwr*m_fNearPwr;
		}
		else
			tmp_corr=0.f;
		if((tmp_corr>tmp_max_corr)&&(i<nDynamicMaxDealy))
		{
			tmp_max_corr=tmp_corr;

			delay=i;
			if((nDynamicMaxDealy==m_nMaxDelay) &&(tmp_max_corr>m_fSerachThreashold))
			{
				nDynamicMaxDealy=i+LOCAL_WINDOW;
				if(nDynamicMaxDealy<m_nDelay)
				{
					nDynamicMaxDealy=m_nDelay+2;
				}
			}
		}

	}
	///////////delay validatity
	/*buffer and sort correlation*/
	if (1 == m_nIsVoice)
	{

		UpdateThreshold(tmp_max_corr);
		m_nDelay=(tmp_max_corr>m_fDecisionThreshold)?delay:m_nDelay;

		*m_pfCorrInd=tmp_max_corr;
		SortCorr();
		m_pfCorrInd++;
		m_pfCorrInd=(m_pfCorrInd==m_pfCorSort)?m_pfCorr:m_pfCorrInd;;
	}
	

	
	return tmp_max_corr;
}
 inline void CDTDetector::UpdateThreshold(float af)
 {
	 float tempf=af*0.8f;
	 if(tempf>m_fDecisionThreshold)
	 {
		 m_fDecisionThreshold=tempf;
		
	 }
	 else 
	 {
		  m_fDecisionThresholdTemp= m_pfCorrInd[0]*0.8;
	 }
	 m_fSerachThreashold=m_fDecisionThreshold*0.75;

 }
float CDTDetector::Correlation(float *far_star,float *near_start)
{
	float* p_far=far_star;
	float* p_near= near_start;
	int max_i_far;
	int max_i_near;
	int i,maxloop,multi_time;
	float fresult_r,fresult_i;
	float f_r,f_i,n_r,n_i;
	fresult_i=0.f;
	fresult_r=0.f;
	multi_time=m_nBins*2*m_nWinlen;//max loop steps
	/*find out wrap point*/
	max_i_far=m_pfcNearBuf-p_far;
	max_i_near=m_pfCorr-p_near;
	maxloop=max_i_near<=max_i_far?max_i_near:max_i_far;
	maxloop=maxloop<=multi_time?maxloop:multi_time;

	/*calculate sum(a(n)*conj(b(n)))*/	
	for (i=0;i<maxloop;i+=2)
	{
		f_r=*p_far++;
		f_i=*p_far++;
		n_r=*p_near++;
		n_i=*p_near++;

		fresult_r+=f_r*n_r+f_i*n_i;
		fresult_i+=f_r*n_i-f_i*n_r;
	}
	/*wrap buffer*/
	p_far= (p_far==m_pfcNearBuf)?m_pfcFarBuf:p_far;
	p_near= (p_near==m_pfCorr)?m_pfcNearBuf:p_near;	

	maxloop=max_i_near>max_i_far?max_i_near:max_i_far;
	maxloop=maxloop<=multi_time?maxloop:multi_time;

	for (;i<maxloop;i+=2)
	{
		f_r=*p_far++;
		f_i=*p_far++;
		n_r=*p_near++;
		n_i=*p_near++;

		fresult_r+=f_r*n_r+f_i*n_i;
		fresult_i+=f_r*n_i-f_i*n_r;

	}
	/*wrap buffer if need*/
	p_far= (p_far==m_pfcNearBuf)?m_pfcFarBuf:p_far;
	p_near= (p_near==m_pfCorr)?m_pfcNearBuf:p_near;

	for (;i<multi_time;i+=2)
	{
		f_r=*p_far++;
		f_i=*p_far++;
		n_r=*p_near++;
		n_i=*p_near++;
		fresult_r+=f_r*n_r+f_i*n_i;
		fresult_i+=f_r*n_i-f_i*n_r;
	} 

	return (fresult_i*fresult_i+fresult_r*fresult_r);//|result|^2

}
int CDTDetector::SortCorr()
{
	int i,j,offset;
	float tmp;
	float crr_c=*m_pfCorrInd;
	float *pf   =m_pfCorrInd;
	/*update sort current value*/
	if (crr_c>*m_pfCorSort)
	{
		*m_pfCorSort=crr_c;
		*m_pnCoverTime=m_nBufCorrLen;
		for (i=1;i<m_nSortLen;i++)
		{
			if (crr_c>m_pfCorSort[i])
			{
				m_pfCorSort[i-1]=m_pfCorSort[i];
				m_pnCoverTime[i-1]=m_pnCoverTime[i];
				m_pfCorSort[i]=crr_c;
				m_pnCoverTime[i]=m_nBufCorrLen;

			}
		}
	}
	/*find removed element*/
	j=-1;
	for (i=0;i<m_nSortLen;i++)
	{
		m_pnCoverTime[i]--;
		j=m_pnCoverTime[i]==0?i:j;
	}
	/*find the (m_nSortLen)th most value*/
	if (j>=0)
	{
		/*delete element remvoed */
		for (i=j;i>0;i--)
		{

			m_pfCorSort[i]=m_pfCorSort[i-1];
			m_pnCoverTime[i]=m_pnCoverTime[i-1];
		}

		m_pfCorSort[0]=-1.f;
		m_pnCoverTime[0]=0;
		j=(m_nBufCorrLen-1);
		offset=m_pfCorrInd-m_pfCorr+1;
		offset=(offset<j)?offset:j;
		tmp=m_pfCorSort[1];
		for (i=0;i<offset;i++)
		{

			crr_c=*pf--;
			if((crr_c>m_pfCorSort[0])&(crr_c<tmp))
			{
				*m_pfCorSort=crr_c;
				*m_pnCoverTime=m_nBufCorrLen-i-1;
			}
		}
		if(pf<m_pfCorr)
			pf=m_pfCorSort-1;
		for (;i<j;i++)
		{
			crr_c=*pf--;
			if((crr_c>m_pfCorSort[0])&(crr_c<tmp))
			{
				*m_pfCorSort=crr_c;
				*m_pnCoverTime=m_nBufCorrLen-i-1;
			}
		}


	}
	return j;

}

float CDTDetector::ExponentialCorrelation(float *far_start,float *near_start, int delay)
{
	
	int frame_time = 2 * m_nBins;
	int multi_time = frame_time * m_nWinlen;
	float* pf_a_far = far_start + multi_time - frame_time; //latest far frame in window
	int wrap_around_len = pf_a_far - m_pfcNearBuf;
	if (wrap_around_len >= 0)
	{
		pf_a_far = m_pfcFarBuf + wrap_around_len;
	}


	float* pf_a_near = near_start + multi_time - frame_time; //latest near frame in window
	//float* pf_a_near = near_start - frame_time;
	wrap_around_len = pf_a_near - m_pfCorr;
	if (wrap_around_len >= 0)
	{
		pf_a_near = m_pfcNearBuf + wrap_around_len;
	}
	

	// reduce old cross product and add latest corss product for a certain delay
	CrossCorrelationProduct(m_pfCrossProduct[delay], pf_a_near, pf_a_far, m_fLameta, 1);
	// |m_pfCrossProduct[delay]|^2
	float power = m_pfCrossProduct[delay].real * m_pfCrossProduct[delay].real
		+ m_pfCrossProduct[delay].image * m_pfCrossProduct[delay].image;

	return power;
}

float CDTDetector::SlideWindowCorrelation(float *far_start, float *near_start, int delay)
{
	int frame_time = 2 * m_nBins;
	int multi_time = frame_time * m_nWinlen;

	float* pf_s_far = far_start; //earliest far frame in window
	float* pf_a_far = far_start + multi_time - frame_time; //latest far frame in window
	int wrap_around_len = pf_a_far - m_pfcNearBuf;
	if (wrap_around_len >= 0)
	{
		pf_a_far = m_pfcFarBuf + wrap_around_len;
	}

	float* pf_s_near = near_start; //earliest near frame in window
	float* pf_a_near = near_start + multi_time - frame_time; //latest near frame in window
	wrap_around_len = pf_a_near - m_pfCorr;
	if (wrap_around_len >= 0)
	{
		pf_a_near = m_pfcNearBuf + wrap_around_len;
	}

	// reduce old cross product and add latest corss product for a certain delay
	CrossCorrelationProduct(m_pfCrossProduct[delay], pf_a_near, pf_a_far, m_fLameta, 1);

	// |m_pfCrossProduct[delay]|^2
	float power = m_pfCrossProduct[delay].real * m_pfCrossProduct[delay].real
		+ m_pfCrossProduct[delay].image * m_pfCrossProduct[delay].image;

	// substract earliest corss product for a certain delay
	CrossCorrelationProduct(m_pfCrossProduct[delay], pf_s_near, pf_s_far, 1, m_fLametaN);

	return power;
}

void CDTDetector::CrossCorrelationProduct(ComplexData_t& crossProduct, float *nearStart, float *farStart, float lameta1, float lameta2)
{
	float fresult_r = 0;
	float fresult_i = 0;

	float sig1_r = 0;
	float sig1_i = 0;
	float sig2_r = 0;
	float sig2_i = 0;
	float* pNear = nearStart;
	float* pFar = farStart;
	int i = 0;
	for (; i < m_nBins; ++i)
	{
		sig1_r = *pNear++;
		sig1_i = *pNear++;
		sig2_r = *pFar++;
		sig2_i = *pFar++;

		fresult_r += sig1_r*sig2_r + sig1_i*sig2_i;
		fresult_i += sig1_i*sig2_r - sig1_r*sig2_i;
	}

	crossProduct.real = lameta1 * crossProduct.real;
	crossProduct.image = lameta1 * crossProduct.image;

	crossProduct.real += lameta2 * fresult_r;
	crossProduct.image += lameta2 * fresult_i;
}

float CDTDetector::AutoCorrelationProduct(float autoCorrelation, float *sig, float lameta1, float lameta2)
{
	float fresult = 0;

	float sig1_r = 0;
	float sig1_i = 0;
	float* pSig = sig;
	int i = 0;
	for (; i < m_nBins; ++i)
	{
		sig1_r = *pSig++;
		sig1_i = *pSig++;

		fresult += sig1_r*sig1_r + sig1_i*sig1_i;
	}

	autoCorrelation = lameta1 * autoCorrelation;

	autoCorrelation += lameta2 * fresult;
	return autoCorrelation;
}