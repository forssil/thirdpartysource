/*	
 *	Name:			MultipleSourceSync.cpp
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					12/19/2014 Created										Zhong Yaozhu
 *					03/05/2015 add PULL MODE								Zhong Yaozhu
 *					03/09/2015 check return value of processData API		Zhong Yaozhu
							   of m_pAudioModuleImpl to decide if to 
							   pop the frame in processing list
							   
 *
 *
 */
#include <assert.h>
#include "MultipleSourceSync.h"
#include "codyyAudioCommon.h"
#include "event_wrapper.h"
#include "audiotypedef.h"
#include "audiotrace.h"
#define SRC_SLICE_MEMORY_POOL_INITIAL_SIZE (100 * MAX_INPUT_THREAD_NUM)
#define MAX_SRC_EXCHANGE_EVENT_WAIT_TIME WEBRTC_EVENT_INFINITE
#define LIST_EMPTY_IND_BIT 0x00000001
#define ALL_LIST_EMPTY_FLAG 0x00000000
#define MAX_TIME_WAIT_FOR_DATA_FROM_PULL_MODE 3


MultipleSourceSync::MultipleSourceSync(
	IAudioModuleImpleBase* a_AudioModuleImpl,
	CAUDIO_U8_t a_nThreadNum,
	CAUDIO_U8_t a_nChannelNumPerThread,
	AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel,
	CAUDIO_U32_t a_nFs,
	SyncMode_e a_eSyncMode,
	DATA_TYPE a_eAudioDataType,
	ChannelId_e a_eChannelId)
	: m_ProcessThread(NULL)
	, m_SrcDataEnoughEvent(NULL)
	, m_SrcExchangeList_CS(NULL)
	, m_pSrcMemoryPool(NULL)
	, m_nExchangeListEmptyInd(ALL_LIST_EMPTY_FLAG) // set all exchange list as empty
	, m_nProcessingListEmptyInd(ALL_LIST_EMPTY_FLAG) // set all processing list as empty
	, m_nAllChannelNotEmptyInd(ALL_LIST_EMPTY_FLAG)
	, m_nThreadNum((a_nThreadNum > MAX_INPUT_THREAD_NUM) ? MAX_INPUT_THREAD_NUM : a_nThreadNum)
	, m_nFs(a_nFs)
	, m_nChannelNumPerThread(a_nChannelNumPerThread)
	, m_nFrameTimeMs(a_nFrameTimeMsPerChannel*a_nChannelNumPerThread)
	, m_nFrameSize(FRAM_LEN(a_nFrameTimeMsPerChannel, a_nFs)*a_nChannelNumPerThread)
	, m_bStartProcess(false)
	, m_bIsInStopState(true)
	, m_pAudioModuleImpl(a_AudioModuleImpl)
	, m_eSyncMode(a_eSyncMode)
	, m_eAudioDataType(a_eAudioDataType)
	, m_bIsInitSuccess(false)
	, m_nMaxListSize(MAX_LIST_TIME_MS / m_nFrameTimeMs)
	, m_bIsListSizeNormal(true)
	, m_bHasEverPulled(false)
	, m_nReSampleBufferSize(0)
	, m_pReSampleBuffer(NULL)
	, m_eChannelId(a_eChannelId)

#if CAPTURE_PLAYBACK_INTEGRATED
	, m_nResampleBufferSize2(0)
	, m_pReSampleBuffer2(NULL)
#endif

{
	if(NULL==m_pAudioModuleImpl && MODE_PUSH_ASYNC_CALL==a_eSyncMode)
	{
		AUDIO_PROCESSING_PRINTF("m_pAudioModuleImpl must not be NULL when in MODE_PUSH_ASYNC_CALL mode");
		return;
	}

	for(CAUDIO_U8_t idx=0; idx<MAX_INPUT_THREAD_NUM; ++idx)
	{
		m_eDataSyncId[idx] = InvalidDS;
	}
}

// this API must be called after constructor is called
bool MultipleSourceSync::Init()
{
	if(Alloc())
	{
		m_bIsInitSuccess = true;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("Alloc members failed");
		m_bIsInitSuccess = false;
	}

	return m_bIsInitSuccess;
}

MultipleSourceSync::~MultipleSourceSync()
{
	// must reset the class to be init state before deleting memory pool
	// as some memory may still be cached in exchange list and 
	// processing list
	StopProcess();
	Reset();

	if(NULL != m_pSrcMemoryPool)
	{
		SrcSlice_MemoryPool::DeleteMemoryPool(m_pSrcMemoryPool);
		m_pSrcMemoryPool = NULL;
	}

	if (m_pReSampleBuffer)
	{
		delete m_pReSampleBuffer;
		m_pReSampleBuffer = NULL;
	}

	m_bIsInitSuccess = false;
}

ModuleState MultipleSourceSync::StartProcess()
{
	if(!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init not success, cannot start process");
		return Audio_Module_State_Dead;
	}

	if(!m_bIsInStopState)
	{
		// multiple times of calling of m_ProcessThread->Start
		// would cause resource leak
		AUDIO_PROCESSING_PRINTF("not in stop state, cannot start process");

		// for outside, this module is not running, should recall this API
		return 	Audio_Module_State_Inited;
	}
	m_bIsInStopState = false;

	Reset();
	if(m_ProcessThread.get())
	{
		// start processing thread in PUSH MODE
		CAUDIO_U32_t threadID(0);
		bool isSuccess = m_ProcessThread->Start(threadID);
		if(!isSuccess)
		{
			AUDIO_PROCESSING_PRINTF("start process thread failed");
			m_bIsInStopState = true;
			return Audio_Module_State_Inited;
		}
	}

	// start data processing
	// switch on data stream after resources being initialized
	m_bStartProcess = true;

	return Audio_Module_State_Running;
}

ModuleState MultipleSourceSync::StopProcess()
{
	// stop data processing
	// disconnect data stream before resources states being changed
	m_bStartProcess = false;

	// set the process not alive to make the process
	// can finish quickly
	if(NULL != m_ProcessThread.get())
	{
		m_ProcessThread->SetNotAlive();
	}

	m_SrcExchangeList_CS->Enter();
	if(NULL != m_SrcDataEnoughEvent.get())
	{
		// set event in PUSH MODE
		// the process may wait in Process(), set the event
		// to make it can finish quickly
		m_SrcDataEnoughEvent->Set();
	}
	m_SrcExchangeList_CS->Leave();

	if(m_ProcessThread.get())
	{
		// stop processing thread in PUSH MODE
		m_ProcessThread->Stop();
	}

	m_bIsInStopState = true;
	m_bHasEverPulled = false;
	return Audio_Module_State_Stopped;
}

