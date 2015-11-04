
// GPUFilterDemoDlg.cpp : implementation file
//

//#include "vld.h"
#include "stdafx.h"
#include <cstring>
#include "strsafe.h"
#include "AudioProcessFilterDemo.h"
#include "AudioProcessFilterDemoDlg.h"
#include "afxdialogex.h"
#include "Utility.h"
#include "uuids.h"
#include "atlconv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DWORD_PTR                       g_userId = 0xACDCACDC;

//{633FC39C-A74F-4DA9-B7A9-3EB68614E8A8}
static const GUID CLSID_AudioProcessFilter = 
{ 0x633fc39c, 0xa74f, 0x4da9, { 0xb7, 0xa9, 0x3e, 0xb6, 0x86, 0x14, 0xe8, 0xa8 } };

//{79A98DE0-BC00-11ce-AC2E-444553540000}
static const GUID CLSID_SynthFilter =
{ 0x79a98de0, 0xbc00, 0x11ce, { 0xac, 0x2e, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

#if DEMO_WINSYNTHFILTER_TEST
//{D3588AB0-0781-11CE-B03A-0020AF0BA770}
static const GUID CLSID_WinSynthFilter=
{ 0xd3588ab0, 0x0781, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };

#else

#endif //DEMO_WINSYNTHFILTER_TEST

// { 36a5f770-fe4c-11ce-a8ed-00aa002feab5 }
static const GUID CLSID_Dump = 
{ 0x36a5f770, 0xfe4c, 0x11ce, {0xa8, 0xed, 0x00, 0xaa, 0x00, 0x2f, 0xea, 0xb5 } };

// {B359F5E7-4119-4C01-89F5-5567DE085960}
static const GUID IID_IAUDIOPROCESS =
{ 0xb359f5e7, 0x4119, 0x4c01, { 0x89, 0xf5, 0x55, 0x67, 0xde, 0x08, 0x59, 0x60 } };

// {00487A78-D875-44b0-ADBB-DECA9CDB51FC}
static const GUID IID_ISynth2 = 
{ 0x487a78, 0xd875, 0x44b0, { 0xad, 0xbb, 0xde, 0xca, 0x9c, 0xdb, 0x51, 0xfc } };




// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{ 
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


// CGPUFilterDemoDlg dialog




CAudioProcessFilterDemoDlg::CAudioProcessFilterDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAudioProcessFilterDemoDlg::IDD, pParent)
	, m_bSelectClassToClassInFile(false)
	, m_bSelectMicInFile(false)
	, m_bSelectOnlineClassOutFile(false)
	, m_bSelectRecorderOutFile(false)
	, m_bSelectOnlineClassInFile(false)
	, m_bSelectElectronicPanioInFile(false)
	, m_bSelectSpeakOutFile(false)

	, m_bMicInRadio(FALSE)
	, m_bClassToClassInRadio(FALSE)
	, m_bOnlineClassOutRadio(FALSE)
	, m_bRecorderOutRadio(FALSE)
	, m_bOnlineClassInRadio(FALSE)
	, m_bElectronicPanioInRadio(FALSE)
	, m_bSpeakOutRadio(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CAudioProcessFilterDemoDlg::DoDataExchange(CDataExchange* pDX) //将控件的值传给变量
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUFFER_SIZE, m_AudioBufferSizeEdit);
	DDX_Control(pDX, IDC_CHECK_AEC_SWITCH, m_AECCheckButton);

	DDX_Control(pDX, IDC_COMBO_ClassToClass_IN_DEVICE, m_AudioClassToClassInComBox);
	DDX_Control(pDX, IDC_COMBO_MIC_IN_DEVICE, m_AudioMicInComBox);
	DDX_Control(pDX, IDC_COMBO_ONLINECLASS_OUT_DEVICE, m_AudioOnlineClassOutComBox);
	DDX_Control(pDX, IDC_COMBO_RECORDER_OUT_DEVICE, m_AudioRecordererOutComBox);
	DDX_Control(pDX, IDC_COMBO_OnlineClass_IN_DEVICE, m_AudioOnlineClassInComBox);
	DDX_Control(pDX, IDC_COMBO_ElectronicPanio_IN_DEVICE, m_AudioElectronicPanioInConBox);
	DDX_Control(pDX, IDC_COMBO_Speak_OUT_DEVICE,m_AudioSpeakOutConBox);

	DDX_Radio(pDX, IDC_RADIO_CLASSTOCLASS_IN_DEVICE, m_bClassToClassInRadio);
	DDX_Radio(pDX, IDC_RADIO_MIC_IN_DEVICE, m_bMicInRadio);
	DDX_Radio(pDX, IDC_RADIO_ONLINECLASS_OUT_DEVICE, m_bOnlineClassOutRadio);
	DDX_Radio(pDX, IDC_RADIO_RECORDER_OUT_DEVICE, m_bRecorderOutRadio);
	
	
	DDX_Radio(pDX,IDC_RADIO_SPEACK_OUT_DEVICE,m_bSpeakOutRadio);
	DDX_Radio(pDX, IDC_RADIO_ELECTRONICPIANO_IN_DEVICE, m_bElectronicPanioInRadio);
	DDX_Radio(pDX, IDC_RADIO_ONLINECLASS_IN_DEVICE, m_bOnlineClassInRadio);
}

