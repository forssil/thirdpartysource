/*	
 *	Name:			DummyPlayer.h
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

#ifndef _DUMMY_PLAYER_
#define _DUMMY_PLAYER_

#include <ctime>
#include "WaveIO.h"
#include "audiotypedef.h"
#include "AudioModuleImplBase.h"
#include "IAudioTransport.h"
#include "AudioDeviceTransport.h"
#include "thread_wrapper.h"
#include "event_wrapper.h"

typedef enum tag_DummyPlayerMode
{
	SYNC_PUSH_MODE = 0,
	SYNC_PULL_MODE = 1,
	SYNC_NONE      = 2
}DummyPlayerMode;

static const CAUDIO_U32_t kMaxMultipleOfFrameSize = 8;

class DummyPlayer 
	: public IAudioModuleImpleBase
	, public IAudioTransport
{
	typedef webrtc::ThreadWrapper ThreadWrapper;
	typedef webrtc::EventWrapper EventWrapper;
	typedef webrtc::scoped_ptr<ThreadWrapper> ThreadWrapperPtr;
	typedef webrtc::scoped_ptr<EventWrapper> EventWrapperPtr;

public:
	DummyPlayer(
		AudioDeviceTransport *const a_pAudioDeviceTransport,
		const CAUDIO_U32_t a_nFs,
		const CAUDIO_U32_t a_nFrameTime,
		const DATA_TYPE a_eAudioDataType,
		const TransportId_e a_eTPId, 
		const DummyPlayerMode a_eDummyPlayerMode,
		const char* a_pOutFileName,
		const char* a_pPlayThreadName);

	virtual ~DummyPlayer();

	static bool PlayerThreadFunction(ThreadObj threadObj);
	virtual bool processData(const AudioFrame** a_AudioFrameArray, CAUDIO_U8_t a_nArrayLen);
	virtual bool Transport(const void* a_pData, DATA_TYPE a_eDataType, CAUDIO_U32_t a_nSize, TransportId_e a_eTPId);

	bool StopPlay();
	bool StartPlay();

	void setWaveHead(SWavFileHead& a_WaveHead);
	void setMinFrameSize(int a_nMinSize);
	void setMaxMultipleOfMinFrameSize(int a_nMaxSize);

	DummyPlayerMode GetDummyPlayerMode();
private:
	
	bool Play();

private:
	SWavFileHead m_WaveHead;
	CWavFileOp *m_pWritefile;
	char* m_pOutFileName;
	
	CAUDIO_U32_t m_nFs;
	CAUDIO_U32_t m_nFrameTime;
	CAUDIO_U32_t m_nFrameSize;
	TransportId_e m_eTPId;
	const DATA_TYPE m_dataType;
	const DummyPlayerMode m_eDummyPlayerMode;

	int m_nMinFrameSize;
	int m_nMaxMultipleOfMinFrameSize;
	int m_nWriteLen;
	
	CAUDIO_S16_t *m_pOutputBuffer;
	CAUDIO_U32_t m_nOutputBufferPtr;
	AUDIO_DATA_TYPE *m_pProcessBuffer;
	
	AudioDeviceTransport *const m_pAudioDeviceTransport;
	
	const char* m_pPlayerThreadName;
	ThreadWrapperPtr m_PlayerThread;
	EventWrapperPtr m_Timer;
	bool m_bIsStartPlay;
	bool m_bIsStopPlay;
	CAUDIO_U32_t m_nThreadIdx;
	clock_t m_currentTime;
	clock_t m_startTime;

};

#endif



