/*! \file   AudioMixImpl.h
*   \author Keil
*   \date   2015/3/3
*   \brief  Audio Mix Implement class
*/

#ifndef _AUDIOPROCESS_AUDIOMIXIMPL_H_
#define _AUDIOPROCESS_AUDIOMIXIMPL_H_

#include "AudioModuleImplBase.h"  // include IAudioProcessImplBase
#include "IAudioMixer.h"          // include IAudioMixer

#define  LOG_MIX_IMPL  "[AudioMixImpl.h]" 

class CAudioMixImpl : public IAudioProcessImplBase
{
public:
	//! Constructor
	CAudioMixImpl(CAUDIO_U8_t  _thread_num_, CAUDIO_U32_t _fs_, AUDIO_DATA_TYPE _time_per_frame_);
	CAudioMixImpl(CAUDIO_U8_t  _thread_num_, CAUDIO_U32_t _fs_, AUDIO_DATA_TYPE _time_per_frame_, AUDIO_PROPERTY_PAGE *_property_page_);

	//! Destructor
	~CAudioMixImpl();

public:
	//! IAudioProcessImplBase API
	virtual int process(IN audio_pro_share &aShareData);

#if 0
	//! set audio channel mix property
	bool SetAudioChannelInfo(CAUDIO_U32_t _channel_index_, AUDIO_MIX_PROPERTY &_audio_mix_property_);
	//! get audio channel mix property
	bool GetAudioChannelInfo(CAUDIO_U32_t _channel_index_, AUDIO_MIX_PROPERTY &_audio_mix_property_);
#endif

	//! reset function
	bool Reset(void);

private:
	//! initial function
	bool __Init(void);
	bool __Init(AUDIO_MIX_INFO **_channel_info_);

	//! Analytical structure
	inline bool __AnalyticalStructure(IN const audio_pro_share &aShareData);

private:
	/*---external para---*/
	CAUDIO_U8_t  m_nThreadNum; 
	CAUDIO_U32_t m_nFs; 
	AUDIO_DATA_TYPE m_fTimePerFrame; 

	/*---internal para---*/
	IAudioMixer *m_iAudioMixer; 
	AUDIO_DATA_TYPE **m_ppInputBuf; 
	AUDIO_DATA_TYPE *m_pOutputBuf; 
	AUDIO_MIX_INFO **m_pChannelInfo;
	CAUDIO_U32_t m_nInputBufLen; 
	CAUDIO_U32_t m_nOutputBufLen; 
	bool m_bIsInitSuccess;

}; 
#endif //_AUDIOPROCESS_AUDIOMIXIMPL_H_

