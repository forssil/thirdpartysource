/*  @file    AudioDeviceTransport.cpp
*	@author  Keil
*	@brief   AudioDeviceTransport is used as a sink between AudioDevice and AudioChannel.
*/

#include "AudioDeviceTransport.h"

AudioDeviceTransport::AudioDeviceTransport(
	const IAudioEngine *pAudioEngine,
	const DATA_TYPE eDataType)

	: m_pAudioEngine(const_cast<IAudioEngine*>(pAudioEngine))
	, m_eDataType(eDataType)
	, m_pPlaybackBuffer(NULL)
	, m_nPlaybackBufferWritePos(0)
	, m_nPlaybackBufferReadPos(0)
	, m_pRecordBuffer(NULL)
	, m_nRecordBufferSize(0)
	, m_pRecordWavFileOp(NULL)
	, m_pRecordWavFileHead(NULL)
	, m_nRecordWaveCounter(0)
	, m_pPlayoutWavFileOp(NULL)
	, m_pPlayoutWavFileHead(NULL)
	, m_nPlayoutWaveCounter(0)
{
}

/*virtual */AudioDeviceTransport::~AudioDeviceTransport()
{
	Release();
}

bool AudioDeviceTransport::Init()
{
	if (NULL == m_pAudioEngine)
	{
		AUDIO_PROCESSING_PRINTF("m_pAudioEngine == NULL.");
		return false;
	}
	if (NULL != m_pPlaybackBuffer)
	{
		delete[]m_pPlaybackBuffer;
	}
	m_pPlaybackBuffer = new CAUDIO_S16_t[kPlaybackBufferSamples];
	if (NULL == m_pPlaybackBuffer)
	{
		AUDIO_PROCESSING_PRINTF("new m_playbackBuffer failed.");
		return false;
	}
	return true;
}

inline void AudioDeviceTransport::Reset()
{

}

inline void AudioDeviceTransport::Release()
{
	if (NULL != m_pPlaybackBuffer)
	{
		delete []m_pPlaybackBuffer;
	}
}


/*virtual */int32_t AudioDeviceTransport::RecordedDataIsAvailable(
	const void* audioSamples,
	const uint32_t nSamples,
	const uint8_t nBytesPerSample,
	const uint8_t nChannels,
	const uint32_t samplesPerSec,
	const uint32_t totalDelayMS,
	const int32_t clockDrift,
	const uint32_t currentMicLevel,
	const bool keyPressed,
	uint32_t& newMicLevel)
{
	bool retval = false;

	// Safety inspection
	if (NULL == audioSamples)
	{
		AUDIO_PROCESSING_PRINTF("audioSamples is NULL");
		return -1;
	}
	if (!SAMPLE_RATE_VALID_CHECK(samplesPerSec))
	{
		AUDIO_PROCESSING_PRINTF("samplesPerSec don't match");
		return -1;
	}
	if (NULL == m_pAudioEngine)
	{
		AUDIO_PROCESSING_PRINTF("m_pAudioEngine == NULL.");
		return -1;
	}
	// type cast
	if (!TypeCast_Record(reinterpret_cast<const CAUDIO_S16_t*>(audioSamples), nSamples, nChannels))
	{
		AUDIO_PROCESSING_PRINTF("TypeCast failed.");
		return -1;
	}

#ifdef AUDIO_WAVE_DEBUG
	WriteWaveFile(reinterpret_cast<const CAUDIO_S16_t*>(audioSamples), nSamples*nChannels, 0, RECORD_HARDWARE_DEVICE);
#endif

	// TODO: we need to pass ChannelId and ThreadId into AudioDeviceTransport for later optimization.
	retval = m_pAudioEngine->FeedData(CaptureCh, m_pRecordBuffer, nSamples, 0, samplesPerSec);

	if (false == retval)
	{
		AUDIO_PROCESSING_PRINTF("FeedData failed.");
		return -1;
	}
	AUDIO_PROCESSING_PRINTF("channel id is %d, thread id is %d, data size is %d", CaptureCh, 0, nSamples*nChannels);

	return 0;
}

