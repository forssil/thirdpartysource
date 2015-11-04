/*! \file   AudioProcessProp.h
*   \author Keil 
*   \date   2014/12/3
*   \brief  Audio Process Property Page
*/

#ifndef _AUDIOPROCESSFILTER_AUDIOPROCESSPROPERTY_H_
#define _AUDIOPROCESSFILTER_AUDIOPROCESSPROPERTY_H_

#include "AudioProcessFilterDefs.h"

//forward declaration
struct IAudioProcess;

//create a thread for refreshing the property page
DWORD WINAPI ThreadProc(PVOID pParam) ;

class CAudioProcessProp : public CBasePropertyPage
{
public:
///////////////////////////////////////////////////////////////////////////////public methods
	//! create instance of CAudioMixProp
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

	DECLARE_IUNKNOWN;
	//! get audio channel info
	void ReflectAudioChannelInfo(void);
	//! set audio channel info
	void EnterAudioChannelInfo(void);

private:
///////////////////////////////////////////////////////////////////////////////private methods
	//! constructor
	CAudioProcessProp(LPUNKNOWN lpunk, HRESULT *phr);
	//! Handles the messages for our property window
	BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	//! Notification of which object this property page should display.
	HRESULT OnConnect(IUnknown *pUnknown);
	//! Release the private interface, release the upstream pin.
	HRESULT OnDisconnect();
	//! We are being activated
	HRESULT OnActivate();
	//! Changes made should be kept
	HRESULT OnApplyChanges();
	//! Sets m_hrDirtyFlag and notifies the property page site of the change
	void SetDirty();

private:
	///////////////////////////////////////////////////////////////////////////////////attribute
	IAudioProcess *m_amInterface; //interface of audio process
	CAUDIO_S32_t m_nIndexOfChannel; //index of channel
	CAUDIO_S32_t m_nIndexOfChannelPre; //last index of channel
	AUDIO_PROPERTY_PAGE m_pPropertyPage;
	bool m_bClassToClass;
	//resource handles
	HWND m_hCompMode; //compression mode 0~4
	HWND m_hAutoGain; //automatic gain
	HWND m_hDelay; //delay time(multiple of frames) , if m_fDelayTime = 1 , then length of delay is m_nSize*1
	HWND m_hWindow;

};
#endif //_AUDIOPROCESSFILTER_AUDIOPROCESSPROPERTY_H_

