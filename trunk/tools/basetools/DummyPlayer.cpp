/*	
 *	Name:			DummyPlayer.cpp
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					12/25/2014 Created
 *
 *
 */
#include "DummyPlayer.h"
#include <stdlib.h>
#include <Windows.h>
#include "audiotrace.h"

DummyPlayer::DummyPlayer(
	AudioDeviceTransport *const a_pAudioDeviceTransport,
	const CAUDIO_U32_t a_nFs,
	const CAUDIO_U32_t a_nFrameTime,
	const DATA_TYPE a_eAudioDataType,
	const TransportId_e a_eTPId,
	const DummyPlayerMode a_eDummyPlayerMode,
	const char* a_pOutFileName,
	const char* a_pPlayThreadName)

	: m_pWritefile(NULL)
	, m_pOutFileName(const_cast<char*>(a_pOutFileName))
	, m_nFs(a_nFs)
	, m_eTPId(a_eTPId)
	, m_nFrameTime(a_nFrameTime)
	, m_nFrameSize(FRAM_LEN(m_nFrameTime, m_nFs))
	, m_dataType(a_eAudioDataType)
	, m_eDummyPlayerMode(a_eDummyPlayerMode)
	, m_nMinFrameSize(0)
	, m_nMaxMultipleOfMinFrameSize(0)
	, m_nWriteLen(0)
	, m_pOutputBuffer(NULL)
	, m_nOutputBufferPtr(0)
	, m_pProcessBuffer(NULL)
	, m_pAudioDeviceTransport(a_pAudioDeviceTransport)
	, m_pPlayerThreadName(a_pPlayThreadName)
	, m_bIsStartPlay(false)
	, m_bIsStopPlay(false)
	, m_nThreadIdx(0)
	, m_currentTime(0)
	, m_startTime(0)
	
{
	if (m_pWritefile)
	{
		delete m_pWritefile;
		m_pWritefile = NULL;
	}
	m_pWritefile=new CWavFileOp(m_pOutFileName,"wb");
	if (m_pWritefile->m_FileStatus==-2)
	{
		printf("open write file %s failed", m_pOutFileName);
		delete m_pWritefile;
		m_pWritefile = NULL;
	}

	if (m_pOutputBuffer)
	{
		delete[]m_pOutputBuffer;
		m_pOutputBuffer = NULL;
	}
	m_pOutputBuffer = new CAUDIO_S16_t[m_nFrameSize*kMaxMultipleOfFrameSize];
	memset(m_pOutputBuffer, 0, sizeof(CAUDIO_S16_t)*m_nFrameSize*kMaxMultipleOfFrameSize);

	if (m_pProcessBuffer)
	{
		delete[]m_pProcessBuffer;
		m_pProcessBuffer = NULL;
	}
	m_pProcessBuffer = new AUDIO_DATA_TYPE[m_nFrameSize * kMaxMultipleOfFrameSize];
	memset(m_pProcessBuffer, 0, sizeof(AUDIO_DATA_TYPE)*m_nFrameSize * kMaxMultipleOfFrameSize);
}

DummyPlayer::~DummyPlayer()
{
	// must shut down player thread before destruct
	m_bIsStopPlay = true;

	m_pWritefile->UpdateHeader(m_WaveHead.NChannels, m_nWriteLen);
	if(NULL != m_pWritefile)
	{
		delete m_pWritefile;
		m_pWritefile = NULL;
	}
	if (m_pOutputBuffer)
	{
		delete[]m_pOutputBuffer;
		m_pOutputBuffer = NULL;
	}
	if (m_pProcessBuffer)
	{
		delete[]m_pProcessBuffer;
		m_pProcessBuffer = NULL;
	}
}

/*static */bool DummyPlayer::PlayerThreadFunction(ThreadObj threadObj)
{
	return reinterpret_cast<DummyPlayer*>(threadObj)->Play();
}

