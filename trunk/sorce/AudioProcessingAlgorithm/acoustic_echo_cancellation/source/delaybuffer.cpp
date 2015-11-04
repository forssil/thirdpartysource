
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
#include "delaybuffer.h"
#include<string.h>

CDelayBuffer::CDelayBuffer(int rows,int columns):
	 m_nWritInd(0)
	,m_nColumn(columns)
	,m_nRow(rows)
	,m_fpBuffer(NULL)
	,m_bInit(false)
	,m_VADBuff(NULL)
	,m_VADBandBuff(NULL)
	, m_VADBandBuffSize(0)
{
	
	
}
CDelayBuffer::~CDelayBuffer()
{
	if(m_fpBuffer)
		delete[] m_fpBuffer;

	if (m_VADBuff)
		delete[] m_VADBuff;

	if (m_VADBandBuff)
		delete[] m_VADBandBuff;
}
bool CDelayBuffer::Init()
{
	m_fpBuffer=new float[m_nRow*m_nColumn];
	m_VADBuff = new bool[m_nColumn];
	m_VADBandBuff = new float[VAD_BAND_NUM*m_nColumn];
	m_VADBandBuffSize = VAD_BAND_NUM*m_nColumn;
	
	if (NULL != m_fpBuffer && NULL != m_VADBuff && NULL != m_VADBandBuff)
	{
		m_bInit = true;
		Reset();
	}

	return m_bInit;
}
void CDelayBuffer::Reset()
{
	if(m_fpBuffer)
		memset(m_fpBuffer,0,sizeof(float)*m_nRow*m_nColumn);

	if (m_VADBuff)
		memset(m_VADBuff, 0, sizeof(bool)*m_nColumn);

	if (m_VADBandBuff)
		memset(m_VADBandBuff, 0, sizeof(float)*m_nColumn*VAD_BAND_NUM);
		

	m_nWritInd=0;
}

void CDelayBuffer::getAudioFrame(int offSet, Audioframe_t* pFrame)
{
	if (NULL == pFrame)
	{
		return;
	}

	float* fp = NULL;
	if (m_bInit)
	{
		int ind = 0;
		if (offSet < m_nColumn)
		{
			ind = m_nWritInd - offSet + m_nColumn;
			ind %= m_nColumn;
			fp = m_fpBuffer + ind*m_nRow;
		}

		pFrame->fp = fp;
		pFrame->VAD = m_VADBuff[ind];
		pFrame->VADBand = &m_VADBandBuff[ind*VAD_BAND_NUM];
		pFrame->VADBandBuffSize = VAD_BAND_NUM;
	}
}


int CDelayBuffer::UpdateData(Audioframe_t* pFrame)
{
	int ret=0;
	if(m_bInit)
	{
		m_nWritInd++;
		m_nWritInd%=m_nColumn;

		// copy audio data
		float *Bp=m_fpBuffer+m_nWritInd*m_nRow;
		float *tm_fp= pFrame->fp;
		for(int i=0;i<m_nRow;i+=2)
		{
			*Bp=*tm_fp;
			Bp++;
			tm_fp++;
			*Bp=*tm_fp;
			Bp++;
			tm_fp++;
		}

		// copy VAD value
		m_VADBuff[m_nWritInd] = pFrame->VAD;
		int vadBandBuffSize = pFrame->VADBandBuffSize;
		int vadBandBuffWriterPtr = m_nWritInd*VAD_BAND_NUM;
		for (int i = 0; 
			i < VAD_BAND_NUM && vadBandBuffWriterPtr < m_VADBandBuffSize && i < vadBandBuffSize;
			i++, vadBandBuffWriterPtr++)
		{
			m_VADBandBuff[vadBandBuffWriterPtr] = pFrame->VADBand[i];
		}
	}
	else
		ret=-1;
	return ret;
}
	

