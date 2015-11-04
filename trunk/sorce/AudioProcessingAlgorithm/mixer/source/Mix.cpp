/*! \file   Mix.cpp
*   \author Keil 
*   \date   2014/11/19
*   \brief  Audio Mix Algorithm
*/

#include "AudioMixerDefs.h"
#include "Mix.h"
#include "Clip.h"
#include "audiotrace.h"
CMix::CMix( IN const CAUDIO_S32_t lengthofchannel, 
			IN const CAUDIO_U8_t numofchannel, 
			IN const CAUDIO_S32_t samplespersecond, 
			IN const CAUDIO_S32_t mixmode)
	: m_bIsInit(false)
	, m_nSize(0)
	, m_nCount(0)
	, m_nMode(0)
	, m_nSamplesPerSecond(0)
	, m_cNumOfChannel(0)
	, m_fAlpha(0.f)
	, m_fStepOfAlpha(0.f)
	, m_fBeta(0.f)
	, m_fEnvelopeFactor(0.f)
	, m_fUpRatioOfSmoothGain(0.f)
	, m_fDwRatioOfSmoothGain(0.f)
	, clip(NULL)
	, m_pWeight(NULL)
	, m_pMemoryIndex(NULL)
	, m_pMixing(NULL)
	, m_pClipping(NULL)
	, m_pEnvelope(NULL)
	, m_pSmoothGain(NULL)
{
	//initial parameters
	__Init(lengthofchannel, numofchannel, samplespersecond, mixmode);
}

CMix::~CMix(void)
{
	if (clip) 
	{
		delete clip;
		clip = NULL;
	}
	if (m_pWeight) 
	{
		delete []m_pWeight;
		m_pWeight = NULL;
	}
	if (m_pMemoryIndex) 
	{
		delete []m_pMemoryIndex;
		m_pMemoryIndex = NULL;
	}
	if (m_pMixing) 
	{
		delete []m_pMixing;
		m_pMixing = NULL;
	}
	if (m_pClipping) 
	{
		delete []m_pClipping;
		m_pClipping = NULL;
	}
	if (m_pEnvelope) 
	{
		delete []m_pEnvelope;
		m_pEnvelope = NULL;
	}
	if (m_pSmoothGain)
	{
		delete []m_pSmoothGain;
		m_pSmoothGain = NULL;
	}
}

//! Multichannel audio mixing(output is float).
/*!
 \param   memory - multichannel input data
 \param   output - data of mixing
 \return  true if success, others false
*/
bool CMix::AudioMix(IN const AUDIO_DATA_TYPE** memory, OUT AUDIO_DATA_TYPE* output)
{
	if (false == m_bIsInit || NULL == memory || NULL == output)
	{
		memset(output, 0, sizeof(AUDIO_DATA_TYPE)*m_nSize);
		AUDIO_PROCESSING_PRINTF("Input parameters are wrong or __Init() failed!");
		return false;
	}
	//choose a mode which you want to use
	switch(m_nMode) 
	{
	case MIX_MODE_LINEAR_SUPER: 
		__LinearSuperMix(memory, m_pMixing);
		break;
	case MIX_MODE_NO_RELATED_LINEAR_SUPER:
		__NoRelatedLinearSuperMix(memory, m_pMixing);
		break;
	case MIX_MODE_AVERAGE:
		__AverageMix(memory, m_pMixing);
		break;
	case MIX_MODE_ATTENUATION_FACTOR:
		__AttenuationFactorMix(memory, m_pMixing);
		break;
	case MIX_MODE_ADAPTIVE_WEIGHT:
		__AdaptiveWeightMix(memory, m_pMixing);
		break;
	case MIX_MODE_HIGH_THRESHOLD_ADAPTIVE:
		__HighThresholdAdaptiveWeightMix(memory, m_pMixing);
		break;
	case MIX_MODE_TRUNCATION:
		__TruncationMix(memory, m_pMixing);
		break;
	case MIX_MODE_ALIGNMENT:
		__AlignmentMix(memory, m_pMixing);
		break;
	default:
		break;
	}
	//audio clip 
	clip->AudioClip(m_pMixing, m_pClipping);
	memcpy_s(output, sizeof(AUDIO_DATA_TYPE)*m_nSize, m_pClipping, sizeof(AUDIO_DATA_TYPE)*m_nSize);
	return true;
}

