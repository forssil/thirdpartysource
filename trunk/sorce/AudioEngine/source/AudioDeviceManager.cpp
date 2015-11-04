/*  @file    AudioDeviceManager.h
*   @author  Keil
*   @brief   Manage play out and record device, so we can capture audio stream from hardware device
			 and transmit audio stream to hardware device to play out.
*/

#include "AudioDeviceManager.h"
#include "IAudioEngine.h"
#include "audiotrace.h"
#include "AudioDeviceTransport.h"

/*static */IAudioDeviceManager* IAudioDeviceManager::CreateAudioDeviceManager(
	const IAudioEngine *pAudioEngine,
	const DATA_TYPE eDataType,
	const std::string folder_name)
{
	AudioDeviceManager* pAudioDeviceManager = new AudioDeviceManager(pAudioEngine, eDataType, folder_name);

	if (pAudioDeviceManager)
	{
		return dynamic_cast<IAudioDeviceManager*>(pAudioDeviceManager);
	}
	else
	{
		return NULL;
	}
}

/*static */bool IAudioDeviceManager::DeleteAudioDeviceManager(IAudioDeviceManager * pAudioDeviceManager)
{
	if (pAudioDeviceManager)
	{
		delete dynamic_cast<AudioDeviceManager*>(pAudioDeviceManager);
		pAudioDeviceManager = NULL;
		return true;
	}
	else
	{
		return false;
	}
}

AudioDeviceManager::AudioDeviceManager(
	const IAudioEngine *pAudioEngine,
	const DATA_TYPE eDataType,
	const std::string folder_name)

	: m_pAudioEngine(pAudioEngine)
	, m_pAudioDeviceModule(NULL)
	, m_pAudioDeviceTransport(NULL)
	, m_eDataType(eDataType)
	, m_sFolderName(folder_name)
	, m_bInitSuccess(false)
	, m_bStartSuccess(false)

{

}

AudioDeviceManager::~AudioDeviceManager()
{
	
}

/*virtual */bool AudioDeviceManager::Init()
{
	int retval = -1;
	CAUDIO_U32_t recordSampleRate = 0;
	CAUDIO_U32_t playoutSampleRate = 0;

	// safety inspection
	if (m_bInitSuccess)
	{
		return true;
	}
	if (NULL == m_pAudioEngine)
	{
		AUDIO_PROCESSING_PRINTF("NULL == m_pAudioEngine.");
		return false;
	}
	// alloc AudioDeviceModule
	if (m_pAudioDeviceModule)
	{
		do 
		{
			retval = m_pAudioDeviceModule->Release();
		} while (retval);
		m_pAudioDeviceModule = NULL;
	}
	m_pAudioDeviceModule = webrtc::CreateAudioDeviceModule(0, webrtc::AudioDeviceModule::kPlatformDefaultAudio);
	if (NULL == m_pAudioDeviceModule)
	{
		AUDIO_PROCESSING_PRINTF("NULL == m_pAudioDeviceModule.");
		return false;
	}
	retval = m_pAudioDeviceModule->AddRef();
	if (0 == retval)
	{
		AUDIO_PROCESSING_PRINTF("Add Ref Failed.");
		return false;
	}
	retval = m_pAudioDeviceModule->Init();
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("Init() Failed.");
		return false;
	}
	retval = m_pAudioDeviceModule->SetRecordingDevice(0);
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("SetRecordingDevice() Failed.");
		return false;
	}
	retval = m_pAudioDeviceModule->SetPlayoutDevice(0);
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("SetPlayoutDevice() Failed.");
		return false;
	}
	retval = m_pAudioDeviceModule->InitPlayout();
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("InitPlayout() Failed.");
		return false;
	}
	retval = m_pAudioDeviceModule->InitRecording();
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("InitRecording() Failed.");
		return false;
	}

	if (!GetSampleRate(0, RECORD_HARDWARE_DEVICE, recordSampleRate))
	{
		AUDIO_PROCESSING_PRINTF("GetSampleRate() Failed.");
		return false;
	}

	if (!GetSampleRate(0, PLAYOUT_HARDWARE_DEVICE, playoutSampleRate))
	{
		AUDIO_PROCESSING_PRINTF("GetSampleRate() Failed.");
		return false;
	}
	// alloc AudioDeviceTransport
	if (m_pAudioDeviceTransport)
	{
		delete m_pAudioDeviceTransport;
		m_pAudioDeviceTransport = NULL;
	}
	m_pAudioDeviceTransport = new AudioDeviceTransport(m_pAudioEngine, m_eDataType);
	if (NULL == m_pAudioDeviceTransport)
	{
		AUDIO_PROCESSING_PRINTF("new AudioDeviceTransport Failed.");
		return false;
	}
	if (!m_pAudioDeviceTransport->Init())
	{
		AUDIO_PROCESSING_PRINTF("new m_pAudioDeviceTransport Init() Failed.");
		return false;
	}
	// register AudioDeviceTransport into AudioDeviceModule.
	retval = m_pAudioDeviceModule->RegisterAudioCallback(m_pAudioDeviceTransport);
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("RegisterAudioCallback() Failed.");
		return false;
	}
	// init wave file
