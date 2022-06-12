// PitchTrack.cpp: implementation of the CPitchTrack class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "PitchTrack.h"

CPitchTrack::CPitchTrack(int nFFTLen, int nHalfFFTLen,int fs)
				:m_nHalfFFTLen(nHalfFFTLen)
{   
	m_pfPitch = NULL;	m_pHarmonicsNum = NULL;
	m_nPfStartBin = 3;
	m_bValid =1;
	m_nfPitchStart = 100.0f;
	m_nfPitchEnd   = 400.0f;
	m_nfPitchStep  = 10.0f;
	m_nfPitchCnt   = int((m_nfPitchEnd-m_nfPitchStart)/m_nfPitchStep+1.0f);
	m_nfTrackEnd   = 2000.0f;
	m_nMaxHarmonicsCnt = int(m_nfTrackEnd/m_nfPitchStart+1.0f);
    m_nInvBinFFT = 1.0f*nFFTLen/fs;
	int i;
	m_pfPitch = new float[m_nfPitchCnt * 2 + m_nHalfFFTLen + m_nMaxHarmonicsCnt];
	memset(m_pfPitch,0, sizeof(float) *(m_nfPitchCnt * 2 + m_nHalfFFTLen + m_nMaxHarmonicsCnt));


	if(!m_pfPitch) {m_bValid =0;	return;}
	m_pHarmonicsBuf		= m_pfPitch				+ m_nfPitchCnt;
	m_pHarmonicsFlagBuf = m_pHarmonicsBuf		+ m_nfPitchCnt;
	m_nfWeightArray		= m_pHarmonicsFlagBuf	+ m_nHalfFFTLen;
	m_nfWeightArray[0]= 1.f;
    m_nfWeightArray[1]= 0.915f;
    m_nfNearEnergy = 0;
    for(i=2;i<m_nMaxHarmonicsCnt;i++){
        m_nfWeightArray[i]=m_nfWeightArray[i-1]*m_nfWeightArray[1];
	}
    
	m_pHarmonicsNum = new int[m_nfPitchCnt];
	if(!m_pHarmonicsNum) {m_bValid =0;	return;}
	for( i=0;i<m_nfPitchCnt;i++){
		m_pfPitch[i]  = m_nfPitchStart+ i*m_nfPitchStep;
		m_pHarmonicsNum[i] = int(m_nfTrackEnd/(m_pfPitch[i]));	// get floor int
	}
	is_echo = true;
	is_noise = true;
	is_pitch = false;
}

CPitchTrack::~CPitchTrack()
{
	if (m_pfPitch) { 
		delete [] m_pfPitch; 
		m_pfPitch = NULL; 
	}
	if (m_pHarmonicsNum) {
		delete [] m_pHarmonicsNum;
		m_pHarmonicsNum = NULL;
	}
}

float CPitchTrack::GetPitch(float *fAmp)
{
	int index, j, i;
	memset(m_pHarmonicsBuf, 0, sizeof(float) * m_nfPitchCnt);
	for(i=0;i<m_nfPitchCnt;i++){
		for(j=1;j<=m_pHarmonicsNum[i];j++){
			index = int(m_pfPitch[i]*j*m_nInvBinFFT+0.5f);
			m_pHarmonicsBuf[i] += m_nfWeightArray[j-1]*fAmp[index];
		}
	}
	int max_bin = 0;
	float tmpMax = m_pHarmonicsBuf[0];
	for (int i = 1; i < m_nfPitchCnt; i++) {
		if (m_pHarmonicsBuf[i] > tmpMax) {
			tmpMax = m_pHarmonicsBuf[i];
			max_bin = i;
		}
	}
	return m_pfPitch[max_bin];
}