//bool MultipleSourceSync::FeedData(	
//	const void* a_pData,
//	DATA_TYPE a_DataType,
//	CAUDIO_U32_t a_nSampleNum,
//	CAUDIO_U32_t a_nFs,
//	CAUDIO_U32_t a_nChannelNum,
//	CAUDIO_U8_t a_nThreadIdx)
//{
//	if(!m_bStartProcess)
//	{
//		AUDIO_PROCESSING_PRINTF("m_bStartProcess is false, feed data only when the process starts successfully");
//		return true;///
//	}
//
//	if(!m_bIsInitSuccess)
//	{
//		AUDIO_PROCESSING_PRINTF("init failed, will not feed any data");
//		return false;
//	}
//
//	// sanity check
//	assert(a_nChannelNum == m_nChannelNumPerThread);
//	if(NULL == a_pData || 0 == a_nSampleNum || (1 != a_nChannelNum && 2 != a_nChannelNum) ||
//		m_nThreadNum <= a_nThreadIdx)
//	{
//		AUDIO_PROCESSING_PRINTF("input param errors!");
//		return false;
//	}
//
//	//ExchangeListSizeCheck();
//
//	if(MODE_PUSH_ASYNC_CALL == m_eSyncMode)
//	{
//		return FeedData_PushMode(a_pData, a_DataType, a_nSampleNum,
//			a_nFs, a_nChannelNum, a_nThreadIdx);
//	}
//	else if(MODE_PUll_ASYNC_CALL == m_eSyncMode)
//	{
//		return FeedData_PullMode_AsyncCall(a_pData, a_DataType, a_nSampleNum,
//			a_nFs, a_nChannelNum, a_nThreadIdx);
//	}
//
//	return false;
//}
//
//bool MultipleSourceSync::FeedData_PushMode(	
//	const void* a_pData,
//	DATA_TYPE a_DataType,
//	CAUDIO_U32_t a_nSampleNum,
//	CAUDIO_U32_t a_nFs,
//	CAUDIO_U32_t a_nChannelNum,
//	CAUDIO_U8_t a_nThreadIdx)
//{
//	m_SrcExchangeList_CS->Enter();
//
//	//for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
//	//{
//	//	if(NULL != m_SrcExchangeList[i].get() && NULL != m_ProcessingList[i].get())
//	//	{
//	//		AUDIO_PRINTF("before feedData::thread %d, exchange list size:%d, process list size:%d", 
//	//			i, m_SrcExchangeList[i]->size()*m_nFrameSize, m_ProcessingList[i]->size()*m_nFrameSize);
//	//	}
//	//}
//
//	if(!FeedData_Common(a_pData, a_DataType, a_nSampleNum,
//		a_nFs, a_nChannelNum, a_nThreadIdx))
//	{
//		m_SrcExchangeList_CS->Leave();
//		return false;
//	}
//
//	if(IsExchListDataEnough())
//	{
//		// notify the processing thread that data in
//		// exchange list is enough for processing
//		m_SrcDataEnoughEvent->Set();
//	}
//
//	m_SrcExchangeList_CS->Leave();
//	return true;
//}
//
//bool MultipleSourceSync::FeedData_PullMode_AsyncCall(
//	const void* a_pData,
//	DATA_TYPE a_DataType,
//	CAUDIO_U32_t a_nSampleNum,
//	CAUDIO_U32_t a_nFs,
//	CAUDIO_U32_t a_nChannelNum,
//	CAUDIO_U8_t a_nThreadIdx)
//{
//	m_SrcExchangeList_CS->Enter();
//
//	if (!FeedData_Common(a_pData, a_DataType, a_nSampleNum,
//		a_nFs, a_nChannelNum, a_nThreadIdx))
//	{
//		m_SrcExchangeList_CS->Leave();
//		return false;
//	}
//
//	m_SrcExchangeList_CS->Leave();
//	return true;
//}
//
//bool MultipleSourceSync::FeedData_Common(
//	const void* a_pData,
//	DATA_TYPE a_DataType,
//	CAUDIO_U32_t a_nSampleNum,
//	CAUDIO_U32_t a_nFs,
//	CAUDIO_U32_t a_nChannelNum,
//	CAUDIO_U8_t a_nThreadIdx)
//{
//
//	// memory of out head buffer alloc inside AudioBufferManager
//	const void* pOutHeadBuffer = NULL;
//	CAUDIO_U32_t dataNeedSize = 0;
//	CAUDIO_U32_t outHeadDataSize = 0;
//	CAUDIO_U32_t outHeadBufferLen = 0;
//	CAUDIO_U32_t dataLeftSize = a_nSampleNum; // would be changed after bufferSrcData calling
//
//	// todo : not safe
//	const void** a_ppDataArray = &a_pData;  // only one channel for each AudioBufferManager instance
//
//	m_AudioBufferMgrPtr[a_nThreadIdx]->bufferSrcData(a_ppDataArray,
//		dataLeftSize, &pOutHeadBuffer, outHeadDataSize, outHeadBufferLen);
//
//	// copy data from out head buffer and input buffer
//	if (outHeadDataSize > 0)
//	{
//		// insert data in deficiency buffer into exchange list
//		SourceSlice_Ref* pSlice = AllocSourceSlice(pOutHeadBuffer, a_DataType,
//			outHeadDataSize, a_nFs, a_nChannelNum, a_nThreadIdx);
//		if (NULL == pSlice)
//		{
//			AUDIO_PROCESSING_PRINTF("alloc silice memory failed");
//			return false;
//		}
//
//		dataNeedSize = m_nFrameSize - outHeadDataSize;
//		pSlice->AppendData(a_pData, dataNeedSize);
//		dataLeftSize -= dataNeedSize;
//
//		// encapsulate the slice ref in a scoped ptr to 
//		// enable the memory of slice can be returned to
//		// the memory pool automatically
//		// todo: move this operation into func AllocSourceSlice
//		SourceSlice_RefPtr sliceRefPtr(pSlice);
//		m_SrcExchangeList[a_nThreadIdx]->push_back(sliceRefPtr);
//	}
//
//	// sanity check
//	if (0 != dataLeftSize % m_nFrameSize)
//	{
//		AUDIO_PROCESSING_PRINTF("input data len is abnormal, dataLeftSize:%d, m_nFrameSize:%d",
//			dataLeftSize, m_nFrameSize);
//	}
//
//	// add source data into exchange list
//	CAUDIO_U32_t loopNum = dataLeftSize / m_nFrameSize;
//	const CAUDIO_U8_t* pData = (CAUDIO_U8_t*)a_pData + DataType2Byte(a_DataType) * dataNeedSize;
//	for (CAUDIO_U32_t i = 0; i < loopNum; ++i)
//	{
//		// separate the input data into slices of |m_nFrameSize| len
//		SourceSlice_Ref* pSlice = AllocSourceSlice(pData, a_DataType, m_nFrameSize,
//			a_nFs, a_nChannelNum, a_nThreadIdx);
//		if (NULL == pSlice)
//		{
//			AUDIO_PROCESSING_PRINTF("alloc silice memory failed");
//			return false;
//		}
//
//		// encapsulate the slice ref in a scoped ptr to 
//		// enable the memory of slice can be returned to
//		// the memory pool automatically
//		// todo: move this operation into func AllocSourceSlice
//		SourceSlice_RefPtr sliceRefPtr(pSlice);
//		m_SrcExchangeList[a_nThreadIdx]->push_back(sliceRefPtr);
//
//		//update pointer to next slice of input data
//		pData += m_nFrameSize * DataType2Byte(a_DataType);
//	}
//
//	if (!m_SrcExchangeList[a_nThreadIdx]->empty())
//	{
//		SetExchangeListAsNotEmpty(a_nThreadIdx);
//	}
//
//	return true;
//}



