/*	
 *	Name:			IAudioTransport.h
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

#ifndef _IAUDIO_TRANSPORT_
#define _IAUDIO_TRANSPORT_
#include "audiotypedef.h"

class IAudioTransport
{
protected:
	IAudioTransport()
	{};
	virtual ~IAudioTransport(){};

public:

	virtual bool Transport(
		const void* a_pData,
		DATA_TYPE a_eDataType,
		CAUDIO_U32_t a_nSize,
		CAUDIO_U8_t a_nThreadIdx){return true;};

	virtual bool Transport(
		const void* a_pData,
		DATA_TYPE a_eDataType,
		CAUDIO_U32_t a_nSize,
		TransportId_e a_eTPId){return true;};

};


#endif  // _IAUDIO_TRANSPORT_

