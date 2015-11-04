/*	
 *	Name:			AudioPlaybackChannel.cpp
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/24/2015 Created			Zhong Yaozhu
 *
 *
 */

#include "AudioPlaybackChannel.h"
#include "audiotrace.h"

#define INVALID_TRIBUTARY_THREAD_INDEX 0xFF

AudioPlaybackChannel::AudioPlaybackChannel(
	CAUDIO_U8_t a_nNetCaptureThreadNum,
	CAUDIO_U8_t a_nSharedOutputThreadNum,
	CAUDIO_U8_t a_nChannelNumPerNetworkThread,
	CAUDIO_U8_t a_nChannelNumPerSharedThread,
	DATA_TYPE a_eInputDataType,
	CAUDIO_U32_t a_nSampleRate_InternalProcessUsed,
	CAUDIO_U32_t a_nSampleRate_SpeakerOut,
	AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel)
	:
	m_nNetCaptureThreadNum(a_nNetCaptureThreadNum),
	m_nSharedOutputThreadNum(a_nSharedOutputThreadNum),
	m_nChannelNumPerNetworkThread(a_nChannelNumPerNetworkThread),
	m_nChannelNumPerSharedThread(a_nChannelNumPerSharedThread),
	m_eInputDataType(a_eInputDataType),
	m_nSampleRate_InternalProcessUsed(a_nSampleRate_InternalProcessUsed),
	m_nSampleRate_SpeakerOut(a_nSampleRate_SpeakerOut),
	m_nFrameTimeMsPerChannel(a_nFrameTimeMsPerChannel),
	m_nFrameSizePerChannel(FRAM_LEN(a_nFrameTimeMsPerChannel, a_nSampleRate_InternalProcessUsed)),
	m_pReceiverInCaptureCh(NULL),
	m_nTributaryIdxToCaptureCh(INVALID_TRIBUTARY_THREAD_INDEX)
	{}


// this API must be called after constructor is called
bool AudioPlaybackChannel::Init()
{
	// alloc audio processing implementation
	m_AudioProcessingPtr.reset(new PlaybackAudioProcessing(
		this,
		m_nNetCaptureThreadNum + m_nSharedOutputThreadNum,
		m_nChannelNumPerNetworkThread,
		m_nFrameTimeMsPerChannel,
		m_nSampleRate_InternalProcessUsed,
		m_eInputDataType,
		RenderCh,
		m_nNetCaptureThreadNum));
	if(NULL == m_AudioProcessingPtr.get())
	{
		return false;
	}
	if (false == m_AudioProcessingPtr->__Init())
	{
		return false;
	}

	// alloc multiple source sync

#if CAPTURE_PLAYBACK_INTEGRATED
	// pull mode
	m_DeviceSrcSyncPtr.reset(new MultipleSourceSync(
		this,
		m_nNetCaptureThreadNum,
		m_nChannelNumPerNetworkThread,
		m_nFrameTimeMsPerChannel,
		m_nSampleRate_InternalProcessUsed,
		MODE_PUll_ASYNC_CALL,
		REAL_FLOAT_DATA,
		RenderCh));
#else
	// push mode
	m_DeviceSrcSyncPtr.reset(new MultipleSourceSync(
		this,
		m_nNetCaptureThreadNum,
		m_nChannelNumPerNetworkThread,
		m_nFrameTimeMsPerChannel,
		m_nSampleRate_InternalProcessUsed,
		MODE_PUSH_ASYNC_CALL,
		REAL_FLOAT_DATA,
		RenderCh));
#endif

	if(NULL == m_DeviceSrcSyncPtr.get())
	{
		return false;
	}
	if(!m_DeviceSrcSyncPtr->Init())
	{
		AUDIO_PROCESSING_PRINTF("init Device Src Sync failed");
		return false;
	}
	m_DeviceSrcSyncPtr->SetDSId(DSFromNetwork);

	// pull mode
	if(0 != m_nSharedOutputThreadNum)
	{
		// alloc shared channel source sync
		m_SharedChSrcSyncPtr.reset(new MultipleSourceSync(
			NULL,
			m_nSharedOutputThreadNum,
			m_nChannelNumPerSharedThread,
			m_nFrameTimeMsPerChannel,
			m_nSampleRate_InternalProcessUsed,
			MODE_PUll_ASYNC_CALL,
			REAL_FLOAT_DATA,
			RenderCh));
		if(NULL == m_SharedChSrcSyncPtr.get())
		{
			return false;
		}
		if(!m_SharedChSrcSyncPtr->Init())
		{
			AUDIO_PROCESSING_PRINTF("init Shared Channel Src Sync failed");
			return false;
		}
		// DSId will be set through tributary receiver when tributary source sync
		// is registered to shared channel & playback channel
		m_SharedChSrcSyncPtr->SetDSId(InvalidDS); 

		// alloc tributary receiver
		m_SharedDataReceiverPtr.reset(new AudioTributaryReceiver(
			REAL_FLOAT_DATA,
			m_nSampleRate_InternalProcessUsed,
			m_nChannelNumPerSharedThread));
		if(NULL == m_SharedDataReceiverPtr.get())
		{
			return false;
		}
		m_SharedDataReceiverPtr->RegisterMulSrcSync(m_SharedChSrcSyncPtr.get());
	}

	// alloc network transport
	m_RenderDevTransportPtr.reset(new AudioTransportImpl(m_nSampleRate_InternalProcessUsed, m_nSampleRate_SpeakerOut));
	if(NULL == m_RenderDevTransportPtr.get())
	{
		return false;
	}

	m_bIsInitSuccess = true;
	return true;
}