bool MultipleSourceSync::FeedData(
	const void* a_pData,
	DATA_TYPE a_DataType,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nChannelNum,
	CAUDIO_U8_t a_nThreadIdx,
	CAUDIO_U32_t a_nOriginalSampleRate)
{
	if (!m_bStartProcess)
	{
		AUDIO_PROCESSING_PRINTF("m_bStartProcess is false, feed data only when the process starts successfully");
		return true;///
	}

	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init failed, will not feed any data");
		return false;
	}

	// sanity check
	assert(a_nChannelNum == m_nChannelNumPerThread);
	if (NULL == a_pData || 0 == a_nSampleNum || (1 != a_nChannelNum && 2 != a_nChannelNum) ||
		m_nThreadNum <= a_nThreadIdx)
	{
		AUDIO_PROCESSING_PRINTF("input param errors!");
		return false;
	}

	//ExchangeListSizeCheck();

	if (MODE_PUSH_ASYNC_CALL == m_eSyncMode)
	{
		return FeedData_PushMode(a_pData, a_DataType, a_nSampleNum,
			a_nOriginalSampleRate, a_nChannelNum, a_nThreadIdx);
	}
	else if (MODE_PUll_ASYNC_CALL == m_eSyncMode)
	{
		return FeedData_PullMode_AsyncCall(a_pData, a_DataType, a_nSampleNum,
			a_nOriginalSampleRate, a_nChannelNum, a_nThreadIdx);
	}

	return false;
}

bool MultipleSourceSync::FeedData_PushMode(
	const void* a_pData,
	DATA_TYPE a_DataType,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nOriginalSampleRate,
	CAUDIO_U32_t a_nChannelNum,
	CAUDIO_U8_t a_nThreadIdx)
{
	m_SrcExchangeList_CS->Enter();

	//for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	//{
	//	if(NULL != m_SrcExchangeList[i].get() && NULL != m_ProcessingList[i].get())
	//	{
	//		AUDIO_PRINTF("before feedData::thread %d, exchange list size:%d, process list size:%d", 
	//			i, m_SrcExchangeList[i]->size()*m_nFrameSize, m_ProcessingList[i]->size()*m_nFrameSize);
	//	}
	//}

	if (!FeedData_Common(a_pData, a_DataType, a_nSampleNum,
		a_nOriginalSampleRate, a_nChannelNum, a_nThreadIdx))
	{
		m_SrcExchangeList_CS->Leave();
		return false;
	}

	if (IsExchListDataEnough())
	{
		// notify the processing thread that data in
		// exchange list is enough for processing
		m_SrcDataEnoughEvent->Set();
	}

	m_SrcExchangeList_CS->Leave();
	return true;
}



bool MultipleSourceSync::FeedData_PullMode_AsyncCall(
	const void* a_pData,
	DATA_TYPE a_DataType,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nOriginalSampleRate,
	CAUDIO_U32_t a_nChannelNum,
	CAUDIO_U8_t a_nThreadIdx)
{
	m_SrcExchangeList_CS->Enter();

	if (!FeedData_Common(a_pData, a_DataType, a_nSampleNum,
		a_nOriginalSampleRate, a_nChannelNum, a_nThreadIdx))
	{
		m_SrcExchangeList_CS->Leave();
		return false;
	}

	m_SrcExchangeList_CS->Leave();
	return true;
}

bool MultipleSourceSync::FeedData_Common(
	const void* a_pData,
	DATA_TYPE a_DataType,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nOriginalSampleRate,
	CAUDIO_U32_t a_nChannelNum,
	CAUDIO_U8_t a_nThreadIdx)
{
	
	// memory of out head buffer alloc inside AudioBufferManager
	const void* pOutHeadBuffer = NULL;
	CAUDIO_U32_t dataNeedSize = 0;
	CAUDIO_U32_t outHeadDataSize = 0;
	CAUDIO_U32_t outHeadBufferLen = 0;
	CAUDIO_U32_t dataLeftSize = 0; // would be changed after bufferSrcData calling
	const void** pActualDataArray = NULL;
	const void*  pActualData = NULL;

	// resample 
	if (0 != a_nOriginalSampleRate)
	{
		CAUDIO_U32_t actual_len = Resample(a_pData, a_nSampleNum, m_nFs, a_nOriginalSampleRate, a_nThreadIdx);
		pActualData = (const void*)m_pReSampleBuffer;
		pActualDataArray = &pActualData;  // only one channel for each AudioBufferManager instance
		dataLeftSize = actual_len; // would be changed after bufferSrcData calling
	}
	else
	{
		pActualData = a_pData;
		pActualDataArray = &pActualData;
		dataLeftSize = a_nSampleNum;
	}

	m_AudioBufferMgrPtr[a_nThreadIdx]->bufferSrcData(pActualDataArray,
		dataLeftSize, &pOutHeadBuffer, outHeadDataSize, outHeadBufferLen);

	// copy data from out head buffer and input buffer
	if (outHeadDataSize > 0)
	{
		// insert data in deficiency buffer into exchange list
		SourceSlice_Ref* pSlice = AllocSourceSlice(pOutHeadBuffer, a_DataType,
			outHeadDataSize, m_nFs, a_nChannelNum, a_nThreadIdx);
		if (NULL == pSlice)
		{
			AUDIO_PROCESSING_PRINTF("alloc silice memory failed");
			return false;
		}

		dataNeedSize = m_nFrameSize - outHeadDataSize;
		pSlice->AppendData(pActualData, dataNeedSize);
		dataLeftSize -= dataNeedSize;

		// encapsulate the slice ref in a scoped ptr to 
		// enable the memory of slice can be returned to
		// the memory pool automatically
		// todo: move this operation into func AllocSourceSlice
		SourceSlice_RefPtr sliceRefPtr(pSlice);
		m_SrcExchangeList[a_nThreadIdx]->push_back(sliceRefPtr);
	}

	// sanity check
	if (0 != dataLeftSize % m_nFrameSize)
	{
		AUDIO_PROCESSING_PRINTF("input data len is abnormal, dataLeftSize:%d, m_nFrameSize:%d",
			dataLeftSize, m_nFrameSize);
	}

	// add source data into exchange list
	CAUDIO_U32_t loopNum = dataLeftSize / m_nFrameSize;
	const CAUDIO_U8_t* pData = (CAUDIO_U8_t*)pActualData + DataType2Byte(a_DataType) * dataNeedSize;

	for (CAUDIO_U32_t i = 0; i < loopNum; ++i)
	{
		// separate the input data into slices of |m_nFrameSize| len
		SourceSlice_Ref* pSlice = AllocSourceSlice(pData, a_DataType, m_nFrameSize,
			m_nFs, a_nChannelNum, a_nThreadIdx);
		if (NULL == pSlice)
		{
			AUDIO_PROCESSING_PRINTF("alloc silice memory failed");
			return false;
		}

		// encapsulate the slice ref in a scoped ptr to 
		// enable the memory of slice can be returned to
		// the memory pool automatically
		// todo: move this operation into func AllocSourceSlice
		SourceSlice_RefPtr sliceRefPtr(pSlice);
		m_SrcExchangeList[a_nThreadIdx]->push_back(sliceRefPtr);

		//update pointer to next slice of input data
		pData += m_nFrameSize * DataType2Byte(a_DataType);
	}

	if (!m_SrcExchangeList[a_nThreadIdx]->empty())
	{
		SetExchangeListAsNotEmpty(a_nThreadIdx);
	}

	return true;
}

