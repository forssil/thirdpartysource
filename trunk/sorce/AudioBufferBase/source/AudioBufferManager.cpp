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
#include <stdio.h>
#include <string.h>
#include "AudioBufferManager.h"
#include "audiotrace.h"
bool AudioBufferManager::init(AudioBufferParam_t& aParam)
{
	mIsInitSuccess = false;
	if(AUDIO_BUFFER_MANAGER_MODE_BUFFER == mMode)
	{
		if(init_buffer_mode(aParam))
		{
			mIsInitSuccess = true;
		}
	}
	else if(init_process_mode(aParam))
	{
		mIsInitSuccess = true;
	}

	return mIsInitSuccess;
}


bool AudioBufferManager::init_process_mode(AudioBufferParam_t& aParam)
{
	mLenOfSample = aParam.mLenOfSample;
	mInChannelNum = aParam.mInChannelNum;
	mOutChannelNum = aParam.mOutChannelNum;
	mProcessBufferNum = mInChannelNum > mOutChannelNum ? mInChannelNum : mOutChannelNum;

	mInDefBufferSize = aParam.mFrameShiftSize;
	mFrameShiftSize = aParam.mFrameShiftSize;
	mInitDelay = aParam.mInitDelaySize;

	mInDefBufferLen = mLenOfSample * mInDefBufferSize;
	mProcessBufferLen = mLenOfSample * mFrameShiftSize;
	mFrameShiftLen = mLenOfSample * mFrameShiftSize;

	if (0 == mLenOfSample || 0 == mFrameShiftSize)
	{
		AUDIO_PROCESSING_PRINTF("init param is invalid!");
		return false;
	}

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

	mpBufferCondition = new BufferCondition_t[mProcessBufferNum];
	memset(mpBufferCondition, 0, mProcessBufferNum*sizeof(BufferCondition_t));
	if (NULL == mpBufferCondition)
	{
		AUDIO_PROCESSING_PRINTF("alloc mpBufferCondition failed!");
		return false;
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		if (!createBuffer(mpBufferCondition[channelIndex].mpInDeficiencyBuffer, mInDefBufferLen))
		{
			AUDIO_PROCESSING_PRINTF("create in deficiency buffer failed!");
			return false;
		}
		mpBufferCondition[channelIndex].mInDefBufferWritePtr = 0;
		//mpBufferCondition[channelIndex].mInDefBufferWritePtr = mLenOfSample*mFrameShiftSize / 2;
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mProcessBufferNum; channelIndex++)
	{
		if (!createBuffer(mpBufferCondition[channelIndex].mpProcessBuffer, mProcessBufferLen))
		{
			AUDIO_PROCESSING_PRINTF("create out deficiency buffer failed!");
			return false;
		}
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mOutChannelNum; channelIndex++)
	{
		// introduce mInitDelay at the beginning of output
		mpBufferCondition[channelIndex].mPostProBufWrtPtr = mInitDelay * mLenOfSample;
		mPostProcessBufferLen = aParam.mPostProcessBufferLen;
		if (!createBuffer(mpBufferCondition[channelIndex].mpPostProcessBuffer, mPostProcessBufferLen))
		{
			AUDIO_PROCESSING_PRINTF("create out post process buffer failed!");
			return false;
		}
	}

	if(NULL != mppProcessBufferPtrs)
	{
		delete[] mppProcessBufferPtrs;
		mppProcessBufferPtrs = NULL;
	}
	mppProcessBufferPtrs = (void**) new CAUDIO_U8_t[mProcessBufferNum * sizeof(void*)];
	if (NULL == mppProcessBufferPtrs)
	{
		AUDIO_PROCESSING_PRINTF("mppProcessBufferPtrs alloc failed!");
		return false;
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mProcessBufferNum; channelIndex++)
	{
		mppProcessBufferPtrs[channelIndex] = mpBufferCondition[channelIndex].mpProcessBuffer;
	}

	return true;
}