#ifdef AUDIO_WAVE_DEBUG
	m_pAudioDeviceTransport->InitWaveFile(m_sFolderName, recordSampleRate, 0, RECORD_HARDWARE_DEVICE);
	m_pAudioDeviceTransport->InitWaveFile(m_sFolderName, playoutSampleRate, 0, PLAYOUT_HARDWARE_DEVICE);
#endif
	// tag
	m_bInitSuccess = true;
	return true;
}

/*virtual */bool AudioDeviceManager::Release()
{
	int nRetval = -1;
	bool bRetval = false;
	// safety inspection
	if (!m_bInitSuccess)
	{
		return true;
	}
	if (m_bStartSuccess)
	{
		bRetval = Stop();
		if (!bRetval)
		{
			AUDIO_PROCESSING_PRINTF("Stop() failed.");
			return false;
		}
	}
	// release wave file
#ifdef AUDIO_WAVE_DEBUG
	m_pAudioDeviceTransport->ReleaseWaveFile(0, RECORD_HARDWARE_DEVICE);
	m_pAudioDeviceTransport->ReleaseWaveFile(0, PLAYOUT_HARDWARE_DEVICE);
#endif
	// release AudioDeviceModule
	nRetval = m_pAudioDeviceModule->Terminate();
	if (0 != nRetval)
	{
		AUDIO_PROCESSING_PRINTF("Terminate() failed.");
		return false;
	}
	nRetval = m_pAudioDeviceModule->Release();
	if (0 != nRetval)
	{
		AUDIO_PROCESSING_PRINTF("Release() failed.");
		return false;
	}
	if (m_pAudioDeviceModule)
	{
		m_pAudioDeviceModule = NULL;
	}
	// release AudioDeviceTransport
	if (m_pAudioDeviceTransport)
	{
		delete m_pAudioDeviceTransport;
		m_pAudioDeviceTransport = NULL;
	}

	// tag
	m_bInitSuccess = false;
	m_bStartSuccess = false;
	return true;
}

/*virtual */bool AudioDeviceManager::Start()
{
	int retval = -1;
	// safety inspection
	if (!m_bInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("we need call Init() failed.");
		return false;
	}
	if (m_bStartSuccess)
	{
		return true;
	}
	// start play out and recording
	retval = m_pAudioDeviceModule->StartPlayout();
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("StartPlayout() failed.");
		return false;
	}
	retval = m_pAudioDeviceModule->StartRecording();
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("StartRecording() failed.");
		return false;
	}
	// tag
	m_bStartSuccess = true;
	return true;
}

/*virtual */bool AudioDeviceManager::Stop()
{
	int retval = -1;
	// safety inspection
	if (!m_bStartSuccess)
	{

		AUDIO_PROCESSING_PRINTF("m_bInitSuccess = false or m_bStartSuccess = false.");
		return false;
	}
	// stop play out and recording 
	retval = m_pAudioDeviceModule->StopPlayout();
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("StopPlayout() failed.");
		return false;
	}
	retval = m_pAudioDeviceModule->StopRecording();
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("StopRecording() failed.");
		return false;
	}

	// tag
	m_bStartSuccess = false;
	return true;
}

/*virtual */bool AudioDeviceManager::GetSampleRate(
	const CAUDIO_U8_t nAudioDeviceId,
	const HardwareDeviceType eHardwareDeviceType,
	CAUDIO_U32_t &nSampleRate) const
{
	CAUDIO_U32_t retval = -1;
	// safety inspection
	if (m_bStartSuccess)
	{
		AUDIO_PROCESSING_PRINTF("m_bStartSuccess.");
		return false;
	}
	// get sample rate
	switch (eHardwareDeviceType)
	{
	case RECORD_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->RecordingSampleRate(&nSampleRate);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call RecordingSampleRate() failed.");
			return false;
		}
		break;
	case PLAYOUT_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->PlayoutSampleRate(&nSampleRate);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call PlayoutSampleRate() failed.");
			return false;
		}
		break;
	default: 
		AUDIO_PROCESSING_PRINTF("NO_HARDWARE_DEVICE == eHardwareDeviceType.");
		return false;
	}
	
	return true;
}