//! Multichannel audio mixing(output is short).
/*!
 \param   memory - multichannel input data
 \param   output - data of mixing
 \return  true if success, others false
*/
bool CMix::AudioMix(IN const AUDIO_DATA_TYPE** memory, OUT CAUDIO_S16_t* output)
{
	if (false == m_bIsInit || NULL == memory || NULL == output)
	{
		memset(output, 0, sizeof(AUDIO_DATA_TYPE)*m_nSize);
		AUDIO_PROCESSING_PRINTF("Input parameters are wrong or __Init() failed!");
		return false;
	}
	//choose a mode which you want to use
	switch(m_nMode) 
	{
	case MIX_MODE_LINEAR_SUPER: 
		__LinearSuperMix(memory, m_pMixing);
		break;
	case MIX_MODE_NO_RELATED_LINEAR_SUPER:
		__NoRelatedLinearSuperMix(memory, m_pMixing);
		break;
	case MIX_MODE_AVERAGE:
		__AverageMix(memory, m_pMixing);
		break;
	case MIX_MODE_ATTENUATION_FACTOR:
		__AttenuationFactorMix(memory, m_pMixing);
		break;
	case MIX_MODE_ADAPTIVE_WEIGHT:
		__AdaptiveWeightMix(memory, m_pMixing);
		break;
	case MIX_MODE_HIGH_THRESHOLD_ADAPTIVE:
		__HighThresholdAdaptiveWeightMix(memory, m_pMixing);
		break;
	case MIX_MODE_TRUNCATION:
		__TruncationMix(memory, m_pMixing);
		break;
	case MIX_MODE_ALIGNMENT:
		__AlignmentMix(memory, m_pMixing);
		break;
	default:
		break;
	}
	//audio clip 
	clip->AudioClip(m_pMixing, m_pClipping);
	//floating point into fixing point
	for(CAUDIO_S32_t k=0; k<m_nSize; ++k) 
	{
		//do judgment
		m_pClipping[k] *= 32767;
		if (m_pClipping[k] > 32767.f) 
		{
			output[k] = 32767;
		}
		else if (m_pClipping[k] < -32768.f) 
		{
			output[k] = -32768;
		}
		else 
		{
			output[k] = static_cast<CAUDIO_S16_t>(m_pClipping[k]);
		}
	}
	return true;
}

#if 0
//! Set parameter 
/*!
 \param   para - type of parameter which you want to change 
 \param   data - value of parameter which you want to change 
 \return  true if success, others false
*/
bool CMix::SetParameter(IN const MIX_PARAMETER_TYPE para, IN const AUDIO_DATA_TYPE data)
{
	if (false == m_bIsInit)
	{
		AUDIO_PROCESSING_PRINTF("__Init() failed!");
		return false;
	}
	switch(para) 
	{
	case MIX_TYPE_MODE:
		if (data<0 || data>7)
		{
			return false;
		}
		m_nMode   = static_cast<CAUDIO_S32_t>(data);
		break;
	case MIX_TYPE_ALPHA:
		if (data<0.f || data>1.f)
		{
			return false;
		}
		m_fAlpha  = data;
		break;
	case MIX_TYPE_BETA:
		if (data<0.f || data>1.f)
		{
			return false;
		}
		m_fBeta   = data;
		break;
	case MIX_CLIP_TYPE_LIMIT:
		clip->SetParameter(CLIP_TYPE_LIMIT, data);
		break;
	case MIX_CLIP_TYPE_MODE:
		clip->SetParameter(CLIP_TYPE_MODE, data);
		break;
	default:
		break;
	}
	return true;
}


