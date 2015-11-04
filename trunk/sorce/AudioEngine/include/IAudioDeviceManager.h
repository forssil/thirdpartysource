/*  @file    IAudioDeviceManager.h
*   @author  Keil
*   @brief   Interface head file of AudioDeviceManager class .
*   @history 2015/9/23 Created.
*/

#ifndef _I_AUDIO_DEVICE_MANAGER_H_
#define _I_AUDIO_DEVICE_MANAGER_H_

#include "audiotypedef.h"

class IAudioEngine;

class IAudioDeviceManager
{
public:
	static IAudioDeviceManager* CreateAudioDeviceManager(
		const IAudioEngine *pAudioEngine,
		const DATA_TYPE eDataType,
		const std::string folder_name);

	static bool DeleteAudioDeviceManager(IAudioDeviceManager * pAudioDeviceManager);

	virtual bool Init() = 0;

	virtual bool Release() = 0;

	virtual bool Start() = 0;

	virtual bool Stop() = 0;

	virtual bool GetSampleRate(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		CAUDIO_U32_t &nSampleRate) const = 0;

	virtual bool SetSampleRate(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		const CAUDIO_U32_t nPresetSampleRate) = 0;

	virtual bool GetVolume(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		CAUDIO_U32_t &nVolume) const = 0;

	virtual bool SetVolume(
		const CAUDIO_U8_t nAudioDeviceId,
		const HardwareDeviceType eHardwareDeviceType,
		const CAUDIO_U32_t nPresetVolume) = 0;

protected:
	virtual ~IAudioDeviceManager(){};

};

#endif //_I_AUDIO_DEVICE_MANAGER_H_

