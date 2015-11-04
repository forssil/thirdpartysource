/*! \file   transfrmextand.h
*   \author Keil 
*   \date   2015/1/7
*   \brief  base class of AudioProcessFilter
*/

#include <streams.h>          
#include "transfrmextand.h"      
#include <wchar.h>
#include <stdio.h>


// =================================================================
// Implements of CTransformExtandFilter
// =================================================================
CTransformExtandFilter::CTransformExtandFilter(TCHAR *pName, LPUNKNOWN pUnk, REFCLSID  clsid)
	: CBaseFilter(pName,pUnk,&m_csFilter, clsid)
	, m_bEOSDelivered(FALSE)
	, m_bQualityChanged(FALSE)
	, m_bSampleSkipped(FALSE)
#if FILTER_MODE_1_IN_2_OUT
	, m_pOnlineClassOutputPin(NULL)
	, m_pRecordOutputPin(NULL)
	, m_bOnlineClassOutputPin(FALSE)
	, m_bRecordOutputPin(FALSE)
	, m_onlineClassInputPinList(NAME("CTransformExtandFilter OnlineClass Input Pins list"))
	, m_nOnlineClassInputPinIndex(0)
	, m_nOnlineClassInputPinCount(0)
	, m_nOnlineClassInputPinCount_CalcUsed(0)
#else
	, m_bBBTInputPin(FALSE)
	, m_bZXKTInputPin(FALSE)
	, m_bDZGQInputPin(FALSE)
	, m_bZXKTOutputPin(FALSE)
	, m_bYYYZOutputPin(FALSE)
	, m_bLZOutputPin(FALSE)
	, m_nMicInputPinCount(0)
	, m_nMicInputPinCount_CalcUsed(0)
	, m_nMicInputPinIndex(0)
	, m_ZXKTOutputPin(NULL)
	, m_YYYXOutputPin(NULL)
	, m_LZOutputPin(NULL)
	, m_DZGQInputPin(NULL)
	, m_ZXKTInputPin(NULL)
	, m_BBTInputPin(NULL)
	, m_MicInputPinList(NAME("CTransformExtandFilter Mic Input Pins list"))
#endif
{

#ifdef PERF
    RegisterPerfId();
#endif //  PERF

}

#ifdef UNICODE
CTransformExtandFilter::CTransformExtandFilter(CHAR *pName, LPUNKNOWN pUnk, REFCLSID  clsid)
	: CBaseFilter(pName,pUnk,&m_csFilter, clsid)
	, m_bEOSDelivered(FALSE)
	, m_bQualityChanged(FALSE)
	, m_bSampleSkipped(FALSE)

#if FILTER_MODE_1_IN_2_OUT
	, m_pOnlineClassOutputPin(NULL)
	, m_pRecordOutputPin(NULL)
	, m_bOnlineClassOutputPin(FALSE)
	, m_bRecordOutputPin(FALSE)
	, m_onlineClassInputPinList(NAME("CTransformExtandFilter OnlineClass Input Pins list"))
	, m_nOnlineClassInputPinIndex(0)
	, m_nOnlineClassInputPinCount(0)
	, m_nOnlineClassInputPinCount_CalcUsed(0)
#else
	, m_bBBTInputPin(FALSE)
	, m_bZXKTInputPin(FALSE)
	, m_bDZGQInputPin(FALSE)
	, m_bZXKTOutputPin(FALSE)
	, m_bYYYZOutputPin(FALSE)
	, m_bLZOutputPin(FALSE)
	, m_nMicInputPinCount(0)
	, m_nMicInputPinCount_CalcUsed(0)
	, m_nMicInputPinIndex(0)
	, m_ZXKTOutputPin(NULL)
	, m_YYYXOutputPin(NULL)
	, m_LZOutputPin(NULL)
	, m_DZGQInputPin(NULL)
	, m_ZXKTInputPin(NULL)
	, m_BBTInputPin(NULL)
	, m_MicInputPinList(NAME("CTransformExtandFilter Mic Input Pins list"))
#endif

{
#ifdef PERF
    RegisterPerfId();
#endif //  PERF
}
#endif //UNICODE

// destructor
CTransformExtandFilter::~CTransformExtandFilter()
{

}

// return the number of pins we provide
int CTransformExtandFilter::GetPinCount()
{
	DbgBreak("CTransformExtandFilter::GetPinCount() should never be called");
	return 0;
}


// return a non-addrefed CBasePin * 
CBasePin *CTransformExtandFilter::GetPin(int n)
{
	UNREFERENCED_PARAMETER(n);
	DbgBreak("CTransformExtandFilter::GetPin() should never be called");
	return NULL;
}

// FindPin
STDMETHODIMP CTransformExtandFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{
	UNREFERENCED_PARAMETER(Id);
	UNREFERENCED_PARAMETER(ppPin);
	DbgBreak("CTransformExtandFilter::FindPin() should never be called");
	return E_FAIL;
}

// override these so that the derived filter can catch them
STDMETHODIMP CTransformExtandFilter::Stop()
{
	CAutoLock lck1(&m_csFilter);
	if (m_State == State_Stopped)
	{
		return NOERROR;
	}

	// Succeed the Stop if we are not completely connected
	if (FALSE == _ConnectStatusOfInputPin() || FALSE == _ConnectStatusOfOutputPin())
	{
		m_State = State_Stopped;
		m_bEOSDelivered = FALSE;
		return NOERROR;
	}
#if FILTER_MODE_1_IN_2_OUT
	// disconnect the input pin before locking or we can deadlock
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while(pos) 
	{
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		if (pInputPin->IsConnected()) 
		{
			pInputPin->Inactive();
		}
	}
	// synchronize with Receive calls
	CAutoLock lck2(&m_csReceive);
	// disconnect the output pin
	if (m_pOnlineClassOutputPin->IsConnected())
	{
		m_pOnlineClassOutputPin->Inactive();
	}
	if (m_pRecordOutputPin->IsConnected())
	{
		m_pRecordOutputPin->Inactive();
	}
#else
    // disconnect the input pin before locking or we can deadlock
	if (m_BBTInputPin->IsConnected())
	{
		m_BBTInputPin->Inactive();
	}
	if (m_ZXKTInputPin->IsConnected())
	{
		m_ZXKTInputPin->Inactive();
	}
	if (m_DZGQInputPin->IsConnected())
	{
		m_DZGQInputPin->Inactive();
	}
	POSITION pos = m_MicInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while(pos) 
	{
		pInputPin = m_MicInputPinList.GetNext(pos);
		if (pInputPin->IsConnected()) 
		{
			pInputPin->Inactive();
		}
	}

	// synchronize with Receive calls
	CAutoLock lck2(&m_csReceive);

	// disconnect the output pin
	if (m_ZXKTOutputPin->IsConnected())
	{
		m_ZXKTOutputPin->Inactive();
	}
	if (m_YYYXOutputPin->IsConnected())
	{
		m_YYYXOutputPin->Inactive();
	}
	if (m_LZOutputPin->IsConnected())
	{
		m_LZOutputPin->Inactive();
	}
#endif

	// allow a class derived from CTransformExtandFilter to know about starting and stopping streaming
	HRESULT hr = StopStreaming();
	if (SUCCEEDED(hr)) 
	{
		// complete the state transition
		m_State = State_Stopped;
		m_bEOSDelivered = FALSE;
	}
	return hr;
}


