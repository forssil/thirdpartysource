/*  @file    AudioDeviceManager.h
*   @author  Keil
*   @brief   Manage play out and record device, so we can capture audio stream from hardware device 
             and transmit audio stream to hardware device to play out.
*   @history 2015/9/23 Created.
*/

#ifndef _AUDIO_DEVICE_MANAGER_H_
#define _AUDIO_DEVICE_MANAGER_H_

#include "IAudioDeviceManager.h"
#include "audio_device.h"


class AudioDeviceTransport;

class AudioDeviceManager 
	: public IAudioDeviceManager
{
public:
	AudioDeviceManager(
		const IAudioEngine *pAudioEngine,
		const DATA_TYPE eDataType,
		const std::string folder_name);

	virtual ~AudioDeviceManager();

	virtual bool Init();

	virtual bool Release();

	virtual bool Start();

	virtual bool Stop();

	virtual bool GetSampleRate(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		CAUDIO_U32_t &nSampleRate) const;

	virtual bool SetSampleRate(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		const CAUDIO_U32_t nPresetSampleRate);

	virtual bool GetVolume(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		CAUDIO_U32_t &nVolume) const;

	virtual bool SetVolume(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		const CAUDIO_U32_t nPresetVolume);

private:
	AudioDeviceManager(const AudioDeviceManager &other);
	AudioDeviceManager& operator = (const AudioDeviceManager &other);

private:
	const IAudioEngine *m_pAudioEngine;
	webrtc::AudioDeviceModule *m_pAudioDeviceModule;
	AudioDeviceTransport *m_pAudioDeviceTransport;
	const DATA_TYPE m_eDataType;
	const std::string m_sFolderName;
	bool m_bInitSuccess;
	bool m_bStartSuccess;

};
#endif //_AUDIO_DEVICE_MANAGER_H_