bool AudioPlaybackChannel::Reset()
{
	if (m_AudioProcessingPtr.get())
	{
		m_AudioProcessingPtr->Reset();
	}

	return true;
}

ModuleState AudioPlaybackChannel::Start()
{
	if(!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("AudioPlaybackChannel init failed");
		return Audio_Module_State_Dead;
	}

	if(m_bStarted)
	{
		AUDIO_PROCESSING_PRINTF("The module already started!");
		return Audio_Module_State_Unknown;
	}

	Reset();

	ModuleState ret = m_DeviceSrcSyncPtr->StartProcess();
	if(Audio_Module_State_Running != ret)
	{
		AUDIO_PROCESSING_PRINTF("start device source sync failed as multiple src sync module is not running");
		return ret;
	}

	if(NULL != m_SharedChSrcSyncPtr.get())
	{
		ret = m_SharedChSrcSyncPtr->StartProcess();
		if(Audio_Module_State_Running != ret)
		{
			AUDIO_PROCESSING_PRINTF("start shared channel source sync failed as multiple src sync module is not running");
			return ret;
		}
	}

	m_bStarted = true;
	return Audio_Module_State_Running;
}

ModuleState AudioPlaybackChannel::Stop()
{
	m_DeviceSrcSyncPtr->StopProcess();

	if(!m_bStarted)
	{
		AUDIO_PROCESSING_PRINTF("this module alredy stopped");
		return Audio_Module_State_Stopped;
	}

	if(NULL != m_SharedChSrcSyncPtr.get())
	{
		m_SharedChSrcSyncPtr->StopProcess();
	}
	return Audio_Module_State_Stopped;
}

AudioTributaryReceiver* AudioPlaybackChannel::GetAudioTributaryReceiver()
{
	return m_SharedDataReceiverPtr.get(); 
}

bool AudioPlaybackChannel::FeedData(	
	const void* a_pData,
	CAUDIO_U32_t a_nDataSize,
	CAUDIO_U8_t a_nThreadIdx,
	CAUDIO_U32_t a_nOriginalSampleRate)
{
	if(m_bIsInitSuccess)
	{
		if(NULL != m_DeviceSrcSyncPtr.get())
		{
			return m_DeviceSrcSyncPtr->FeedData(a_pData, m_eInputDataType, a_nDataSize,
				m_nChannelNumPerNetworkThread, a_nThreadIdx, a_nOriginalSampleRate);
		}
	}

	return false;	
}