STDMETHODIMP CTransformExtandFilter::Pause()
{
	CAutoLock lck(&m_csFilter);

	HRESULT hr = NOERROR;
	if (m_State == State_Paused) 
	{
		// (This space left deliberately blank)
	}
	// If we have no input pin or it isn't yet connected then when we are asked to pause we deliver an end of stream to the downstream filter.
	// This makes sure that it doesn't sit there forever waiting for samples which we cannot ever deliver without an input connection.
	else if (_ConnectStatusOfInputPin() == FALSE) 
	{
		if (TRUE == _ConnectStatusOfOutputPin() && FALSE == m_bEOSDelivered)
		{
#if	FILTER_MODE_1_IN_2_OUT
			m_pOnlineClassOutputPin->IsConnected() ? m_pOnlineClassOutputPin->DeliverEndOfStream() : NULL;
			m_pRecordOutputPin->IsConnected() ? m_pRecordOutputPin->DeliverEndOfStream() : NULL;
			m_bEOSDelivered = TRUE;
#else
			m_ZXKTOutputPin->IsConnected() ? m_ZXKTOutputPin->DeliverEndOfStream() : NULL;
			m_YYYXOutputPin->IsConnected() ? m_YYYXOutputPin->DeliverEndOfStream() : NULL;
			m_LZOutputPin->IsConnected() ? m_LZOutputPin->DeliverEndOfStream() : NULL;
			m_bEOSDelivered = TRUE;
#endif
		}
		m_State = State_Paused;
	}
	// We may have an input connection but no output connection, However, if we have an input pin we do have an output pin
	else if (FALSE == _ConnectStatusOfOutputPin())
	{
		m_State = State_Paused;
	}

	else
	{
		if (m_State == State_Stopped) 
		{
			// allow a class derived from CTransformFilter
			// to know about starting and stopping streaming
			CAutoLock lck2(&m_csReceive);
			hr = StartStreaming();
		}
		if (SUCCEEDED(hr)) 
		{
			hr = CBaseFilter::Pause();
		}
	}

	m_bSampleSkipped = FALSE;
	m_bQualityChanged = FALSE;
	return hr;
}


// override these two functions if you want to inform something about entry to or exit from streaming state.
HRESULT CTransformExtandFilter::StartStreaming()
{
	DbgBreak("CTransformExtandFilter::StartStreaming() should never be called");
    return NOERROR;
}

HRESULT CTransformExtandFilter::StopStreaming()
{
	DbgBreak("CTransformExtandFilter::StopStreaming() should never be called");
    return NOERROR;
}

// Return S_FALSE to mean "pass the note on upstream"
// Return NOERROR (Same as S_OK)
// to mean "I've done something about it, don't pass it on"
HRESULT CTransformExtandFilter::AlterQuality(Quality q)
{
	UNREFERENCED_PARAMETER(q);
	return S_FALSE;
}

// override this to know when the media type is really set
HRESULT CTransformExtandFilter::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt, CAUDIO_S32_t nIndex)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pmt);
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT CTransformExtandFilter::CheckConnect(PIN_DIRECTION dir,IPin *pPin)
{
    UNREFERENCED_PARAMETER(dir);
    UNREFERENCED_PARAMETER(pPin);
    return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT CTransformExtandFilter::BreakConnect(PIN_DIRECTION dir, CAUDIO_S32_t nIndex)
{
    UNREFERENCED_PARAMETER(dir);
    return NOERROR;
}

// Let derived classes know about connection completion
HRESULT CTransformExtandFilter::CompleteConnect(PIN_DIRECTION direction,CAUDIO_S32_t nIndex, IPin *pReceivePin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pReceivePin);
    return NOERROR;
}



