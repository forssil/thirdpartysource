/*! \file   AudioMixer.h
*   \author Zhong Yaozhu
*   \date   2014/11/26
*   \brief  Audio Mixer class
*/

#include "AudioMixer.h"     
#include "Compandor.h"  // include CCompandor
#include "Mix.h"        // include CMix

/*---factory function---*/
#ifdef __cplusplus
	extern "C"
	{
#endif
	//! use this API to create an instance of AudioMixer
	/*!
		\param   aChannelNum            -   number of channel that are used to act mixing, should not larger than 8
		\param   aFs                    -   sample rate
		\param   aTimePerFrame          -   time for one frame (ms) 
		\param   aChannelInfo           -   array of AudioChannelInfo_t which contains gain,delay,etc. information for every input channel and output channel
		\return  interface pointer
	*/
	IAudioMixer* CreateIAudioMixerInst(
		CAUDIO_S32_t aChannelNum, 
		CAUDIO_S32_t aFs,
		AUDIO_DATA_TYPE aTimePerFrame,
		AUDIO_MIX_INFO **aChannelInfo)
	{
		//using same formula to calculate frame len
		CAUDIO_S32_t processFrameLen = FRAM_LEN(aTimePerFrame, aFs);
		AudioMixer *inst = new AudioMixer(aChannelNum, processFrameLen, aFs, aChannelInfo);
		if (inst)
		{
			return (IAudioMixer*)inst;
		}
		else
		{
			return NULL;
		}		
	};
	
	//! use this API to delete an instance of AudioMixer
	/*!
		\param   IAudioMixerInst   -   interface of AudioMixer
		\return  true if success, others false
	*/
	bool DeleteIAudioMixerInst(IAudioMixer*  IAudioMixerInst)	
	{
		if(IAudioMixerInst)
		{
			delete (AudioMixer*)IAudioMixerInst;
			return true;
		}
		else
		{
			return false;
		}
	};
#ifdef __cplusplus
	};
#endif

//! Constructor
/*!
	\param   aInChannelNum          -   number of input channels
	\param   aProcessFrameLen       -   length of process frame buffer
	\param   aFs                    -   samples per frame
	\param   aChannelInfo           -   input and output audio channel info
	\return  none
*/
AudioMixer::AudioMixer(
	IN CAUDIO_S32_t aChannelNum,
	IN CAUDIO_S32_t aProcessFrameLen,
	IN CAUDIO_S32_t aFs,
	IN AUDIO_MIX_INFO **aChannelInfo)

	: mInChannelNum(aChannelNum)
	, mFs(aFs)
	, mProcessFrameLen(aProcessFrameLen)
	, mpInBufPtrs(NULL)
	, mpMix(NULL)
	, mCompandor(NULL)
	, mChannelInfo(aChannelInfo)
	, m_bIsInit(false)
{
	__Init();
}

//! Destructor
/*!
	\param   none
	\return  none
*/
AudioMixer::~AudioMixer()
{
	if (mCompandor)
	{
		for (CAUDIO_S32_t i = 0; i < mInChannelNum + 1; ++i)
		{
			if (NULL != mCompandor[i])
			{
				delete mCompandor[i];
				mCompandor[i] = NULL;
			}
		}
		delete[]mCompandor;
		mCompandor = NULL;
	}

	if(NULL != mpMix)
	{
		delete mpMix;
		mpMix = NULL;
	}

	if(NULL != mpInBufPtrs)
	{
		delete[] mpInBufPtrs;
		mpInBufPtrs = NULL;
	}
}

