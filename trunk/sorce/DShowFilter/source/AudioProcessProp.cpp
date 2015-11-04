/*! \file   AudioProcessProp.h
*   \author Keil 
*   \date   2014/12/3
*   \brief  Audio Process Property Page
*/

#pragma warning(disable: 4511 4512)
#include <streams.h>               // include CBasePropertyPage 
#include <windowsx.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <atlstr.h>                // include CString 
#include <initguid.h>              // include definition of GUID 
#include "IAudioProcess.h"         // include IAudioProcess
#include "AudioProcessProp.h"
#include "Resource.h"              // include resource IDs
#include "audiotrace.h"
//! create a thread for refresh the page
DWORD WINAPI ThreadProc(PVOID pParam) 
{
	CAudioProcessProp *pThis = (CAudioProcessProp *)pParam;
	if (NULL == pThis)
	{
		AUDIO_PROCESSING_PRINTF("[AudioProcessFilter.vcxproj - AudioProcessProp.cpp] Input parameters cannot be setted NULL!");
		return -1;
	}
	pThis->ReflectAudioChannelInfo();
	return 0;
}

CUnknown * WINAPI CAudioProcessProp::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	CUnknown *punk = new CAudioProcessProp(lpunk, phr);
	if (punk == NULL)
	{
		*phr = E_OUTOFMEMORY;
	}
	return punk;
}

//! Constructs and initializes a object
CAudioProcessProp::CAudioProcessProp(LPUNKNOWN pUnk, HRESULT *phr)
	: CBasePropertyPage(NAME("Coddy Audio Mix Property Page"), pUnk,IDD_DIALOGBAR2, IDS_INFO)
	, m_nIndexOfChannel(0)
	, m_nIndexOfChannelPre(0)
	, m_bClassToClass(false)
	, m_amInterface(NULL)
{
	ASSERT(phr);
}

// Override CBasePropertyPage method.
// Handles the messages for our property window
BOOL CAudioProcessProp::OnReceiveMessage(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			//access to resource handles
			m_hCompMode = GetDlgItem(hwnd, IDC_EDIT_COMPANDOR_MODE2);
			m_hAutoGain = GetDlgItem(hwnd, IDC_EDIT_GAIN2);
			m_hDelay    = GetDlgItem(hwnd, IDC_EDIT_DELAY2);
			m_hWindow = hwnd;
		
			// add strings to combo box 
			CString name[3] = { _T("capture channel - in"), _T("share channel - in"), _T("capture channel - out")};

			// get parameters from IAudioProcess
			m_amInterface->GetPropertyPage(m_pPropertyPage);
			m_amInterface->GetMixState(m_bClassToClass);
			SendDlgItemMessage(hwnd, IDC_COMBO_NUMOFCHANNEL2, CB_ADDSTRING, -1, (LPARAM)((LPCTSTR)name[0]));
			if (m_bClassToClass)
			{
				SendDlgItemMessage(hwnd, IDC_COMBO_NUMOFCHANNEL2, CB_ADDSTRING, -1, (LPARAM)((LPCTSTR)name[1]));
			}
			SendDlgItemMessage(hwnd, IDC_COMBO_NUMOFCHANNEL2, CB_ADDSTRING, -1, (LPARAM)((LPCTSTR)name[2]));
			SendDlgItemMessage(hwnd, IDC_COMBO_NUMOFCHANNEL2, CB_SETCURSEL, (WPARAM)0, 0);

			CheckRadioButton(hwnd, IDC_RADIO_AEC_ON, IDC_RADIO_AEC_OFF, IDC_RADIO_AEC_OFF);
			CheckRadioButton(hwnd, IDC_RADIO_NR_ON, IDC_RADIO_NR_OFF, IDC_RADIO_NR_OFF);

			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO_AEC_ON:
			m_pPropertyPage.pAudioAECInfo_->bAECOn_ = true;
			SetDirty();
			break;
		case IDC_RADIO_AEC_OFF:
			m_pPropertyPage.pAudioAECInfo_->bAECOn_ = false;
			SetDirty();
			break;
		case IDC_RADIO_NR_ON:
			m_pPropertyPage.pAudioAECInfo_->bNROn_ = true;
			SetDirty();
			break;
		case IDC_RADIO_NR_OFF:
			m_pPropertyPage.pAudioAECInfo_->bNROn_ = false;
			SetDirty();
			break;
		default:
			{
				//get current option in the combo box
				m_nIndexOfChannel = (CAUDIO_S32_t)SendDlgItemMessage(hwnd, IDC_COMBO_NUMOFCHANNEL2, CB_GETCURSEL, (WPARAM)0, 0);
				if (m_nIndexOfChannel != m_nIndexOfChannelPre)
				{
					HANDLE handle1 = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
					CloseHandle(handle1);
					m_nIndexOfChannelPre = m_nIndexOfChannel;
				}
				SetDirty();
				break;
			}
		}

	}
	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
} 