BEGIN_MESSAGE_MAP(CAudioProcessFilterDemoDlg, CDialogEx)//添加自定义消息响应
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_BTN_START, &CAudioProcessFilterDemoDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CAudioProcessFilterDemoDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CAudioProcessFilterDemoDlg::OnBnClickedButtonClear)

	ON_BN_CLICKED(IDC_RADIO_CLASSTOCLASS_IN_DEVICE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioClasstoclassInDevice)
	ON_BN_CLICKED(IDC_RADIO_CLASSTOCLASS_IN_FILE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioClasstoclassInFile)
	ON_BN_CLICKED(IDC_RADIO_MIC_IN_DEVICE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioMicInDevice)
	ON_BN_CLICKED(IDC_RADIO_MIC_IN_FILE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioMicInFile)
	ON_BN_CLICKED(IDC_RADIO_ONLINECLASS_OUT_DEVICE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineclassOutDevice)
	ON_BN_CLICKED(IDC_RADIO_ONLINECLASS_OUT_FILE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineclassOutFile)
	ON_BN_CLICKED(IDC_RADIO_RECORDER_OUT_DEVICE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioRecorderOutDevice)
	ON_BN_CLICKED(IDC_RADIO_RECORDER_OUT_FILE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioRecorderOutFile)
	ON_BN_CLICKED(IDC_RADIO_ONLINECLASS_IN_DEVICE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineClassInDevice)
	ON_BN_CLICKED(IDC_RADIO_ONLINECLASS_IN_FILE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineClassInFile)
	ON_BN_CLICKED(IDC_RADIO_ELECTRONICPIANO_IN_DEVICE,&CAudioProcessFilterDemoDlg::OnBnClickedRadioElectronicpianoInDevice)
	ON_BN_CLICKED(IDC_RADIO_ELECTRONICPIANO_IN_FILE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioElectronicpianoInFile)
	ON_BN_CLICKED(IDC_RADIO_SPEACK_OUT_DEVICE,&CAudioProcessFilterDemoDlg::OnBnClickedRadioSpeackOutDevice)
	ON_BN_CLICKED(IDC_RADIO_SPEACK_OUT_FILE, &CAudioProcessFilterDemoDlg::OnBnClickedRadioSpeackOutFile)

	ON_CBN_SELCHANGE(IDC_COMBO_ClassToClass_IN_DEVICE, &CAudioProcessFilterDemoDlg::OnCbnSelchangeComboClasstoclassInDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_MIC_IN_DEVICE, &CAudioProcessFilterDemoDlg::OnCbnSelchangeComboMicInDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_ONLINECLASS_OUT_DEVICE, &CAudioProcessFilterDemoDlg::OnCbnSelchangeComboOnlineclassOutDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_RECORDER_OUT_DEVICE, &CAudioProcessFilterDemoDlg::OnCbnSelchangeComboRecorderOutDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_ElectronicPanio_IN_DEVICE, &CAudioProcessFilterDemoDlg::OnCbnSelchangeComboElectronicpanioInDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_OnlineClass_IN_DEVICE, &CAudioProcessFilterDemoDlg::OnCbnSelchangeComboOnlineclassInDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_Speak_OUT_DEVICE, &CAudioProcessFilterDemoDlg::OnCbnSelchangeComboSpeakOutDevice)

	ON_BN_CLICKED(IDC_BUTTON_CLASSTOCLASS_IN_OPENFILE, &CAudioProcessFilterDemoDlg::OnBnClickedButtonClasstoclassInOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_MIC_IN_OPENFILE, &CAudioProcessFilterDemoDlg::OnBnClickedButtonMicInOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_ONLINECLASS_OUT_OPENFILE, &CAudioProcessFilterDemoDlg::OnBnClickedButtonOnlineclassOutOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_RECORDER_OUT_OPENFILE, &CAudioProcessFilterDemoDlg::OnBnClickedButtonRecorderOutOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_ONLINECLASS_IN_OPENFILE, &CAudioProcessFilterDemoDlg::OnBnClickedButtonOnlineclassInOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_ELECTRONIC_IN_OPENFILE, &CAudioProcessFilterDemoDlg::OnBnClickedButtonElectronicInOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_SPEACK_OUT_OPENFILE, &CAudioProcessFilterDemoDlg::OnBnClickedButtonSpeackOutOpenfile)

	ON_EN_CHANGE(IDC_EDIT_CLASSTOCLASS_IN_FILE, &CAudioProcessFilterDemoDlg::OnEnChangeEditClasstoclassInFile)
	ON_EN_CHANGE(IDC_EDIT_OnlineClass_IN_FILE, &CAudioProcessFilterDemoDlg::OnEnChangeEditOnlineclassInFile)
	ON_EN_CHANGE(IDC_EDIT_ElectronicPanio_IN_FILE, &CAudioProcessFilterDemoDlg::OnEnChangeEditElectronicpanioInFile)
	ON_EN_CHANGE(IDC_EDIT_Speak_Out_FILE, &CAudioProcessFilterDemoDlg::OnEnChangeEditSpeakOutFile)
	ON_EN_CHANGE(IDC_EDIT_MIC_IN_FILE, &CAudioProcessFilterDemoDlg::OnEnChangeEditMicInFile)
	ON_EN_CHANGE(IDC_EDIT_ONLINECLASS_OUT_FILE, &CAudioProcessFilterDemoDlg::OnEnChangeEditOnlineclassOutFile)
	ON_EN_CHANGE(IDC_EDIT_RECORDER_OUT_FILE, &CAudioProcessFilterDemoDlg::OnEnChangeEditRecorderOutFile)
END_MESSAGE_MAP()


// CGPUFilterDemoDlg message handlers

BOOL CAudioProcessFilterDemoDlg::OnInitDialog()//与类对象相关的window窗体函数控件初始化问题
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	// 初始化com对象
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	SetTimer(1,1000,NULL);
	
	_ReSetData();
	_EnbaleSettingPanel(TRUE);
	// 初始化视频设备
	_InitDevice(CLSID_AudioInputDeviceCategory, &m_mapIdAudioInputDevice);
	_InitDevice(CLSID_AudioRendererCategory, &m_mapIdAudioRenderer);
	//
	m_AudioClassToClassInComBox.InsertString(0, __T("请选择..."));
	m_AudioMicInComBox.InsertString(0, __T("请选择..."));
	m_AudioOnlineClassOutComBox.InsertString(0, __T("请选择..."));
	m_AudioRecordererOutComBox.InsertString(0, __T("请选择..."));
	m_AudioOnlineClassInComBox.InsertString(0, __T("请选择..."));
	m_AudioElectronicPanioInConBox.InsertString(0, __T("请选择..."));
	m_AudioSpeakOutConBox.InsertString(0, __T("请选择..."));

	_mapIDDevice::iterator iter_input = m_mapIdAudioInputDevice.begin();
	while (iter_input != m_mapIdAudioInputDevice.end())
	{
		m_AudioClassToClassInComBox.InsertString(iter_input->first + 1, iter_input->second->DisplayName);
		m_AudioMicInComBox.InsertString(iter_input->first + 1, iter_input->second->DisplayName);
		m_AudioOnlineClassInComBox.InsertString(iter_input->first + 1, iter_input->second->DisplayName);
		m_AudioElectronicPanioInConBox.InsertString(iter_input->first + 1, iter_input->second->DisplayName);
		iter_input++;
	}
	_mapIDDevice::iterator iter_renderer = m_mapIdAudioRenderer.begin();
	while (iter_renderer != m_mapIdAudioRenderer.end())
	{
		m_AudioOnlineClassOutComBox.InsertString(iter_renderer->first + 1, iter_renderer->second->DisplayName);
		m_AudioRecordererOutComBox.InsertString(iter_renderer->first + 1, iter_renderer->second->DisplayName);
		m_AudioSpeakOutConBox.InsertString(iter_renderer->first + 1, iter_renderer->second->DisplayName);
		iter_renderer++;
	}
	m_AudioClassToClassInComBox.SetCurSel(0);
	m_AudioMicInComBox.SetCurSel(0);
	m_AudioOnlineClassInComBox.SetCurSel(0);
	m_AudioElectronicPanioInConBox.SetCurSel(0);
	m_AudioOnlineClassOutComBox.SetCurSel(0);
	m_AudioRecordererOutComBox.SetCurSel(0);
	m_AudioSpeakOutConBox.SetCurSel(0);

	m_sClassToClassInFileName.Empty();
	m_sMicInFileName.Empty();
	m_sOnlineClassOutFileName.Empty();
	m_sRecorderOutFileName.Empty();
	m_sElectronicPanioInFileName.Empty();
	m_sOnlineClassInFileName.Empty();
	m_sSpeakOutFileName.Empty();
	

	m_bMicInRadio = 0;
	m_bClassToClassInRadio = 0;
	m_bOnlineClassOutRadio = 0;
	m_bRecorderOutRadio = 0;
	m_bOnlineClassInRadio - 0;
	m_bElectronicPanioInRadio = 0;
	m_bSpeakOutRadio = 0;

	m_AudioBufferSizeEdit.SetWindowText(__T("10"));
	m_AECCheckButton.SetCheck(BST_CHECKED);

	GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_MIC_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_Speak_Out_FILE)->SetWindowText(_T("请选择wave文件..."));

	UpdateData(FALSE);

	// TODO : set sample rate  SSampleRate m_sSampleRate
	m_sSampleRate.nSampleRate_ClassToClassIn = DEMO_SAMPLERATE_CLASSTOCLASSIN;
	m_sSampleRate.nSampleRate_OnLineClassIn = DEMO_SAMPLERATE_ONLINECLASSIN;
	m_sSampleRate.nSampleRate_ElectronicPianoIn = DEMO_SAMPLERATE_ELECTRONICPIANOIN;
	m_sSampleRate.nSampleRate_MicIn = DEMO_SAMPLERATE_MICIN;
	m_sSampleRate.nSampleRate_InterProcessUsed = DEMO_SAMPLERATE_INTERPROCESSUSED;
	m_sSampleRate.nSampleRate_OnlineClassOut = DEMO_SAMPLERATE_ONLINECLASSOUT;
	m_sSampleRate.nSampleRate_RecorderOut = DEMO_SAMPLERATE_RECORDEROUT;
	m_sSampleRate.nSampleRate_SpeakerOut = DEMO_SAMPLERATE_SPEAKEROUT;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAudioProcessFilterDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAudioProcessFilterDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAudioProcessFilterDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 是否启用设置面板