/*virtual */int32_t AudioDeviceTransport::NeedMorePlayData(
	const uint32_t nSamples,
	const uint8_t nBytesPerSample,
	const uint8_t nChannels,
	const uint32_t samplesPerSec,
	void* audioSamples,
	uint32_t& nSamplesOut)
{
	bool retval = false;
	CAUDIO_U32_t needDataLength = 0;
	CAUDIO_U32_t actualDataLength = 0;
	CAUDIO_U32_t copySize = 0;
	CAUDIO_U32_t copySize2 = 0;
	// Safety inspection
	if (NULL == audioSamples)
	{
		AUDIO_PROCESSING_PRINTF("audioSamples is NULL");
		return -1;
	}
	if (!SAMPLE_RATE_VALID_CHECK(samplesPerSec))
	{
		AUDIO_PROCESSING_PRINTF("samplesPerSec don't match");
		return -1;
	}
	if (NULL == m_pAudioEngine)
	{
		AUDIO_PROCESSING_PRINTF("m_pAudioEngine == NULL.");
		return -1;
	}
	needDataLength = nSamples*nChannels;
	actualDataLength = m_nPlaybackBufferWritePos >= m_nPlaybackBufferReadPos 
		? m_nPlaybackBufferWritePos - m_nPlaybackBufferReadPos 
		: m_nPlaybackBufferWritePos + kPlaybackBufferSamples - m_nPlaybackBufferReadPos;

	// reset playback buffer when data accumulated more than 5s.
	if (actualDataLength > kPlaybackBufferSamples/2)
	{
		memset(m_pPlaybackBuffer, 0, kPlaybackBufferSamples*sizeof(CAUDIO_S16_t));
		m_nPlaybackBufferWritePos = 0;
		m_nPlaybackBufferReadPos = 0;
		actualDataLength = 0;
	}

	while (needDataLength > actualDataLength)
	{
		//we need pull data from playback channel, 
		AudioFrame DataPulledFromPlaybackChannel[MAX_THREAD_NUM];
		CAUDIO_S32_t channelNumOfAudioFrame = 0;
		CAUDIO_S32_t samplesPerChannelOfAudioFrame = 0;
		AUDIO_DATA_TYPE* dataOfAudioFrame = NULL;

		retval = m_pAudioEngine->NeedData(DataPulledFromPlaybackChannel, MAX_THREAD_NUM, samplesPerSec, nChannels);
		if (false == retval)
		{
			AUDIO_PROCESSING_PRINTF("NeedData() failed.");
			return -1;
		}
		channelNumOfAudioFrame = DataPulledFromPlaybackChannel[0].AudioPara_.num_channels_;
		samplesPerChannelOfAudioFrame = DataPulledFromPlaybackChannel[0].AudioPara_.samples_per_channel_;
		dataOfAudioFrame = reinterpret_cast<AUDIO_DATA_TYPE*>(DataPulledFromPlaybackChannel[0].data_);
		// Safety inspection
		if (nChannels != channelNumOfAudioFrame || NULL == dataOfAudioFrame)
	    {
			return -1;
	    }
		// Copy data from playback channel to AudioDeviceTransport
		if (!TypeCast_PlayOut(dataOfAudioFrame, samplesPerChannelOfAudioFrame, channelNumOfAudioFrame))
		{
			AUDIO_PROCESSING_PRINTF("TypeCast_PlayOut() failed.");
			return -1;
		}
		actualDataLength = m_nPlaybackBufferWritePos >= m_nPlaybackBufferReadPos
			? m_nPlaybackBufferWritePos - m_nPlaybackBufferReadPos
			: m_nPlaybackBufferWritePos + kPlaybackBufferSamples - m_nPlaybackBufferReadPos;
	}
	// Copy data from AudioDeviceTransport to AudioDevice
	if (m_nPlaybackBufferReadPos + nSamples*nChannels > kPlaybackBufferSamples)
	{
		copySize = kPlaybackBufferSamples - m_nPlaybackBufferReadPos;
		copySize2 = nSamples*nChannels - copySize;
		memcpy_s(
			reinterpret_cast<CAUDIO_S16_t*>(audioSamples),
			copySize*sizeof(CAUDIO_S16_t),
			m_pPlaybackBuffer+m_nPlaybackBufferReadPos,
			copySize*sizeof(CAUDIO_S16_t));

		memcpy_s(
			reinterpret_cast<CAUDIO_S16_t*>(audioSamples)+copySize,
			copySize2*sizeof(CAUDIO_S16_t),
			m_pPlaybackBuffer,
			copySize2*sizeof(CAUDIO_S16_t));

		m_nPlaybackBufferReadPos = copySize2;

#ifdef AUDIO_WAVE_DEBUG
		WriteWaveFile(reinterpret_cast<CAUDIO_S16_t*>(audioSamples), copySize + copySize2, 0, PLAYOUT_HARDWARE_DEVICE);
#endif
	}
	else
	{
		copySize = nSamples*nChannels;
		memcpy_s(
			reinterpret_cast<CAUDIO_S16_t*>(audioSamples),
			copySize*sizeof(CAUDIO_S16_t),
			m_pPlaybackBuffer+m_nPlaybackBufferReadPos,
			copySize*sizeof(CAUDIO_S16_t));

		m_nPlaybackBufferReadPos += copySize;

#ifdef AUDIO_WAVE_DEBUG
		WriteWaveFile(reinterpret_cast<CAUDIO_S16_t*>(audioSamples), copySize, 0, PLAYOUT_HARDWARE_DEVICE);
#endif
	}
	nSamplesOut = nSamples;
	AUDIO_PROCESSING_PRINTF("data size is %d", nSamples*nChannels);

	return 0;
}

