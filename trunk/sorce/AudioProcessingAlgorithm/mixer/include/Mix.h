/*! \file   Mix.h
*   \author Keil 
*   \date   2014/11/19
*   \brief  Audio Mix Algorithm
*/

#ifndef _AUDIOMIXER_MIX_H_
#define _AUDIOMIXER_MIX_H_

//class declaration
class CClip;
enum MIX_PARAMETER_TYPE;

class CMix
{
public:
///////////////////////////////////////////////////////////////////////constructor/destructor
	CMix(IN const CAUDIO_S32_t lengthofchannel, 
		 IN const CAUDIO_U8_t numofchannel, 
		 IN const CAUDIO_S32_t samplespersecond, 
		 IN const CAUDIO_S32_t mixmode = MIX_MODE_USED);
	~CMix(void);
public:
///////////////////////////////////////////////////////////////////////////////public methods
	//! Multichannel audio mixing.
	bool AudioMix(IN const AUDIO_DATA_TYPE** memory, OUT AUDIO_DATA_TYPE* output);
	//! Multichannel audio mixing.
	bool AudioMix(IN const AUDIO_DATA_TYPE** memory, OUT CAUDIO_S16_t* output);
#if 0
	//! Set parameter 
	bool SetParameter(IN const MIX_PARAMETER_TYPE para, IN const AUDIO_DATA_TYPE data);
	//! Get parameter 
	AUDIO_DATA_TYPE GetParameter(IN const MIX_PARAMETER_TYPE para) const; 
#endif

private:
	//! Initial mix class.
	bool __Init(IN const CAUDIO_S32_t size, 
		        IN const CAUDIO_U8_t numofchannel, 
				IN const CAUDIO_S32_t samplespersecond, 
				IN const CAUDIO_S32_t mode);

//[[ function group 1 --- 8 mix modes
	//! Linear superposition method
	void __LinearSuperMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
	//! No-related linear superposition method
	void __NoRelatedLinearSuperMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
	//! Average method
	void __AverageMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
	//! Attenuation factor method
	void __AttenuationFactorMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
	//! Adaptive weighting method
	void __AdaptiveWeightMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
	//! High threshold adaptive weighting method
	void __HighThresholdAdaptiveWeightMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
	//! Truncation method
	void __TruncationMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
	//! Alignment method
	void __AlignmentMix(IN const AUDIO_DATA_TYPE** in, OUT AUDIO_DATA_TYPE* out);
//]] function group 1

	//! gain smooth
	bool __GainSmooth(IN const AUDIO_DATA_TYPE newdata, IN OUT AUDIO_DATA_TYPE& smoothgain);
	//! recursive way
	inline AUDIO_DATA_TYPE __Recursive(IN OUT AUDIO_DATA_TYPE recursivedata, 
									   IN const AUDIO_DATA_TYPE newdata, 
									   IN const AUDIO_DATA_TYPE ratio1, 
									   IN const AUDIO_DATA_TYPE ratio2);

private:
	////////////////////////////////////////////////////////////////////////////////////attribute
	CClip *clip; //instance of class CClip 
	CAUDIO_S32_t m_nSize; //length of data
	CAUDIO_S32_t m_nMode; //mix mode, 0~7
	CAUDIO_S32_t m_nCount; //count of mix channels
	CAUDIO_S32_t m_nSamplesPerSecond; // samples per second
	bool m_bIsInit; //initial flag
	CAUDIO_S8_t m_cNumOfChannel; //num of channel
	CAUDIO_S32_t *m_pMemoryIndex; //index of data memory
	AUDIO_DATA_TYPE *m_pMixing; //output of mixing
	AUDIO_DATA_TYPE *m_pClipping; //output of clipping
	double m_fAlpha; //mode3 , pincers factor
	double m_fStepOfAlpha;//mode3 , step of pincers factor
	AUDIO_DATA_TYPE m_fBeta; //mode5 , high-threshold
	AUDIO_DATA_TYPE m_fEnvelopeFactor; //mode4 ¡¢7, envelope factor
	AUDIO_DATA_TYPE *m_pEnvelope; //mode4 ¡¢7, storage of speech signal envelope
	AUDIO_DATA_TYPE *m_pWeight; //mode4¡¢mode5 , weighting factor array
	AUDIO_DATA_TYPE *m_pSmoothGain; //smooth gain
	AUDIO_DATA_TYPE m_fUpRatioOfSmoothGain; //up ratio of smooth gain
	AUDIO_DATA_TYPE m_fDwRatioOfSmoothGain; //down ratio of smooth gain

private:
	/////////////////////////////////////////////////////////////////////////////disabled methods
	CMix(IN const CMix &rhs);
	CMix& operator= (IN const CMix &rhs);
};
#endif //_AUDIOMIXER_MIX_H_