bool AudioBufferManager::init_buffer_mode(AudioBufferParam_t& aParam)
{
	mLenOfSample = aParam.mLenOfSample;
	mInChannelNum = aParam.mInChannelNum;
	mOutChannelNum = aParam.mOutChannelNum;
	if(mInChannelNum != mOutChannelNum)
	{
		AUDIO_PROCESSING_PRINTF("in channel num and out channel num should be the same in buffer mode");
		return false;
	}

	mProcessBufferNum = mInChannelNum;

	mInDefBufferSize = aParam.mFrameShiftSize;
	mFrameShiftSize = aParam.mFrameShiftSize;
	mInitDelay = aParam.mInitDelaySize;

	mInDefBufferLen = mLenOfSample * mInDefBufferSize;
	//mProcessBufferLen = mLenOfSample * mFrameShiftSize;
	mFrameShiftLen = mLenOfSample * mFrameShiftSize;

	if (0 == mLenOfSample || 0 == mFrameShiftSize)
	{
		AUDIO_PROCESSING_PRINTF("init param is invalid!");
		return false;
	}

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

		delete[] mpBufferCondition;
		mpBufferCondition = NULL;
	}

	mpBufferCondition = new BufferCondition_t[mProcessBufferNum];
	memset(mpBufferCondition, 0, mProcessBufferNum*sizeof(BufferCondition_t));
	if (NULL == mpBufferCondition)
	{
		AUDIO_PROCESSING_PRINTF("alloc mpBufferCondition failed!");
		return false;
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		if (!createBuffer(mpBufferCondition[channelIndex].mpInDeficiencyBuffer, mInDefBufferLen))
		{
			AUDIO_PROCESSING_PRINTF("create in deficiency buffer failed!");
			return false;
		}
		mpBufferCondition[channelIndex].mInDefBufferWritePtr = mInitDelay * mLenOfSample;
		//mpBufferCondition[channelIndex].mInDefBufferWritePtr = mLenOfSample*mFrameShiftSize / 2;
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mOutChannelNum; channelIndex++)
	{
		// introduce mInitDelay at the beginning of output
		mpBufferCondition[channelIndex].mPostProBufWrtPtr = mInitDelay * mLenOfSample;
		//mPostProcessBufferLen = aParam.mPostProcessBufferLen;
		mPostProcessBufferLen = mFrameShiftLen;  // this len is different from process mode
		if (!createBuffer(mpBufferCondition[channelIndex].mpPostProcessBuffer, mPostProcessBufferLen))
		{
			AUDIO_PROCESSING_PRINTF("create out post process buffer failed!");
			return false;
		}
	}

	return true;
}


bool AudioBufferManager::reset()
{
	if(AUDIO_BUFFER_MANAGER_MODE_BUFFER == mMode)
	{
		return reset_buffer_mode();
	}

	return reset_process_mode();
}

bool AudioBufferManager::reset_process_mode()
{
	if (0 == mLenOfSample || 0 == mFrameShiftSize || 0 == mPostProcessBufferLen)
	{
		AUDIO_PROCESSING_PRINTF("init param is invalid!");
		return false;
	}

	CAUDIO_U32_t initDelayLen = mInitDelay * mLenOfSample;
	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		mpBufferCondition[channelIndex].mSrcBufferReadPtr = 0;
		mpBufferCondition[channelIndex].mInDefBufferWritePtr = 0;
		//mpBufferCondition[channelIndex].mInDefBufferWritePtr = mLenOfSample*mFrameShiftSize / 2;
		resetBuffer(mpBufferCondition[channelIndex].mpInDeficiencyBuffer, mInDefBufferLen);
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mProcessBufferNum; channelIndex++)
	{
		resetBuffer(mpBufferCondition[channelIndex].mpProcessBuffer, mProcessBufferLen);
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mOutChannelNum; channelIndex++)
	{
		mpBufferCondition[channelIndex].mPostProBufWrtPtr = initDelayLen;
		resetBuffer(mpBufferCondition[channelIndex].mpPostProcessBuffer, mPostProcessBufferLen);
	}
	

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mProcessBufferNum; channelIndex++)
	{
		mppProcessBufferPtrs[channelIndex] = mpBufferCondition[channelIndex].mpProcessBuffer;
	}

	return true;
}