//! initial function
/*!
	\param   none
	\return  true if success, others false
*/
bool AudioMixer::__Init()
{
	if (m_bIsInit)
	{
		return false;
	}
	if (MAX_THREAD_NUM < mInChannelNum)
	{
		mInChannelNum = MAX_THREAD_NUM;
		AUDIO_PROCESSING_PRINTF("aInChannelNum:%d is too large, AUDIO_MIXER_MAX_CHANNEL_NUM:%f", mInChannelNum, MAX_THREAD_NUM);
	}

	mCompandor = new CCompandor*[mInChannelNum + 1];
	// additional one for out channel
	for(CAUDIO_S32_t i=0; i<mInChannelNum+1; ++i)
	{
		//if (NULL != mCompandor[i])
		//{
		//	delete mCompandor[i];
		//	mCompandor[i] = NULL;
		//}

		mCompandor[i] = new CCompandor(mFs, mProcessFrameLen, mChannelInfo[i]);

		if(NULL == mCompandor[i])
		{
			AUDIO_PROCESSING_PRINTF("new  Compandor failed");
			return false;
		}

		// todo: set different params for in and out channel compandors?
		__SetCompandorParam(i);
	}

	if(NULL != mpMix)
	{
		delete mpMix;
	}

	CAUDIO_U8_t channelIndication = 0x00;
	CAUDIO_S8_t channelSelected = 0x01;
	for(CAUDIO_S32_t i=0; i<mInChannelNum; ++i)
	{
		channelIndication = channelIndication | channelSelected;
		channelSelected = channelSelected << 1;
	}

	mpMix = new CMix(mProcessFrameLen, channelIndication, mFs, MIX_MODE_USED);
	if(NULL == mpMix)
	{
		AUDIO_PROCESSING_PRINTF("new CMix failed");
		return false;
	}

	if(NULL != mpInBufPtrs)
	{
		delete[] mpInBufPtrs;
	}
	mpInBufPtrs = new const AUDIO_DATA_TYPE*[mInChannelNum];
	if(NULL == mpInBufPtrs)
	{
		AUDIO_PROCESSING_PRINTF("new mpInBufPtrs failed");
		return false;
	}

	m_bIsInit = true;
	return true;
};

void AudioMixer::__SetCompandorParam(CAUDIO_S32_t aIdx)
{

}

//! reset function
/*!
	\param   none
	\return  true if success, others false
*/
bool AudioMixer::Reset()
{
	// new CCompandor
	// additional one for out channel
	for(CAUDIO_S32_t i=0; i<mInChannelNum+1; ++i)
	{
		if(NULL != mCompandor[i])
		{
			delete mCompandor[i];
		}

		mCompandor[i] = new CCompandor(mFs, mProcessFrameLen, mChannelInfo[i]);

		if(NULL == mCompandor[i])
		{
			AUDIO_PROCESSING_PRINTF("new  Compandor failed");
			return false;
		}

		// todo: set different params for in and out channel compandors?
		__SetCompandorParam(i);
	}

	// new CMix
	if(NULL != mpMix)
	{
		delete mpMix;
	}

	CAUDIO_S8_t channelIndication = 0x00;
	CAUDIO_S8_t channelSelected = 0x01;
	for(CAUDIO_S32_t i=0; i<mInChannelNum; ++i)
	{
		channelIndication = channelIndication | channelSelected;
		channelSelected = channelSelected << 1;
	}

	mpMix = new CMix(mProcessFrameLen, channelIndication, mFs, MIX_MODE_USED);
	if(NULL == mpMix)
	{
		AUDIO_PROCESSING_PRINTF("new CMix failed");
		return false;
	}

	if(NULL != mpInBufPtrs)
	{
		for(CAUDIO_S32_t i=0; i<mInChannelNum; ++i)
		{
			mpInBufPtrs[i] = NULL;
		}
	}
	return true;
}

