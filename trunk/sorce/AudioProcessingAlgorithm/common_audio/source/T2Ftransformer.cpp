/**************************************************
*         Copyright 2014 GaoH Inc.
*         Author:  Gu Cheng
*         History: 10/15/2014
*         Content: The Fourier Transform
***************************************************/

#include "T2Ftransformer.h"
#include<cstdlib>
#include<cstring>
#include<cmath>
#include "fft.h"

T2Ftransformer::T2Ftransformer(void): 
m_input_tim(NULL),
m_output_spe(NULL),
m_ana_win(NULL),
m_shift(0),
m_fft_len(0) {

}

T2Ftransformer::~T2Ftransformer(void) {
	FreeFDanaly();
}

/***************************************************
name:    InitFDanaly
para:    size  (IN)
content: initial parameter
***************************************************/
void T2Ftransformer::InitFDanaly(const int size) {
	m_shift = size;
	m_fft_len   = size*2;
	const float *fp=NULL;
	switch(m_fft_len)
	{
	case 256:
		fp   = hwin256;
		break;
	case 512:
		fp   = hwin512;
		break;
	case 1024:
		fp   = hwin1024;
		break;
	default:
		fp   = hwin256;
		m_fft_len=256;

	}
	m_ana_win=new float[m_fft_len];

	for(int i=0; i<m_fft_len; i++)
	{
		m_ana_win[i]=sqrtf(fp[i]);
	}
	

	m_input_tim  = new float[size*2];
	m_output_spe = new float[size*2];

	memset(m_input_tim,  0, sizeof(float)*size*2);
	memset(m_output_spe, 0, sizeof(float)*size*2);
}

/***************************************************
name:    FreeFDanaly
content: release the allocated memory
***************************************************/
inline void T2Ftransformer::FreeFDanaly() {
	if(m_input_tim)	 delete []m_input_tim;
	if(m_output_spe) delete []m_output_spe;
	if( m_ana_win) delete m_ana_win;
}

/***************************************************
name:    T2F
para:    inbuf   (IN)  
	     outbuf  (OUT)
content: the fourier transform
***************************************************/
void T2Ftransformer::T2F(const float* inbuf, float* outbuf) {
	//update buffer
	UpdateFDbuffer(inbuf);
	//multiple win
	for(int i = 0; i < m_fft_len; ++i)
		outbuf[i] = m_input_tim[i]*m_ana_win[i];
	//the fourier transform
	FFT(outbuf, m_output_spe, m_fft_len);
	//update buffer
	memcpy(m_input_tim, m_input_tim+m_shift, sizeof(float)*m_shift);
}

/***************************************************
name:    UpdateFDbuffer
para:    data   (IN)  
content: update data
***************************************************/
inline void T2Ftransformer::UpdateFDbuffer(const float* data) {
	const float* fpin   = data;
	float* fpFDin = m_input_tim + m_shift;
	memcpy(fpFDin, fpin, sizeof(float)*m_shift);
}
