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
#ifndef _DELAY_EST_
#define _DELAY_EST_

typedef struct ComplexData
{
	float real;
	float image;
}ComplexData_t;

class CDTDetector
{
public:
	CDTDetector(int Fs = 16000, int Winlen = 15, int StarBin = 4, int EndBin = 60, int MaxDely = 5, float framelen_timeUms = 10.f, float updateCoeff=1.f, bool isExponentialCorrelation = true);
	~CDTDetector();
	int DTDReset();
	float process(float* far_data,float *near_data,int FarVAD);
	float processDelay(float* far_data,float *near_data,int FarVAD);
	int GetDelay()
	{ 
		int tmp=m_nDelay/5*5;
		if((tmp>5)&&(m_nDelay-tmp)<3)
		{
			tmp-=2;
		}
		return tmp;};
private:
	int UpdateBuffer(float* far_data,float *near_data);
	float Correlation(float *far_star,float *near_start);
	float ExponentialCorrelation(float *far_start, float *near_start, int delay);
	float SlideWindowCorrelation(float *far_start, float *near_start, int delay);
	void  CrossCorrelationProduct(ComplexData_t& crossProduct, float *nearStart, float *farStart, float lameta1, float lameta2);
	float AutoCorrelationProduct(float autoCorrelation, float *sig, float lameta1, float lameta2);
	float Cal_Correlation();
	int SortCorr();
	inline void UpdateThreshold(float af);
private:
	int m_nMemfloatsum;
	int m_nFarBufferLen;   //length of far data buffer
	int m_nWinlen;         //length of calculate
	int m_nStarBin;        //begin band
	int m_nEndBin;         //end band
	int m_nBins;
	int m_nMaxDelay;       //frame number responding to max delay time
	int m_nBufCorrLen;     //length of buffering correlation
	int m_nSortLen;        //length of sort buffer 
	int m_nVADTail; //
	int m_nVADCount;  //count >m_nWlen, start calculate
	int m_nOffset;    // start time 
	int* m_pnCoverTime;    //m_nBufCorrLen
	float m_fNearPwr;      //power of near data

	ComplexData_t* m_pfCrossProduct;
	float m_fLameta;
	float m_fLametaN;
	float m_fNearPwrUpdate;
	float m_fFarPwrUpdate;
	bool  m_bIsExponentialCorrelation;

	float* m_pfFarPwr;     //power array of far data, length is m_nBufCorrLen

	float* m_pfcFarBuf;     //bins x m_nFarBufferLen array, 2i real, 2i+1 imag; sort: line1, line2,...
	float* m_pFarBufInd;    //index for start frame

	float* m_pfcNearBuf;    //bins x m_nWinlen array, 2i real, 2i+1 imag; sort: line1, line2,...
	float* m_pfNearBufInd;  //index for start frame

	float* m_pfCorr;        //m_nBufCorrLen length
	float* m_pfCorrInd; 
	float* m_pfCorSort;
	//float

	int  m_nDelay;
	int  m_nIsVoice;

	float  m_fDecisionThreshold;
	float  m_fDecisionThresholdTemp;
	float  m_fSerachThreashold;



};

#endif //_DELAY_EST_