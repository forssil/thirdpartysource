/*	
 *	Name:			AudioEngineImpl.cpp
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
 *
 *
 */


#include <streams.h>             // include CTransInPlaceFilter 
#include <stdio.h>
#include "AudioEngineImpl.h"
#include "AudioProcessingImpl.h"
#include "DummyPlayer.h"
#include "audiotrace.h"

//1s 48kHz max out buffer 
#define MAX_OUT_BUFFER_SIZE (48000 * MAX_INPUT_THREAD_NUM)

IAudioEngine* IAudioEngine:: Create(
	AUDIO_TYPE_NUM_t a_sThreadNumInfo,
	CAUDIO_U8_t a_nInChannelNumPerThread,
	AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel,
	SSampleRate a_sSampleRate,
	DATA_TYPE a_eInputDataType)
{
	AudioEngineImpl* p = new AudioEngineImpl(a_sThreadNumInfo, a_nInChannelNumPerThread, a_nFrameTimeMsPerChannel, a_sSampleRate, a_eInputDataType);
	if(NULL != p)
		return static_cast<IAudioEngine*>(p);
	else
		return NULL;
}

bool IAudioEngine::Delete(IAudioEngine*& voiceEngine)
{
	if (voiceEngine == NULL)
		return false;

	AudioEngineImpl* s = static_cast<AudioEngineImpl*>(voiceEngine);
	delete s;
	voiceEngine = NULL;
	
	return true;

}

AudioEngineImpl::AudioEngineImpl(
	AUDIO_TYPE_NUM_t a_sThreadNumInfo,
	CAUDIO_U8_t a_nInChannelNumPerThread,
	AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel,
	SSampleRate a_sSampleRate,
	DATA_TYPE a_eInputDataType)
	: 
	m_nCaptureChannelThreadNum(a_sThreadNumInfo.nMic_in_),
	m_nSharedChannelThreadNum(a_sThreadNumInfo.nClassToClass_in_+a_sThreadNumInfo.nElectronicPiano_in_),
	m_nNetCaptureThreadNum(a_sThreadNumInfo.nOnLineClass_in_),
	m_bIsRecordingEnable(0!=a_sThreadNumInfo.nRecording_out_),
	m_nChannelNumPerThread(a_nInChannelNumPerThread),
	m_nFrameSizePerChannel(FRAM_LEN(a_nFrameTimeMsPerChannel, a_sSampleRate.nSampleRate_InterProcessUsed)),
	m_eInputDataType(a_eInputDataType),
	m_nSampleRate_InterProcessUsed(a_sSampleRate.nSampleRate_InterProcessUsed),
	m_nSampleRate_OnlineClassOut(a_sSampleRate.nSampleRate_OnlineClassOut),
	m_nSampleRate_SpeakerOut(a_sSampleRate.nSampleRate_SpeakerOut),
	m_nSampleRate_RecorderOut(a_sSampleRate.nSampleRate_RecorderOut),
	m_nFrameTimeMsPerChannel(a_nFrameTimeMsPerChannel),
	m_bIsInitSuccess(false),
	m_bStarted(false),
	m_pCaptureChannel(NULL),
	m_pSharedChannel(NULL),
	m_pPlaybackChannel(NULL),
	m_pPropertyPage(NULL),

#if CAPTURE_PLAYBACK_INTEGRATED
	m_pAudioDeviceManager(NULL)
#endif
{
	memcpy_s(&m_sThreadNumInfo, sizeof(AUDIO_TYPE_NUM), &a_sThreadNumInfo, sizeof(AUDIO_TYPE_NUM));
}

