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

#ifndef _AUDIO_DATA_SORTER_
#define _AUDIO_DATA_SORTER_

#include "codyyAudioCommon.h"
#include "audiotypedef.h"

#define SORTING_DATA_TYPE float
#define INVALID_HISTORY_POSITION 0xFFFFFFFF
#define SORT_TYPE_MAXIMUM 0
#define SORT_TYPE_MINIMUM 1

typedef struct ExtremeNode
{
	CAUDIO_U8_t mHistoryBuffPosition;
	SORTING_DATA_TYPE mValue;
	ExtremeNode* mpPreNode;
	ExtremeNode* mpPostNode;
}ExtremeNode_t;

typedef struct ExtremeList
{
	ExtremeNode_t* mpHead;
	CAUDIO_U32_t mListLen;
}ExtremeList_t;

typedef struct HistoryDataInfo
{
	ExtremeNode_t* mpExtremNode;
	SORTING_DATA_TYPE mValue;
}HistoryDataInfo_t;

typedef struct AudioDataSorterParam
{
	CAUDIO_U32_t mHistoryBufferLen;
	CAUDIO_U32_t mExtremumListLen;
	CAUDIO_U8_t mSortType;
}AudioDataSorterParam_t;

// todo: can only be applied for non negative data
class AudioDataSorter
{
public:
	AudioDataSorter()
	{
		mHistoryBuffer = NULL;
		mHistoryBufferLen = 0;

		mExtremumList.mListLen = 0;
		mExtremumList.mpHead = NULL;
		mpMidExtremNode = NULL;

		mHistoryBufferWritePtr = 0;
		mSortType = SORT_TYPE_MAXIMUM;
	}

	~AudioDataSorter()
	{
		deleteBuffer(mHistoryBuffer);
		deleteExtremList();
	}

	bool init(AudioDataSorterParam_t& aParam);

	

	SORTING_DATA_TYPE getExtremum();

	void updateHistoryBuffer(SORTING_DATA_TYPE aDataValue);

	ExtremeNode_t* updateExtremList(ExtremeNode_t* apInsertedNode, ExtremeNode_t* apDeletedNode);

	ExtremeNode_t* updateExtremList(ExtremeNode_t* apInsertedNode);

	CAUDIO_U32_t searchQualityValueInHistoryBuffer(CAUDIO_U32_t aBeginPos);

	inline bool isExtremum(SORTING_DATA_TYPE aDataValue);

	inline bool isQualityValue(SORTING_DATA_TYPE aValidated, SORTING_DATA_TYPE aReference);

	inline bool isUnQualityValue(SORTING_DATA_TYPE aValidated, SORTING_DATA_TYPE aReference);

	bool createExtremList(CAUDIO_U32_t aListLen);

	bool deleteExtremList();

	void initExtremNode(ExtremeNode_t* aNode);

	bool insertExtremNode(ExtremeNode_t* aCurNode, ExtremeNode_t* aNextNode);

	bool deleteExtremNode(ExtremeNode_t* apCurNode);

	ExtremeNode_t* getExtremNode(CAUDIO_U32_t aPos);

private:
	HistoryDataInfo_t* mHistoryBuffer;
	CAUDIO_U32_t mHistoryBufferLen;
	CAUDIO_U32_t mHistoryBufferWritePtr;

	ExtremeList_t mExtremumList;
	ExtremeNode_t* mpMidExtremNode;

private:
	CAUDIO_U8_t mSortType;
};

#endif //_DATA_SORTER_