void CPitchTrack::GetPitchGain(float *fModGain, float *fPwrRatio, float *fStateNoies)
{
    int j,i,index;
	float *px, *py, *pz;
	if (m_nfPitchFre < m_nfPitchStart || m_nfPitchFre > m_nfPitchEnd)
		return;
	m_nEnhanceLowBin= int(600.0/m_nfPitchFre+0.5f);
	//m_nfMaxEnhBin =int(3.2*(m_pHarmonicsNum[max_bin]))+3;//When 16k,DSP process
	m_nfMaxEnhBin = int(4000.0 / m_nfPitchFre + 0.5f);//2*m_nfTrackEnd=4000Hz;
    int BinUp = int(m_nfPitchFre*m_nfMaxEnhBin*m_nInvBinFFT+0.5f)+1;

	//m_pHarmonicsFlagBuf as tmp-buf
	px = fPwrRatio + m_nPfStartBin;
	py = fModGain + m_nPfStartBin;
	pz = m_pHarmonicsFlagBuf + m_nPfStartBin;
	for (i = 0; i < (BinUp - m_nPfStartBin); i++)
		*pz++ = *px++ * *py++;
	
	memset(m_pHarmonicsFlagBuf, 0,sizeof(float)*m_nHalfFFTLen);
	for(j=1;j<=m_nEnhanceLowBin;j++){
		index = int(m_nfPitchFre*j*m_nInvBinFFT+0.5f);
        if((fStateNoies[index]<0.3f)){//Do H
            if (fModGain[index] < 1)
                fModGain[index] = 1;
        }
		m_pHarmonicsFlagBuf[index-1] =  1;
		m_pHarmonicsFlagBuf[index]   =  1;
		m_pHarmonicsFlagBuf[index+1] =  1;
	}
	
	for(j=m_nEnhanceLowBin+1;j<=m_nfMaxEnhBin;j++)
	{
		index = int(m_nfPitchFre*j*m_nInvBinFFT+0.5f);
		if((fStateNoies[index]<0.3f)){//Do H
            if (fModGain[index] < fPwrRatio[index])
                fModGain[index] = fPwrRatio[index];
		}
		m_pHarmonicsFlagBuf[index-1] =  1;
		m_pHarmonicsFlagBuf[index]   =  1;
		m_pHarmonicsFlagBuf[index+1] =  1;
	}
}
//void CPitchTrack::GetPitchGain(float *fAmp,float *fModGain, float *fPwrRatio, float *fStateNoies)
//{
//	int j,i,index;
//	GetPitch(fAmp);
//	m_nEnhanceLowBin= int(600.0/m_nfPitchFre+0.5f);
//    int BinUp = int(m_nfPitchFre*m_nfMaxEnhBin*m_nInvBinFFT+0.5f)+1;
//
//	//m_pHarmonicsFlagBuf as tmp-buf
//	V_Mult(fPwrRatio+m_nPfStartBin,fModGain+m_nPfStartBin,m_pHarmonicsFlagBuf+m_nPfStartBin,(BinUp-m_nPfStartBin));//
// 	float nMinGain = V_GetMin(m_pHarmonicsFlagBuf+m_nPfStartBin,(BinUp-m_nPfStartBin));
//	
//	memset(m_pHarmonicsFlagBuf,0,sizeof(float)*m_nHalfFFTLen);
//	for(j=1;j<=m_nEnhanceLowBin;j++){
//		index = int(m_nfPitchFre*j*m_nInvBinFFT+0.5f);
//		if((fStateNoies[index]<0.3f))//Do H
//			fModGain[index] = 1;
//		m_pHarmonicsFlagBuf[index-1] =  1;
//		m_pHarmonicsFlagBuf[index]   =  1;
//		m_pHarmonicsFlagBuf[index+1] =  1;
//	}
//	
//	for(j=m_nEnhanceLowBin+1;j<=m_nfMaxEnhBin;j++)
//	{
//		index = int(m_nfPitchFre*j*m_nInvBinFFT+0.5f);
//		if((fStateNoies[index]<0.3f)){//Do H
//			fModGain[index] = fPwrRatio[index];
//		}	
//		m_pHarmonicsFlagBuf[index-1] =  1;
//		m_pHarmonicsFlagBuf[index]   =  1;
//		m_pHarmonicsFlagBuf[index+1] =  1;
//	}
//	
//	for (i=m_nPfStartBin;i<BinUp;i++){
//		fModGain[i] = ((1-m_pHarmonicsFlagBuf[i])* nMinGain + m_pHarmonicsFlagBuf[i]*fModGain[i]);
//	}
// }
void CPitchTrack::UpdateProb(float sout_echo, float echo_sin, int far_activity) {
	float b, NoiseProbility, tmp, E = 0;
	float DelFre;
	for (int i = 0; i < PREVLENGTH; i++)
		E += m_nfPrevPitchFre[i];
	E /= PREVLENGTH;
	b = (m_nfPitchFre - E > 0) ? (m_nfPitchFre - E) : (E - m_nfPitchFre);
	if (b > GROSSPITCHTHRES) {
		NoiseProbility = 1;
		tmp = 0.1*m_nfPitchFre + 0.9 * E;
	}
	else {
		tmp = 0.7*m_nfPitchFre + 0.3 * E;
		NoiseProbility = b / GROSSPITCHTHRES;
	}
	for (int i = PREVLENGTH - 1; i > 0; i--) {
		m_nfPrevPitchFre[i] = m_nfPrevPitchFre[i - 1];
	}
	m_nfPrevPitchFre[0] = tmp;
	is_noise = (NoiseProbility > 0.6) ? true : false;
	sout_echo = (sout_echo > 0) ? sout_echo : (-sout_echo);
	echo_sin = (echo_sin > 0) ? echo_sin : (-echo_sin);
	DelFre = (m_nfEchoPitchFre - m_nfPitchFre > 0) ? (m_nfEchoPitchFre - m_nfPitchFre) : (m_nfPitchFre - m_nfEchoPitchFre);
	if (sout_echo > 0.1 || echo_sin < 0.7 || DelFre > DELPITCHTHRES)
		is_echo = true;
	else
		is_echo = false;
	if(!far_activity)
		is_echo = false;
	if (!is_echo && !is_noise)
		is_pitch = true;
	else
		is_pitch = false;
	if (m_nfPitchFre < m_nfPitchStart || m_nfPitchFre > m_nfPitchEnd)
		is_pitch = false;
}

void CPitchTrack::Process(float *AmpEst, float *AmpErr, float corr_sout_echo, float corr_echo_sin, bool far_activity) {
	m_nfEchoPitchFre = GetPitch(AmpEst);
	m_nfPitchFre = GetPitch(AmpErr);
	UpdateProb(corr_sout_echo, corr_echo_sin, far_activity);
}