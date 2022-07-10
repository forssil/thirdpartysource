#include "noisereduction.h"
#include <memory.h>


#include <math.h>
#include <stdio.h>
#include "processingconfig.h"
CNoiseRedu::CNoiseRedu(int fs, int fftlen)
{
 	int i,maxloop;
	float delt;
	m_nFFTLen=fftlen;
// 	m_nFs= fs;
	m_CPsd=new CPSDsmooth(fftlen,fs);
	m_nQNum=m_CPsd->GetTranfLen();
	m_CPsd_echo=new CPSDsmooth(fftlen,fs);
	m_CNois=new CNoiseEst(fs,m_nQNum,MS);
//	m_CSpeech=new CSpeechEst(fs,m_nQNum,fftlen);
	m_CSpeechStatic=new CSpeechEst(fs,m_nQNum,fftlen);
	m_CSpeechTransient = new CSpeechEst(fs, m_nQNum, fftlen);
	m_pfNoise=m_CNois->GetNoise();
	m_pfTrans=m_CNois->GetTrans();
	m_pfAlpha=m_CPsd->GetAlpha();
//	m_pfGaintemp=m_CSpeech->GetGain();
	m_pfGaintemp = new float[2*m_nQNum];
	m_pfTransGain = m_pfGaintemp + m_nQNum;
	memset(m_pfGaintemp, 0, sizeof(float)*m_nQNum);
	for (int i = 0; i < m_nQNum; i++) {
		m_pfTransGain[i] = 1.f;
	}
	m_pfPwr=m_CPsd->GetCQPsd();
	m_pfPwrFd=m_CPsd->GetCQPsdFd();
	m_fMinGain=0.2f;
	maxloop=fftlen/2;
	m_pfGainout=new float[4*maxloop];
	m_pfNoiseLine=m_pfGainout+maxloop;
	m_pfWinWeight=m_pfNoiseLine+maxloop;
    m_pfnrin_smooth = m_pfWinWeight + maxloop;
	delt= 1.f/float(maxloop);
	for (i=0;i<maxloop;i++)
	{
		m_pfWinWeight[i]=Window(i,delt);
		m_pfNoiseLine[i]=minvalue;
        m_pfnrin_smooth[i] = 0.f;

	}
	//init
	m_sAecdata.pDesireFFT_=NULL;
	m_sAecdata.nLengthFFT_=0;
	m_sAecdata.pEstimationFFT_=NULL;	
	m_sAecdata.pErrorFFT_=NULL;
	m_sAecdata.pRefferFFT_=NULL;

	m_sAecdata.pGain_=NULL;
	m_sAecdata.nGainLength_=0;
	m_sAecdata.nOffsetBin_=0; //AEC start bin in fft
	m_sAecdata.pNoiseSPwr=NULL;

	m_sAecdata.nFarVAD_=0;


}

