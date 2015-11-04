//------------------------------------------------------------------------------
// File: Synth.cpp
//
// Desc: DirectShow sample code - implements an audio signal generator
//       source filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <streams.h>

#include <math.h>
#include <mmreg.h>
#include <msacm.h>

#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

#include "DynSrc.h"
#include "isynth.h"
#include "synth.h"

// setup data

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{ &MEDIATYPE_Audio      // clsMajorType
, &MEDIASUBTYPE_NULL }; // clsMinorType

const AMOVIESETUP_PIN sudOpPin =
{ L"Output"          // strName
, FALSE              // bRendered
, TRUE               // bOutput
, FALSE              // bZero
, FALSE              // bMany
, &CLSID_NULL        // clsConnectsToFilter
, L"Input"           // strConnectsToPin
, 1                  // nTypes
, &sudOpPinTypes };  // lpTypes

const AMOVIESETUP_FILTER sudSynth =
{ &CLSID_SynthFilter     // clsID
, L"Codyy Audio File Source" // strName
, MERIT_UNLIKELY       // dwMerit
, 1                    // nPins
, &sudOpPin };         // lpPin

// -------------------------------------------------------------------------
// g_Templates
// -------------------------------------------------------------------------
// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {

    { L"Codyy Audio File Source"
    , &CLSID_SynthFilter
    , CSynthFilter::CreateInstance
    , NULL
    , &sudSynth }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// There are 8 bits in a byte.
const DWORD BITS_PER_BYTE = 8;

// -------------------------------------------------------------------------
// CSynthFilter, the main filter object
// -------------------------------------------------------------------------
//
// CreateInstance
//
// The only allowed way to create Synthesizers

CUnknown * WINAPI CSynthFilter::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    ASSERT(phr);
    
    CUnknown *punk = new CSynthFilter(lpunk, phr);
    if (punk == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return punk;
}


//
// CSynthFilter::Constructor
//
// initialise a CSynthStream object so that we have a pin.

CSynthFilter::CSynthFilter(LPUNKNOWN lpunk, HRESULT *phr)
    : CDynamicSource(NAME("Codyy Audio File Source Filter"),lpunk, CLSID_SynthFilter, phr)
	, m_pFileName(NULL)
	, m_llSize(0)
	, m_pbData(NULL)
	, m_cWavFileOp(NULL)
	, m_llPosition(POS_START)
	, m_dwTimeStart(0)
	, m_wfexPCM(NULL)
	, m_nTimeInterval(TIME_INTERVAL)
{
    m_paStreams = (CDynamicSourceStream **) new CSynthStream*[1];
    if (m_paStreams == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }

    m_paStreams[0] = new CSynthStream(phr, this, L"output");
    if (m_paStreams[0] == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }

	m_wfexPCM = new WAVEFORMATEX;
}

//
// CSynthFilter::Destructor
//
CSynthFilter::~CSynthFilter(void) 
{
    //
    //  Base class will free our pins
    //
	if (m_wfexPCM)
	{
		delete m_wfexPCM;
		m_wfexPCM = NULL;
	}
}

//
// NonDelegatingQueryInterface
//
// Reveal our property page, persistance, and control interfaces

STDMETHODIMP CSynthFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IFileSourceFilter) {
		return GetInterface((IFileSourceFilter *)this, ppv);
	}
	if (riid == IID_ISynth2) {
		return GetInterface((ISynth2 *)this, ppv);
	}
    else {
        return CDynamicSource::NonDelegatingQueryInterface(riid, ppv);
    }
}


HRESULT CSynthFilter::EndOfStream()
{
	HRESULT hr = NOERROR;
	if (NULL != *m_paStreams)
	{
		hr = (*m_paStreams)->DeliverEndOfStream();
	}

	return hr;
}

/*  IFileSourceFilter methods */

