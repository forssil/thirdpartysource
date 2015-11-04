/*! \file   Compandor.cpp
*   \author Keil 
*   \date   2014/11/20
*   \brief  Audio Compandor Algorithm
*/

#include "AudioMixerDefs.h"
#include "Compandor.h"
#include "audiotrace.h"
CCompandor::CCompandor( IN const CAUDIO_S32_t samplespersecond, 
						IN const CAUDIO_S32_t lengthofchannel, 
						IN AUDIO_MIX_INFO *mChannelInfo)
	: m_bIsInitial(false)
	, m_nSamplesPerSecond(samplespersecond)
	, m_nSize(lengthofchannel)
	, m_nAttack(0)
	, m_nRelease(0)
	, m_nReleaseTimeSample(0)
	, m_nAttackTimeSample(0)
	, m_lIndexOfDelayMemory(0L)
	, m_lIndexOfSlideWindowMemory(0L)
	, m_fEnergy(0.f)
	, m_fEnvelope(0.f)
	, m_fSmoothGainOfCompandor(0.f)
	, m_fPastGainOfCompandor(0.f)
	, m_fDelta(0.f)
	, m_pDelayMemory(NULL)
	, m_fSlideWindowMemory(NULL)
	, m_pChannelInfo(mChannelInfo)
	, m_fSliceWindowValue(0)
{
	//initial 
	__Init();
}

CCompandor::~CCompandor(void)
{
	if (m_pDelayMemory) 
	{
		delete []m_pDelayMemory;
		m_pDelayMemory = NULL;
	}
	if (m_fSlideWindowMemory)
	{
		delete []m_fSlideWindowMemory;
		m_fSlideWindowMemory = NULL;
	}
}

//! deal with audio compandor 
/*!
 \param   memory - input data
 \param   memory - output data
 \return  true if success, others false
*/
bool CCompandor::AudioCompandor(IN OUT AUDIO_DATA_TYPE* memory)
{
	if(true != m_bIsInitial)
	{
		AUDIO_PROCESSING_PRINTF("__Init() is failed!");
		return false;
	}
	if (NULL == memory)
	{
		AUDIO_PROCESSING_PRINTF("memory cannot be setted NULL!");
		return false;
	}
	//do some judge
	if (m_compandorParameter.levelMode<0 || m_compandorParameter.levelMode>1)
	{
		AUDIO_PROCESSING_PRINTF("m_compandorParameter.levelMode is setted non-legal!");
		return false;
	}
	if (m_compandorParameter.amplitudeMode<0 || m_compandorParameter.amplitudeMode>1)
	{
		AUDIO_PROCESSING_PRINTF("m_compandorParameter.amplitudeMode is setted non-legal!");
		return false;
	}
	if (m_compandorParameter.smoothMode<0 || m_compandorParameter.smoothMode>1)
	{
		AUDIO_PROCESSING_PRINTF("m_compandorParameter.smoothMode is setted non-legal!");
		return false;
	}

	AUDIO_DATA_TYPE* idata = memory;
	// delay = 0
	if (0 == (m_pChannelInfo->nChannelDelay_))
	{
		__PerformCompandor(idata); 
	} 
	// delay > 0
	else 
	{
		memcpy_s(&m_pDelayMemory[m_nSize*((m_lIndexOfDelayMemory + (m_pChannelInfo->nChannelDelay_)) % (m_pChannelInfo->nChannelDelay_))], sizeof(AUDIO_DATA_TYPE)*m_nSize, idata, sizeof(AUDIO_DATA_TYPE)*m_nSize);
		++m_lIndexOfDelayMemory;
		if (m_lIndexOfDelayMemory >= 0) 
		{
			__PerformCompandor(&m_pDelayMemory[m_nSize*((m_lIndexOfDelayMemory) % (m_pChannelInfo->nChannelDelay_))]);
			memcpy_s(idata, sizeof(AUDIO_DATA_TYPE)*m_nSize, &m_pDelayMemory[m_nSize*((m_lIndexOfDelayMemory) % (m_pChannelInfo->nChannelDelay_))], sizeof(AUDIO_DATA_TYPE)*m_nSize);
		}
		else 
		{
			memset(idata, 0, sizeof(AUDIO_DATA_TYPE)*m_nSize);
		}
	}
	return true;
}

