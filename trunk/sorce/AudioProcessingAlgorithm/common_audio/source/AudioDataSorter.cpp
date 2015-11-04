/***********************************************************************
*  Author
;*      Zhong Yaozhu
;*
;*
;*
;*  History
;*      10/23/2014 Created
;*
;*
;*************************************************************************/

#include "AudioDataSorter.h"
#include "audiotrace.h"
bool AudioDataSorter::init(AudioDataSorterParam_t& aParam)
{
	mHistoryBufferLen = aParam.mHistoryBufferLen;
	mSortType = aParam.mSortType;
	mHistoryBufferWritePtr = 0;

	if (!createBuffer((void*&)mHistoryBuffer, mHistoryBufferLen * sizeof(HistoryDataInfo_t)))
	{
		AUDIO_PROCESSING_PRINTF("create History buffer failed");
		return false;
	}

	if(!createExtremList(aParam.mExtremumListLen))
	{
		AUDIO_PROCESSING_PRINTF("create extremum list failed");
		return false;
	}

	mpMidExtremNode = getExtremNode(aParam.mExtremumListLen/2+1);
	return true;
}


SORTING_DATA_TYPE AudioDataSorter::getExtremum()
{
	if(NULL == mpMidExtremNode)
	{
		AUDIO_PROCESSING_PRINTF("mpMidExtremNode is NULL");
		return 0;
	}

	return mpMidExtremNode->mValue;
}


void AudioDataSorter::updateHistoryBuffer(SORTING_DATA_TYPE aDataValue)
{
	//need update extremum buffer if the abundoned data exists in extremum list
	bool isAbundonInExremumBuffer = NULL != mHistoryBuffer[mHistoryBufferWritePtr].mpExtremNode;

	if (isAbundonInExremumBuffer)
	{
		// delete this timeout data in exremum list and search one new from history buffer
		CAUDIO_U32_t qualityPos = searchQualityValueInHistoryBuffer(mHistoryBufferWritePtr + 1);

		ExtremeNode_t* pExtremNode = new ExtremeNode_t;
		if(NULL == pExtremNode)
		{
			AUDIO_PROCESSING_PRINTF("pExtremNode is NULL");
			return;
		}

		pExtremNode->mHistoryBuffPosition = qualityPos;
		pExtremNode->mValue = mHistoryBuffer[qualityPos].mValue;
		pExtremNode->mpPreNode = NULL;
		pExtremNode->mpPostNode = NULL;
		mHistoryBuffer[qualityPos].mpExtremNode = updateExtremList(pExtremNode, mHistoryBuffer[mHistoryBufferWritePtr].mpExtremNode);
	}

	mHistoryBuffer[mHistoryBufferWritePtr].mValue = aDataValue;

	// check if the newest data is an extremum data ,
	// update exremum list if it is
	ExtremeNode_t* pLeastQualityNode = mExtremumList.mpHead->mpPostNode;
	AUDIO_DATA_TYPE leastQualityValue = pLeastQualityNode->mValue;
	if (isQualityValue(aDataValue, leastQualityValue))
	{
		ExtremeNode_t* pExtremNode = new ExtremeNode_t;
		if(NULL == pExtremNode)
		{
			AUDIO_PROCESSING_PRINTF("pExtremNode is NULL");
			return;
		}
		pExtremNode->mHistoryBuffPosition = mHistoryBufferWritePtr;
		pExtremNode->mValue = aDataValue;
		pExtremNode->mpPostNode = NULL;
		pExtremNode->mpPreNode = NULL;

		// always remove the least quality data in retremum buffer
		// update position info of extreme threshold in history buffer
		CAUDIO_U32_t extremeThrHisBuffPos = pLeastQualityNode->mHistoryBuffPosition;
		mHistoryBuffer[extremeThrHisBuffPos].mpExtremNode = NULL;
		
		// update new data to extremum buffer
		mHistoryBuffer[mHistoryBufferWritePtr].mpExtremNode = updateExtremList(pExtremNode);
	}
	else
	{
		mHistoryBuffer[mHistoryBufferWritePtr].mpExtremNode = NULL;
	}

	if (++mHistoryBufferWritePtr == mHistoryBufferLen)
	{
		mHistoryBufferWritePtr = 0;
	}
}


ExtremeNode_t* AudioDataSorter::updateExtremList(ExtremeNode_t* apInsertedNode, ExtremeNode_t* apDeletedNode)
{
	// insert node after head
	deleteExtremNode(apDeletedNode);
	ExtremeNode_t* pHead = mExtremumList.mpHead;
	ExtremeNode_t* pNext = pHead->mpPostNode;

	if(NULL ==pNext)
	{
		return NULL;
	}

	pHead->mpPostNode = apInsertedNode;
	pNext->mpPreNode = apInsertedNode;
	apInsertedNode->mpPostNode = pNext;
	apInsertedNode->mpPreNode = pHead;

	mpMidExtremNode = getExtremNode(mExtremumList.mListLen/2+1);
	return apInsertedNode;
}

ExtremeNode_t* AudioDataSorter::updateExtremList(ExtremeNode_t* apInsertedNode)
{
	// always delete the least quality node
	ExtremeNode_t* pLeastQualityNode = mExtremumList.mpHead->mpPostNode;
	deleteExtremNode(pLeastQualityNode);

	// get insert position
	ExtremeNode_t* pNode = mExtremumList.mpHead->mpPostNode;
	ExtremeNode_t* pNext = pNode->mpPostNode;
	AUDIO_DATA_TYPE refValue = pNode->mValue;
	AUDIO_DATA_TYPE validateValue = apInsertedNode->mValue;
	while (isQualityValue(validateValue, refValue))
	{
		pNode = pNode->mpPostNode;
		refValue = pNode->mValue;
		if(NULL == pNode->mpPostNode)
		{
			break;
		}
	}

	insertExtremNode(apInsertedNode, pNode);
	mpMidExtremNode = getExtremNode(mExtremumList.mListLen/2+1);
	return apInsertedNode;
}


