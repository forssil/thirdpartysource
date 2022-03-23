/**************************************************
*         Copyright 2014 GaoH Inc.
*         Author:  Gu Cheng
*         History: 10/15/2014
*         Content: The Fourier Inverse Transform
***************************************************/

#include <math.h>
#include<string.h>
#include "fft.h"
#include "F2Ttransformer.h"

F2Ttransformer::F2Ttransformer(void):
m_input_spe(NULL)
,m_input_tim(NULL)
,m_output_spe(NULL)
,m_shift(0)
,m_fft_len(0)
,AnaWin(NULL)
{
}


F2Ttransformer::~F2Ttransformer(void)
{
	FreeFDanaly();
}

/***************************************************
name:    InitFDanaly
para:    size  (IN)
content: initial parameter
***************************************************/
void F2Ttransformer::InitFDanaly(const int size)
{
	m_fft_len  = size*2;

    const float inv_size = 1.0f / (float)1024;
    const float step = 2.0f * AUDIO_COMMON_PI / ((float)1024 - 1.0f);
    for (int i = 0; i < 1024; ++i) {
        float weight = (0.5f * (1.0f - cosf((float)i * step)));
        syn_win1024_new[i] = weight;
        //syn_win1024_new[i] = weight * inv_size;
    }
	const float *fp=NULL;
	switch(m_fft_len)
	{
	case 256:
		fp   = syn_hwin256;
		break;
	case 512:
		fp   = syn_hwin512;
		break;
	case 1024:
		fp   = syn_hwin1024;
		break;
    case 960:
        //float syn_win1024_new[1024] = { 0.f };
        m_fft_len = 1024;
        //const float inv_size = 1.0f / (float)m_fft_len;
        //const float step = 2.0f * AUDIO_COMMON_PI / ((float)m_fft_len - 1.0f);
        //for (int i = 0; i < m_fft_len; ++i) {
        //    float weight = (0.5f * (1.0f - cosf((float)i * step)));
        //    //hwin1024_new[i] = weight;
        //    syn_win1024_new[i] = weight * inv_size;
        //}
        fp = syn_win1024_new;
        break;
	default:
		fp   = syn_hwin256;
		m_fft_len=256;

	}

    m_framelen = size;
    m_shift = m_fft_len - size;

	AnaWin= new float[m_fft_len];
	for(int i=0; i<m_fft_len; i++)
	{
		AnaWin[i]=sqrtf(fp[i]);
	}
	

	//m_input_spe  = new float[size*2];
	//m_input_tim  = new float[size*2];
	//m_output_spe = new float[size*2];

	//memset(m_input_spe,  0, sizeof(float)*size*2);
	//memset(m_input_tim,  0, sizeof(float)*size*2);
	//memset(m_output_spe, 0, sizeof(float)*size*2);

    m_input_spe = new float[m_fft_len];
    m_input_tim = new float[m_fft_len];
    m_output_spe = new float[m_fft_len];

    memset(m_input_spe, 0, sizeof(float)*m_fft_len);
    memset(m_input_tim, 0, sizeof(float)*m_fft_len);
    memset(m_output_spe, 0, sizeof(float)*m_fft_len);

#ifdef ARM_NEON
	if (ne10_init() != NE10_OK)
	{
		return 1;
	}

	m_cfg = ne10_fft_alloc_r2c_float32(m_fft_len);
	m_pdst = new ne10_fft_cpx_float32_t[m_fft_len / 2 + 1];
	memset(m_pdst, 0, sizeof(ne10_fft_cpx_float32_t)*(m_fft_len / 2 + 1));

#endif
}

/***************************************************
name:    FreeFDanaly
content: release the allocated memory
***************************************************/
void F2Ttransformer::FreeFDanaly()
{
	if(m_input_spe)	    delete []m_input_spe;
	if(m_input_tim)	    delete []m_input_tim;
	if(m_output_spe)	delete []m_output_spe;
	if(AnaWin) delete AnaWin;
#ifdef ARM_NEON
	ne10_fft_destroy_r2c_float32(m_cfg);
	if (m_pdst) delete[] m_pdst;

#endif
}

/***************************************************
name:    F2T
para:    inbuf   (IN)  
	        outbuf  (OUT)
content: the fourier inverse transform
***************************************************/
void F2Ttransformer::F2T(const float *inbuf, float *outbuf)
{
	//ifft
#ifndef ARM_NEON
	memcpy(m_input_spe,inbuf,m_fft_len*sizeof(float));

	InvFFT(m_input_spe, m_output_spe, m_fft_len);
#else
	for (int i = 0; i < m_fft_len; i += 2) {
		m_pdst[i / 2].r = inbuf[i];
		m_pdst[i / 2].i = inbuf[i+1];
	}
	ne10_fft_c2r_1d_float32(m_input_spe,m_pdst, m_cfg);
	
#endif

	//update buffer
	UpdateFDbuffer(m_input_spe);
	//multiple win
	memcpy(outbuf,m_input_tim,sizeof(float)*m_framelen);
    // only add comfact for 480 framelen
    if (m_framelen == 480) {
        for (int n = 0; n < m_framelen; n++) {
            outbuf[n] = outbuf[n] * comfact[n] * 1.f;
        }
    }
	// half shift length 
	//memcpy(m_input_tim,m_input_spe+m_framelen, m_shift*sizeof(float));
    memcpy(m_input_tim, m_input_tim + m_framelen, m_shift * sizeof(float));// save histroy win-buffer 480~544

    memset(m_input_tim + m_shift, 0 ,sizeof(float)*m_framelen);
}

/***************************************************
name:    UpdateFDbuffer
para:    data   (IN)  
content: update data
***************************************************/
void F2Ttransformer::UpdateFDbuffer(float* data)
{
	float* fpin = data;
	float* fpFDin = m_input_tim;
	int i=0;
	for ( i=0; i<m_shift; i++)
	{
        (*fpin) *= AnaWin[i];
		*fpFDin+=*fpin;//* comfact[i]
		fpin++;
		fpFDin++;
	}
	for(; i<m_fft_len; i++)
	{
		(*fpin) *=  AnaWin[i];
        *fpFDin += *fpin;
		fpin++;
        fpFDin++;
	}

}