CAUDIO_U32_t MultipleSourceSync::Resample(
	const void* a_pData,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nDefaultSampleRate,
	CAUDIO_U32_t a_nOriginalSampleRate,
	CAUDIO_U8_t a_nThreadIdx)
{
	CAUDIO_U32_t nActualLen = 0;
	CAUDIO_U32_t nResampleLen = 0;
	AUDIO_DATA_TYPE fResampleLen = 0.f;

	const AUDIO_DATA_TYPE* data = reinterpret_cast<const AUDIO_DATA_TYPE*>(a_pData);
	fResampleLen = static_cast<AUDIO_DATA_TYPE>(a_nSampleNum)/a_nOriginalSampleRate*a_nDefaultSampleRate;
	nResampleLen = fResampleLen > static_cast<CAUDIO_U32_t>(fResampleLen) 
					? static_cast<CAUDIO_U32_t>(fResampleLen) + 1 
					: static_cast<CAUDIO_U32_t>(fResampleLen);

#if CAPTURE_PLAYBACK_INTEGRATED
	if (-1 == a_nThreadIdx)
	{
		AllocReSampleBuff_CapturePlaybackIntegrated(nResampleLen);
		m_resample2->Process(
			data, 
			m_pReSampleBuffer2, 
			a_nSampleNum, 
			nResampleLen,
			a_nOriginalSampleRate, 
			a_nDefaultSampleRate, 
			nActualLen);
	}
	else
	{
		AllocReSampleBuff(nResampleLen);
		m_resample[a_nThreadIdx]->Process(
			data, 
			m_pReSampleBuffer, 
			a_nSampleNum, 
			nResampleLen, 
			a_nOriginalSampleRate, 
			a_nDefaultSampleRate, 
			nActualLen);
	}
#else
	AllocReSampleBuff(nResampleLen);
	m_resample[a_nThreadIdx]->Process(
		data, 
		m_pReSampleBuffer, 
		a_nSampleNum, 
		nResampleLen, 
		a_nOriginalSampleRate, 
		a_nDefaultSampleRate, 
		nActualLen);
#endif

	return nActualLen;
}

bool MultipleSourceSync::AllocReSampleBuff(CAUDIO_U32_t resample_len)
{
	if (NULL != m_pReSampleBuffer && m_nReSampleBufferSize == resample_len)
	{
		return true;
	}
	if (NULL != m_pReSampleBuffer)
	{
		delete[] m_pReSampleBuffer;
		m_pReSampleBuffer = NULL;
	}
	m_pReSampleBuffer = new AUDIO_DATA_TYPE[resample_len];
	if (NULL == m_pReSampleBuffer)
	{
		AUDIO_PROCESSING_PRINTF("alloc resample buffer failed!");
		return false;
	}
	memset(m_pReSampleBuffer, 0, sizeof(AUDIO_DATA_TYPE)*resample_len);
	m_nReSampleBufferSize = resample_len;

	return true;
}

// pull one frame per thread at one call
bool MultipleSourceSync::PullData(
	AudioFrame* a_AudioFrameArray, 
	CAUDIO_U8_t a_nArrayLen,
	CAUDIO_U8_t& a_nThreadNum,
	bool& a_bIsDataPulledOut)
{
	m_bHasEverPulled = true;
	a_bIsDataPulledOut = false;
	a_nThreadNum = m_nThreadNum;

	if(!m_bStartProcess)
	{
		AUDIO_PROCESSING_PRINTF("m_bStartProcess is false, return directly without any data pulled");
		return true;
	}

	if(a_nArrayLen < m_nThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("a_nArrayLen:%d is too small for m_nThreadNum:%d", a_nArrayLen, a_nThreadNum);
		return false;
	}
	
	//ProcessingListSizeCheck();

	// copy slice to processing list firstly
	CopySliceToProcessingList();
	CAUDIO_U32_t checkSize = GetMaxProcessingListSize();
	if (0 == checkSize)
	{
		// give another chance, would block the outside thread for MAX_TIME_WAIT_FOR_DATA_FROM_PULL_MODE ms 
		// to wait for enough data, if time exceeds, would generate zero data
		const webrtc::EventTypeWrapper ret = m_SrcDataEnoughEvent->Wait(MAX_TIME_WAIT_FOR_DATA_FROM_PULL_MODE);
		if(ret == webrtc::kEventError)
		{
			AUDIO_PROCESSING_PRINTF("m_SrcDataEnoughEvent error");
			//return false;
		}
		
		m_SrcDataEnoughEvent->Reset(); // reset the event
		CopySliceToProcessingList();	//require data from exchange list again
		checkSize = GetMaxProcessingListSize();
		AUDIO_PROCESSING_PRINTF("channel id is %d, try to copy data from exchange list again after 3 ms,\
								 checkSize(max size in processing lists):%d", m_eChannelId, checkSize);
	}


	ExchangeListSizeCheck();

	// reset all the frames to zero firstly anyway
	if (!AllThreadGenerateZeroData(a_AudioFrameArray, a_nArrayLen))
	{
		return false;
	}

	CAUDIO_U32_t pullFrameNumPerThread = 1;
	if (checkSize > 0)
	{
		// some threads may generate zero data as some processing lists may be empty
		if(!PassSliceToProcessingModule(pullFrameNumPerThread, const_cast<const AudioFrame**>(&a_AudioFrameArray)))
		{
			return false;
		}
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("channel id is %d, not any data in processing list, would generate zero data.", m_eChannelId);
		//if (!AllThreadGenerateZeroData(a_AudioFrameArray, a_nArrayLen))
		//{
		//	return false;
		//}
	}

	ProcessingListSizeCheck();
	a_bIsDataPulledOut = true;
	return true;
}

#if CAPTURE_PLAYBACK_INTEGRATED
bool MultipleSourceSync::PullData_CapturePlaybackIntegrated(
	AudioFrame*  pAudioFrameNeeded,
	const CAUDIO_U32_t nMaxFrameSize,
	const CAUDIO_U32_t nSamplesPerSec,
	const CAUDIO_U8_t nChannel)
{
	CAUDIO_U32_t nCheckSize = 0;
	CAUDIO_U32_t nActualSize = 0;
	CAUDIO_U32_t nResampleSize = 0;
	void* pResampleData = NULL;

	//safety inspection
	if (!m_bStartProcess)
	{
		AUDIO_PROCESSING_PRINTF("m_bStartProcess is false, return directly without any data pulled.");
		return true;
	}
	if (NULL == pAudioFrameNeeded)
	{
		AUDIO_PROCESSING_PRINTF("NULL == pAudioFrameNeeded.");
		return false;
	}
	if (nMaxFrameSize < m_nThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("nMaxFrameSize < m_nThreadNum.");
		return false;
	}

	// copy slice to processing list firstly
	CopySliceToProcessingList();
	nCheckSize = GetMaxProcessingListSize();
	
	if (0 == nCheckSize)
	{
		// give another chance, would block the outside thread for MAX_TIME_WAIT_FOR_DATA_FROM_PULL_MODE ms 
		// to wait for enough data, if time exceeds, would generate zero data
		const webrtc::EventTypeWrapper ret = m_SrcDataEnoughEvent->Wait(MAX_TIME_WAIT_FOR_DATA_FROM_PULL_MODE);
		if (ret == webrtc::kEventError)
		{
			AUDIO_PROCESSING_PRINTF("m_SrcDataEnoughEvent error");
			//return false;
		}

		m_SrcDataEnoughEvent->Reset(); // reset the event
		CopySliceToProcessingList();	//require data from exchange list again
		nCheckSize = GetMaxProcessingListSize();

	}
	ExchangeListSizeCheck();

	// reset all the frames to zero firstly anyway
	if (!AllThreadGenerateZeroData(pAudioFrameNeeded, nMaxFrameSize))
	{
		return false;
	}
	
	if (nCheckSize >= 0)
	{
		// some threads may generate zero data as some processing lists may be empty
		if (!PassSlice_CapturePlaybackIntegrated(pAudioFrameNeeded))
		{
			return false;
		}
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("channel id is %d, not any data in processing list, would generate zero data.", m_eChannelId);
	}
	ProcessingListSizeCheck();

	//resample 
	pResampleData = pAudioFrameNeeded[0].data_;
	nResampleSize = pAudioFrameNeeded[0].AudioPara_.samples_per_channel_;
	if (nSamplesPerSec != m_nFs)
	{
		nActualSize = Resample(pResampleData, nResampleSize, nSamplesPerSec, m_nFs);
	}
	else
	{
		AllocReSampleBuff_CapturePlaybackIntegrated(nResampleSize);
		memcpy_s(m_pReSampleBuffer2, 
			m_nResampleBufferSize2*DataType2Byte(m_eAudioDataType), 
			pResampleData, 
			nResampleSize*DataType2Byte(m_eAudioDataType));
		nActualSize = nResampleSize;
	}

	// TODO: remove copy operation if we want to optimize
	// copy data back to pAudioFrameNeeded
	for (CAUDIO_U32_t i = 0; i < nActualSize; ++i)
	{
		for (CAUDIO_U32_t j = 0; j < nChannel; ++j)
		{
			reinterpret_cast<AUDIO_DATA_TYPE*>(pResampleData)[i*nChannel + j] = m_pReSampleBuffer2[i];
		}
	}
	pAudioFrameNeeded[0].AudioPara_.samples_per_channel_ = nActualSize;
	pAudioFrameNeeded[0].AudioPara_.num_channels_ = nChannel;

	return true;
}

