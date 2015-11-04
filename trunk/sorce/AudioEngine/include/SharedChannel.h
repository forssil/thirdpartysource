/*! \file     SharedChannel.h
*   \author   Keil
*   \brief    used to mix audio data from class-to-class and electronic piano , then transmission mixed data to capture channel.
*   \history  2015/3/9 created shared channel class.
*/

#ifndef _SHARED_CHANNEL_H_
#define _SHARED_CHANNEL_H_

#include "scoped_ptr.h"
#include "AudioChannelBase.h"
#include "AudioProcessingImpl.h"
#include "MultipleSourceSync.h"
#include "AudioTransportImpl.h"

class AudioEngineImpl;
typedef webrtc::scoped_ptr<CAudioProcessingImpl> CAudioProcessingImplPtr;
typedef webrtc::scoped_ptr<MultipleSourceSync> MultipleSourceSyncPtr;
typedef webrtc::scoped_ptr<AudioTransportImpl> AudioTransportImplPtr;

class CSharedChannel
	: public AudioChannelBase
{
public:
	CSharedChannel(
		//AudioEngineImpl *_audio_engine_impl_,		
		CAUDIO_U8_t _thread_num_,
		CAUDIO_U8_t _channel_num_per_thread_,
		AUDIO_DATA_TYPE _frame_time_ms_,
		CAUDIO_U32_t _fs_,
		DATA_TYPE _input_data_type_);
	~CSharedChannel();

public:
	virtual bool Init();
	virtual bool Reset();
	virtual ModuleState Start();
	virtual ModuleState Stop();
	virtual bool FeedData(const void* a_pData, CAUDIO_U32_t a_nDataSize, CAUDIO_U8_t a_nThreadIdx, CAUDIO_U32_t a_nOriginalSampleRate = 0);
	virtual bool Transport(const void* a_pData, DATA_TYPE a_eDataType, CAUDIO_U32_t a_nSize, TransportId_e a_eTPId);
	virtual bool processData(const AudioFrame **a_AudioFrameArray, CAUDIO_U8_t a_nArrayLen);

	bool RegisterAudioTransport(
		IAudioTransport* a_pAudioTransport, 
		TransportId_e a_eTPId, 
		CAUDIO_U8_t a_nAssignedThreadIdx	// the transport registered may received data from multiple threads
		);

	bool DeregisterAudioTransport(TransportId_e a_eTPId);

private:
	//AudioEngineImpl *m_cAudioEngineImpl;	
	CAUDIO_U8_t m_nNumOfThread;
	CAUDIO_U8_t m_nChannelNumPerThread;
	AUDIO_DATA_TYPE m_fFrameTime;
	CAUDIO_U32_t m_nDefaultSampleRate;
	DATA_TYPE m_eInputDataType;

	CAudioProcessingImplPtr m_cAudioProcessingImplPtr;
	MultipleSourceSyncPtr   m_cMultipleSourceSyncPtr;
	//AudioTransportImplPtr   m_cAudioTransportImplPtr;
	IAudioTransport *m_pTransportToCaptureCh;
	CAUDIO_U8_t m_nAssignedIdxToCaptureCh;
	IAudioTransport *m_pTransportToPlaybackCh;
	CAUDIO_U8_t m_nAssignedIdxToPlaybackCh;
};
#endif //_SHARED_CHANNEL_H_