bool AudioDeviceTransport::TypeCast_Record(const CAUDIO_S16_t *pData, const CAUDIO_U32_t nSize, const CAUDIO_U32_t nChannels)
{
	CAUDIO_S16_t nCastData = 0;
	// realloc record buffer
	if (!AllocRecordBuffer_Record(nSize))
	{
		AUDIO_PROCESSING_PRINTF("alloc record buffer failed!");
		return false;
	}
	// type cast
	for (CAUDIO_U32_t i = 0; i < nSize; ++i)
	{
		m_pRecordBuffer[i] = static_cast<AUDIO_DATA_TYPE>(pData[i*nChannels])/32768;
	}

	return true;
}

bool AudioDeviceTransport::AllocRecordBuffer_Record(const CAUDIO_U32_t nSize)
{
	if (NULL != m_pRecordBuffer && m_nRecordBufferSize == nSize)
	{
		return true;
	}
	if (NULL != m_pRecordBuffer)
	{
		delete[] m_pRecordBuffer;
		m_pRecordBuffer = NULL;
	}
	m_pRecordBuffer = new AUDIO_DATA_TYPE[nSize];
	if (NULL == m_pRecordBuffer)
	{
		AUDIO_PROCESSING_PRINTF("alloc record buffer failed!");
		return false;
	}
	memset(m_pRecordBuffer, 0, sizeof(AUDIO_DATA_TYPE)*nSize);
	m_nRecordBufferSize = nSize;

	return true;
}