bool AudioPlaybackChannel::Transport(
	const void* a_pData,
	DATA_TYPE a_eDataType,
	CAUDIO_U32_t a_nSize,
	TransportId_e a_eTPId)
{
	if (TP2RenderDev == a_eTPId)
	{
		if (NULL != m_RenderDevTransportPtr.get())
		{
			return m_RenderDevTransportPtr->Transport(a_pData, a_eDataType, a_nSize, TP2RenderDev);
		}
	}
	else if (TP2CaptureCh == a_eTPId)
	{
		if (NULL != m_pReceiverInCaptureCh)
		{
			return m_pReceiverInCaptureCh->Transport(a_pData, a_eDataType, a_nSize, m_nTributaryIdxToCaptureCh);
		}
	}

	AUDIO_PROCESSING_PRINTF("m_pReceiverInCaptureCh is NULL, cannot transport audio data!");
	return false;
}



// todo: this logic is almost the same with which is in
// capture channel class, there is redundant codes here
bool AudioPlaybackChannel::processData(
	const AudioFrame *a_AudioFrameArray[], 
	CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init operation failed");
		return false;
	}

	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nNetCaptureThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("unresonable input param!");
		return false;
	}

	CAUDIO_U8_t sharedChannelThreadNum = 0;

	// todo: currently, we need not to pull data from shared channel to process,
	// as the logic of playback audio to rendor device is not ready. 
	// pull data from shared channel
	bool hasDataPullOut = false;
	if(NULL != m_SharedChSrcSyncPtr.get())
	{
		if(!m_SharedChSrcSyncPtr->PullData(
			m_SharedChannelFrameBuff, 
			MAX_THREAD_NUM, 
			sharedChannelThreadNum,
			hasDataPullOut))
		{
			AUDIO_PROCESSING_PRINTF("pull data from shared channel failed!");
			return false;
		}

		assert(sharedChannelThreadNum == m_nSharedOutputThreadNum);

		if(!hasDataPullOut)
		{
			AUDIO_PROCESSING_PRINTF("not any data pulled out, the mulSrcSync module may has been stopped processing.");
			return true;
		}
	}

	if (MAX_THREAD_NUM < m_nNetCaptureThreadNum + sharedChannelThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("sanity check failed in AudioCaptureChannel::processData");
		return false;
	}

	const AudioFrame *toBeProcessedFrameArray[MAX_THREAD_NUM];
	CAUDIO_U8_t toBeProcessedIdx=0;
	for(; toBeProcessedIdx<m_nNetCaptureThreadNum; ++toBeProcessedIdx)
	{
		toBeProcessedFrameArray[toBeProcessedIdx] = a_AudioFrameArray[toBeProcessedIdx];
	}
	for(CAUDIO_U8_t sharedFrameIdx=0; sharedFrameIdx<sharedChannelThreadNum; ++toBeProcessedIdx, ++sharedFrameIdx)
	{
		toBeProcessedFrameArray[toBeProcessedIdx] = &m_SharedChannelFrameBuff[sharedFrameIdx];
	}

	// pass frame from capture device & shared channel to Processing Module
	if(NULL != m_AudioProcessingPtr.get())
	{
		// todo: add a parameter which indicates frame number in array in processData
		// and remove the parameter in the constructor of AudioProcessing
		if(!m_AudioProcessingPtr->processData(toBeProcessedFrameArray, MAX_THREAD_NUM))
		{
			AUDIO_PROCESSING_PRINTF("processed data failed in AudioProcessing!");
			return true;  // this frame is processed though with error, but still return true
		}
	}

	return true;
}

