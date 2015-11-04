/*! \file   AudioProcessorBase.h
*   \author Zhong Yaozhu
*   \date   2014/10/15
*   \brief  audio processor base class
*/

#ifndef _AUDIOBUFFERBASE_AUDIOPROCESSORBASE_H_
#define _AUDIOBUFFERBASE_AUDIOPROCESSORBASE_H_

#include "codyyAudioCommon.h"

class AudioProcessorBase
{
public:
	AudioProcessorBase::AudioProcessorBase(){};
	virtual ~AudioProcessorBase(){};

public:
	virtual bool processData(
		void** appProcessBuffer, 
		CAUDIO_U16_t aProcessBufferLen, 
		CAUDIO_U8_t aInChannelNum,
		CAUDIO_U8_t aOutChannelNum)
	{ 
		return true;
	};
};

// for test
class AudioProcessorTest : public AudioProcessorBase
{
public:
	AudioProcessorTest(){};
	virtual ~AudioProcessorTest(){};

	bool processData(
		void** appProcessBuffer, 
		CAUDIO_U16_t aProcessBufferLen, 
		CAUDIO_U8_t aInChannelNum,
		CAUDIO_U8_t aOutChannelNum)
	{
		return true;
	};
};

#endif //_AUDIOBUFFERBASE_AUDIOPROCESSORBASE_H_