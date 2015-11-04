/***********************************************************************
*  Author
;*      Zhang Huabing
;*
;*
;*
;*  History
;*      2015/10/8 Created
;*
;*
;*************************************************************************/

#ifndef AUDIO_DEVICE_ENUMERATION_H
#define AUDIO_DEVICE_ENUMERATION_H

#include <windows.h>
#include <iostream>
#include <vector>
#include "typedefs.h"
#include "audio_device_defines.h"

using namespace std;
using namespace webrtc;

typedef enum AudioDeviceId
{
	ZXKTinput,
	ZXKToutput,
	BBTinput
}AudioDeviceId_e;

class CAudioDeviceEnumeration
{
public:
	CAudioDeviceEnumeration();
	~CAudioDeviceEnumeration();

	void AudioDeviceEnumerationInit();
	int GetAudioDeviceIdx(string DeviceName, AudioDeviceId_e DeviceId);
	int AudioDeviceEnumerationToFile();
	int GetDeviceNameFromTxtfile(string &DeviceNameZXKTin, string &DeviceNameZXKTout, string &DeviceNameBBTin);

	// Device enumeration
	virtual int16_t PlayoutDevices() = 0;
	virtual int16_t RecordingDevices() = 0;
	virtual int32_t PlayoutDeviceName(uint16_t index,
		char name[kAdmMaxDeviceNameSize],
		char guid[kAdmMaxGuidSize]) = 0;
	virtual int32_t RecordingDeviceName(uint16_t index,
		char name[kAdmMaxDeviceNameSize],
		char guid[kAdmMaxGuidSize]) = 0;

private:
	// UTF-8 to ASCII
	wstring __Utf82Unicode(const string& utf8string);
	string __WideByte2Acsi(wstring& wstrcode);
	string __UTF_82ASCII(string& strUtf8Code);

private:
	CAudioDeviceEnumeration *m_pAudioDevice;
	FILE* m_pAudioDeviceSettingFile;
	FILE* m_pAudioDeviceEnumerationFile;

};

#endif