/*	
 *	Name:			AudioCaptureChannel.cpp
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/06/2015 Created			Zhong Yaozhu
 *
 *
 */

#include "AudioCaptureChannel.h"
#include "audiotrace.h"

AudioCaptureChannel::AudioCaptureChannel(
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
	std::string a_sFolderName)
	:
	m_nCaptureThreadNum(a_nCaptureThreadNum),
	m_nChannelNumPerCaptureThread(a_nChannelNumPerCaptureThread),
	m_nChannelNumPerSharedThread(a_nChannelNumPerSharedThread),
	m_eInputDataType(a_eInputDataType),
	m_nSampleRate_InternalProcessUsed(a_nSampleRate_InternalProcessUsed),
	m_nSampleRate_OnlineClassOut(a_nSampleRate_OnlineClassOut),
	m_nSampleRate_RecorderOut(a_nSampleRate_RecorderOut),
	m_nFrameTimeMsPerChannel(a_nFrameTimeMsPerChannel),
	m_nFrameSizePerChannel(FRAM_LEN(a_nFrameTimeMsPerChannel, a_nSampleRate_InternalProcessUsed)),
	m_pPropertyPage(a_pPropertyPage),
	m_bIsPlayChEnable(a_bIsPlayChEnable),
	m_bIsSharedChEnable(a_bIsSharedChEnable),
	m_bIsRecordingEnable(a_bIsRecordingEnable),
	m_sFolderName(a_sFolderName)
	{}


// this API must be called after constructor is called
bool AudioCaptureChannel::Init()
{
	
	// alloc audio processing implementation
	m_AudioProcessingPtr.reset(new CaputureAudioProcessing(
		this,
		m_nCaptureThreadNum,
		m_nChannelNumPerCaptureThread,
		m_nFrameTimeMsPerChannel,
		m_nSampleRate_InternalProcessUsed,
		m_eInputDataType,
		CaptureCh,
		m_pPropertyPage,
		m_bIsSharedChEnable,
		m_bIsPlayChEnable,
		m_bIsRecordingEnable,
		m_sFolderName));
	if(NULL == m_AudioProcessingPtr.get())
	{
		return false;
	}
	if (false == m_AudioProcessingPtr->__Init())
	{
		return false;
	}

	// alloc multiple source sync
	// push mode
	m_DeviceSrcSyncPtr.reset(new MultipleSourceSync(
		this,
		m_nCaptureThreadNum,
		m_nChannelNumPerCaptureThread,
		m_nFrameTimeMsPerChannel,
		m_nSampleRate_InternalProcessUsed,
		MODE_PUSH_ASYNC_CALL,
		REAL_FLOAT_DATA,
		CaptureCh));
	if(NULL == m_DeviceSrcSyncPtr.get())
	{
		return false;
	}
	if(!m_DeviceSrcSyncPtr->Init())
	{
		AUDIO_PROCESSING_PRINTF("init Device Src Sync failed");
		return false;
	}
	m_DeviceSrcSyncPtr->SetDSId(DSFromCaptureDev);

	// pull mode
	CAUDIO_U8_t tributaryThreadNum = m_bIsSharedChEnable?1:0;
	tributaryThreadNum += m_bIsPlayChEnable?1:0;
	if(0 != tributaryThreadNum)
	{
		// alloc tributary source sync
		m_TributarySrcSyncPtr.reset(new MultipleSourceSync(
			NULL,
			tributaryThreadNum,
			m_nChannelNumPerSharedThread,
			m_nFrameTimeMsPerChannel,
			m_nSampleRate_InternalProcessUsed,
			MODE_PUll_ASYNC_CALL,
			REAL_FLOAT_DATA,
			CaptureCh));
		if(NULL == m_TributarySrcSyncPtr.get())
		{
			return false;
		}
		if(!m_TributarySrcSyncPtr->Init())
		{
			AUDIO_PROCESSING_PRINTF("init Shared Channel Src Sync failed");
			return false;
		}
		// DSId will be set through tributary receiver when tributary source sync
		// is registered to shared channel & playback channel
		m_TributarySrcSyncPtr->SetDSId(InvalidDS); 

		// alloc tributary receiver
		m_TributaryDataReceiverPtr.reset(new AudioTributaryReceiver(
			REAL_FLOAT_DATA,
			m_nSampleRate_InternalProcessUsed,
			m_nChannelNumPerSharedThread));
		if(NULL == m_TributaryDataReceiverPtr.get())
		{
			return false;
		}
		m_TributaryDataReceiverPtr->RegisterMulSrcSync(m_TributarySrcSyncPtr.get());
	}

	// alloc network transport
	m_NetworkTransportPtr.reset(new AudioTransportImpl(m_nSampleRate_InternalProcessUsed, m_nSampleRate_OnlineClassOut));
	if(NULL == m_NetworkTransportPtr.get())
	{
		return false;
	}

	if(true == m_bIsRecordingEnable)
	{
		// alloc recording transport
		m_RecordingTransportPtr.reset(new AudioTransportImpl(m_nSampleRate_InternalProcessUsed, m_nSampleRate_RecorderOut));
		if(NULL == m_RecordingTransportPtr.get())
		{
			return false;
		}
	}

	m_bIsInitSuccess = true;
	return true;
}