#if 0
//! set parameter int struct CompandorType
/*!
 \param   para - type of parameter which you want to change 
 \param   data - value of parameter which you want to change 
 \return  true if success, others false
*/
bool CCompandor::SetParameter(IN const COMPANDOR_PARAMETER_TYPE para, IN const AUDIO_DATA_TYPE data)
{
	if(true != m_bIsInitial)
	{
		AUDIO_PROCESSING_PRINTF("__Init is failed!");
		return false;
	}
	//choose which one you want to change
	switch(para) 
	{
	case COMPANDOR_LEVEL_MODE:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.levelMode cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.levelMode = static_cast<CAUDIO_S32_t>(data);
		break;
	case COMPANDOR_AMPLITUDE_MODE:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.amplitudeMode cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.amplitudeMode = static_cast<CAUDIO_S32_t>(data);
		break;
	case COMPANDOE_SMOOTH_MODE:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.smoothMode cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.smoothMode = static_cast<CAUDIO_S32_t>(data);
		break;
	case COMPANDOR_UP_THRESHOLD_AMPLITUDE:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.upThresholdOfAmplitude cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.upThresholdOfAmplitude = data;
		break;
	case COMPANDOR_DOWN_THRESHOLD_AMPLITUDE:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.downThresholdOfAmplitude cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.downThresholdOfAmplitude = data;
		break;
	case COMPANDOR_UP_RATIO_GAIN_UPDATE:
		if (data<0)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.upRatioOfGainUpdate cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.upRatioOfGainUpdate = data;
		break;
	case COMPANDOR_DOWN_RATIO_GAIN_UPDATE:
		if (data<0)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.downRatioOfGainUpdate cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.downRatioOfGainUpdate = data;
		break;
	case COMPANDOR_UP_THRESHOLD_GAIN_SMOOTH:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.upThresholdOfGainSmooth cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.upThresholdOfGainSmooth = data;
		break;
	case COMPANDOR_DOWN_THRESHOLD_GAIN_SMOOTH:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.downThresholdOfGainSmooth cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.downThresholdOfGainSmooth = data;
		break;
	case COMPANDOR_UP_THRESHOLD_LEVEL_SMOOTH:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.upThresholdOfLevelSmooth cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.upThresholdOfLevelSmooth = data;
		break;
	case COMPANDOR_DOWN_THRESHOLD_LEVEL_SMOOTH:
		if (data<0 || data>1)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.downThresholdOfLevelSmooth cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.downThresholdOfLevelSmooth = data;
		break;
	case COMPANDOR_ATTACK_TIME:
		if(data<0)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.attackTime cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.attackTime = data;
		break;
	case COMPANDOR_RELEASE_TIME:
		if(data<0)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.releaseTime cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.releaseTime = data;
		break;
	case COMPANDOR_CONCEAL_LEVEL:
		if(data<0)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.concealLevel cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.concealLevel = data;
		break;
	case COMPANDOR_MODERATE_LEVEL_GAIN:
		if(data<0)
		{
			AUDIO_PROCESSING_PRINTF("m_compandorParameter.moderateLevelGain cannot be setted by this value!");
			return false;
		}
		m_compandorParameter.moderateLevelGain = data;
		break;																					
	case COMPANDOR_GAIN:
		if(data<0)
		{
			AUDIO_PROCESSING_PRINTF("m_fAutomaticGain cannot be setted by this value!");
			return false;
		}
		m_fAutomaticGain = data;
		break;
	case COMPANDOR_DELAY:
		m_nDelayTime = static_cast<CAUDIO_S32_t>(data);
		//apply buffer for delay
		if (!__CompandorDelay())
		{
			AUDIO_PROCESSING_PRINTF("__CompandorDelay() failed!");
			return false;
		}
		else
		{
			break;
		}
	case COMPANDOR_COMPANDOR_MODE:
		if (data<0 || data>4)
		{
			AUDIO_PROCESSING_PRINTF("m_nCompandorMode cannot be setted by this value!");
			return false;
		}
		m_nCompandorMode = static_cast<CAUDIO_S32_t>(data);
		break;
	default:
		break;
	}
	//update parameter 
	m_nAttackTimeSample = static_cast<CAUDIO_S32_t>(m_compandorParameter.attackTime*m_nSamplesPerSecond/1000);
	m_nReleaseTimeSample = static_cast<CAUDIO_S32_t>(m_compandorParameter.releaseTime*m_nSamplesPerSecond/1000);
	return true;
}