void CAudioProcessFilterDemoDlg::_EnbaleSettingPanel(BOOL bEnable)
{
	if(bEnable == TRUE)
	{
		GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CLEAR)->EnableWindow(TRUE);

		m_AudioBufferSizeEdit.EnableWindow(TRUE);
		m_AECCheckButton.EnableWindow(TRUE);

		// set Mic 
		if (m_bSelectMicInFile)
		{
			GetDlgItem(IDC_BUTTON_MIC_IN_OPENFILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_MIC_IN_FILE)->EnableWindow(TRUE);
			m_AudioMicInComBox.EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_MIC_IN_OPENFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_MIC_IN_FILE)->EnableWindow(FALSE);
			m_AudioMicInComBox.EnableWindow(TRUE);
		}

		// set ClassToClass
		if (m_bSelectClassToClassInFile)
		{
			GetDlgItem(IDC_BUTTON_CLASSTOCLASS_IN_OPENFILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->EnableWindow(TRUE);
			m_AudioClassToClassInComBox.EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_CLASSTOCLASS_IN_OPENFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->EnableWindow(FALSE);
			m_AudioClassToClassInComBox.EnableWindow(TRUE);
		}
		
		// set OnlineClass
		if (m_bSelectOnlineClassOutFile)
		{
			GetDlgItem(IDC_BUTTON_ONLINECLASS_OUT_OPENFILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->EnableWindow(TRUE);
			m_AudioOnlineClassOutComBox.EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_ONLINECLASS_OUT_OPENFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->EnableWindow(FALSE);
			m_AudioOnlineClassOutComBox.EnableWindow(TRUE);
		}
		
		// set Recorder
		if (m_bSelectRecorderOutFile)
		{
			GetDlgItem(IDC_BUTTON_RECORDER_OUT_OPENFILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->EnableWindow(TRUE);
			m_AudioRecordererOutComBox.EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_RECORDER_OUT_OPENFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->EnableWindow(FALSE);
			m_AudioRecordererOutComBox.EnableWindow(TRUE);
		}

		GetDlgItem(IDC_RADIO_CLASSTOCLASS_IN_DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_CLASSTOCLASS_IN_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_MIC_IN_DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_MIC_IN_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ONLINECLASS_OUT_DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ONLINECLASS_OUT_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_RECORDER_OUT_DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_RECORDER_OUT_FILE)->EnableWindow(TRUE);
		
#if DEMO_MULINMULOUT_TEST

		// set OnlineClass in
		if (m_bSelectOnlineClassInFile)
		{
			GetDlgItem(IDC_BUTTON_ONLINECLASS_IN_OPENFILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->EnableWindow(TRUE);
			m_AudioOnlineClassInComBox.EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_ONLINECLASS_IN_OPENFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->EnableWindow(FALSE);
			m_AudioOnlineClassInComBox.EnableWindow(TRUE);
		}

		// set ElectronicPanio in
		if (m_bSelectElectronicPanioInFile)
		{
			GetDlgItem(IDC_BUTTON_ELECTRONIC_IN_OPENFILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->EnableWindow(TRUE);
			m_AudioElectronicPanioInConBox.EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_ELECTRONIC_IN_OPENFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->EnableWindow(FALSE);
			m_AudioElectronicPanioInConBox.EnableWindow(TRUE);
		}

		// set Speak out
		if (m_bSelectSpeakOutFile)
		{
			GetDlgItem(IDC_BUTTON_SPEACK_OUT_OPENFILE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_Speak_Out_FILE)->EnableWindow(TRUE);
			m_AudioSpeakOutConBox.EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_SPEACK_OUT_OPENFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_Speak_Out_FILE)->EnableWindow(FALSE);
			m_AudioSpeakOutConBox.EnableWindow(TRUE);
		}

		GetDlgItem(IDC_RADIO_ONLINECLASS_IN_DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ONLINECLASS_IN_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ELECTRONICPIANO_IN_DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_ELECTRONICPIANO_IN_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_SPEACK_OUT_DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_SPEACK_OUT_FILE)->EnableWindow(TRUE);

#else

#endif  // DEMO_MULINMULOUT_TEST

	}
	else
	{
		GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_CLEAR)->EnableWindow(FALSE);

		m_AudioBufferSizeEdit.EnableWindow(FALSE);
		m_AECCheckButton.EnableWindow(FALSE);

		GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_MIC_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_Speak_Out_FILE)->EnableWindow(FALSE);

		GetDlgItem(IDC_BUTTON_CLASSTOCLASS_IN_OPENFILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_MIC_IN_OPENFILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ONLINECLASS_OUT_OPENFILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_RECORDER_OUT_OPENFILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ONLINECLASS_IN_OPENFILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ELECTRONIC_IN_OPENFILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SPEACK_OUT_OPENFILE)->EnableWindow(FALSE);
		
		GetDlgItem(IDC_RADIO_CLASSTOCLASS_IN_DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_CLASSTOCLASS_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_MIC_IN_DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_MIC_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ONLINECLASS_OUT_DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ONLINECLASS_OUT_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_RECORDER_OUT_DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_RECORDER_OUT_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ONLINECLASS_IN_DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ONLINECLASS_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ELECTRONICPIANO_IN_DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_ELECTRONICPIANO_IN_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_SPEACK_OUT_DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_SPEACK_OUT_FILE)->EnableWindow(FALSE);

		m_AudioMicInComBox.EnableWindow(FALSE);
		m_AudioClassToClassInComBox.EnableWindow(FALSE);
		m_AudioOnlineClassOutComBox.EnableWindow(FALSE);
		m_AudioRecordererOutComBox.EnableWindow(FALSE);
		m_AudioOnlineClassInComBox.EnableWindow(FALSE);
		m_AudioElectronicPanioInConBox.EnableWindow(FALSE);
		m_AudioSpeakOutConBox.EnableWindow(FALSE);

	}
}

// 初始化硬件列表
HRESULT CAudioProcessFilterDemoDlg::_InitDevice(GUID pSrcGUID, _mapIDDevice *pMapDevice)
{
	// enumerate all video capture devices
	ICreateDevEnum  *pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)	
		return false;

	IEnumMoniker *pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(pSrcGUID, &pEm, 0);
	if (hr != NOERROR)	
		return false;

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	int index = 0;
	int deviceid = 0;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				LPOLESTR pszDisplayName = NULL;
				if( SUCCEEDED( hr=pM->GetDisplayName( NULL, NULL, &pszDisplayName ) ) )
				{
					Device_Info* pInfo = new Device_Info;
					pInfo->nDeviceId = index;
					StringCchCopy(pInfo->DisplayName, 256, var.bstrVal);
					pInfo->pMoniker = pM;
					pInfo->pMoniker->AddRef();

					pMapDevice->insert(std::make_pair(deviceid++,pInfo));
					
					CoTaskMemFree((LPVOID)pszDisplayName);
					//pM->BindToObject(0, 0, IID_IBaseFilter, (void**)ppFilter);
				}
				SysFreeString(var.bstrVal);
			}
			if(pBag)
				pBag->Release();
		}
		if(pM)
			pM->Release();
		index++;
	}

	return hr;
}

void CAudioProcessFilterDemoDlg::_ReSetData()
{
	m_mediaControl = NULL; 
	m_graph        = NULL;


	m_pAudioProcessFilter = NULL;
	m_iAudioProcess = NULL;

	
	m_pMicInFilter = NULL;
	m_pClassToClassInFilter = NULL;
	m_pOnlineClassOutFilter = NULL;
	m_pRecorderOutFilter = NULL;
	m_pOnlineClassInFliter = NULL;
	m_pElectronicPanioInFliter = NULL;
	m_pSpeakOutFilter = NULL;

	m_iMicInFileSource = NULL;
	m_iClassToClassInFileSource = NULL;
	m_iOnlineClassOutFileSource = NULL;
	m_iRecorderOutFileSource = NULL;
	m_iOnlineClassInFileSource= NULL;
	m_iElectronicPanioInFileSource = NULL;
	m_iSpeakOutFileSource = NULL;


	m_iMicInSynth2 = NULL;
	m_iClassToClassInSynth2 = NULL;
	m_iOnlineClassSynth2 = NULL;
	m_iElectronicPanioSynth2=NULL;


}

