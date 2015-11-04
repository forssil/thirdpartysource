/*	
 *	Name:			AudioCaptureChannel.h
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/06/2015 Created			Zhong Yaozhu
 *
 */

#ifndef _AUDIO_CAPTURE_CHANNEL_
#define _AUDIO_CAPTURE_CHANNEL_

#include "audiotypedef.h"
#include "AudioTransportImpl.h"
#include "IAudioTransport.h"
#include "MultipleSourceSync.h"
#include "AudioChannelBase.h" 
#include "AudioTributaryReceiver.h"
#include "AudioProcessingImpl.h"

class AudioCaptureChannel : public AudioChannelBase
{
typedef webrtc::scoped_ptr<MultipleSourceSync> MultipleSourceSyncPtr;
typedef webrtc::scoped_ptr<AudioTransportImpl> AudioTransportImplPtr;
typedef webrtc::scoped_ptr<AudioTributaryReceiver> AudioTributaryReceiverPtr;
typedef webrtc::scoped_ptr<CAudioProcessingImpl> AudioProcessingPtr;
typedef webrtc::CriticalSectionWrapper CriticalSectionWrapper;
typedef webrtc::scoped_ptr<CriticalSectionWrapper> CriticalSectionWrapperPtr;

public:
	AudioCaptureChannel(
		CAUDIO_U8_t a_nCaptureThreadNum,
		CAUDIO_U8_t a_nChannelNumPerCaptureThread,
		CAUDIO_U8_t a_nChannelNumPerSharedThread,
		DATA_TYPE a_eInputDataType,
		CAUDIO_U32_t a_nSampleRate_InternalProcessUsed,
		CAUDIO_U32_t a_nSampleRate_OnlineClassOut,
		CAUDIO_U32_t a_nSampleRate_RecorderOut,
		AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel,
		AUDIO_PROPERTY_PAGE *a_pPropertyPage,
		bool a_bIsPlayChEnable,
		bool a_bIsSharedChEnable,
		bool a_bIsRecordingEnable,
		std::string a_sFolderName);

	virtual ~AudioCaptureChannel(){};

// AudioChannelBase virtual API
public:
	// this API must be called after constructor is called
	virtual bool Init();

	virtual bool Reset();

	// sync between start and stop will be guaranteed by audio engine layer
	virtual ModuleState Start();

	virtual ModuleState Stop();

	//virtual bool FeedData(	
	//	const void* a_pData,
	//	CAUDIO_U32_t a_nDataSize,
	//	CAUDIO_U8_t a_nThreadIdx);

	virtual bool FeedData(
		const void* a_pData,
		CAUDIO_U32_t a_nDataSize,
		CAUDIO_U8_t a_nThreadIdx,
		CAUDIO_U32_t a_nOriginalSampleRate = 0);

	virtual bool Transport(
		const void* a_pData,
		DATA_TYPE a_eDataType,
		CAUDIO_U32_t a_nSize,
		TransportId_e a_eTPId);

	virtual bool processData(
		const AudioFrame *a_AudioFrameArray[], 
		CAUDIO_U8_t a_nArrayLen);

public:

	bool RegisterAudioTransport(IAudioTransport* a_pAudioTransport, TransportId_e a_nTPId);

	bool DeregisterAudioTransport(TransportId_e a_eTPId);

	AudioTributaryReceiver* GetAudioTributaryReceiver();

private:
	AudioProcessingPtr m_AudioProcessingPtr;
	MultipleSourceSyncPtr m_DeviceSrcSyncPtr;
	MultipleSourceSyncPtr m_TributarySrcSyncPtr;
	AudioTransportImplPtr m_NetworkTransportPtr;	// output
	AudioTransportImplPtr m_RecordingTransportPtr;	// input
	AudioTributaryReceiverPtr m_TributaryDataReceiverPtr;	// input

private:
	CAUDIO_U8_t m_nCaptureThreadNum;
	CAUDIO_U8_t m_nChannelNumPerCaptureThread;
	CAUDIO_U8_t m_nChannelNumPerSharedThread; 
	CAUDIO_U16_t m_nFrameSizePerChannel;
	DATA_TYPE m_eInputDataType;
	CAUDIO_U32_t m_nSampleRate_InternalProcessUsed;
	CAUDIO_U32_t m_nSampleRate_OnlineClassOut;
	CAUDIO_U32_t m_nSampleRate_RecorderOut;
	AUDIO_DATA_TYPE m_nFrameTimeMsPerChannel;
	AudioFrame m_SharedChannelFrameBuff[MAX_THREAD_NUM];
	AUDIO_PROPERTY_PAGE *m_pPropertyPage;
	bool m_bIsPlayChEnable;
	bool m_bIsSharedChEnable;
	bool m_bIsRecordingEnable;
	std::string m_sFolderName;
};

#endif  // _AUDIO_CAPTURE_CHANNEL_