// override this to customize the transform process
HRESULT CTransformExtandFilter::Receive(CAUDIO_S32_t nIndex, IMediaSample *pSample)
{
	UNREFERENCED_PARAMETER(nIndex);
	UNREFERENCED_PARAMETER(pSample);
	DbgBreak("CTransformExtandFilter::Receive() should never be called");
	return E_UNEXPECTED;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT CTransformExtandFilter::EndOfStream(CAUDIO_S32_t nIndex)
{
    HRESULT hr = NOERROR;
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins

#if FILTER_MODE_1_IN_2_OUT
	if (m_pOnlineClassOutputPin->IsConnected())
	{
		hr = m_pOnlineClassOutputPin->DeliverEndOfStream();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_pRecordOutputPin->IsConnected())
	{
		hr = m_pRecordOutputPin->DeliverEndOfStream();
		if (FAILED(hr))
		{
			return hr;
		}
	}
#else
	 if (m_ZXKTOutputPin->IsConnected())
	{
        hr = m_ZXKTOutputPin->DeliverEndOfStream();
		if (FAILED(hr))
		{
			return hr;
		}
    }
	if (m_YYYXOutputPin->IsConnected())
	{
		hr = m_YYYXOutputPin->DeliverEndOfStream();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_LZOutputPin->IsConnected())
	{
		hr = m_LZOutputPin->DeliverEndOfStream();
		if (FAILED(hr))
		{
			return hr;
		}
	}
#endif

    return hr;
}

// enter flush state. Receives already blocked
// must override this if you have queued data or a worker thread
HRESULT CTransformExtandFilter::BeginFlush(void)
{
    HRESULT hr = NOERROR;
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins

#if FILTER_MODE_1_IN_2_OUT
	if (m_pOnlineClassOutputPin != NULL)
	{
		hr = m_pOnlineClassOutputPin->DeliverBeginFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_pRecordOutputPin != NULL)
	{
		hr = m_pRecordOutputPin->DeliverBeginFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
#else
	if (m_ZXKTOutputPin != NULL)
	{
		hr = m_ZXKTOutputPin->DeliverBeginFlush();
		if (FAILED(hr))
		{
			return hr;
		}
    }
	if (m_YYYXOutputPin != NULL)
	{
		hr = m_YYYXOutputPin->DeliverBeginFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_LZOutputPin != NULL)
	{
		hr = m_LZOutputPin->DeliverBeginFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
#endif

    return hr;
}

// leave flush state. must override this if you have queued data or a worker thread
HRESULT CTransformExtandFilter::EndFlush(void)
{
    // sync with pushing thread -- we have no worker thread
    // ensure no more data to go downstream -- we have no queued data
    // call EndFlush on downstream pins
	HRESULT hr = NOERROR;

#if FILTER_MODE_1_IN_2_OUT
	if (m_pOnlineClassOutputPin != NULL)
	{
		hr = m_pOnlineClassOutputPin->DeliverEndFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_pRecordOutputPin != NULL)
	{
		hr = m_pRecordOutputPin->DeliverEndFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
#else
	if (m_ZXKTOutputPin != NULL)
	{
		hr = m_ZXKTOutputPin->DeliverEndFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_YYYXOutputPin != NULL)
	{
		hr = m_YYYXOutputPin->DeliverEndFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_LZOutputPin != NULL)
	{
		hr = m_LZOutputPin->DeliverEndFlush();
		if (FAILED(hr))
		{
			return hr;
		}
	}
#endif
	
	return hr;

    // caller (the input pin's method) will unblock Receives
}


HRESULT CTransformExtandFilter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	HRESULT hr = NOERROR;

#if FILTER_MODE_1_IN_2_OUT
	if (m_pOnlineClassOutputPin != NULL)
	{
		hr = m_pOnlineClassOutputPin->DeliverNewSegment(tStart, tStop, dRate);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_pRecordOutputPin != NULL)
	{
		hr = m_pRecordOutputPin->DeliverNewSegment(tStart, tStop, dRate);
		if (FAILED(hr))
		{
			return hr;
		}
	}
#else
    if (m_ZXKTOutputPin != NULL)
	{
        hr = m_ZXKTOutputPin->DeliverNewSegment(tStart, tStop, dRate);
		if (FAILED(hr))
		{
			return hr;
		}
    }
	if (m_YYYXOutputPin != NULL)
	{
		hr = m_YYYXOutputPin->DeliverNewSegment(tStart, tStop, dRate);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (m_LZOutputPin != NULL)
	{
		hr = m_LZOutputPin->DeliverNewSegment(tStart, tStop, dRate);
		if (FAILED(hr))
		{
			return hr;
		}
	}
#endif

	return hr;
}

BOOL CTransformExtandFilter::GetSampleProps(AM_SAMPLE2_PROPERTIES** a_ppProps, TransportId_e a_eTPId)
{
	BOOL re=TRUE;

	if(NULL == a_ppProps)
	{
		return FALSE;
	}
	*a_ppProps = NULL;

	m_csReceive.Lock();

	if (State_Stopped == m_State)
	{
		m_csReceive.Unlock();
		return re;
	}

#if	FILTER_MODE_1_IN_2_OUT
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while (pos)
	{
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
		{
			*a_ppProps = pInputPin->SampleProps();
			break;
		}
	}
#else
	#if CAPTURE_PLAYBACK_INTEGRATED
		if (NULL != m_ZXKTInputPin && TRUE == m_ZXKTInputPin->IsConnected())
			*a_ppProps = m_ZXKTInputPin->SampleProps();
	#else
		if (TP2RenderDev == a_eTPId)
		{
			if (NULL != m_ZXKTInputPin && TRUE == m_ZXKTInputPin->IsConnected())
				*a_ppProps = m_ZXKTInputPin->SampleProps();
		}
		else
		{
			// check mike input pins
			POSITION pos = m_MicInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;
			while (pos)
			{
				pInputPin = m_MicInputPinList.GetNext(pos);
				if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
				{
					*a_ppProps = pInputPin->SampleProps();
					break;
				}
			}
		}
	#endif
#endif

	if(NULL == *a_ppProps)
	{
		re= FALSE;
	}
	
	m_csReceive.Unlock();
	return re;
}

//! check connection status of input pin 
/*virtual */BOOL CTransformExtandFilter::_ConnectStatusOfInputPin(CAUDIO_S32_t nIndex)
{
#if FILTER_MODE_1_IN_2_OUT
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while (pos)
	{
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
		{
			return TRUE;
		}
	}
	return FALSE;
#else
	switch (nIndex)
	{
		// OnlineClass_out
		case 0:
		{
	#if CAPTURE_PLAYBACK_INTEGRATED
			if (NULL != m_ZXKTInputPin && TRUE == m_ZXKTInputPin->IsConnected())
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
	#else
			// mike input pin 
			POSITION pos = m_MicInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;
			while (pos)
			{
				pInputPin = m_MicInputPinList.GetNext(pos);
				if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
				{
					return TRUE;
				}
			}
			return FALSE;
	#endif
		}
		// Speaker_out
		case 1:
		{
			if (NULL != m_ZXKTInputPin && TRUE == m_ZXKTInputPin->IsConnected())
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		// Recorder_out
		case 2:
		{
			// mike input pin 
			POSITION pos = m_MicInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;
			while (pos)
			{
				pInputPin = m_MicInputPinList.GetNext(pos);
				if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
				{
					return TRUE;
				}
			}
			return FALSE;
		}
		default:
		{
			if (NULL != m_ZXKTInputPin && TRUE == m_ZXKTInputPin->IsConnected())
			{
				return TRUE;
			}
			// mike input pin 
			POSITION pos = m_MicInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;
			while (pos)
			{
				pInputPin = m_MicInputPinList.GetNext(pos);
				if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
				{
					return TRUE;
				}
			}
			return FALSE;
		}
	}
#endif

	return FALSE;
}


//! check connection status of output pin 
/*virtual */BOOL CTransformExtandFilter::_ConnectStatusOfOutputPin(void)
{
#if FILTER_MODE_1_IN_2_OUT
	// on-line class output pin 
	if (NULL != m_pOnlineClassOutputPin && TRUE == m_pOnlineClassOutputPin->IsConnected())
	{
		return TRUE;
	}
	// recorder output pin 
	else if (NULL != m_pRecordOutputPin && TRUE == m_pRecordOutputPin->IsConnected())
	{
		return TRUE;
	}
#else
	// on-line class output pin 
	if (NULL != m_ZXKTOutputPin && TRUE == m_ZXKTOutputPin->IsConnected())
	{
		return TRUE;
	}
	// speaker output pin 
	else if (NULL != m_YYYXOutputPin && TRUE == m_YYYXOutputPin->IsConnected())
	{
		return TRUE;
	}
	// recorder output pin 
	else if (NULL != m_LZOutputPin && TRUE == m_LZOutputPin->IsConnected())
	{
		return TRUE;
	}
#endif

	return FALSE;
}


#if FILTER_MODE_1_IN_2_OUT
//! reset on-line class input pin list
/*virtual */BOOL CTransformExtandFilter::_ResetOnlineClassInputPinsList(void)
{
	// release all input pin
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;

	while (pos)
	{
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		pInputPin->Release();
	}
	m_onlineClassInputPinList.RemoveAll();

	m_vOnlineClassInputPinIndex_StoreUsed.clear();
	m_nOnlineClassInputPinCount = 0;
	m_nOnlineClassInputPinCount_CalcUsed = 0;
	m_nOnlineClassInputPinIndex = 0;

	return TRUE;
}

//! get pin which index is n from on-line class input pins list
/*virtual */CTransformExtandInputPin* CTransformExtandFilter::_GetOnlineClassInputPinFromList(CAUDIO_S32_t n)
{
	// Validate the position being asked for
	if (n > m_nOnlineClassInputPinCount)
	{
		return NULL;
	}

	// Make the number 1 based
	// Get the head of the list
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while (n)
	{
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		n--;
	}
	return pInputPin;
}

//! create a new on-line class input pin
/*virtual */BOOL CTransformExtandFilter::_CreateNewOnlineClassInputPin(CTransformExtandFilter *pFilter)
{
	// count of OnlineClass cannot beyond max number
	if (m_nOnlineClassInputPinCount >= MAX_THREAD_NUM)
	{
		++m_nOnlineClassInputPinCount_CalcUsed;
		return TRUE;
	}

	// prepare a name for the pin
	HRESULT hr = NOERROR;
	WCHAR name[16] = { 0 };
	swprintf(name, sizeof(name), L"OnlineClass%dIn", m_nOnlineClassInputPinIndex);
	CTransformExtandInputPin* pInputPin = new CTransformExtandInputPin(NAME("Input"), this, &hr, name, m_nOnlineClassInputPinIndex);
	if (FAILED(hr) || NULL == pInputPin)
	{
		delete pInputPin;
		return FALSE;
	}
	pInputPin->AddRef();
	++m_nOnlineClassInputPinCount;
	++m_nOnlineClassInputPinCount_CalcUsed;
	m_vOnlineClassInputPinIndex_StoreUsed.push_back(m_nOnlineClassInputPinIndex);
	++m_nOnlineClassInputPinIndex;
	m_onlineClassInputPinList.AddTail(pInputPin);
	IncrementPinVersion();
	return TRUE;
}

//! delete a on-line class input pin
/*virtual */BOOL CTransformExtandFilter::_DeleteOnlineClassInputPin(CTransformExtandInputPin *pPin)
{
	ASSERT(pPin);
	// choose which pin to delete
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	std::vector<CAUDIO_S32_t>::iterator iter = m_vOnlineClassInputPinIndex_StoreUsed.begin();
	while (pos)
	{
		POSITION posold = pos;         // Remember this position
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		if (pInputPin == pPin)
		{
			m_onlineClassInputPinList.Remove(posold);
			delete pPin;

			m_nOnlineClassInputPinCount--;
			if (m_nOnlineClassInputPinCount <= 1)
			{
				m_nOnlineClassInputPinCount_CalcUsed = 1;
			}
			else
			{
				m_nOnlineClassInputPinCount_CalcUsed--;
			}
			m_vOnlineClassInputPinIndex_StoreUsed.erase(iter);
			IncrementPinVersion();
			break;
		}
		iter++;
	}
	return TRUE;
}

//! get free on-line class input pin's count
/*virtual */CAUDIO_S32_t CTransformExtandFilter::_GetFreeOnlineClassInputPinCount(void)
{
	CAUDIO_S32_t n = 0;
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while(pos) 
	{
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		if (pInputPin && pInputPin->m_Connected == NULL)
			n++;
	}
	return n;
}

#else
//! reset mic input pin list
/*virtual */BOOL CTransformExtandFilter::_ResetMicInputPinsList(void)
{
	// release all input pin
	POSITION pos = m_MicInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;

	while(pos)
	{
		pInputPin = m_MicInputPinList.GetNext(pos);
		pInputPin->Release();
	}
	m_MicInputPinList.RemoveAll();

	m_vMicInputPinIndex_StoreUsed.clear();
	m_nMicInputPinCount = 0;
	m_nMicInputPinCount_CalcUsed = 0;
	m_nMicInputPinIndex = 0;

	return TRUE;
}

//! get pin which index is n from mic input pins list
/*virtual */CTransformExtandInputPin* CTransformExtandFilter::_GetMicInputPinFromList(CAUDIO_S32_t n)
{
	// Validate the position being asked for
	if (n > m_nMicInputPinCount)
	{
		return NULL;
	}
		
	// Make the number 1 based
	// Get the head of the list
	POSITION pos = m_MicInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while(n) 
	{
		pInputPin = m_MicInputPinList.GetNext(pos);
		n--;
	}
	return pInputPin;
}

//! create a new mic input pin
/*virtual */BOOL CTransformExtandFilter::_CreateNewMicInputPin(CTransformExtandFilter *pFilter)
{
	// count of mic cannot beyond max number
	if (m_nMicInputPinCount >= MAX_THREAD_NUM-1-1-1)
	{
		++m_nMicInputPinCount_CalcUsed;
		return TRUE;
	}

	// prepare a name for the pin
	HRESULT hr = NOERROR;
	WCHAR name[16] = {0};
	swprintf(name, sizeof(name), L"Mic%dIn", m_nMicInputPinIndex);
	CTransformExtandInputPin* pInputPin = new CTransformExtandInputPin(NAME("Input"), this, &hr, name, m_nMicInputPinIndex);
	if(FAILED(hr) || NULL == pInputPin)
	{
		delete pInputPin;
		return FALSE;
	}
	pInputPin->AddRef();
	++m_nMicInputPinCount;
	++m_nMicInputPinCount_CalcUsed;
    m_vMicInputPinIndex_StoreUsed.push_back(m_nMicInputPinIndex);
	++m_nMicInputPinIndex;
	m_MicInputPinList.AddTail(pInputPin);
	IncrementPinVersion();
	return TRUE;
}

//! delete a mic input pin
/*virtual */BOOL CTransformExtandFilter::_DeleteMicInputPin(CTransformExtandInputPin * pPin)
{
	ASSERT(pPin);
	// choose which pin to delete
	POSITION pos = m_MicInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	std::vector<CAUDIO_S32_t>::iterator iter = m_vMicInputPinIndex_StoreUsed.begin();
	while(pos) 
	{
		POSITION posold = pos;         // Remember this position
		pInputPin = m_MicInputPinList.GetNext(pos);
		if (pInputPin == pPin) 
		{
			m_MicInputPinList.Remove(posold);
			delete pPin;

			m_nMicInputPinCount--;
			if(m_nMicInputPinCount<=1)
			{
				m_nMicInputPinCount_CalcUsed = 1;
			}
			else
			{
				m_nMicInputPinCount_CalcUsed--;
			}
			m_vMicInputPinIndex_StoreUsed.erase(iter);
			IncrementPinVersion();
			break;
		}
		iter++;
	}
	return TRUE;
}

//! get free mid input pin's count
/*virtual */CAUDIO_S32_t CTransformExtandFilter::_GetFreeMicInputPinCount(void)
{
	CAUDIO_S32_t n = 0;
	POSITION pos = m_MicInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while(pos) 
	{
		pInputPin = m_MicInputPinList.GetNext(pos);
		if (pInputPin && pInputPin->m_Connected == NULL)
			n++;
	}
	return n;
}
#endif

bool CTransformExtandFilter::IsDataComeIn()
{
#if FILTER_MODE_1_IN_2_OUT
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	CTransformExtandInputPin *pInputPin = NULL;
	while(pos) 
	{
		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		if (pInputPin && pInputPin->m_Connected != NULL)
		{
			return pInputPin->m_bIsDataComeIn;
		}
	}
#else
	return m_ZXKTInputPin->m_bIsDataComeIn;
#endif
}


// =================================================================
// Implements of CTransformExtandInputPin
// =================================================================
// Check streaming status
HRESULT CTransformExtandInputPin::CheckStreaming()
{
	if (FALSE == m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
        return VFW_E_NOT_CONNECTED;
    }
	else
	{
        //  Shouldn't be able to get any data if we're not connected!
        ASSERT(IsConnected());
        //  we're flushing
        if (m_bFlushing)
		{
            return S_FALSE;
        }
        //  Don't process stuff in Stopped state
        if (IsStopped())
		{
            return VFW_E_WRONG_STATE;
        }
        if (m_bRunTimeError)
		{
    	    return VFW_E_RUNTIME_ERROR;
        }
        return S_OK;
    }
}

// constructor
CTransformExtandInputPin::CTransformExtandInputPin( TCHAR *pObjectName, CTransformExtandFilter *pTransformFilter,  HRESULT * phr,  LPCWSTR pName, CAUDIO_S32_t nIndex)
	: CBaseInputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pName)
	, m_nIndex(nIndex)
	, m_cOurRef(0)
	, m_pTransformFilter(NULL)
	, m_bIsDataComeIn(false)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformInputPin::CTransformInputPin")));
    m_pTransformFilter = pTransformFilter;
}

#ifdef UNICODE
CTransformExtandInputPin::CTransformExtandInputPin(CHAR *pObjectName, CTransformExtandFilter *pTransformFilter,  HRESULT * phr,  LPCWSTR pName, CAUDIO_S32_t nIndex)
	: CBaseInputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pName)
	, m_nIndex(nIndex)
	, m_cOurRef(0)
	, m_pTransformFilter(NULL)
	, m_bIsDataComeIn(false)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformInputPin::CTransformInputPin")));
    m_pTransformFilter = pTransformFilter;
}
#endif

// Override since the life time of pins and filters are not the same
//
// NonDelegatingAddRef
//
// We need override this method so that we can do proper reference counting
// on our output pin. The base class CBasePin does not do any reference
// counting on the pin in RETAIL.
//
// Please refer to the comments for the NonDelegatingRelease method for more
// info on why we need to do this.
//
STDMETHODIMP_(ULONG) CTransformExtandInputPin::NonDelegatingAddRef()
{
	CAutoLock lock_it(m_pLock);

#ifdef DEBUG
	// Update the debug only variable maintained by the base class
	m_cRef++;
	ASSERT(m_cRef > 0);
#endif

	// Now update our reference count
	m_cOurRef++;
	ASSERT(m_cOurRef > 0);
	return m_cOurRef;
}

//
// NonDelegatingRelease
//
// CTeeOutputPin overrides this class so that we can take the pin out of our
// output pins list and delete it when its reference count drops to 1 and there
// is atleast two free pins.
//
// Note that CreateNextOutputPin holds a reference count on the pin so that
// when the count drops to 1, we know that no one else has the pin.
//
// Moreover, the pin that we are about to delete must be a free pin(or else
// the reference would not have dropped to 1, and we must have atleast one
// other free pin(as the filter always wants to have one more free pin)
//
// Also, since CBasePin::NonDelegatingAddRef passes the call to the owning
// filter, we will have to call Release on the owning filter as well.
//
// Also, note that we maintain our own reference count m_cOurRef as the m_cRef
// variable maintained by CBasePin is debug only.
//
STDMETHODIMP_(ULONG) CTransformExtandInputPin::NonDelegatingRelease()
{
	CAutoLock lock_it(m_pLock);

#ifdef DEBUG
	// Update the debug only variable in CBasePin
	m_cRef--;
	ASSERT(m_cRef >= 0);
#endif

	// Now update our reference count
	m_cOurRef--;
	ASSERT(m_cOurRef >= 0);

	// if the reference count on the object has gone to one, remove
	// the pin from our output pins list and physically delete it
	// provided there are atealst two free pins in the list(including
	// this one)

	// Also, when the ref count drops to 0, it really means that our
	// filter that is holding one ref count has released it so we
	// should delete the pin as well.

#if FILTER_MODE_1_IN_2_OUT
	if (m_cOurRef <= 1)
	{
		int n = 2;                     // default forces pin deletion
		if(m_cOurRef == 1)
		{
			// Walk the list of pins, looking for count of free pins
			n = m_pTransformFilter->_GetFreeOnlineClassInputPinCount();
		}

		// If there are two free pins, delete this one.
		// NOTE: normall
		if(n >= 2)
		{
			m_cOurRef = 0;
#ifdef DEBUG
			m_cRef = 0;
#endif
			m_pTransformFilter->_DeleteOnlineClassInputPin(this);

			return(ULONG) 0;
		}
	}
#else
	if(m_cOurRef <= 1)
	{
		int n = 2;                     // default forces pin deletion
		if(m_cOurRef == 1)
		{
			// Walk the list of pins, looking for count of free pins
			n = m_pTransformFilter->_GetFreeMicInputPinCount();
		}

		// If there are two free pins, delete this one.
		// NOTE: normall
		if(n >= 2)
		{
			m_cOurRef = 0;
#ifdef DEBUG
			m_cRef = 0;
#endif
			m_pTransformFilter->_DeleteMicInputPin(this);

			return(ULONG) 0;
		}
	}
#endif

	return(ULONG) m_cOurRef;
}

// provides derived filter a chance to grab extra interfaces
HRESULT CTransformExtandInputPin::CheckConnect(IPin *pPin)
{
    HRESULT hr = m_pTransformFilter->CheckConnect(PINDIR_INPUT,pPin);
    if (FAILED(hr)) 
	{
    	return hr;
    }
    return CBaseInputPin::CheckConnect(pPin);
}

// provides derived filter a chance to release it's extra interfaces
HRESULT CTransformExtandInputPin::BreakConnect()
{
    //  Can't disconnect unless stopped
    ASSERT(IsStopped());
	if(IsConnected())
		m_pTransformFilter->BreakConnect(PINDIR_INPUT, m_nIndex);
    return CBaseInputPin::BreakConnect();
}

// Let derived class know when the input pin is connected
HRESULT CTransformExtandInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
	{
        return hr;
    }
    return m_pTransformFilter->CompleteConnect(PINDIR_INPUT,m_nIndex, pReceivePin);
}

