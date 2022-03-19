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
	
	m_fft_len   = size*2;

    const float inv_size = 1.0f / (float)1024;
    const float step = 2.0f * AUDIO_COMMON_PI / ((float)1024 - 1.0f);
    for (int i = 0; i < 1024; ++i) {
        float weight = (0.5f * (1.0f - cosf((float)i * step)));
        hwin1024_new[i] = weight;
        //hwin1024_new[i] = weight * inv_size;
    }

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
    case 960:
        //float hwin1024_new[1024] = { 0.f };
        m_fft_len = 1024;
        //const float inv_size = 1.0f / (float)m_fft_len;
        //const float step = 2.0f * AUDIO_COMMON_PI / ((float)m_fft_len - 1.0f);
        //for (int i = 0; i < m_fft_len; ++i) {
        //    float weight = (0.5f * (1.0f - cosf((float)i * step)));
        //    hwin1024_new[i] = weight;
        //    //hwin1024_new[i] = weight * inv_size;
        //}
        fp = hwin1024_new;
        break;
	default:
		fp   = hwin256;
		m_fft_len=256;

	}
    m_framelen = size;
    m_shift = m_fft_len - size;
	m_ana_win=new float[m_fft_len];

	for(int i=0; i<m_fft_len; i++)
	{
		m_ana_win[i]=sqrtf(fp[i]);
	}
	

	//m_input_tim  = new float[size*2];
	//m_output_spe = new float[size*2];

	//memset(m_input_tim,  0, sizeof(float)*size*2);
	//memset(m_output_spe, 0, sizeof(float)*size*2);

    m_input_tim = new float[m_fft_len];
    m_output_spe = new float[m_fft_len];

    memset(m_input_tim, 0, sizeof(float)*m_fft_len);
    memset(m_output_spe, 0, sizeof(float)*m_fft_len);
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
	memcpy(m_input_tim, m_input_tim + m_framelen, sizeof(float)*m_shift);
}

/***************************************************
name:    UpdateFDbuffer
para:    data   (IN)  
content: update data
***************************************************/
inline void T2Ftransformer::UpdateFDbuffer(const float* data) {
	const float* fpin   = data;
	float* fpFDin = m_input_tim + m_shift;
	memcpy(fpFDin, fpin, sizeof(float)*m_framelen);
}