//! get parameter int struct CompandorType
/*!
 \param   para - type of parameter which you want to change 
 \return  value of parameter
*/
AUDIO_DATA_TYPE CCompandor::GetParameter(IN COMPANDOR_PARAMETER_TYPE para) const
{
	if(true != m_bIsInitial)
	{
		return false;
	}
	//choose which one you want to change
	switch(para) 
	{
	case COMPANDOR_LEVEL_MODE:
		return static_cast<AUDIO_DATA_TYPE>(m_compandorParameter.levelMode);
	case COMPANDOR_AMPLITUDE_MODE:
		return static_cast<AUDIO_DATA_TYPE>(m_compandorParameter.amplitudeMode);
	case COMPANDOE_SMOOTH_MODE:
		return static_cast<AUDIO_DATA_TYPE>(m_compandorParameter.smoothMode);
	case COMPANDOR_UP_THRESHOLD_AMPLITUDE:
		return m_compandorParameter.upThresholdOfAmplitude;
	case COMPANDOR_DOWN_THRESHOLD_AMPLITUDE:
		return m_compandorParameter.downThresholdOfAmplitude;
	case COMPANDOR_UP_RATIO_GAIN_UPDATE:
		return m_compandorParameter.upRatioOfGainUpdate;
	case COMPANDOR_DOWN_RATIO_GAIN_UPDATE:
		return m_compandorParameter.downRatioOfGainUpdate;
	case COMPANDOR_UP_THRESHOLD_GAIN_SMOOTH:
		return m_compandorParameter.upThresholdOfGainSmooth;
	case COMPANDOR_DOWN_THRESHOLD_GAIN_SMOOTH:
		return m_compandorParameter.downThresholdOfGainSmooth;
	case COMPANDOR_UP_THRESHOLD_LEVEL_SMOOTH:
		return m_compandorParameter.upThresholdOfLevelSmooth;
	case COMPANDOR_DOWN_THRESHOLD_LEVEL_SMOOTH:
		return m_compandorParameter.downThresholdOfLevelSmooth;
	case COMPANDOR_ATTACK_TIME:
		return m_compandorParameter.attackTime;
	case COMPANDOR_RELEASE_TIME:
		return m_compandorParameter.releaseTime;
	case COMPANDOR_CONCEAL_LEVEL:
		return m_compandorParameter.concealLevel;
	case COMPANDOR_MODERATE_LEVEL_GAIN:
		return m_compandorParameter.moderateLevelGain;																				
	case COMPANDOR_GAIN:
		return m_fAutomaticGain;
	case COMPANDOR_DELAY:
		if (0 == m_nDelayTime)
		{
			return 0;
		} 
		else
		{
			return static_cast<AUDIO_DATA_TYPE>(m_nDelayTime - 1);
		}
	case COMPANDOR_COMPANDOR_MODE:
		return static_cast<AUDIO_DATA_TYPE>(m_nCompandorMode);
	default:
		return 0;
	}
}
#endif

