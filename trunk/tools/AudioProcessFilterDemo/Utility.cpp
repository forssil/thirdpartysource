#include "stdafx.h"
#include "Utility.h"
#include "mmsystem.h"
#include "wxdebug.h"
#include "wxutil.h"
#include "Mtype.h"


#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif
#define SAFE_RELEASE(X)                     {if (X) { X->Release(); X = NULL; }}

CUtility::CUtility()
{
}

CUtility::~CUtility()
{

}

HRESULT	CUtility::ConnectFilters(IGraphBuilder *pGraph, IPin *pOut, IBaseFilter *pDest)
{
	if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}

#ifdef debug
	PIN_DIRECTION PinDir;
	pOut->QueryDirection(&PinDir);
	_ASSERTE(PinDir == PINDIR_OUTPUT);
#endif

	IPin *pIn = 0;
	HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pGraph->Connect(pOut, pIn);
	pIn->Release();
	return hr;
}

HRESULT	CUtility::ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest)
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}

	IPin *pOut = 0;
	HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = ConnectFilters(pGraph, pOut, pDest);
	pOut->Release();
	return hr;
}

HRESULT	CUtility::ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest, CONNECT_TYPE eConnectType)
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}

	HRESULT hr = NOERROR;
	IPin *pIn  = NULL;
	IPin *pOut = NULL;
	switch (eConnectType)
	{
		case CON_MIC_IN:
		{
			hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pDest->FindPin(L"Mic0In", &pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pGraph->Connect(pOut, pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			break;
		}
		case CON_CLASSTOCLASS_IN:
		{
			hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pDest->FindPin(L"ClassToClass0In", &pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pGraph->Connect(pOut, pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			break;
		}

		case CON_ONLINECLASS_IN:
		{
			hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pDest->FindPin(L"OnlineClassIn", &pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pGraph->Connect(pOut, pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			break;
		}
		case CON_ELECTRONICPIANO_IN:
		{
			hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pDest->FindPin(L"ElectronicPianoIn", &pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = pGraph->Connect(pOut, pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			break;
		}
		case CON_SPEAKER_OUT:
		{
			hr = pSrc->FindPin(L"SpeakerOut", &pOut);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
			if (FAILED(hr))
			{
				return hr;
			}

			hr = pGraph->Connect(pOut, pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			break;
		}
		case CON_ONLINECLASS_OUT:
		{
			hr = pSrc->FindPin(L"OnlineClassOut", &pOut);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			
			hr = pGraph->Connect(pOut, pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			break;
		}
		case CON_RECORDERER_OUT:
		{
			hr = pSrc->FindPin(L"RecorderOut", &pOut);
			if (FAILED(hr))
			{
				return hr;
			}
			hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
			if (FAILED(hr))
			{
				return hr;
			}

			hr = pGraph->Connect(pOut, pIn);
			if (FAILED(hr))
			{
				return hr;
			}
			break;
		}
		default:
		{
			break;
		}
			
	}

	pIn->Release();
	pOut->Release();
	return hr;
}

void CUtility::NukeDownstream(IGraphBuilder* pGraph, IBaseFilter* pFilter, bool bRemove)
{
	HRESULT hr = E_FAIL;

	if (pGraph && pFilter)
	{
		IEnumPins * pinEnum = 0;
		if (SUCCEEDED(pFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();
			IPin * pin = 0;
			ULONG cFetched = 0;
			bool pass = true;
			while (pass && SUCCEEDED(pinEnum->Next(1, &pin, &cFetched)))
			{
				if (pin && cFetched)
				{
					IPin * connectedPin = 0;
					pin->ConnectedTo(&connectedPin);
					if(connectedPin) 
					{
						PIN_INFO pininfo;
						if (SUCCEEDED(connectedPin->QueryPinInfo(&pininfo)))
						{
							if(pininfo.dir == PINDIR_INPUT) 
							{
								NukeDownstream(pGraph, pininfo.pFilter, bRemove);
								hr=pGraph->Disconnect(connectedPin);
								hr=pGraph->Disconnect(pin);
								hr=pin->Disconnect();

								if(bRemove)
								{
									pGraph->RemoveFilter(pininfo.pFilter);
								}
							}

							pininfo.pFilter->Release();
						}
						connectedPin->Release();
					}
					pin->Release();
				}
				else
				{
					pass = false;
				}
			}
			pinEnum->Release();
		}
	}
}

HRESULT CUtility::GetUnconnectedPin(IBaseFilter * pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
{
	*ppPin = 0;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection(&ThisPinDir);

// 		PIN_INFO info;
// 		hr = pPin->QueryPinInfo(&info);
// 
// 		if(info.pFilter)
// 		{
// 			FILTER_INFO filter_info;
// 			hr = info.pFilter->QueryFilterInfo(&filter_info);
// 
// 			if(filter_info.pGraph)
// 			{
// 				filter_info.pGraph->Release();
// 			}
// 		}

		if (ThisPinDir == PinDir)
		{			
			IPin *pTmp = 0;
			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))  // Already connected, not the pin we want.
			{
				pTmp->Release();
			}
			else  // Unconnected, this is the pin we want.
			{
				pEnum->Release();
				*ppPin = pPin;
				return S_OK;
			}
		}

// 		if(info.pFilter)
// 		{
// 			info.pFilter->Release();
// 		}

		pPin->Release();
	}

	pEnum->Release();
	// Did not find a matching pin.

	return E_FAIL;
}

BOOL CUtility::IsConnected(IBaseFilter* pSourceFilter, IBaseFilter* pDestFilter)
{
	if( (pSourceFilter == NULL) || (pDestFilter == NULL) )
	{
		return FALSE;
	}

	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = E_FAIL;
	if( FAILED( pSourceFilter->EnumPins(&pEnum) ) )
	{
		return FALSE;
	}

	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PINDIR_OUTPUT)
		{
			IPin *pTmp = 0;
			if( SUCCEEDED( hr=pPin->ConnectedTo(&pTmp) ) ) // Already connected, not the pin we want.
			{
				PIN_INFO pinInfo;
				if( SUCCEEDED( pTmp->QueryPinInfo(&pinInfo) ) )
				{
					if( IsEqualObject(pinInfo.pFilter, pDestFilter) )
					{
						SAFE_RELEASE(pinInfo.pFilter);
						SAFE_RELEASE(pTmp);
						SAFE_RELEASE(pPin);
						SAFE_RELEASE(pEnum);

						return TRUE;
					}

					SAFE_RELEASE(pinInfo.pFilter);
				}
			}
			SAFE_RELEASE(pTmp);
		}
		SAFE_RELEASE(pPin);
	}

	SAFE_RELEASE(pEnum);

	return FALSE;
}

HRESULT CUtility::AddFilterByCLSID(IGraphBuilder *pGraph, const GUID& clsid, 
											LPCWSTR wszName, IBaseFilter **ppF)
{
	if (!pGraph || ! ppF) return E_POINTER;

	*ppF = 0;
	IBaseFilter *pF = 0;

	HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, reinterpret_cast<void**>(&pF));

	if (SUCCEEDED(hr))
	{
		hr = pGraph->AddFilter(pF, wszName);
		if (SUCCEEDED(hr))
			*ppF = pF;
		else
			pF->Release();
	}

	return hr;
}

int CUtility::GetCurPath(LPSTR path)
{
	if(path == NULL)
	{
		return -1;
	}

	HMODULE hmod;

#if _MSC_VER < 1300    // earlier than .NET compiler (VC 6.0)

	// Here's a trick that will get you the handle of the module
	// you're running in without any a-priori knowledge:
	// http://www.dotnet247.com/247reference/msgs/13/65259.aspx

	MEMORY_BASIC_INFORMATION mbi;
	static int dummy;
	VirtualQuery( &dummy, &mbi, sizeof(mbi) );

	hmod =  reinterpret_cast<HMODULE>(mbi.AllocationBase);

#else    // VC 7.0
	hmod = reinterpret_cast<HMODULE>(&__ImageBase);
#endif
	CHAR szFull[_MAX_PATH] = {0};  
	CHAR szDrive[_MAX_DRIVE] = {0};
	CHAR szDir[_MAX_DIR] = {0};
	DWORD dwLen ;

	dwLen = ::GetModuleFileNameA(hmod, szFull, sizeof(szFull)/sizeof(TCHAR)); 
	if (dwLen <= 0)
		return -2;

	_splitpath(szFull, szDrive, szDir, NULL, NULL); 
	strcpy(path, szDrive);  
	strcat(path, szDir); 

	return 0;
}

// TODO : add sample rate parameter
HRESULT CUtility::_set_audio_buffer(IBaseFilter *pFilter, ULONG ulMillisecond, int nSampleRate)
{
	IPin * pCapturePin = NULL;

	pFilter->FindPin(L"Capture", &pCapturePin);

	if (pCapturePin)
	{
		DWORD  dwBytesPerSec = 0;
		WORD   nBlockAlign = 0;
		DWORD  dwLatencyInMilliseconds = 40;
		// Query the current media type used by the capture output pin
		AM_MEDIA_TYPE * pmt = {0};
		IAMStreamConfig * pCfg = NULL;
		HRESULT hr = pCapturePin->QueryInterface(IID_IAMStreamConfig, 
			(void **)&pCfg);
		if (SUCCEEDED(hr))
		{
			hr = pCfg->GetFormat(&pmt);
			if (SUCCEEDED(hr))
			{
				// Fill in values for the new format
				WAVEFORMATEX *pWF = (WAVEFORMATEX *) pmt->pbFormat;
				dwBytesPerSec     = pWF->nAvgBytesPerSec;
				nBlockAlign = pWF->nBlockAlign;
				DeleteMediaType(pmt);
				pmt = NULL; // Added for compatibility
			}
			SAFE_RELEASE(pCfg);
		}

		if (dwBytesPerSec)
		{
			IAMBufferNegotiation * pNeg = NULL;
			hr = pCapturePin->QueryInterface(IID_IAMBufferNegotiation, 
				(void **)&pNeg);
			if (SUCCEEDED(hr))
			{
				ALLOCATOR_PROPERTIES AllocProp;
				AllocProp.cbAlign  = -1;  // -1 means no preference.
				//AllocProp.cbBuffer = dwBytesPerSec *  ulMillisecond / 1000;
				//TODO : set buffer size          
				AllocProp.cbBuffer = nBlockAlign * nSampleRate *  ulMillisecond / 1000;
				AllocProp.cbPrefix = -1;
				AllocProp.cBuffers = -1;
				return pNeg->SuggestAllocatorProperties(&AllocProp);
				SAFE_RELEASE(pNeg);
			}
		}
	}

	return E_FAIL;
}