/*virtual */bool DummyPlayer::processData(const AudioFrame* a_AudioFrameArray[], CAUDIO_U8_t a_nArrayLen)
{
	assert(a_nArrayLen == 3);
	// the first file
	const AudioFrame* ppAudioFrame0 = a_AudioFrameArray[0];
	const AUDIO_FRAME_PARA& frameParam0 =  ppAudioFrame0->AudioPara_;

	const AudioFrame* ppAudioFrame1 = a_AudioFrameArray[1];
	const AUDIO_FRAME_PARA& frameParam1 =  ppAudioFrame1->AudioPara_;

	const AudioFrame* ppAudioFrame2 = a_AudioFrameArray[2];
	const AUDIO_FRAME_PARA& frameParam2 =  ppAudioFrame2->AudioPara_;

	int srcDataSize = frameParam0.samples_per_channel_;

	assert(m_WaveHead.NChannels == frameParam0.num_channels_);
	
	short* data_out_s0=new short[srcDataSize*m_WaveHead.NChannels];
	memset(data_out_s0, 0, srcDataSize*m_WaveHead.NChannels*sizeof(short));
	memcpy(data_out_s0, ppAudioFrame0->data_, frameParam0.num_channels_*frameParam0.samples_per_channel_*sizeof(short));

	short* data_out_s1=new short[srcDataSize*m_WaveHead.NChannels];
	memset(data_out_s1, 0, srcDataSize*m_WaveHead.NChannels*sizeof(short));
	memcpy(data_out_s1, ppAudioFrame1->data_, frameParam1.num_channels_*frameParam1.samples_per_channel_*sizeof(short));
	
	short* data_out_s2=new short[srcDataSize*m_WaveHead.NChannels];
	memset(data_out_s2, 0, srcDataSize*m_WaveHead.NChannels*sizeof(short));
	memcpy(data_out_s2, ppAudioFrame2->data_, frameParam2.num_channels_*frameParam2.samples_per_channel_*sizeof(short));

	short* data_out_s=new short[srcDataSize*m_WaveHead.NChannels];
	for(int i=0; i<srcDataSize*m_WaveHead.NChannels; ++i)
	{
		data_out_s[i] = 1.f/3 * data_out_s0[i] + 1.f/3 * data_out_s1[i] + 1.f/3 * data_out_s2[i];
		//data_out_s[i] = data_out_s0[i];
	}

	m_nWriteLen += srcDataSize;
	m_pWritefile->WriteSample(data_out_s, (srcDataSize*m_WaveHead.NChannels));

	delete data_out_s;
	delete data_out_s0;
	delete data_out_s1;
	delete data_out_s2;

	return true;
}

/*virtual*/ bool DummyPlayer::Transport(const void* a_pData, DATA_TYPE a_eDataType, CAUDIO_U32_t a_nSize, TransportId_e a_eTPId)
{
	// call Transport() only in push mode.
	if (SYNC_PUSH_MODE != m_eDummyPlayerMode)
	{
		printf("SYNC_PUSH_MODE != m_eDummyPlayerMode");
		return false;
	}

	if(m_eTPId != a_eTPId)
	{
		printf("transport is not correct");
		return true;
	}

	for(int i=0; i<a_nSize; ++i)
	{
		for(int j=0; j<m_WaveHead.NChannels; ++j)
		{
			m_pOutputBuffer[m_nOutputBufferPtr + i*m_WaveHead.NChannels + j] = ((short*)a_pData)[i];
		}
	}
	m_nOutputBufferPtr += a_nSize*m_WaveHead.NChannels;
	m_nWriteLen += a_nSize;

	while(m_nOutputBufferPtr >= m_nFrameSize*m_WaveHead.NChannels)
	{
		m_pWritefile->WriteSample((short*)m_pOutputBuffer, (m_nFrameSize*m_WaveHead.NChannels));
		AUDIO_PROCESSING_PRINTF("transport id is %d, data size is %d", m_eTPId, m_nFrameSize);
		m_nOutputBufferPtr -= m_nFrameSize*m_WaveHead.NChannels;
		memcpy(m_pOutputBuffer, m_pOutputBuffer + m_nFrameSize*m_WaveHead.NChannels, sizeof(CAUDIO_S16_t)*m_nOutputBufferPtr);
	}

	return true;
}

void DummyPlayer::setWaveHead(SWavFileHead& a_WaveHead)
{
	m_WaveHead = a_WaveHead;
	// write 2 channels
	m_WaveHead.NChannels=2;
	m_pWritefile->WriteHeader(m_WaveHead);
}

void DummyPlayer::setMinFrameSize(int a_nMinSize)
{
	m_nMinFrameSize = a_nMinSize;
}

void DummyPlayer::setMaxMultipleOfMinFrameSize(int a_nMaxSize)
{
	m_nMaxMultipleOfMinFrameSize = a_nMaxSize;
}