//! Get parameter 
/*!
 \param   para - type of parameter which you want to change 
 \return  value of parameter
*/
AUDIO_DATA_TYPE CMix::GetParameter(IN const MIX_PARAMETER_TYPE para) const 
{
	if (false == m_bIsInit)
	{
		AUDIO_PROCESSING_PRINTF("__Init() failed!");
		return false;
	}
	switch(para) 
	{
	case MIX_TYPE_MODE:
		return static_cast<AUDIO_DATA_TYPE>(m_nMode);
	case MIX_TYPE_ALPHA:
		return static_cast<AUDIO_DATA_TYPE>(m_fAlpha);
	case MIX_TYPE_BETA:
		return m_fBeta;
	case MIX_CLIP_TYPE_LIMIT:
		return clip->GetParameter(CLIP_TYPE_LIMIT);
	case MIX_CLIP_TYPE_MODE:
		return clip->GetParameter(CLIP_TYPE_MODE);
	default:
		return 0;
	}
}
#endif
//! Initial mix class.
/*!
 \param   size - length of data
 \param   numofchannel - number of channel
 \param   samplespersecond - samples per second
 \param   mode - mix mode, 0~7
 \return  true if success, others false
*/
bool CMix::__Init(  IN const CAUDIO_S32_t size, 
					IN const CAUDIO_U8_t numofchannel, 
					IN const CAUDIO_S32_t samplespersecond, 
					IN const CAUDIO_S32_t mode)
{
	if (size <= 0 || numofchannel <= 0 || samplespersecond <= 0 || mode < 0 || mode >7)
	{
		AUDIO_PROCESSING_PRINTF("Input parameters are wrong!");
		return false;
	}
	m_nSize = size;
	m_nMode = mode;
	m_cNumOfChannel = numofchannel;
	m_nSamplesPerSecond = samplespersecond;
	//how many channels are there
	CAUDIO_U8_t channel = 0x01;
	for (CAUDIO_S32_t i=0; i<MAX_THREAD_NUM; ++i) 
	{
		if (channel & m_cNumOfChannel)
		{
			++m_nCount;
		}
		channel <<= 1;
	}
	if (0 == m_nCount)
	{
		AUDIO_PROCESSING_PRINTF("Audio mix channels' count cannot be setted zero!");
		return false;
	}
	//if we know m_nCount, we can initial sth
	m_fBeta = 1.f/m_nCount;
	if (NULL != m_pWeight)
	{
		delete []m_pWeight;
	}
	m_pWeight = new AUDIO_DATA_TYPE[m_nCount];
	if (NULL == m_pWeight)
	{
		AUDIO_PROCESSING_PRINTF("New m_pWeight failed!");
		return false;
	}

	if (NULL != m_pMemoryIndex)
	{
		delete []m_pMemoryIndex;
	}
	m_pMemoryIndex = new CAUDIO_S32_t[m_nCount];
	if (NULL == m_pMemoryIndex)
	{
		AUDIO_PROCESSING_PRINTF("New m_pMemoryIndex failed!");
		return false;
	}

	if (NULL != m_pEnvelope)
	{
		delete[]m_pEnvelope;
	}
	m_pEnvelope = new AUDIO_DATA_TYPE[m_nCount];
	if (NULL == m_pEnvelope)
	{
		AUDIO_PROCESSING_PRINTF("New m_pEnvelope failed!");
		return false;
	}

	if (NULL != m_pSmoothGain)
	{
		delete[]m_pSmoothGain;
	}
	m_pSmoothGain = new AUDIO_DATA_TYPE[m_nCount];
	if (NULL == m_pSmoothGain)
	{
		AUDIO_PROCESSING_PRINTF("New m_pSmoothGain failed!");
		return false;
	}

	if (NULL != m_pMixing)
	{
		delete[]m_pMixing;
	}
	m_pMixing = new AUDIO_DATA_TYPE[m_nSize];
	if (NULL == m_pMixing)
	{
		AUDIO_PROCESSING_PRINTF("New m_pMixing failed!");
		return false;
	}

	if (NULL != m_pClipping)
	{
		delete[]m_pClipping;
	}
	m_pClipping = new AUDIO_DATA_TYPE[m_nSize];
	if (NULL == m_pClipping)
	{
		AUDIO_PROCESSING_PRINTF("New m_pClipping failed!");
		return false;
	}

	memset(m_pMixing, 0, sizeof(AUDIO_DATA_TYPE)*m_nSize);	
	memset(m_pClipping, 0, sizeof(AUDIO_DATA_TYPE)*m_nSize);
	memset(m_pWeight, 0, sizeof(AUDIO_DATA_TYPE)*m_nCount);
	memset(m_pMemoryIndex, 0, sizeof(CAUDIO_S32_t)*m_nCount);
	memset(m_pEnvelope, 0, sizeof(CAUDIO_S32_t)*m_nCount);
	for (CAUDIO_S32_t i=0; i<m_nCount; ++i)
	{
		m_pSmoothGain[i] = 1.f;
	}
	//find channel index and put them into m_pfMemoryIndex
	m_nCount = 0;
	channel = 0x01;
	for (CAUDIO_S32_t i=0; i<MAX_THREAD_NUM; ++i) 
	{
		if(channel & m_cNumOfChannel) 
		{
			m_pMemoryIndex[m_nCount] = i;
			++m_nCount;
		}
		channel <<= 1;
	}
	//update ratio 
	m_fAlpha = 1.f;

	if (ADAPTIVE_WEIGHT_MIX_CALC_MODE == ADAPTIVE_WEIGHT_MIX_CALC_MODE_BY_FRAME)
	{
		m_fEnvelopeFactor = K / (K + MIX_ENVELOPESMOOTH_TIME*m_nSamplesPerSecond / (m_nSize*1000.f));//0.1;
		m_fUpRatioOfSmoothGain = K / (K + MIX_GAINSMOOTH_UP_TIME*m_nSamplesPerSecond / (m_nSize*1000.f));//0.06;
		m_fDwRatioOfSmoothGain = K / (K + MIX_GAINSMOOTH_DOWN_TIME*m_nSamplesPerSecond / (m_nSize*1000.f));//0.0034;
	}
	else if (ADAPTIVE_WEIGHT_MIX_CALC_MODE == ADAPTIVE_WEIGHT_MIX_CALC_MODE_BY_SAMPLE)
	{
		m_fEnvelopeFactor = K / (K + MIX_ENVELOPESMOOTH_TIME*m_nSamplesPerSecond / 1000.f);//0.1;
		m_fUpRatioOfSmoothGain = K / (K + MIX_GAINSMOOTH_UP_TIME*m_nSamplesPerSecond / 1000.f);//0.06;
		m_fDwRatioOfSmoothGain = K / (K + MIX_GAINSMOOTH_DOWN_TIME*m_nSamplesPerSecond / 1000.f);//0.0034;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("ADAPTIVE_WEIGHT_MIX_CALC_MODE is wrong");
		return false;
	}

	//new CClip
	if (NULL != clip)
	{
		delete []clip;
	}
	clip = new CClip(m_nSize, CLIP_MODE_USED, CLIP_LIMIT);	
	if (NULL == clip)
	{
		AUDIO_PROCESSING_PRINTF("New clip failed!");
		return false;
	}

	//initial is success
	m_bIsInit = true;
	return true;
}