// check that we can support a given media type
HRESULT CTransformExtandInputPin::CheckMediaType(const CMediaType* pmt)
{
    // Check the input type
    HRESULT hr = m_pTransformFilter->CheckInputType(pmt,1);
    if (S_OK != hr)
	{
        return hr;
    }
    // if the output pin is still connected, then we have to check the transform not just the input format
	if (m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
		CMediaType *mt;

#if FILTER_MODE_1_IN_2_OUT
		mt =  m_pTransformFilter->m_pOnlineClassOutputPin->IsConnected() 
			? &m_pTransformFilter->m_pOnlineClassOutputPin->CurrentMediaType() 
			: &m_pTransformFilter->m_pRecordOutputPin->CurrentMediaType();
#else
		mt = m_pTransformFilter->m_ZXKTOutputPin->IsConnected() ? &m_pTransformFilter->m_ZXKTOutputPin->CurrentMediaType() :
			(m_pTransformFilter->m_YYYXOutputPin->IsConnected() ? &m_pTransformFilter->m_YYYXOutputPin->CurrentMediaType() :
			&m_pTransformFilter->m_LZOutputPin->CurrentMediaType());
#endif

		return m_pTransformFilter->CheckTransform(pmt, mt, m_nIndex);
    }
	else
	{
        return hr;
    }
}