// this API must be called after constructor is called
bool AudioEngineImpl::Init(bool aec_switch, std::string folder_name)
{
	// alloc member lock
	m_MemberLock.reset(CriticalSectionWrapper::CreateCriticalSection());

	// Sanity check
	if(m_nCaptureChannelThreadNum>MAX_INPUT_THREAD_NUM || m_nSharedChannelThreadNum>MAX_INPUT_THREAD_NUM 
		|| m_nNetCaptureThreadNum>MAX_INPUT_THREAD_NUM)
	{
		AUDIO_PROCESSING_PRINTF("too many thread number, up to %d thread is allow", MAX_INPUT_THREAD_NUM);
		return false;
	}

	if (22050 != m_nSampleRate_InterProcessUsed 
		&& 44100 != m_nSampleRate_InterProcessUsed
		&& 48000 != m_nSampleRate_InterProcessUsed)
	{
		AUDIO_PROCESSING_PRINTF("m_nFs:%d is not supported!", m_nSampleRate_InterProcessUsed);
		return false;
	}

	if(((m_nFrameSizePerChannel !=256) ||(m_nFrameSizePerChannel !=512) ))
	{
		if (22050 == m_nSampleRate_InterProcessUsed)
		{
			m_nFrameTimeMsPerChannel = 11.61f;
		}
		else if (44100 == m_nSampleRate_InterProcessUsed)
		{
			m_nFrameTimeMsPerChannel = 11.61f;
		}
		else if (48000 == m_nSampleRate_InterProcessUsed)
		{
			m_nFrameTimeMsPerChannel = 10.67f;
		}
		m_nFrameSizePerChannel = FRAM_LEN(m_nFrameTimeMsPerChannel, m_nSampleRate_InterProcessUsed);
	}

#if CAPTURE_PLAYBACK_INTEGRATED
#else
	if(0==m_nCaptureChannelThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("capture channel thread must not be zero!");
		return false;
	}
#endif

	// alloc AudioPropertyPage
	CAUDIO_U32_t propertypage_thread_num = m_nSharedChannelThreadNum ? 2 : 1;
	m_pPropertyPage = new CAudioPropertyPage(propertypage_thread_num, aec_switch);

#if CAPTURE_PLAYBACK_INTEGRATED
	// alloc AudioDeviceTransport
	if (m_pAudioDeviceManager)
	{
	    IAudioDeviceManager::DeleteAudioDeviceManager(m_pAudioDeviceManager);
		m_pAudioDeviceManager = NULL;
	}
	m_pAudioDeviceManager = IAudioDeviceManager::CreateAudioDeviceManager(this, m_eInputDataType, folder_name);
	if (NULL == m_pAudioDeviceManager)
	{
		AUDIO_PROCESSING_PRINTF("new IAudioDeviceManager failed!");
		return false;
	}
	if (false == m_pAudioDeviceManager->Init())
	{
		AUDIO_PROCESSING_PRINTF("IAudioDeviceManager initial failed!");
		return false;
	}
	
#endif

	// alloc AudioCaptureChannel
	// for now, shared channel out put thread is 1 && playback channel output is 1
#if CAPTURE_PLAYBACK_INTEGRATED
	
	m_pCaptureChannel = new AudioCaptureChannel(
		1,			
		1,	 
		1,									
		m_eInputDataType,
		m_nSampleRate_InterProcessUsed,
		m_nSampleRate_OnlineClassOut,
		m_nSampleRate_RecorderOut,
		m_nFrameTimeMsPerChannel,
		m_pPropertyPage->m_sPropertyPage,
		0!=m_nNetCaptureThreadNum,
		0!=m_nSharedChannelThreadNum,
		m_bIsRecordingEnable,
		folder_name);
#else
	m_pCaptureChannel = new AudioCaptureChannel(
		m_nCaptureChannelThreadNum,			
		1,	 
		1,									
		m_eInputDataType,
		m_nSampleRate_InterProcessUsed,
		m_nSampleRate_OnlineClassOut,
		m_nSampleRate_RecorderOut,
		m_nFrameTimeMsPerChannel,
		m_pPropertyPage->m_sPropertyPage,
		0!=m_nNetCaptureThreadNum,
		0!=m_nSharedChannelThreadNum,
		m_bIsRecordingEnable,
		folder_name);
#endif

	if(NULL == m_pCaptureChannel)
	{
		return false;
	}
	if (!m_pCaptureChannel->Init())
	{
		return false;
	}

	// alloc AudioPlaybackChannel
	if(0 != m_nNetCaptureThreadNum)
	{
		m_pPlaybackChannel = new AudioPlaybackChannel(
			m_nNetCaptureThreadNum,
			(0 == m_nSharedChannelThreadNum) ? 0 : 1,	
			1,     
			1,		
			m_eInputDataType,
			m_nSampleRate_InterProcessUsed,
			m_nSampleRate_SpeakerOut,
			m_nFrameTimeMsPerChannel);

		if(NULL == m_pPlaybackChannel)
		{
			return false;
		}

		if(!m_pPlaybackChannel->Init())
		{
			return false;
		}

		//get transport from capture channel and register to playback channel
		if(!RegisterTrinutaryReceiverToPlaybackChannel())
		{
			return false;
		}
	}

	// alloc AudioSharedChannel
	if(0 != m_nSharedChannelThreadNum)
	{
		m_pSharedChannel = new CSharedChannel(		
			m_nSharedChannelThreadNum,
			1,//m_nChannelNumPerThread,
			m_nFrameTimeMsPerChannel,
			m_nSampleRate_InterProcessUsed,
			m_eInputDataType);
		if(NULL == m_pSharedChannel)
		{
			return false;
		}
		if(!m_pSharedChannel->Init())
		{
			return false;
		}

		//get transport from capture&playback channel and register to shared channel
		if(!RegisterTrinutaryReceiverToSharedChannel())
		{
			return false;
		}
	}

	m_bIsInitSuccess = true;
	return true;
}

