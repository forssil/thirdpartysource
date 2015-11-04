/*! \file   AudioMixer.h
*   \author Zhong Yaozhu
*   \date   2014/11/26
*   \brief  Audio Mixer class
*/

#ifndef _AUDIOMIXER_AUDIOMIXER_
#define _AUDIOMIXER_AUDIOMIXER_

#include "IAudioMixer.h" // include IAudioMixer

class CCompandor;
class CMix;

class AudioMixer: IAudioMixer
{
public:
	//! Constructor
	AudioMixer(
		IN CAUDIO_S32_t aChannelNum, 
		IN CAUDIO_S32_t aProcessFrameLen, 
		IN CAUDIO_S32_t aFs,
		IN AUDIO_MIX_INFO **aChannelInfo);

	//! Destructor
	virtual ~AudioMixer();

public:
	/*---IAudioMixer API---*/
	//! audio mix process function
	virtual bool Process(
		AUDIO_DATA_TYPE* apOutBuffer,
		CAUDIO_S32_t aOutBufferLen,
		const AUDIO_DATA_TYPE** appInBuffer, 
		CAUDIO_S32_t aInDataLen
		);
	//! reset function
	virtual bool Reset();
#if 0
	//! get audio channel mix property
	virtual bool GetAudioChannelInfo(CAUDIO_S32_t aChannelIndex, AUDIO_MIX_PROPERTY& aAudioPropertyPageInfo);
	//! set audio channel mix property
	virtual bool SetAudioChannelInfo(CAUDIO_S32_t aChannelIndex, AUDIO_MIX_PROPERTY& aAudioPropertyPageInfo);
#endif

private:
	//! initial function
	bool __Init();
	//! set compandor parameters
	void __SetCompandorParam(CAUDIO_S32_t aIdx);

private:
	
	CAUDIO_S32_t mInChannelNum; 
	CAUDIO_S32_t mFs; 
	CAUDIO_S32_t mProcessFrameLen;
	const AUDIO_DATA_TYPE** mpInBufPtrs;
	CCompandor** mCompandor; // additional one for out channel
	CMix* mpMix;
	AUDIO_MIX_INFO **mChannelInfo; // additional one for out channel
	bool m_bIsInit;
};

#endif //_AUDIOMIXER_AUDIOMIXER_