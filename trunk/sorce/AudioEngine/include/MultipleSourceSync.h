/*	
 *	Name:			MultipleSourceSync.h
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	
 *  
 *	History:
 *					12/19/2014 Created			Zhong Yaozhu
 *					03/05/2015 add PULL MODE	Zhong Yaozhu
 *
 *
 */

#ifndef MULTIPLE_SOURCE_SYNC_H_
#define MULTIPLE_SOURCE_SYNC_H_
#include <list>
#include "thread_wrapper.h"
#include "event_wrapper.h"
#include "scoped_ptr.h"
#include "scoped_refptr.h"
#include "critical_section_wrapper.h"
#include "memory_pool.h"
#include "audiotypedef.h"
#include "AudioBufferManager.h"
#include "AudioModuleImplBase.h"
#include "ReSampleImpl.h"
#include "WaveIO.h"


//namespace codyy_audio {


#define MAX_INPUT_THREAD_NUM  MAX_THREAD_NUM
#define SOURCE_SLICE_TIME_LEN 10 // 10MS
#define MAX_SOURCE_SLICE_SIZE 48 * SOURCE_SLICE_TIME_LEN // 48KHZ, 10MS
#define MAX_LIST_TIME_MS 2000

typedef enum SyncMode
{
	MODE_PUSH_ASYNC_CALL,
	MODE_PUll_SYNC_CALL,
	MODE_PUll_ASYNC_CALL,
}SyncMode_e;

class MultipleSourceSync
{

public:
	MultipleSourceSync(
		IAudioModuleImpleBase *a_AudioProcessor,
		CAUDIO_U8_t a_nThreadNum,
		CAUDIO_U8_t a_nChannelNumPerThread,
		AUDIO_DATA_TYPE a_nFrameTimeMsPerChannel,
		CAUDIO_U32_t a_nFs,
		SyncMode_e a_eSyncMode,
		DATA_TYPE a_eAudioDataType,
		ChannelId_e a_eChannelId);

	// this API must be called after constructor is called
	bool Init();

	virtual ~MultipleSourceSync();

// MultipleSourceSync API
public:
	// data fed before this API is called will be abandoned
	// sync between StartProcess and StopProcess is guaranteed
	// by caller
	ModuleState StartProcess();
	
	ModuleState StopProcess();

	void SetFs(CAUDIO_U32_t a_nFs){m_nFs = a_nFs;}

	//bool FeedData(	
	//	const void* a_pData,
	//	DATA_TYPE a_DataType,
	//	CAUDIO_U32_t a_nSampleNum,
	//	CAUDIO_U32_t a_nFs,
	//	CAUDIO_U32_t a_nChannelNum,
	//	CAUDIO_U8_t a_nThreadIdx);

	bool FeedData(
		const void* a_pData,
		DATA_TYPE a_DataType,
		CAUDIO_U32_t a_nSampleNum,
		CAUDIO_U32_t a_nChannelNum,
		CAUDIO_U8_t a_nThreadIdx,
		CAUDIO_U32_t a_nOriginalSampleRate = 0);

	// pull one frame per thread at one call
	bool PullData(
		AudioFrame* a_AudioFrameArray, 
		CAUDIO_U8_t a_nArrayLen,
		CAUDIO_U8_t& a_nThreadNum,
		bool& a_bIsDataPulledOut);

#if CAPTURE_PLAYBACK_INTEGRATED
	bool PullData_CapturePlaybackIntegrated(
		AudioFrame*  pAudioFrameNeeded,
		const CAUDIO_U32_t nMaxFrameSize,
		const CAUDIO_U32_t nSamplesPerSec,
		const CAUDIO_U8_t nChannel);

	bool PassSlice_CapturePlaybackIntegrated(
		AudioFrame* a_AudioFrameArray);

	bool AllocReSampleBuff_CapturePlaybackIntegrated(CAUDIO_U32_t resample_len);
#endif

	SyncMode_e GetSyncMode(){return m_eSyncMode;}

	bool HasEverPulled(){return m_bHasEverPulled;}

	void SetDSId(DataSyncId_e a_eDSId, CAUDIO_U8_t a_nThreadIdx)
	{
		m_eDataSyncId[a_nThreadIdx] = a_eDSId;
	}

	void SetDSId(DataSyncId_e a_eDSId)
	{
		for(CAUDIO_U8_t idx=0; idx<MAX_INPUT_THREAD_NUM; ++idx)
		{
			m_eDataSyncId[idx] = a_eDSId;
		}
	}

private:
	//bool FeedData_PushMode(	
	//	const void* a_pData,
	//	DATA_TYPE a_DataType,
	//	CAUDIO_U32_t a_nSampleNum,
	//	CAUDIO_U32_t a_nFs,
	//	CAUDIO_U32_t a_nChannelNum,
	//	CAUDIO_U8_t a_nThreadIdx);

