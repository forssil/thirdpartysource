/*! \file   CAudioPropertyPage.cpp
*   \author Keil
*   \date   2015/4/2
*   \brief  Using this class to store all audio process algorithm properties, and to register it to Audio Process Filter and Audio Channels
			at the time of Audio Engine initialization. So the user can customize audio algorithm properties by adjusting Property Page which
			attached to Audio Process Filter.
*/

#include "AudioPropertyPage.h"

CAudioPropertyPage::CAudioPropertyPage(CAUDIO_U32_t mix_num, bool aec_switch)
	: m_bIsInit(false)
	, m_nMixNum(mix_num)
	, m_sPropertyPage(NULL)
{
	__Init(aec_switch);
}

CAudioPropertyPage::~CAudioPropertyPage()
{
	__Release();
}

bool CAudioPropertyPage::RegisterProperty(IAudioPropertyPage *property_page)
{
	if (!m_bIsInit)
	{
		return false;
	}
	if (!property_page->RegisterProperty(m_sPropertyPage))
	{
		return false;
	}
	return true;
}

bool CAudioPropertyPage::UnregisterProperty(IAudioPropertyPage *property_page)
{
	if (!m_bIsInit)
	{
		return false;
	}
	if (!property_page->UnRegisterProperty())
	{
		return false;
	}
	return true;
}

bool CAudioPropertyPage::__Init(bool aec_switch)
{
	if (m_bIsInit)
	{
		return false;
	}

	m_sPropertyPage = new AUDIO_PROPERTY_PAGE;
	m_sPropertyPage->pAudioAECInfo_ = new AUDIO_AEC_INFO;
	m_sPropertyPage->pAudioAECInfo_->bAECOn_ = aec_switch;
	m_sPropertyPage->pAudioAECInfo_->bNROn_  = true;
	m_sPropertyPage->pAudioMixInfo_ = new AUDIO_MIX_INFO *[m_nMixNum + 1]; // input pin number is m_nMixNum, and output is one 
	for (CAUDIO_U32_t compandor_index = 0; compandor_index < m_nMixNum + 1; ++compandor_index)
	{
		m_sPropertyPage->pAudioMixInfo_[compandor_index] = new AUDIO_MIX_INFO;
		m_sPropertyPage->pAudioMixInfo_[compandor_index]->nChannelDelay_ = 0;
		m_sPropertyPage->pAudioMixInfo_[compandor_index]->fGain_ = 1;
		m_sPropertyPage->pAudioMixInfo_[compandor_index]->nCompandorMode_ = 0;
	}

	m_bIsInit = true;
	return true;
}

bool CAudioPropertyPage::__Release(void)
{
	if (m_sPropertyPage)
	{
		if (m_sPropertyPage->pAudioMixInfo_)
		{
			for (CAUDIO_U32_t compandor_index = 0; compandor_index < m_nMixNum + 1; ++compandor_index)
			{
				if (m_sPropertyPage->pAudioMixInfo_[compandor_index])
				{
					delete m_sPropertyPage->pAudioMixInfo_[compandor_index];
					m_sPropertyPage->pAudioMixInfo_[compandor_index] = NULL;
				}
			}
			delete[] m_sPropertyPage->pAudioMixInfo_;
			m_sPropertyPage->pAudioMixInfo_ = NULL;
		}

		if (m_sPropertyPage->pAudioAECInfo_)
		{
			delete m_sPropertyPage->pAudioAECInfo_;
			m_sPropertyPage->pAudioAECInfo_ = NULL;
		}

		delete m_sPropertyPage;
		m_sPropertyPage = NULL;
	}

	m_bIsInit = false;
	return true;
}