// 开始渲染
HRESULT	CAudioProcessFilterDemoDlg::_StartGraph()
{
	HRESULT hr = E_FAIL;

	// Clear DirectShow interfaces (COM smart pointers)
	_StopGraph();

	SetTimer(2,1000,NULL);
  	m_ctime = CTime::GetCurrentTime();

	if (NULL != m_mediaControl)
	{
		hr = m_mediaControl->Run();
		if(FAILED(hr))
		{
			char errorlog[64];
			sprintf_s(errorlog,"run error : code = 0X%x",hr);
			MessageBoxA(NULL,errorlog,"Error", MB_OK);
		}

		return hr;
	}

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_graph);
	if (FAILED(hr))
	{
		return hr;
	}
	
	BOOL bFlag = TRUE;
	int nAudioValue = 0;
	int nAudioIndex = 0;
	LPCOLESTR lpwszFileName = NULL;


#if DEMO_WINSYNTHFILTER_TEST

	if (SUCCEEDED(hr))
	{
		nAudioValue = GetDlgItemInt(IDC_BUFFER_SIZE, &bFlag, TRUE);

		// add Mic in filter
		if (0 == m_bMicInRadio)
		{
			nAudioIndex = m_AudioMicInComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioInputDevice.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioInputDevice.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pMicInFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pMicInFilter, L"MicIn filter")))
					{
						return hr;
					}
					if (FAILED(CUtility::_set_audio_buffer(m_pMicInFilter, nAudioValue, m_sSampleRate.nSampleRate_MicIn)))
					{
						char errorlog[64];
						sprintf_s(errorlog, "set audio buffer size error : code = 0X%x", hr);
						MessageBoxA(NULL, errorlog, "Error", MB_OK);
					}
				}
			}
		}
		else 
		{
			if (m_bSelectMicInFile)
			{
				lpwszFileName = NULL;


				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_WinSynthFilter, L"MicIn filter", &m_pMicInFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pMicInFilter->QueryInterface(IID_IFileSourceFilter, reinterpret_cast<void**>(&m_iMicInFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sMicInFileName.GetBuffer();
				if (FAILED(hr = m_iMicInFileSource->Load(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}

				//if (FAILED(hr = m_pMicInFilter->QueryInterface(IID_ISynth2, reinterpret_cast<void**>(&m_iMicInSynth2))))
				//{
				//	return hr;
				//}
				//if (FAILED(hr = m_iMicInSynth2->Set_TimeInterval(nAudioValue)))
				//{
				//	return hr;
				//}
			}
		}

		// add ClassToClass in filter
		if (0 == m_bClassToClassInRadio)
		{
			nAudioIndex = m_AudioClassToClassInComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioInputDevice.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioInputDevice.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pClassToClassInFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pClassToClassInFilter, L"ClassToClassIn filter")))
					{
						return hr;
					}
					if (FAILED(CUtility::_set_audio_buffer(m_pClassToClassInFilter, nAudioValue, m_sSampleRate.nSampleRate_ClassToClassIn)))
					{
						char errorlog[64];
						sprintf_s(errorlog, "set audio buffer size error : code = 0X%x", hr);
						MessageBoxA(NULL, errorlog, "Error", MB_OK);
					}
				}
			}
		}
		else 
		{
			if (m_bSelectClassToClassInFile)
			{
				lpwszFileName = NULL;


				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_WinSynthFilter, L"ClassToClassIn filter", &m_pClassToClassInFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pClassToClassInFilter->QueryInterface(IID_IFileSourceFilter, reinterpret_cast<void**>(&m_iClassToClassInFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sClassToClassInFileName.GetBuffer();
				if (FAILED(hr = m_iClassToClassInFileSource->Load(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}

				/*if (FAILED(hr = m_pClassToClassInFilter->QueryInterface(IID_ISynth2, reinterpret_cast<void**>(&m_iClassToClassInSynth2))))
				{
					return hr;
				}
				if (FAILED(hr = m_iClassToClassInSynth2->Set_TimeInterval(nAudioValue)))
				{
					return hr;
				}*/
			}
		}

		// add OnlineClass out filter
		if (0 == m_bOnlineClassOutRadio)
		{
			nAudioIndex = m_AudioOnlineClassOutComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioRenderer.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioRenderer.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pOnlineClassOutFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pOnlineClassOutFilter, L"OnLineClassOut filter")))
					{
						return hr;
					}
				}
			}
		}
		else 
		{
			if (m_bSelectOnlineClassOutFile)
			{
				lpwszFileName = NULL;

				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_Dump, L"OnLineClassOut filter", &m_pOnlineClassOutFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pOnlineClassOutFilter->QueryInterface(IID_IFileSinkFilter, reinterpret_cast<void**>(&m_iOnlineClassOutFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sOnlineClassOutFileName.GetBuffer();
				if (FAILED(hr = m_iOnlineClassOutFileSource->SetFileName(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}
			}
		}

		// add Recorder out filter
		if (0 == m_bRecorderOutRadio)
		{
			nAudioIndex = m_AudioRecordererOutComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioRenderer.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioRenderer.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pRecorderOutFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pRecorderOutFilter, L"RecorderOut filter")))
					{
						return hr;
					}
				}
			}
		}
		else 
		{
			if (m_bSelectRecorderOutFile)
			{
				lpwszFileName = NULL;

				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_Dump, L"RecorderOut filter", &m_pRecorderOutFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pRecorderOutFilter->QueryInterface(IID_IFileSinkFilter, reinterpret_cast<void**>(&m_iRecorderOutFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sRecorderOutFileName.GetBuffer();
				if (FAILED(hr = m_iRecorderOutFileSource->SetFileName(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}
			}

#else

	if (SUCCEEDED(hr))
	{
		nAudioValue = GetDlgItemInt(IDC_BUFFER_SIZE, &bFlag, TRUE);

		// add Mic in filter
		if (0 == m_bMicInRadio)
		{
			nAudioIndex = m_AudioMicInComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioInputDevice.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioInputDevice.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pMicInFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pMicInFilter, L"MicIn filter")))
					{
						return hr;
					}
					if (FAILED(CUtility::_set_audio_buffer(m_pMicInFilter, nAudioValue, m_sSampleRate.nSampleRate_MicIn)))
					{
						char errorlog[64];
						sprintf_s(errorlog, "set audio buffer size error : code = 0X%x", hr);
						MessageBoxA(NULL, errorlog, "Error", MB_OK);
					}
				}
			}
		}
		else
		{
			if (m_bSelectMicInFile)
			{
				lpwszFileName = NULL;


				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_SynthFilter, L"MicIn filter", &m_pMicInFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pMicInFilter->QueryInterface(IID_IFileSourceFilter, reinterpret_cast<void**>(&m_iMicInFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sMicInFileName.GetBuffer();
				if (FAILED(hr = m_iMicInFileSource->Load(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}

				if (FAILED(hr = m_pMicInFilter->QueryInterface(IID_ISynth2, reinterpret_cast<void**>(&m_iMicInSynth2))))
				{
					return hr;
				}
				if (FAILED(hr = m_iMicInSynth2->Set_TimeInterval(nAudioValue)))
				{
					return hr;
				}
			}
		}

		// add ClassToClass in filter
		if (0 == m_bClassToClassInRadio)
		{
			nAudioIndex = m_AudioClassToClassInComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioInputDevice.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioInputDevice.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pClassToClassInFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pClassToClassInFilter, L"ClassToClassIn filter")))
					{
						return hr;
					}
					if (FAILED(CUtility::_set_audio_buffer(m_pClassToClassInFilter, nAudioValue, m_sSampleRate.nSampleRate_ClassToClassIn)))
					{
						char errorlog[64];
						sprintf_s(errorlog, "set audio buffer size error : code = 0X%x", hr);
						MessageBoxA(NULL, errorlog, "Error", MB_OK);
					}
				}
			}
		}
		else
		{
			if (m_bSelectClassToClassInFile)
			{
				lpwszFileName = NULL;


				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_SynthFilter, L"ClassToClassIn filter", &m_pClassToClassInFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pClassToClassInFilter->QueryInterface(IID_IFileSourceFilter, reinterpret_cast<void**>(&m_iClassToClassInFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sClassToClassInFileName.GetBuffer();
				if (FAILED(hr = m_iClassToClassInFileSource->Load(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}

				if (FAILED(hr = m_pClassToClassInFilter->QueryInterface(IID_ISynth2, reinterpret_cast<void**>(&m_iClassToClassInSynth2))))
				{
					return hr;
				}
				if (FAILED(hr = m_iClassToClassInSynth2->Set_TimeInterval(nAudioValue)))
				{
					return hr;
				}
			}
		}

		// add OnlineClass out filter
		if (0 == m_bOnlineClassOutRadio)
		{
			nAudioIndex = m_AudioOnlineClassOutComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioRenderer.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioRenderer.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pOnlineClassOutFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pOnlineClassOutFilter, L"OnLineClassOut filter")))
					{
						return hr;
					}
				}
			}
		}
		else
		{
			if (m_bSelectOnlineClassOutFile)
			{
				lpwszFileName = NULL;

				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_Dump, L"OnLineClassOut filter", &m_pOnlineClassOutFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pOnlineClassOutFilter->QueryInterface(IID_IFileSinkFilter, reinterpret_cast<void**>(&m_iOnlineClassOutFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sOnlineClassOutFileName.GetBuffer();
				if (FAILED(hr = m_iOnlineClassOutFileSource->SetFileName(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}
			}
		}

		// add Recorder out filter
		if (0 == m_bRecorderOutRadio)
		{
			nAudioIndex = m_AudioRecordererOutComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioRenderer.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioRenderer.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pRecorderOutFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pRecorderOutFilter, L"RecorderOut filter")))
					{
						return hr;
					}
				}
			}
		}
		else
		{
			if (m_bSelectRecorderOutFile)
			{
				lpwszFileName = NULL;

				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_Dump, L"RecorderOut filter", &m_pRecorderOutFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pRecorderOutFilter->QueryInterface(IID_IFileSinkFilter, reinterpret_cast<void**>(&m_iRecorderOutFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sRecorderOutFileName.GetBuffer();
				if (FAILED(hr = m_iRecorderOutFileSource->SetFileName(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}
			}

		}
		// add OnlineClass in filter
		if (0 == m_bOnlineClassInRadio)
		{
			nAudioIndex = m_AudioOnlineClassInComBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioInputDevice.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioInputDevice.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pOnlineClassInFliter);
					if (FAILED(hr = m_graph->AddFilter(m_pOnlineClassInFliter, L"OnlineClassIn filter")))
					{
						return hr;
					}
					if (FAILED(CUtility::_set_audio_buffer(m_pOnlineClassInFliter, nAudioValue, m_sSampleRate.nSampleRate_ElectronicPianoIn)))
					{
						char errorlog[64];
						sprintf_s(errorlog, "set audio buffer size error : code = 0X%x", hr);
						MessageBoxA(NULL, errorlog, "Error", MB_OK);
					}
				}
			}
		}
		else
		{
			if (m_bSelectOnlineClassInFile)
			{
				lpwszFileName = NULL;


				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_SynthFilter, L"OnlineClassIn filter", &m_pOnlineClassInFliter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pOnlineClassInFliter->QueryInterface(IID_IFileSourceFilter, reinterpret_cast<void**>(&m_iOnlineClassInFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sOnlineClassInFileName.GetBuffer();
				if (FAILED(hr = m_iOnlineClassInFileSource->Load(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}

				if (FAILED(hr = m_pOnlineClassInFliter->QueryInterface(IID_ISynth2, reinterpret_cast<void**>(&m_iOnlineClassSynth2))))
				{
					return hr;
				}
				if (FAILED(hr = m_iOnlineClassSynth2->Set_TimeInterval(nAudioValue)))
				{
					return hr;
				}
			}
		}

		//add ElectronicPanio in filter
		if (0 == m_bElectronicPanioInRadio)
		{
			nAudioIndex = m_AudioElectronicPanioInConBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioInputDevice.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioInputDevice.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pElectronicPanioInFliter);
					if (FAILED(hr = m_graph->AddFilter(m_pElectronicPanioInFliter, L"ElectronicPanioIn filter")))
					{
						return hr;
					}
					if (FAILED(CUtility::_set_audio_buffer(m_pElectronicPanioInFliter, nAudioValue, m_sSampleRate.nSampleRate_ElectronicPianoIn)))
					{
						char errorlog[64];
						sprintf_s(errorlog, "set audio buffer size error : code = 0X%x", hr);
						MessageBoxA(NULL, errorlog, "Error", MB_OK);
					}
				}
			}
		}
		else
		{
			if (m_bSelectElectronicPanioInFile)
			{
				lpwszFileName = NULL;


				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_SynthFilter, L"ElectronicPanioIn filter", &m_pElectronicPanioInFliter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pElectronicPanioInFliter->QueryInterface(IID_IFileSourceFilter, reinterpret_cast<void**>(&m_iElectronicPanioInFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sElectronicPanioInFileName.GetBuffer();
				if (FAILED(hr = m_iElectronicPanioInFileSource->Load(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}

				if (FAILED(hr = m_pElectronicPanioInFliter->QueryInterface(IID_ISynth2, reinterpret_cast<void**>(&m_iElectronicPanioSynth2))))
				{
					return hr;
				}
				if (FAILED(hr = m_iElectronicPanioSynth2->Set_TimeInterval(nAudioValue)))
				{
					return hr;
				}
			}
		}

		//add SpeakOut in filter
		if (0 == m_bSpeakOutRadio)
		{
			nAudioIndex = m_AudioSpeakOutConBox.GetCurSel() - 1;
			if (nAudioIndex >= 0)
			{
				_mapIDDevice::iterator itv_mic1 = m_mapIdAudioRenderer.find(nAudioIndex);
				if (itv_mic1 != m_mapIdAudioRenderer.end())
				{
					itv_mic1->second->pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pSpeakOutFilter);
					if (FAILED(hr = m_graph->AddFilter(m_pSpeakOutFilter, L"SpeakOut filter")))
					{
						return hr;
					}
				}
			}
		}
		else
		{
			if (m_bSelectSpeakOutFile)
			{
				lpwszFileName = NULL;

				if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_Dump, L"SpeakOut filter", &m_pSpeakOutFilter)))
				{
					return hr;
				}

				if (FAILED(hr = m_pSpeakOutFilter->QueryInterface(IID_IFileSinkFilter, reinterpret_cast<void**>(&m_iSpeakOutFileSource))))
				{
					return hr;
				}

				lpwszFileName = m_sSpeakOutFileName.GetBuffer();
				if (FAILED(hr = m_iSpeakOutFileSource->SetFileName(lpwszFileName, NULL)))
				{
					char errorlog[64];
					sprintf_s(errorlog, "can not find wave file error : code = 0X%x", hr);
					MessageBoxA(NULL, errorlog, "Error", MB_OK);
					return hr;
				}
			}
		}
#endif //DEMO_WINSYNTHFILTER_TEST



#if DEMO_CAPTURE_RENDERER_TEST

	
		// connect MicIn to OnlineClassOut
		if (m_pMicInFilter)
		{
			hr = CUtility::ConnectFilters(m_graph, m_pMicInFilter, m_pOnlineClassOutFilter);

			if (FAILED(hr))
			{
				return hr;
			}
		}

		// connect ClassToClassIn to RecordererOut
		if (m_pClassToClassInFilter)
		{
			hr = CUtility::ConnectFilters(m_graph, m_pClassToClassInFilter,m_pRecorderOutFilter );

			if (FAILED(hr))
			{
				return hr;
			}
		}

#else

		// add audio process filter
		if (m_pAudioProcessFilter == NULL)
		{
			if (FAILED(hr = CUtility::AddFilterByCLSID(m_graph, CLSID_AudioProcessFilter, L"audio process filter", &m_pAudioProcessFilter)))
			{
				return hr;
			}
			if (FAILED(hr = m_pAudioProcessFilter->QueryInterface(IID_IAUDIOPROCESS, reinterpret_cast<void**>(&m_iAudioProcess))))
			{
				return hr;
			}
			//// make AEC disable
			//if (m_AECCheckButton.GetCheck() == BST_UNCHECKED)
			//{
			//	if (FAILED(hr = m_iAudioProcess->SetAecState(false)))
			//	{
			//		return hr;
			//	}
			//}
			//else
			//{
			//	if (FAILED(hr = m_iAudioProcess->SetAecState(true)))
			//	{
			//		return hr;
			//	}
			//}

			//// TODO : set sample rate       
			//if (FAILED(hr = m_iAudioProcess->SetSampleRate(m_sSampleRate)))
			//{
			//	return hr;
			//}

			// connect MicIn to process filter
			if (m_pMicInFilter)
			{
				hr = CUtility::ConnectFilters(m_graph, m_pMicInFilter, m_pAudioProcessFilter, CON_MIC_IN);

				if (FAILED(hr))
				{
					return hr;
				}
			}

			// connect ClassToClassIn to process filter
			if (m_pClassToClassInFilter)
			{
				hr = CUtility::ConnectFilters(m_graph, m_pClassToClassInFilter, m_pAudioProcessFilter, CON_CLASSTOCLASS_IN);

				if (FAILED(hr))
				{
					return hr;
				}
			}


			// connect OnlineClass in to process filter
			if (m_pOnlineClassInFliter)
			{
				hr = CUtility::ConnectFilters(m_graph, m_pOnlineClassInFliter, m_pAudioProcessFilter, CON_ONLINECLASS_IN);

				if (FAILED(hr))
				{
					return hr;
				}
			}

			// connect ElectronicPanio in to process filter
			if (m_pElectronicPanioInFliter)
			{
				hr = CUtility::ConnectFilters(m_graph, m_pElectronicPanioInFliter, m_pAudioProcessFilter, CON_ELECTRONICPIANO_IN);

				if (FAILED(hr))
				{
					return hr;
				}
			}

			// connect process filter to OnlineClassOut
			if (m_pOnlineClassOutFilter)
			{
				hr = CUtility::ConnectFilters(m_graph, m_pAudioProcessFilter, m_pOnlineClassOutFilter, CON_ONLINECLASS_OUT);

				if (FAILED(hr))
				{
					return hr;
				}
			}

			// connect process filter to RecordererOut
			if (m_pRecorderOutFilter)
			{
				hr = CUtility::ConnectFilters(m_graph, m_pAudioProcessFilter, m_pRecorderOutFilter, CON_RECORDERER_OUT);

				if (FAILED(hr))
				{
					return hr;
				}
			}
			// connect process filter to speakout
			if (m_pSpeakOutFilter)
			{
				hr = CUtility::ConnectFilters(m_graph, m_pAudioProcessFilter, m_pSpeakOutFilter, CON_SPEAKER_OUT);

				if (FAILED(hr))
				{
					return hr;
				}
			}



		}

#endif //DEMO_CAPTURE_RENDERER_TEST




		if (SUCCEEDED(hr))
		{
			hr = m_graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&m_mediaControl));
		}

		if (SUCCEEDED(hr))
		{
			hr = m_mediaControl->Run();
			if(FAILED(hr))
			{
				char errorlog[64];
				sprintf_s(errorlog,"run error : code = 0X%x",hr);
				MessageBoxA(NULL,errorlog,"Error", MB_OK);
			}
		}

		
	}

	return hr;
}

