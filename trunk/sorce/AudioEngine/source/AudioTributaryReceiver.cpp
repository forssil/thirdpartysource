/*	
 *	Name:			AudioTributaryReceiver.cpp
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/05/2015 Created			Zhong Yaozhu
 *
 */

#include "AudioTributaryReceiver.h"
#include "audiotrace.h"

AudioTributaryReceiver::AudioTributaryReceiver(
	DATA_TYPE a_eDataType,
	CAUDIO_U32_t a_nFs,
	CAUDIO_U8_t a_nChannelNum)
	:
	m_eDataType(a_eDataType),
	m_nFs(a_nFs),
	m_nChannelNum(a_nChannelNum),
	m_pMulSrcSync(NULL),
	m_nThreadNum(0)
{
}


bool AudioTributaryReceiver::Transport(
	const void* a_pData,
	DATA_TYPE a_eDataType,
	CAUDIO_U32_t a_nSize,
	CAUDIO_U8_t a_nThreadIdx)
{
	if(a_eDataType != m_eDataType)
	{
		return false;
	}

	if(NULL != m_pMulSrcSync)
	{
		if(!m_pMulSrcSync->HasEverPulled())
		{
			// return without feeding any data to avoid accumulating
			// to much data in list.
			return true;
		}

		bool ret = m_pMulSrcSync->FeedData(a_pData, m_eDataType, a_nSize,
			m_nChannelNum, a_nThreadIdx);
		 return ret;
	}

	AUDIO_PROCESSING_PRINTF("m_pMulSrcSync is unregistered!");
	return true;
}

void AudioTributaryReceiver::RegisterMulSrcSync(MultipleSourceSync* a_pMulSrcSync)
{
	if(NULL == a_pMulSrcSync)
	{
		AUDIO_PROCESSING_PRINTF("a_pMulSrcSync is NULL, cannot be registered into AudioTributaryReceiver!");
		return;
	}

	if(MODE_PUll_ASYNC_CALL != a_pMulSrcSync->GetSyncMode())
	{
		AUDIO_PROCESSING_PRINTF("the mulSrcSync registered must be MODE_PUll_ASYNC_CALL mode");
		return;
	}

	m_pMulSrcSync = a_pMulSrcSync;
}

void AudioTributaryReceiver::UnregisterMulSrcSync()
{
	m_pMulSrcSync = NULL;
}