// set the media type for this connection
HRESULT CTransformExtandInputPin::SetMediaType(const CMediaType* mtIn)
{
    // Set the base class media type (should always succeed)
    HRESULT hr = CBasePin::SetMediaType(mtIn);
    if (FAILED(hr))
	{
        return hr;
    }

    // check the transform can be done (should always succeed)
    ASSERT(SUCCEEDED(m_pTransformFilter->CheckInputType(mtIn,1)));

    return m_pTransformFilter->SetMediaType(PINDIR_INPUT,mtIn,m_nIndex);
}

// provide EndOfStream that passes straight downstream
STDMETHODIMP
CTransformExtandInputPin::EndOfStream(void)
{
    CAutoLock lck(&m_pTransformFilter->m_csReceive);
    HRESULT hr = CheckStreaming();
	// TODO:test code
    if (S_OK == hr)
	{
#if FILTER_MODE_1_IN_2_OUT
		// get sequence of input pin
		CAUDIO_S32_t nSeq = 0;
		for (; nSeq < (m_pTransformFilter->m_nOnlineClassInputPinCount_CalcUsed - 1); ++nSeq)
		{
			if (m_pTransformFilter->m_vOnlineClassInputPinIndex_StoreUsed[nSeq] == m_nIndex)
			{
				break;
			}
		}
		ASSERT(nSeq < (m_pTransformFilter->m_nOnlineClassInputPinCount_CalcUsed-1) );
		hr = m_pTransformFilter->EndOfStream(nSeq);
#else
		// get sequence of input pin
		CAUDIO_S32_t nSeq = 0;
		for (; nSeq < (m_pTransformFilter->m_nMicInputPinCount_CalcUsed-1); ++nSeq)
		{
			if (m_pTransformFilter->m_vMicInputPinIndex_StoreUsed[nSeq] == m_nIndex)
			{
				break;
			}
		}
		ASSERT(nSeq < (m_pTransformFilter->m_nMicInputPinCount_CalcUsed-1) );
       hr = m_pTransformFilter->EndOfStream(nSeq);
#endif
    }
    return hr;
}


// enter flushing state. Call default handler to block Receives, then pass to overridable method in filter
STDMETHODIMP CTransformExtandInputPin::BeginFlush(void)
{
    CAutoLock lck(&m_pTransformFilter->m_csFilter);
    //  Are we actually doing anything?
	if (!IsConnected() || !m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
        return VFW_E_NOT_CONNECTED;
    }
    HRESULT hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr))
	{
    	return hr;
    }

    return m_pTransformFilter->BeginFlush();
}


// leave flushing state.
// Pass to overridable method in filter, then call base class to unblock receives (finally)
STDMETHODIMP CTransformExtandInputPin::EndFlush(void)
{
    CAutoLock lck(&m_pTransformFilter->m_csFilter);
    //  Are we actually doing anything?
    //ASSERT(m_pTransformFilter->m_ZXKTOutputPin != NULL);
	if (!IsConnected() || !m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
        return VFW_E_NOT_CONNECTED;
    }

    HRESULT hr = m_pTransformFilter->EndFlush();
    if (FAILED(hr))
	{
        return hr;
    }

    return CBaseInputPin::EndFlush();
}

// here's the next block of data from the stream.
// AddRef it yourself if you need to hold it beyond the end of this call.

HRESULT CTransformExtandInputPin::Receive(IMediaSample * pSample)
{
    HRESULT hr;
    CAutoLock lck(&m_pTransformFilter->m_csReceive);
    ASSERT(pSample);

	m_bIsDataComeIn = true;

    // check all is well with the base class
    hr = CBaseInputPin::Receive(pSample);
    if (S_OK == hr)
	{
        hr = m_pTransformFilter->Receive(m_nIndex, pSample);
    }
    return hr;
}

// override to pass downstream
STDMETHODIMP CTransformExtandInputPin::NewSegment(REFERENCE_TIME tStart,  REFERENCE_TIME tStop, double dRate)
{
    //  Save the values in the pin
    CBasePin::NewSegment(tStart, tStop, dRate);
    return m_pTransformFilter->NewSegment(tStart, tStop, dRate);
}
//merged from v0.5 end
// =================================================================
// Implements of CTransformOriginInputPin
// =================================================================
// constructor
CTransformOriginInputPin::CTransformOriginInputPin(TCHAR *pObjectName, CTransformExtandFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t nIndex)
	: CBaseInputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pName)
	, m_nIndex(nIndex)
	, m_cOurRef(0)
	, m_pTransformFilter(NULL)
	, m_bIsDataComeIn(false)

{
	DbgLog((LOG_TRACE, 2, TEXT("CTransformInputPin::CTransformInputPin")));
	m_pTransformFilter = pTransformFilter;
}

#ifdef UNICODE
CTransformOriginInputPin::CTransformOriginInputPin(CHAR *pObjectName, CTransformExtandFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t nIndex)
	: CBaseInputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pName)
	, m_nIndex(nIndex)
	, m_cOurRef(0)
	, m_pTransformFilter(NULL)
	, m_bIsDataComeIn(false)

{
	DbgLog((LOG_TRACE, 2, TEXT("CTransformInputPin::CTransformInputPin")));
	m_pTransformFilter = pTransformFilter;
}
#endif

