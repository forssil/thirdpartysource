
#include <math.h>
#include <stdio.h>
#include <memory.h>
#include "processingconfig.h"
#include "SubbandAdap.h"
#include "subbandgroup.h"


CSubbandAdap::CSubbandAdap(int Fs, float fft_ms)
{
	m_nFs=Fs;
	m_fFFTlen_ms=fft_ms;
	m_nFFTsize=int(m_fFFTlen_ms*m_nFs/1000);
	Subband_init();

}
CSubbandAdap::CSubbandAdap(int Fs, int fft_samples)
{
	m_nFs=Fs;
	m_nFFTsize=fft_samples;
	m_fFFTlen_ms=m_nFFTsize*1000.f/m_nFs;
	Subband_init();
}
CSubbandAdap::~CSubbandAdap(void)
{


	if (m_CpAdapG)
	{
		delete m_CpAdapG;
	}

	if (m_npFilterLen)
	{
		delete m_npFilterLen;
	}


	
}
void CSubbandAdap::Subband_init()
{
	int i;
	
	if (m_nFs == 48000)
	{

		m_nSubused = SUBBANDUSE44k;


	}

	if (m_nFs==44100)
	{
		
		m_nSubused =SUBBANDUSE44k;          
	
		
	}
	if (m_nFs==22050)
	{
		
		m_nSubused =SUBBANDUSE22k;     
	
	}
	

	m_npFilterLen=new int[m_nSubused];
	memset(m_npFilterLen,0,sizeof(int)*(m_nSubused));
	
	for(i=0;i<m_nSubused;i++)
	{        
		m_npFilterLen[i]=FILTER_LEN;
		

	}

	m_CpAdapG=new CAdapFilterGroup(m_nSubused,m_npFilterLen);

	m_nSubbandGroupk=0;
	
	Decay_init();

	m_nFarVAD=0;


}
void CSubbandAdap::Decay_init()
{
	int i;
    float attack=1;
	for(i=0;i<m_nSubused;i++)
	{
		m_CpAdapG->UpdateDecay(i,0,&attack,1.f,1.f);
	}
}

static float powf_to_n(float x, int n)
{
	float y;
	if( n&0x1 )
	{
		y=x;
	}
	else
	{
		y=1.0f;
	}
	n /= 2;
	while( n!=0 )
	{
		x *= x;
		if( n&0x1 )
		{
			y*=x;
		}
		n /= 2;
	}
	return y;
}
// offset in frequency domain 
void CSubbandAdap::process(float * Refer ,float *Des,float *OutErr,float *OutEst,int offset,audio_pro_share aecdata)
{
	
	int decay_computed=0;

	float *re=Refer+offset*2;
	float *de=Des+offset*2;
	float *outerr=OutErr+offset*2;
	float *outest=OutEst+offset*2;
	bool copied = false;


	m_nFarVAD=aecdata.nFarVAD_;

	//do filter
	//m_CpAdapG->UpdateStep(aecdata.fDTDgain);
	m_CpAdapG->UpdateDelta(aecdata.pNoiseSPwr+offset, aecdata.fDTDgain);
	//m_CpAdapG->UpdateDelta(NULL);
	m_CpAdapG->process(re,de,outest,outerr,m_nFarVAD);

	
}
