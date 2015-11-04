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
#ifndef CODYY_AUDIO_COMMON_H
#define CODYY_AUDIO_COMMON_H

#include <stdio.h>
#include <string.h>
#include "audiotypedef.h"

#define TEM_BUFFER_LEN 1280
#define INVALID_VALUE_32 0xFFFFFFFF

bool createBuffer(void*& apBuffer, CAUDIO_U32_t aBufferSize);
bool createBuffer(void*& apBuffer, CAUDIO_U32_t aBufferSize, CAUDIO_U32_t& aBufferWrtPtr);
void deleteBuffer(void* apBuffer);
void resetBuffer(void* apBuffer, CAUDIO_U32_t aBufferLen);
void resetBuffer(void* apBuffer, CAUDIO_U32_t aBufferLen, CAUDIO_U32_t& aBufferWrtPtr);

#endif