#if 0
//! get audio channel mix property
/*!
	\param   aChannelIndex            -   channel index
	\param   aAudioPropertyPageInfo   -   AUDIO_MIX_PROPERTY
	\return  true if success, others false
*/
bool AudioMixer::GetAudioChannelInfo(CAUDIO_S32_t aChannelIndex, AUDIO_MIX_PROPERTY& aAudioPropertyPageInfo)
{
	if(aChannelIndex>=mInChannelNum+1 || aChannelIndex>=MAX_THREAD_NUM+1 || 0>aChannelIndex)
	{

		AUDIO_PROCESSING_PRINTF("channel:%d doesn't exist when get channel info, mInChannelNum:%d", aChannelIndex, mInChannelNum);
		memset(&aAudioPropertyPageInfo, 0, sizeof(aAudioPropertyPageInfo));
		return false;
	}
	else
	{
		aAudioPropertyPageInfo.nAmplitudeMode = static_cast<CAUDIO_S32_t>(mCompandor[aChannelIndex]->GetParameter(COMPANDOR_AMPLITUDE_MODE));
		aAudioPropertyPageInfo.fAttackTime = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_ATTACK_TIME);
		aAudioPropertyPageInfo.fDownRatioOfGainUpdate = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_DOWN_RATIO_GAIN_UPDATE);
		aAudioPropertyPageInfo.fDownThresholdOfAmplitude = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_DOWN_THRESHOLD_AMPLITUDE);
		aAudioPropertyPageInfo.fDownThresholdOfGainSmooth = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_DOWN_THRESHOLD_GAIN_SMOOTH);
		aAudioPropertyPageInfo.fDownThresholdOfLevelSmooth = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_DOWN_THRESHOLD_LEVEL_SMOOTH);
		aAudioPropertyPageInfo.nLevelMode = static_cast<CAUDIO_S32_t>(mCompandor[aChannelIndex]->GetParameter(COMPANDOR_LEVEL_MODE));
		aAudioPropertyPageInfo.fAutomaticGain = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_GAIN);
		aAudioPropertyPageInfo.nCompandorMode = static_cast<CAUDIO_S32_t>(mCompandor[aChannelIndex]->GetParameter(COMPANDOR_COMPANDOR_MODE));
		aAudioPropertyPageInfo.nDelayTime = static_cast<CAUDIO_S32_t>(mCompandor[aChannelIndex]->GetParameter(COMPANDOR_DELAY));
		aAudioPropertyPageInfo.fReleaseTime = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_RELEASE_TIME);
		aAudioPropertyPageInfo.nSmoothMode = static_cast<CAUDIO_S32_t>(mCompandor[aChannelIndex]->GetParameter(COMPANDOE_SMOOTH_MODE));
		aAudioPropertyPageInfo.fUpRatioOfGainUpdate = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_UP_RATIO_GAIN_UPDATE);
		aAudioPropertyPageInfo.fUpThresholdOfAmplitude = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_UP_THRESHOLD_AMPLITUDE);
		aAudioPropertyPageInfo.fUpThresholdOfGainSmooth = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_UP_THRESHOLD_GAIN_SMOOTH);
		aAudioPropertyPageInfo.fUpThresholdOfLevelSmooth = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_UP_THRESHOLD_LEVEL_SMOOTH);
		aAudioPropertyPageInfo.fConcealLevel = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_CONCEAL_LEVEL);
		aAudioPropertyPageInfo.fModerateLevelGain = mCompandor[aChannelIndex]->GetParameter(COMPANDOR_MODERATE_LEVEL_GAIN);

		aAudioPropertyPageInfo.fClipLimit = mpMix->GetParameter(MIX_CLIP_TYPE_LIMIT);
		aAudioPropertyPageInfo.nClipMode = static_cast<CAUDIO_S32_t>(mpMix->GetParameter(MIX_CLIP_TYPE_MODE));
		aAudioPropertyPageInfo.fMixAlpha = mpMix->GetParameter(MIX_TYPE_ALPHA);
		aAudioPropertyPageInfo.fMixBeta = mpMix->GetParameter(MIX_TYPE_BETA);
		aAudioPropertyPageInfo.nMixMode = static_cast<CAUDIO_S32_t>(mpMix->GetParameter(MIX_TYPE_MODE));
		return true;
	}
}

