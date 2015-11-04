#include <windows.h> //Sleep
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "AudioDeviceTransport_unitest.h"

using namespace std;

int main(int argc, char **argv)
{
	int num;
	int ret;
	webrtc::AudioDeviceModule *audio;
	int DeviceIndex;

	audio = webrtc::CreateAudioDeviceModule(0, webrtc::AudioDeviceModule::kPlatformDefaultAudio);
	ret = audio->AddRef();
	assert(audio);
	ret = audio->Init();

	string DeviceNameZXKTin;
	string DeviceNameZXKTout;
	string DeviceNameBBTin;

	audio->AudioDeviceEnumerationInit();
	ret = audio->AudioDeviceEnumerationToFile();

	ret = audio->GetDeviceNameFromTxtfile(DeviceNameZXKTin, DeviceNameZXKTout, DeviceNameBBTin);
	DeviceIndex = audio->GetAudioDeviceIdx(DeviceNameZXKTin, ZXKTinput);
	ret = audio->SetRecordingDevice(DeviceIndex);

	DeviceIndex = audio->GetAudioDeviceIdx(DeviceNameZXKTout, ZXKToutput);
	ret = audio->SetPlayoutDevice(DeviceIndex);

	DeviceIndex = audio->GetAudioDeviceIdx(DeviceNameBBTin, BBTinput);
	ret = audio->SetRecordingDevice(DeviceIndex);

	// Audio transport initialization
	ret = audio->InitPlayout();
	ret = audio->InitRecording();

	// Native sample rate controls (samples/sec)
	uint32_t rate = 0;
	ret = audio->PlayoutSampleRate(&rate);
	ret = audio->SetPlayoutSampleRate(48000);  //--- not passed
	if (ret == -1){
		uint32_t rate = 0;
		ret = audio->PlayoutSampleRate(&rate);
		printf("use resampler for playout, device samplerate: %u \n", rate);
	}

	bool available = false;
	uint32_t volume = 255;
	uint32_t MaxVolume = 0;
	uint32_t MinVolume = 0;
	uint32_t SetVolume = 255;
	uint16_t stepSize;
	ret = audio->SpeakerVolumeIsAvailable(&available);
	ret = audio->SetSpeakerVolume(SetVolume);  // ---pass

	// Microphone volume controls
	ret = audio->MicrophoneVolumeIsAvailable(&available);
	ret = audio->SetMicrophoneVolume(volume); // ---pass
	ret = audio->MicrophoneVolume(&volume);
	ret = audio->MaxMicrophoneVolume(&MaxVolume);
	ret = audio->MinMicrophoneVolume(&MinVolume);
	ret = audio->MicrophoneVolumeStepSize(&stepSize);

	//============= play 1khz.wav ==============
	AudioDeviceTransport callback(audio);
	callback.OpenWavFile("1KHz.wav");
	ret = audio->RegisterAudioCallback(&callback);
	ret = audio->StartPlayout();
	ret = audio->StartRecording();

	//waiting for stop
	while (callback.m_bStopAudioPlayRecordFlag == false){
		Sleep(20);
	}

	ret = audio->StopPlayout();
	ret = audio->StopRecording();

	ret = audio->Terminate();
	ret = audio->Release();

	return 0;
}