DummyPlayerMode DummyPlayer::GetDummyPlayerMode()
{
	if (m_eDummyPlayerMode)
	{
		return m_eDummyPlayerMode;
	}
	else
	{
		return SYNC_NONE;
	}
}

bool DummyPlayer::StopPlay()
{
	m_bIsStopPlay = true; 

	return true;
}

bool DummyPlayer::StartPlay()
{
	bool retval = false;
	if (SYNC_PULL_MODE != m_eDummyPlayerMode)
	{
		printf("SYNC_PULL_MODE != m_eDummyPlayerMode");
		return false;
	}
	if (NULL == m_pAudioDeviceTransport)
	{
		return false;
	}
	m_PlayerThread.reset(ThreadWrapper::CreateThread(PlayerThreadFunction,
		this, webrtc::kHighPriority, m_pPlayerThreadName));
	if (NULL == m_PlayerThread.get())
	{
		return false;
	}
	else
	{
		retval = m_PlayerThread->Start(m_nThreadIdx);
		if (false == retval)
		{
			return false;
		}
	}
	m_Timer.reset(EventWrapper::Create());
	if (NULL == m_Timer.get())
	{
		return false;
	}
	m_bIsStartPlay = true;
	m_startTime = clock();

	return true;
}

bool DummyPlayer::Play()
{
	CAUDIO_U32_t actualSampleSize = 0;
	AUDIO_DATA_TYPE copyData_f = 0.f;
	CAUDIO_S16_t copyData_s = 0;
	CAUDIO_U32_t retval = -1;
	CAUDIO_U32_t duration = 0;

	if (!m_bIsStartPlay)
	{
		return true;
	}
	if (SYNC_PULL_MODE != m_eDummyPlayerMode || NULL == m_pAudioDeviceTransport)
	{
		return false;
	}

	retval = m_pAudioDeviceTransport->NeedMorePlayData(m_nFrameSize, sizeof(m_dataType), m_WaveHead.NChannels, m_nFs, m_pProcessBuffer, actualSampleSize);
	if (0 != retval)
	{
		AUDIO_PROCESSING_PRINTF("NeedMorePlayData failed.");
		return false;
	}

	for (int i = 0; i < actualSampleSize; ++i)
	{
		copyData_f = m_pProcessBuffer[i*m_WaveHead.NChannels] * 32767;
		if (copyData_f > 32767)
		{
			copyData_s = 32767;
		}
		else if (copyData_f < -32768)
		{
			copyData_s = -32768;
		}
		else
		{
			copyData_s = static_cast<CAUDIO_S16_t>(copyData_f);
		}
		for (int j = 0; j < m_WaveHead.NChannels; ++j)
		{
			m_pOutputBuffer[m_nOutputBufferPtr + i*m_WaveHead.NChannels + j] = copyData_s;
		}
	}
	m_nOutputBufferPtr += actualSampleSize*m_WaveHead.NChannels;
	m_nWriteLen += actualSampleSize;

	while (m_nOutputBufferPtr >= m_nFrameSize*m_WaveHead.NChannels)
	{
		m_pWritefile->WriteSample((short*)m_pOutputBuffer, (m_nFrameSize*m_WaveHead.NChannels));
		AUDIO_PROCESSING_PRINTF("transport id is %d, data size is %d", m_eTPId, m_nFrameSize);
		m_nOutputBufferPtr -= m_nFrameSize*m_WaveHead.NChannels;
		memcpy(m_pOutputBuffer, m_pOutputBuffer + m_nFrameSize*m_WaveHead.NChannels, sizeof(CAUDIO_S16_t)*m_nOutputBufferPtr);
	}

	m_currentTime = clock();
	duration = (m_currentTime - m_startTime);
	printf("duration: %ld\n", m_nFrameTime - duration);
	if (duration < m_nFrameTime)
	{
		Sleep(m_nFrameTime - duration);
	}
	m_startTime = clock();

	//// start timer
	//m_Timer->StartTimer(false, m_nFrameTime);
	//webrtc::EventTypeWrapper st = m_Timer->Wait(m_nFrameTime);
	//if (webrtc::kEventSignaled != st && webrtc::kEventTimeout != st)
	//{
	//	printf("read timer signal missing\n");
	//}

	if (m_bIsStopPlay)
	{
		m_PlayerThread->SetNotAlive();
		m_PlayerThread->Stop();
	}

	return true;
}