// provides derived filter a chance to grab extra interfaces
HRESULT CTransformOriginInputPin::CheckConnect(IPin *pPin)
{
	HRESULT hr = m_pTransformFilter->CheckConnect(PINDIR_INPUT, pPin);
	if (FAILED(hr))
	{
		return hr;
	}
	return CBaseInputPin::CheckConnect(pPin);
}

// provides derived filter a chance to release it's extra interfaces
HRESULT CTransformOriginInputPin::BreakConnect()
{
	//  Can't disconnect unless stopped
	ASSERT(IsStopped());
	if (IsConnected())
		m_pTransformFilter->BreakConnect(PINDIR_INPUT, m_nIndex);
	return CBaseInputPin::BreakConnect();
}

// Let derived class know when the input pin is connected
HRESULT CTransformOriginInputPin::CompleteConnect(IPin *pReceivePin)
{
	HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
	if (FAILED(hr))
	{
		return hr;
	}
	return m_pTransformFilter->CompleteConnect(PINDIR_INPUT, m_nIndex, pReceivePin);
}

// Check streaming status
HRESULT CTransformOriginInputPin::CheckStreaming()
{
	if (FALSE == m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
		return VFW_E_NOT_CONNECTED;
	}
	else
	{
		//  Shouldn't be able to get any data if we're not connected!
		ASSERT(IsConnected());
		//  we're flushing
		if (m_bFlushing)
		{
			return S_FALSE;
		}
		//  Don't process stuff in Stopped state
		if (IsStopped())
		{
			return VFW_E_WRONG_STATE;
		}
		if (m_bRunTimeError)
		{
			return VFW_E_RUNTIME_ERROR;
		}
		return S_OK;
	}
}

// check that we can support a given media type
HRESULT CTransformOriginInputPin::CheckMediaType(const CMediaType* pmt)
{
	// Check the input type
	HRESULT hr = m_pTransformFilter->CheckInputType(pmt, m_nIndex);
	if (S_OK != hr)
	{
		return hr;
	}
	// if the output pin is still connected, then we have to check the transform not just the input format
	if (m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
		CMediaType *mt;

#if FILTER_MODE_1_IN_2_OUT
		mt =  m_pTransformFilter->m_pOnlineClassOutputPin->IsConnected() 
			? &m_pTransformFilter->m_pOnlineClassOutputPin->CurrentMediaType() 
			: &m_pTransformFilter->m_pRecordOutputPin->CurrentMediaType();
#else
			mt = m_pTransformFilter->m_ZXKTOutputPin->IsConnected() ? &m_pTransformFilter->m_ZXKTOutputPin->CurrentMediaType() :
			(m_pTransformFilter->m_YYYXOutputPin->IsConnected() ? &m_pTransformFilter->m_YYYXOutputPin->CurrentMediaType() :
			&m_pTransformFilter->m_LZOutputPin->CurrentMediaType());
#endif

		return m_pTransformFilter->CheckTransform(pmt, mt, m_nIndex);
	}
	else
	{
		return hr;
	}
}

// set the media type for this connection
HRESULT CTransformOriginInputPin::SetMediaType(const CMediaType* mtIn)
{
	// Set the base class media type (should always succeed)
	HRESULT hr = CBasePin::SetMediaType(mtIn);
	if (FAILED(hr))
	{
		return hr;
	}

	// check the transform can be done (should always succeed)
	ASSERT(SUCCEEDED(m_pTransformFilter->CheckInputType(mtIn, m_nIndex)));

	return m_pTransformFilter->SetMediaType(PINDIR_INPUT, mtIn, m_nIndex);
}

// provide EndOfStream that passes straight downstream
STDMETHODIMP
CTransformOriginInputPin::EndOfStream(void)
{
	CAutoLock lck(&m_pTransformFilter->m_csReceive);
	HRESULT hr = CheckStreaming();
	hr = m_pTransformFilter->EndOfStream(m_nIndex);
	return hr;
}


// enter flushing state. Call default handler to block Receives, then pass to overridable method in filter
STDMETHODIMP CTransformOriginInputPin::BeginFlush(void)
{
	CAutoLock lck(&m_pTransformFilter->m_csFilter);
	//  Are we actually doing anything?
	if (!IsConnected() || !m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
		return VFW_E_NOT_CONNECTED;
	}
	HRESULT hr = CBaseInputPin::BeginFlush();
	if (FAILED(hr))
	{
		return hr;
	}

	return m_pTransformFilter->BeginFlush();
}


// leave flushing state.
// Pass to overridable method in filter, then call base class to unblock receives (finally)
STDMETHODIMP CTransformOriginInputPin::EndFlush(void)
{
	CAutoLock lck(&m_pTransformFilter->m_csFilter);
	//  Are we actually doing anything?
	if (!IsConnected() || !m_pTransformFilter->_ConnectStatusOfOutputPin())
	{
		return VFW_E_NOT_CONNECTED;
	}

	HRESULT hr = m_pTransformFilter->EndFlush();
	if (FAILED(hr))
	{
		return hr;
	}

	return CBaseInputPin::EndFlush();
}

// here's the next block of data from the stream.
// AddRef it yourself if you need to hold it beyond the end of this call.

HRESULT CTransformOriginInputPin::Receive(IMediaSample * pSample)
{
	
	HRESULT hr;
	CAutoLock lck(&m_pTransformFilter->m_csReceive);
	ASSERT(pSample);

	m_bIsDataComeIn = true;

	//AUDIO_PROCESSING_PRINTF("input pin index is %d, Entry Receive()", m_nIndex);
	// check all is well with the base class
	hr = CBaseInputPin::Receive(pSample);
	if (S_OK == hr)
	{
		hr = m_pTransformFilter->Receive(m_nIndex, pSample);
	}
	//AUDIO_PROCESSING_PRINTF("input pin index is %d, Leave Receive()", m_nIndex);
	return hr;
}

// override to pass downstream
STDMETHODIMP CTransformOriginInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	//  Save the values in the pin
	CBasePin::NewSegment(tStart, tStop, dRate);
	return m_pTransformFilter->NewSegment(tStart, tStop, dRate);
}


// =================================================================
// Implements of CTransformExtandOutputPin
// =================================================================
// constructor
CTransformExtandOutputPin::CTransformExtandOutputPin(TCHAR *pObjectName,  CTransformExtandFilter *pTransformFilter,  HRESULT * phr, LPCWSTR pPinName, CAUDIO_S32_t nIndex)
	: CBaseOutputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pPinName)
	, m_nIndex(nIndex)
	, m_pPosition(NULL)
	, m_pTransformFilter(NULL)
	, m_pWavFileOp(NULL)
	, m_pWavFileHead(NULL)
	, m_nWaveCounter(0)
	, m_nSampleSize(0)
	, m_pOutBuffer(NULL)
	, m_nSampleRate_in(0)
	, m_nSampleRate_out(0)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformOutputPin::CTransformOutputPin")));
    m_pTransformFilter = pTransformFilter;

	ZeroMemory(&m_pProp, sizeof(ALLOCATOR_PROPERTIES));
	m_pProp.cBuffers = 1;
	m_pProp.cbBuffer = 1;

}

#ifdef UNICODE
CTransformExtandOutputPin::CTransformExtandOutputPin( CHAR *pObjectName, CTransformExtandFilter *pTransformFilter,  HRESULT * phr, LPCWSTR pPinName, CAUDIO_S32_t nIndex)
	: CBaseOutputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pPinName)
	, m_pPosition(NULL)
	, m_nIndex(nIndex)
	, m_pTransformFilter(NULL)
	, m_pWavFileOp(NULL)
	, m_pWavFileHead(NULL)
	, m_nWaveCounter(0)
	, m_nSampleSize(0)
	, m_pOutBuffer(NULL)
	, m_nSampleRate_in(0)
	, m_nSampleRate_out(0)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformOutputPin::CTransformOutputPin")));
    m_pTransformFilter = pTransformFilter;

	ZeroMemory(&m_pProp, sizeof(ALLOCATOR_PROPERTIES));
	m_pProp.cBuffers = 1;
	m_pProp.cbBuffer = 1;

}
#endif

