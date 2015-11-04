/***********************************************************************
*  Author
;*      Zhang Huabing
;*
;*
;*
;*  History
;*      2015/9/15 Created
;*
;*
;*************************************************************************/

#ifndef AUDIO_DEVICE_TRANSPORT_UNITEST_H
#define AUDIO_DEVICE_TRANSPORT_UNITEST_H

#include <stdio.h>
#include <stdlib.h>
#include "audio_device.h"
#include "WaveIO.h"

#ifdef DEBUG
# define DEBUG_PRINT(x) printf(x)
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

class AudioDeviceTransport : public webrtc::AudioTransport
{
public:
	bool m_bStopAudioPlayRecordFlag;

public:
	AudioDeviceTransport(webrtc::AudioDeviceModule* audio);
	~AudioDeviceTransport();

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

	//open the wavfile, initialise to open a "wavfile.wav"
	//and write a "wavfile-out.wav"for recording the audio data
	int OpenWavFile(char* filename);
	int UpdataWavHeader();

private:
	CWavFileOp *writefile;
	CWavFileOp *readfile;
	SWavFileHead writewavhead;
	SWavFileHead readwavhead;

	int totalLen;

};

#endif