//! Linear superposition method
/*!
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CMix::__LinearSuperMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t j = 0;
	AUDIO_DATA_TYPE temp_out = 0.f;

	for (; i<m_nSize; ++i) 
	{
		temp_out = 0.f;
		//the superposition of point by point
		for (j=0; j<m_nCount; ++j)
		{
			temp_out += *(in[m_pMemoryIndex[j]]+i);
		}
		*out++ = temp_out;
	}
}

//! No-related linear superposition method
/*!
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CMix::__NoRelatedLinearSuperMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t j = 0;
	AUDIO_DATA_TYPE temp_in  = 0.f;
	AUDIO_DATA_TYPE temp_out = 0.f;

	for (; i<m_nSize; ++i) 
	{
		temp_in  = 0.f;
		temp_out = 0.f;
		//the superposition of point by point
		for (j=0; j<m_nCount; ++j) 
		{
			temp_in = *(in[m_pMemoryIndex[j]]+i);
			temp_out = temp_in + temp_out - temp_in*temp_out;
		}
		*out++ = temp_out;
	}
}

//! Average method
/*!
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CMix::__AverageMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t j = 0;
	AUDIO_DATA_TYPE temp_out = 0.f;

	for (; i<m_nSize; ++i) 
	{
		temp_out = 0.f;
		//the superposition of point by point
		for (j=0; j<m_nCount; ++j) 
		{
			temp_out += *(in[m_pMemoryIndex[j]]+i)/m_nCount;
		}
		*out++ = temp_out;
	}
}

//! Attenuation factor method
/*!
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CMix::__AttenuationFactorMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t j = 0;
	double temp_out = 0.f;

	for (; i<m_nSize; ++i)
	{
		temp_out = 0.f;
		//the superposition of point by point
		for (j=0; j<m_nCount; ++j)
		{
			temp_out += *(in[m_pMemoryIndex[j]]+i);
		}
		//multiplied by the attenuation factor
		temp_out *= m_fAlpha;
		//update attenuation factor
		if (temp_out > 1.f) 
		{
			m_fAlpha = 1 / (temp_out + MINIMUM)*m_fAlpha;
			temp_out = 1.f;
		}
		//update attenuation factor step 
		if (1 != m_fAlpha) 
		{
			m_fAlpha += m_fStepOfAlpha;
			m_fStepOfAlpha = (1-m_fAlpha)/32;
		}
		*out++ = static_cast<AUDIO_DATA_TYPE>(temp_out);
	}
}

//! Adaptive weighting method
/*!
 \param   in - input data
 \param   out - output data
 \return  none
*/
void CMix::__AdaptiveWeightMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t k = 0;
	AUDIO_DATA_TYPE fTotalEvenlope = 0.f; //result of superposition of multi-channel envelope
	AUDIO_DATA_TYPE *fLinearSuper_abs = NULL;
	AUDIO_DATA_TYPE fLinearSuper = 0.f; //result of superposition of multi-channel data
	AUDIO_DATA_TYPE fOutput = 0.f; //the final output
	AUDIO_DATA_TYPE fHighThreshold = 0.75f; // high threshold 
	AUDIO_DATA_TYPE fTotalEvenlopePast = 1.f;
	AUDIO_DATA_TYPE fLinearSuperPast = 1.f;

	if (ADAPTIVE_WEIGHT_MIX_CALC_MODE == ADAPTIVE_WEIGHT_MIX_CALC_MODE_BY_FRAME)
	{
		fLinearSuper_abs = new AUDIO_DATA_TYPE[m_nCount];
		memset(fLinearSuper_abs, 0, sizeof(AUDIO_DATA_TYPE)*m_nCount);

		for (; i < m_nSize; ++i)
		{
			fOutput = 0.f;
			//calculate the adaptive weighting
			for (k = 0; k<m_nCount; ++k)
			{
				fOutput += m_pSmoothGain[k] * (*(in[m_pMemoryIndex[k]] + i));
				fLinearSuper += *(in[m_pMemoryIndex[k]] + i);
				fLinearSuper_abs[k] += *(in[m_pMemoryIndex[k]] + i) >= 0 ? *(in[m_pMemoryIndex[k]] + i) : -(*(in[m_pMemoryIndex[k]] + i));
			}
			*out++ = fOutput;
		}

		if (fLinearSuper > fHighThreshold*m_nSize)
		{
			for (k = 0; k < m_nCount; ++k)
			{
				m_pEnvelope[k] = m_pEnvelope[k] * (1 - m_fEnvelopeFactor) + fLinearSuper_abs[k] * m_fEnvelopeFactor;
				fTotalEvenlope += m_pEnvelope[k];
			}
			for (k = 0; k < m_nCount; ++k)
			{
				m_pWeight[k] = m_pEnvelope[k] / (fTotalEvenlope + MINIMUM);
				__GainSmooth(m_pWeight[k], m_pSmoothGain[k]);
			}
		}
		else
		{
			for (k = 0; k < m_nCount; ++k)
			{
				__GainSmooth(1, m_pSmoothGain[k]);
			}
		}

		delete[]fLinearSuper_abs;
	}
	else if (ADAPTIVE_WEIGHT_MIX_CALC_MODE == ADAPTIVE_WEIGHT_MIX_CALC_MODE_BY_SAMPLE)
	{
		for (; i < m_nSize; ++i)
		{
			fLinearSuper = fTotalEvenlope = fOutput = 0.f;
			//calculate the adaptive weighting
			for (k = 0; k < m_nCount; ++k)
			{
				m_pEnvelope[k] = m_pEnvelope[k] * (1 - m_fEnvelopeFactor) + abs(*(in[m_pMemoryIndex[k]] + i))*m_fEnvelopeFactor;
				fTotalEvenlope += m_pEnvelope[k];
				m_pWeight[k] = m_pEnvelope[k] / (fTotalEvenlopePast + MINIMUM);
				if (fLinearSuperPast > fHighThreshold)
				{
					__GainSmooth(m_pWeight[k], m_pSmoothGain[k]);
					fOutput += m_pSmoothGain[k] * (*(in[m_pMemoryIndex[k]] + i));
				}
				else
				{
					__GainSmooth(1, m_pSmoothGain[k]);
					fOutput += m_pSmoothGain[k] * (*(in[m_pMemoryIndex[k]] + i));
				}
				fLinearSuper += *(in[m_pMemoryIndex[k]] + i);
			}
			fTotalEvenlopePast = fTotalEvenlope;
			fLinearSuperPast = fLinearSuper;
			*out++ = fOutput;
		}
	}

}