bool AudioBufferManager::reset_buffer_mode()
{
	if (0 == mLenOfSample || 0 == mFrameShiftSize || 0 == mPostProcessBufferLen)
	{
		AUDIO_PROCESSING_PRINTF("init param is invalid!");
		return false;
	}

	CAUDIO_U32_t initDelayLen = mInitDelay * mLenOfSample;
	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		mpBufferCondition[channelIndex].mSrcBufferReadPtr = 0;
		mpBufferCondition[channelIndex].mInDefBufferWritePtr = mInitDelay * mLenOfSample;;
		//mpBufferCondition[channelIndex].mInDefBufferWritePtr = mLenOfSample*mFrameShiftSize / 2;
		resetBuffer(mpBufferCondition[channelIndex].mpInDeficiencyBuffer, mInDefBufferLen, mpBufferCondition[channelIndex].mInDefBufferWritePtr);
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mOutChannelNum; channelIndex++)
	{
		mpBufferCondition[channelIndex].mPostProBufWrtPtr = initDelayLen;
		resetBuffer(mpBufferCondition[channelIndex].mpPostProcessBuffer, mPostProcessBufferLen);
	}

	return true;
}

bool AudioBufferManager::processSrcData(
	const void** appSrcBuffer,
	CAUDIO_U32_t aSrcBufferLen,
	CAUDIO_U32_t aSrcDataSize,
	void** appOutBuffer,
	CAUDIO_U32_t aOutBufferLen)
{
	if(!mIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init AudioBufferManager failed");
		return false;
	}

	if (NULL == appSrcBuffer)
	{
		AUDIO_PROCESSING_PRINTF("apSrcBuffer is NULL");
		return false;
	}

	if(mMode != AUDIO_BUFFER_MANAGER_MODE_PROCESS)
	{
		AUDIO_PROCESSING_PRINTF("this API is only for process mode");
		return false;
	}

	CAUDIO_U32_t srcDataLen = mLenOfSample * aSrcDataSize;

	// calculate processing rounds according to mFrameShiftLen
	CAUDIO_U32_t maxLoop = (srcDataLen + mpBufferCondition[0].mInDefBufferWritePtr) / mFrameShiftLen;
	for (CAUDIO_U32_t i = 0; i < maxLoop; i++)
	{
		if (!readFrameShiftDataFromSrcBuffer(appSrcBuffer, aSrcBufferLen, srcDataLen))
		{
			return false;
		}

		// call callback function
		if (!mAudioProcessor.processData(mppProcessBufferPtrs, mProcessBufferLen, mInChannelNum, mOutChannelNum))
		{
			AUDIO_PROCESSING_PRINTF("process data failed");
		}

		// copy data to post processed buffer as much as what is processed
		if (!writeToPostProcessBuffer())
		{
			return false;
		}
	}

	// copy data from post process buffer to out channel buffer for every out channel
	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mOutChannelNum; channelIndex++)
	{
		void* pOutChannelBuf = *(appOutBuffer + channelIndex);
		memcpy_s(pOutChannelBuf, aOutBufferLen, (const CAUDIO_U8_t*)mpBufferCondition[channelIndex].mpPostProcessBuffer, srcDataLen);
		if (srcDataLen < mpBufferCondition[channelIndex].mPostProBufWrtPtr)
		{
			// move left data in post process buffer to the head of buffer
			memmove_s(mpBufferCondition[channelIndex].mpPostProcessBuffer, mPostProcessBufferLen, 
				(const CAUDIO_U8_t*)mpBufferCondition[channelIndex].mpPostProcessBuffer + srcDataLen, mpBufferCondition[channelIndex].mPostProBufWrtPtr - srcDataLen);
		}

		mpBufferCondition[channelIndex].mPostProBufWrtPtr -= srcDataLen;
	}

	// copy left source data from in channel buffer to in deficiency buffer
	// these data would be processed at next calling
	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		CAUDIO_U32_t srcDataLeft = srcDataLen - mpBufferCondition[channelIndex].mSrcBufferReadPtr;
		if (0 < srcDataLeft)
		{
			const void* pSrcChannelBuf = *(appSrcBuffer + channelIndex);
			memcpy_s((CAUDIO_U8_t *)mpBufferCondition[channelIndex].mpInDeficiencyBuffer + mpBufferCondition[channelIndex].mInDefBufferWritePtr, 
				mInDefBufferLen - mpBufferCondition[channelIndex].mInDefBufferWritePtr,
				(const CAUDIO_U8_t*)pSrcChannelBuf + mpBufferCondition[channelIndex].mSrcBufferReadPtr, 
				srcDataLeft);
			mpBufferCondition[channelIndex].mInDefBufferWritePtr += srcDataLeft;
		}

		mpBufferCondition[channelIndex].mSrcBufferReadPtr = 0;
	}

	return true;
}