// destructor
CTransformExtandOutputPin::~CTransformExtandOutputPin()
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformOutputPin::~CTransformOutputPin")));

	if(NULL != m_pOutBuffer)
	{
		delete[] m_pOutBuffer;
		m_pOutBuffer = NULL;
	}

    if (m_pPosition)
		m_pPosition->Release();
	m_nIndex = -1;
}

HRESULT CTransformExtandOutputPin::InitWavFile(std::string file_name, CAUDIO_U32_t sample_rate)
{
	m_pWavFileOp = new CWavFileOp(const_cast<char *>(file_name.c_str()), "wb");
	m_pWavFileHead = new SWavFileHead;
	if (-2 == m_pWavFileOp->m_FileStatus)
	{
		delete m_pWavFileOp;
		delete m_pWavFileHead;
		m_pWavFileOp = NULL;
		m_pWavFileHead = NULL;
		return E_FAIL;
	}
	m_pWavFileHead->NChannels = 2;
	m_pWavFileHead->RIFF[0] = 'R';
	m_pWavFileHead->RIFF[1] = 'I';
	m_pWavFileHead->RIFF[2] = 'F';
	m_pWavFileHead->RIFF[3] = 'F';
	m_pWavFileHead->data[0] = 'd';
	m_pWavFileHead->data[1] = 'a';
	m_pWavFileHead->data[2] = 't';
	m_pWavFileHead->data[3] = 'a';
	m_pWavFileHead->WAVEfmt_[0] = 'W';
	m_pWavFileHead->WAVEfmt_[1] = 'A';
	m_pWavFileHead->WAVEfmt_[2] = 'V';
	m_pWavFileHead->WAVEfmt_[3] = 'E';
	m_pWavFileHead->WAVEfmt_[4] = 'f';
	m_pWavFileHead->WAVEfmt_[5] = 'm';
	m_pWavFileHead->WAVEfmt_[6] = 't';
	m_pWavFileHead->WAVEfmt_[7] = ' ';

	m_pWavFileHead->noUse = 0x00000010;
	m_pWavFileHead->FormatCategory = 1;
	m_pWavFileHead->SampleRate = sample_rate;
	m_pWavFileHead->SampleBytes = sample_rate * 4;
	m_pWavFileHead->BytesPerSample = 4;
	m_pWavFileHead->NBitsPersample = 16;

	m_nWaveCounter = 0;
	if (m_pWavFileOp)
		m_pWavFileOp->WriteHeader(*m_pWavFileHead);
	
	return S_OK;
}

HRESULT CTransformExtandOutputPin::ReleaseWavFile(void)
{
	if (m_pWavFileHead && m_pWavFileOp)
	{
		m_pWavFileOp->UpdateHeader(m_pWavFileHead->NChannels, m_nWaveCounter / m_pWavFileHead->NChannels);
		delete m_pWavFileOp;
		delete m_pWavFileHead;
		m_pWavFileOp = NULL;
		m_pWavFileHead = NULL;
	}

	return S_OK;
}

HRESULT CTransformExtandOutputPin::WriteWavFile(CAUDIO_S16_t *data, CAUDIO_S32_t size)
{
	if (m_pWavFileHead && m_pWavFileOp)
	{
		m_pWavFileOp->WriteSample(data, size);
		m_nWaveCounter += size;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

CAUDIO_U32_t CTransformExtandOutputPin::Ceil(AUDIO_DATA_TYPE data)
{
	if (data <= 0)
	{
		return 0;
	}
	if (data > static_cast<CAUDIO_U32_t>(data))
	{
		return static_cast<CAUDIO_U32_t>(data)+1;
	}
	else
	{
		return static_cast<CAUDIO_U32_t>(data);
	}
}

bool CTransformExtandOutputPin::IsDataComeIn()
{
	return m_pTransformFilter->IsDataComeIn();
}

// overriden to expose IMediaPosition and IMediaSeeking control interfaces
STDMETHODIMP CTransformExtandOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));
	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}

// provides derived filter a chance to grab extra interfaces
HRESULT CTransformExtandOutputPin::CheckConnect(IPin *pPin)
{
    // we should have an input connection first
    if ((m_pTransformFilter->_ConnectStatusOfInputPin() == FALSE))
	{
	    return E_UNEXPECTED;
    }

    HRESULT hr = m_pTransformFilter->CheckConnect(PINDIR_OUTPUT,pPin);
    if (FAILED(hr))
	{
	    return hr;
    }
    return CBaseOutputPin::CheckConnect(pPin);
}

// provides derived filter a chance to release it's extra interfaces
HRESULT CTransformExtandOutputPin::BreakConnect()
{
    //  Can't disconnect unless stopped
    ASSERT(IsStopped());
    m_pTransformFilter->BreakConnect(PINDIR_OUTPUT, m_nIndex);

    return CBaseOutputPin::BreakConnect();
}

// Let derived class know when the output pin is connected
HRESULT CTransformExtandOutputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = m_pTransformFilter->CompleteConnect(PINDIR_OUTPUT,m_nIndex,pReceivePin);
    if (FAILED(hr))
	{
        return hr;
    }
    return CBaseOutputPin::CompleteConnect(pReceivePin);
}

// check a given transform - must have selected input type firs
HRESULT CTransformExtandOutputPin::CheckMediaType(const CMediaType* pmtOut)
{
	HRESULT hr = S_OK;
    // must have selected input first
    if ((m_pTransformFilter->_ConnectStatusOfInputPin() == FALSE))
	{
	        return E_INVALIDARG;
    }
    return hr;
}


// called after we have agreed a media type to actually set it in which case
// we run the CheckTransform function to get the output format type again

HRESULT CTransformExtandOutputPin::SetMediaType(const CMediaType* pmtOut)
{
    HRESULT hr = NOERROR;
    // Set the base class media type (should always succeed)
    hr = CBasePin::SetMediaType(pmtOut);
    if (FAILED(hr))
	{
        return hr;
    }

#ifdef DEBUG
    if (FAILED(CheckMediaType(pmtOut)))
	{
		DbgLog((LOG_ERROR,0,TEXT("*** This filter is accepting an output media type")));
		DbgLog((LOG_ERROR,0,TEXT("    that it can't currently transform to.  I hope")));
		DbgLog((LOG_ERROR,0,TEXT("    it's smart enough to reconnect its input.")));
    }
#endif

    return m_pTransformFilter->SetMediaType(PINDIR_OUTPUT,pmtOut,m_nIndex);
}

