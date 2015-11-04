/*	
 *	Name:			AudioTransportImpl.cpp
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					03/03/2015 Created
 *
 *
 */

#include "AudioTransportImpl.h"
#include "audiotrace.h"

AudioTransportImpl::AudioTransportImpl()
: m_pTypeCastBuff(NULL)
, m_nTypeCastBuffSize(0)
, m_pAudioSink(NULL)
, m_nSampleRate_in(0)
, m_nSampleRate_out(0)
, m_nReSampleBufferSize(0)
, m_pReSampleBuffer(NULL)
, m_cReSample(NULL)
, m_nActualOutBufferSize(0)
{}

AudioTransportImpl::AudioTransportImpl(CAUDIO_U32_t sample_rate_in, CAUDIO_U32_t sample_rate_out)
: m_pTypeCastBuff(NULL)
, m_nTypeCastBuffSize(0)
, m_pAudioSink(NULL)
, m_nSampleRate_in(sample_rate_in)
, m_nSampleRate_out(sample_rate_out)
, m_nReSampleBufferSize(0)
, m_pReSampleBuffer(NULL)
, m_cReSample(NULL)
, m_nActualOutBufferSize(0)
{
	m_cReSample = new ReSampleImpl(1);
}

AudioTransportImpl::~AudioTransportImpl()
{
	if (m_cReSample)
	{
		delete m_cReSample;
		m_cReSample = NULL;
	}
	if (m_pTypeCastBuff)
	{
		delete m_pTypeCastBuff;
		m_pTypeCastBuff = NULL;
	}
	if (m_pReSampleBuffer)
	{
		delete m_pReSampleBuffer;
		m_pReSampleBuffer = NULL;
	}
};

bool AudioTransportImpl::Transport(
	const void* a_pData,
	DATA_TYPE a_eDataType,
	CAUDIO_U32_t a_nSize,
	TransportId_e a_eTPId)
{
	if(NULL == a_pData)
	{
		AUDIO_PROCESSING_PRINTF("a_pData should not be NULL!");
		return false;
	}

	if(a_eDataType != REAL_FLOAT_DATA)
	{
		return false;
	}

	if(!AudioDataTypeToShort((AUDIO_DATA_TYPE*)a_pData, a_nSize))
	{
		AUDIO_PROCESSING_PRINTF("AudioDataTypeToShort failed!");
		return false;
	}

	//CAUDIO_U32_t frameSize = m_nChannelNum * a_nSize;
	//if(!m_AudioSink.Transport(m_pTypeCastBuff, frameSize))

	if(NULL != m_pAudioSink)
	{
		if (!m_pAudioSink->Transport(m_pTypeCastBuff, REAL_FIX_DATA, m_nActualOutBufferSize, a_eTPId))
		{
			AUDIO_PROCESSING_PRINTF("Transport in audio sink failed!");
			return false;
		}
	}

	return true;
}

bool AudioTransportImpl::RegisterAudioSink(IAudioTransport* a_pAudioSink)
{
	if(NULL == a_pAudioSink)
	{
		AUDIO_PROCESSING_PRINTF("a_AudioSink is NULL!");
		return false;
	}

	m_pAudioSink = a_pAudioSink;
	return true;
}

bool AudioTransportImpl::DeregisterAudioSink()
{
	m_pAudioSink = NULL;
	return true;
}

bool AudioTransportImpl::AllocTypeCastBuff(CAUDIO_U32_t a_nBuffersize)
{
	if( NULL!=m_pTypeCastBuff && m_nTypeCastBuffSize==a_nBuffersize )
	{
		return true;
	}

	if(NULL!=m_pTypeCastBuff)
	{
		delete[] m_pTypeCastBuff;
		m_pTypeCastBuff = NULL;
	}

	m_pTypeCastBuff = new CAUDIO_S16_t[a_nBuffersize];
	if(NULL == m_pTypeCastBuff)
	{
		AUDIO_PROCESSING_PRINTF("alloc type cast buffer failed!");
		return false;
	}
	memset(m_pTypeCastBuff, 0, sizeof(CAUDIO_S16_t)*a_nBuffersize);
	m_nTypeCastBuffSize = a_nBuffersize;

	return true;
}

