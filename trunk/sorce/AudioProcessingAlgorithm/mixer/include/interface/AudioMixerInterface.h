/* ---------------------------------------------------------------- *
* Author         :     Zhong Yaozhu
* History        :     11/26/2014 Created
* Content        :     Interface of AudioMixer
*
*                Copyright 2014 Codyy Inc.
* ---------------------------------------------------------------- */

#ifndef _AUDIO_MIXER_INTERFACE_
#define _AUDIO_MIXER_INTERFACE_

#include "AudioMixerDefs.h"

class AudioMixerInterface
{
public:

	/* ----------------------------------------------------------------------- *
	* Function Name	    :	Reset
	* Description		:	reset AudioMixer instance to initial state
	* Return			:	return true if reset success, or else return false
	* ------------------------------------------------------------------------ */
	virtual bool Reset()=0;

	/* ------------------------------------------------------------------------ *
	* Function Name	    :	GetAudioChannelInfo
	* Description		:	access gain and delay of channel which 
							is indicated by aChannelIndex
	* Input			    :	aChannelIndex - index of channel, begin from 0 and 
							maximum of aChannelIndex is 7
	* ------------------------------------------------------------------------- */
	virtual bool GetAudioChannelInfo(CAUDIO_S32_t aChannelIndex, AUDIO_MIX_PROPERTY& aAudioPropertyPage)=0;

	/* ------------------------------------------------------------------------ *
	* Function Name	    :	SetAudioChannelInfo
	* Description		:	access gain and delay of channel which 
							is indicated by aChannelIndex
	* Input			    :	aChannelIndex - index of channel, begin from 0 and 
							maximum of aChannelIndex is 7
							aChannelInfo  - channel information
	* ------------------------------------------------------------------------- */
	virtual bool SetAudioChannelInfo(CAUDIO_S32_t aChannelIndex, AUDIO_MIX_PROPERTY& aChannelInfo)=0;


	/* ------------------------------------------------------------------------- *
	* Function Name	    :	Process
	* Description		:	use this API to act audio mixing
	* Input			    :	aOutBuffer - buffer to store mixed data, memory
							allocated outside
							aOutBufferLen - len of out buffer,unit:sample
							appInBuffer - buffers to store source data which is 
							going to be mixed
							aInDataLen - len of source data for every channel,
							unit:sample
	* Output			:	aOutBuffer - buffer where to put mixed data
	* Return			:	return true if mixing action success
	* -------------------------------------------------------------------------- */
	virtual bool Process(AUDIO_DATA_TYPE* apOutBuffer, CAUDIO_S32_t aOutBufferLen, const AUDIO_DATA_TYPE** appInBuffer, CAUDIO_S32_t aInDataLen) = 0;

protected:
	AudioMixerInterface(){};
	virtual ~AudioMixerInterface() {};
};

#ifdef __cplusplus
extern "C"
{
#endif
	/* ------------------------------------------------------------------------- *
	* Function Name	    :	CreateIAudioMixerInst
	* Description		:	use this API to create an instance of AudioMixer
	* Input			    :	aInAudioChannelInfo  - array of AudioChannelInfo_t which
												   contains gain,delay,etc. information 
												   for every input channel
							aOutAudioChannelInfo - array of AudioChannelInfo_t which
												   contains gain,delay,etc. information 
												   for output channel
							aChannelNum		     - number of channel that are used to
												   act mixing, should not larger than 8
							aFs				     - sample rate
	* Output			:	none
	* Return			:	return the interface of AudioMixer
	* -------------------------------------------------------------------------- */
	AudioMixerInterface* CreateIAudioMixerInst(
		const AUDIO_CHANNEL_INFO aInAudioChannelInfo[],
		const AUDIO_CHANNEL_INFO& aOutAudioChannelInfo,
		CAUDIO_S32_t aInChannelNum,
		CAUDIO_S32_t aFs);

	/* ------------------------------------------------------------------------- *
	* Function Name	    :	DeleteIAudioMixerInst
	* Description		:	use this API to delete an instance of AudioMixer
	* Input			    :	IAudioMixerInst -  interface of AudioMixer
	* Output			:	none
	* Return			:	return true if delete action success
	* -------------------------------------------------------------------------- */
	bool DeleteIAudioMixerInst (AudioMixerInterface*  IAudioMixerInst);	
#ifdef __cplusplus
};
#endif

#endif //_AUDIO_MIXER_INTERFACE_ 