bool AudioDeviceTransport::TypeCast_PlayOut(const AUDIO_DATA_TYPE *pData, const CAUDIO_U32_t nSize, const CAUDIO_U32_t nChannels)
{
	CAUDIO_U32_t copySize = 0;
	CAUDIO_U32_t copySize2 = 0;
	CAUDIO_S16_t *pDest = NULL;
	AUDIO_DATA_TYPE *pSrc = NULL;
	AUDIO_DATA_TYPE nCastData = 0;

	if (nSize*nChannels + m_nPlaybackBufferWritePos > kPlaybackBufferSamples)
	{
		copySize = kPlaybackBufferSamples - m_nPlaybackBufferWritePos;
		copySize2 = nSize*nChannels - copySize;
		pSrc  = const_cast<AUDIO_DATA_TYPE*>(pData);
		pDest = m_pPlaybackBuffer + m_nPlaybackBufferWritePos;
		for (CAUDIO_U32_t i = 0; i < copySize; ++i)
		{
			nCastData = pSrc[i] * 32768;
			if (nCastData > 32767)
			{
				pDest[i] = 32767;
			}
			else if (nCastData < -32768)
			{
				pDest[i] = -32768;
			}
			else
			{
				pDest[i] = static_cast<CAUDIO_S16_t>(nCastData);
			}
		}
		
		pSrc = const_cast<AUDIO_DATA_TYPE*>(pData)+copySize;
		pDest = m_pPlaybackBuffer;
		for (CAUDIO_U32_t i = 0; i < copySize2; ++i)
		{
			nCastData = pSrc[i] * 32768;
			if (nCastData > 32767)
			{
				pDest[i] = 32767;
			}
			else if (nCastData < -32768)
			{
				pDest[i] = -32768;
			}
			else
			{
				pDest[i] = static_cast<CAUDIO_S16_t>(nCastData);
			}
		}
		m_nPlaybackBufferWritePos = copySize2;
	}
	else
	{
		copySize = nSize*nChannels;
		pSrc = const_cast<AUDIO_DATA_TYPE*>(pData);
		pDest = m_pPlaybackBuffer + m_nPlaybackBufferWritePos;
		for (CAUDIO_U32_t i = 0; i < copySize; ++i)
		{
			nCastData = pSrc[i] * 32768;
			if (nCastData > 32767)
			{
				pDest[i] = 32767;
			}
			else if (nCastData < -32768)
			{
				pDest[i] = -32768;
			}
			else
			{
				pDest[i] = static_cast<CAUDIO_S16_t>(nCastData);
			}
		}

		m_nPlaybackBufferWritePos += copySize;
	}

	return true;
}