//! set audio channel mix property
/*!
	\param   aChannelIndex            -   channel index
	\param   aAudioPropertyPageInfo   -   AUDIO_MIX_PROPERTY
	\return  true if success, others false
*/
bool AudioMixer::SetAudioChannelInfo(CAUDIO_S32_t aChannelIndex, AUDIO_MIX_PROPERTY& aAudioPropertyPageInfo)
{
	if(aChannelIndex>=mInChannelNum+1 || aChannelIndex>=MAX_THREAD_NUM+1 || 0>aChannelIndex)
	{
		AUDIO_PROCESSING_PRINTF("channel:%d doesn't exist when get channel info, mInChannelNum:%d", aChannelIndex, mInChannelNum);
		return false;
	}
	else
	{
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_AMPLITUDE_MODE, static_cast<AUDIO_DATA_TYPE>(aAudioPropertyPageInfo.nAmplitudeMode));
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_ATTACK_TIME, aAudioPropertyPageInfo.fAttackTime);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_DOWN_RATIO_GAIN_UPDATE, aAudioPropertyPageInfo.fDownRatioOfGainUpdate);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_DOWN_THRESHOLD_AMPLITUDE, aAudioPropertyPageInfo.fDownThresholdOfAmplitude);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_DOWN_THRESHOLD_GAIN_SMOOTH, aAudioPropertyPageInfo.fDownThresholdOfGainSmooth);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_DOWN_THRESHOLD_LEVEL_SMOOTH, aAudioPropertyPageInfo.fDownThresholdOfLevelSmooth);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_LEVEL_MODE, static_cast<AUDIO_DATA_TYPE>(aAudioPropertyPageInfo.nLevelMode));
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_GAIN, aAudioPropertyPageInfo.fAutomaticGain);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_COMPANDOR_MODE, static_cast<AUDIO_DATA_TYPE>(aAudioPropertyPageInfo.nCompandorMode));
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_DELAY, static_cast<AUDIO_DATA_TYPE>(aAudioPropertyPageInfo.nDelayTime));
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_RELEASE_TIME, aAudioPropertyPageInfo.fReleaseTime);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOE_SMOOTH_MODE, static_cast<AUDIO_DATA_TYPE>(aAudioPropertyPageInfo.nSmoothMode));
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_UP_RATIO_GAIN_UPDATE, aAudioPropertyPageInfo.fUpRatioOfGainUpdate);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_UP_THRESHOLD_AMPLITUDE, aAudioPropertyPageInfo.fUpThresholdOfAmplitude);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_UP_THRESHOLD_GAIN_SMOOTH, aAudioPropertyPageInfo.fUpThresholdOfGainSmooth);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_UP_THRESHOLD_LEVEL_SMOOTH, aAudioPropertyPageInfo.fUpThresholdOfLevelSmooth);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_CONCEAL_LEVEL, aAudioPropertyPageInfo.fConcealLevel);
		mCompandor[aChannelIndex]->SetParameter(COMPANDOR_MODERATE_LEVEL_GAIN, aAudioPropertyPageInfo.fModerateLevelGain);

		mpMix->SetParameter(MIX_CLIP_TYPE_LIMIT, aAudioPropertyPageInfo.fClipLimit);
		mpMix->SetParameter(MIX_CLIP_TYPE_MODE, static_cast<AUDIO_DATA_TYPE>(aAudioPropertyPageInfo.nClipMode));
		mpMix->SetParameter(MIX_TYPE_ALPHA, aAudioPropertyPageInfo.fMixAlpha);
		mpMix->SetParameter(MIX_TYPE_BETA, aAudioPropertyPageInfo.fMixBeta);
		mpMix->SetParameter(MIX_TYPE_MODE, static_cast<AUDIO_DATA_TYPE>(aAudioPropertyPageInfo.nMixMode));
		return true;
	}
}
#endif

//! audio mix process function
/*!
	\param   apOutBuffer     -   output buffer
	\param   aOutBufferLen   -   length of output buffer
	\param   appInBuffer     -   input buffer 
	\param   aInDataLen      -   length of input buffer
	\return  true if success, others false
*/
bool AudioMixer::Process(
	AUDIO_DATA_TYPE* apOutBuffer,
	CAUDIO_S32_t aOutBufferLen,
	const AUDIO_DATA_TYPE** appInBuffer, 
	CAUDIO_S32_t aInDataLen)
{
	if (!m_bIsInit)
	{
		return false;
	}
	if (aOutBufferLen < aInDataLen)
	{
		AUDIO_PROCESSING_PRINTF("aOutBufferLen is too small");
		return false;
	}

	if (NULL == apOutBuffer || NULL == appInBuffer)
	{
		AUDIO_PROCESSING_PRINTF("buffer is NULL");
		return false;
	}

	// get current in-channel buffer position
	for(CAUDIO_S32_t inChannelIdx = 0; inChannelIdx < mInChannelNum; inChannelIdx++)
	{
		mpInBufPtrs[inChannelIdx] = appInBuffer[inChannelIdx];
	}

	// compress or expand in channel data
	for (CAUDIO_S32_t i = 0; i < mInChannelNum; ++i)
	{
		mCompandor[i]->AudioCompandor(const_cast<AUDIO_DATA_TYPE*>(mpInBufPtrs[i]));
	}

	if (1 == mInChannelNum)
	{
		memcpy_s(apOutBuffer, aOutBufferLen*sizeof(AUDIO_DATA_TYPE), *mpInBufPtrs, mProcessFrameLen*sizeof(AUDIO_DATA_TYPE));
		return true;
	}
	mpMix->AudioMix(mpInBufPtrs, apOutBuffer);
	// compress or expand out channel data
	mCompandor[mInChannelNum]->AudioCompandor(const_cast<AUDIO_DATA_TYPE*>(apOutBuffer));

	return true;
}