bool MultipleSourceSync::PassSlice_CapturePlaybackIntegrated(
	AudioFrame* a_AudioFrameArray)
{
	CAUDIO_U8_t threadIdx = 0;
	SourceSlice_RefPtr srcSlicePtr[MAX_INPUT_THREAD_NUM];

	if (NULL == a_AudioFrameArray)
	{
		return false;
	}
	if (!m_bStartProcess)
	{
		// stop() is called when m_bStartProcess is false
		// break to end processing thread more quickly
		// the processing thread must end before start() is called
		return true;
	}	
	for (CAUDIO_U8_t i = 0; i < m_nThreadNum; ++i)
	{
		if (0 == m_ProcessingList[i]->size())
		{
			// the processing list size would be zero in MODE_PUll_ASYNC_CALL
			continue;
		}
		srcSlicePtr[i] = m_ProcessingList[i]->front();
		if (NULL == srcSlicePtr[i].get())
		{
			AUDIO_PROCESSING_PRINTF("src slice ptr is NULL!");
			return false;
		}
		threadIdx = srcSlicePtr[i]->GetThreadIdx();
		assert(threadIdx == i);
		const AudioFrame& audioFrame = srcSlicePtr[i]->getAudioFrame();
		a_AudioFrameArray[i].CopyFrom(audioFrame);
	}
	// process source slice
	if (NULL != m_pAudioModuleImpl)
	{
		// todo: define a return type of processData with multiple values to identify if the frame is processed
		// or unprocessed or processed with error
		if (!m_pAudioModuleImpl->processData(a_AudioFrameArray, m_nThreadNum))
		{
			AUDIO_PROCESSING_PRINTF("the frame is unprocessed");//, would be remained in processing list.");
			//return true;
		}
	}

	for (CAUDIO_U8_t j = 0; j < m_nThreadNum; ++j)
	{
		if (!m_ProcessingList[j]->empty())
		{
			m_ProcessingList[j]->pop_front();
		}
	}

	return true;
}

bool MultipleSourceSync::AllocReSampleBuff_CapturePlaybackIntegrated(CAUDIO_U32_t resample_len)
{
	if (NULL != m_pReSampleBuffer2 && m_nResampleBufferSize2 == resample_len)
	{
		return true;
	}
	if (NULL != m_pReSampleBuffer2)
	{
		delete[] m_pReSampleBuffer2;
		m_pReSampleBuffer2 = NULL;
	}
	m_pReSampleBuffer2 = new AUDIO_DATA_TYPE[resample_len];
	if (NULL == m_pReSampleBuffer2)
	{
		AUDIO_PROCESSING_PRINTF("alloc resample buffer failed!");
		return false;
	}
	memset(m_pReSampleBuffer2, 0, sizeof(AUDIO_DATA_TYPE)*resample_len);
	m_nResampleBufferSize2 = resample_len;

	return true;
}
#endif


// nested class func start

void MultipleSourceSync::SourceSlice_Ref::SetData(
	const void* a_pData,
	DATA_TYPE a_DataType,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nFs,
	CAUDIO_U32_t a_nChannelNum,
	DataSyncId_e a_eDataSyncId)
{
	AUDIO_FRAME_PARA audioFrameParam;
	audioFrameParam.id_ = 0;
	audioFrameParam.timestamp_ = 0;
	CAUDIO_U32_t samplesPerChannel = a_nSampleNum/a_nChannelNum;
	audioFrameParam.samples_per_channel_ = samplesPerChannel;
	audioFrameParam.sample_rate_hz_ = a_nFs;
	audioFrameParam.num_channels_ = a_nChannelNum;
	audioFrameParam.speech_type_ = kNormalSpeech;
	audioFrameParam.vad_activity_ = kVadUnknown;
	audioFrameParam.energy_ = -1;
	audioFrameParam.datatype_ = a_DataType;
	audioFrameParam.dataSyncId_ = a_eDataSyncId;
	m_AudioFrame.UpdateFrame(audioFrameParam, static_cast<const CAUDIO_U8_t*>(a_pData));
}

void MultipleSourceSync::SourceSlice_Ref::AppendData(
	const void* a_pData,
	CAUDIO_U32_t a_nSampleNum)
{
	CAUDIO_U32_t appendLength = a_nSampleNum * DataType2Byte(m_AudioFrame.AudioPara_.datatype_);
	CAUDIO_U32_t frameLen = m_AudioFrame.AudioPara_.samples_per_channel_ 
		* m_AudioFrame.AudioPara_.num_channels_ * DataType2Byte(m_AudioFrame.AudioPara_.datatype_);

	if(frameLen + appendLength > m_AudioFrame.kMaxDataSizeSamples)
	{
		AUDIO_PROCESSING_PRINTF("no enough memory to store data, frameLen:%d, appendLength:%d", frameLen, appendLength);
		return;
	}

	memcpy_s(m_AudioFrame.data_ + frameLen, m_AudioFrame.kMaxDataSizeSamples - frameLen, a_pData, appendLength);
	m_AudioFrame.AudioPara_.samples_per_channel_ += a_nSampleNum / m_AudioFrame.AudioPara_.num_channels_;
}

void MultipleSourceSync::SourceSlice_Ref::Init(
	SrcSlice_MemoryPool* a_pMemoryPool,
	const void* a_pData,
	DATA_TYPE a_DataType,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nFs,
	CAUDIO_U32_t a_nChannelNum,
	CAUDIO_U8_t a_nThreadIdx,
	DataSyncId_e a_eDataSyncId)
{
	assert(NULL != a_pMemoryPool);
	Reset();

	m_pMemoryPool = a_pMemoryPool;
	m_nThreadIdx = a_nThreadIdx;
	SetData(a_pData, a_DataType, a_nSampleNum,
		a_nFs, a_nChannelNum, a_eDataSyncId);
}