/*!	High threshold adaptive weighting method
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CMix::__HighThresholdAdaptiveWeightMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t j = 0;
	CAUDIO_S32_t k = 0;
	AUDIO_DATA_TYPE totalspeech = 0.f;
	AUDIO_DATA_TYPE totalofhighspeech = 0.f;
	AUDIO_DATA_TYPE totalenvelope = 0.f;
	double temp_out = 0.f;

	for (; i<m_nSize; ++i) 
	{

		totalspeech = totalofhighspeech = totalenvelope = 0.f;
		temp_out = 0.;
		//the superposition of point by point
		for (j=0; j<m_nCount; ++j)
		{
			totalspeech += *(in[m_pMemoryIndex[j]]+i);
		}
		//if the output is less than 1, remain
		if (totalspeech <= 1.f) 
		{
			temp_out = totalspeech;
		} 
		else 
		{
			//calculate the adaptive weighting of speechs which are high than the threshold 
			for (k=0; k<m_nCount; ++k) 
			{
				m_pEnvelope[k] = m_pEnvelope[k]*(1-m_fEnvelopeFactor) + abs(*(in[m_pMemoryIndex[k]]+i))*m_fEnvelopeFactor;
				if (m_pEnvelope[k] <= m_fBeta) 
				{
					totalofhighspeech += *(in[m_pMemoryIndex[k]]+i);
				}
				else
				{
					totalenvelope += m_pEnvelope[k];
				}
			}
			//calculate the output
			for (k=0; k<m_nCount; ++k)
			{
				if (m_pEnvelope[k] > m_fBeta) 
				{
					m_pWeight[k] = m_pEnvelope[k]/(totalenvelope + MINIMUM);
					temp_out += m_pWeight[k]*(*(in[m_pMemoryIndex[k]]+i));
				}
			}
			temp_out += totalofhighspeech;
		}
		*out++ = static_cast<AUDIO_DATA_TYPE>(temp_out);
	}
}

/*!	Truncation method
 \param   in - input data
 \param   out - output data
 \return  none
*/
void CMix::__TruncationMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t j = 0;
	AUDIO_DATA_TYPE temp_out = 0.f;

	for (; i<m_nSize; ++i) 
	{
		temp_out = 0.f;
		//the superposition of point by point
		for (j=0; j<m_nCount; ++j)
		{
			temp_out += *(in[m_pMemoryIndex[j]]+i);
		}
		//truncation
		if (temp_out > 1.f)
			temp_out = 1.f;
		*out++ = temp_out;
	}
}