// 停止渲染
HRESULT CAudioProcessFilterDemoDlg::_StopGraph()
{
	if( m_mediaControl != NULL ) 
	{
		OAFilterState state;
		do {
			m_mediaControl->Stop();
			m_mediaControl->GetState(0, & state );
		} while( state != State_Stopped ) ;
	}

	//_ReSetData();
	KillTimer(2);

	return S_OK;
}



void CAudioProcessFilterDemoDlg::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here

	if ((0 == m_AudioMicInComBox.GetCurSel() && false == m_bSelectMicInFile) 
		&& (0 == m_AudioClassToClassInComBox.GetCurSel() && false == m_bSelectClassToClassInFile)
		&& (0 == m_AudioOnlineClassInComBox.GetCurSel() && false == m_bSelectOnlineClassInFile)
		&& (0 == m_AudioElectronicPanioInConBox.GetCurSel() && false == m_bSelectElectronicPanioInFile)
		)
	{
		MessageBoxA(NULL,"we must select input device !!","error select", MB_OK);
		return;
	}

	if ((0 == m_AudioOnlineClassOutComBox.GetCurSel() && false == m_bSelectOnlineClassOutFile) 
		&& (0 == m_AudioRecordererOutComBox.GetCurSel() && false == m_bSelectRecorderOutFile)
		&& (0 == m_AudioSpeakOutConBox.GetCurSel() && false == m_bSelectSpeakOutFile)
		)
	{
		MessageBoxA(NULL,"we must select output device !!","error select", MB_OK);
		return;
	}

	//NOTE: there is only OnlineClassIn input pin connected.
	//if (0 == m_AudioMicInComBox.GetCurSel() && false == m_bSelectMicInFile)
	//{
	//	MessageBoxA(NULL,"we must select Mic_in !!","error select", MB_OK);
	//	return;
	//}

	if (0 == m_AudioOnlineClassOutComBox.GetCurSel() && false == m_bSelectOnlineClassOutFile)
	{
		MessageBoxA(NULL,"we must select OnlineClass_out !!","error select", MB_OK);
		return;
	}

	_EnbaleSettingPanel(FALSE);
	HRESULT hr = _StartGraph();
	if(FAILED(hr))
	{
		MessageBoxA(NULL,"wrong start graph","info",MB_OK);
		return;
	}
}


