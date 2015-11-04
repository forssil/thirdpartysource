/*! \file   Clip.cpp
*   \author Keil 
*   \date   2014/11/18
*   \brief  Amplitude limiter Algorithm
*/

#include "AudioMixerDefs.h"
#include "Clip.h"
#include "audiotrace.h"
CClip::CClip(IN const CAUDIO_S32_t size, IN const CAUDIO_S32_t mode, IN const AUDIO_DATA_TYPE limit)
	: m_bIsInit(false)
	, m_nSize(0)
	, m_nMode(0)
	, m_fLimit(0.f)
{
	//initial parameters
	__Init(size, mode, limit);
}


CClip::~CClip(void)
{
}

//! Initial clip class.
/*!
 \param   size - length of data
 \param   mode - clip mode
 \param   limit - limit value
 \return  true if success, others false
*/
bool CClip::__Init(IN const CAUDIO_S32_t size, IN const CAUDIO_S32_t mode, IN const AUDIO_DATA_TYPE limit)
{
	if (size <= 0 || mode<0 || mode>3 || limit<0.f || limit>1.f)
	{
		AUDIO_PROCESSING_PRINTF("Input parameters are wrong!");
		return false;
	}
	m_nSize = size;
	m_nMode = mode;
	m_fLimit = limit;
	m_bIsInit = true;
	return true;
}

//! Audio Amplitude Clip.
/*!
 \param   in - input data
 \param   out - output data
 \return  true if success, others false
*/
bool CClip::AudioClip(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out)
{
	if (false == m_bIsInit || NULL == in || NULL == out)
	{
		AUDIO_PROCESSING_PRINTF("Input parameters are wrong or __Init() is failed!");
		return false;
	}
	//choose a mode which you want to use
	switch(m_nMode) 
	{
	case 0:
		__Clip0(in, out);
		break;
	case 1:
		__Clip1(in, out);
		break;
	case 2:
		__Clip2(in, out);
		break;
	case 3:
		__Clip3(in, out);
		break;
	default:
		break;
	}
	return true;
}

#if 0
//! SetParameter
/*!
 \param   para - type of parameter which you want to change 
 \param   data - value of parameter which you want to change 
 \return  true if success, others false
*/
bool CClip::SetParameter(IN const CLIP_PARAMETER_TYPE para, IN const AUDIO_DATA_TYPE data)
{
	if (false == m_bIsInit)
	{
		AUDIO_PROCESSING_PRINTF("__Init() is failed!");
		return false;
	}
	switch(para)
	{
	case CLIP_TYPE_LIMIT:
		if (data<0.f || data>1.f)
		{
			return false;
		}
		m_fLimit = data;
		break;
	case CLIP_TYPE_MODE:
		if (data<0 || data>3)
		{
			return false;
		}
		m_nMode = static_cast<CAUDIO_S32_t>(data);
		break;
	default:
		break;
	}
	return true;
}

//! GetParameter
/*!
 \param   para - type of parameter which you want to change 
 \return  value of parameter
*/
AUDIO_DATA_TYPE CClip::GetParameter(IN const CLIP_PARAMETER_TYPE para) const 
{
	if (false == m_bIsInit)
	{
		AUDIO_PROCESSING_PRINTF("__Init() is failed!");
		return false;
	}
	switch(para)
	{
	case CLIP_TYPE_LIMIT:
		return m_fLimit;
	case CLIP_TYPE_MODE:
		return static_cast<AUDIO_DATA_TYPE>(m_nMode);
	default:
		return 0.f;
	}
}
#endif

//! Clip0
/*!
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CClip::__Clip0(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out)
{
	CAUDIO_S32_t i = 0;
	AUDIO_DATA_TYPE temp_in = 0.f;
	double temp_out = 0.f;

	for (; i<m_nSize; ++i)
	{
		temp_in  = *in++;
		if (abs(temp_in)<m_fLimit)
		{
			temp_out = m_fLimit*sin(PI*temp_in/(2*m_fLimit));
		}
		else
		{
			temp_out = m_fLimit*__Sign(temp_in);
		}
		*out++ = static_cast<AUDIO_DATA_TYPE>(temp_out);
	}
}

//! Clip1
/*!
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CClip::__Clip1(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out)
{
	CAUDIO_S32_t i = 0;
	AUDIO_DATA_TYPE temp_in = 0.f;
	AUDIO_DATA_TYPE temp_out = 0.f;

	for (; i<m_nSize; ++i)
	{
		temp_in  = *in++;
		if (abs(temp_in)<m_fLimit)
		{
			temp_out = m_fLimit*tanh(temp_in/m_fLimit)/tanh(1.f);
		}
		else
		{
			temp_out = m_fLimit*__Sign(temp_in);
		}
		*out++ = temp_out;
	}
}

//! Clip2
/*!
 \param   in  - input data
 \param   out - output data
 \return  none
*/
void CClip::__Clip2(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out)
{
	CAUDIO_S32_t i = 0;
	AUDIO_DATA_TYPE fRatio = 2.f;
	AUDIO_DATA_TYPE fHighLimit = 1.5f*m_fLimit;
	AUDIO_DATA_TYPE fLowLimit = fRatio*m_fLimit - fHighLimit;
	AUDIO_DATA_TYPE temp_in  = 0.f;
	AUDIO_DATA_TYPE temp_out = 0.f;
	AUDIO_DATA_TYPE temp = 0.f;

	for (; i<m_nSize; ++i)
	{
		temp_in  = *in++;
		if (abs(temp_in)<fLowLimit)
		{
			temp_out = temp_in;
		} 
		else if(abs(temp_in)>=fLowLimit && abs(temp_in)<fHighLimit)
		{
			temp = fLowLimit+(abs(temp_in)-fLowLimit)/(1+pow(((abs(temp_in)-fLowLimit)/(fHighLimit-fLowLimit) ),2) );
			temp_out = __Sign(temp_in)*temp;
		}
		else
		{
			temp_out = __Sign(temp_in)*(fLowLimit + fHighLimit) / fRatio;
		}
		*out++ = temp_out;
	}
}

//! Clip3
/*!
 \param   in - input data
 \param   out - output data
 \return  none
*/
void CClip::__Clip3(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out)
{
	CAUDIO_S32_t i = 0;
	AUDIO_DATA_TYPE fLowLimit = 0.9f*m_fLimit;
	AUDIO_DATA_TYPE temp_in = 0.f;
	double temp_out = 0.f;
	double temp = 0.f;
	for (; i<m_nSize; ++i)
	{
		temp_in  = *in++;
		if (abs(temp_in)<fLowLimit)
		{
			temp_out = temp_in;
		} 
		else if(abs(temp_in)>=fLowLimit && abs(temp_in)<m_fLimit)
		{
			temp = fLowLimit+(m_fLimit-fLowLimit)*sin(PI*(abs(temp_in)-fLowLimit)/(2*(m_fLimit-fLowLimit) ) );
			temp_out = __Sign(temp_in)*temp;
		}
		else
		{
			temp_out = __Sign(temp_in)*m_fLimit;
		}
		*out++ = static_cast<AUDIO_DATA_TYPE>(temp_out);
	}
}

//! Sign
/*!
 \param   value - input value
 \return  sign value
					   _
					  |  1 ,  x>0
			sign(x) = |  0 ,  x=0
					  |_ -1,  x<0
*/
/*inline */CAUDIO_S32_t CClip::__Sign(IN const AUDIO_DATA_TYPE value) const
{
	if (value > 0.f)
	{
		return 1;
	}
	else if(value < 0.f)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}