//! initial compandor class.
/*!
 \param   none
 \return  true if success, others false
*/
bool CCompandor::__Init()
{
	if (m_nSamplesPerSecond <= 0 || 
		m_nSize <= 0 || 
		m_pChannelInfo->nCompandorMode_ < 0 ||
		m_pChannelInfo->nCompandorMode_ > 4 ||
		m_pChannelInfo->fGain_ < 0 || 
		m_pChannelInfo->nChannelDelay_ < 0)
	{
		AUDIO_PROCESSING_PRINTF("Input parameters are wrong!");
		return false;
	}
	m_fSmoothGainOfCompandor = 1.f;
	m_fPastGainOfCompandor = 1.f;

	//apply buffer for delay
	if (m_pChannelInfo->nChannelDelay_ > 0)
	{
		m_pChannelInfo->nChannelDelay_ += 1;
		m_lIndexOfDelayMemory = -1 * ((CAUDIO_S64_t)m_pChannelInfo->nChannelDelay_);
		if (NULL != m_pDelayMemory)
		{
			delete []m_pDelayMemory;
		}
		m_pDelayMemory = new AUDIO_DATA_TYPE[(m_pChannelInfo->nChannelDelay_)*m_nSize];
		if (NULL == m_pDelayMemory)
		{
			AUDIO_PROCESSING_PRINTF("new m_pDelayMemory is failed");
			return false;
		}
		memset(m_pDelayMemory, 0, sizeof(AUDIO_DATA_TYPE)*(m_pChannelInfo->nChannelDelay_)*m_nSize);
	}	

	if (NULL != m_fSlideWindowMemory)
	{
		delete[]m_fSlideWindowMemory;
	}
	m_fSlideWindowMemory = new AUDIO_DATA_TYPE[m_nSize];
	if (NULL == m_fSlideWindowMemory)
	{
		AUDIO_PROCESSING_PRINTF("new m_fSlideWindowMemory failed");
		return false;
	}
	memset(m_fSlideWindowMemory, 0, sizeof(AUDIO_DATA_TYPE)*m_nSize);

	//update CompandorParameter 
	//NOTE: available for 22050 sample rate per second
	m_compandorParameter.levelMode = COMPANDOR_LEVEL_MODE_FRAME;
	m_compandorParameter.amplitudeMode = COMPANDOR_ENVELOP_MODE_USED;
	m_compandorParameter.smoothMode = COMPANDOE_SMOOTH_MODE_USED;
	m_compandorParameter.upThresholdOfAmplitude = COMPANDOR_UP_THRESHOLD_AMPLITUDE_USED;
	m_compandorParameter.downThresholdOfAmplitude = COMPANDOR_DOWN_THRESHOLD_AMPLITUDE_USED;
	m_compandorParameter.upRatioOfGainUpdate = COMPANDOR_UP_GAIN_RATIO_USED;
	m_compandorParameter.downRatioOfGainUpdate = COMPANDOR_DOWN_GAIN_RATIO_USED;
	m_compandorParameter.attackTime = COMPANDOR_ATTACK_TIME_USED;
	m_compandorParameter.releaseTime = COMPANDOR_RELEASE_TIME_USED;
	m_compandorParameter.moderateLevelGain = COMPANDOR_MODERATE_LEVEL_GAIN_USED;
	m_compandorParameter.concealLevel = COMPANDOR_CONCEAL_LEVEL_USED;
	//update member variable
	m_nAttackTimeSample = static_cast<CAUDIO_S32_t>(m_compandorParameter.attackTime*m_nSamplesPerSecond/1000.f);
	m_nReleaseTimeSample = static_cast<CAUDIO_S32_t>(m_compandorParameter.releaseTime*m_nSamplesPerSecond/1000.f);
	m_compandorParameter.upThresholdOfGainSmooth = K/(K+COMPANDOR_GAINSMOOTH_UP_TIME*m_nSamplesPerSecond/1000.f);
	m_compandorParameter.downThresholdOfGainSmooth = K/(K+COMPANDOR_GAINSMOOTH_DOWN_TIME*m_nSamplesPerSecond/1000.f);
	m_compandorParameter.upThresholdOfLevelSmooth = K/(K+COMPANDOR_LEVELSMOOTH_UP_TIME*m_nSamplesPerSecond/1000.f);
	m_compandorParameter.downThresholdOfLevelSmooth = K/(K+COMPANDOR_LEVELSMOOTH_DOWN_TIME*m_nSamplesPerSecond/1000.f);
	//initial is success
	m_bIsInitial = true;

	return true;
}