void CAudioProcessFilterDemoDlg::OnBnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	_EnbaleSettingPanel(TRUE);
	_StopGraph();
}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonClear()
{
	// TODO: Add your control notification handler code here
	m_sClassToClassInFileName.Empty();
	m_sMicInFileName.Empty();
	m_sOnlineClassOutFileName.Empty();
	m_sRecorderOutFileName.Empty();
	m_sElectronicPanioInFileName.Empty();
	m_sOnlineClassInFileName.Empty();
	m_sSpeakOutFileName.Empty();

	m_bSelectClassToClassInFile = false;
	m_bSelectMicInFile = false;
	m_bSelectOnlineClassOutFile = false;
	m_bSelectRecorderOutFile = false;
	m_bSelectOnlineClassInFile = false;
	m_bSelectElectronicPanioInFile = false;
	m_bSelectSpeakOutFile = false;

	GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_MIC_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->SetWindowText(_T("请选择wave文件..."));
	GetDlgItem(IDC_EDIT_Speak_Out_FILE)->SetWindowText(_T("请选择wave文件..."));

	m_AudioMicInComBox.SetCurSel(0);
	m_AudioClassToClassInComBox.SetCurSel(0);
	m_AudioOnlineClassOutComBox.SetCurSel(0);
	m_AudioRecordererOutComBox.SetCurSel(0);
	m_AudioOnlineClassInComBox.SetCurSel(0);
	m_AudioElectronicPanioInConBox.SetCurSel(0);
	m_AudioSpeakOutConBox.SetCurSel(0);

	m_bMicInRadio = 0;
	m_bClassToClassInRadio = 0;
	m_bOnlineClassOutRadio = 0;
	m_bRecorderOutRadio = 0;
	m_bOnlineClassInRadio = 0;
	m_bElectronicPanioInRadio = 0;
	m_bSpeakOutRadio = 0;

	m_AudioMicInComBox.EnableWindow(TRUE);
	m_AudioClassToClassInComBox.EnableWindow(TRUE);
	m_AudioOnlineClassOutComBox.EnableWindow(TRUE);
	m_AudioRecordererOutComBox.EnableWindow(TRUE);
	m_AudioOnlineClassInComBox.EnableWindow(TRUE);
	m_AudioElectronicPanioInConBox.EnableWindow(TRUE);
	m_AudioSpeakOutConBox.EnableWindow(TRUE);

	GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_MIC_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_Speak_Out_FILE)->EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_CLASSTOCLASS_IN_OPENFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_MIC_IN_OPENFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ONLINECLASS_OUT_OPENFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_RECORDER_OUT_OPENFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ONLINECLASS_IN_OPENFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ELECTRONIC_IN_OPENFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SPEACK_OUT_OPENFILE)->EnableWindow(FALSE);

	UpdateData(FALSE);
}


void CAudioProcessFilterDemoDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	_mapIDDevice::iterator iteaV = m_mapIdAudioInputDevice.begin();
	while (iteaV != m_mapIdAudioInputDevice.end())
	{
		iteaV->second->pMoniker->Release();
		delete iteaV->second;

		iteaV++;
	}
	m_mapIdAudioInputDevice.clear();

	_mapIDDevice::iterator iteaA = m_mapIdAudioRenderer.begin();
	while (iteaA != m_mapIdAudioRenderer.end())
	{
		iteaA->second->pMoniker->Release();
		delete iteaA->second;

		iteaA++;
	}
	m_mapIdAudioRenderer.clear();
	_ReSetData();

	KillTimer(1);
	// 停止使用com
	 CoUninitialize();
	CDialogEx::OnClose();
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioClasstoclassInDevice()
{
	// TODO: Add your control notification handler code here
	m_bClassToClassInRadio = 0;
	GetDlgItem(IDC_COMBO_ClassToClass_IN_DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_CLASSTOCLASS_IN_OPENFILE)->EnableWindow(FALSE);
	UpdateData(FALSE);
}


void CAudioProcessFilterDemoDlg::OnBnClickedRadioClasstoclassInFile()
{
	// TODO: Add your control notification handler code here
	m_bClassToClassInRadio = 1;
	GetDlgItem(IDC_COMBO_ClassToClass_IN_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_CLASSTOCLASS_IN_OPENFILE)->EnableWindow(TRUE);
	UpdateData(FALSE);
}