bool AudioCaptureChannel::Reset()
{
	if (m_AudioProcessingPtr.get())
	{
		m_AudioProcessingPtr->Reset();
	}

	return true;
}


ModuleState AudioCaptureChannel::Start()
{
	if(!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("AudioCaptureChannel init failed");
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

	if(NULL != m_TributarySrcSyncPtr.get())
	{
		ret = m_TributarySrcSyncPtr->StartProcess();
		if(Audio_Module_State_Running != ret)
		{
			AUDIO_PROCESSING_PRINTF("start shared channel source sync failed as multiple src sync module is not running");
			return ret;
		}
	}

	m_bStarted = true;
	return Audio_Module_State_Running;
}

ModuleState AudioCaptureChannel::Stop()
{
	if(!m_bStarted)
	{
		AUDIO_PROCESSING_PRINTF("this module alredy stopped");
		return Audio_Module_State_Stopped;
	}

	if (NULL != m_AudioProcessingPtr.get())
	{
		m_AudioProcessingPtr->StopProcess();
	}

	if(NULL != m_TributarySrcSyncPtr.get())
	{
		m_TributarySrcSyncPtr->StopProcess();
	}

	if(NULL != m_DeviceSrcSyncPtr.get())
	{
		m_DeviceSrcSyncPtr->StopProcess();
	}

	m_bStarted = false;
	return Audio_Module_State_Stopped;
}

AudioTributaryReceiver* AudioCaptureChannel::GetAudioTributaryReceiver()
{
	return m_TributaryDataReceiverPtr.get(); 
}

//bool AudioCaptureChannel::FeedData(
//	const void* a_pData,
//	CAUDIO_U32_t a_nDataSize,
//	CAUDIO_U8_t a_nThreadIdx)
//{
//	if (m_bIsInitSuccess)
//	{
//		if (NULL != m_DeviceSrcSyncPtr.get())
//		{
//			return m_DeviceSrcSyncPtr->FeedData(a_pData, m_eInputDataType, a_nDataSize,
//				m_nDefaultSampleRate, m_nChannelNumPerCaptureThread, a_nThreadIdx);
//		}
//	}
//
//	return false;
//}

bool AudioCaptureChannel::FeedData(	
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
				m_nChannelNumPerCaptureThread, a_nThreadIdx, a_nOriginalSampleRate);
		}
	}

	return false;	
}

bool AudioCaptureChannel::Transport(
	const void* a_pData,
	DATA_TYPE a_eDataType,
	CAUDIO_U32_t a_nSize,
	TransportId_e a_eTPId)
{
	if(TP2Network==a_eTPId && NULL!=m_NetworkTransportPtr.get())
	{
		return m_NetworkTransportPtr->Transport(a_pData, a_eDataType, a_nSize, a_eTPId);
	}
	else if(TP2RecordingDev==a_eTPId && NULL!=m_RecordingTransportPtr.get())
	{
		return m_RecordingTransportPtr->Transport(a_pData, a_eDataType, a_nSize, a_eTPId);
	}


	AUDIO_PROCESSING_PRINTF("transport is NULL or invalid TPID, cannot transport audio data!");
	return false;
}

