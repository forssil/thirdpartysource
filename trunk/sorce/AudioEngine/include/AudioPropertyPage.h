/*! \file   CAudioPropertyPage.h
*   \author Keil
*   \date   2015/4/2
*   \brief  Using this class to store all audio process algorithm properties, and to register it to Audio Process Filter and Audio Channels 
			at the time of Audio Engine initialization. So the user can customize audio algorithm properties by adjusting Property Page which
			attached to Audio Process Filter. 
*/

#ifndef _AUDIO_PROPERTYPAGE_H_
#define _AUDIO_PROPERTYPAGE_H_

#include "audiotypedef.h"
#include "IAudioPropertyPage.h"

class CAudioPropertyPage
{
public:
	CAudioPropertyPage(CAUDIO_U32_t mix_num, bool aec_switch);

	~CAudioPropertyPage();

	bool RegisterProperty(IAudioPropertyPage *property_page);

	bool UnregisterProperty(IAudioPropertyPage *property_page);

private:
	bool __Init(bool aec_switch);

	bool __Release(void);

public:
	CAUDIO_U32_t m_nMixNum;
	AUDIO_PROPERTY_PAGE *m_sPropertyPage;
	bool m_bIsInit;
};
#endif //_AUDIO_PROPERTYPAGE_H_

