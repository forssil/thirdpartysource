/*! \file     CDCRemoverImpl.cpp
*   \author   Keil
*   \brief    used to remove the DC component from audio stream.
*   \history  2015/3/10 created CDCRemoverImpl class.
*/

#include "DCRemoverImpl.h"
#include "audiotrace.h"

CDCRemoverImpl::CDCRemoverImpl(IN const int _fs_, IN const int _frame_len_, IN const int _thread_num_)
	: m_nFs(_fs_)
	, m_nFrameLen(_frame_len_)
	, m_nThreadNum(_thread_num_)
	, m_bIsInitSuccess(false)
	, m_cDCRemover(NULL)
	, m_ppInputBuf(NULL)
{
	__Init();
}


CDCRemoverImpl::~CDCRemoverImpl()
{
	if (m_cDCRemover)
	{
		delete m_cDCRemover;
		m_cDCRemover = NULL;
	}
	m_bIsInitSuccess = false;
}

//! initial function
bool CDCRemoverImpl::__Init(void)
{
	if (m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init operation failed");
		return false;
	}
	
	if (NULL != m_cDCRemover)
	{
		delete m_cDCRemover;
		m_cDCRemover = NULL;
	}
	m_cDCRemover = new DCRemover(m_nFs, m_nFrameLen, m_nThreadNum);
	if (NULL == m_cDCRemover)
	{
		AUDIO_PROCESSING_PRINTF("init operation failed");
		return false;
	}

	m_bIsInitSuccess = true;
	return true;
}

//! IAudioProcessImplBase API
/*virtual */int CDCRemoverImpl::process(IN audio_pro_share &aShareData)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init operation failed");
		return false;
	}
	//__AnalyticalStructure(aShareData);
	
	if(NULL != aShareData.pCapture_)
	{
		m_ppInputBuf=aShareData.pCapture_;
		for (CAUDIO_U32_t i = 0; i < aShareData.nChannelsInCapture_; i++)
		{
			m_cDCRemover->findLevelAndDcRemove(m_ppInputBuf[i], i);
		}

	}
	if(NULL != aShareData.pRender_)
	{
		m_ppInputBuf=aShareData.pRender_;
		for (CAUDIO_U32_t i = 0; i < aShareData.nChannelsInRender_; i++)
		{
			m_cDCRemover->findLevelAndDcRemove(m_ppInputBuf[i], i);
		}
		
	}
	
	if(NULL != aShareData.pShare_)
	{
		m_ppInputBuf=aShareData.pShare_;
		for (CAUDIO_U32_t i = 0; i < aShareData.nChannelsInShare_; i++)
		{
			m_cDCRemover->findLevelAndDcRemove(m_ppInputBuf[i], i);
		}
	}
	return 0;
}

//! Analytical structure
bool CDCRemoverImpl::__AnalyticalStructure(IN const audio_pro_share &aShareData)
{

	return true;
}