bool AudioEngineImpl::Reset()
{
	if(NULL!=m_pCaptureChannel)
	{
		if(!m_pCaptureChannel->Reset())
		{
			return false;
		}
	}

	if(NULL!=m_pPlaybackChannel)
	{
		if(!m_pPlaybackChannel->Reset())
		{
			return false;
		}
	}

	return true;
}

AudioEngineImpl::~AudioEngineImpl()
{
	Stop();

	m_MemberLock->Enter();
	m_bIsInitSuccess = false;
	if(NULL != m_pCaptureChannel)
	{
		delete m_pCaptureChannel;
		m_pCaptureChannel = NULL;
	}

	if(NULL != m_pPlaybackChannel)
	{
		delete m_pPlaybackChannel;
		m_pPlaybackChannel = NULL;
	}

	if(NULL != m_pSharedChannel)
	{
		delete m_pSharedChannel;
		m_pSharedChannel = NULL;
	}

	if (NULL != m_pPropertyPage)
	{
		delete m_pPropertyPage;
		m_pPropertyPage = NULL;
	}
	if (m_pAudioDeviceManager)
	{
		m_pAudioDeviceManager->Release();
		IAudioDeviceManager::DeleteAudioDeviceManager(m_pAudioDeviceManager);
		m_pAudioDeviceManager = NULL;
	}

	m_MemberLock->Leave();
}

bool AudioEngineImpl::RegisterAudioTransport(IAudioTransport* a_pAudioTransport, TransportId_e a_eTPId)
{
	m_MemberLock->Enter();
	if(NULL == a_pAudioTransport)
	{
		AUDIO_PROCESSING_PRINTF("a_pAudioTransport is NULL!");
		m_MemberLock->Leave();
		return false;
	}
	else if(!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init failed, a_pAudioTransport cannot be registered");
		m_MemberLock->Leave();
		return false;
	}
	else if(m_bStarted)
	{
		AUDIO_PROCESSING_PRINTF("Audio Engine has been started, should not register transport again");
		m_MemberLock->Leave();
		return false;
	}
	
	if(TP2Network==a_eTPId && 0!=m_sThreadNumInfo.nOnLineClass_out_)
	{
		if(!m_pCaptureChannel->RegisterAudioTransport(a_pAudioTransport, a_eTPId))
		{
			m_MemberLock->Leave();
			return false;
		}
	}
	else if(TP2RecordingDev==a_eTPId && 0!=m_sThreadNumInfo.nRecording_out_)
	{
		if(!m_pCaptureChannel->RegisterAudioTransport(a_pAudioTransport, a_eTPId))
		{
			m_MemberLock->Leave();
			return false;
		}
	}
	else if(TP2RenderDev==a_eTPId && NULL!=m_pPlaybackChannel && 0!=m_sThreadNumInfo.nSpeaker_out_)
	{
		if(!m_pPlaybackChannel->RegisterAudioTransport(a_pAudioTransport, a_eTPId))
		{
			m_MemberLock->Leave();
			return false;
		}
	}

	m_MemberLock->Leave();
	return true;
}


