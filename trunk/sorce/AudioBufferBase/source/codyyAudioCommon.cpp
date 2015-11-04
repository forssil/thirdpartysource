/***********************************************************************
 *  Author
;*      Zhong Yaozhu
;*      
;*     
;*
;*  History
;*      10/15/2014 Created
;*
;*
;*************************************************************************/
#include "codyyAudioCommon.h"
#include "audiotrace.h"
bool createBuffer(void*& apBuffer, CAUDIO_U32_t aBufferSize)
{
	deleteBuffer(apBuffer);

	apBuffer = new char[aBufferSize];
	if (NULL == apBuffer)
	{
		AUDIO_PROCESSING_PRINTF("alloc Buffer for len of %d failed!", aBufferSize);
		return false;
	}

	resetBuffer(apBuffer, aBufferSize);

	return true;
}

bool createBuffer(void*& apBuffer, CAUDIO_U32_t aBufferSize, CAUDIO_U32_t& aBufferWrtPtr)
{
	deleteBuffer(apBuffer);

	apBuffer = new char[aBufferSize];
	if (NULL == apBuffer)
	{
		AUDIO_PROCESSING_PRINTF("alloc Buffer for len of %d failed!", aBufferSize);
		return false;
	}

	resetBuffer(apBuffer, aBufferSize, aBufferWrtPtr);

	return true;
}

void deleteBuffer(void* apBuffer)
{
	if (NULL != apBuffer)
	{
		delete[] apBuffer;
		apBuffer = NULL;
	}
}

void resetBuffer(void* apBuffer, CAUDIO_U32_t aBufferLen)
{
	memset(apBuffer, 0, aBufferLen);
}

void resetBuffer(void* apBuffer, CAUDIO_U32_t aBufferLen, CAUDIO_U32_t& aBufferWrtPtr)
{
	memset(apBuffer, 0, aBufferLen);
//	aBufferWrtPtr = 0;
}