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

#ifndef AUDIO_BUFFER_MANAGER_H_
#define AUDIO_BUFFER_MANAGER_H_

#include "audiotypedef.h"
#include "CodyyAudioCommon.h"
#include "AudioProcessorBase.h"


typedef struct AudioBufferParam
{
	CAUDIO_U16_t mLenOfSample; // in byte
	CAUDIO_U16_t mFrameShiftSize; // in sample
	CAUDIO_U32_t mPostProcessBufferLen; // not for buffer mode, use to store processed data to output
	CAUDIO_U16_t mInitDelaySize; //introduce output delay in samples
	CAUDIO_U8_t mInChannelNum;
	CAUDIO_U8_t mOutChannelNum;
}AudioBufferParam_t;


typedef enum AUDIO_BUFFER_PARAM
{
	ABP_LEN_OF_SAMPLLE,
	ABP_FRAME_SHIFT_SIZE,
	ABP_POST_PROCESS_BUFFER_LEN,
	ABP_INIT_DELAY,
	ABP_IN_CHANNEL_NUM,
	ABP_OUT_CHANNEL_NUM,
	ABP_PROCESSING_BUFFER_NUM,
	ABP__MAX
}AUDIO_BUFFER_PARAM_e;

// use to save buffer conditions between processings of channels
typedef struct BufferCondition
{
	CAUDIO_U32_t mSrcBufferReadPtr; // in bytes
	CAUDIO_U32_t mInDefBufferWritePtr; // in bytes
	CAUDIO_U32_t mPostProBufWrtPtr; // in bytes

	void* mpInDeficiencyBuffer;
	void* mpProcessBuffer;
	void* mpPostProcessBuffer;
}BufferCondition_t;


class AudioBufferManager
{

typedef enum AUDIO_BUFFER_MANAGER_MODE
{
	AUDIO_BUFFER_MANAGER_MODE_PROCESS,  // process source data inside, need set a processing callback function 
	AUDIO_BUFFER_MANAGER_MODE_BUFFER	// just buffering the source data
}AUDIO_BUFFER_MANAGER_MODE_e;

public:

	// if you want to process source data inside, use this constructor
	AudioBufferManager(AudioProcessorBase& aAudioProcessorBase)
		:
		mAudioProcessor(aAudioProcessorBase)
	{
		mLenOfSample = 0;
		mInChannelNum = 0;
		mOutChannelNum = 0;
		mProcessBufferNum = 0;

		mInDefBufferSize = 0;
		mFrameShiftSize = 0;
		mInitDelay = 0;

		mPostProcessBufferLen = 0;

		mpBufferCondition = NULL;
		mppProcessBufferPtrs = NULL;

		mMode = AUDIO_BUFFER_MANAGER_MODE_PROCESS;
		mIsInitSuccess = false;
	}

	// if you just want to buffer the source data, use this constructor
	AudioBufferManager() : mAudioProcessor(*(new AudioProcessorBase())) // alloc a dummy AudioProcessorBase
	{
		mLenOfSample = 0;
		mInChannelNum = 0;
		mOutChannelNum = 0;
		mProcessBufferNum = 0;

		mInDefBufferSize = 0;
		mFrameShiftSize = 0;
		mInitDelay = 0;

		mPostProcessBufferLen = 0;

		mpBufferCondition = NULL;
		mppProcessBufferPtrs = NULL;

		mMode = AUDIO_BUFFER_MANAGER_MODE_BUFFER;
		mIsInitSuccess = false;
	}

	virtual ~AudioBufferManager()
	{
		if(NULL != mpBufferCondition)
		{
			for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
			{
				deleteBuffer(mpBufferCondition[channelIndex].mpInDeficiencyBuffer);
			}

			for (CAUDIO_U8_t channelIndex = 0; channelIndex < mOutChannelNum; channelIndex++)
			{
				deleteBuffer(mpBufferCondition[channelIndex].mpPostProcessBuffer);
			}

			for (CAUDIO_U8_t channelIndex = 0; channelIndex < mProcessBufferNum; channelIndex++)
			{
				deleteBuffer(mpBufferCondition[channelIndex].mpProcessBuffer);
			}

			delete[] mpBufferCondition;
			mpBufferCondition = NULL;
		}

		if (NULL != mppProcessBufferPtrs)
		{
			delete[] mppProcessBufferPtrs;
			mppProcessBufferPtrs = NULL;
		}

		if(AUDIO_BUFFER_MANAGER_MODE_BUFFER == mMode)
		{
			delete &mAudioProcessor;
		}

		mIsInitSuccess = false;
	}

public:
	bool init(AudioBufferParam_t& aParam);
	bool init_process_mode(AudioBufferParam_t& aParam);
	bool init_buffer_mode(AudioBufferParam_t& aParam);
	//bool reset(AudioBufferParam_t& aParam){ return init(aParam); }
	bool reset();
	bool reset_process_mode();
	bool reset_buffer_mode();

public:
	// user interface , you would get how much data you've fed,
	// the output data would have been processed by your callback 
	// function
	bool processSrcData(
		const void** appSrcBuffer,
		CAUDIO_U32_t aSrcBufferLen,
		CAUDIO_U32_t aSrcDataSize,
		void** appOutBuffer,
		CAUDIO_U32_t aOutBufferLen);

	// user interface, you would get integer times of len of 
	// FrameShift of data, the left input data would be buffering until next 
	// calling, the output data is separate into two parts, the first block
	// is put into OutHeadBuffer, the second part is still in source 
	// buffer, but the output length maybe has been changed.
	// memory of head buffer is alloc inside
	bool bufferSrcData(
		const void** appSrcBuffer,
		CAUDIO_U32_t& aSrcDataSize,
		const void** appOutHeadBuffer,
		CAUDIO_U32_t& aOutHeadDataSize,
		CAUDIO_U32_t& aOutHeadBufferLen);

private:

	// write source data to processing buffer to process
	bool readFrameShiftDataFromSrcBuffer(
		const void** appSrcBuffer,
		CAUDIO_U32_t aSrcBufferLen,
		CAUDIO_U32_t aSrcDataLen);

	bool writeDeficiencyDataToOutBuffer(CAUDIO_U32_t& aOutHeadDataSize);

	bool writeToPostProcessBuffer();

private:
	CAUDIO_U16_t mLenOfSample;
	CAUDIO_U8_t mInChannelNum;
	CAUDIO_U8_t mOutChannelNum;
	CAUDIO_U8_t mProcessBufferNum;

	// in number of samples
	CAUDIO_U16_t mInDefBufferSize;
	CAUDIO_U16_t mFrameShiftSize;
	CAUDIO_U16_t mInitDelay;

	// in bytes
	CAUDIO_U16_t mInDefBufferLen;
	CAUDIO_U16_t mProcessBufferLen;
	CAUDIO_U32_t mPostProcessBufferLen;
	CAUDIO_U16_t mFrameShiftLen;

	BufferCondition_t* mpBufferCondition;
	void** mppProcessBufferPtrs; //save the pointers to process buffers
	AUDIO_BUFFER_MANAGER_MODE_e mMode;
	bool mIsInitSuccess;
private:
	AudioProcessorBase& mAudioProcessor;
};


#endif //AUDIO_BUFFER_MANAGER_H