bool AudioDeviceTransport::InitWaveFile(
	const std::string sFolderName,
	const CAUDIO_U32_t nSampleRate,
	const CAUDIO_U8_t nAudioDeviceId,
	const HardwareDeviceType eHardwareDeviceType)
{
	char szWaveName[256] = "";

	switch (eHardwareDeviceType)
	{
	case RECORD_HARDWARE_DEVICE:
		memset(szWaveName, 0, sizeof(char) * 256);
		sprintf_s(szWaveName, sizeof(char) * 256, "%s\\pRecord.wav", sFolderName.data());
		if (m_pRecordWavFileOp)
		{
			delete m_pRecordWavFileOp;
			m_pRecordWavFileOp = NULL;
		}
		m_pRecordWavFileOp = new CWavFileOp(szWaveName, "wb");
		if (NULL == m_pRecordWavFileOp)
		{
			AUDIO_PROCESSING_PRINTF("new CWavFileOp failed.");
			return false;
		}

		if (m_pRecordWavFileHead)
		{
			delete m_pRecordWavFileHead;
			m_pRecordWavFileHead = NULL;
		}
		m_pRecordWavFileHead = new SWavFileHead;
		if (NULL == m_pRecordWavFileHead)
		{
			AUDIO_PROCESSING_PRINTF("new SWavFileHead failed.");
			return false;
		}

		if (-2 == m_pRecordWavFileOp->m_FileStatus)
		{
			delete m_pRecordWavFileOp;
			delete m_pRecordWavFileHead;
			m_pRecordWavFileOp = NULL;
			m_pRecordWavFileHead = NULL;
			return false;
		}
		m_pRecordWavFileHead->NChannels = 2;
		m_pRecordWavFileHead->RIFF[0] = 'R';
		m_pRecordWavFileHead->RIFF[1] = 'I';
		m_pRecordWavFileHead->RIFF[2] = 'F';
		m_pRecordWavFileHead->RIFF[3] = 'F';
		m_pRecordWavFileHead->data[0] = 'd';
		m_pRecordWavFileHead->data[1] = 'a';
		m_pRecordWavFileHead->data[2] = 't';
		m_pRecordWavFileHead->data[3] = 'a';
		m_pRecordWavFileHead->WAVEfmt_[0] = 'W';
		m_pRecordWavFileHead->WAVEfmt_[1] = 'A';
		m_pRecordWavFileHead->WAVEfmt_[2] = 'V';
		m_pRecordWavFileHead->WAVEfmt_[3] = 'E';
		m_pRecordWavFileHead->WAVEfmt_[4] = 'f';
		m_pRecordWavFileHead->WAVEfmt_[5] = 'm';
		m_pRecordWavFileHead->WAVEfmt_[6] = 't';
		m_pRecordWavFileHead->WAVEfmt_[7] = ' ';
		m_pRecordWavFileHead->noUse = 0x00000010;
		m_pRecordWavFileHead->FormatCategory = 1;
		m_pRecordWavFileHead->SampleRate = nSampleRate;
		m_pRecordWavFileHead->SampleBytes = nSampleRate * 4;
		m_pRecordWavFileHead->BytesPerSample = 4;
		m_pRecordWavFileHead->NBitsPersample = 16;

		m_nRecordWaveCounter = 0;
		if (m_pRecordWavFileOp)
			m_pRecordWavFileOp->WriteHeader(*m_pRecordWavFileHead);

		break;
	case PLAYOUT_HARDWARE_DEVICE:
		memset(szWaveName, 0, sizeof(char) * 256);
		sprintf_s(szWaveName, sizeof(char) * 256, "%s\\pPlayout.wav", sFolderName.data());
		if (m_pPlayoutWavFileOp)
		{
			delete m_pPlayoutWavFileOp;
			m_pPlayoutWavFileOp = NULL;
		}
		m_pPlayoutWavFileOp = new CWavFileOp(szWaveName, "wb");
		if (NULL == m_pPlayoutWavFileOp)
		{
			AUDIO_PROCESSING_PRINTF("new CWavFileOp failed.");
			return false;
		}

		if (m_pPlayoutWavFileHead)
		{
			delete m_pPlayoutWavFileHead;
			m_pPlayoutWavFileHead = NULL;
		}
		m_pPlayoutWavFileHead = new SWavFileHead;
		if (NULL == m_pPlayoutWavFileHead)
		{
			AUDIO_PROCESSING_PRINTF("new SWavFileHead failed.");
			return false;
		}

		if (-2 == m_pPlayoutWavFileOp->m_FileStatus)
		{
			delete m_pPlayoutWavFileOp;
			delete m_pPlayoutWavFileHead;
			m_pPlayoutWavFileOp = NULL;
			m_pPlayoutWavFileHead = NULL;
			return false;
		}
		m_pPlayoutWavFileHead->NChannels = 2;
		m_pPlayoutWavFileHead->RIFF[0] = 'R';
		m_pPlayoutWavFileHead->RIFF[1] = 'I';
		m_pPlayoutWavFileHead->RIFF[2] = 'F';
		m_pPlayoutWavFileHead->RIFF[3] = 'F';
		m_pPlayoutWavFileHead->data[0] = 'd';
		m_pPlayoutWavFileHead->data[1] = 'a';
		m_pPlayoutWavFileHead->data[2] = 't';
		m_pPlayoutWavFileHead->data[3] = 'a';
		m_pPlayoutWavFileHead->WAVEfmt_[0] = 'W';
		m_pPlayoutWavFileHead->WAVEfmt_[1] = 'A';
		m_pPlayoutWavFileHead->WAVEfmt_[2] = 'V';
		m_pPlayoutWavFileHead->WAVEfmt_[3] = 'E';
		m_pPlayoutWavFileHead->WAVEfmt_[4] = 'f';
		m_pPlayoutWavFileHead->WAVEfmt_[5] = 'm';
		m_pPlayoutWavFileHead->WAVEfmt_[6] = 't';
		m_pPlayoutWavFileHead->WAVEfmt_[7] = ' ';
		m_pPlayoutWavFileHead->noUse = 0x00000010;
		m_pPlayoutWavFileHead->FormatCategory = 1;
		m_pPlayoutWavFileHead->SampleRate = nSampleRate;
		m_pPlayoutWavFileHead->SampleBytes = nSampleRate * 4;
		m_pPlayoutWavFileHead->BytesPerSample = 4;
		m_pPlayoutWavFileHead->NBitsPersample = 16;

		m_nPlayoutWaveCounter = 0;
		if (m_pPlayoutWavFileOp)
			m_pPlayoutWavFileOp->WriteHeader(*m_pPlayoutWavFileHead);

		break;
	default:
		AUDIO_PROCESSING_PRINTF("NO_HARDWARE_DEVICE == eHardwareDeviceType.");
		return false;
	}

	return true;
}

