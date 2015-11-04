/**************************************************
*         Copyright 2014 Coddy Inc.
*         Author:  Gu Cheng
*         History: 10/15/2014
*         Content: Calculation of Wave Energy
***************************************************/

#ifndef AECHALFDUPLEX_SPEST_H_
#define AECHALFDUPLEX_SPEST_H_

class SPEst {
public:
	SPEst(void);
	~SPEst(void);

	/***************************************************
	name:    InitPara
	para:    size  (IN)
	content: initial parameter
	***************************************************/
	void InitPara(const int size);

	/***************************************************
	name:    PwrEnergy
	para:    inbuf   (IN)  
	         outbuf  (OUT)
	content: calculate energy by using frequency data after fourier change
	***************************************************/
	void PwrEnergy(const float* inbuf, float* smooth, float* nonsmooth);

private:
	/***************************************************
	name:    FreePara
	content: release the allocated memory
	***************************************************/
	void FreePara();

	int m_shift;                 //the length of the offset
	float* m_input_spe_smthslow; //energy value
};
#endif

