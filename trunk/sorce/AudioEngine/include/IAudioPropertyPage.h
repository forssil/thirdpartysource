/*! \file   IAudioPropertyPage.h
*   \author Keil
*   \date   2015/4/2
*   \brief  Using it for transporting pointer of audio algorithm property.  
*/

#ifndef _I_AUDIO_PROPERTYPAGE_H_
#define _I_AUDIO_PROPERTYPAGE_H_

#include "audiotypedef.h"

class IAudioPropertyPage
{
protected:
	IAudioPropertyPage(){};
	virtual ~IAudioPropertyPage(){};

public:
	virtual bool RegisterProperty(AUDIO_PROPERTY_PAGE *property_page) = 0;

	virtual bool UnRegisterProperty() = 0;
};

#endif //_I_AUDIO_PROPERTYPAGE_H_