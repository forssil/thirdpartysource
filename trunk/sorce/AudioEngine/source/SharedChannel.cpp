/*! \file     SharedChannel.h
*   \author   Keil
*   \brief    used to mix audio data from class-to-class and electronic piano , then transmission mixed data to capture channel.
*   \history  2015/3/9 created shared channel class.
*/

#include "SharedChannel.h"
#include "AudioEngineImpl.h"
#include "audiotrace.h"

#define INVALID_ASSIGNED_INDEX 0xFF

CSharedChannel::CSharedChannel(
	//AudioEngineImpl *_audio_engine_impl_,	
	CAUDIO_U8_t _thread_num_,
	CAUDIO_U8_t _channel_num_per_thread_,
	AUDIO_DATA_TYPE _frame_time_ms_,
	CAUDIO_U32_t _fs_,
	DATA_TYPE _input_data_type_)
	:
	//, m_cAudioEngineImpl(_audio_engine_impl_)
	  m_nNumOfThread(_thread_num_)
	, m_nChannelNumPerThread(_channel_num_per_thread_)
	, m_fFrameTime(_frame_time_ms_)
	, m_nDefaultSampleRate(_fs_)
	, m_eInputDataType(_input_data_type_)
	, m_pTransportToCaptureCh(NULL)
	, m_pTransportToPlaybackCh(NULL)
	, m_nAssignedIdxToCaptureCh(INVALID_ASSIGNED_INDEX)
	, m_nAssignedIdxToPlaybackCh(INVALID_ASSIGNED_INDEX)
{
}

CSharedChannel::~CSharedChannel()
{

}

/*virtual */bool CSharedChannel::Init()
{
	m_cAudioProcessingImplPtr.reset(new CAudioProcessingImpl(
		//m_cAudioTransportImplPtr.get(),
		this,
		m_nNumOfThread,
		m_nChannelNumPerThread,
		m_fFrameTime,
		m_nDefaultSampleRate,
		m_eInputDataType,
		SharedCh));
	if(NULL == m_cAudioProcessingImplPtr.get())
	{
		return false;
	}
	if (false == m_cAudioProcessingImplPtr->__Init())
	{
		return false;
	}

	// alloc multiple source sync
	m_cMultipleSourceSyncPtr.reset(new MultipleSourceSync(
		//m_cAudioProcessingImplPtr.get(),
		this,
		m_nNumOfThread,
		m_nChannelNumPerThread,
		m_fFrameTime,
		m_nDefaultSampleRate,
		MODE_PUSH_ASYNC_CALL,
		REAL_FLOAT_DATA,
		SharedCh));
	if(NULL == m_cMultipleSourceSyncPtr.get())
	{
		return false;
	}
	if (!m_cMultipleSourceSyncPtr->Init())
	{
		AUDIO_PROCESSING_PRINTF("init Multiple source sync failed");
		return false;
	}
	m_cMultipleSourceSyncPtr->SetDSId(DSFromCaptureDev);

	m_bIsInitSuccess = true;
	return true;
}

/*virtual */bool CSharedChannel::Reset()
{
	if (m_cAudioProcessingImplPtr.get())
	{
		m_cAudioProcessingImplPtr->Reset();
	}
	return true;
}

/*virtual */ModuleState CSharedChannel::Start()
{
	if (!m_bStarted)
	{
		ModuleState ret = m_cMultipleSourceSyncPtr->StartProcess();
		if (Audio_Module_State_Running != ret)
		{
			AUDIO_PROCESSING_PRINTF("start CSharedChannel failed as multiple src sync module is not running");
			return ret;
		}

		m_bStarted = true;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("Start operation failed");
		return Audio_Module_State_Unknown;
	}

	return Audio_Module_State_Running;
}

/*virtual */ModuleState CSharedChannel::Stop()
{
	if (m_bStarted)
	{
		ModuleState ret = m_cMultipleSourceSyncPtr->StopProcess();

		if (Audio_Module_State_Stopped != ret)
		{
			AUDIO_PROCESSING_PRINTF("stop Audio Engine failed as multiple src sync module is not stopping");
			return ret;
		}

		m_bStarted = false;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("Stop operation failed");
		return Audio_Module_State_Dead;
	}

	return Audio_Module_State_Stopped;
}

/*virtual */bool CSharedChannel::FeedData(const void* a_pData, CAUDIO_U32_t a_nDataSize, CAUDIO_U8_t a_nThreadIdx, CAUDIO_U32_t a_nOriginalSampleRate)
{
	bool ret = false;

	if (m_bStarted && m_bIsInitSuccess)
	{
		if (NULL != m_cMultipleSourceSyncPtr.get())
		{
			ret = m_cMultipleSourceSyncPtr->FeedData(a_pData, m_eInputDataType, a_nDataSize,
				m_nChannelNumPerThread, a_nThreadIdx, a_nOriginalSampleRate);
		}
	}
	return ret;
}

bool CSharedChannel::Transport(const void* a_pData, DATA_TYPE a_eDataType, CAUDIO_U32_t a_nSize, TransportId_e a_eTPId)
{
	if(NULL!=m_pTransportToCaptureCh)
	{
		if(!m_pTransportToCaptureCh->Transport(a_pData, a_eDataType, a_nSize, m_nAssignedIdxToCaptureCh))
		{
			AUDIO_PROCESSING_PRINTF("transport data from shared channel to capture channel failed!")
		}
	}


	if(NULL!=m_pTransportToPlaybackCh)
	{
		if (!m_pTransportToPlaybackCh->Transport(a_pData, a_eDataType, a_nSize, m_nAssignedIdxToPlaybackCh))
		{
			AUDIO_PROCESSING_PRINTF("transport data from shared channel to playback channel failed!")
			return false;
		}
	}


	return true;
}

bool CSharedChannel::processData(const AudioFrame **a_AudioFrameArray, CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init operation failed");
		return false;
	}

	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nNumOfThread)
	{
		AUDIO_PROCESSING_PRINTF("unresonable input param!");
		return false;
	}

	// pass frame from capture device to Processing Module
	if(NULL != m_cAudioProcessingImplPtr.get())
	{
		if(!m_cAudioProcessingImplPtr->processData(a_AudioFrameArray, a_nArrayLen))
		{
			AUDIO_PROCESSING_PRINTF("processed data failed in AudioProcessing!");
			return true;  // this frame is processed though with error, but still return true
		}
	}

	return true;
}

bool CSharedChannel::RegisterAudioTransport(
	IAudioTransport* a_pAudioTransport,
	TransportId_e a_eTPId,
	CAUDIO_U8_t a_nAssignedThreadIdx)
{
	if (NULL == a_pAudioTransport)
	{
		return false;
	}

	if(TP2CaptureCh == a_eTPId)
	{
		m_pTransportToCaptureCh = a_pAudioTransport;
		m_nAssignedIdxToCaptureCh = a_nAssignedThreadIdx;
		return true;
	}
	else if(TP2RenderCh ==a_eTPId)
	{
		m_pTransportToPlaybackCh = a_pAudioTransport;
		m_nAssignedIdxToPlaybackCh = a_nAssignedThreadIdx;
		return true;
	}

	AUDIO_PROCESSING_PRINTF("Invalid transport registered");
	return false;
}

bool CSharedChannel::DeregisterAudioTransport(TransportId_e a_eTPId)
{
	if (a_eTPId == TP2CaptureCh)
	{
		m_pTransportToCaptureCh = NULL;
		return true;
	}
	else if(a_eTPId == TP2RenderCh)
	{
		m_pTransportToPlaybackCh = NULL;
		return true;
	}

	return false;
}