CAUDIO_U32_t AudioDataSorter::searchQualityValueInHistoryBuffer(CAUDIO_U32_t aBeginPos)
{
	CAUDIO_U32_t qualityPos = 0;
	AUDIO_DATA_TYPE candidateValue = 0;

	AUDIO_DATA_TYPE temHistoryValue = 0;

	for (CAUDIO_U32_t temPos = aBeginPos; temPos < mHistoryBufferLen; temPos++)
	{
		if (NULL != mHistoryBuffer[temPos].mpExtremNode)
		{
			continue;
		}

		temHistoryValue = mHistoryBuffer[temPos].mValue;
		if (isQualityValue(temHistoryValue, candidateValue))
		{
			candidateValue = temHistoryValue;
			qualityPos = temPos;
		}
	}

	for (CAUDIO_U32_t temPos = 0; temPos < aBeginPos; temPos++)
	{
		if (NULL != mHistoryBuffer[temPos].mpExtremNode)
		{
			continue;
		}

		temHistoryValue = mHistoryBuffer[temPos].mValue;
		if (isQualityValue(temHistoryValue, candidateValue))
		{
			candidateValue = temHistoryValue;
			qualityPos = temPos;
		}
	}

	return qualityPos;
}

bool AudioDataSorter::isExtremum(SORTING_DATA_TYPE aDataValue)
{
	ExtremeNode_t* pLeastQualityNode = mExtremumList.mpHead->mpPostNode;
	AUDIO_DATA_TYPE leastQualityValue = pLeastQualityNode->mValue;

	if (SORT_TYPE_MAXIMUM == mSortType)
	{
		return aDataValue >= leastQualityValue;
	}

	return aDataValue <= leastQualityValue;
}


bool AudioDataSorter::isQualityValue(SORTING_DATA_TYPE aValidated, SORTING_DATA_TYPE aReference)
{
	if (SORT_TYPE_MAXIMUM == mSortType)
	{
		return aValidated >= aReference;
	}

	return aValidated <= aReference;
}


bool AudioDataSorter::isUnQualityValue(SORTING_DATA_TYPE aValidated, SORTING_DATA_TYPE aReference)
{
	if (SORT_TYPE_MAXIMUM == mSortType)
	{
		return aValidated < aReference;
	}

	return aValidated > aReference;
}


bool AudioDataSorter::createExtremList(CAUDIO_U32_t aListLen)
{
	mExtremumList.mpHead = new ExtremeNode_t;
	if(NULL == mExtremumList.mpHead)
	{
		return false;
	}

	ExtremeNode_t* pPre = mExtremumList.mpHead;
	ExtremeNode_t* pCurrent = NULL;
	for(CAUDIO_U32_t idx = 0; idx < aListLen+1; ++idx)
	{
		pCurrent = new ExtremeNode_t;
		if(NULL == pCurrent)
		{
			return false;
		}
		initExtremNode(pCurrent);

		pPre->mpPostNode = pCurrent;
		pCurrent->mpPreNode = pPre;
		pPre = pCurrent;
	}

	mExtremumList.mListLen = aListLen;
	return true;
}

bool AudioDataSorter::deleteExtremList()
{
	ExtremeNode_t* pNode = mExtremumList.mpHead;
	ExtremeNode_t* pNext = NULL;
	while(NULL != pNode)
	{
		pNext = pNode->mpPostNode;
		delete pNode;
		pNode = pNext;
	}

	return true;
}

void AudioDataSorter::initExtremNode(ExtremeNode_t* apNode)
{
	apNode->mHistoryBuffPosition = INVALID_HISTORY_POSITION;
	apNode->mpPostNode = NULL;
	apNode->mpPreNode = NULL;
	apNode->mValue = 0;
}

bool AudioDataSorter::insertExtremNode(ExtremeNode_t* apCurNode, ExtremeNode_t* apNextNode)
{
	ExtremeNode_t* pPre = apNextNode->mpPreNode;
	if(NULL == pPre)
	{
		return false;
	}

	pPre->mpPostNode = apCurNode;
	apCurNode->mpPreNode = pPre;
	apCurNode->mpPostNode = apNextNode;
	apNextNode->mpPreNode = apCurNode;
	return true;
}

bool AudioDataSorter::deleteExtremNode(ExtremeNode_t* apCurNode)
{
	ExtremeNode_t* pPre = apCurNode->mpPreNode;
	ExtremeNode_t* pNext = apCurNode->mpPostNode;

	if(NULL == pPre || NULL == pNext)
	{
		return false;
	}

	pPre->mpPostNode = pNext;
	pNext->mpPreNode = pPre;

	delete apCurNode;

	return true;
}

ExtremeNode_t* AudioDataSorter::getExtremNode(CAUDIO_U32_t aPos)
{
	if(aPos > mExtremumList.mListLen)
	{
		return NULL;
	}

	CAUDIO_U32_t idx=0;
	ExtremeNode_t* pNode = mExtremumList.mpHead;
	while(idx != aPos)
	{
		if(NULL == pNode)
		{
			return false;
		}
		pNode = pNode->mpPostNode;
		++idx;
	}

	return pNode;
}