void CAudioProcessFilterDemoDlg::OnBnClickedRadioMicInDevice()
{
	// TODO: Add your control notification handler code here
	m_bMicInRadio = 0;
	GetDlgItem(IDC_COMBO_MIC_IN_DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_MIC_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_MIC_IN_OPENFILE)->EnableWindow(FALSE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioMicInFile()
{
	// TODO: Add your control notification handler code here
	m_bMicInRadio = 1;
	GetDlgItem(IDC_COMBO_MIC_IN_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_MIC_IN_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_MIC_IN_OPENFILE)->EnableWindow(TRUE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineclassOutDevice()
{
	// TODO: Add your control notification handler code here
	m_bOnlineClassOutRadio = 0;
	GetDlgItem(IDC_COMBO_ONLINECLASS_OUT_DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ONLINECLASS_OUT_OPENFILE)->EnableWindow(FALSE);
	UpdateData(FALSE);

}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineclassOutFile()
{
	// TODO: Add your control notification handler code here
	m_bOnlineClassOutRadio = 1;
	GetDlgItem(IDC_COMBO_ONLINECLASS_OUT_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ONLINECLASS_OUT_OPENFILE)->EnableWindow(TRUE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioRecorderOutDevice()
{
	// TODO: Add your control notification handler code here
	m_bRecorderOutRadio = 0;
	GetDlgItem(IDC_COMBO_RECORDER_OUT_DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_RECORDER_OUT_OPENFILE)->EnableWindow(FALSE);
	UpdateData(FALSE);

}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioRecorderOutFile()
{
	// TODO: Add your control notification handler code here
	m_bRecorderOutRadio = 1;
	GetDlgItem(IDC_COMBO_RECORDER_OUT_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_RECORDER_OUT_OPENFILE)->EnableWindow(TRUE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineClassInDevice()
{
	// TODO: Add your control notification handler code here
	m_bOnlineClassInRadio = 0;
	GetDlgItem(IDC_COMBO_OnlineClass_IN_DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ONLINECLASS_IN_OPENFILE)->EnableWindow(FALSE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioOnlineClassInFile()
{
	// TODO: Add your control notification handler code here
	m_bOnlineClassInRadio = 1;
	GetDlgItem(IDC_COMBO_OnlineClass_IN_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ONLINECLASS_IN_OPENFILE)->EnableWindow(TRUE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioElectronicpianoInDevice()
{
	// TODO: Add your control notification handler code here
	m_bElectronicPanioInRadio = 0;
	GetDlgItem(IDC_COMBO_ElectronicPanio_IN_DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ELECTRONIC_IN_OPENFILE)->EnableWindow(FALSE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioElectronicpianoInFile()
{
	// TODO: Add your control notification handler code here
	m_bElectronicPanioInRadio = 1;
	GetDlgItem(IDC_COMBO_ElectronicPanio_IN_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ELECTRONIC_IN_OPENFILE)->EnableWindow(TRUE);
	UpdateData(FALSE);
}

void CAudioProcessFilterDemoDlg::OnBnClickedRadioSpeackOutDevice()
{
	// TODO: Add your control notification handler code here
	m_bSpeakOutRadio= 0;
	GetDlgItem(IDC_COMBO_Speak_OUT_DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_Speak_Out_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SPEACK_OUT_OPENFILE)->EnableWindow(FALSE);
	UpdateData(FALSE);
}


void CAudioProcessFilterDemoDlg::OnBnClickedRadioSpeackOutFile()
{
	// TODO: Add your control notification handler code here
	m_bSpeakOutRadio = 1;
	GetDlgItem(IDC_COMBO_Speak_OUT_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_Speak_Out_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_SPEACK_OUT_OPENFILE)->EnableWindow(TRUE);
	UpdateData(FALSE);
}


void CAudioProcessFilterDemoDlg::OnCbnSelchangeComboClasstoclassInDevice()
{
	// TODO: Add your control notification handler code here
	if (0 != m_AudioClassToClassInComBox.GetCurSel())
	{
		if ((!m_bSelectMicInFile && m_AudioMicInComBox.GetCurSel() == m_AudioClassToClassInComBox.GetCurSel())
			|| (!m_bSelectElectronicPanioInFile && m_AudioElectronicPanioInConBox.GetCurSel() == m_AudioClassToClassInComBox.GetCurSel())
			|| (!m_bSelectOnlineClassInFile && m_AudioOnlineClassInComBox.GetCurSel() == m_AudioClassToClassInComBox.GetCurSel())
			)
		{
			MessageBoxA(NULL, "could not select the same audio device which others selected!", "Error info", MB_OK);
			m_AudioClassToClassInComBox.SetCurSel(0);
		}
	}
}

void CAudioProcessFilterDemoDlg::OnCbnSelchangeComboMicInDevice()
{
	// TODO: Add your control notification handler code here
	if (0 != m_AudioMicInComBox.GetCurSel())
	{
		if ((!m_bSelectClassToClassInFile && m_AudioClassToClassInComBox.GetCurSel() == m_AudioMicInComBox.GetCurSel())
			|| (!m_bSelectOnlineClassInFile && m_AudioOnlineClassInComBox.GetCurSel() == m_AudioMicInComBox.GetCurSel())
			|| (!m_bSelectElectronicPanioInFile && m_AudioElectronicPanioInConBox.GetCurSel() == m_AudioMicInComBox.GetCurSel())

			)
		{
			MessageBoxA(NULL, "could not select the same audio device which others selected!", "Error info", MB_OK);
			m_AudioMicInComBox.SetCurSel(0);
		}
	}
}


void CAudioProcessFilterDemoDlg::OnCbnSelchangeComboElectronicpanioInDevice()
{
	// TODO: Add your control notification handler code here
	if (0 != m_AudioElectronicPanioInConBox.GetCurSel())
	{
		if ((!m_bSelectMicInFile && m_AudioMicInComBox.GetCurSel() == m_AudioElectronicPanioInConBox.GetCurSel())
			|| (!m_bSelectOnlineClassInFile && m_AudioOnlineClassInComBox.GetCurSel() == m_AudioElectronicPanioInConBox.GetCurSel())
			|| (!m_bSelectClassToClassInFile && m_AudioClassToClassInComBox.GetCurSel() == m_AudioElectronicPanioInConBox.GetCurSel())

			)
		{
			MessageBoxA(NULL, "could not select the same audio device which others selected!", "Error info", MB_OK);
			m_AudioElectronicPanioInConBox.SetCurSel(0);
		}
	}
}


void CAudioProcessFilterDemoDlg::OnCbnSelchangeComboOnlineclassInDevice()
{
	// TODO: Add your control notification handler code here
	if (0 != m_AudioOnlineClassInComBox.GetCurSel())
	{
		if ((!m_bSelectMicInFile && m_AudioMicInComBox.GetCurSel() == m_AudioOnlineClassInComBox.GetCurSel())
			|| (!m_bSelectClassToClassInFile && m_AudioClassToClassInComBox.GetCurSel() == m_AudioOnlineClassInComBox.GetCurSel())
			|| (!m_bSelectElectronicPanioInFile && m_AudioElectronicPanioInConBox.GetCurSel() == m_AudioOnlineClassInComBox.GetCurSel())

			)
		{
			MessageBoxA(NULL, "could not select the same audio device which others selected!", "Error info", MB_OK);
			m_AudioOnlineClassInComBox.SetCurSel(0);
		}
	}
}


void CAudioProcessFilterDemoDlg::OnCbnSelchangeComboSpeakOutDevice()
{
	// TODO: Add your control notification handler code here
	if (0 != m_AudioSpeakOutConBox.GetCurSel())
	{
		if ((!m_bSelectOnlineClassOutFile && m_AudioOnlineClassOutComBox.GetCurSel() == m_AudioSpeakOutConBox.GetCurSel())
			|| (!m_bSelectRecorderOutFile && m_AudioRecordererOutComBox.GetCurSel() == m_AudioSpeakOutConBox.GetCurSel())
			)
		{
			MessageBoxA(NULL, "could not select the same audio device which others selected!", "Error info", MB_OK);
			m_AudioSpeakOutConBox.SetCurSel(0);
		}
	}
}



void CAudioProcessFilterDemoDlg::OnCbnSelchangeComboOnlineclassOutDevice()
{
	// TODO: Add your control notification handler code here
	if (0 != m_AudioOnlineClassOutComBox.GetCurSel())
	{
		if ((!m_bSelectRecorderOutFile && m_AudioRecordererOutComBox.GetCurSel() == m_AudioOnlineClassOutComBox.GetCurSel())
			|| (!m_bSelectSpeakOutFile && m_AudioSpeakOutConBox.GetCurSel() == m_AudioOnlineClassOutComBox.GetCurSel())
			)
		{
			MessageBoxA(NULL, "could not select the same audio device which others selected!", "Error info", MB_OK);
			m_AudioOnlineClassOutComBox.SetCurSel(0);
		}
	}
}

void CAudioProcessFilterDemoDlg::OnCbnSelchangeComboRecorderOutDevice()
{
	// TODO: Add your control notification handler code here
	if (0 != m_AudioRecordererOutComBox.GetCurSel())
	{
		if ((!m_bSelectOnlineClassOutFile && m_AudioOnlineClassOutComBox.GetCurSel() == m_AudioRecordererOutComBox.GetCurSel())
			|| (!m_bSelectSpeakOutFile && m_AudioSpeakOutConBox.GetCurSel() == m_AudioRecordererOutComBox.GetCurSel())
			)
		{
			MessageBoxA(NULL, "could not select the same audio device which others selected!", "Error info", MB_OK);
			m_AudioRecordererOutComBox.SetCurSel(0);
		}
	}
}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonClasstoclassInOpenfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		(LPCTSTR)_TEXT("*.wav"),
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_sClassToClassInFileName = dlg.GetPathName(); //文件名保存在了FilePathName里
		GetDlgItem(IDC_EDIT_CLASSTOCLASS_IN_FILE)->SetWindowText(m_sClassToClassInFileName.GetBuffer());
		m_bSelectClassToClassInFile = true;
		UpdateData(FALSE);
	}

}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonMicInOpenfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		(LPCTSTR)_TEXT("*.wav"),
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_sMicInFileName = dlg.GetPathName(); //文件名保存在了FilePathName里
		GetDlgItem(IDC_EDIT_MIC_IN_FILE)->SetWindowText(m_sMicInFileName.GetBuffer());
		m_bSelectMicInFile = true;
		UpdateData(FALSE);
	}
}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonOnlineclassInOpenfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		(LPCTSTR)_TEXT("*.wav"),
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_sOnlineClassInFileName = dlg.GetPathName(); //文件名保存在了FilePathName里
		GetDlgItem(IDC_EDIT_OnlineClass_IN_FILE)->SetWindowText(m_sOnlineClassInFileName.GetBuffer());
		m_bSelectOnlineClassInFile = true;
		UpdateData(FALSE);
	}
}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonElectronicInOpenfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		(LPCTSTR)_TEXT("*.wav"),
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_sElectronicPanioInFileName = dlg.GetPathName(); //文件名保存在了FilePathName里
		GetDlgItem(IDC_EDIT_ElectronicPanio_IN_FILE)->SetWindowText(m_sElectronicPanioInFileName.GetBuffer());
		m_bSelectElectronicPanioInFile= true;
		UpdateData(FALSE);
	}
}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonSpeackOutOpenfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		(LPCTSTR)_TEXT("*.wav"),
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_sSpeakOutFileName= dlg.GetPathName(); //文件名保存在了FilePathName里
		GetDlgItem(IDC_EDIT_Speak_Out_FILE)->SetWindowText(m_sSpeakOutFileName.GetBuffer());
		m_bSelectSpeakOutFile = true;
		UpdateData(FALSE);
	}
}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonOnlineclassOutOpenfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		(LPCTSTR)_TEXT("*.wav"),
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_sOnlineClassOutFileName = dlg.GetPathName(); //文件名保存在了FilePathName里
		GetDlgItem(IDC_EDIT_ONLINECLASS_OUT_FILE)->SetWindowText(m_sOnlineClassOutFileName.GetBuffer());
		m_bSelectOnlineClassOutFile = true;
		UpdateData(FALSE);
	}
}

void CAudioProcessFilterDemoDlg::OnBnClickedButtonRecorderOutOpenfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		(LPCTSTR)_TEXT("*.wav"),
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_sRecorderOutFileName = dlg.GetPathName(); //文件名保存在了FilePathName里
		GetDlgItem(IDC_EDIT_RECORDER_OUT_FILE)->SetWindowText(m_sRecorderOutFileName.GetBuffer());
		m_bSelectRecorderOutFile = true;
		UpdateData(FALSE);
	}
}

void CAudioProcessFilterDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnTimer(nIDEvent);
	switch(nIDEvent)
	{
	case 1:
		{
			CTime time = CTime::GetCurrentTime();
			CString sTime = time.Format(_T("        %Y-%m-%d - %H:%M:%S"));
			SetDlgItemText(IDC_EDIT_SYSTEM_TIME, sTime);
			break;
		}
	case 2:
		{
			CTime time = CTime::GetCurrentTime();
			CTimeSpan timespan = time-m_ctime;
			CString sTime = timespan.Format(_T("                 %D - %H:%M:%S"));
			SetDlgItemText(IDC_EDIT_RUN_TIME, sTime);
			break;
		}
	default:
		break;
	}

}





void CAudioProcessFilterDemoDlg::OnEnChangeEditClasstoclassInFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAudioProcessFilterDemoDlg::OnEnChangeEditOnlineclassInFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAudioProcessFilterDemoDlg::OnEnChangeEditElectronicpanioInFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAudioProcessFilterDemoDlg::OnEnChangeEditSpeakOutFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAudioProcessFilterDemoDlg::OnEnChangeEditMicInFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAudioProcessFilterDemoDlg::OnEnChangeEditOnlineclassOutFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAudioProcessFilterDemoDlg::OnEnChangeEditRecorderOutFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