// Release a reference. Will delete the object if the reference count
// reaches zero.
CAUDIO_S32_t MultipleSourceSync::SourceSlice_Ref::Release() 
{
	CAUDIO_S32_t ref_count;
	ref_count = --m_nRefCount;
	if (0 == ref_count)
	{
		// return memory to the pool
		if(NULL != m_pMemoryPool)
		{
			SourceSlice_Ref* pThis = this;
			m_pMemoryPool->PushMemory(pThis);
		}
		else
		{
			AUDIO_PROCESSING_PRINTF("m_pMemoryPool is NULL!");
		}
	}
	return ref_count;
}

// Add a reference.
CAUDIO_S32_t MultipleSourceSync::SourceSlice_Ref::AddRef() 
{ 
	return ++m_nRefCount; 
}

void MultipleSourceSync::SourceSlice_Ref::Reset()
{
	m_AudioFrame.Reset();
	m_nThreadIdx = kInvalidThreadIdx;
}


////////////////////////////////////////////////////////////// nested class func end


bool MultipleSourceSync::ProcessThreadFunction(ThreadObj threadObj)
{
	return static_cast<MultipleSourceSync*>(threadObj)->Process();
}

bool MultipleSourceSync::Process()
{
	if(!m_bStartProcess)
	{
		// wait for 2 ms to avoid too many loops which would consume much CPU resource
		//AUDIO_PROCESSING_PRINTF("m_bStartProcess is false, wait for 2 ms and return");
		
		const webrtc::EventTypeWrapper ret = m_SrcDataEnoughEvent->Wait(2);

		m_SrcExchangeList_CS->Enter();
		m_SrcDataEnoughEvent->Reset();
		m_SrcExchangeList_CS->Leave();

		//AUDIO_PROCESSING_PRINTF("process return");
		return true;
	}

	//AUDIO_PRINTF("--------------------------------end process----------------------------");
	const webrtc::EventTypeWrapper ret = m_SrcDataEnoughEvent->Wait(MAX_SRC_EXCHANGE_EVENT_WAIT_TIME);
	if(ret == webrtc::kEventError)
	{
		AUDIO_PROCESSING_PRINTF("m_SrcDataEnoughEvent error");
		return false;
	}
	else if(ret == webrtc::kEventTimeout)
	{
		AUDIO_PROCESSING_PRINTF("m_SrcDataEnoughEvent timeout");
		return false;
	}

	//AUDIO_PRINTF("--------------------------------start process----------------------------");

	//ProcessingListSizeCheck();

	// todo: from start process to pull data need 7 ms, need to check this issue
	CAUDIO_U32_t minExchangeListSize = CopySliceToProcessingList();

	ExchangeListSizeCheck();

	if(0 == minExchangeListSize)
	{
		//AUDIO_PROCESSING_PRINTF("minimum echange list size is 0, return directly");
		return true;
	}

	const AudioFrame* ppSrcBuffer[MAX_INPUT_THREAD_NUM];
	if(!PassSliceToProcessingModule(minExchangeListSize, ppSrcBuffer))
	{
		return true;
	}

	ProcessingListSizeCheck();
	return true;
}

MultipleSourceSync::SourceSlice_Ref* MultipleSourceSync::AllocSourceSlice(
	const void* a_pData,
	DATA_TYPE a_DataType,
	CAUDIO_U32_t a_nSampleNum,
	CAUDIO_U32_t a_nFs,
	CAUDIO_U32_t a_nChannelNum,
	CAUDIO_U8_t a_nThreadIdx)
{
	MultipleSourceSync::SourceSlice_Ref* pSrcSlice = NULL;
	m_pSrcMemoryPool->PopMemory(pSrcSlice);

	if(NULL != pSrcSlice)
	{
		pSrcSlice->Init(m_pSrcMemoryPool, a_pData, a_DataType,
			a_nSampleNum, a_nFs, a_nChannelNum, a_nThreadIdx, m_eDataSyncId[a_nThreadIdx]);
	}

	return pSrcSlice;
}

void MultipleSourceSync::SetExchangeListAsEmpty(CAUDIO_U8_t a_nChannelIdex)
{
	// set corresponding bit as 0
	m_nExchangeListEmptyInd = m_nExchangeListEmptyInd & ~(LIST_EMPTY_IND_BIT<<a_nChannelIdex);
}

void MultipleSourceSync::SetExchangeListAsNotEmpty(CAUDIO_U8_t a_nChannelIdex)
{
	// set corresponding bit as 1
	m_nExchangeListEmptyInd = m_nExchangeListEmptyInd | (LIST_EMPTY_IND_BIT<<a_nChannelIdex);
}

void MultipleSourceSync::SetProcessingListAsEmpty(CAUDIO_U8_t a_nChannelIdex)
{
	// set corresponding bit as 0
	m_nProcessingListEmptyInd = m_nProcessingListEmptyInd & ~(LIST_EMPTY_IND_BIT<<a_nChannelIdex);
}

void MultipleSourceSync::SetProcessingListAsNotEmpty(CAUDIO_U8_t a_nChannelIdex)
{
	// set corresponding bit as 1
	m_nProcessingListEmptyInd = m_nProcessingListEmptyInd | (LIST_EMPTY_IND_BIT<<a_nChannelIdex);
}

bool MultipleSourceSync::IsExchListDataEnough()
{
	return m_nAllChannelNotEmptyInd == m_nExchangeListEmptyInd;
}

CAUDIO_U32_t MultipleSourceSync::GetMinExchangeListSize()
{
	CAUDIO_U32_t minSize = m_SrcExchangeList[0]->size();
	CAUDIO_U32_t size = 0;
	for (CAUDIO_U8_t i = 1; i < m_nThreadNum; ++i)
	{
		size = m_SrcExchangeList[i]->size();
		if (size < minSize)
		{
			minSize = size;
		}
	}

	return minSize;
}

CAUDIO_U32_t MultipleSourceSync::GetMaxExchangeListSize()
{
	CAUDIO_U32_t maxSize = m_SrcExchangeList[0]->size();
	CAUDIO_U32_t size = 0;
	for (CAUDIO_U8_t i = 1; i < m_nThreadNum; ++i)
	{
		size = m_SrcExchangeList[i]->size();
		if (size > maxSize)
		{
			maxSize = size;
		}
	}

	return maxSize;
}

CAUDIO_U32_t MultipleSourceSync::GetMaxProcessingListSize()
{
	CAUDIO_U32_t maxSize = m_ProcessingList[0]->size();
	CAUDIO_U32_t size = 0;
	for (CAUDIO_U8_t i = 1; i < m_nThreadNum; ++i)
	{
		size = m_ProcessingList[i]->size();
		if (size > maxSize)
		{
			maxSize = size;
		}
	}

	return maxSize;
}

// need not to add m_SrcExchangeList_CS as this func is
// only called in startProcess
void MultipleSourceSync::ClearList()
{
	for (CAUDIO_U8_t threadIdx = 0; threadIdx < MAX_INPUT_THREAD_NUM; ++threadIdx)
	{
		if (NULL != m_SrcExchangeList[threadIdx].get())
		{
			while (!m_SrcExchangeList[threadIdx]->empty())
			{
				m_SrcExchangeList[threadIdx]->pop_front();
			}
		}

		if (NULL != m_ProcessingList[threadIdx].get())
		{
			while (!m_ProcessingList[threadIdx]->empty())
			{
				m_ProcessingList[threadIdx]->pop_front();
			}
		}
	}

	m_nExchangeListEmptyInd = ALL_LIST_EMPTY_FLAG;
	m_nAllChannelNotEmptyInd = ALL_LIST_EMPTY_FLAG;
}

