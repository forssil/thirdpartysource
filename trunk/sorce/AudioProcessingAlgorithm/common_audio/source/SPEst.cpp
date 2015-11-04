/**************************************************
*         Copyright 2014 Coddy Inc.
*         Author:  Gu Cheng
*         History: 10/15/2014
*         Content: Calculation of Wave Energy
***************************************************/

#include "SPEst.h"
#include "cstring"

SPEst::SPEst(void): 
m_shift(0),
m_input_spe_smthslow(NULL) {

}

SPEst::~SPEst(void) {
	FreePara();
}

/***************************************************
name:    InitPara
para:    size  (IN)
content: initial parameter
***************************************************/
void SPEst::InitPara(const int size) {
	m_shift = size;
	m_input_spe_smthslow = new float[size];
	memset(m_input_spe_smthslow, 0, sizeof(float)*size);
}

/***************************************************
name:    FreePara
content: release the allocated memory
***************************************************/
inline void SPEst::FreePara() {
	if(m_input_spe_smthslow) delete []m_input_spe_smthslow;
}

/***************************************************
name:    PwrEnergy
para:    inbuf   (IN)  
	     outbuf  (OUT)
content: calculate energy by using frequency data after fourier change
***************************************************/
void SPEst::PwrEnergy(const float* inbuf, float* smooth, float* nonsmooth) {
	const float* fpreal = inbuf;
	float* fpimag  = m_input_spe_smthslow;
	float tempreal = 0.f;
	float tempimag = 0.f;
	float tempPwr  = 0.f;
	float alpha    = 0.7f;
	//calculate energy by using frequency data after fourier change
	for(int i = 0; i < m_shift; ++i) {
		tempreal  = *fpreal++;
		tempimag  = *fpreal++;
		tempPwr   = (tempimag*tempimag + tempreal*tempreal);
		//update energy
		*fpimag  *= 1-alpha;
		*fpimag  += alpha*tempPwr;
		*smooth++    = *fpimag++;
		*nonsmooth++ = tempPwr;
	}
}