bool AudioDeviceTransport::ReleaseWaveFile(
	const CAUDIO_U8_t nAudioDeviceId,
	const HardwareDeviceType eHardwareDeviceType)
{
	switch (eHardwareDeviceType)
	{
	case RECORD_HARDWARE_DEVICE:
		if (m_pRecordWavFileOp && m_pRecordWavFileHead)
		{
			m_pRecordWavFileOp->UpdateHeader(m_pRecordWavFileHead->NChannels, m_nRecordWaveCounter / m_pRecordWavFileHead->NChannels);
			delete m_pRecordWavFileOp;
			delete m_pRecordWavFileHead;
			m_pRecordWavFileOp = NULL;
			m_pRecordWavFileHead = NULL;
		}
		break;
	case PLAYOUT_HARDWARE_DEVICE:
		if (m_pPlayoutWavFileOp && m_pPlayoutWavFileHead)
		{
			m_pPlayoutWavFileOp->UpdateHeader(m_pPlayoutWavFileHead->NChannels, m_nPlayoutWaveCounter / m_pPlayoutWavFileHead->NChannels);
			delete m_pPlayoutWavFileOp;
			delete m_pPlayoutWavFileHead;
			m_pPlayoutWavFileOp = NULL;
			m_pPlayoutWavFileHead = NULL;
		}
		break;
	default:
		AUDIO_PROCESSING_PRINTF("NO_HARDWARE_DEVICE == eHardwareDeviceType.");
		return false;
	}
	return true;
}

bool AudioDeviceTransport::WriteWaveFile(
	const CAUDIO_S16_t *pData,
	const CAUDIO_U32_t nSize,
	const CAUDIO_U8_t nAudioDeviceId,
	const HardwareDeviceType eHardwareDeviceType)
{
	switch (eHardwareDeviceType)
	{
	case RECORD_HARDWARE_DEVICE:
		if (m_pRecordWavFileOp)
		{
			m_pRecordWavFileOp->WriteSample(const_cast<CAUDIO_S16_t*>(pData), nSize);
			m_nRecordWaveCounter += nSize;
		}
		break;
	case PLAYOUT_HARDWARE_DEVICE:
		if (m_pPlayoutWavFileOp)
		{
			m_pPlayoutWavFileOp->WriteSample(const_cast<CAUDIO_S16_t*>(pData), nSize);
			m_nPlayoutWaveCounter += nSize;
		}
		break;
	default:
		AUDIO_PROCESSING_PRINTF("NO_HARDWARE_DEVICE == eHardwareDeviceType.");
		return false;
	}
	return true;

}

