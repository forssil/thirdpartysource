/*! \file   IAudioProcess.h
*   \author Keil
*   \date   2014/12/4
*   \brief  Interface of AudioProcessFilter
*/

#ifndef _IAUDIOPROCESS_H_
#define _IAUDIOPROCESS_H_

// Used for placing the audio properties page information(mix\AEC)
struct AUDIO_PROPERTY_PAGE;
// Used for placing the sample rate information(input used\internal process used\output used)
struct SSampleRate;

// {B359F5E7-4119-4C01-89F5-5567DE085960}
DEFINE_GUID(IID_IAUDIOPROCESS,
	0xb359f5e7, 0x4119, 0x4c01, 0x89, 0xf5, 0x55, 0x67, 0xde, 0x8, 0x59, 0x60);


#ifdef __cplusplus
	extern "C" {
#endif
		DECLARE_INTERFACE_(IAudioProcess, IUnknown)
		{
			// get\set audio property page 
			STDMETHOD(GetPropertyPage) (THIS_ AUDIO_PROPERTY_PAGE&) PURE;

			// know whether share channel need to participate in mixing 
			STDMETHOD(GetMixState) (THIS_ bool&) PURE;

			// used to check filter state, if m_State != State_Paused, then return E_FAIL 
			STDMETHOD(GetFilterState) ()PURE;

			// use this function to help outer demo project to control AEC switch, by the way, audio property page will not use it.
			STDMETHOD(SetAecState) (THIS_ bool) PURE;

			// use this function to help outer demo project to control sample rate
			STDMETHOD(SetSampleRate) (THIS_ SSampleRate) PURE;
		};
#ifdef __cplusplus
	}
#endif //DECLARE_INTERFACE_

#endif //_IAUDIOPROCESS_H_