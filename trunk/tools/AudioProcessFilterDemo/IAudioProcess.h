/*! \file   IAudioProcess.h
*   \author Keil
*   \date   2014/12/4
*   \brief  used to implement data interaction between property page and
*           filter, this is an interface.
*/

#ifndef _AUDIOPROCESSFILTER_IAUDIOPROCESS_H_
#define _AUDIOPROCESSFILTER_IAUDIOPROCESS_H_

//forward declaration
struct AUDIO_PROPERTY_PAGE;
struct SSampleRate;

// {B359F5E7-4119-4C01-89F5-5567DE085960}
DEFINE_GUID(IID_IAUDIOPROCESS,
	0xb359f5e7, 0x4119, 0x4c01, 0x89, 0xf5, 0x55, 0x67, 0xde, 0x8, 0x59, 0x60);
//define globally unique ID  {633FC39C-A74F-4DA9-B7A9-3EB68614E8A8}
DEFINE_GUID(CLSID_AudioProcessFilter,
	0x633fc39c, 0xa74f, 0x4da9, 0xb7, 0xa9, 0x3e, 0xb6, 0x86, 0x14, 0xe8, 0xa8);
// {587840C5-7055-4CB4-AC56-E8768E81BF5F}
DEFINE_GUID(CLSID_AudioProcessProp,
	0x587840c5, 0x7055, 0x4cb4, 0xac, 0x56, 0xe8, 0x76, 0x8e, 0x81, 0xbf, 0x5f);

#ifdef __cplusplus
extern "C" {
#endif
	DECLARE_INTERFACE_(IAudioProcess, IUnknown)
	{
		// get audio channel info
		STDMETHOD(GetPropertyPage) (THIS_
			AUDIO_PROPERTY_PAGE&
			) PURE;
		// get audio channel number
		STDMETHOD(GetMixState) (THIS_
			bool&) PURE;
		// used to check filter state, if m_State != State_Paused, then return E_FAIL
		STDMETHOD(GetFilterState) ()PURE;

		// set switch on/off AEC
		// use this function to help outer demo project to control AEC switch, by the way, audio property page will not use it.
		STDMETHOD(SetAecState) (THIS_
			bool) PURE;
		// set default sample rate
		// use this function to help outer demo project to control default sample rate, by the way, audio property page will not use it.
		STDMETHOD(SetSampleRate) (THIS_
			SSampleRate) PURE;

	};
#ifdef __cplusplus
}
#endif //DECLARE_INTERFACE_

#endif //_AUDIOPROCESSFILTER_IAUDIOPROCESS_H_