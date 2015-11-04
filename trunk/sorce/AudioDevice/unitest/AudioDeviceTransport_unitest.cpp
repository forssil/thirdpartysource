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
#include "AudioDeviceTransport_unitest.h"

AudioDeviceTransport::AudioDeviceTransport(webrtc::AudioDeviceModule* audio)
	:
	m_bStopAudioPlayRecordFlag(false),
	totalLen(0)
{
}

AudioDeviceTransport::~AudioDeviceTransport()
{
}

// callback function: using for transport mic data to audio engine
int32_t AudioDeviceTransport::RecordedDataIsAvailable(
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

	int framelen = nSamples;
	short *data_out_s;
	int ret;

	if (m_bStopAudioPlayRecordFlag == false)
	{
		//write the recording buffer to the file
		data_out_s = (int16_t *)audioSamples;
		ret = writefile->WriteSample(data_out_s, (framelen*writewavhead.NChannels));
		totalLen = totalLen + framelen;
	}

	return 0;
}

//callback funtion: transport playback data from audio engine
int32_t AudioDeviceTransport::NeedMorePlayData(
		const uint32_t nSamples,
		const uint8_t nBytesPerSample,
		const uint8_t nChannels,
		const uint32_t samplesPerSec,
		void* audioSamples,
		uint32_t& nSamplesOut)
{
	// TODO: multithread lock
	/*
	log_debug("playout %d %d %d %d", nSamples,
	nBytesPerSample, nChannels,
	samplesPerSec);
	*/

	//wavfile read to buffer
	int framelen = nSamples;
	int read_len=0;
	short *data_out_s;

	data_out_s = (int16_t *)audioSamples;

	read_len = readfile->ReadSample((int16_t *)audioSamples, framelen*readwavhead.NChannels);
	//when reach the end of the file, stop play and record  
	if (read_len < framelen*readwavhead.NChannels)
	{
		m_bStopAudioPlayRecordFlag = true;
		//avoid playout niose sound, set 0s to the play buffer
		for (int i = 0; i < framelen*readwavhead.NChannels; i++)
		{
			*data_out_s++ = 0;
		}
	}

	nSamplesOut = nSamples;
	return 0;
}

int AudioDeviceTransport::OpenWavFile(char* filename)
{
	char *inFileName;
	char outFileName[256];

	inFileName = filename;

	int i = 0;
	while ((inFileName[i] != '.') && (i<240))
	{
		outFileName[i] = inFileName[i];
		i++;
	}

	outFileName[i + 0] = '-';
	outFileName[i + 1] = 'o';
	outFileName[i + 2] = 'u';
	outFileName[i + 3] = 't';

	outFileName[i + 4] = '.';
	outFileName[i + 5] = 'w';
	outFileName[i + 6] = 'a';
	outFileName[i + 7] = 'v';
	outFileName[i + 8] = '\0';

	readfile = new CWavFileOp(inFileName, "rb");
	if (readfile->m_FileStatus == -2)
	{
		delete readfile;
		DEBUG_PRINT("open infile failed!\n");
		return 0;
	}

	writefile = new CWavFileOp(outFileName, "wb");
	if (writefile->m_FileStatus == -2)
	{
		delete writefile;
		DEBUG_PRINT("open outfile failed!\n");
		return 0;
	}

	readfile->ReadHeader(&readwavhead);
	writewavhead = readwavhead;
	writefile->WriteHeader(writewavhead);

	return 0;
}

int AudioDeviceTransport::UpdataWavHeader()
{
	int ret; 
	ret = writefile->UpdateHeader(writewavhead.NChannels, totalLen);

	totalLen = 0;

	delete writefile;
	delete readfile;

	return ret;
}


