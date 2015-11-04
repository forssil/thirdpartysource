/*! \file   AudioProcessingImpl.h
*   \author Gao Hua
*   \date   2015/3/31
*   \brief  Audio Processing interface class
*/
#ifndef _IAUDIOENGINE_
#define _IAUDIOENGINE_

#include <iostream>

class IAudioPropertyPage;
class IAudioTransport;
class AudioDeviceTransport;

class IAudioEngine
{
public:
	static IAudioEngine* Create(AUDIO_TYPE_NUM_t m_sThreadNumInfo,
		CAUDIO_U8_t a_nInChannelNumPerThread,
		AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel,
		SSampleRate a_sSampleRate,
		DATA_TYPE a_eInputDataType);

	 static bool Delete(IAudioEngine*& voiceEngine);

	 virtual bool Init(bool aec_switch, std::string folder_name) = 0;

	 // external API
public:
	virtual bool RegisterAudioTransport(IAudioTransport* a_pAudioTransport, TransportId_e a_eTPId)=0;
	virtual bool DeregisterAudioTransport(TransportId_e a_eTPId)=0;

	virtual bool RegisterPropertyPage(IAudioPropertyPage* a_pAudioTransport)=0;
	virtual bool UnRegisterPropertyPage(IAudioPropertyPage* a_pAudioTransport)=0;

	// the atomicity between FeedData and start&stop action 
	// is guarantee by outside
	virtual bool FeedData(
		ChannelId_e a_eChannelId,
		const void* a_pData,
		CAUDIO_U32_t a_nDataSize,
		CAUDIO_U8_t a_nThreadIdx,
		CAUDIO_U32_t a_nOriginalSampleRate) = 0;

#if CAPTURE_PLAYBACK_INTEGRATED
	virtual bool NeedData(
		AudioFrame*  pAudioFrameNeeded,
		const CAUDIO_U32_t nMaxFrameSize,
		const CAUDIO_U32_t nSamplesPerSec,
		const CAUDIO_U8_t nChannel) = 0;
#endif

	//! start implement
	virtual ModuleState Start()=0;
	//! stop implement
	virtual ModuleState Stop()=0;

protected:
	IAudioEngine(){};
	~IAudioEngine(){};
};
#endif //_IAUDIOENGINE_