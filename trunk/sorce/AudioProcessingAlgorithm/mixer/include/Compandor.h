/*! \file   Compandor.h
*   \author Keil 
*   \date   2014/11/20
*   \brief  Audio Compandor Algorithm
*/

#ifndef _AUDIOMIXER_COMPANDOR_H_
#define _AUDIOMIXER_COMPANDOR_H_

//class declaration
enum COMPANDOR_PARAMETER_TYPE;
struct COMPANDOR_PARAMETER;

class CCompandor
{
public:
///////////////////////////////////////////////////////////////////////constructor/destructor
	CCompandor(IN const CAUDIO_S32_t samplespersecond, 
			   IN const CAUDIO_S32_t lengthofchannel, 
			   IN AUDIO_MIX_INFO *mChannelInfo);
	~CCompandor(void);

public:
///////////////////////////////////////////////////////////////////////////////public methods
	//! deal with audio compandor 
	bool AudioCompandor(IN OUT AUDIO_DATA_TYPE* memory);
#if 0
	//! set parameter int struct CompandorParameterType
	bool SetParameter(IN const COMPANDOR_PARAMETER_TYPE para, IN const AUDIO_DATA_TYPE data);
	//! get parameter int struct CompandorParameterType
	AUDIO_DATA_TYPE GetParameter(IN COMPANDOR_PARAMETER_TYPE para) const;
#endif

private:
///////////////////////////////////////////////////////////////////////////////private methods	
	//! initial compandor class.
	bool __Init(void);

	//! perform compandor
	void __PerformCompandor(IN OUT AUDIO_DATA_TYPE* idata);
	//! level smooth
	void __LevelSmooth(IN const AUDIO_DATA_TYPE data);
	//! gain update
	AUDIO_DATA_TYPE __GainUpdate();
	//! sample gain smooth
	void __SampleGainSmooth(IN const AUDIO_DATA_TYPE gain);
	//! frame gain smooth
	void __FrameGainSmooth(IN const AUDIO_DATA_TYPE gain);
	//! recursive way
	inline AUDIO_DATA_TYPE __Recursive( IN OUT AUDIO_DATA_TYPE recursivedata, 
										IN const AUDIO_DATA_TYPE newdata, 
										IN const AUDIO_DATA_TYPE ratio1, 
										IN const AUDIO_DATA_TYPE ratio2);
	//! slide window way
	AUDIO_DATA_TYPE __SlideWindow(IN const AUDIO_DATA_TYPE newdata);
	//! initial delay time
	bool __CompandorDelay();

private:
////////////////////////////////////////////////////////////////////////////////////attribute

	CAUDIO_S32_t m_nSamplesPerSecond; //samples per second
	CAUDIO_S32_t m_nSize; //length of data	
	CAUDIO_S64_t m_lIndexOfDelayMemory; //index of delay memory	
	AUDIO_DATA_TYPE *m_pDelayMemory; //memory which to store delay frames
	bool m_bIsInitial; //flag of initialization
	AUDIO_DATA_TYPE m_fEnergy; //energy
	AUDIO_DATA_TYPE m_fEnvelope; //envelope
	CAUDIO_S64_t m_lIndexOfSlideWindowMemory; //index of slide window memory
	AUDIO_DATA_TYPE *m_fSlideWindowMemory; //slide window memory
	AUDIO_DATA_TYPE m_fSmoothGainOfCompandor; //smooth gain of compandor 
	AUDIO_DATA_TYPE m_fPastGainOfCompandor; //past gain of compandor
	CAUDIO_S32_t m_nAttack; //attack time count
	CAUDIO_S32_t m_nRelease; //release time count
	AUDIO_DATA_TYPE m_fDelta; //amplitude of gain variation
	CAUDIO_S32_t m_nReleaseTimeSample; //release time sample
	CAUDIO_S32_t m_nAttackTimeSample; //attack time sample
	AUDIO_MIX_INFO *m_pChannelInfo;
	COMPANDOR_PARAMETER m_compandorParameter; //CompandorParameter
	AUDIO_DATA_TYPE m_fSliceWindowValue;

private:
/////////////////////////////////////////////////////////////////////////////disabled methods
	CCompandor(IN const CCompandor &rhs);
	CCompandor& operator= (IN const CCompandor &rhs);
};
#endif //_AUDIOMIXER_COMPANDOR_H_