//  Load a (new) file
STDMETHODIMP CSynthFilter::Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
{
	CheckPointer(lpwszFileName, E_POINTER);

	// lstrlenW is one of the few Unicode functions that works on win95
	int cch = lstrlenW(lpwszFileName) + 1;

#ifndef UNICODE
	TCHAR *lpszFileName = 0;
	lpszFileName = new char[cch * 2];
	if (!lpszFileName) {
		return E_OUTOFMEMORY;
	}
	WideCharToMultiByte(GetACP(), 0, lpwszFileName, -1,
		lpszFileName, cch, NULL, NULL);
#else
	TCHAR lpszFileName[MAX_PATH] = { 0 };
	(void)StringCchCopy(lpszFileName, NUMELMS(lpszFileName), lpwszFileName);
#endif
	CAutoLock lck(&m_csFilter);

	/*  Check the file type */
	CMediaType cmt;
	if (NULL == pmt) {
		//cmt.SetType(&MEDIATYPE_Stream);
		//cmt.SetSubtype(&MEDIASUBTYPE_NULL);
		cmt.SetType(&MEDIATYPE_Stream);
		cmt.SetSubtype(&MEDIASUBTYPE_WAVE);
	}
	else {
		cmt = *pmt;
	}

	if (!ReadTheFile(lpszFileName)) {
#ifndef UNICODE
		delete[] lpszFileName;
#endif
		return E_FAIL;
	}

	m_pFileName = new WCHAR[cch];

	if (m_pFileName != NULL)
		CopyMemory(m_pFileName, lpwszFileName, cch*sizeof(WCHAR));

	// this is not a simple assignment... pointers and format
	// block (if any) are intelligently copied
	m_mt = cmt;

	/*  Work out file type */
	cmt.bTemporalCompression = TRUE;	       //???
	cmt.lSampleSize = 1;

	return S_OK;
}

// Modeled on IPersistFile::Load
// Caller needs to CoTaskMemFree or equivalent.

STDMETHODIMP CSynthFilter::GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt)
{
	CheckPointer(ppszFileName, E_POINTER);
	*ppszFileName = NULL;

	if (m_pFileName != NULL) {
		DWORD n = sizeof(WCHAR)*(1 + lstrlenW(m_pFileName));

		*ppszFileName = (LPOLESTR)CoTaskMemAlloc(n);
		if (*ppszFileName != NULL) {
			CopyMemory(*ppszFileName, m_pFileName, n);
		}
	}

	//if (pmt != NULL) {
	//	CopyMediaType(pmt, &m_mt);
	//}

	return NOERROR;
}

STDMETHODIMP CSynthFilter::Set_TimeInterval(unsigned int time_interval)
{
	if (time_interval > 0)
	{
		m_nTimeInterval = time_interval;
		return NOERROR;
	}
	else
	{
		return E_FAIL;
	}
}

BOOL CSynthFilter::ReadTheFile(LPCTSTR lpszFileName)
{
	DWORD dwBytesRead;

	// Open the requested file
	HANDLE hFile = CreateFile(lpszFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DbgLog((LOG_TRACE, 2, TEXT("Could not open %s\n"), lpszFileName));
		return FALSE;
	}

	// Determine the file size
	ULARGE_INTEGER uliSize;
	uliSize.LowPart = GetFileSize(hFile, &uliSize.HighPart);

	PBYTE pbMem = new BYTE[uliSize.LowPart];
	if (pbMem == NULL)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	// Read the data from the file
	if (!ReadFile(hFile,
		(LPVOID)pbMem,
		uliSize.LowPart,
		&dwBytesRead,
		NULL) ||
		(dwBytesRead != uliSize.LowPart))
	{
		DbgLog((LOG_TRACE, 1, TEXT("Could not read file\n")));

		delete[] pbMem;
		CloseHandle(hFile);
		return FALSE;
	}

	// Save a pointer to the data that was read from the file
	m_pbData = pbMem;
	m_llSize = (LONGLONG)uliSize.QuadPart;

	// Close the file
	CloseHandle(hFile);

	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpszFileName, -1, NULL, 0, NULL, FALSE);
	char *filename = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpszFileName, -1, filename, num, NULL, FALSE);
	m_cWavFileOp = new CWavFileOp(filename, "rb");
	if (m_cWavFileOp->m_FileStatus == -2)
	{
		delete m_cWavFileOp;
		delete[]filename;
		return FALSE;
	}
	m_cWavFileOp->ReadHeader(&m_sWavFileHead);

	m_wfexPCM->wFormatTag = WAVE_FORMAT_PCM;
	m_wfexPCM->nChannels = m_sWavFileHead.NChannels;
	m_wfexPCM->nSamplesPerSec = m_sWavFileHead.SampleRate;
	m_wfexPCM->wBitsPerSample = m_sWavFileHead.NBitsPersample;
	m_wfexPCM->nBlockAlign = m_sWavFileHead.BytesPerSample;
	m_wfexPCM->nAvgBytesPerSec = m_sWavFileHead.SampleBytes;
	m_wfexPCM->cbSize = m_sWavFileHead.RawDataFileLength;

	delete m_cWavFileOp;
	delete[]filename;

	return TRUE;
}

// -------------------------------------------------------------------------
// CSynthStream, the output pin
// -------------------------------------------------------------------------

