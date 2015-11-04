/*! \file     CDCRemoverImpl.h
*   \author   Keil
*   \brief    used to remove the DC component from audio stream.
*   \history  2015/3/10 created CDCRemoverImpl class.
*/

#ifndef _DCREMOVER_IMPL_H_
#define _DCREMOVER_IMPL_H_

#include "DCRemover.h"
#include "AudioModuleImplBase.h"  

class CDCRemoverImpl : public IAudioProcessImplBase
{
public:
	CDCRemoverImpl(IN const int _fs_, IN const int _frame_len_, IN const int _thread_num_);
	~CDCRemoverImpl();

	//! IAudioProcessImplBase API
	virtual int process(IN audio_pro_share &aShareData);

private:
	//! initial function
	bool __Init(void);
	//! Analytical structure
	bool __AnalyticalStructure(IN const audio_pro_share &aShareData);

public:
	
	CAUDIO_U16_t m_nFrameLen;
	CAUDIO_U8_t  m_nThreadNum;
	CAUDIO_U32_t m_nFs;
	AUDIO_DATA_TYPE **m_ppInputBuf;
	bool m_bIsInitSuccess;
	DCRemover *m_cDCRemover;
};
#endif //_DCREMOVER_IMPL_H_

