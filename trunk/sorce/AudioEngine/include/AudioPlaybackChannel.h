/*	
 *	Name:			AudioPlaybackChannel.h
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/24/2015 Created			Zhong Yaozhu
 *
 */

#ifndef _AUDIO_PLAYBACK_CHANNEL_
#define _AUDIO_PLAYBACK_CHANNEL_

#include "audiotypedef.h"
#include "AudioTransportImpl.h"
#include "IAudioTransport.h"
#include "MultipleSourceSync.h"
#include "AudioChannelBase.h" 
#include "AudioTributaryReceiver.h"
#include "AudioProcessingImpl.h"


class AudioPlaybackChannel : public AudioChannelBase
{
typedef webrtc::scoped_ptr<MultipleSourceSync> MultipleSourceSyncPtr;
typedef webrtc::scoped_ptr<AudioTransportImpl> AudioTransportImplPtr;
typedef webrtc::scoped_ptr<AudioTributaryReceiver> AudioTributaryReceiverPtr;
typedef webrtc::scoped_ptr<PlaybackAudioProcessing> AudioProcessingPtr;
typedef webrtc::CriticalSectionWrapper CriticalSectionWrapper;
typedef webrtc::scoped_ptr<CriticalSectionWrapper> CriticalSectionWrapperPtr;

public:
	AudioPlaybackChannel(
		CAUDIO_U8_t a_nPlaybackThreadNum,
		CAUDIO_U8_t a_nSharedOutputThreadNum,
		CAUDIO_U8_t a_nChannelNumPerNetworkThread,
		CAUDIO_U8_t a_nChannelNumPerSharedThread,
		DATA_TYPE a_eInputDataType,
		CAUDIO_U32_t a_nSampleRate_InternalProcessUsed,
		CAUDIO_U32_t a_nSampleRate_SpeakerOut,
		AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel);

	virtual ~AudioPlaybackChannel(){};

// AudioChannelBase virtual API
public:
	// this API must be called after constructor is called
	virtual bool Init();

	virtual bool Reset();

	// sync between start and stop will be guaranteed by audio engine layer
	virtual ModuleState Start();

	virtual ModuleState Stop();

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


#if CAPTURE_PLAYBACK_INTEGRATED
	virtual bool processData(
		AudioFrame a_AudioFrameArray[],
		CAUDIO_U8_t a_nArrayLen);

	bool NeedData(
		AudioFrame*  pAudioFrameNeeded,
		const CAUDIO_U32_t nMaxFrameSize,
		const CAUDIO_U32_t nSamplesPerSec,
		const CAUDIO_U8_t nChannel);
#endif


public:

	bool RegisterAudioTransport(IAudioTransport* a_pAudioTransport, TransportId_e a_eTPId, CAUDIO_U8_t a_nAssignedIdx=0);

	bool DeregisterAudioTransport(TransportId_e a_eTPId);

	AudioTributaryReceiver* GetAudioTributaryReceiver();

private:
	AudioProcessingPtr m_AudioProcessingPtr;
	MultipleSourceSyncPtr m_DeviceSrcSyncPtr;
	MultipleSourceSyncPtr m_SharedChSrcSyncPtr;
	AudioTransportImplPtr m_RenderDevTransportPtr; // output
	IAudioTransport* m_pReceiverInCaptureCh; // output
	CAUDIO_U8_t m_nTributaryIdxToCaptureCh;
	AudioTributaryReceiverPtr m_SharedDataReceiverPtr; // input

private:
	CAUDIO_U8_t m_nNetCaptureThreadNum;
	CAUDIO_U8_t m_nSharedOutputThreadNum;
	CAUDIO_U8_t m_nChannelNumPerNetworkThread;
	CAUDIO_U8_t m_nChannelNumPerSharedThread; 
	CAUDIO_U16_t m_nFrameSizePerChannel;
	DATA_TYPE m_eInputDataType;
	CAUDIO_U32_t m_nSampleRate_InternalProcessUsed;
	CAUDIO_U32_t m_nSampleRate_SpeakerOut;
	AUDIO_DATA_TYPE m_nFrameTimeMsPerChannel;
	AudioFrame m_SharedChannelFrameBuff[MAX_THREAD_NUM];
};


#endif  // _AUDIO_CAPTURE_CHANNEL_