	//bool FeedData_PullMode_AsyncCall(	
	//	const void* a_pData,
	//	DATA_TYPE a_DataType,
	//	CAUDIO_U32_t a_nSampleNum,
	//	CAUDIO_U32_t a_nFs,
	//	CAUDIO_U32_t a_nChannelNum,
	//	CAUDIO_U8_t a_nThreadIdx);

	//bool FeedData_Common(	
	//	const void* a_pData,
	//	DATA_TYPE a_DataType,
	//	CAUDIO_U32_t a_nSampleNum,
	//	CAUDIO_U32_t a_nFs,
	//	CAUDIO_U32_t a_nChannelNum,
	//	CAUDIO_U8_t a_nThreadIdx);

	bool FeedData_PushMode(
		const void* a_pData,
		DATA_TYPE a_DataType,
		CAUDIO_U32_t a_nSampleNum,
		CAUDIO_U32_t a_nOriginalSampleRate,
		CAUDIO_U32_t a_nChannelNum,
		CAUDIO_U8_t a_nThreadIdx);

	bool FeedData_PullMode_AsyncCall(
		const void* a_pData,
		DATA_TYPE a_DataType,
		CAUDIO_U32_t a_nSampleNum,
		CAUDIO_U32_t a_nOriginalSampleRate,
		CAUDIO_U32_t a_nChannelNum,
		CAUDIO_U8_t a_nThreadIdx);

	bool FeedData_Common(
		const void* a_pData,
		DATA_TYPE a_DataType,
		CAUDIO_U32_t a_nSampleNum,
		CAUDIO_U32_t a_nOriginalSampleRate,
		CAUDIO_U32_t a_nChannelNum,
		CAUDIO_U8_t a_nThreadIdx);

	CAUDIO_U32_t Resample(
		const void* a_pData, 
		CAUDIO_U32_t a_nSampleNum, 
		CAUDIO_U32_t a_nDefaultSampleRate, 
		CAUDIO_U32_t a_nOriginalSampleRate,
		CAUDIO_U8_t a_nThreadIdx = -1);

	bool AllocReSampleBuff(CAUDIO_U32_t resample_len);
// MultipleSourceSync inside types
private:

	// redefine webrtc types
	typedef webrtc::ThreadWrapper ThreadWrapper;
	typedef webrtc::scoped_ptr<ThreadWrapper> ThreadWrapperPtr;
	typedef webrtc::EventWrapper EventWrapper;
	typedef webrtc::scoped_ptr<EventWrapper> EventWrapperPtr;
	typedef webrtc::CriticalSectionWrapper CriticalSectionWrapper;
	typedef webrtc::scoped_ptr<CriticalSectionWrapper> CriticalSectionWrapperPtr;
	typedef webrtc::scoped_ptr<AudioBufferManager> AudioBufferManagerPtr;
	typedef webrtc::scoped_ptr<ReSampleImpl> ReSampleImplPtr;

	// define nested class SourceSlice_Ref
	class SourceSlice_Ref 
	{
		typedef webrtc::MemoryPool<SourceSlice_Ref> SrcSlice_MemoryPool;
		static const CAUDIO_U8_t kInvalidThreadIdx = 0xFF;

	public:
		SourceSlice_Ref() : m_nThreadIdx(kInvalidThreadIdx), m_nRefCount(0), m_pMemoryPool(NULL) {}

		virtual ~SourceSlice_Ref() {}

	// SourceSlice_Ref API
	public:
		void SetData(
			const void* a_pData,
			DATA_TYPE a_DataType,
			CAUDIO_U32_t a_nSampleNum,
			CAUDIO_U32_t a_nFs,
			CAUDIO_U32_t a_nChannelNum,
			DataSyncId_e a_eDataSyncId);

		void AppendData(
			const void* a_pData,
			CAUDIO_U32_t a_nSampleNum);

		inline const AudioFrame& getAudioFrame() const {return m_AudioFrame;}

		inline CAUDIO_U8_t GetThreadIdx() const { return m_nThreadIdx;}

		void Init(
			SrcSlice_MemoryPool* a_pMemoryPool,
			const void* a_pData,
			DATA_TYPE a_DataType,
			CAUDIO_U32_t a_nSampleNum,
			CAUDIO_U32_t a_nFs,
			CAUDIO_U32_t a_nChannelNum,
			CAUDIO_U8_t a_nThreadIdx,
			DataSyncId_e a_eDataSyncId);

	// for reference count
	public:

		// Release a reference. Will delete the object if the reference count
		// reaches zero.
		CAUDIO_S32_t Release();

		// Add a reference.
		inline CAUDIO_S32_t AddRef();

	private:
		void Reset();

	private:
		AudioFrame m_AudioFrame;						// audio data.
		CAUDIO_U8_t m_nThreadIdx;						// input thread index
		CAUDIO_S32_t m_nRefCount;						// Counts the number of references to a source slice.

	private:
		SrcSlice_MemoryPool* m_pMemoryPool;
	};

	// define memory pool of source slice
	typedef webrtc::MemoryPool<SourceSlice_Ref> SrcSlice_MemoryPool;
	typedef webrtc::scoped_refptr<SourceSlice_Ref> SourceSlice_RefPtr;