//! perform compandor
/*!
 \param   idata - input/output data
 \return  none
*/
void CCompandor::__PerformCompandor(IN OUT AUDIO_DATA_TYPE* idata)
{
	CAUDIO_S32_t i = 0;
	AUDIO_DATA_TYPE gain = 1.f; //temporary gain 
	AUDIO_DATA_TYPE functionofamplitude = 0.f; //energy = pow(amplitude, 2) , envelope = abs(amplitude)
	//not compressing, not expanding
	if (0 == m_pChannelInfo->nCompandorMode_)
	{
		for (; i<m_nSize; ++i)
			idata[i] = idata[i] * (m_pChannelInfo->fGain_);
	} 
	//compressing or expanding
	//frame by frame processing
	else if(1 == m_compandorParameter.levelMode)	
	{
		//the Level Detector determines the current waveform input amplitude,
		//which is used as the input side of the dynamics processor
		if (0 == m_compandorParameter.amplitudeMode) //energy tracking 
		{
			for (; i< m_nSize; ++i)
				functionofamplitude += idata[i]*idata[i];		
		} 
		else if (1 == m_compandorParameter.amplitudeMode) //envelope tracking
		{
			for (; i< m_nSize; ++i)
				functionofamplitude += abs(idata[i]);
		}
		functionofamplitude /= m_nSize;
		//Level Smooth
		__LevelSmooth(functionofamplitude);
		//Gain Update
		gain = __GainUpdate();
		//Gain Smooth
		__FrameGainSmooth(gain);
		for (i=0; i<m_nSize; ++i) 
		{
			idata[i] = idata[i] * m_fSmoothGainOfCompandor * (m_pChannelInfo->fGain_);
		}
	}
	//sample by sample processing
	else if (0 == m_compandorParameter.levelMode)
	{
		//the Gain Processor amplifies or attenuates the signal depending on the amplitude detected
		for (; i<m_nSize; ++i) 
		{
			//the Level Detector determines the current waveform input amplitude,
			//which is used as the input side of the dynamics processor
			if (0 == m_compandorParameter.amplitudeMode) //energy tracking 
			{
				functionofamplitude = pow(idata[i], 2);
			} 
			else if (1 == m_compandorParameter.amplitudeMode) //envelope tracking
			{
				functionofamplitude = abs(idata[i]);
			}
			//Level Smooth
			__LevelSmooth(functionofamplitude);
			//Gain Update
			gain = __GainUpdate();
			//Gain Smooth
			__SampleGainSmooth(gain);
			//idata[j] = m_compandorParameter.energy;
			idata[i] = idata[i] * m_fSmoothGainOfCompandor * (m_pChannelInfo->fGain_);
		}
	}
}

//! level smooth
/*!
 \param   data - energy/envelope value
 \return  none
*/
void CCompandor::__LevelSmooth(IN const AUDIO_DATA_TYPE data)
{
	//Level Smooth (energy tracking or evelope tracking)
	AUDIO_DATA_TYPE recursivedata = 0.f;
	AUDIO_DATA_TYPE newdata = data;
	AUDIO_DATA_TYPE ratio1 = m_compandorParameter.upThresholdOfLevelSmooth;
	AUDIO_DATA_TYPE ratio2 = m_compandorParameter.downThresholdOfLevelSmooth;
	if (0 == m_compandorParameter.amplitudeMode) //energy tracking 
	{
		recursivedata = m_fEnergy;
		if (0 == m_compandorParameter.smoothMode)
		{ //recursive way
			m_fEnergy = __Recursive(recursivedata, newdata, ratio1, ratio2);
		} 
		else if(1 == m_compandorParameter.smoothMode)
		{ //slide window way
			AUDIO_DATA_TYPE win_average_energy = __SlideWindow(newdata);
			m_fEnergy = __Recursive(m_fEnergy, win_average_energy, ratio1, ratio2);
		}	
	} 
	else if (1 == m_compandorParameter.amplitudeMode) //envelope tracking
	{
		recursivedata = m_fEnvelope;
		if (0 == m_compandorParameter.smoothMode)
		{ //recursive way
			m_fEnvelope = __Recursive(recursivedata, newdata, ratio1, ratio2);
		} 
		else if(1 == m_compandorParameter.smoothMode)
		{ //slide window way
			AUDIO_DATA_TYPE envelop_average_energy = __SlideWindow(newdata);
			m_fEnvelope = __Recursive(m_fEnvelope, envelop_average_energy, ratio1, ratio2);
		}
	}
}