bool AudioBufferManager::bufferSrcData(
	const void** appSrcBuffer,
	CAUDIO_U32_t& aSrcDataSize,
	const void** appOutHeadBuffer,
	CAUDIO_U32_t& aOutHeadDataSize,
	CAUDIO_U32_t& aOutHeadBufferLen)
{
	if(!mIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init AudioBufferManager failed");
		return false;
	}

	if (NULL == appSrcBuffer)
	{
		AUDIO_PROCESSING_PRINTF("apSrcBuffer is NULL");
		return false;
	}

	if(mMode != AUDIO_BUFFER_MANAGER_MODE_BUFFER)
	{
		AUDIO_PROCESSING_PRINTF("this API is only for buffer mode");
		return false;
	}

	CAUDIO_U32_t srcDataLen = mLenOfSample * aSrcDataSize;
	CAUDIO_U32_t aOutHeadDataLen = 0;
	CAUDIO_U32_t allDataLen = srcDataLen + mpBufferCondition[0].mInDefBufferWritePtr;
	CAUDIO_U32_t deficiencyLen = 0;
	if(allDataLen >= mFrameShiftLen)
	{
		deficiencyLen = (srcDataLen + mpBufferCondition[0].mInDefBufferWritePtr) % mFrameShiftLen;

		if (!writeDeficiencyDataToOutBuffer(aOutHeadDataLen))
		{
			return false;
		}
	}
	else
	{
		// all data is not enough for one frame,
		// so copy append input data to deficiency buffer
		//AUDIO_PROCESSING_PRINTF("data not enough for one frame");
		deficiencyLen = srcDataLen;
	}

	// copy left source data from in channel buffer to in deficiency buffer
	// these data would be output at next calling
	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		if (0 < deficiencyLen)
		{
			const void* pSrcChannelBuf = *(appSrcBuffer + channelIndex);
			memcpy_s((char *)mpBufferCondition[channelIndex].mpInDeficiencyBuffer + mpBufferCondition[channelIndex].mInDefBufferWritePtr, 
				mInDefBufferLen - mpBufferCondition[channelIndex].mInDefBufferWritePtr,
				(const CAUDIO_U8_t*)pSrcChannelBuf + srcDataLen - deficiencyLen, deficiencyLen);
			mpBufferCondition[channelIndex].mInDefBufferWritePtr += deficiencyLen;
		}
	}

	//set return value
	aSrcDataSize = (srcDataLen - deficiencyLen) / mLenOfSample;
	aOutHeadDataSize = aOutHeadDataLen / mLenOfSample;

	for(CAUDIO_U8_t i=0; i<mOutChannelNum; ++i)
	{
		appOutHeadBuffer[i] = mpBufferCondition[i].mpPostProcessBuffer;
	}
	aOutHeadBufferLen = mPostProcessBufferLen;

	return true;
}


