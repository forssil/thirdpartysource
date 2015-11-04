
// GPUFilterDemoDlg.h : header file
//

#pragma once

#include "strmif.h"
#include <d3d9.h>
#include <vmr9.h>
#include <map>
#include "control.h"
#include "afxwin.h"
#include "smartptr.h"
#include "IAudioProcess.h"
#include "isynth.h"
#include "audiotypedef.h"

#define FAIL_RET(x) do { if( FAILED( hr = ( x  ) ) ) \
	return hr; } while(0)

// TODO : add definition 
#define DEMO_SAMPLERATE_INTERPROCESSUSED    48000
#define DEMO_SAMPLERATE_ONLINECLASSOUT      48000
#define DEMO_SAMPLERATE_SPEAKEROUT          48000
#define DEMO_SAMPLERATE_RECORDEROUT         48000
#define DEMO_SAMPLERATE_CLASSTOCLASSIN      48000
#define DEMO_SAMPLERATE_ONLINECLASSIN       48000
#define DEMO_SAMPLERATE_ELECTRONICPIANOIN   48000
#define DEMO_SAMPLERATE_MICIN               48000

#define DEMO_CAPTURE_RENDERER_TEST        0    //TODO: this macro is used to test capture filter and renderer filter 2015-8-28 by sainan.
#define DEMO_WINSYNTHFILTER_TEST          0    //TODO: this macro is used to test the original SynthFilter 2015-8-31 by sainan.
#define DEMO_MULINMULOUT_TEST             1    //TODO: this macro is used to test  multiple input and multiple out 2015-09-16 by sainan.

typedef struct tag_Device_Info
{
	TCHAR DisplayName[256];
	int		nDeviceId;
	IMoniker*	pMoniker;

	tag_Device_Info()
	{
		memset(DisplayName, 0, 256 * sizeof(TCHAR));
		nDeviceId = -1;
		pMoniker = NULL;
	}
}Device_Info;

typedef std::map<int,Device_Info*> _mapIDDevice;

// CGPUFilterDemoDlg dialog
class CAudioProcessFilterDemoDlg : public CDialogEx
{
// Construction
public:
	CAudioProcessFilterDemoDlg(CWnd* pParent = NULL);	// standard constructor
	

// Dialog Data
	enum { IDD = IDD_AUDIODEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


	afx_msg void OnEnChangeEditOnlineclassInFile();
	afx_msg void OnEnChangeEditElectronicpanioInFile();
	afx_msg void OnEnChangeEditSpeakOutFile();
	afx_msg void OnEnChangeEditMicInFile();
	afx_msg void OnEnChangeEditOnlineclassOutFile();
	afx_msg void OnEnChangeEditRecorderOutFile();
	afx_msg void OnEnChangeEditClasstoclassInFile();
	afx_msg void OnClose();

	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedButtonClear();

	afx_msg void OnBnClickedRadioClasstoclassInDevice();
	afx_msg void OnBnClickedRadioClasstoclassInFile();
	afx_msg void OnBnClickedRadioMicInDevice();
	afx_msg void OnBnClickedRadioMicInFile();
	afx_msg void OnBnClickedRadioOnlineClassInDevice();
	afx_msg void OnBnClickedRadioOnlineClassInFile();
	afx_msg void OnBnClickedRadioElectronicpianoInDevice();
	afx_msg void OnBnClickedRadioElectronicpianoInFile();
	afx_msg void OnBnClickedRadioOnlineclassOutDevice();
	afx_msg void OnBnClickedRadioOnlineclassOutFile();
	afx_msg void OnBnClickedRadioRecorderOutDevice();
	afx_msg void OnBnClickedRadioRecorderOutFile();
	afx_msg void OnBnClickedRadioSpeackOutDevice();
	afx_msg void OnBnClickedRadioSpeackOutFile();

	afx_msg void OnCbnSelchangeComboClasstoclassInDevice();
	afx_msg void OnCbnSelchangeComboMicInDevice();
	afx_msg void OnCbnSelchangeComboOnlineclassOutDevice();
	afx_msg void OnCbnSelchangeComboRecorderOutDevice();
	afx_msg void OnCbnSelchangeComboOnlineclassInDevice();
	afx_msg void OnCbnSelchangeComboElectronicpanioInDevice();
	afx_msg void OnCbnSelchangeComboSpeakOutDevice();
	

