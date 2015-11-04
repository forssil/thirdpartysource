/*  @file    AudioDeviceTransport.h
*   @author  Keil
*	@brief   AudioDeviceTransport is used as a sink between AudioDevice and AudioChannel.
*	@history 2015/9/8  Realize RecordedDataIsAvailable function.
*	         2015/9/15 Realize NeedMorePlayData function.
*/

#ifndef _AUDIO_DEVICE_TRANSPORT_H_
#define _AUDIO_DEVICE_TRANSPORT_H_

#include <map>
#include "audio_device_defines.h"
#include "scoped_ptr.h"
#include "audiotrace.h"
#include "IAudioEngine.h"
#include "IAudioTransport.h"
#include "WaveIO.h"

//open up a piece of memory, (48KHz*stereo*10s)sample.
static const CAUDIO_U32_t kPlaybackBufferSamples = 48000 * 2 * 10;

class AudioDeviceTransport 
	: public webrtc::AudioTransport
{
public:
	AudioDeviceTransport(
		const IAudioEngine *pAudioEngine,
		const DATA_TYPE eDataType);

	virtual ~AudioDeviceTransport();

	bool Init();

	inline void Reset();

	inline void Release();

	virtual int32_t RecordedDataIsAvailable(
		const void* audioSamples,
		const uint32_t nSamples,
		const uint8_t nBytesPerSample,
		const uint8_t nChannels,
		const uint32_t samplesPerSec,
		const uint32_t totalDelayMS,
		const int32_t clockDrift,
		const uint32_t currentMicLevel,
		const bool keyPressed,
		uint32_t& newMicLevel);

	virtual int32_t NeedMorePlayData(
		const uint32_t nSamples,
		const uint8_t nBytesPerSample,
		const uint8_t nChannels,
		const uint32_t samplesPerSec,
		void* audioSamples,
		uint32_t& nSamplesOut);

	bool InitWaveFile(
		const std::string sFolderName,
		const CAUDIO_U32_t nSampleRate,
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType);

	bool ReleaseWaveFile(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType);

	bool WriteWaveFile(
		const CAUDIO_S16_t *pData,
		const CAUDIO_U32_t nSize,
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType);

private:
	bool TypeCast_Record(const CAUDIO_S16_t *pData, const CAUDIO_U32_t nSize, const CAUDIO_U32_t nChannels);
	bool AllocRecordBuffer_Record(const CAUDIO_U32_t nSize);
	bool TypeCast_PlayOut(const AUDIO_DATA_TYPE *pData, const CAUDIO_U32_t nSize, const CAUDIO_U32_t nChannels);


	AudioDeviceTransport(const AudioDeviceTransport& other);
	AudioDeviceTransport& operator =(const AudioDeviceTransport& other);


private:
	IAudioEngine         *m_pAudioEngine;
	const DATA_TYPE       m_eDataType;
	CAUDIO_S16_t         *m_pPlaybackBuffer;
	CAUDIO_U32_t          m_nPlaybackBufferWritePos;
	CAUDIO_U32_t          m_nPlaybackBufferReadPos;

	AUDIO_DATA_TYPE      *m_pRecordBuffer;
	CAUDIO_U32_t          m_nRecordBufferSize;

	CWavFileOp   *m_pRecordWavFileOp;
	SWavFileHead *m_pRecordWavFileHead;
	CAUDIO_U32_t  m_nRecordWaveCounter;
	CWavFileOp   *m_pPlayoutWavFileOp;
	SWavFileHead *m_pPlayoutWavFileHead;
	CAUDIO_U32_t  m_nPlayoutWaveCounter;
};

#endif _AUDIO_DEVICE_TRANSPORT_H_

