/*	
 *	Name:			AudioChannelBase.h
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/05/2015 Created			Zhong Yaozhu
 *
 */

#ifndef _AUDIO_CHANNEL_BASE_
#define _AUDIO_CHANNEL_BASE_

#include "AudioTributaryReceiver.h"
#include "MultipleSourceSync.h"

class AudioChannelBase : public IAudioModuleImpleBase
{
public:
	AudioChannelBase():m_bIsInitSuccess(false),m_bStarted(false){}

	virtual ~AudioChannelBase(){}

public:
	virtual bool Init() = 0;

	virtual bool Reset() = 0;

	virtual ModuleState Start() = 0;

	virtual ModuleState Stop() = 0;

	virtual bool FeedData(	
		const void* a_pData,
		CAUDIO_U32_t a_nDataSize,
		CAUDIO_U8_t a_nThreadIdx,
		CAUDIO_U32_t a_nOriginalSampleRate = 0) = 0;

	virtual bool Transport(
		const void* a_pData,
		DATA_TYPE a_eDataType,
		CAUDIO_U32_t a_nSize,
		TransportId_e a_eTPId) = 0;

protected:
	bool m_bIsInitSuccess; // initial flag
	bool m_bStarted; // start flag
};

#endif  // _AUDIO_CHANNEL_BASE_