bool CTransformExtandOutputPin::Transport(
	const void* a_pData,
	DATA_TYPE a_eDataType,
	CAUDIO_U32_t a_nSize, 
	TransportId_e a_eTPId)
{
	if(!IsConnected())
	{
		return false;
	}

	if(NULL == a_pData)
	{
		return false;
	}

	if(REAL_FIX_DATA != a_eDataType)
	{
		return false;
	}

	if(a_eTPId != m_eTPId)
	{
		return false;
	}

#if CAPTURE_PLAYBACK_INTEGRATED
	if (!IsDataComeIn())
	{
		return true;
	}
#endif

	// save one channel data
	CAUDIO_U32_t sizeleft = m_nOutBufferSize - m_nOutBufferWritePtr;
	CAUDIO_U32_t copySize = a_nSize;
	copySize = (copySize>sizeleft) ? sizeleft : copySize;
	memcpy_s(m_pOutBuffer + m_nOutBufferWritePtr, sizeleft*sizeof(CAUDIO_S16_t),
		(CAUDIO_S16_t*)a_pData, static_cast<rsize_t>(copySize*sizeof(CAUDIO_S16_t)));
	m_nOutBufferWritePtr += copySize;

	if (m_nSampleSize > m_nOutBufferWritePtr)
	{
		return true;
	}

	while (m_nSampleSize <= m_nOutBufferWritePtr)
	{
		AUDIO_PROCESSING_PRINTF("input pin index is %d, start transport data", m_nIndex);

		IMediaSample * pOutSample = NULL;
		HRESULT hr = NOERROR;
		BYTE* pBufOut = NULL;
		// Set up the output sample
		m_pTransformFilter->m_csReceive.Lock();
		hr = InitializeOutputSample(&pOutSample, a_eTPId);
		m_pTransformFilter->m_csReceive.Unlock();
		if (FAILED(hr))
		{
			return true;
		}
		//
		if(NULL==pOutSample)
		{
			return false;
		}

		hr = pOutSample->GetPointer(&pBufOut);
		CAUDIO_U32_t sample_size = pOutSample->GetActualDataLength() / sizeof(CAUDIO_S16_t);
		ASSERT(SUCCEEDED(hr));
		CAUDIO_S16_t *psOut = (CAUDIO_S16_t*)pBufOut;

		for (CAUDIO_U32_t i = 0; i < m_nSampleSize; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				*psOut++ = m_pOutBuffer[i];
			}
		}

		m_nOutBufferWritePtr -= m_nSampleSize;
		memmove_s(m_pOutBuffer, m_nOutBufferSize*sizeof(CAUDIO_S16_t), m_pOutBuffer + m_nSampleSize, m_nOutBufferWritePtr*sizeof(CAUDIO_S16_t));

#ifdef AUDIO_WAVE_DEBUG
		// write sample data to wave file
		WriteWavFile(psOut - m_nSampleSize * 2, static_cast<CAUDIO_S32_t>(m_nSampleSize * 2));
#endif

		AUDIO_PROCESSING_PRINTF("input pin index is %d, copy size is %d, sample size is %d, rest size is %d, input sample rate is %d, output sample rate is %d", m_nIndex, m_nSampleSize * 2, sample_size, m_nOutBufferWritePtr*2, m_nSampleRate_in, m_nSampleRate_out);
		m_pInputPin->Receive(pOutSample);
		pOutSample->Release();

		AUDIO_PROCESSING_PRINTF("input pin index is %d, finish transport data", m_nIndex);
	}

	return true;
}

HRESULT CTransformExtandOutputPin::InitializeOutputSample(IMediaSample **ppOutSample, TransportId_e a_eTPId)
{

	IMediaSample *pOutSample = NULL;
	AM_SAMPLE2_PROPERTIES* pProps = NULL;
	bool rt = m_pTransformFilter->GetSampleProps(&pProps, a_eTPId);	// check filter state in this call

	if(true != rt)
	{
		return S_FALSE;
	}

	if(NULL == pProps)
	{
		// the state of filter doesn't allow continuing to play
		return S_OK;
	}

	HRESULT hr = NOERROR;
	ASSERT(m_pAllocator != NULL);
	hr = m_pAllocator->GetBuffer(&pOutSample
		, pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID ? &pProps->tStart : NULL
		, pProps->dwSampleFlags & AM_SAMPLE_STOPVALID ? &pProps->tStop : NULL
		, AM_GBF_NOTASYNCPOINT);
	
	*ppOutSample = pOutSample;
	if (FAILED(hr)) 
	{
		return hr;
	}

	ASSERT(pOutSample);
	IMediaSample2 *pOutSample2;
	if (SUCCEEDED(pOutSample->QueryInterface(IID_IMediaSample2, (void **)&pOutSample2)))
	{
		/*  Modify it */
		AM_SAMPLE2_PROPERTIES OutProps;
		EXECUTE_ASSERT(SUCCEEDED(pOutSample2->GetProperties(FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, tStart), (PBYTE)&OutProps)));

		// Added by Keil, 2015/3/24, copy sample properties from input pin to output pin
		memcpy(&OutProps, pProps, sizeof(AM_SAMPLE2_PROPERTIES));
		OutProps.cbBuffer = m_nSampleSize*sizeof(CAUDIO_S16_t) * 2;
		OutProps.lActual = m_nSampleSize*sizeof(CAUDIO_S16_t) * 2;

		hr = pOutSample2->SetProperties(FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId),(PBYTE)&OutProps);
		pOutSample2->Release();
	} 
	else 
	{
		return E_FAIL;
	}
	return S_OK;
}


// pass the buffer size decision through to the main transform class
HRESULT CTransformExtandOutputPin::DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES* pProp)
{
	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr;

	DbgLog((LOG_MEMORY, 1, TEXT("Setting Allocator Requirements")));
	DbgLog((LOG_MEMORY, 1, TEXT("Count %d, Size %d"),
		m_pProp.cBuffers, m_pProp.cbBuffer));

	// Pass the allocator requirements to our output side
	// but do a little sanity checking first or we'll just hit
	// asserts in the allocator.



	m_nSampleSize = Ceil(static_cast<AUDIO_DATA_TYPE>(m_pProp.cbBuffer)/(sizeof(CAUDIO_S16_t)*2)/m_nSampleRate_in*m_nSampleRate_out);
	pProp->cbBuffer = m_nSampleSize*sizeof(CAUDIO_S16_t)*2;
	pProp->cBuffers = m_pProp.cBuffers;
	pProp->cbAlign = m_pProp.cbAlign;
	if (pProp->cBuffers <= 0) { pProp->cBuffers = 1; }
	if (pProp->cbBuffer <= 0) { pProp->cbBuffer = 1; }
	hr = pAllocator->SetProperties(pProp, &Actual);

	if (FAILED(hr)) {
		return hr;
	}

	DbgLog((LOG_MEMORY, 1, TEXT("Obtained Allocator Requirements")));
	DbgLog((LOG_MEMORY, 1, TEXT("Count %d, Size %d, Alignment %d"),
		Actual.cBuffers, Actual.cbBuffer, Actual.cbAlign));

	// Make sure we got the right alignment and at least the minimum required

	if ((m_pProp.cBuffers > Actual.cBuffers)
		|| (m_pProp.cbBuffer > Actual.cbBuffer)
		|| (m_pProp.cbAlign > Actual.cbAlign)
		) {
		return E_FAIL;
	}

	if (NULL != m_pOutBuffer)
	{
		delete[] m_pOutBuffer;
		m_pOutBuffer = NULL;
	}
	m_nOutBufferSize = m_nSampleRate_out * 2; //1s
	m_pOutBuffer = new short[m_nOutBufferSize];
	m_nOutBufferWritePtr = m_nSampleSize/2;
	memset(m_pOutBuffer, 0, sizeof(short)*m_nOutBufferSize);

	return NOERROR;
}

HRESULT CTransformExtandOutputPin::SetAllocaterProp(ALLOCATOR_PROPERTIES *pProp)
{
	if (NULL != pProp)
	{
		m_pProp.cbAlign = pProp->cbAlign;
		m_pProp.cbBuffer = pProp->cbBuffer;
		m_pProp.cbPrefix = pProp->cbPrefix;
		m_pProp.cBuffers = pProp->cBuffers;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

// return a specific media type indexed by iPosition
HRESULT CTransformExtandOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	//  We don't have any media types if our input is not connected
	if (m_pTransformFilter->_ConnectStatusOfInputPin(m_nIndex))
	{
		return m_pTransformFilter->GetMediaType(iPosition, pMediaType, m_nIndex);
	}
	else
	{
		return VFW_S_NO_MORE_ITEMS;
	}
}

// Override this if you can do something constructive to act on the quality message.
// Consider passing it upstream as well Pass the quality message on upstream.
STDMETHODIMP CTransformExtandOutputPin::Notify(IBaseFilter* pSender, Quality q)
{
    UNREFERENCED_PARAMETER(pSender);
    ValidateReadPtr(pSender,sizeof(IBaseFilter));

    // First see if we want to handle this ourselves
    HRESULT hr = m_pTransformFilter->AlterQuality(q);
    if (hr!=S_FALSE)
	{
        return hr;        // either S_OK or a failure
    }

#if FILTER_MODE_1_IN_2_OUT
	CTransformExtandInputPin *pInputPin = m_pTransformFilter->m_onlineClassInputPinList.GetHead();
#else
	CTransformExtandInputPin *pInputPin = m_pTransformFilter->m_MicInputPinList.GetHead();
#endif

	if (FAILED(pInputPin->PassNotify(q)))
	{
		return E_FAIL;
	}
	//merged from v0.5 end

	return NOERROR;
} // Notify

// the following removes a very large number of level 4 warnings from the microsoft
// compiler output, which are not useful at all in this case.
#pragma warning(disable:4514)