//
// CSynthStream::Constructor
//

CSynthStream::CSynthStream(HRESULT *phr, CSynthFilter *pParent, LPCWSTR pName)
    : CDynamicSourceStream(NAME("output"),phr, pParent, pName)
    , m_hPCMToMSADPCMConversionStream(NULL)
    , m_dwTempPCMBufferSize(0)
    , m_fFirstSampleDelivered(FALSE)
    , m_llSampleMediaTimeStart(0) 
	, m_llCounter(0)
{
    ASSERT(phr);
    m_pParent = pParent;
}


//
// CSynthStream::Destructor
//
CSynthStream::~CSynthStream(void) 
{
}


//
// FillBuffer
//
// Stuffs the buffer with data
HRESULT CSynthStream::FillBuffer(IMediaSample *pms)
{
	CheckPointer(pms, E_POINTER);
	BYTE *pData = NULL;
	DWORD dwBytesToRead = 0;
	DWORD dwTime = 0;
	DWORD dwTimeToArrive = 0;

	HRESULT hr = pms->GetPointer(&pData);
	if (FAILED(hr)) {
		return hr;
	}
	// This function must hold the state lock because it calls
	// FillPCMAudioBuffer().
	CAutoLock lStateLock(m_pParent->pStateLock());

	// This lock must be held because this function uses
	// m_dwTempPCMBufferSize, m_hPCMToMSADPCMConversionStream,
	// m_rtSampleTime, m_fFirstSampleDelivered and
	// m_llSampleMediaTimeStart.
	CAutoLock lShared(&m_cSharedState);

	
	dwBytesToRead = pms->GetSize();	
	memset(pData, 0, dwBytesToRead);
	pms->SetActualDataLength(dwBytesToRead);
	dwTimeToArrive = (DWORD)(dwBytesToRead / (m_pParent->m_wfexPCM->nAvgBytesPerSec / 1000.f));
	

	/*  Wait until the bytes are here! */

	if (m_pParent->m_llPosition + dwBytesToRead > m_pParent->m_llSize) 
	{
		switch(PLAYBACK_MODE_USED)
		{
		case SINGLE_CYCLE:
			m_pParent->m_llPosition = POS_START;
			break;
		case SINGLE_PLAY:
			if (m_pParent->m_llSize - m_pParent->m_llPosition > 0)
			{
				dwBytesToRead = (m_pParent->m_llSize - m_pParent->m_llPosition);
			}
			else
			{
				return S_FALSE;
			}
			break;
		default:
			return S_FALSE;
	    }
	}
	

	dwTime = timeGetTime();
	if (dwTime - m_pParent->m_dwTimeStart < dwTimeToArrive && m_llCounter >= 10)
	{
		Sleep(dwTimeToArrive - dwTime + m_pParent->m_dwTimeStart);
	}
	m_pParent->m_dwTimeStart = timeGetTime();

	CopyMemory((PVOID)pData, (PVOID)(m_pParent->m_pbData + m_pParent->m_llPosition),
		dwBytesToRead);
	m_pParent->m_llPosition += dwBytesToRead;


	// Set the sample's time stamps.  
	CRefTime rtStart = m_rtSampleTime;

	m_rtSampleTime = rtStart + (REFERENCE_TIME)(UNITS * pms->GetActualDataLength()) /
		(REFERENCE_TIME)m_pParent->m_wfexPCM->nAvgBytesPerSec;

	hr = pms->SetTime((REFERENCE_TIME*)&rtStart, (REFERENCE_TIME*)&m_rtSampleTime);

	if (FAILED(hr)) {
		return hr;
	}

	// Set the sample's properties.
	hr = pms->SetPreroll(FALSE);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pms->SetMediaType(NULL);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pms->SetDiscontinuity(!m_fFirstSampleDelivered);
	if (FAILED(hr)) {
		return hr;
	}

	hr = pms->SetSyncPoint(!m_fFirstSampleDelivered);
	if (FAILED(hr)) {
		return hr;
	}

	LONGLONG llMediaTimeStart = m_llSampleMediaTimeStart;

	DWORD dwNumAudioSamplesInPacket = pms->GetActualDataLength() /
		m_pParent->m_wfexPCM->nBlockAlign;

	LONGLONG llMediaTimeStop = m_llSampleMediaTimeStart + dwNumAudioSamplesInPacket;

	hr = pms->SetMediaTime(&llMediaTimeStart, &llMediaTimeStop);
	if (FAILED(hr)) {
		return hr;
	}

	m_llSampleMediaTimeStart = llMediaTimeStop;
	m_fFirstSampleDelivered = TRUE;

	m_llCounter++;

	return NOERROR;
}