#if CAPTURE_PLAYBACK_INTEGRATED
/*virtual */bool AudioPlaybackChannel::processData(
	AudioFrame a_AudioFrameArray[],
	CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init operation failed");
		return false;
	}

	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nNetCaptureThreadNum + m_nSharedOutputThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("unresonable input param!");
		return false;
	}

	CAUDIO_U8_t sharedChannelThreadNum = 0;

	// todo: currently, we need not to pull data from shared channel to process,
	// as the logic of playback audio to rendor device is not ready. 
	// pull data from shared channel
	bool hasDataPullOut = false;
	if (NULL != m_SharedChSrcSyncPtr.get())
	{
		if (!m_SharedChSrcSyncPtr->PullData(
			m_SharedChannelFrameBuff,
			MAX_THREAD_NUM,
			sharedChannelThreadNum,
			hasDataPullOut))
		{
			AUDIO_PROCESSING_PRINTF("pull data from shared channel failed!");
			return false;
		}

		assert(sharedChannelThreadNum == m_nSharedOutputThreadNum);

		if (!hasDataPullOut)
		{
			AUDIO_PROCESSING_PRINTF("not any data pulled out, the mulSrcSync module may has been stopped processing.");
			return true;
		}
	}

	if (MAX_THREAD_NUM < m_nNetCaptureThreadNum + sharedChannelThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("sanity check failed in AudioCaptureChannel::processData");
		return false;
	}

	CAUDIO_U8_t toBeProcessedIdx = m_nNetCaptureThreadNum;
	for (CAUDIO_U8_t sharedFrameIdx = 0; sharedFrameIdx < sharedChannelThreadNum; ++toBeProcessedIdx, ++sharedFrameIdx)
	{
		a_AudioFrameArray[toBeProcessedIdx].CopyFrom((const AudioFrame&)m_SharedChannelFrameBuff[sharedFrameIdx]);
	}

	// pass frame from capture device & shared channel to Processing Module
	if (NULL != m_AudioProcessingPtr.get())
	{
		// todo: add a parameter which indicates frame number in array in processData
		// and remove the parameter in the constructor of AudioProcessing
		if (!m_AudioProcessingPtr->processData(a_AudioFrameArray, a_nArrayLen))
		{
			AUDIO_PROCESSING_PRINTF("processed data failed in AudioProcessing!");
			return true;  // this frame is processed though with error, but still return true
		}
	}

	return true;
}

bool AudioPlaybackChannel::NeedData(
	AudioFrame*  pAudioFrameNeeded,
	const CAUDIO_U32_t nMaxFrameSize,
	const CAUDIO_U32_t nSamplesPerSec,
	const CAUDIO_U8_t nChannel)
{
	bool retval = false;
	
	if (NULL == pAudioFrameNeeded)
	{
		AUDIO_PROCESSING_PRINTF("NULL == m_pPlaybackChannel.");
		return false;
	}
	if (nMaxFrameSize < m_nNetCaptureThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("nMaxFrameSize < m_nNetCaptureThreadNum.");
		return false;
	}
	if (!SAMPLE_RATE_VALID_CHECK(nSamplesPerSec))
	{
		AUDIO_PROCESSING_PRINTF("nSamplesPerSec doesn't match.");
		return false;
	}
	if (NULL == m_DeviceSrcSyncPtr.get())
	{
		AUDIO_PROCESSING_PRINTF("NULL == m_DeviceSrcSyncPtr.get().");
		return false;
	}
	retval = m_DeviceSrcSyncPtr->PullData_CapturePlaybackIntegrated(
		pAudioFrameNeeded,
		nMaxFrameSize,
		nSamplesPerSec,
		nChannel);
	if (false == retval)
	{
		AUDIO_PROCESSING_PRINTF("PullData() failed.");
		return false;
	}

	return true;
}
#endif

bool AudioPlaybackChannel::RegisterAudioTransport(
	IAudioTransport* a_pAudioTransport, 
	TransportId_e a_eTPId, 
	CAUDIO_U8_t a_nAssignedIdx)
{
	if(NULL == a_pAudioTransport)
	{
		AUDIO_PROCESSING_PRINTF("a_pAudioTransport is NULL!");
		return false;
	}
	else if(!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init failed, a_pAudioTransport cannot be registered");
		return false;
	}
	
	if(TP2RenderDev == a_eTPId)
	{
		return m_RenderDevTransportPtr->RegisterAudioSink(a_pAudioTransport);
	}
	else if(TP2CaptureCh == a_eTPId)
	{
		m_pReceiverInCaptureCh = a_pAudioTransport;
		m_nTributaryIdxToCaptureCh = a_nAssignedIdx;
		return true;
	}

	AUDIO_PROCESSING_PRINTF("invalid transport id");
	return false;
}

bool AudioPlaybackChannel::DeregisterAudioTransport(TransportId_e a_eTPId)
{
	if(TP2RenderDev == a_eTPId)
	{
		return m_RenderDevTransportPtr->DeregisterAudioSink();
	}
	else if(TP2CaptureCh == a_eTPId)
	{
		m_pReceiverInCaptureCh = NULL;
		return true;
	}

	AUDIO_PROCESSING_PRINTF("invalid transport id");
	return false;
}