	afx_msg void OnBnClickedButtonClasstoclassInOpenfile();
	afx_msg void OnBnClickedButtonMicInOpenfile();
	afx_msg void OnBnClickedButtonOnlineclassOutOpenfile();
	afx_msg void OnBnClickedButtonRecorderOutOpenfile();
	afx_msg void OnBnClickedButtonOnlineclassInOpenfile();
	afx_msg void OnBnClickedButtonElectronicInOpenfile();
	afx_msg void OnBnClickedButtonSpeackOutOpenfile();


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()

private:
	// 是否启用设置面板
	void _EnbaleSettingPanel(BOOL bEnable);
	HRESULT _InitDevice(GUID pSrcGUID, _mapIDDevice* pMapDevice);
	void	_ReSetData();

	HRESULT	_StartGraph();
	HRESULT _StopGraph();
	HRESULT _SetAllocatorPresenter( IBaseFilter* pStitchFilter,IBaseFilter *filter, HWND window );

	SmartPtr<IGraphBuilder>          m_graph;
	SmartPtr<IMediaControl>          m_mediaControl;


	SmartPtr<IBaseFilter>			 m_pAudioProcessFilter;
	SmartPtr<IAudioProcess>          m_iAudioProcess;

	

	SmartPtr<IBaseFilter>            m_pMicInFilter;
	SmartPtr<IBaseFilter>            m_pClassToClassInFilter;
	SmartPtr<IBaseFilter>            m_pOnlineClassOutFilter;
	SmartPtr<IBaseFilter>            m_pRecorderOutFilter;
	SmartPtr<IBaseFilter>		     m_pOnlineClassInFliter;
	SmartPtr<IBaseFilter>			 m_pElectronicPanioInFliter;
	SmartPtr<IBaseFilter>			 m_pSpeakOutFilter;


	SmartPtr<IFileSourceFilter>      m_iMicInFileSource;
	SmartPtr<IFileSourceFilter>      m_iClassToClassInFileSource;
	SmartPtr<IFileSinkFilter>        m_iOnlineClassOutFileSource;
	SmartPtr<IFileSinkFilter>        m_iRecorderOutFileSource;
	SmartPtr<IFileSourceFilter>	   	 m_iOnlineClassInFileSource;
	SmartPtr<IFileSourceFilter>		 m_iElectronicPanioInFileSource;
	SmartPtr<IFileSinkFilter>		 m_iSpeakOutFileSource;



	SmartPtr<ISynth2>                m_iMicInSynth2;
	SmartPtr<ISynth2>                m_iClassToClassInSynth2;
	SmartPtr<ISynth2>				 m_iOnlineClassSynth2;
	SmartPtr<ISynth2>				 m_iElectronicPanioSynth2;


	_mapIDDevice	m_mapIdAudioInputDevice;
	_mapIDDevice	m_mapIdAudioRenderer;

	bool m_bSelectMicInFile;
	bool m_bSelectClassToClassInFile;
	bool m_bSelectOnlineClassOutFile;
	bool m_bSelectRecorderOutFile;
	bool m_bSelectOnlineClassInFile;
	bool m_bSelectElectronicPanioInFile;
	bool m_bSelectSpeakOutFile;

	CString m_sMicInFileName;
	CString m_sClassToClassInFileName;
	CString m_sOnlineClassOutFileName;
	CString m_sRecorderOutFileName;
	CString m_sOnlineClassInFileName;
	CString m_sElectronicPanioInFileName;
	CString m_sSpeakOutFileName;

	CComboBox m_AudioMicInComBox;
	CComboBox m_AudioClassToClassInComBox;
	CComboBox m_AudioOnlineClassOutComBox;
	CComboBox m_AudioRecordererOutComBox;
	CComboBox m_AudioOnlineClassInComBox;
	CComboBox m_AudioElectronicPanioInConBox;
	CComboBox m_AudioSpeakOutConBox;


	BOOL m_bMicInRadio;
	BOOL m_bClassToClassInRadio;
	BOOL m_bOnlineClassOutRadio;
	BOOL m_bRecorderOutRadio;
	BOOL m_bOnlineClassInRadio;
	BOOL m_bElectronicPanioInRadio;
	BOOL m_bSpeakOutRadio;

	CEdit     m_AudioBufferSizeEdit;
	CButton   m_AECCheckButton;

	SSampleRate m_sSampleRate;
	CTime m_ctime;
	

};