bool AudioEngineImpl::DeregisterAudioTransport(TransportId_e a_eTPId)
{
	m_MemberLock->Enter();
	
	if(TP2Network==a_eTPId || TP2RecordingDev==a_eTPId)
	{
		if(!m_pCaptureChannel->DeregisterAudioTransport(TP2Network))
		{
			m_MemberLock->Leave();
			return false;
		}
	}
	else if(TP2RenderDev==a_eTPId && NULL!=m_pPlaybackChannel)
	{
		if(!m_pPlaybackChannel->DeregisterAudioTransport(TP2RenderDev))
		{
			m_MemberLock->Leave();
			return false;
		}
	}
	else
	{
		m_MemberLock->Leave();
		return false;
	}

	m_MemberLock->Leave();
	return true;
}

bool AudioEngineImpl::RegisterPropertyPage(IAudioPropertyPage* a_pAudioTransport)
{
	if (NULL != a_pAudioTransport)
	{
		return m_pPropertyPage->RegisterProperty(a_pAudioTransport);
	}
	else
	{
		return false;
	}
}

bool AudioEngineImpl::UnRegisterPropertyPage(IAudioPropertyPage* a_pAudioTransport)
{
	
	if (NULL != a_pAudioTransport)
	{
		return m_pPropertyPage->UnregisterProperty(a_pAudioTransport);
	}
	else
	{
		return false;
	}
}

bool AudioEngineImpl::FeedData(
	ChannelId_e a_eChannelId,
	const void* a_pData,
	CAUDIO_U32_t a_nDataSize,
	CAUDIO_U8_t a_nThreadIdx,
	CAUDIO_U32_t a_nOriginalSampleRate)
{
	if(0 != a_nDataSize%m_nChannelNumPerThread)
	{
		AUDIO_PROCESSING_PRINTF("number of datasize must be multiple times of channel number");
		return false;
	}
	
	if(m_bStarted && m_bIsInitSuccess)
	{
		if(CaptureCh==a_eChannelId && NULL!=m_pCaptureChannel)
		{
			
			return m_pCaptureChannel->FeedData(
				a_pData, 
				a_nDataSize,
				a_nThreadIdx,
				a_nOriginalSampleRate);
		}
		else if(SharedCh==a_eChannelId && NULL!=m_pSharedChannel)
		{
			return m_pSharedChannel->FeedData(
				a_pData, 
				a_nDataSize,
				a_nThreadIdx,
				a_nOriginalSampleRate);
		}
		else if(RenderCh==a_eChannelId && NULL!=m_pPlaybackChannel)
		{
			return m_pPlaybackChannel->FeedData(
				a_pData, 
				a_nDataSize,
				a_nThreadIdx,
				a_nOriginalSampleRate);
		}
	}
	return false;
}

#if CAPTURE_PLAYBACK_INTEGRATED
bool AudioEngineImpl::NeedData(
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
		AUDIO_PROCESSING_PRINTF("nSamplesPerSec doesn't match");
		return -1;
	}
	if (NULL == m_pPlaybackChannel)
	{
		AUDIO_PROCESSING_PRINTF("NULL == m_pPlaybackChannel.");
		return false;
	}
	retval = m_pPlaybackChannel->NeedData(pAudioFrameNeeded, nMaxFrameSize, nSamplesPerSec, nChannel);
	if (false == retval)
	{
		AUDIO_PROCESSING_PRINTF("NeedData called fails.");
		return false;
	}
	return true;
}
#endif

