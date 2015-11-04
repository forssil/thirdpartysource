/*	
 *	Name:			DummyCapturer.cpp
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
#include "DummyCapturer.h"
#include <stdlib.h>
#include <Windows.h>
#include "WaveIO.h"
#include "MultipleSourceSync.h"

DummyCapturer::DummyCapturer(
	AudioDeviceTransport *const a_pAudioDeviceTransport,
	MultipleSourceSync* a_pMultipleSourceSync,
	IAudioEngine* a_pAudioEngine,
	int a_nFileIdx,
	int a_nFrameTime, 
	const char* a_pCaptureThreadName,
	const char* a_pFileName,
	ChannelId_e a_eChannelId,
	int a_nThreadIdx)

	: m_nFileIdx(a_nFileIdx)
	, m_nFrameTime(a_nFrameTime)
	, m_pMultipleSourceSync(a_pMultipleSourceSync)
	, m_pAudioEngine(a_pAudioEngine)
	, m_Timer(EventWrapper::Create())
	, m_nFs(0)
	, m_nMaxMultipleOfMinFrameSize(4)
	, m_pReadFile(NULL)
	, m_CaptureThread(ThreadWrapper::CreateThread(CapturerThreadFunction, this, 
	                  webrtc::kHighPriority, a_pCaptureThreadName))
	, m_nReadFilePtr(0)
	, m_bIsStartCapture(false)
	, m_bIsStopCapture(false)
	, m_nOutLen(0)
	, m_eChannelId(a_eChannelId)
	, m_nThreadIdx(a_nThreadIdx)
	, m_pAudioDeviceTransport(a_pAudioDeviceTransport)
	, m_currentTime(0)
	, m_startTime(0)

{
	// open read file
	if (m_pReadFile)
	{
		delete m_pReadFile;
		m_pReadFile = NULL;
	}
	m_pReadFile = new CWavFileOp(const_cast<char*>(a_pFileName),"rb");
	if (m_pReadFile->m_FileStatus==-2)
	{
		delete m_pReadFile;
		printf("open file : %s failed!\n", a_pFileName);
		return;
	}

	m_pReadFile->ReadHeader(&m_Readwavhead);
	if (m_Readwavhead.NChannels>2)
	{
		return;
	}

	// file len, sample rate
	m_nFileLen = m_Readwavhead.RawDataFileLength/m_Readwavhead.BytesPerSample*m_Readwavhead.NChannels;
	m_nFileLen_f = m_Readwavhead.RawDataFileLength / m_Readwavhead.BytesPerSample;
	m_nFs = m_Readwavhead.SampleRate;

	// alloc memory for whole file
	m_data_in_s=new short[m_nFileLen];
	memset(m_data_in_s, 0, m_nFileLen*sizeof(short));

	m_data_in_f=new float[m_nFileLen];
	memset(m_data_in_f, 0, m_nFileLen*sizeof(float));

	m_data_in_f2 = new float[m_nFileLen_f];
	memset(m_data_in_f2, 0, m_nFileLen_f*sizeof(float));

	// read data
	m_pReadFile->ReadSample(m_data_in_s, m_nFileLen);

	for (int i = 0; i < m_nFileLen_f; ++i)
	{
		m_data_in_f2[i] = m_data_in_s[i*m_Readwavhead.NChannels] / 32768.f;
	}

	// start thread
	unsigned int threadID(0);
	bool isSuccess;
	if (NULL != m_CaptureThread.get())
	{
		isSuccess = m_CaptureThread->Start(threadID);
	}

	if(!isSuccess)
	{
		printf("start thread failed\n");
	}
}

DummyCapturer::~DummyCapturer()
{
	if (NULL != m_data_in_s)
	{
		delete[] m_data_in_s;
		m_data_in_s = NULL;
	}
	if (NULL != m_data_in_f)
	{
		delete[] m_data_in_f;
		m_data_in_f = NULL;
	}
	if (NULL != m_data_in_f2)
	{
		delete[] m_data_in_f2;
		m_data_in_f2 = NULL;
	}
	if (NULL != m_pReadFile)
	{
		delete m_pReadFile;
		m_pReadFile = NULL;
	}
}

bool DummyCapturer::CapturerThreadFunction(ThreadObj threadObj)
{
	return static_cast<DummyCapturer*>(threadObj)->Capture();
}

bool DummyCapturer::Capture()
{
	if(!m_bIsStartCapture)
	{
		return true;
	}

	int multiple = 1;
	int frameSize = multiple * FRAM_LEN(m_nFrameTime, m_nFs);

	CAUDIO_U32_t dataSize = frameSize;
	const AUDIO_DATA_TYPE* pData = m_data_in_f2;
	CAUDIO_U32_t retval = -1;
	CAUDIO_U32_t duration = 0;

	// read data

	float* temFloatBuff = new float[dataSize];
	if (m_nFileLen_f - m_nReadFilePtr <= dataSize)
	{
		// wrap around
		m_nReadFilePtr = 0;
	}
	memcpy_s(temFloatBuff, dataSize*sizeof(float), &m_data_in_f2[m_nReadFilePtr], dataSize*sizeof(float));
	m_nReadFilePtr += dataSize;

	if (NULL != m_pMultipleSourceSync)
	{
		m_pMultipleSourceSync->FeedData(m_data_in_s, REAL_FIX_DATA, dataSize,
			m_nFs, m_Readwavhead.NChannels,m_nFileIdx);
	}
	else if (NULL != m_pAudioEngine)
	{
		m_pAudioEngine->FeedData(m_eChannelId, temFloatBuff, dataSize, m_nThreadIdx, m_nFs);
		AUDIO_PROCESSING_PRINTF("channel id is %d, thread id is %d, data size is %d", m_eChannelId, m_nThreadIdx, dataSize);
	}
	else if (NULL != m_pAudioDeviceTransport)
	{
		CAUDIO_U32_t newMicLevel = 0;
		retval = m_pAudioDeviceTransport->RecordedDataIsAvailable(
			temFloatBuff, 
			dataSize, 
			DataType2Byte(REAL_FLOAT_DATA),
			1, 
			m_nFs, 
			0, 
			0,
			0,
			false,
			newMicLevel);

		if (0 != retval)
		{
			AUDIO_PROCESSING_PRINTF("RecordedDataIsAvailable failed.");
			return false;
		}
	}


	m_currentTime = clock();
	duration = (m_currentTime - m_startTime);
	if (duration < m_nFrameTime)
	{
		Sleep(m_nFrameTime - duration);
	}
	m_startTime = clock();

	//// calculate duration of this frame
	//unsigned long timems = multiple * m_nFrameTime;

	//// start timer
	//m_Timer->StartTimer(false, timems);
	//webrtc::EventTypeWrapper st = m_Timer->Wait(timems);
	//if(webrtc::kEventSignaled != st && webrtc::kEventTimeout != st)
	//{
	//	printf("read timer signal missing\n");
	//}
	//printf("%d ms is passed, file idx:%d\n", timems, m_nFileIdx);

	delete[] temFloatBuff;
	temFloatBuff = NULL;

	if(m_bIsStopCapture)
	{
		m_CaptureThread->SetNotAlive();
		m_CaptureThread->Stop();
	}

	return true;
}