CAUDIO_U32_t MultipleSourceSync::CopySliceToProcessingList()
{
	m_SrcExchangeList_CS->Enter();

	CAUDIO_U32_t checkSize = 0;
	CAUDIO_U32_t copySize = 0;
	if (MODE_PUSH_ASYNC_CALL == m_eSyncMode)
	{
		// check size if min size
		// copy sizes are the same for all threads in MODE_PUSH_ASYNC_CALL
		checkSize = GetMinExchangeListSize();
		copySize = checkSize;
	}
	else if (MODE_PUll_ASYNC_CALL == m_eSyncMode)
	{
		// check size if max size
		checkSize = GetMaxExchangeListSize();
	}

	if (0 == checkSize)
	{
		//AUDIO_PROCESSING_PRINTF("checkSize is 0, return directly");
		m_SrcExchangeList_CS->Leave();
		return 0;
	}

	for (CAUDIO_U8_t i = 0; i < m_nThreadNum; ++i)
	{
		CAUDIO_U32_t copyIdx = 0;
		if (MODE_PUll_ASYNC_CALL == m_eSyncMode)
		{
			// copy size changes for per thread in MODE_PUll_ASYNC_CALL
			copySize = m_SrcExchangeList[i]->size();
			if (0 == copySize)
			{
				continue;
			}
		}

		// the reference count would be changed automatically after 
		// object is popped and pushed
		for (; copyIdx<copySize; ++copyIdx)
		{
			m_ProcessingList[i]->push_back(m_SrcExchangeList[i]->front());
			m_SrcExchangeList[i]->pop_front();
		}

		if (MODE_PUSH_ASYNC_CALL==m_eSyncMode && m_SrcExchangeList[i]->empty())
		{
			SetExchangeListAsEmpty(i);
		}
		else if(m_SrcExchangeList[i]->size() > m_nMaxListSize)
		{
			// indicate list size has been abnormal,
			// this multiple sync needs a restart
			// todo: restart logic is not implemented yet
			m_bIsListSizeNormal = false;
		}
	}

	// reset event in PUSH MODE
	if(MODE_PUSH_ASYNC_CALL == m_eSyncMode)
	{
		m_SrcDataEnoughEvent->Reset();
	}

	m_SrcExchangeList_CS->Leave();
	return checkSize;
}

bool MultipleSourceSync::PassSliceToProcessingModule(CAUDIO_U32_t a_nPassSize, const AudioFrame** a_AudioFrameArray)
{
	// for pull mode, the value of a_nPassSize should be 1
	AudioFrame* pPulledFrameArray = const_cast<AudioFrame*>(*a_AudioFrameArray);
	if(	( MODE_PUll_ASYNC_CALL==m_eSyncMode && NULL==pPulledFrameArray)
		|| (MODE_PUll_ASYNC_CALL==m_eSyncMode && 1!=a_nPassSize)
		|| NULL == a_AudioFrameArray )
	{
		return false;
	}

	const AUDIO_DATA_TYPE* pData = NULL;
	CAUDIO_U32_t dataSize = 0;
	CAUDIO_U8_t threadIdx = 0;
	for(CAUDIO_U32_t i=0; i<a_nPassSize; ++i)
	{
		if(!m_bStartProcess)
		{
			// stop() is called when m_bStartProcess is false
			// break to end processing thread more quickly
			// the processing thread must end before start() is called
			break;
		}

		SourceSlice_RefPtr srcSlicePtr[MAX_INPUT_THREAD_NUM];
		for(CAUDIO_U8_t j=0; j<m_nThreadNum; ++j)
		{
			if (MODE_PUll_ASYNC_CALL == m_eSyncMode)
			{
				if (0 == m_ProcessingList[j]->size())
				{
					// the processing list size would be zero in MODE_PUll_ASYNC_CALL
					continue;
				}
			}

			srcSlicePtr[j] = m_ProcessingList[j]->front();
			if(NULL == srcSlicePtr[j].get())
			{
				AUDIO_PROCESSING_PRINTF("src slice ptr is NULL!");
				return true;
			}

			threadIdx = srcSlicePtr[j]->GetThreadIdx();
			assert(threadIdx==j);

			const AudioFrame& audioFrame = srcSlicePtr[j]->getAudioFrame();
			if(MODE_PUSH_ASYNC_CALL == m_eSyncMode)
			{
				a_AudioFrameArray[j] = &audioFrame;
			}
			else if(MODE_PUll_ASYNC_CALL == m_eSyncMode)
			{
				pPulledFrameArray[j].CopyFrom(audioFrame);
			}
		}

		// process source slice
		if(MODE_PUSH_ASYNC_CALL == m_eSyncMode)
		{
			if(NULL != m_pAudioModuleImpl)
			{
				// todo: define a return type of processData with multiple values to identify if the frame is processed
				// or unprocessed or processed with error
				if(!m_pAudioModuleImpl->processData(a_AudioFrameArray, m_nThreadNum))
				{
					AUDIO_PROCESSING_PRINTF("the frame is unprocessed");//, would be remained in processing list.");
					//return true;
				}
			}
		}

		for(CAUDIO_U8_t j=0; j<m_nThreadNum; ++j)
		{
			if(!m_ProcessingList[j]->empty())
			{
				m_ProcessingList[j]->pop_front();
			}
		}
	}

	//for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	//{
		//if(NULL != m_SrcExchangeList[i].get() && NULL != m_ProcessingList[i].get())
		//{
			//int size = m_SrcExchangeList[i]->size();
			//AUDIO_PRINTF("process data::thread %d, exchange list size:%d, process list size:%d, processd size:%d", 
				//i, m_SrcExchangeList[i]->size()*m_nFrameSize, m_ProcessingList[i]->size()*m_nFrameSize, a_nMinExchangeListSize*m_nFrameSize);
		//}
	//}

	return true;
}


void MultipleSourceSync::GetAudioFrameParam(AUDIO_FRAME_PARA& a_Param, CAUDIO_U8_t a_nThreadIdx)
{
	a_Param.id_ = 0;
	a_Param.timestamp_ = 0;
	CAUDIO_U32_t samplesPerChannel = m_nFrameSize/m_nChannelNumPerThread;
	a_Param.samples_per_channel_ = samplesPerChannel;
	a_Param.sample_rate_hz_ = m_nFs;
	a_Param.num_channels_ = m_nChannelNumPerThread;
	a_Param.speech_type_ = kNormalSpeech;
	a_Param.vad_activity_ = kVadUnknown;
	a_Param.energy_ = -1;
	a_Param.datatype_ = m_eAudioDataType;
	a_Param.dataSyncId_ = m_eDataSyncId[a_nThreadIdx];
}

bool MultipleSourceSync::AllThreadGenerateZeroData(
	AudioFrame a_AudioFrameArray[], 
	CAUDIO_U8_t a_nArrayLen)
{
	if(a_nArrayLen < m_nThreadNum)
	{
		return false;
	}

	for(CAUDIO_U8_t idx=0; idx<m_nThreadNum; ++idx)
	{
		AUDIO_FRAME_PARA frameParam;
		GetAudioFrameParam(frameParam, idx);
		a_AudioFrameArray[idx].UpdateFrame(frameParam, NULL);
	}

	return true;
}