//
// GetMediaType
//
HRESULT CSynthStream::GetMediaType(CMediaType *pmt) 
{
    CheckPointer(pmt,E_POINTER);

    // The caller must hold the state lock because this function
    // calls get_OutputFormat() and GetPCMFormatStructure().
    // The function assumes that the state of the m_Synth
    // object does not change between the two calls.  The
    // m_Synth object's state will not change if the 
    // state lock is held.
    ASSERT(CritCheckIn(m_pParent->pStateLock()));

    WAVEFORMATEX *pwfex;

	pwfex = (WAVEFORMATEX *)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
	if (NULL == pwfex)
	{
		return E_OUTOFMEMORY;
	}
	memcpy(pwfex, m_pParent->m_wfexPCM, sizeof(WAVEFORMATEX));
    return CreateAudioMediaType(pwfex, pmt, TRUE);
}


HRESULT CSynthStream::CompleteConnect(IPin *pReceivePin)
{
    // This lock must be held because this function uses
    // m_hPCMToMSADPCMConversionStream, m_fFirstSampleDelivered 
    // and m_llSampleMediaTimeStart.
    CAutoLock lShared(&m_cSharedState);

    HRESULT hr;
 
    hr = CDynamicSourceStream::CompleteConnect(pReceivePin);
	if (FAILED(hr))
	{
		return E_FAIL;
	}
 
    m_fFirstSampleDelivered = FALSE;
    m_llSampleMediaTimeStart = 0;

    return S_OK;
}

HRESULT CSynthStream::BreakConnect(void)
{
    // This lock must be held because this function uses
    // m_hPCMToMSADPCMConversionStream and m_dwTempPCMBufferSize.
    CAutoLock lShared(&m_cSharedState);

    HRESULT hr = CDynamicSourceStream::BreakConnect();
    if(FAILED(hr))
    {
        return hr;
    }

    if(NULL != m_hPCMToMSADPCMConversionStream)
    {
        // acmStreamClose() should never fail because m_hPCMToMSADPCMConversionStream
        // holds a valid ACM stream handle and all operations using the handle are 
        // synchronous.
        EXECUTE_ASSERT(0 == acmStreamClose(m_hPCMToMSADPCMConversionStream, 0));
        m_hPCMToMSADPCMConversionStream = NULL;
        m_dwTempPCMBufferSize = 0;
    }

    return S_OK;
}

//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what format we agreed to.
// Then we can ask for buffers of the correct size to contain them.
HRESULT CSynthStream::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    // The caller should always hold the shared state lock 
    // before calling this function.  This function must hold 
    // the shared state lock because it uses m_hPCMToMSADPCMConversionStream
    // m_dwTempPCMBufferSize.
    ASSERT(CritCheckIn(&m_cSharedState));

    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    WAVEFORMATEX *pwfexCurrent = (WAVEFORMATEX*)m_mt.Format();
	int nBitsPerSample = pwfexCurrent->wBitsPerSample;
	int nSamplesPerSec = pwfexCurrent->nSamplesPerSec;
	int nChannels = pwfexCurrent->nChannels;

	pProperties->cbBuffer = pwfexCurrent->nAvgBytesPerSec * m_pParent->m_nTimeInterval / 1000;
	pProperties->cBuffers = 8;
	pProperties->cbAlign  = 1;
	pProperties->cbPrefix = 0;

    // Ask the allocator to reserve us the memory

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAlloc->SetProperties(pProperties,&Actual);	
    if(FAILED(hr))
    {
        return hr;
    }

    // Is this allocator unsuitable

	if (Actual.cbBuffer != pProperties->cbBuffer )
    {
        return E_FAIL;
    }

    return NOERROR;
}

//
// Active
//
HRESULT CSynthStream::Active(void)
{
    // This lock must be held because the function
    // uses m_rtSampleTime, m_fFirstSampleDelivered
    // and m_llSampleMediaTimeStart.
    CAutoLock lShared(&m_cSharedState);

    HRESULT hr = CDynamicSourceStream::Active();
    if(FAILED(hr))
    {
        return hr;
    }

	m_pParent->m_dwTimeStart = timeGetTime();
	m_pParent->m_llPosition = POS_START;

    m_rtSampleTime = 0;
	m_llSampleMediaTimeStart = 0;
    m_fFirstSampleDelivered = FALSE;
	m_llCounter = 0;

    return NOERROR;
}

////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


