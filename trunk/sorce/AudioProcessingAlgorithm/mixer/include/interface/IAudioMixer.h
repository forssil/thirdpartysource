/* ---------------------------------------------------------------- *
* Author         :     Zhong Yaozhu
* History        :     11/26/2014 Created
* Content        :     Interface of AudioMixer
*
*                Copyright 2014 Codyy Inc.
* ---------------------------------------------------------------- */

#ifndef _AUDIOMIXER_IAUDIOMIXER_
#define _AUDIOMIXER_IAUDIOMIXER_

#include "AudioMixerDefs.h"

class IAudioMixer
{
public:

	/* ----------------------------------------------------------------------- *
	* Function Name	    :	Reset
	* Description		:	reset AudioMixer instance to initial state
	* Return			:	return true if reset success, or else return false
	* ------------------------------------------------------------------------ */
	virtual bool Reset()=0;

#if 0
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
#endif

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
	virtual bool Process(
		AUDIO_DATA_TYPE* apOutBuffer, 
		CAUDIO_S32_t aOutBufferLen, 
		const AUDIO_DATA_TYPE** appInBuffer, 
		CAUDIO_S32_t aInDataLen) = 0;

protected:
	IAudioMixer(){};
	virtual ~IAudioMixer() {};
};

#ifdef __cplusplus
extern "C"
{
#endif
	/* ------------------------------------------------------------------------- *
	* Function Name	    :	CreateIAudioMixerInst
	* Description		:	use this API to create an instance of AudioMixer
	* Input			    :	aChannelNum		     - number of channel that are used to
												   act mixing, should not larger than 8
							aFs				     - sample rate
							aChannelInfo         - array of AudioChannelInfo_t which
												   contains gain,delay,etc. information
												   for every input channel and output channel
	* Output			:	none
	* Return			:	return the interface of AudioMixer
	* -------------------------------------------------------------------------- */
	IAudioMixer* CreateIAudioMixerInst(
		CAUDIO_S32_t aInChannelNum,
		CAUDIO_S32_t aFs,
		AUDIO_DATA_TYPE aTimePerFrame,
		AUDIO_MIX_INFO **aChannelInfo);

	/* ------------------------------------------------------------------------- *
	* Function Name	    :	DeleteIAudioMixerInst
	* Description		:	use this API to delete an instance of AudioMixer
	* Input			    :	IAudioMixerInst -  interface of AudioMixer
	* Output			:	none
	* Return			:	return true if delete action success
	* -------------------------------------------------------------------------- */
	bool DeleteIAudioMixerInst (IAudioMixer*  IAudioMixerInst);	
#ifdef __cplusplus
};
#endif

#endif //_AUDIOMIXER_IAUDIOMIXER_ 