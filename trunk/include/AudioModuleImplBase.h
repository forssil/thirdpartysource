/*! \file   AudioModuleImplBase.h
*   \author Zhong Yaozhu
*   \date   2014/12/1
*   \brief  Audio Implement Base Class
*/

#ifndef AUDIO_MODULE_IMPLE_BASE_H_
#define AUDIO_MODULE_IMPLE_BASE_H_

#include "audiotypedef.h"


class IAudioModuleImpleBase
{
protected:
	IAudioModuleImpleBase(){};
	virtual ~IAudioModuleImpleBase(){};

public:
	virtual bool processData(const AudioFrame** a_AudioFrameArray, CAUDIO_U8_t a_nArrayLen) = 0;
#if CAPTURE_PLAYBACK_INTEGRATED
	virtual bool processData(AudioFrame a_AudioFrameArray[], CAUDIO_U8_t a_nArrayLen) { return true; };
#endif
};


class IAudioProcessImplBase
{
protected:
	IAudioProcessImplBase(){};
	virtual ~IAudioProcessImplBase(){};

public:
	virtual int process(audio_pro_share& aShareData) = 0;
};
#endif //AUDIO_MODULE_IMPLE_BASE_H_