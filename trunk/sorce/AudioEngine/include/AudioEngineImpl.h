/*	
 *	Name:			AudioEngineImpl.h
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					01/08/2015 Created									Zhong Yaozhu
 *					03/03/2015 add AudioTransport module				Zhong Yaozhu
 *					03/05/2015 add AudioProcessing module				Gu Cheng
 *					03/09/2015 move AudioTransport&AudioProcessing
 *							   into AudioCaptrueChannel module, add
 *							   AudioCaptrueChannel module to audio
 *							   engine									Zhong Yaozhu
 *					03/24/2-15 add AudioPlayback channel				Zhong Yaozhu
 *
 */

#ifndef _AUDIO_ENGINE_IMPL_
#define _AUDIO_ENGINE_IMPL_

#include <stdio.h>
#include "scoped_ptr.h"
#include "scoped_refptr.h"
#include "critical_section_wrapper.h"
#include "audiotypedef.h"
#include "IAudioTransport.h"
#include "IAudioEngine.h"
#include "AudioCaptureChannel.h"
#include "SharedChannel.h"
#include "AudioPlaybackChannel.h"
#include "AudioPropertyPage.h"
#include "IAudioDeviceManager.h"

class CAudioProcessingImpl;
class AudioEngineImpl : public IAudioEngine
{
typedef webrtc::CriticalSectionWrapper CriticalSectionWrapper;
typedef webrtc::scoped_ptr<CriticalSectionWrapper> CriticalSectionWrapperPtr;
typedef webrtc::scoped_ptr<AudioCaptureChannel> AudioCaptureChannelPtr;
typedef webrtc::scoped_ptr<CSharedChannel> AudioSharedChannelPtr;

public:

	AudioEngineImpl(
		AUDIO_TYPE_NUM_t a_sThreadNumInfo,
		CAUDIO_U8_t a_nInChannelNumPerThread,
		AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel,
		SSampleRate a_sSampleRate,
		DATA_TYPE a_eInputDataType);


	~AudioEngineImpl();

	// this API must be called after constructor is called
	bool Init(bool aec_switch, std::string folder_name);

// external API
public:
	bool RegisterAudioTransport(IAudioTransport* a_pAudioTransport, TransportId_e a_eTPId);
	bool DeregisterAudioTransport(TransportId_e a_eTPId);

	bool RegisterPropertyPage(IAudioPropertyPage* a_pAudioTransport);
	bool UnRegisterPropertyPage(IAudioPropertyPage* a_pAudioTransport);

	// the atomicity between FeedData and start&stop action 
	// is guarantee by outside
	bool FeedData(
		ChannelId_e a_eChannelId,
		const void* a_pData,
		CAUDIO_U32_t a_nDataSize,
		CAUDIO_U8_t a_nThreadIdx,
		CAUDIO_U32_t a_nOriginalSampleRate);

#if CAPTURE_PLAYBACK_INTEGRATED
	bool NeedData(
		AudioFrame*  pAudioFrameNeeded,
		const CAUDIO_U32_t nMaxFrameSize,
		const CAUDIO_U32_t nSamplesPerSec,
		const CAUDIO_U8_t nChannel);
#endif

	//! start implement
	ModuleState Start();
	//! stop implement
	ModuleState Stop();

private:
	//! reset function
	bool Reset();
	bool RegisterTrinutaryReceiverToSharedChannel();
	bool RegisterTrinutaryReceiverToPlaybackChannel();

private:
	AUDIO_TYPE_NUM m_sThreadNumInfo;
	AudioCaptureChannel* m_pCaptureChannel;
	AudioPlaybackChannel* m_pPlaybackChannel;
	CAUDIO_U8_t m_nCaptureChannelThreadNum;
	CAUDIO_U8_t m_nNetCaptureThreadNum;
	bool m_bIsRecordingEnable;
	CSharedChannel* m_pSharedChannel;
	CAUDIO_U8_t m_nSharedChannelThreadNum;
	CAUDIO_U8_t m_nChannelNumPerThread; // number of channels per thread
	CAUDIO_U16_t m_nFrameSizePerChannel; // frame size per channel
	DATA_TYPE m_eInputDataType; // input data type 
	CAUDIO_U32_t m_nSampleRate_InterProcessUsed; // samples per frame
	CAUDIO_U32_t m_nSampleRate_OnlineClassOut; // samples per frame
	CAUDIO_U32_t m_nSampleRate_SpeakerOut;
	CAUDIO_U32_t m_nSampleRate_RecorderOut;
	AUDIO_DATA_TYPE m_nFrameTimeMsPerChannel; // time of frame (ms)
	CriticalSectionWrapperPtr m_MemberLock; // critical section
	bool m_bIsInitSuccess; // initial flag
	bool m_bStarted; // start flag
	CAudioPropertyPage *m_pPropertyPage;

#if CAPTURE_PLAYBACK_INTEGRATED
	IAudioDeviceManager *m_pAudioDeviceManager;
#endif
};

#endif  // _AUDIO_ENGINE_IMPL_