bool AudioBufferManager::readFrameShiftDataFromSrcBuffer(
	const void** appSrcBuffer,
	CAUDIO_U32_t aSrcBufferLen,
	CAUDIO_U32_t aSrcDataLen)
{
	if (NULL == appSrcBuffer)
	{
		AUDIO_PROCESSING_PRINTF("argument is invalid!");
		return false;
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		// check if there is deficiency data
		if (mpBufferCondition[channelIndex].mInDefBufferWritePtr > 0)
		{
			if (mpBufferCondition[channelIndex].mInDefBufferWritePtr > mFrameShiftLen)
			{
				return false;
			}

			// copy deficiency data firstly , these data is left unprocessed at previous calling
			// so copy deficiency data firstly and then copy data from in channel buffer
			memcpy_s(mpBufferCondition[channelIndex].mpProcessBuffer, mProcessBufferLen, 
				mpBufferCondition[channelIndex].mpInDeficiencyBuffer, mpBufferCondition[channelIndex].mInDefBufferWritePtr);

			if (mpBufferCondition[channelIndex].mInDefBufferWritePtr == mFrameShiftLen)
			{
				AUDIO_PROCESSING_PRINTF("data in inBuffer is sufficient");
				mpBufferCondition[channelIndex].mInDefBufferWritePtr = 0;
				continue;

				if (channelIndex == mInChannelNum-1)
				{
					return true;
				}
			}

			// copy data from in channel buffer
			CAUDIO_U32_t srcDataNeededLen = mFrameShiftLen - mpBufferCondition[channelIndex].mInDefBufferWritePtr;
			CAUDIO_U32_t srcDataLeft = aSrcDataLen - mpBufferCondition[channelIndex].mSrcBufferReadPtr;
			CAUDIO_U32_t srcDataCopiedLen = (srcDataLeft > srcDataNeededLen) ? srcDataNeededLen : srcDataLeft;

			const void* pChannelBuf = *(appSrcBuffer + channelIndex);
			memcpy_s((CAUDIO_U8_t *)mpBufferCondition[channelIndex].mpProcessBuffer + mpBufferCondition[channelIndex].mInDefBufferWritePtr, 
				mProcessBufferLen - mpBufferCondition[channelIndex].mInDefBufferWritePtr, (const CAUDIO_U8_t *)pChannelBuf + mpBufferCondition[channelIndex].mSrcBufferReadPtr, srcDataCopiedLen);
			mpBufferCondition[channelIndex].mInDefBufferWritePtr = 0;
			mpBufferCondition[channelIndex].mSrcBufferReadPtr += srcDataCopiedLen;
		}
		else
		{
			// copy data from in channel buffer
			CAUDIO_U32_t srcDataLeft = aSrcDataLen - mpBufferCondition[channelIndex].mSrcBufferReadPtr;
			CAUDIO_U32_t srcDataCopiedLen = (srcDataLeft > mFrameShiftLen) ? mFrameShiftLen : srcDataLeft;

			const void* pChannelBuf = *(appSrcBuffer + channelIndex);
			memcpy_s(mpBufferCondition[channelIndex].mpProcessBuffer, mProcessBufferLen, (const CAUDIO_U8_t*)pChannelBuf + mpBufferCondition[channelIndex].mSrcBufferReadPtr, srcDataCopiedLen);
			mpBufferCondition[channelIndex].mInDefBufferWritePtr = 0;
			mpBufferCondition[channelIndex].mSrcBufferReadPtr += srcDataCopiedLen;
		}

		if (mpBufferCondition[channelIndex].mSrcBufferReadPtr > aSrcDataLen)
		{
			AUDIO_PROCESSING_PRINTF("mSrcBufferReadPtr is larger than srcDataLen, this should not happend!");
			return false;
		}
	}

	return true;
}


bool AudioBufferManager::writeDeficiencyDataToOutBuffer(CAUDIO_U32_t& aOutHeadDataLen)
{
	// mInChannelNum must equal to mOutChannelNum
	if(mInChannelNum != mOutChannelNum)
	{
		AUDIO_PROCESSING_PRINTF("mInChannelNum doesn't equal to mOutChannelNum!");
		return false;
	}

	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mInChannelNum; channelIndex++)
	{
		// check if there is deficiency data
		if (mpBufferCondition[channelIndex].mInDefBufferWritePtr > 0)
		{
			if (mpBufferCondition[channelIndex].mInDefBufferWritePtr > mFrameShiftLen)
			{
				return false;
			}

			// copy deficiency data, these data is left unprocessed at previous calling
			memcpy_s( mpBufferCondition[channelIndex].mpPostProcessBuffer, mPostProcessBufferLen, mpBufferCondition[channelIndex].mpInDeficiencyBuffer, 
				mpBufferCondition[channelIndex].mInDefBufferWritePtr);
			aOutHeadDataLen = mpBufferCondition[channelIndex].mInDefBufferWritePtr;
			mpBufferCondition[channelIndex].mInDefBufferWritePtr = 0;
		}
	}

	return true;
}

bool AudioBufferManager::writeToPostProcessBuffer()
{
	for (CAUDIO_U8_t channelIndex = 0; channelIndex < mOutChannelNum; channelIndex++)
	{
		memcpy_s((CAUDIO_U8_t*)mpBufferCondition[channelIndex].mpPostProcessBuffer + mpBufferCondition[channelIndex].mPostProBufWrtPtr, 
			mPostProcessBufferLen - mpBufferCondition[channelIndex].mPostProBufWrtPtr, mpBufferCondition[channelIndex].mpProcessBuffer, mFrameShiftLen);
		mpBufferCondition[channelIndex].mPostProBufWrtPtr += mFrameShiftLen;
	}
	return true;
}