	// define a list to store source slice
	typedef std::list<SourceSlice_RefPtr> SourceSlice_RefPtrList;
	typedef webrtc::scoped_ptr<SourceSlice_RefPtrList> SourceSlice_RefPtrList_Ptr;

// internal tool func
private:
	static bool ProcessThreadFunction(ThreadObj threadObj);

	bool Process();

	SourceSlice_Ref* AllocSourceSlice(
		const void* a_pData,
		DATA_TYPE a_DataType,
		CAUDIO_U32_t a_nSampleNum,
		CAUDIO_U32_t a_nFs,
		CAUDIO_U32_t a_nChannelNum,
		CAUDIO_U8_t a_nThreadIdx);

	void SetExchangeListAsEmpty(CAUDIO_U8_t a_nChannelIdex);

	void SetExchangeListAsNotEmpty(CAUDIO_U8_t a_nChannelIdex);

	void SetProcessingListAsEmpty(CAUDIO_U8_t a_nChannelIdex);

	void SetProcessingListAsNotEmpty(CAUDIO_U8_t a_nChannelIdex);

	bool IsExchListDataEnough();

	CAUDIO_U32_t GetMinExchangeListSize();

	CAUDIO_U32_t GetMaxExchangeListSize();

	CAUDIO_U32_t GetMaxProcessingListSize();

	void ClearList();

	CAUDIO_U32_t CopySliceToProcessingList();

	bool PassSliceToProcessingModule(
		CAUDIO_U32_t a_nPassSize, 
		const AudioFrame** a_AudioFrameArray);

	void GetAudioFrameParam(AUDIO_FRAME_PARA& a_Param, CAUDIO_U8_t a_nThreadIdx);

	bool AllThreadGenerateZeroData(
		AudioFrame a_AudioFrameArray[], 
		CAUDIO_U8_t a_nArrayLen);

	void ExchangeListSizeCheck();

	void ProcessingListSizeCheck();

// internal use api
private:
	bool Alloc();

	void Reset();

	bool AllocPushMode();

	bool AllocPullModeAsyncCall();

	void ResetPushMode();

	void ResetPullModeAsyncCall();



private:
	ThreadWrapperPtr m_ProcessThread;
	EventWrapperPtr m_SrcDataEnoughEvent;
	CriticalSectionWrapperPtr m_SrcExchangeList_CS;
	SrcSlice_MemoryPool* m_pSrcMemoryPool;
	AudioBufferManagerPtr m_AudioBufferMgrPtr[MAX_INPUT_THREAD_NUM];
	SourceSlice_RefPtrList_Ptr m_SrcExchangeList[MAX_INPUT_THREAD_NUM];  // a list for source exchanging between outside thread and processing thread
	SourceSlice_RefPtrList_Ptr m_ProcessingList[MAX_INPUT_THREAD_NUM]; // a list to buffer data for processing thread
	CAUDIO_U8_t m_nExchangeListEmptyInd; // indicate if exchange list is empty , 8 bit for 8 threads
	CAUDIO_U8_t m_nProcessingListEmptyInd; // indicate if processing list is empty , 8 bit for 8 threads, unused
	CAUDIO_U8_t m_nAllChannelNotEmptyInd;
	CAUDIO_U8_t m_nThreadNum;
	CAUDIO_U8_t m_nChannelNumPerThread;
	AUDIO_DATA_TYPE m_nFrameTimeMs; // frame time for all channels in one thread, must be an even number
	CAUDIO_U32_t m_nFrameSize;  // frame size for all channels in one thread, must be an even number for stereo
	CAUDIO_U32_t m_nFs; // sample rate
	bool m_bStartProcess; // for data stream control
	bool m_bIsInStopState; // state flag, for init, reset and processing action sync control
	IAudioModuleImpleBase *m_pAudioModuleImpl;
	SyncMode_e m_eSyncMode;
	DATA_TYPE m_eAudioDataType;
	bool m_bIsInitSuccess;
	CAUDIO_U32_t m_nMaxListSize;
	bool m_bIsListSizeNormal;
	DataSyncId_e m_eDataSyncId[MAX_INPUT_THREAD_NUM];  // indicate where the data is from
	bool m_bHasEverPulled;  // indicate if any pull data action has ever been taken before

	ReSampleImplPtr m_resample[MAX_INPUT_THREAD_NUM];
	CAUDIO_U32_t m_nReSampleBufferSize;
	AUDIO_DATA_TYPE* m_pReSampleBuffer;
	ChannelId_e m_eChannelId;

#if CAPTURE_PLAYBACK_INTEGRATED
	ReSampleImplPtr m_resample2;
	CAUDIO_U32_t m_nResampleBufferSize2;
	AUDIO_DATA_TYPE* m_pReSampleBuffer2;
#endif
};


//}  // namespace codyy_audio

#endif  // MULTIPLE_SOURCE_SYNC_H_