CNoiseRedu::~CNoiseRedu(void)
{
	delete m_CNois;
//	delete m_CSpeech;
	delete m_CSpeechStatic;
	delete m_CSpeechTransient;
	delete m_CPsd;
	delete m_pfGainout;
	delete m_CPsd_echo;
	delete[]m_pfGaintemp;
}
void CNoiseRedu::Process(float *input,float *echonoise,audio_pro_share & aecdata,float *pfAft,float *pfBef)
{
	m_sAecdata=aecdata;
	m_CPsd->processing(input,m_pfNoise);
	if (NULL != aecdata.pNRDynamicRefer_)
	{
		m_CPsd_echo->processing(echonoise,NULL);
	}

    /*attention: m_pfPwr[0] is all band power!!!!!!*/
	m_CNois->Process(m_pfPwr,m_pfAlpha);

	/*transent noise*/
	if(aecdata.pNRDynamicRefer_)
	{
		transientnois();
	}

    /*attention: m_pfNoise[0] is all band power!!!!!!*/
//	m_CSpeech->Porcess(m_pfPwr,m_pfNoise,m_pfTrans);
#if 0
    /*attention: m_pfNoise[0] is all band power!!!!!!*/
	m_CSpeechStatic->Porcess(m_pfPwr, m_pfNoise, m_pfTrans);
	memcpy(m_pfGaintemp, m_CSpeechStatic->GetGain(), m_nQNum*sizeof(float));
#else
    if (aecdata.ChannelIndex_ == 1) {
        //m_CSpeechStatic->SetGmin(1);
    }
    
	m_CSpeechStatic->Porcess(m_pfPwr, m_pfNoise);
	// set m_pfTrans[0] = 0 , so m_fAllbandProb = 1
	m_pfTrans[0] = 0;
	m_CSpeechTransient->Porcess(m_pfPwr, m_pfTrans);

	float *gain_static = m_CSpeechStatic->GetGain();
	float *gain_transient = m_CSpeechTransient->GetGain();
	//FullBandCtrl(pfBef,pfAft,gain_transient);
	for (CAUDIO_U32_t i = 0; i < m_nQNum; ++i)
	{
		m_pfGaintemp[i] = min(gain_static[i], gain_transient[i]);
        //m_pfGaintemp[i] = gain_transient[i];
	}

#endif

	m_CPsd->CQSpread(m_pfGaintemp,m_pfGainout);	
    // to do: add rnn gain, min
    int valid_bin[512] = { 0 };
    float tmp_gain_low = 0.f;
    float tmp_gain_sum = 0.f;
    float tmp_psd_low = 0.f;
    float tmp_psd_sum = 0.f;

    for (CAUDIO_U32_t i = 0; i < m_nFFTLen/2; i++)
    {
        m_pfnrin_smooth[i] += 0.05 * (input[i] - m_pfnrin_smooth[i]);
        tmp_gain_sum += aecdata.RnnGain_[i];
        if (i < 86) { // 4kHz
            tmp_gain_low += aecdata.RnnGain_[i];
            tmp_psd_low += m_pfnrin_smooth[i];
        }
        tmp_psd_sum += m_pfnrin_smooth[i];        
    }

    tmp_gain_low /= 86;
    tmp_gain_sum /= (m_nFFTLen / 2);
    tmp_psd_low /= 86;
    tmp_psd_sum /= (m_nFFTLen / 2);

    for (CAUDIO_U32_t i = 0; i < m_nFFTLen / 2; i++)
    {
        if (tmp_gain_low < 0.2 && tmp_gain_sum < 0.2 && tmp_psd_low < 0.000001 && tmp_psd_sum < 0.000001) { // -60dB
            m_pfGainout[i] = min(m_pfGainout[i], aecdata.RnnGain_[i]);
        }

        if (tmp_gain_low < 0.1 && tmp_gain_sum < 0.2 && tmp_psd_low < 0.00001 && tmp_psd_sum < 0.000001) { // -60dB
            //m_pfGainout[i] = min(m_pfGainout[i], aecdata.RnnGain_[i]);
        }

        if (tmp_gain_low < 0.7 && tmp_gain_sum < 0.5 && tmp_psd_low < 100) {
            //m_pfGainout[i] = min(m_pfGainout[i], aecdata.RnnGain_[i]);
        }
    }
	aecdata.fProiSNR_ = m_CSpeechStatic->GetProiSNR();
	m_CPsd->CQSpread(m_pfNoise,m_pfNoiseLine);	
	aecdata.fNoisePwr_ = m_pfNoiseLine[0];
}
///////////////fullband control
void CNoiseRedu::FullBandCtrl(float *pfAft,float *pfBef,float* gain)
{
	float tmp_pwr_allband=0.f;
	float tmp_pwr_supress=0.f;
	m_CPsd->CQSpread(gain,m_pfGainout);	
	for (int i=1;i<m_nFFTLen/2;i++)
	{
		tmp_pwr_allband+=pfBef[i];
		tmp_pwr_supress+=pfAft[i]*m_pfGainout[i]*m_pfGainout[i];
	}
	float tempFullgain= 4.f*tmp_pwr_supress/(tmp_pwr_allband+minvalue);
	tempFullgain*=tempFullgain;
	if (tempFullgain<0.1f)
	{
		tempFullgain=0.1f;
	}else if (tempFullgain>1.f)
	{
		tempFullgain=1.f;
	}

	for (int i=0;i<m_nQNum;i++)
	{
		gain[i]*=tempFullgain;
	}
}
void CNoiseRedu::transientnois()
{
	int i;
	float temp=0.f;
	float threshold = 0.15;
	float gain_temp = 1.f;
	for (i=0;i<m_nQNum;i++)
	{  
		gain_temp = m_CPsd->m_pfPsdCQ_Fd[i] / (m_CPsd_echo->m_pfPsdCQ_Fd[i] + 1e-10);
		if (gain_temp < threshold && 0.001f < gain_temp)
			m_pfTransGain[i] += 0.1f *(gain_temp - m_pfTransGain[i]);
		temp=m_CPsd_echo->m_pfPsdCQ_Fd[i]* m_pfTransGain[i];
		m_pfTrans[i]=temp>m_pfNoise[i]/10.f?temp:m_pfNoise[i]/10.f;///for protect snr equal infinite  
		
	}
}

/*return noise level in dB after nr; noisebef is noise power befor nr;*/
float CNoiseRedu::NoisePwr_allband(float* Noisebef)
{
	float g;
	int i,maxloop;
	//float delt;
	float *fp=m_pfGainout;
	float temp;
	float temp_aft=0.f;
	float temp_bef=0.f;

	maxloop=m_nFFTLen/2;
  	
	/*attention: m_pfNoiseLine[0] is all band noise!!!!!!*/
	for (i=1;i<maxloop;i++)
	{
		g=fp[i];
		temp=m_pfNoiseLine[i];
;
		temp_bef+=temp;
		temp_aft+=temp*g*g;
	}
	temp_bef/=(2.f*16.f);
	temp_aft/=(2.f*16.f);
			
    *Noisebef=temp_bef;

	if (temp_aft>minvalue)
	{
		temp_aft=10.f*log10f(temp_aft);
	}
	else
		temp_aft=-260;
	return temp_aft;
	

}
float CNoiseRedu::Window(int n,float delt)
{
	/*line interpolation [1,-1]*/ 
	float temp;

    temp=1+0.8125f*0.8125f-2.f*0.8125f*cosf(n*delt*3.1415926f);
	temp+=minvalue;
	return (1.f/(temp));
}
/* update m_pfGainout from parents*/
void CNoiseRedu::ReUpdateGain(float *G)
{
	memcpy(m_pfGainout,G,m_nFFTLen/2*sizeof(float));
}