void MultipleSourceSync::ExchangeListSizeCheck()
{
	m_SrcExchangeList_CS->Enter();

	for(CAUDIO_U8_t idx=0; idx<m_nThreadNum; ++idx)
	{
		//AUDIO_PROCESSING_PRINTF("this:%p, exchange list %d's size:%d, max size:%d", 
		//	this, idx, m_SrcExchangeList[idx]->size(), m_nMaxListSize);

		if(m_SrcExchangeList[idx]->size() > m_nMaxListSize)
		{
			// m_bIsListSizeNormal = false;
			AUDIO_PROCESSING_PRINTF("channel id is %d, clear exchange list", m_eChannelId);
			while(!m_SrcExchangeList[idx]->empty())
			{
				m_SrcExchangeList[idx]->pop_front();
			}
		}
	}

	m_SrcExchangeList_CS->Leave();
}

void MultipleSourceSync::ProcessingListSizeCheck()
{
	for(CAUDIO_U8_t idx=0; idx<m_nThreadNum; ++idx)
	{
		//AUDIO_PROCESSING_PRINTF("this:%p, processing list %d's size:%d, max size:%d", 
		//	this, idx, m_ProcessingList[idx]->size(), m_nMaxListSize);

		if(m_ProcessingList[idx]->size() > m_nMaxListSize)
		{
			//m_bIsListSizeNormal = false;
			AUDIO_PROCESSING_PRINTF("channel id is %d, clear processing list", m_eChannelId);
			while(!m_ProcessingList[idx]->empty())
			{
				m_ProcessingList[idx]->pop_front();
			}
		}
	}
}

bool MultipleSourceSync::Alloc()
{
	// alloc audio buffer manager
	for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	{
		// alloc audio buffer manager for each channel
		m_AudioBufferMgrPtr[i].reset(new AudioBufferManager());
		if(NULL == m_AudioBufferMgrPtr[i].get())
		{
			return false;
		}
	}

	// init AudioBufferManager
	AudioBufferParam_t param;
	param.mLenOfSample = DataType2Byte(m_eAudioDataType);
	
	param.mPostProcessBufferLen 
		= 100 * m_nFs  * param.mLenOfSample / 1000;

	param.mFrameShiftSize = m_nFrameSize;
	param.mInChannelNum = 1;
	param.mOutChannelNum = 1;

	if (MODE_PUSH_ASYNC_CALL == m_eSyncMode)
	{
		param.mInitDelaySize = m_nFrameSize/2;
	}
	else if (MODE_PUll_ASYNC_CALL == m_eSyncMode)
	{
		param.mInitDelaySize = 0;
	}

	for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	{
		if(!m_AudioBufferMgrPtr[i]->init(param))
		{
			AUDIO_PROCESSING_PRINTF("init Audio Buffer Manager failed");
			return false;
		}
	}

	// alloc memory pool
	SrcSlice_MemoryPool::CreateMemoryPool(m_pSrcMemoryPool, SRC_SLICE_MEMORY_POOL_INITIAL_SIZE);
	if(NULL == m_pSrcMemoryPool)
	{
		AUDIO_PROCESSING_PRINTF("create memory pool failed");
		return false;
	}

	if (MODE_PUSH_ASYNC_CALL==m_eSyncMode && !AllocPushMode())
	{
		AUDIO_PROCESSING_PRINTF("AllocPushMode failed");
		return false;
	}
	else if (MODE_PUll_ASYNC_CALL==m_eSyncMode && !AllocPullModeAsyncCall())
	{
		AUDIO_PROCESSING_PRINTF("AllocPullModeAsyncCall failed");
		return false;
	}
	else if(MODE_PUll_SYNC_CALL==m_eSyncMode)
	{
		// todo
	}

	// alloc audio resample 
	for (CAUDIO_U8_t i = 0; i < m_nThreadNum; ++i)
	{
		m_resample[i].reset(new ReSampleImpl(1));
		if (NULL == m_resample[i].get())
		{
			return false;
		}
	}

#if CAPTURE_PLAYBACK_INTEGRATED
	m_resample2.reset(new ReSampleImpl(1));
	if (NULL == m_resample2.get())
	{
		return false;
	}
#endif

	return true;
}

void MultipleSourceSync::Reset()
{
	// reset audio buffer manager
	for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	{
		m_AudioBufferMgrPtr[i]->reset();
	}

	// clear exchange list and processing list
	ClearList();

	if (MODE_PUSH_ASYNC_CALL == m_eSyncMode)
	{
		ResetPushMode();
	}
	else if (MODE_PUll_ASYNC_CALL == m_eSyncMode)
	{
		ResetPullModeAsyncCall();
	}
	else if(MODE_PUll_SYNC_CALL == m_eSyncMode)
	{
		// todo
	}

	m_bIsListSizeNormal = true;
}

bool MultipleSourceSync::AllocPushMode()
{
	m_SrcExchangeList_CS.reset(CriticalSectionWrapper::CreateCriticalSection());
	if(NULL == m_SrcExchangeList_CS.get())
	{
		return false;
	}

	if(0 == m_nThreadNum)
	{
		return true;
	}

	m_ProcessThread.reset(ThreadWrapper::CreateThread(ProcessThreadFunction,
		this, webrtc::kRealtimePriority,
		"Multiple_Source_Sync_Process_Thread"));
	if(NULL == m_ProcessThread.get())
	{
		return false;
	}

	m_SrcDataEnoughEvent.reset(EventWrapper::Create());
	if(NULL == m_SrcDataEnoughEvent.get())
	{
		return false;
	}


	for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	{
		// alloc exchange list and processing list
		m_SrcExchangeList[i].reset(new SourceSlice_RefPtrList());
		if(NULL == m_SrcExchangeList[i].get())
		{
			return false;
		}

		m_ProcessingList[i].reset(new SourceSlice_RefPtrList());
		if(NULL == m_ProcessingList[i].get())
		{
			return false;
		}
	}

	return true;
}

bool MultipleSourceSync::AllocPullModeAsyncCall()
{
	m_SrcExchangeList_CS.reset(CriticalSectionWrapper::CreateCriticalSection());
	if(NULL == m_SrcExchangeList_CS.get())
	{
		return false;
	}

	m_SrcDataEnoughEvent.reset(EventWrapper::Create());
	if(NULL == m_SrcDataEnoughEvent.get())
	{
		return false;
	}

	for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	{
		// alloc exchange list and processing list
		m_SrcExchangeList[i].reset(new SourceSlice_RefPtrList());
		if(NULL == m_SrcExchangeList[i].get())
		{
			return false;
		}

		m_ProcessingList[i].reset(new SourceSlice_RefPtrList());
		if(NULL == m_ProcessingList[i].get())
		{
			return false;
		}
	}

	return true;
}

void MultipleSourceSync::ResetPushMode()
{
	// reset m_SrcDataEnoughEvent state
	if(NULL != m_SrcDataEnoughEvent.get())
	{
		m_SrcDataEnoughEvent->Reset();
	}

	// reset channel empty indicate byte for input channels
	for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	{
		m_nAllChannelNotEmptyInd = m_nAllChannelNotEmptyInd | (LIST_EMPTY_IND_BIT<<i);
	}
}


void MultipleSourceSync::ResetPullModeAsyncCall()
{
	// reset m_SrcDataEnoughEvent state
	if(NULL != m_SrcDataEnoughEvent.get())
	{
		m_SrcDataEnoughEvent->Reset();
	}

	// reset channel empty indicate byte for input channels
	for(CAUDIO_U8_t i=0; i<m_nThreadNum; ++i)
	{
		m_nAllChannelNotEmptyInd = m_nAllChannelNotEmptyInd | (LIST_EMPTY_IND_BIT<<i);
	}
}