/*!	Alignment method
 \param   in - input data
 \param   out - output data
 \return  none
*/
void CMix::__AlignmentMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out)
{
	CAUDIO_S32_t i = 0;
	CAUDIO_S32_t j = 0;
	CAUDIO_S32_t index = 0;
	AUDIO_DATA_TYPE temp_max = 0.f;

	for (; i<m_nSize; ++i) 
	{
		temp_max = 0.f;
		//the superposition of point by point
		for (j=0; j<m_nCount; ++j) 
		{
			//update envelope
			m_pEnvelope[j] = m_pEnvelope[j]*(1-m_fEnvelopeFactor) + abs(*(in[m_pMemoryIndex[j]]+i))*m_fEnvelopeFactor;
			if (m_pEnvelope[j] > temp_max) 
			{
				temp_max = m_pEnvelope[j];
				index  = j;
			}			
		}
		*out++ = *(in[m_pMemoryIndex[index]]+i);
	}
}

/*!	GainSmooth
 \param   newdata - current new data
 \param   smoothgain - current smooth gain
 \return  true if success, others false
*/
bool CMix::__GainSmooth(IN const AUDIO_DATA_TYPE newdata, IN OUT AUDIO_DATA_TYPE& smoothgain)
{
	AUDIO_DATA_TYPE ratio1 = m_fUpRatioOfSmoothGain;
	AUDIO_DATA_TYPE ratio2 = m_fDwRatioOfSmoothGain;
	AUDIO_DATA_TYPE recursivedata = smoothgain;
	//recursive way
	smoothgain = __Recursive(recursivedata, newdata, ratio1, ratio2);
	return true;
}

/*!		  recursive way
 \param   recursivedata - recursive data 
 \param   newdata - new data 
 \param   ratio1 - up ratio
 \param   ratio2 - down ratio
 \return  recursive result
*/
/*inline */AUDIO_DATA_TYPE CMix::__Recursive(   IN OUT AUDIO_DATA_TYPE recursivedata, 
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