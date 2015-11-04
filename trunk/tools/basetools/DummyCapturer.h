/*	
 *	Name:			DummyCapturer.h
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

#ifndef _DUMMY_CAPTURER_
#define _DUMMY_CAPTURER_

#include <ctime>
#include "thread_wrapper.h"
#include "event_wrapper.h"
#include "scoped_ptr.h"
#include "scoped_refptr.h"
#include "critical_section_wrapper.h"
#include "memory_pool.h"
#include "include/audiotypedef.h"
#include "audioBufferBase/include/AudioBufferManager.h"
#include "MultipleSourceSync.h"
#include <list>
#include "DummyPlayer.h"
#include "AudioEngineImpl.h"
#include "AudioDeviceTransport.h"

#define READ_THREAD_NUM 3

class DummyCapturer
{
	typedef webrtc::ThreadWrapper ThreadWrapper;
	typedef webrtc::EventWrapper EventWrapper;
	typedef webrtc::CriticalSectionWrapper CriticalSectionWrapper;
	typedef webrtc::scoped_ptr<CriticalSectionWrapper> CriticalSectionWrapperPtr;
	typedef webrtc::scoped_ptr<ThreadWrapper> ThreadWrapperPtr; 
	typedef webrtc::scoped_ptr<EventWrapper> EventWrapperPtr;

public:
	DummyCapturer(
		AudioDeviceTransport *const a_pAudioDeviceTransport,
		MultipleSourceSync* a_MultipleSourceSync,
		IAudioEngine* a_AudioEngine,
		int a_nFileIdx,
		int a_nFrameTime, 
		const char* a_pCaptureThreadName,
		const char* a_pFileName,
		ChannelId_e a_eChannelId,
		int a_nThreadIdx);

	virtual ~DummyCapturer();

public:

	static bool CapturerThreadFunction(ThreadObj threadObj);

	void StopCapture(){ m_bIsStopCapture = true;};

	bool Capture();

	void StartCapture(){ m_startTime = clock(); m_bIsStartCapture = true; };

	int getFs(){return m_nFs;}

	SWavFileHead getReadWavHead(){return m_Readwavhead;}

private:
	EventWrapperPtr m_Timer;
	ThreadWrapperPtr m_CaptureThread;
	CriticalSectionWrapperPtr m_ReadFileCS;

	MultipleSourceSync* m_pMultipleSourceSync;
	IAudioEngine* m_pAudioEngine;
	int m_nFrameTime;
	int m_nFs;
	int m_nMaxMultipleOfMinFrameSize;
	int m_nFileIdx;

	bool m_bIsStartCapture;
	bool m_bIsStopCapture;

	short* m_data_in_s;
	float* m_data_in_f;
	float* m_data_in_f2;
	SWavFileHead m_Readwavhead;
	CWavFileOp* m_pReadFile;
	int m_nFileLen;
	int m_nFileLen_f;
	int m_nOutLen;
	int m_nReadFilePtr;
	ChannelId_e m_eChannelId;
	int m_nThreadIdx;
	AudioDeviceTransport *const m_pAudioDeviceTransport;
	clock_t m_currentTime;
	clock_t m_startTime;
};
#endif



