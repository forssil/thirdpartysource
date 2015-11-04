/***********************************************************************
*  Author
;*      Gao Hua
;*
;*
;*
;*  History
;*      10/17/2014 Created
;*
;*
;*************************************************************************/

#include "DCRemover.h"

DCRemover::DCRemover(int Fs, int frameLen, int chn)
{
	m_nChn = chn;
	m_nFRAMESIZE =frameLen;// int(Fs*frame_ms / 1000);
	PrePosAnaInit();
	m_nPreEnhance = 1;
};

DCRemover::~DCRemover()
{
	if (NULL != m_fpDCRemov)
	{
		delete[] m_fpDCRemov;
		m_fpDCRemov = NULL;
	}

	if (NULL != m_fpDCRemovPre)
	{
		delete[] m_fpDCRemovPre;
		m_fpDCRemovPre = NULL;
	}
}

void DCRemover::PrePosAnaInit()
{
	m_fInvFRAMESIZE = 1.f / float(m_nFRAMESIZE);

	m_fpDCRemov = new float[m_nChn];;
	memset((void*)m_fpDCRemov, 0, m_nChn*sizeof(float));

	m_fpDCRemovPre = new float[m_nChn];;
	memset((void*)m_fpDCRemovPre, 0, m_nChn*sizeof(float));
}


void DCRemover::findLevelAndDcRemove(float *pData, int ch)

{
	float tmp1, tmp2, out1, out2;
	//float fp1,fp2;
	float fm1, fm2;
	float *bufptr;
	float DcOffset1, DcOffset2;
	float DcRemoveStepsize, DcRemoveStepsizex2, meanLevel;
	int k;

	/* init DC-remove variables */
	DcRemoveStepsize = (m_fpDCRemov[ch] - m_fpDCRemovPre[ch]) * m_fInvFRAMESIZE;
	DcRemoveStepsizex2 = 2.0f * DcRemoveStepsize;


	fm1 = 0.0f;
	fm2 = 0.0f;

	//fp1 = 0.0f;
	//fp2 = 0.0f;

	bufptr = pData;

	DcOffset1 = m_fpDCRemovPre[ch] + DcRemoveStepsize;
	DcOffset2 = m_fpDCRemovPre[ch] + DcRemoveStepsizex2;

	for (k = 0; k < m_nFRAMESIZE; k += 2)
	{
		tmp1 = *bufptr;
		fm1 += tmp1;
		out1 = tmp1 - DcOffset1;
		*bufptr++ = out1;
		//fp1       += fabsf(out1);
		DcOffset1 += DcRemoveStepsizex2;

		tmp2 = *bufptr;
		fm2 += tmp2;
		out2 = tmp2 - DcOffset2;
		*bufptr++ = out2;
		//fp2       += fabsf(out2);
		DcOffset2 += DcRemoveStepsizex2;
	}
	/* After adjust */
	meanLevel = (fm1 + fm2) * m_fInvFRAMESIZE;
	//AbsLevel[ch] = (fp1+fp2) * m_fInvFRAMESIZE;
	/* Compute amount the signal is to be adjusted nest time DcReomve runs because of DC components*/
	m_fpDCRemovPre[ch] = m_fpDCRemov[ch];
	m_fpDCRemov[ch] += (meanLevel - m_fpDCRemov[ch]) * DCREMOVERATE;

}