bool AudioCaptureChannel::processData(
	const AudioFrame *a_AudioFrameArray[], 
	CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init operation failed");
		return false;
	}

	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nCaptureThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("unresonable input param!");
		return false;
	}

	CAUDIO_U8_t sharedChannelThreadNum = 0;
	// pull data from shared channel
	bool hasDataPullOut = false;
	if(NULL != m_TributarySrcSyncPtr.get())
	{
		if(!m_TributarySrcSyncPtr->PullData(
			m_SharedChannelFrameBuff, 
			MAX_THREAD_NUM, 
			sharedChannelThreadNum,
			hasDataPullOut))
		{
			AUDIO_PROCESSING_PRINTF("pull data from shared channel failed!");
			return false;
		}

		if(!hasDataPullOut)
		{
			AUDIO_PROCESSING_PRINTF("not any data pulled out, the mulSrcSync module may has been stopped processing.");
			return true;
		}
	}

	if (MAX_THREAD_NUM < m_nCaptureThreadNum + sharedChannelThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("sanity check failed in AudioCaptureChannel::processData");
		return false;
	}

	const AudioFrame *toBeProcessedFrameArray[MAX_THREAD_NUM];
	CAUDIO_U8_t toBeProcessedIdx=0;
	for(; toBeProcessedIdx<m_nCaptureThreadNum; ++toBeProcessedIdx)
	{
		toBeProcessedFrameArray[toBeProcessedIdx] = a_AudioFrameArray[toBeProcessedIdx];
	}
	for(CAUDIO_U8_t sharedFrameIdx=0; sharedFrameIdx<sharedChannelThreadNum; ++toBeProcessedIdx, ++sharedFrameIdx)
	{
		toBeProcessedFrameArray[toBeProcessedIdx] = &m_SharedChannelFrameBuff[sharedFrameIdx];
	}

	// pass frames from capture device & shared channel & playback channel to Processing Module
	if(NULL != m_AudioProcessingPtr.get())
	{
		// todo: add a parameter which indicate frames number in processData interface
		// and remove the parameter in the constructor of AudioProcessing
		if(!m_AudioProcessingPtr->processData(toBeProcessedFrameArray, MAX_THREAD_NUM))
		{
			AUDIO_PROCESSING_PRINTF("processed data failed in AudioProcessing!");
			return true;  // this frame is processed though with error, but still return true
		}
	}

	return true;
}

bool AudioCaptureChannel::RegisterAudioTransport(IAudioTransport* a_pAudioTransport, TransportId_e a_eTPId)
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
	
	if(TP2Network == a_eTPId)
	{
		return m_NetworkTransportPtr->RegisterAudioSink(a_pAudioTransport);
	}
	else if(TP2RecordingDev == a_eTPId && NULL != m_RecordingTransportPtr.get())
	{
		return m_RecordingTransportPtr->RegisterAudioSink(a_pAudioTransport);
	}

	AUDIO_PROCESSING_PRINTF("invalid transport id");
	return false;
}

bool AudioCaptureChannel::DeregisterAudioTransport(TransportId_e a_eTPId)
{
	if(TP2Network != a_eTPId)
	{
		AUDIO_PROCESSING_PRINTF("only TP2Network transport is allowed to be unregistered");
		return false;
	}

	if(TP2Network == a_eTPId)
	{
		return m_NetworkTransportPtr->DeregisterAudioSink();
	}
	else if(TP2RecordingDev == a_eTPId && NULL != m_RecordingTransportPtr.get())
	{
		return m_RecordingTransportPtr->DeregisterAudioSink();
	}

	AUDIO_PROCESSING_PRINTF("invalid transport id");
	return false;
}