bool AudioTransportImpl::AllocReSampleBuff(CAUDIO_U32_t a_nReSampleLen)
{
	if (NULL != m_pReSampleBuffer && m_nReSampleBufferSize == a_nReSampleLen)
	{
		return true;
	}

	if (NULL != m_pReSampleBuffer)
	{
		delete[] m_pReSampleBuffer;
		m_pReSampleBuffer = NULL;
	}

	m_pReSampleBuffer = new AUDIO_DATA_TYPE[a_nReSampleLen];
	if (NULL == m_pReSampleBuffer)
	{
		AUDIO_PROCESSING_PRINTF("alloc resample buffer failed!");
		return false;
	}
	memset(m_pReSampleBuffer, 0, sizeof(AUDIO_DATA_TYPE)*a_nReSampleLen);
	m_nReSampleBufferSize = a_nReSampleLen;

	return true;
}

bool AudioTransportImpl::AudioDataTypeToShort(const AUDIO_DATA_TYPE* a_pData, CAUDIO_U32_t m_nSize)
{
	if(NULL == a_pData)
	{
		AUDIO_PROCESSING_PRINTF("a_pData should not be NULL!");
		return false;
	}

	//if(!AllocTypeCastBuff(m_nSize*m_nChannelNum))

	CAUDIO_U32_t resample_bufferlen = 0;
	CAUDIO_U32_t resample_bufferlen_actual = 0;
	

	if (m_cReSample)
	{
		AUDIO_DATA_TYPE temp = AUDIO_DATA_TYPE(m_nSize) / m_nSampleRate_in*m_nSampleRate_out;
		resample_bufferlen = temp > CAUDIO_U32_t(temp) ? CAUDIO_U32_t(temp) + 1 : CAUDIO_U32_t(temp);

		if (!AllocTypeCastBuff(resample_bufferlen))
		{
			AUDIO_PROCESSING_PRINTF("AllocTypeCastBuff failed!");
			return false;
		}
		if (!AllocReSampleBuff(resample_bufferlen))
		{
			AUDIO_PROCESSING_PRINTF("AllocReSampleBuff failed!");
			return false;
		}
		m_cReSample->Process(
			a_pData, 
			m_pReSampleBuffer, 
			m_nSize, 
			resample_bufferlen, 
			m_nSampleRate_in, 
			m_nSampleRate_out, 
			resample_bufferlen_actual);

		CAUDIO_S16_t* pBuff = m_pTypeCastBuff;
		for (CAUDIO_U32_t i = 0; i < resample_bufferlen_actual; ++i)
		{
			AUDIO_DATA_TYPE audioDataTypeData = m_pReSampleBuffer[i] * 32767;
			if (audioDataTypeData > 32767.f)
			{
				*pBuff++ = 32767;
				//*pBuff++ = 32767;
			}
			else if (audioDataTypeData < -32768.f)
			{
				*pBuff++ = -32768;
				//*pBuff++ = -32768;
			}
			else
			{
				*pBuff++ = static_cast<CAUDIO_S16_t>(audioDataTypeData);
				//*pBuff++ = static_cast<CAUDIO_S16_t>(audioDataTypeData);
			}
		}

		m_nActualOutBufferSize = resample_bufferlen_actual;
		
	}
	else
	{
		if (!AllocTypeCastBuff(m_nSize))
		{
			AUDIO_PROCESSING_PRINTF("AllocTypeCastBuff failed!");
			return false;
		}
		CAUDIO_S16_t* pBuff = m_pTypeCastBuff;
		for (CAUDIO_U32_t i = 0; i < m_nSize; ++i)
		{
			AUDIO_DATA_TYPE audioDataTypeData = a_pData[i] * 32767;
			if (audioDataTypeData > 32767.f)
			{
				*pBuff++ = 32767;
				//*pBuff++ = 32767;
			}
			else if (audioDataTypeData < -32768.f)
			{
				*pBuff++ = -32768;
				//*pBuff++ = -32768;
			}
			else
			{
				*pBuff++ = static_cast<CAUDIO_S16_t>(audioDataTypeData);
				//*pBuff++ = static_cast<CAUDIO_S16_t>(audioDataTypeData);
			}
		}

		m_nActualOutBufferSize = m_nSize;
	}

	return true;
}