//! gain update
/*!
 \return  output gain value
*/
AUDIO_DATA_TYPE CCompandor::__GainUpdate()
{
	AUDIO_DATA_TYPE gain = 1.f;
	AUDIO_DATA_TYPE recursivedata = 0.f;
	AUDIO_DATA_TYPE upthreshold = 0.f;
	AUDIO_DATA_TYPE downthreshold = 0.f;
	AUDIO_DATA_TYPE upratio = m_compandorParameter.upRatioOfGainUpdate;
	AUDIO_DATA_TYPE downratio = m_compandorParameter.downRatioOfGainUpdate;
	AUDIO_DATA_TYPE concealLevel = m_compandorParameter.concealLevel;
	AUDIO_DATA_TYPE moderateLevelGain = m_compandorParameter.moderateLevelGain;
	upthreshold = m_compandorParameter.upThresholdOfAmplitude;
	downthreshold = m_compandorParameter.downThresholdOfAmplitude;
	if (0 == m_compandorParameter.amplitudeMode) //energy tracking 
	{
		recursivedata = sqrt(m_fEnergy);
	} 
	else if (1 == m_compandorParameter.amplitudeMode) //envelope tracking
	{
		
		recursivedata = m_fEnvelope;
	}	
	//mode1 = high expand, low expand 
	if (1 == m_pChannelInfo->nCompandorMode_)
	{
		if (recursivedata > upthreshold)
		{ //if greater than the up limit 
			gain = pow((recursivedata / upthreshold), (upratio - 1));
		}
		else if (concealLevel < recursivedata && (recursivedata < downthreshold))
		{ //if smaller than the down limit 
			gain = pow((recursivedata / downthreshold), (1/downratio - 1));
		}
	} 
	//mode2 = high expand, low compress	
	else if (2 == m_pChannelInfo->nCompandorMode_)
	{
		if (recursivedata > upthreshold)
		{ //if greater than the up limit 
			gain = pow((recursivedata / upthreshold), (upratio - 1));
		}
		else if (recursivedata < downthreshold)
		{ //if smaller than the down limit
			gain = pow((recursivedata / downthreshold), (downratio - 1));
		}
	}
	//mode3 = high compress, low expand 
	else if (3 == m_pChannelInfo->nCompandorMode_)
	{
		if (recursivedata > upthreshold)
		{ //if greater than the up limit 
			gain = moderateLevelGain * pow((recursivedata / upthreshold), (1/upratio - 1));
		}
		else if (concealLevel < recursivedata && (recursivedata < downthreshold))
		{ //if smaller than the down limit 
			gain = moderateLevelGain * pow((recursivedata / downthreshold), (1/downratio - 1));
		}
		else if(concealLevel < recursivedata)
		{
			gain = moderateLevelGain;
		}
		else 
		{
			gain = 1;
		}
	}
	//mode4 = high compress, low compress
	else if (4 == m_pChannelInfo->nCompandorMode_)
	{
		if (recursivedata > upthreshold)
		{ //if greater than the up limit 
			gain = pow((recursivedata / upthreshold), (1/upratio - 1));
		}
		else if (recursivedata < downthreshold)
		{ //if smaller than the down limit
			gain = pow((recursivedata / downthreshold), (downratio - 1));
		}
	}
	return gain;
}

//! sample gain smooth
/*!
 \param   gain - gain value of current sample
 \return  none
*/
void CCompandor::__SampleGainSmooth(IN const AUDIO_DATA_TYPE gain)
{
	//if gain != m_fPastGainOfCompandor , then begin to adjust gain
	if (gain != m_fPastGainOfCompandor)
	{
		m_nAttack = 0;
	}
	//let m_fSmoothGainOfCompandor back to 1
	if (1 == gain)
	{ //release time , let gain value back to 1
		if (0 == m_nRelease)
		{
			m_fDelta = (1 - m_fSmoothGainOfCompandor) / m_nReleaseTimeSample;
			m_nAttack = 0;
		}
		if (m_nRelease < m_nReleaseTimeSample)
		{
			m_nRelease += 1;
			m_fSmoothGainOfCompandor += m_fDelta;
		}
	} 
	//make m_fSmoothGainOfCompandor keep up with gain
	else
	{ //attack time , change gain value 
		if (0 == m_nAttack)
		{
			m_fDelta = (gain - m_fSmoothGainOfCompandor) / m_nAttackTimeSample;
			m_nRelease = 0;
		}
		if (m_nAttack < m_nAttackTimeSample)
		{
			m_nAttack += 1;
			m_fSmoothGainOfCompandor += m_fDelta;
		} 
	}
	m_fPastGainOfCompandor = gain;
}