// Override CBasePropertyPage method.
// Notification of which object this property page should display.
// We query the object for the IFltTrace interface.
HRESULT CAudioProcessProp::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IAUDIOPROCESS, (void **)&m_amInterface);

	if (FAILED(hr))
	{
		return E_NOINTERFACE;
	}
	ASSERT(m_amInterface);

	hr = m_amInterface->GetFilterState();
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	return NOERROR;

}

// Override CBasePropertyPage method.
// Release the private interface, release the upstream pin.
HRESULT CAudioProcessProp::OnDisconnect()
{
	// Release of Interface
	if (m_amInterface == NULL)
		return E_UNEXPECTED;
	m_amInterface->Release();
	m_amInterface = NULL;
	return NOERROR;
} 

// We are being activated
HRESULT CAudioProcessProp::OnActivate()
{
	ReflectAudioChannelInfo();
	return NOERROR;
} 

// Changes made should be kept
HRESULT CAudioProcessProp::OnApplyChanges()
{
	EnterAudioChannelInfo();
	return NOERROR;
} 

// Sets m_hrDirtyFlag and notifies the property page site of the change
void CAudioProcessProp::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
	{
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
	}
} 

void CAudioProcessProp::ReflectAudioChannelInfo(void)
{
	//show the audio process properties in the dialog
	TCHAR szRatio[100];
	_stprintf_s(szRatio, _T("%d"), m_pPropertyPage.pAudioMixInfo_[m_nIndexOfChannel]->nCompandorMode_);
	SetWindowText(m_hCompMode, szRatio);
	_stprintf_s(szRatio, _T("%f"), m_pPropertyPage.pAudioMixInfo_[m_nIndexOfChannel]->fGain_);
	SetWindowText(m_hAutoGain, szRatio);
	_stprintf_s(szRatio, _T("%d"), m_pPropertyPage.pAudioMixInfo_[m_nIndexOfChannel]->nChannelDelay_);
	SetWindowText(m_hDelay, szRatio);
	if (m_pPropertyPage.pAudioAECInfo_->bAECOn_)
	{
		CheckDlgButton(m_hWindow, IDC_RADIO_AEC_ON, BST_CHECKED);
		CheckDlgButton(m_hWindow, IDC_RADIO_AEC_OFF, BST_UNCHECKED);
	}
	else

	{
		CheckDlgButton(m_hWindow, IDC_RADIO_AEC_OFF, BST_CHECKED);
		CheckDlgButton(m_hWindow, IDC_RADIO_AEC_ON, BST_UNCHECKED);
	}
	
	if(m_pPropertyPage.pAudioAECInfo_->bNROn_)
	{
		CheckDlgButton(m_hWindow, IDC_RADIO_NR_ON, BST_CHECKED);
		CheckDlgButton(m_hWindow, IDC_RADIO_NR_OFF, BST_UNCHECKED);
	}
	else
	{
		CheckDlgButton(m_hWindow, IDC_RADIO_NR_OFF, BST_CHECKED);
		CheckDlgButton(m_hWindow, IDC_RADIO_NR_ON, BST_UNCHECKED);
	}
}

void CAudioProcessProp::EnterAudioChannelInfo(void)
{
	//set the audio process properties to the dialog
	TCHAR szRatio[100];
	GetWindowText(m_hCompMode, szRatio, 100);
	m_pPropertyPage.pAudioMixInfo_[m_nIndexOfChannel]->nCompandorMode_ = _tstoi(szRatio);
	GetWindowText(m_hAutoGain, szRatio, 100);
	m_pPropertyPage.pAudioMixInfo_[m_nIndexOfChannel]->fGain_ = _tstof(szRatio);
	GetWindowText(m_hDelay, szRatio, 100);
	m_pPropertyPage.pAudioMixInfo_[m_nIndexOfChannel]->nChannelDelay_ = _tstoi(szRatio);
	GetWindowText(m_hCompMode, szRatio, 100);
}