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

#include "AudioDeviceEnumeration.h"

CAudioDeviceEnumeration::CAudioDeviceEnumeration()
	:
	m_pAudioDevice(NULL),
	m_pAudioDeviceSettingFile(NULL),
	m_pAudioDeviceEnumerationFile(NULL)
{
}

CAudioDeviceEnumeration::~CAudioDeviceEnumeration()
{
}

void CAudioDeviceEnumeration::AudioDeviceEnumerationInit()
{
	m_pAudioDevice = this;
}


int CAudioDeviceEnumeration::AudioDeviceEnumerationToFile()
{
	int num;
	string devicename;

	m_pAudioDeviceEnumerationFile = fopen("E:\\AudioDeviceEnumeration.txt","w");
	if (NULL == m_pAudioDeviceEnumerationFile)
		return -1;

	// Recording device enumeration
	fprintf(m_pAudioDeviceEnumerationFile, "InputDevcie:\n");
	num = m_pAudioDevice->RecordingDevices();
	if (num == 0)
	{
		fprintf(m_pAudioDeviceEnumerationFile, "No input device!\n");
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			char name[webrtc::kAdmMaxDeviceNameSize];
			char guid[webrtc::kAdmMaxGuidSize];
			int ret = m_pAudioDevice->RecordingDeviceName(i, name, guid);
			if (ret != -1)
			{
				devicename = __UTF_82ASCII((string)name);
				fprintf(m_pAudioDeviceEnumerationFile, "%s\n", devicename.c_str());
			}
		}
	}

	// Playout device enumeration
	fprintf(m_pAudioDeviceEnumerationFile, "\nOutputDevcie:\n");
	num = m_pAudioDevice->PlayoutDevices();
	if (num == 0)
	{
		fprintf(m_pAudioDeviceEnumerationFile, "No output device!\n");
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			char name[webrtc::kAdmMaxDeviceNameSize];
			char guid[webrtc::kAdmMaxGuidSize];
			int ret = m_pAudioDevice->PlayoutDeviceName(i, name, guid);
			if (ret != -1)
			{
				devicename = __UTF_82ASCII((string)name);
				fprintf(m_pAudioDeviceEnumerationFile, "%s\n", devicename.c_str());
			}
		}
	}

	fclose(m_pAudioDeviceEnumerationFile);
	return 0;
}

//set the device according to ini file
int CAudioDeviceEnumeration::GetAudioDeviceIdx(string DeviceName, AudioDeviceId_e DeviceId)
{
	int ret;
	int num;
	int DeviceIndex = -1;
	string devicename;

	if (DeviceId == ZXKTinput || DeviceId == BBTinput)
	{
		// Recording device enumeration
		num = m_pAudioDevice->RecordingDevices();
		if (num == 0)
		{
			return -1;
		}

		//eumerate and compare to the ini file's set value 
		for (int i = 0; i < num; i++)
		{
			char name[webrtc::kAdmMaxDeviceNameSize];
			char guid[webrtc::kAdmMaxGuidSize];
			int ret = m_pAudioDevice->RecordingDeviceName(i, name, guid);
			if (ret != -1)
			{
				devicename = __UTF_82ASCII((string)name);
				if (!DeviceName.compare(devicename))
				{
					DeviceIndex = i;
				}
			}
		}
	}
	else if (DeviceId == ZXKToutput)
	{
		// Playout device enumeration
		num = m_pAudioDevice->PlayoutDevices();
		if (num == 0)
		{
			return -1;
		}

		//eumerate and compare to the ini file's set value 
		for (int i = 0; i < num; i++)
		{
			char name[webrtc::kAdmMaxDeviceNameSize];
			char guid[webrtc::kAdmMaxGuidSize];
			int ret = m_pAudioDevice->PlayoutDeviceName(i, name, guid);
			if (ret != -1)
			{
				devicename = __UTF_82ASCII((string)name);
				if (!DeviceName.compare(devicename))
				{
					DeviceIndex = i;
				}
			}
		}
	}
	else
	{
		return -1;
	}

	return 	DeviceIndex;
}

int CAudioDeviceEnumeration::GetDeviceNameFromTxtfile(string &DeviceNameZXKTin, string &DeviceNameZXKTout, string &DeviceNameBBTin)
{
	int lineNumber = 0;
	m_pAudioDeviceSettingFile = fopen("E:\\AudioDeviceSetting.txt", "r");

	if (NULL == m_pAudioDeviceSettingFile)
	{
		return -1;
	}
	else
	{
		char line[256]; /* or other suitable maximum line size */
		while (fgets(line, sizeof(line), m_pAudioDeviceSettingFile) != NULL) /* read a line */
		{
			strtok(line, "\n");
			switch (lineNumber)
			{
			case 0:
				//ZXKT IN
				DeviceNameZXKTin = (string)line;
				break;
			case 1:
				//zxkt out
				DeviceNameZXKTout = (string)line;
				break;
			case 3:
				//bbt in
				DeviceNameBBTin = (string)line;
				break;
			default:
				break;
			}
			//printf("%s\n", line);
			lineNumber++;
		}

		fclose(m_pAudioDeviceSettingFile);
		return 0;
	}		
}

//utf8 to Unicode  
wstring CAudioDeviceEnumeration::__Utf82Unicode(const string& utf8string)
{
	int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION)
	{
		throw exception("Invalid UTF-8 sequence.");
	}
	if (widesize == 0)
	{
		throw exception("Error in conversion.");
	}

	vector<wchar_t> resultstring(widesize);

	int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);

	if (convresult != widesize)
	{
		throw exception("Error!");
	}

	return wstring(&resultstring[0]);
}

//unicode to ascii  
string CAudioDeviceEnumeration::__WideByte2Acsi(wstring& wstrcode)
{
	int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
	if (asciisize == ERROR_NO_UNICODE_TRANSLATION)
	{
		throw exception("Invalid UTF-8 sequence.");
	}
	if (asciisize == 0)
	{
		throw exception("Error in conversion.");
	}
	vector<char> resultstring(asciisize);
	int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);

	if (convresult != asciisize)
	{
		throw exception("Error.");
	}

	return string(&resultstring[0]);
}

//utf8 to ascii  
string CAudioDeviceEnumeration::__UTF_82ASCII(string& strUtf8Code)
{
	string strRet("");
	//convert utf8 to unicode first 
	wstring wstr = __Utf82Unicode(strUtf8Code);
	//then convert unicode to ascii  
	strRet = __WideByte2Acsi(wstr);
	return strRet;
}