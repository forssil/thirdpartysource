/*! \file   audiotrace.h
*   \author Gao Hua
*   \date   2015/2/28
*   \brief  Audio trace
*/

#ifndef _AUDIO_TRACE_
#define _AUDIO_TRACE_

//#include <AccCtrl.h>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include "trace.h"
#include "audiotypedef.h"

#define MAX_DEBUG_INFO_STRING_LEN 384

// for logging
#ifdef AUDIO_TRACE_DEBUG

// use below macros to print trace
#define AUDIO_PROCESSING_PRINTF(x, ...) \
			{char DEBUG_INFO[MAX_DEBUG_INFO_STRING_LEN]; \
			memset(DEBUG_INFO, 0, MAX_DEBUG_INFO_STRING_LEN); \
			sprintf_s(DEBUG_INFO, MAX_DEBUG_INFO_STRING_LEN-1, "[%40s]  %s", __FUNCTION__, x); \
			WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceUndefined, 0, DEBUG_INFO, ##__VA_ARGS__);}

#define CreateTrace(x) \
	webrtc::Trace::CreateTrace(); \
	webrtc::Trace::SetLevelFilter(webrtc::kTraceDebug); \
	webrtc::Trace::SetTraceFile(x, true)

#define ReturnTrace() webrtc::Trace::ReturnTrace()

#else

#define AUDIO_PROCESSING_PRINTF(x, ...)
#define CreateTrace(x)
#define ReturnTrace()

#endif

#endif //_AUDIO_TRACE_