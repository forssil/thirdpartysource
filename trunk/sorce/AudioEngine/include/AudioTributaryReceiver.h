/*	
 *	Name:			AudioTributaryReceiver.h
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/05/2015 Created			Zhong Yaozhu
 *
 */

#ifndef _AUDIO_TRIBUTARY_RECEIVER_
#define _AUDIO_TRIBUTARY_RECEIVER_

#include "audiotrace.h"
#include "IAudioTransport.h"
#include "MultipleSourceSync.h"

class AudioTributaryReceiver : public IAudioTransport
{
typedef webrtc::CriticalSectionWrapper CriticalSectionWrapper;
typedef webrtc::scoped_ptr<CriticalSectionWrapper> CriticalSectionWrapperPtr;

public:
	AudioTributaryReceiver(
		DATA_TYPE a_eDataType,
		CAUDIO_U32_t a_nFs,
		CAUDIO_U8_t a_nChannelNum
		);

	~AudioTributaryReceiver(){};

public:
	virtual bool Transport(
		const void* a_pData,
		DATA_TYPE a_eDataType,
		CAUDIO_U32_t a_nSize,
		CAUDIO_U8_t a_nThreadIdx);

	void RegisterMulSrcSync(MultipleSourceSync* a_pMulSrcSync);

	void UnregisterMulSrcSync();

	CAUDIO_U8_t SetDSId(DataSyncId_e a_eDSId)
	{
		if(NULL == m_pMulSrcSync)
		{
			AUDIO_PROCESSING_PRINTF("m_pMulSrcSync is NULL!");
		}

		m_pMulSrcSync->SetDSId(a_eDSId, m_nThreadNum);

		CAUDIO_U8_t threadIdx = m_nThreadNum;
		++m_nThreadNum;
		return threadIdx;
	}

private:
	MultipleSourceSync* m_pMulSrcSync;
	DATA_TYPE m_eDataType;
	CAUDIO_U32_t m_nFs;
	CAUDIO_U8_t m_nChannelNum;
	CAUDIO_U8_t m_nThreadNum;	// increase when new DSId is set, indicate how many thread is involded in
};

#endif  // _AUDIO_TRIBUTARY_RECEIVER_