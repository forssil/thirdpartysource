/*! \file   AudioMixImpl.h
*   \author Keil
*   \date   2015/3/3
*   \brief  Audio Mix Implement class
*/

#include "AudioMixImpl.h" 

//! Constructor
/*!
	\param   _thread_num_       -   number of threads
	\param   _fs_               -   sample per frame
	\param   _time_per_frame_   -   time of one frame (ms)
	\return  none
*/
CAudioMixImpl::CAudioMixImpl(CAUDIO_U8_t  _thread_num_, CAUDIO_U32_t _fs_, AUDIO_DATA_TYPE _time_per_frame_)
	: m_nThreadNum(_thread_num_)
	, m_nFs(_fs_)
	, m_fTimePerFrame(_time_per_frame_)

	, m_nInputBufLen(0)
	, m_nOutputBufLen(0)
	, m_bIsInitSuccess(false)
	, m_ppInputBuf(NULL)
	, m_pOutputBuf(NULL)
	, m_iAudioMixer(NULL)
	, m_pChannelInfo(NULL)
{
	__Init();
}

CAudioMixImpl::CAudioMixImpl(CAUDIO_U8_t  _thread_num_, CAUDIO_U32_t _fs_, AUDIO_DATA_TYPE _time_per_frame_, AUDIO_PROPERTY_PAGE *_property_page_)
	: m_nThreadNum(_thread_num_)
	, m_nFs(_fs_)
	, m_fTimePerFrame(_time_per_frame_)

	, m_nInputBufLen(0)
	, m_nOutputBufLen(0)
	, m_bIsInitSuccess(false)
	, m_ppInputBuf(NULL)
	, m_pOutputBuf(NULL)
	, m_iAudioMixer(NULL)
	, m_pChannelInfo(NULL)
{
	__Init(_property_page_->pAudioMixInfo_);
}

//! Destructor
/*!
	\param   none
	\return  none
*/
CAudioMixImpl::~CAudioMixImpl()
{
	if (m_iAudioMixer)
	{
		DeleteIAudioMixerInst(m_iAudioMixer);
		m_iAudioMixer = NULL;
	}

	if (NULL != m_pChannelInfo)
	{
		for (CAUDIO_U8_t i = 0; i < m_nThreadNum + 1; ++i)
		{
			if (NULL != m_pChannelInfo[i])
			{
				delete m_pChannelInfo[i];
				m_pChannelInfo[i] = NULL;
			}
		}
		delete m_pChannelInfo;
		m_pChannelInfo = NULL;
	}

	m_ppInputBuf = NULL;
	m_pOutputBuf = NULL;
	m_bIsInitSuccess = false;
}

//! initial function
/*!
	\param   none
	\return  true if success, others false
*/
bool CAudioMixImpl::__Init(void)
{
	if (m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("%s init operation failed", LOG_MIX_IMPL);
		return false;
	}

	m_pChannelInfo = new AUDIO_MIX_INFO*[m_nThreadNum + 1];
	for (CAUDIO_U8_t i = 0; i < m_nThreadNum + 1; ++i)
	{
		m_pChannelInfo[i] = new AUDIO_MIX_INFO;
		m_pChannelInfo[i]->nChannelDelay_ = 0;
		m_pChannelInfo[i]->fGain_ = 1;
		m_pChannelInfo[i]->nCompandorMode_ = 0;
	}

	if (m_iAudioMixer)
	{
		DeleteIAudioMixerInst(m_iAudioMixer);
		m_iAudioMixer = NULL;
	}
	m_iAudioMixer = CreateIAudioMixerInst(m_nThreadNum, m_nFs, m_fTimePerFrame, m_pChannelInfo);
	if (!m_iAudioMixer)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_MIX_IMPL);
		return false;
	}

	m_bIsInitSuccess = true;
	return true;
}

bool CAudioMixImpl::__Init(AUDIO_MIX_INFO **_channel_info_)
{
	if (m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("%s init operation failed", LOG_MIX_IMPL);
		return false;
	}

	if (m_iAudioMixer)
	{
		DeleteIAudioMixerInst(m_iAudioMixer);
		m_iAudioMixer = NULL;
	}
	m_iAudioMixer = CreateIAudioMixerInst(m_nThreadNum, m_nFs, m_fTimePerFrame, _channel_info_);
	if (!m_iAudioMixer)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_MIX_IMPL);
		return false;
	}

	m_bIsInitSuccess = true;
	return true;
}

//! reset function
/*!
	\param   none
	\return  true if success, others false
*/
bool CAudioMixImpl::Reset(void)
{
	if (m_iAudioMixer)
	{
		m_iAudioMixer->Reset();
	}
	return true;
}

// Analytical structure
/*!
	\param   aShareData   -   AUDIO_PROCESSING_DATA_SHARE
	\return  true if success, others false
*/
bool CAudioMixImpl::__AnalyticalStructure(IN const audio_pro_share &aShareData)
{
	m_ppInputBuf = aShareData.pMixIn_; 
	m_pOutputBuf = aShareData.pMixOut_; 
	m_nInputBufLen = aShareData.nSamplesPerMixInChannel_; 
	m_nOutputBufLen = aShareData.nLenOfMixOut_;
	assert(aShareData.nChannelsInMixIn_ == m_nThreadNum);
	
	return true;
}

//! AudioProcessingImplBase API
/*!
	\param   aShareData   -   AUDIO_PROCESSING_DATA_SHARE
	\return  int
*/
/*virtual */int CAudioMixImpl::process(IN audio_pro_share &aShareData)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("%s init operation failed", LOG_MIX_IMPL);
		return false;
	}

	__AnalyticalStructure(aShareData);

	if (m_iAudioMixer)
	{
		m_iAudioMixer->Process(m_pOutputBuf, m_nOutputBufLen, (const AUDIO_DATA_TYPE**)m_ppInputBuf, m_nInputBufLen);
	}
	 
	return 0;
}

#if 0
//! set audio channel mix property
/*!
	\param   _channel_index_        -   channel index
	\param   _audio_mix_property_   -   AUDIO_MIX_PROPERTY
	\return  true if success, others false
*/
bool CAudioMixImpl::SetAudioChannelInfo(CAUDIO_U32_t _channel_index_, AUDIO_MIX_PROPERTY &_audio_mix_property_)
{
	bool ret = false;
	if (m_iAudioMixer)
	{
		ret = m_iAudioMixer->SetAudioChannelInfo(_channel_index_, _audio_mix_property_);
	}
		
	return ret;
}

//! get audio channel mix property
/*!
	\param   _channel_index_        -   channel index
	\param   _audio_mix_property_   -   AUDIO_MIX_PROPERTY
	\return  true if success, others false
*/
bool CAudioMixImpl::GetAudioChannelInfo(CAUDIO_U32_t _channel_index_, AUDIO_MIX_PROPERTY &_audio_mix_property_)
{
	bool ret = false;
	if (m_iAudioMixer)
	{
		ret = m_iAudioMixer->GetAudioChannelInfo(_channel_index_, _audio_mix_property_);
	}
		
	return ret;
}
#endif