/*virtual */bool AudioDeviceManager::SetSampleRate(
	const CAUDIO_U8_t nAudioDeviceId,
	const HardwareDeviceType eHardwareDeviceType,
	const CAUDIO_U32_t nPresetSampleRate)
{
	CAUDIO_U32_t retval = -1;
	// safety inspection
	if (m_bStartSuccess)
	{
		AUDIO_PROCESSING_PRINTF("m_bStartSuccess.");
		return false;
	}
	//set sample rate
	switch (eHardwareDeviceType)
	{
	case RECORD_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->SetRecordingSampleRate(nPresetSampleRate);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call SetRecordingSampleRate() failed.");
			return false;
		}
		break;
	case PLAYOUT_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->SetPlayoutSampleRate(nPresetSampleRate);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call SetPlayoutSampleRate() failed.");
			return false;
		}
		break;
	default:
		AUDIO_PROCESSING_PRINTF("NO_HARDWARE_DEVICE == eHardwareDeviceType.");
		return false;
	}

	return true;
}

/*virtual */bool AudioDeviceManager::GetVolume(
	const CAUDIO_U8_t nAudioDeviceId,
	const HardwareDeviceType eHardwareDeviceType,
	CAUDIO_U32_t &nVolume) const
{
	CAUDIO_U32_t retval = -1;

	// safety inspection
	if (m_bStartSuccess)
	{
		AUDIO_PROCESSING_PRINTF("m_bStartSuccess.");
		return false;
	}
	// get volume
	switch (eHardwareDeviceType)
	{
	case RECORD_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->MicrophoneVolume(&nVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call MicrophoneVolume() failed.");
			return false;
		}
		break;
	case PLAYOUT_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->SpeakerVolume(&nVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call SpeakerVolume() failed.");
			return false;
		}
		break;
	default:
		AUDIO_PROCESSING_PRINTF("NO_HARDWARE_DEVICE == eHardwareDeviceType.");
		return false;
	}

	return true;
}

/*virtual */bool AudioDeviceManager::SetVolume(
	const CAUDIO_U8_t nAudioDeviceId,
	const HardwareDeviceType eHardwareDeviceType,
	const CAUDIO_U32_t nPresetVolume)
{
	CAUDIO_U32_t retval = -1;
	CAUDIO_U32_t maxVolume = 0;
	CAUDIO_U32_t minVolume = 0;
	CAUDIO_U32_t presetVolume = 0;

	// safety inspection
	if (m_bStartSuccess)
	{
		AUDIO_PROCESSING_PRINTF("m_bStartSuccess.");
		return false;
	}
	//set volume
	switch (eHardwareDeviceType)
	{
	case RECORD_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->MaxMicrophoneVolume(&maxVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call MaxMicrophoneVolume() failed.");
			return false;
		}
		retval = m_pAudioDeviceModule->MinMicrophoneVolume(&minVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call MinMicrophoneVolume() failed.");
			return false;
		}
		presetVolume = presetVolume > maxVolume ? maxVolume : presetVolume;
		presetVolume = presetVolume < minVolume ? minVolume : presetVolume;

		retval = m_pAudioDeviceModule->SetMicrophoneVolume(presetVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call SetMicrophoneVolume() failed.");
			return false;
		}
		break;
	case PLAYOUT_HARDWARE_DEVICE:
		retval = m_pAudioDeviceModule->MaxSpeakerVolume(&maxVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call MaxSpeakerVolume() failed.");
			return false;
		}
		retval = m_pAudioDeviceModule->MinSpeakerVolume(&minVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call MinSpeakerVolume() failed.");
			return false;
		}
		presetVolume = presetVolume > maxVolume ? maxVolume : presetVolume;
		presetVolume = presetVolume < minVolume ? minVolume : presetVolume;

		retval = m_pAudioDeviceModule->SetSpeakerVolume(presetVolume);
		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("call SetSpeakerVolume() failed.");
			return false;
		}
		break;
	default:
		AUDIO_PROCESSING_PRINTF("NO_HARDWARE_DEVICE == eHardwareDeviceType.");
		return false;
	}

	return true;
}