ModuleState AudioEngineImpl::Start()
{
	m_MemberLock->Enter();

	if(!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("Audio Engine init failed");
		return Audio_Module_State_Dead;
	}
	if(!m_bStarted)
	{
		Reset();
		if(NULL!=m_pSharedChannel && Audio_Module_State_Running!=m_pSharedChannel->Start())
		{
			AUDIO_PROCESSING_PRINTF("start shared channel failed!");
			return Audio_Module_State_Inited;
		}
		if(NULL!=m_pPlaybackChannel && Audio_Module_State_Running != m_pPlaybackChannel->Start())
		{
			AUDIO_PROCESSING_PRINTF("start playback channel failed!");
			return Audio_Module_State_Inited;
		}
		if(Audio_Module_State_Running != m_pCaptureChannel->Start())
		{
			AUDIO_PROCESSING_PRINTF("start capture channel failed!");
			return Audio_Module_State_Inited;
		}
		if (NULL == m_pAudioDeviceManager || !m_pAudioDeviceManager->Start())
		{
			AUDIO_PROCESSING_PRINTF("start AudioDeviceManager failed!");
			return Audio_Module_State_Inited;
		}
		m_bStarted=true;
	}
	m_MemberLock->Leave();
	return Audio_Module_State_Running;
}

ModuleState AudioEngineImpl::Stop()
{
	m_MemberLock->Enter();

	if(m_bStarted)
	{
		if (NULL != m_pAudioDeviceManager)
		{
			m_pAudioDeviceManager->Stop();
		}
		m_pCaptureChannel->Stop();
		if(NULL!=m_pPlaybackChannel)
		{
			m_pPlaybackChannel->Stop();
		}
		if(NULL!=m_pSharedChannel)
		{
			m_pSharedChannel->Stop();
		}

		m_bStarted=false;
	}
	m_MemberLock->Leave();
	return Audio_Module_State_Stopped;
}

bool AudioEngineImpl::RegisterTrinutaryReceiverToSharedChannel()
{
	m_MemberLock->Enter();
	AudioTributaryReceiver* a_pAudioTransport = NULL;
	CAUDIO_U8_t tributaryReceiverIdx = 0;

	if(NULL != m_pPlaybackChannel)
	{
		// register transport to playback channel
		a_pAudioTransport = m_pPlaybackChannel->GetAudioTributaryReceiver();
		if(NULL == a_pAudioTransport)
		{
			AUDIO_PROCESSING_PRINTF("cannot get transport from playback channel");
			return false;
		}
		tributaryReceiverIdx = a_pAudioTransport->SetDSId(DSFromSharedCh);
		if(!m_pSharedChannel->RegisterAudioTransport(
			static_cast<AudioTributaryReceiver*>(a_pAudioTransport),
			TP2RenderCh,
			tributaryReceiverIdx))
		{
			return false;
		}
	}
	// register transport to capture channel
	a_pAudioTransport = NULL;
	a_pAudioTransport = m_pCaptureChannel->GetAudioTributaryReceiver();
	if(NULL == a_pAudioTransport)
	{
		AUDIO_PROCESSING_PRINTF("cannot get transport from capture channel");
		return false;
	}
	tributaryReceiverIdx = a_pAudioTransport->SetDSId(DSFromSharedCh);
	if(!m_pSharedChannel->RegisterAudioTransport(
		a_pAudioTransport, 
		TP2CaptureCh, 
		tributaryReceiverIdx))
	{
		return false;
	}

	m_MemberLock->Leave();
	return true;
}

bool AudioEngineImpl::RegisterTrinutaryReceiverToPlaybackChannel()
{
	m_MemberLock->Enter();
	AudioTributaryReceiver* a_pAudioTransport = NULL;
	CAUDIO_U8_t tributaryReceiverIdx = 0;
	if(NULL != m_pPlaybackChannel)
	{
		// register transport to capture channel
		a_pAudioTransport = m_pCaptureChannel->GetAudioTributaryReceiver();
		if(NULL == a_pAudioTransport)
		{
			AUDIO_PROCESSING_PRINTF("cannot get transport from capture channel");
			return false;
		}

		tributaryReceiverIdx = a_pAudioTransport->SetDSId(DSFromRenderCh);
		if(!m_pPlaybackChannel->RegisterAudioTransport(
			static_cast<AudioTributaryReceiver*>(a_pAudioTransport),
			TP2CaptureCh,
			tributaryReceiverIdx))
		{
			return false;
		}
	}

	m_MemberLock->Leave();
	return true;
}