//! frame gain smooth
/*!
 \param   gain - gain value of current sample
 \return  none
*/
void CCompandor::__FrameGainSmooth(IN const AUDIO_DATA_TYPE gain)
{
	AUDIO_DATA_TYPE ratio1 = m_compandorParameter.upThresholdOfGainSmooth;
	AUDIO_DATA_TYPE ratio2 = m_compandorParameter.downThresholdOfGainSmooth;
	AUDIO_DATA_TYPE recursivedata = m_fSmoothGainOfCompandor;
	//recursive way
	m_fSmoothGainOfCompandor = __Recursive(recursivedata, gain, ratio1, ratio2);
}

//! recursive way
/*!
 \param   recursivedata - recursive data 
 \param   newdata - new data 
 \param   ratio1 - up ratio
 \param   ratio2 - down ratio
 \return  recursive result
*/
/*inline */AUDIO_DATA_TYPE CCompandor::__Recursive( IN OUT AUDIO_DATA_TYPE recursivedata,
													IN const AUDIO_DATA_TYPE newdata, 
													IN const AUDIO_DATA_TYPE ratio1, 
													IN const AUDIO_DATA_TYPE ratio2)
{
	if (newdata > recursivedata)
	{
		recursivedata = recursivedata*(1-ratio1) + newdata*ratio1;
	}
	else 
	{
		recursivedata = recursivedata*(1-ratio2) + newdata*ratio2;
	}
	return recursivedata;
}

//! slide window way
/*!
 \param   newdata - new data 
 \return  slide window result
*/
AUDIO_DATA_TYPE CCompandor::__SlideWindow(IN const AUDIO_DATA_TYPE newdata)
{
	if(m_lIndexOfSlideWindowMemory>=m_nSize)
	{
		m_lIndexOfSlideWindowMemory = 0;
	}

	AUDIO_DATA_TYPE abandonData = m_fSlideWindowMemory[m_lIndexOfSlideWindowMemory];
	m_fSlideWindowMemory[m_lIndexOfSlideWindowMemory] = newdata;
	m_lIndexOfSlideWindowMemory++;

	m_fSliceWindowValue = m_fSliceWindowValue*m_nSize + newdata - abandonData;
	m_fSliceWindowValue /= m_nSize;

	return m_fSliceWindowValue;
}

//! initial delay time
/*!
 \param   none
 \return  none
*/
bool CCompandor::__CompandorDelay()
{
	//apply buffer for delay
	if (m_pChannelInfo->nChannelDelay_ < 0)
	{
		AUDIO_PROCESSING_PRINTF("m_nDelayTime cannot be setted less than zero");
		return false;
	}
	else if (m_pChannelInfo->nChannelDelay_ > 0)
	{
		m_pChannelInfo->nChannelDelay_ += 1;
		m_lIndexOfDelayMemory = -1 * ((CAUDIO_S64_t)m_pChannelInfo->nChannelDelay_);
		if (m_pDelayMemory)
		{
			delete []m_pDelayMemory;
			m_pDelayMemory = NULL;
		}
		m_pDelayMemory = new AUDIO_DATA_TYPE[(m_pChannelInfo->nChannelDelay_) * m_nSize];
		if (NULL == m_pDelayMemory)
		{
			AUDIO_PROCESSING_PRINTF("new m_nDelayTime failed");
			return false;
		}
	}	
	memset(m_pDelayMemory, 0, sizeof(AUDIO_DATA_TYPE)*(m_pChannelInfo->nChannelDelay_)*m_nSize);
	return true;
}