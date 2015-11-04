//------------------------------------------------------------------------------
// File: Synth.h
//
// Desc: DirectShow sample code - header file for audio signal generator 
//       source filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef __AUDIOSYNTH__
#define __AUDIOSYNTH__

#include <strsafe.h>
#include "WaveIO.h"

//CLSID_SynthFilter
//{79A98DE0-BC00-11ce-AC2E-444553540000}
DEFINE_GUID(CLSID_SynthFilter,
0x79a98de0, 0xbc00, 0x11ce, 0xac, 0x2e, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);


enum PLAYBACK_MODE{ SINGLE_CYCLE = 0, SINGLE_PLAY = 1 };
#define POS_START      44
#define TIME_INTERVAL  10
#define PLAYBACK_MODE_USED   SINGLE_CYCLE     



// -------------------------------------------------------------------------
// CSynthFilter
// -------------------------------------------------------------------------
// CSynthFilter manages filter level stuff

class CSynthFilter 
	: public CDynamicSource
	, public IFileSourceFilter
	, public ISynth2
{
public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    ~CSynthFilter();

	HRESULT EndOfStream();

    DECLARE_IUNKNOWN;
    // override this to reveal our property interface
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	/*  IFileSourceFilter methods */
	//  Load a (new) file
	STDMETHODIMP Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt);
	// Modeled on IPersistFile::Load
	// Caller needs to CoTaskMemFree or equivalent.
	STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt);
	STDMETHODIMP Set_TimeInterval(unsigned int time_interval);

private:

    // it is only allowed to to create these objects with CreateInstance
    CSynthFilter(LPUNKNOWN lpunk, HRESULT *phr);
	BOOL ReadTheFile(LPCTSTR lpszFileName);

	LPWSTR     m_pFileName;
	SWavFileHead m_sWavFileHead;
	CWavFileOp *m_cWavFileOp;

public:
	LONGLONG   m_llSize;
	LONGLONG   m_llPosition;
	PBYTE      m_pbData;
	DWORD      m_dwTimeStart;
	WAVEFORMATEX* m_wfexPCM;
	unsigned int m_nTimeInterval;
};

// -------------------------------------------------------------------------
// CSynthStream
// -------------------------------------------------------------------------
// CSynthStream manages the data flow from the output pin.

class CSynthStream : public CDynamicSourceStream {

public:

    CSynthStream(HRESULT *phr, CSynthFilter *pParent, LPCWSTR pPinName);
    ~CSynthStream();

    BOOL ReadyToStop(void) {return FALSE;}

    // stuff an audio buffer with the current format
    HRESULT FillBuffer(IMediaSample *pms);

    // ask for buffers of the size appropriate to the agreed media type.
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    HRESULT GetMediaType(CMediaType *pmt);

    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect(void);

    // resets the stream time to zero.
    HRESULT Active(void);

private:
    // Access to this state information should be serialized with the filters
    // critical section (m_pFilter->pStateLock())

    // This lock protects: m_dwTempPCMBufferSize, m_hPCMToMSADPCMConversionStream,
    // m_rtSampleTime, m_fFirstSampleDelivered and m_llSampleMediaTimeStart
    CCritSec    m_cSharedState;     

    CRefTime     m_rtSampleTime;    // The time to be stamped on each sample
    HACMSTREAM m_hPCMToMSADPCMConversionStream;

    DWORD m_dwTempPCMBufferSize;
    bool m_fFirstSampleDelivered;
    LONGLONG m_llSampleMediaTimeStart;
	LONGLONG m_llCounter;

    CSynthFilter *m_pParent;
};

#endif // _AUDIOSYNTH_IMPLEMENTATION_ implementation only....





