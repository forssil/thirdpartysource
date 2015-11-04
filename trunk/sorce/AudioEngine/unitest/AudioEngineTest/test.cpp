#include "DummyCapturer.h"
#include "DummyPlayer.h"
#include "IAudioEngine.h"
#include <direct.h>
#include <ctime>
#include <cstdio>
#include <WaveIO.h>

#define MAX_MIC_PIN_NUM      3
#define UNIT_TEST_MODE_LOOP  0

int main()
{
	std::string Dir;
	std::string File;
	char folder_name[256];
	char trace_name[256];
	tm *local;
	time_t t;

	// create directory 
	Dir.clear();
	Dir = "C:\\AudioEngineUnitTest";
	_mkdir(Dir.c_str());

	// create folder
	t = time(NULL);
	local = localtime(&t);
	memset(folder_name, 0, sizeof(char) * 256);
	sprintf_s(folder_name, 256, "%s\\%04d_%02d_%02d_%02d_%02d_%02d",
		Dir.data(), local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	_mkdir(folder_name);

	// add trace
	memset(trace_name, 0, sizeof(char) * 256);
	sprintf_s(trace_name, 256, "%s\\audio_engine_log.txt", folder_name);
	CreateTrace(trace_name);

	SSampleRate sample;
	int frameTime = 10; // 10 ms
	int inChannelNumPerThread = 1;
	DATA_TYPE dataType = REAL_FLOAT_DATA;

	// connect input and output pin
	int networkInPinNum = 1;	// up to 1
	int sharedPin1Num = 0;	// up to 1
	int sharedPin2Num = 0;	// up to 1
	int micPinNum = 1;	// up to MAX_MIC_PIN_NUM
	int networkOutPinNum = 1;	// up to 1
	int renderPinNum = 1;	// up to 1
	int recordingPinNum = 0;	// up to 1

	// fill in struct AUDIO_TYPE_NUM_t according to pin number connected
	AUDIO_TYPE_NUM_t channelInfo;
	memset(&channelInfo, 0, sizeof(AUDIO_TYPE_NUM_t));
	channelInfo.nClassToClass_in_ = sharedPin1Num;
	channelInfo.nElectronicPiano_in_ = sharedPin2Num;
	channelInfo.nMic_in_ = micPinNum;
	channelInfo.nOnLineClass_in_ = networkInPinNum;
	channelInfo.nTotal_in_ = sharedPin1Num + sharedPin2Num + micPinNum + networkInPinNum;
	channelInfo.nOnLineClass_out_ = networkOutPinNum;
	channelInfo.nRecording_out_ = recordingPinNum;
	channelInfo.nSpeaker_out_ = recordingPinNum;
	channelInfo.nTotal_out_ = networkOutPinNum + recordingPinNum + recordingPinNum;

	// create Audio Engine
	IAudioEngine* pAudioEngine = IAudioEngine::Create(channelInfo, inChannelNumPerThread,
		frameTime, sample, dataType);
	if (NULL == pAudioEngine)
	{
		printf("alloc audio engine failed");
		return false;
	}

	// init Audio Engine
	if (!pAudioEngine->Init(true, folder_name))
	{
		return false;
	}

	// alloc dummy player and register to engine as a transport
	DummyPlayer* networkOutPinPlayer = NULL;
	DummyPlayer* renderPinPlayer = NULL;
	DummyPlayer* recordingPinPlayer = NULL;
	if(0 != networkOutPinNum)
	{
		char networkPinName[256];
		memset(networkPinName, 0, sizeof(char) * 256);
		sprintf_s(networkPinName, sizeof(char) * 256, "%s\\networkOut.wav", folder_name);
		networkOutPinPlayer = new DummyPlayer(
			NULL, 
			sample.nSampleRate_OnlineClassOut, 
			frameTime, 
			REAL_FLOAT_DATA, 
			TP2Network, 
			SYNC_PUSH_MODE, 
			networkPinName, 
			NULL);

		// register transport to network
		pAudioEngine->RegisterAudioTransport(networkOutPinPlayer, TP2Network);
	}
	//if(0 != renderPinNum)
	//{
	//	char renderPinName[256];
	//	memset(renderPinName, 0, sizeof(char) * 256);
	//	sprintf_s(renderPinName, sizeof(char) * 256, "%s\\render.wav", folder_name);

	//	renderPinPlayer = new DummyPlayer(
	//	    NULL,
	//		sample.nSampleRate_SpeakerOut,
	//		frameTime,
	//		REAL_FLOAT_DATA,
	//		TP2RenderDev,
	//		SYNC_PUSH_MODE,
	//		renderPinName,
	//		NULL);
	//	// register transport to render device
	//	pAudioEngine->RegisterAudioTransport(renderPinPlayer, TP2RenderDev);
	//}
	//if(0 != recordingPinNum)
	//{
	//	char recordingPinName[256];
	//	memset(recordingPinName, 0, sizeof(char) * 256);
	//	sprintf_s(recordingPinName, sizeof(char) * 256, "%s\\recording.wav", folder_name);
	//	recordingPinPlayer = new DummyPlayer(
	//		NULL,
	//		sample.nSampleRate_RecorderOut,
	//		frameTime,
	//		REAL_FLOAT_DATA,
	//		TP2RecordingDev,
	//		SYNC_PUSH_MODE,
	//		recordingPinName,
	//		NULL);
	//	// register transport to recording device
	//	pAudioEngine->RegisterAudioTransport(recordingPinPlayer, TP2RecordingDev);
	//}

	// alloc dummy capturer
	int shareThreadIdx = 0;
	int networkThreadIdx = 0;
	int micThreadIdx = 0;
	DummyCapturer* networkCapturer = NULL;
	DummyCapturer* sharePin1Capturer = NULL;
	DummyCapturer* sharePin2Capturer = NULL;
	DummyCapturer* micCapturer[MAX_MIC_PIN_NUM];
	memset(micCapturer, 0, sizeof(DummyCapturer*)*MAX_MIC_PIN_NUM);

	//if(0 != sharedPin1Num)
	//{
	//	sharePin1Capturer = new DummyCapturer(
	//		NULL,
	//		NULL,
	//		pAudioEngine,
	//		shareThreadIdx, 
	//		frameTime,
	//		"capture_thread_sharedPin1", 
	//		"test1.wav", 
	//		SharedCh, 
	//		shareThreadIdx);
	//	shareThreadIdx++;
	//}
	if(0 != networkInPinNum)
	{
		networkCapturer = new DummyCapturer(
			NULL,
			NULL,
			pAudioEngine,
			networkThreadIdx,
			frameTime,
			"capture_thread_network",
			"test2.wav",
			RenderCh,
			networkThreadIdx);
		networkThreadIdx++;
	}
	//if (0 != sharedPin2Num)
	//{
	//	sharePin2Capturer = new DummyCapturer(
	//		NULL,
	//		NULL,
	//		pAudioEngine,
	//		shareThreadIdx,
	//		frameTime,
	//		"capture_thread_sharedPin2",
	//		"test3.wav",
	//		SharedCh,
	//		shareThreadIdx);
	//	shareThreadIdx++;
	//}
	//if(0 != micPinNum)
	//{
	//	char threadNameBuffer[100];
	//	char fileNameBuffer[100];
	//	memset(threadNameBuffer, 0, 100);
	//	memset(fileNameBuffer, 0, 100);
	//	for(int i=0; i<micPinNum; ++i)
	//	{
	//		sprintf(threadNameBuffer, "capture_thread_mic_%d", i);
	//		sprintf(fileNameBuffer, "test%d.wav", i + 4);

	//		micCapturer[i] = new DummyCapturer(
	//			NULL,
	//			NULL,
	//			pAudioEngine,
	//			micThreadIdx,
	//			frameTime,
	//			threadNameBuffer,
	//			fileNameBuffer,
	//			CaptureCh,
	//			micThreadIdx);

	//		micThreadIdx++;
	//	}
	//}

	// set wav file header to player
	SWavFileHead wavhead = networkCapturer->getReadWavHead();
	if(NULL != networkOutPinPlayer)
	{
		wavhead.SampleRate = sample.nSampleRate_OnlineClassOut;
		wavhead.SampleBytes = wavhead.BytesPerSample*sample.nSampleRate_OnlineClassOut;
		networkOutPinPlayer->setWaveHead(wavhead);
	}
	//if(NULL != renderPinPlayer)
	//{
	//	wavhead.SampleRate = sample.nSampleRate_SpeakerOut;
	//	wavhead.SampleBytes = wavhead.BytesPerSample*sample.nSampleRate_SpeakerOut;
	//	renderPinPlayer->setWaveHead(wavhead);
	//}
	//if(NULL != recordingPinPlayer)
	//{
	//	wavhead.SampleRate = sample.nSampleRate_RecorderOut;
	//	wavhead.SampleBytes = wavhead.BytesPerSample*sample.nSampleRate_RecorderOut;
	//	recordingPinPlayer->setWaveHead(wavhead);
	//}
	
#if UNIT_TEST_MODE_LOOP
	int i=0;
	while(1)
	{
		i++;
		delete pAudioEngine;
		//printf("delete engine\n");
		pAudioEngine = new AudioEngineImpl(channelInfo, 2, frameTime, fs, REAL_FIX_DATA);
		//printf("new engine\n");
		pAudioEngine->Init();
		//printf("init engine\n");
		pAudioEngine->Start();
		printf("start engine:%d\n", i);

		//capture1.StartCapture();
		//capture2.StartCapture();
		//capture3.StartCapture();

		//getchar();
		//printf("start capture\n");
		pAudioEngine->Stop();
		printf("stop engine\n");
	}
#endif

	// must start sync first
	pAudioEngine->Start();
	//if(NULL != sharePin1Capturer)
	//{
	//	sharePin1Capturer->StartCapture();
	//}
	if(NULL != networkCapturer)
	{
		networkCapturer->StartCapture();
	}
	//if(NULL != sharePin2Capturer)
	//{
	//	sharePin2Capturer->StartCapture();
	//}
	//for(int i=0; i<micPinNum; ++i)
	//{
	//	micCapturer[i]->StartCapture();
	//}


	printf("input any char to stop the engine");
	getchar();


	//if(NULL != sharePin1Capturer)
	//{
	//	sharePin1Capturer->StopCapture();
	//}
	if(NULL != networkCapturer)
	{
		networkCapturer->StopCapture();
	}
	//if(NULL != sharePin2Capturer)
	//{
	//	sharePin2Capturer->StopCapture();
	//}
	//for(int i=0; i<micPinNum; ++i)
	//{
	//	micCapturer[i]->StopCapture();
	//}
	pAudioEngine->Stop();


	printf("wait for 2 seconds.");
	Sleep(2000);


	IAudioEngine::Delete(pAudioEngine);
	// delete
	//if(NULL != sharePin1Capturer)
	//{
	//	delete sharePin1Capturer;
	//	sharePin1Capturer = NULL;
	//}
	if(NULL != networkCapturer)
	{
		delete networkCapturer;
		networkCapturer = NULL;
	}
	//if(NULL != sharePin2Capturer)
	//{
	//	delete sharePin2Capturer;
	//	sharePin2Capturer = NULL;
	//}
	//for (int i = 0; i < micPinNum; ++i)
	//{
	//	delete micCapturer[i];
	//	micCapturer[i] = NULL;
	//}
	if (NULL != networkOutPinPlayer)
	{
		delete networkOutPinPlayer;
		networkOutPinPlayer = NULL;
	}
	//if (NULL != renderPinPlayer)
	//{
	//	delete renderPinPlayer;
	//	renderPinPlayer = NULL;
	//}
	//if (NULL != recordingPinPlayer)
	//{
	//	delete recordingPinPlayer;
	//	recordingPinPlayer = NULL;
	//}
	ReturnTrace();

	return 0;
}

