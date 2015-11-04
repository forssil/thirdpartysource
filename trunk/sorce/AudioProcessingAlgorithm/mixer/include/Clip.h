/*! \file   Clip.h
*   \author Keil 
*   \date   2014/11/18
*   \brief  Amplitude limiter Algorithm
*/

#ifndef _AUDIOMIXER_CLIP_H_
#define _AUDIOMIXER_CLIP_H_

//class declaration
enum CLIP_PARAMETER_TYPE;

class CClip
{
public:
///////////////////////////////////////////////////////////////////////constructor/destructor
	CClip(IN const CAUDIO_S32_t size, IN const CAUDIO_S32_t mode, IN const AUDIO_DATA_TYPE limit);
	~CClip(void);		

public:
///////////////////////////////////////////////////////////////////////////////public methods
	//! audio clip
	bool AudioClip(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out);
#if 0
	//! set parameter
	bool SetParameter(IN const CLIP_PARAMETER_TYPE para, IN const AUDIO_DATA_TYPE data);
	//! get parameter
	AUDIO_DATA_TYPE GetParameter(IN const CLIP_PARAMETER_TYPE para) const ;
#endif

private:
///////////////////////////////////////////////////////////////////////////////private methods	
	//! initial clip class
	bool __Init(IN const CAUDIO_S32_t size, IN const CAUDIO_S32_t mode, IN const AUDIO_DATA_TYPE limit);

//[[ function group 1 --- 4 clip modes
	//! clip mode 0
	void __Clip0(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out);
	//! clip mode 1
	void __Clip1(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out);
	//! clip mode 2
	void __Clip2(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out);
	//! clip mode 3
	void __Clip3(IN const AUDIO_DATA_TYPE *in, OUT AUDIO_DATA_TYPE *out);
//]] function group 1

	//! sign function
	inline CAUDIO_S32_t __Sign(IN const AUDIO_DATA_TYPE value) const;

private:
////////////////////////////////////////////////////////////////////////////////////attribute
	CAUDIO_S32_t m_nSize; //length of data
	CAUDIO_S32_t m_nMode; //clip mode, 0~3
	AUDIO_DATA_TYPE m_fLimit; //0.f<= limit <=1.f
	bool m_bIsInit; //initial flag

private:
/////////////////////////////////////////////////////////////////////////////disabled methods
	CClip(IN const CClip &rhs);
	CClip& operator= (IN const CClip &rhs);
};
#endif //_AUDIOMIXER_CLIP_H_

