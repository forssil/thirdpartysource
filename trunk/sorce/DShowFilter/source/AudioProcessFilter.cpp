/*! \file   AudioProcessFilter.h
*   \author Keil
*   \brief  The filter is used here as a data transceiver. It can receive audio stream from input pins,
			such as ClassToClass¡¢Mic, and then transmit them to internal
			audio process engine, which really performs audio process tasks. At last, the filter will
			send processed audio steams to different output pins.
*   \history   2014/12/3 created
*/

#pragma warning(disable: 4511 4512 4514) //disable warning 4511 4512 4514
#if (1100 > _MSC_VER)
	#include <olectlid.h>
#else
	#include <olectl.h>
#endif

#include <streams.h>               
#include <measure.h>               // performance measurement (MSR_)
#include <initguid.h>              
#include <process.h>               
#include "WaveIO.h"                
#include "AudioProcessProp.h"      
#include "AudioProcessFilter.h"    
#include "IAudioEngine.h"       
#include "audiotrace.h"  

//define globally unique ID  {633FC39C-A74F-4DA9-B7A9-3EB68614E8A8}
DEFINE_GUID(CLSID_AudioProcessFilter,
	0x633fc39c, 0xa74f, 0x4da9, 0xb7, 0xa9, 0x3e, 0xb6, 0x86, 0x14, 0xe8, 0xa8);
// {587840C5-7055-4CB4-AC56-E8768E81BF5F}
DEFINE_GUID(CLSID_AudioProcessProp,
	0x587840c5, 0x7055, 0x4cb4, 0xac, 0x56, 0xe8, 0x76, 0x8e, 0x81, 0xbf, 0x5f);

// as output data len may not be the same as input data len, we need a
// longer buffer to buffer out put data to avoid over flowing
// 32KHZ 1s
#define MAX_OUT_BUFFER_SIZE 32000    

/* ---------------------------------------------------------------- *
Information of Filters and Pins
* ---------------------------------------------------------------- */
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_Audio,  // Major type
	&MEDIASUBTYPE_PCM, // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
#if FILTER_MODE_1_IN_2_OUT
	{
		L"OnlineClass0In",			// String pin name
		FALSE,              // Is it rendered
		FALSE,              // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Output",          // Connects to pin
		1,                  // Number of types
		&sudPinTypes		// The pin details
	},
	{
		L"OnlineClassOut",			// String pin name
		FALSE,              // Is it rendered
		TRUE,               // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Input",           // Connects to pin
		1,                  // Number of types
		&sudPinTypes        // The pin details
	},
	{
		L"RecorderOut",			// String pin name
		FALSE,              // Is it rendered
		TRUE,               // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Input",           // Connects to pin
		1,                  // Number of types
		&sudPinTypes        // The pin details
	}
#else
	{
		L"ClassToClass_in",	// String pin name
		FALSE,              // Is it rendered
		FALSE,              // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Output",          // Connects to pin
		1,                  // Number of types
		&sudPinTypes		// The pin details
	},
	//merged from v0.5
	{
		L"OnlineClass_in",			// String pin name
		FALSE,              // Is it rendered
		FALSE,              // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Output",          // Connects to pin
		1,                  // Number of types
		&sudPinTypes		// The pin details
	},
	{
		L"ElectronicPiano_in",			// String pin name
		FALSE,              // Is it rendered
		FALSE,              // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Output",          // Connects to pin
		1,                  // Number of types
		&sudPinTypes		// The pin details
	},
	{
		L"Mic_in0",			// String pin name
		FALSE,              // Is it rendered
		FALSE,              // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Output",          // Connects to pin
		1,                  // Number of types
		&sudPinTypes		// The pin details
	},
	{
		L"OnlineClass_out",			// String pin name
		FALSE,              // Is it rendered
		TRUE,               // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Input",           // Connects to pin
		1,                  // Number of types
		&sudPinTypes        // The pin details
	},
	//merged from v0.5
	{
		L"Speaker_out",			// String pin name
		FALSE,              // Is it rendered
		TRUE,               // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Input",           // Connects to pin
		1,                  // Number of types
		&sudPinTypes        // The pin details
	},
	{
		L"Recorder_out",			// String pin name
			FALSE,              // Is it rendered
			TRUE,               // Is it an output
			FALSE,              // Allowed none
			FALSE,              // Allowed many
			&CLSID_NULL,        // Connects to filter
			L"Input",           // Connects to pin
			1,                  // Number of types
			&sudPinTypes        // The pin details
	}
#endif
};

const AMOVIESETUP_FILTER sudFilter =
{
#if FILTER_MODE_1_IN_2_OUT
	&CLSID_AudioProcessFilter,       // Filter CLSID
	L"Codyy Audio Process Filter",   // Filter name
	MERIT_DO_NOT_USE,            // Its merit
	3,							 // Number of pins
	psudPins					 // Pin details
#else
	&CLSID_AudioProcessFilter,       // Filter CLSID
	L"Codyy Audio Process Filter",   // Filter name
	MERIT_DO_NOT_USE,            // Its merit
	7,							 // Number of pins
	psudPins					 // Pin details
#endif
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance
CFactoryTemplate g_Templates[] =
{
	{
			L"Codyy Audio Process Filter",
			&CLSID_AudioProcessFilter,
			CAudioProcessFilter::CreateInstance,
			NULL,
			&sudFilter
	},
	{
			L"Codyy Audio Process Property Page",
			&CLSID_AudioProcessProp,
			CAudioProcessProp::CreateInstance
	}
};

CAUDIO_S32_t g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

CAudioProcessFilter::CAudioProcessFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
	: CTransformExtandFilter(tszName, punk, CLSID_AudioProcessFilter)
	, m_nChannelNum(2)
	, m_nInputPinFactNum(0)
	, m_bAecSwitch(true)
	, m_pFolderName("")
	, m_pAudioEngine(NULL)
	, m_pPropertyPage(NULL)
	, m_pInBuffer_FeedData(NULL)
	, m_nInBufferSize_FeedData(0)
	//merged from v0.5
	, m_pEOSFlag(NULL)
{
	HRESULT hr = NOERROR;
	hr = __InitAudioProcessFilter();
	ASSERT(SUCCEEDED(hr));
}

CAudioProcessFilter::~CAudioProcessFilter(void)
{
	HRESULT hr = NOERROR;
	hr = __ReleaseAudioProcessFilter();
	ASSERT(SUCCEEDED(hr));
}

/*static */CUnknown * WINAPI CAudioProcessFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
	CAudioProcessFilter *pNewObject = new CAudioProcessFilter(NAME("Codyy Audio Process Filter"), punk, phr);
	if (pNewObject == NULL)
	{
		*phr = E_OUTOFMEMORY;
	}
	return pNewObject;
}

/*virtual */CAUDIO_S32_t CAudioProcessFilter::GetPinCount()
{
#if FILTER_MODE_1_IN_2_OUT
	return m_nOnlineClassInputPinCount /* on-line class input pin*/
			+1 /*on-line class output pin*/
			+1;/*recorder output pin*/
#else
	return m_nMicInputPinCount /*mike input pin*/
			+1 /*class-to-class input pin*/
			+1 /*on-line class input pin*/
			+1 /*electronic piano input pin*/
			+1 /*on-line class output pin*/
			+1 /*speaker output pin*/
			+1;/*recorder output pin*/
#endif
}

CBasePin* CAudioProcessFilter::GetPin(int n)
{
#if FILTER_MODE_1_IN_2_OUT
	if (n < 0)
	{
		return NULL;
	}
	if (n == m_nOnlineClassInputPinCount)
	{
		return m_pOnlineClassOutputPin;
	}
	if (n == m_nOnlineClassInputPinCount + 1)
	{
		return m_pRecordOutputPin;
	}
	return _GetOnlineClassInputPinFromList(n+1);
#else
	if (n < 0)
	{
		return NULL;
	}
	// index of class-to-class input pin is 0
	if (n == 0)
	{
		return m_BBTInputPin;
	}
	// index of on-line class input pin is 1
	if (n == 1)
	{
		return m_ZXKTInputPin;
	}
	// index of electronic piano input pin is 2
	if (n == 2)
	{
		return m_DZGQInputPin;
	}
	// index of on-line class output pin is m_nMicInputPinCount + 3
	if (n == m_nMicInputPinCount + 3)
	{
		return m_ZXKTOutputPin;
	}
	// index of on-line class output pin is m_nMicInputPinCount + 4
	if (n == m_nMicInputPinCount + 4)
	{
		return m_YYYXOutputPin;
	}
	// index of on-line class output pin is m_nMicInputPinCount + 5
	if (n == m_nMicInputPinCount + 5)
	{
		return m_LZOutputPin;
	}
	// return mike input pin
	return _GetMicInputPinFromList(n-2);
#endif
}

HRESULT CAudioProcessFilter::Receive(CAUDIO_S32_t nIndex, IMediaSample *pSample)
{
	if (m_State == State_Stopped)
	{
		return S_FALSE;
	}

	ASSERT(NULL != pSample);
	BYTE* pBufIn = NULL;
	HRESULT reslt = pSample->GetPointer(&pBufIn);
	ASSERT(SUCCEEDED(reslt));

	const CAUDIO_S16_t* psIn = (CAUDIO_S16_t*)pBufIn;
	CAUDIO_U32_t sampleSize = pSample->GetActualDataLength() / sizeof(CAUDIO_S16_t);

	// set wave name by pin index
#if FILTER_MODE_1_IN_2_OUT
	std::string wave_name;
	char onlineclass_name[256];
	sprintf_s(onlineclass_name, 256, "OnlineClass%dIn", nIndex);
	wave_name = onlineclass_name;
#else
	std::string wave_name;
	if (-3 == nIndex)
	{
		wave_name = "ClassToClassIn";
	}
	else if (-2 == nIndex)
	{
		wave_name = "OnlineClassIn";
	}
	else if (-1 == nIndex)
	{
		wave_name = "ElectronicPianoIn";
	}
	else
	{
		char mic_name[256];
		sprintf_s(mic_name, 256, "Mic%dIn", nIndex);
		wave_name = mic_name;
	}
#endif

#ifdef _DEBUG
	__WriteWavFile(const_cast<CAUDIO_S16_t*>(psIn), static_cast<CAUDIO_S32_t>(sampleSize), wave_name);
#endif

	AUDIO_PROCESSING_PRINTF("input pin index is %d, sample size is %d", nIndex, sampleSize);

	// achieve sample rate 
	CAUDIO_U32_t original_sample_rate = 0;
	reslt = __GetOriginalSampleRate(wave_name, original_sample_rate);
	if (original_sample_rate <= 0)
	{
		return S_FALSE;
	}

	__Fix2Float(psIn, sampleSize);

	MapReceivedSampleInfo::iterator iter = m_mReceivedSampleInfo.find(nIndex);
	m_pAudioEngine->FeedData(iter->second.channel_id_, m_pInBuffer_FeedData, static_cast<CAUDIO_U32_t>(m_nInBufferSize_FeedData), iter->second.thread_id_, original_sample_rate);

	return reslt;
}

/*virtual */bool CAudioProcessFilter::RegisterProperty(AUDIO_PROPERTY_PAGE *property_page)
{
	if (NULL != property_page)
	{
		m_pPropertyPage = property_page;
		return true;
	}
	else
	{
		return false;
	}
}

/*virtual */bool CAudioProcessFilter::UnRegisterProperty()
{
	if (NULL != m_pPropertyPage)
	{
		m_pPropertyPage = NULL;
		return true;
	}
	else
	{
		return false;
	}
}


/*virtual */HRESULT CAudioProcessFilter::CheckInputType(const CMediaType* mtIn, CAUDIO_S32_t nIndex) //PURE
{
	//check whether the media type supported dynamic change
	if (!IsStopped())
	{
		AUDIO_PROCESSING_PRINTF("m_State == State_Stopped");
		return E_FAIL;
	}
	//check whether the media type is the supported audio formats or not
	if (mtIn->majortype != MEDIATYPE_Audio
		||mtIn->subtype != MEDIASUBTYPE_PCM
		||mtIn->formattype != FORMAT_WaveFormatEx)
	{
		AUDIO_PROCESSING_PRINTF("media type does not match");
		return E_FAIL;
	}
    //todo check input type for multi input
	//
	WAVEFORMATEX *wfx = (WAVEFORMATEX*)mtIn->pbFormat;
	if (!SAMPLE_RATE_VALID_CHECK(wfx->nSamplesPerSec))
	{
		return E_FAIL;
	}
	if (wfx->wBitsPerSample != 16)
	{
		AUDIO_PROCESSING_PRINTF("wfx->wBitsPerSample != 16");
		return E_FAIL;
	}

	if (wfx->nChannels != 2)
	{
		AUDIO_PROCESSING_PRINTF("wfx->nChannels != 2");
		return E_FAIL;
	}

	return NOERROR;
}

//! check if you can support the transform from this input to this output
/*virtual */HRESULT CAudioProcessFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut, CAUDIO_S32_t nIndex) //PURE
{
	HRESULT hr = NOERROR;
	if (FAILED(CheckInputType(mtIn, nIndex)))
	{
		AUDIO_PROCESSING_PRINTF("CheckInputType failed");
		hr = E_FAIL;
	}
		
	if (mtOut->majortype != mtIn->majortype
		||mtOut->subtype != mtIn->subtype
		||mtOut->formattype != mtIn->formattype)
	{
		AUDIO_PROCESSING_PRINTF("media type does not match");
		hr = E_FAIL;
	}
	return hr;
}

//! override to suggest OUTPUT pin media types
/*virtual */HRESULT CAudioProcessFilter::GetMediaType(CAUDIO_S32_t iPosition, CMediaType *pmt, CAUDIO_S32_t nIndex) //PURE
{
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}
	if (iPosition > 0)
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	ASSERT(pmt);
	pmt->InitMediaType();
	pmt->SetType(&MEDIATYPE_Audio);
	pmt->SetSubtype(&MEDIASUBTYPE_PCM);
	pmt->SetFormatType(&FORMAT_WaveFormatEx);
	pmt->SetTemporalCompression(FALSE);
	pmt->pUnk = NULL;

	WAVEFORMATEX *wfx = (WAVEFORMATEX *)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
	ZeroMemory(wfx, sizeof(WAVEFORMATEX));

	wfx->wFormatTag = WAVE_FORMAT_PCM;
	wfx->cbSize = 0;
	wfx->nChannels = 2;
	wfx->nBlockAlign = 2 * (16 / 8);
	wfx->wBitsPerSample = 16;

#if FILTER_MODE_1_IN_2_OUT
	switch (nIndex)
	{
		// OnlineClass_out
		case 0:
		{
			wfx->nSamplesPerSec = m_sSampleRate.nSampleRate_OnlineClassOut;
			m_pOnlineClassOutputPin->m_nSampleRate_out = m_sSampleRate.nSampleRate_OnlineClassOut;

			if (m_nOnlineClassInputPinCount > 1)
			{
				POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
				CTransformExtandInputPin *pInputPin = NULL;
				while (pos)
				{
					pInputPin = m_onlineClassInputPinList.GetNext(pos);
					if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
					{
						CMediaType pmt = pInputPin->CurrentMediaType();
						if (pmt.formattype != FORMAT_WaveFormatEx)
						{
							return E_FAIL;
						}
						WAVEFORMATEX *wfx = (WAVEFORMATEX*)pmt.pbFormat;
						CAUDIO_U32_t sample_rate = wfx->nSamplesPerSec;
						if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
						{
							return E_FAIL;
						}
						m_pOnlineClassOutputPin->m_nSampleRate_in = sample_rate;
						break;
					}
				}
			}
			break;
		}
		// Record_out
		case 1:
		{
			wfx->nSamplesPerSec = m_sSampleRate.nSampleRate_RecorderOut;
			m_pRecordOutputPin->m_nSampleRate_out = m_sSampleRate.nSampleRate_RecorderOut;

			if (m_nOnlineClassInputPinCount > 1)
			{
				POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
				CTransformExtandInputPin *pInputPin = NULL;
				while (pos)
				{
					pInputPin = m_onlineClassInputPinList.GetNext(pos);
					if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
					{
						CMediaType pmt = pInputPin->CurrentMediaType();
						if (pmt.formattype != FORMAT_WaveFormatEx)
						{
							return E_FAIL;
						}
						WAVEFORMATEX *wfx = (WAVEFORMATEX*)pmt.pbFormat;
						CAUDIO_U32_t sample_rate = wfx->nSamplesPerSec;
						if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
						{
							return E_FAIL;
						}
						m_pRecordOutputPin->m_nSampleRate_in = sample_rate;
						break;
					}
				}
			}
			break;
		}
		default:
			return E_FAIL;
	}
#else
	switch (nIndex)
	{
		// OnlineClass_out
	case 0:
	{
		wfx->nSamplesPerSec = m_sSampleRate.nSampleRate_OnlineClassOut;
		m_ZXKTOutputPin->m_nSampleRate_out = m_sSampleRate.nSampleRate_OnlineClassOut;

#if CAPTURE_PLAYBACK_INTEGRATED 
		if (NULL != m_ZXKTInputPin && TRUE == m_ZXKTInputPin->IsConnected())
		{
			CMediaType pmt = m_ZXKTInputPin->CurrentMediaType();
			if (pmt.formattype != FORMAT_WaveFormatEx)
			{
				return E_FAIL;
			}
			WAVEFORMATEX *wfx = (WAVEFORMATEX*)pmt.pbFormat;
			CAUDIO_U32_t sample_rate = wfx->nSamplesPerSec;
			if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
			{
				return E_FAIL;
			}
			m_ZXKTOutputPin->m_nSampleRate_in = sample_rate;

		}
#else
		if (m_nMicInputPinCount > 1)
		{
			POSITION pos = m_MicInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;
			while (pos)
			{
				pInputPin = m_MicInputPinList.GetNext(pos);
				if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
				{
					CMediaType pmt = pInputPin->CurrentMediaType();
					if (pmt.formattype != FORMAT_WaveFormatEx)
					{
						return E_FAIL;
					}
					WAVEFORMATEX *wfx = (WAVEFORMATEX*)pmt.pbFormat;
					CAUDIO_U32_t sample_rate = wfx->nSamplesPerSec;
					if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
					{
						return E_FAIL;
					}
					m_ZXKTOutputPin->m_nSampleRate_in = sample_rate;
					break;
				}
			}
		}
#endif

		break;
	}
	// Speaker_out
	case 1:
	{
		wfx->nSamplesPerSec = m_sSampleRate.nSampleRate_SpeakerOut;
		m_YYYXOutputPin->m_nSampleRate_out = m_sSampleRate.nSampleRate_SpeakerOut;

		if (NULL != m_ZXKTInputPin && TRUE == m_ZXKTInputPin->IsConnected())
		{
			CMediaType pmt = m_ZXKTInputPin->CurrentMediaType();
			if (pmt.formattype != FORMAT_WaveFormatEx)
			{
				return E_FAIL;
			}
			WAVEFORMATEX *wfx = (WAVEFORMATEX*)pmt.pbFormat;
			CAUDIO_U32_t sample_rate = wfx->nSamplesPerSec;
			if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
			{
				return E_FAIL;
			}
			m_YYYXOutputPin->m_nSampleRate_in = sample_rate;
			break;
		}
		break;
	}
	// Recorder_out
	case 2:
	{
		wfx->nSamplesPerSec = m_sSampleRate.nSampleRate_RecorderOut;
		m_LZOutputPin->m_nSampleRate_out = m_sSampleRate.nSampleRate_RecorderOut;

		if (m_nMicInputPinCount > 1)
		{
			POSITION pos = m_MicInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;
			while (pos)
			{
				pInputPin = m_MicInputPinList.GetNext(pos);
				if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
				{
					CMediaType pmt = pInputPin->CurrentMediaType();
					if (pmt.formattype != FORMAT_WaveFormatEx)
					{
						return E_FAIL;
					}
					WAVEFORMATEX *wfx = (WAVEFORMATEX*)pmt.pbFormat;
					CAUDIO_U32_t sample_rate = wfx->nSamplesPerSec;
					if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
					{
						return E_FAIL;
					}
					m_LZOutputPin->m_nSampleRate_in = sample_rate;
					break;
				}
			}
		}
		break;
	}
	default:
		return E_FAIL;
	}
#endif

	wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;

	return NOERROR;
}

//! complete connect
HRESULT CAudioProcessFilter::CompleteConnect(PIN_DIRECTION direction, CAUDIO_S32_t nIndex, IPin *pReceivePin)
{
	// add input Pin
	// NOTE: CompleteConnect will be called more than one time, so this way may be have some bugs.
	
#if FILTER_MODE_1_IN_2_OUT
	if (direction == PINDIR_INPUT)
	{
		HRESULT hr = NOERROR;
		CMediaType pmt;
		WAVEFORMATEX *wfx = NULL;
		CAUDIO_U32_t sample_rate = 0;

		POSITION pos = m_onlineClassInputPinList.GetTailPosition();
		CTransformExtandInputPin *pInputPin = m_onlineClassInputPinList.GetNext(pos);
		char name[16] = { 0 };
		sprintf_s(name, sizeof(name), "OnlineClass%dIn", m_vOnlineClassInputPinIndex_StoreUsed.back());

		// check sample rate 
		pmt = pInputPin->CurrentMediaType();
		wfx = (WAVEFORMATEX*)pmt.pbFormat;
		sample_rate = wfx->nSamplesPerSec;
		if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
		{
			AUDIO_PROCESSING_PRINTF("sample_rate is invalid");
			return E_FAIL;
		}
		MapSampleRateInfo::iterator iter = m_mInputSampleRate.find(name);
		if (m_mInputSampleRate.end() != iter)
		{
			iter->second = sample_rate;
		}
		else
		{
			m_mInputSampleRate.insert(std::make_pair(name, sample_rate));
		}
		_CreateNewOnlineClassInputPin(this);
	}
	else if (direction == PINDIR_OUTPUT)
	{
		HRESULT hr = NOERROR;
		ALLOCATOR_PROPERTIES pProp;
		POSITION pos;
		CTransformExtandInputPin *pInputPin = NULL;

		// OnlineClassOut 
		if (0 == nIndex)
		{
			if (m_nOnlineClassInputPinCount > 1)
			{
				POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
				while (pos)
				{
					pInputPin = m_onlineClassInputPinList.GetNext(pos);
					if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
					{
						if (FAILED(hr = pInputPin->PeekAllocator()->GetProperties(&pProp)))
						{
							return E_FAIL;
						}
						else
						{
							if (FAILED(hr = m_pOnlineClassOutputPin->SetAllocaterProp(&pProp)))
							{
								return E_FAIL;
							}
						}
						break;
					}
				}
			}
			m_bOnlineClassOutputPin = TRUE;
		}
		// RecordOut
		else if (1 == nIndex)
		{
			if (m_nOnlineClassInputPinCount > 1)
			{
				POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
				while (pos)
				{
					pInputPin = m_onlineClassInputPinList.GetNext(pos);
					if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
					{
						if (FAILED(hr = pInputPin->PeekAllocator()->GetProperties(&pProp)))
						{
							return E_FAIL;
						}
						else
						{
							if (FAILED(hr = m_pRecordOutputPin->SetAllocaterProp(&pProp)))
							{
								return E_FAIL;
							}
						}
						break;
					}
				}
			}
			m_bRecordOutputPin = TRUE;
		}
	}
#else
	if (direction == PINDIR_INPUT)
	{
		HRESULT hr = NOERROR;
		CMediaType pmt;
		WAVEFORMATEX *wfx = NULL;
		CAUDIO_U32_t sample_rate = 0;

		// BBT input pin
		if (-3 == nIndex)
		{
			pmt = m_BBTInputPin->CurrentMediaType();
			wfx = (WAVEFORMATEX*)pmt.pbFormat;
			sample_rate = wfx->nSamplesPerSec;
			if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
			{
				AUDIO_PROCESSING_PRINTF("sample_rate is invalid");
				return E_FAIL;
			}
			MapSampleRateInfo::iterator iter = m_mInputSampleRate.find("ClassToClassIn");
			if (m_mInputSampleRate.end() != iter)
			{
				iter->second = sample_rate;
			}
			else
			{
				m_mInputSampleRate.insert(std::make_pair("ClassToClassIn", sample_rate));
			}
			m_bBBTInputPin = TRUE;
		}
		// ZXKT input pin
		else if (-2 == nIndex)
		{
			pmt = m_ZXKTInputPin->CurrentMediaType();
			wfx = (WAVEFORMATEX*)pmt.pbFormat;
			sample_rate = wfx->nSamplesPerSec;
			if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
			{
				AUDIO_PROCESSING_PRINTF("sample_rate is invalid");
				return E_FAIL;
			}
			MapSampleRateInfo::iterator iter = m_mInputSampleRate.find("OnlineClassIn");
			if (m_mInputSampleRate.end() != iter)
			{
				iter->second = sample_rate;
			}
			else
			{
				m_mInputSampleRate.insert(std::make_pair("OnlineClassIn", sample_rate));
			}
			m_bZXKTInputPin = TRUE;		
		}
		// DZGQ input pin
		else if (-1 == nIndex)
		{
			pmt = m_DZGQInputPin->CurrentMediaType();
			wfx = (WAVEFORMATEX*)pmt.pbFormat;
			sample_rate = wfx->nSamplesPerSec;
			if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
			{
				AUDIO_PROCESSING_PRINTF("sample_rate is invalid");
				return E_FAIL;
			}
			MapSampleRateInfo::iterator iter = m_mInputSampleRate.find("ElectronicPianoIn");
			if (m_mInputSampleRate.end() != iter)
			{
				iter->second = sample_rate;
			}
			else
			{
				m_mInputSampleRate.insert(std::make_pair("ElectronicPianoIn", sample_rate));
			}
			m_bDZGQInputPin = TRUE;
		}
		// mic input pin
		else
		{
			POSITION pos = m_MicInputPinList.GetTailPosition();
			CTransformExtandInputPin *pInputPin = m_MicInputPinList.GetNext(pos);
			char name[16] = { 0 };
			sprintf_s(name, sizeof(name), "Mic%dIn", m_vMicInputPinIndex_StoreUsed.back());

			// check sample rate 
			pmt = pInputPin->CurrentMediaType();
			wfx = (WAVEFORMATEX*)pmt.pbFormat;
			sample_rate = wfx->nSamplesPerSec;
			if (!SAMPLE_RATE_VALID_CHECK(sample_rate))
			{
				AUDIO_PROCESSING_PRINTF("sample_rate is invalid");
				return E_FAIL;
			}
			MapSampleRateInfo::iterator iter = m_mInputSampleRate.find(name);
			if (m_mInputSampleRate.end() != iter)
			{
				iter->second = sample_rate;
			}
			else
			{
				m_mInputSampleRate.insert(std::make_pair(name, sample_rate));
			}
			_CreateNewMicInputPin(this);
		}
	}
	else if (direction == PINDIR_OUTPUT)
	{
		// ZXKT output pin
		if (0 == nIndex)
		{
#if CAPTURE_PLAYBACK_INTEGRATED
			if (m_bZXKTInputPin)
			{
				HRESULT hr = NOERROR;
				ALLOCATOR_PROPERTIES pProp;
				if (FAILED(hr = m_ZXKTInputPin->PeekAllocator()->GetProperties(&pProp)))
				{
					return E_FAIL;
				}
				else
				{
					if (FAILED(hr = m_ZXKTOutputPin->SetAllocaterProp(&pProp)))
					{
						return E_FAIL;
					}
				}
			}
#else
			if (m_nMicInputPinCount > 1)
			{
				HRESULT hr = NOERROR;
				ALLOCATOR_PROPERTIES pProp;
				POSITION pos = m_MicInputPinList.GetHeadPosition();
				CTransformExtandInputPin *pInputPin = NULL;
				while (pos)
				{
					pInputPin = m_MicInputPinList.GetNext(pos);
					if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
					{
						if (FAILED(hr = pInputPin->PeekAllocator()->GetProperties(&pProp)))
						{
							return E_FAIL;
						}
						else
						{
							if (FAILED(hr = m_ZXKTOutputPin->SetAllocaterProp(&pProp)))
							{
								return E_FAIL;
							}
						}
						break;
					}
				}
			}
#endif
			m_bZXKTOutputPin = TRUE;
		}
		// YYYX output pin
		else if (1 == nIndex)
		{
			if (m_bZXKTInputPin)
			{
				HRESULT hr = NOERROR;
				ALLOCATOR_PROPERTIES pProp;
				if (FAILED(hr = m_ZXKTInputPin->PeekAllocator()->GetProperties(&pProp)))
				{
					return E_FAIL;
				}
				else
				{
					if (FAILED(hr = m_YYYXOutputPin->SetAllocaterProp(&pProp)))
					{
						return E_FAIL;
					}
				}
			}
			m_bYYYZOutputPin = TRUE;
		}
		// LZ output pin
		else if (2 == nIndex)
		{
			if (m_nMicInputPinCount > 1)
			{
				HRESULT hr = NOERROR;
				ALLOCATOR_PROPERTIES pProp;
				POSITION pos = m_MicInputPinList.GetHeadPosition();
				CTransformExtandInputPin *pInputPin = NULL;
				while (pos)
				{
					pInputPin = m_MicInputPinList.GetNext(pos);
					if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
					{
						if (FAILED(hr = pInputPin->PeekAllocator()->GetProperties(&pProp)))
						{
							return E_FAIL;
						}
						else
						{
							if (FAILED(hr = m_LZOutputPin->SetAllocaterProp(&pProp)))
							{
								return E_FAIL;
							}
						}
						break;
					}
				}
			}
			m_bLZOutputPin = TRUE;
		}
	}
#endif

	return CTransformExtandFilter::CompleteConnect(direction, nIndex, pReceivePin);
}

//! break connect
HRESULT CAudioProcessFilter::BreakConnect(PIN_DIRECTION dir, CAUDIO_S32_t nIndex)
{
#if FILTER_MODE_1_IN_2_OUT
	if (dir == PINDIR_OUTPUT)
	{
		// OnlineClassOut
		if (0 == nIndex)
		{
			m_bOnlineClassOutputPin = FALSE;
		}
		// RecordOut
		else if (1 == nIndex)
		{
			m_bRecordOutputPin = FALSE;
		}
	}
#else
	if (dir == PINDIR_INPUT)
	{
		// BBT input pin
		if (-3 == nIndex)
		{
			m_bBBTInputPin  = FALSE;
		}
		// ZXKT input pin
		else if (-2 == nIndex)
		{
			m_bZXKTInputPin = FALSE;
		}
		// DZGQ input pin
		else if (-1 == nIndex)
		{
			m_bDZGQInputPin = FALSE;
		}
	}
	else if (dir == PINDIR_OUTPUT)
	{
		// ZXKT output pin
		if (0 == nIndex)
		{
			m_bZXKTOutputPin = FALSE;
		}
		// YYYX output pin
		else if (1 == nIndex)
		{
			m_bYYYZOutputPin = FALSE;
		}
		// LZ output pin
		else if (2 == nIndex)
		{
			m_bLZOutputPin = FALSE;
		}
	}
#endif

	return CTransformExtandFilter::BreakConnect(dir,nIndex);
}

//! start streaming 
/*virtual */HRESULT CAudioProcessFilter::StartStreaming()
{
	HRESULT hr = NOERROR;

	hr = __InitStreaming();
	if (FAILED(hr))
	{
		AUDIO_PROCESSING_PRINTF("__InitStreaming() failed!");
		return E_FAIL;
	}

	return hr;
}


/*virtual */HRESULT CAudioProcessFilter::StopStreaming()
{
	HRESULT hr = NOERROR;

	hr = __ReleaseStreaming();
	if (FAILED(hr))
	{
		AUDIO_PROCESSING_PRINTF("__ReleaseStreaming() failed!");
		return E_FAIL;
	}
	
	return hr;
}

//! end of stream
/*virtual */HRESULT CAudioProcessFilter::EndOfStream(CAUDIO_S32_t nIndex)
{
#if FILTER_MODE_1_IN_2_OUT
	m_pEOSFlag[nIndex] = true;

	for (CAUDIO_S32_t i=0; i<m_nInputPinFactNum-1; ++i)
	{
		if (!m_pEOSFlag[i])
		{
			return NOERROR;
		}
	}
#else
   CAUDIO_S32_t inputpin_index = nIndex;
	// BBT input pin
	if (-3 == nIndex)
	{
		inputpin_index = m_nMicInputPinCount_CalcUsed - 1;
	}
	// ZXKT input pin
	else if (-2 == nIndex)
	{
		if (m_bBBTInputPin)
		{
			inputpin_index = m_nMicInputPinCount_CalcUsed;
		}
		else
		{
			inputpin_index = m_nMicInputPinCount_CalcUsed - 1;
		}
	}
	// DZGQ input pin
	else if (-1 == nIndex)
	{
		if (m_bBBTInputPin)
		{
			if (m_bZXKTInputPin)
			{
				inputpin_index = m_nMicInputPinCount_CalcUsed + 1;
			}
			else
			{
				inputpin_index = m_nMicInputPinCount_CalcUsed;
			}
		}
		else
		{
			if (m_bZXKTInputPin)
			{
				inputpin_index = m_nMicInputPinCount_CalcUsed;
			}
			else
			{
				inputpin_index = m_nMicInputPinCount_CalcUsed - 1;
			}
		}
	}

	m_pEOSFlag[inputpin_index] = true;

	for (CAUDIO_S32_t i=0; i<m_nInputPinFactNum-1; ++i)
    {
		if (!m_pEOSFlag[i])
		{
			return NOERROR;
		}
    }

#endif
	//deliver EndOfStream to output pin
	return CTransformExtandFilter::EndOfStream(nIndex);
}

HRESULT CAudioProcessFilter::EndFlush()
{
	return CTransformExtandFilter::EndFlush();
}


HRESULT CAudioProcessFilter::__InitAudioProcessFilter(void)
{
	HRESULT hr = NOERROR;

#if FILTER_MODE_1_IN_2_OUT
	// reset OnlineClass input pins' list 
	if (_ResetOnlineClassInputPinsList())
	{
		// create a new one
		if (!_CreateNewOnlineClassInputPin(this))
		{
			AUDIO_PROCESSING_PRINTF("_CreateNewOnlineClassInputPin() failed!");
			return false;
		}
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("_ResetOnlineClassInputPinsList() failed!");
		return false;
	}

	// add on-line class output pin
	if (NULL != m_pOnlineClassOutputPin)
	{
		delete m_pOnlineClassOutputPin;
		m_pOnlineClassOutputPin = NULL;
	}
	m_pOnlineClassOutputPin = new CTransformExtandOutputPin("Output", this, &hr, L"OnlineClassOut", 0);
	m_pOnlineClassOutputPin->m_eTPId = TP2Network;
	if (NULL == m_pOnlineClassOutputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_pOnlineClassOutputPin failed!");
		return false;
	}
	IncrementPinVersion();

	// add recorder output pin
	if (NULL != m_pRecordOutputPin)
	{
		delete m_pRecordOutputPin;
		m_pRecordOutputPin = NULL;
	}
	m_pRecordOutputPin = new CTransformExtandOutputPin("Output", this, &hr, L"RecorderOut", 1);
	m_pRecordOutputPin->m_eTPId = TP2RecordingDev;
	if (NULL == m_pRecordOutputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_pRecordOutputPin failed!");
		return false;
	}
	IncrementPinVersion();
#else
	// add class-to-class input pin
	if (NULL != m_BBTInputPin)
	{
		delete m_BBTInputPin;
		m_BBTInputPin = NULL;
	}
	m_BBTInputPin = new CTransformOriginInputPin("Input", this, &hr, L"ClassToClassIn", -3);
	if (NULL == m_BBTInputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_BBTInputPin failed!");
		return false;
	}
	IncrementPinVersion();

	// add on-line class input pin
	if (NULL != m_ZXKTInputPin)
	{
		delete m_ZXKTInputPin;
		m_ZXKTInputPin = NULL;
	}
	m_ZXKTInputPin = new CTransformOriginInputPin("Input", this, &hr, L"OnlineClassIn", -2);
	if (NULL == m_ZXKTInputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_ZXKTInputPin failed!");
		return false;
	}
	IncrementPinVersion();

	// add electronic piano input pin
	if (NULL != m_DZGQInputPin)
	{
		delete m_DZGQInputPin;
		m_DZGQInputPin = NULL;
	}
	m_DZGQInputPin = new CTransformOriginInputPin("Input", this, &hr, L"ElectronicPianoIn", -1);
	if (NULL == m_DZGQInputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_DZGQInputPin failed!");
		return false;
	}
	IncrementPinVersion();

	// reset mike input pins' list 
	if (_ResetMicInputPinsList())
	{
		// create a new one
		if (!_CreateNewMicInputPin(this))
		{
			AUDIO_PROCESSING_PRINTF("_CreateNewMicInputPin() failed!");
			return false;
		}
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("_ResetMicInputPinsList() failed!");
		return false;
	}

	// add on-line class output pin
	if (NULL != m_ZXKTOutputPin)
	{
		delete m_ZXKTOutputPin;
		m_ZXKTOutputPin = NULL;
	}
	m_ZXKTOutputPin = new CTransformExtandOutputPin("Output", this, &hr, L"OnlineClassOut", 0);
	m_ZXKTOutputPin->m_eTPId=TP2Network;
	if (NULL == m_ZXKTOutputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_ZXKTOutputPin failed!");
		return false;
	}
	IncrementPinVersion();

	// add speaker output pin
	if (NULL != m_YYYXOutputPin)
	{
		delete m_YYYXOutputPin;
		m_YYYXOutputPin = NULL;
	}
	m_YYYXOutputPin = new CTransformExtandOutputPin("Output", this, &hr, L"SpeakerOut", 1);
	m_YYYXOutputPin->m_eTPId=TP2RenderDev;
	if (NULL == m_YYYXOutputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_YYYXOutputPin failed!");
		return false;
	}
	IncrementPinVersion();

	// add recorder output pin
	if (NULL != m_LZOutputPin)
	{
		delete m_LZOutputPin;
		m_LZOutputPin = NULL;
	}

	m_LZOutputPin = new CTransformExtandOutputPin("Output", this, &hr, L"RecorderOut", 2);
		m_LZOutputPin->m_eTPId=TP2RecordingDev;
	if (NULL == m_LZOutputPin)
	{
		AUDIO_PROCESSING_PRINTF("New m_LZOutputPin failed!");
		return false;
	}
	IncrementPinVersion();

#endif
	return true;
}


HRESULT CAudioProcessFilter::__ReleaseAudioProcessFilter(void)
{
	HRESULT hr = NOERROR;

#if FILTER_MODE_1_IN_2_OUT
	// delete OnlineClass input pins 
	if (!_ResetOnlineClassInputPinsList())
	{
		AUDIO_PROCESSING_PRINTF("_ResetOnlineClassInputPinsList() failed!");
	}

	// delete on-line class output pin 
	if (m_pOnlineClassOutputPin)
	{
		delete m_pOnlineClassOutputPin;
		m_pOnlineClassOutputPin = NULL;
	}
	// delete recorder output pin 
	if (m_pRecordOutputPin)
	{
		delete m_pRecordOutputPin;
		m_pRecordOutputPin = NULL;
	}
#else
	// delete class-to-class input pin 
	if (m_BBTInputPin)
	{
		delete m_BBTInputPin;
		m_BBTInputPin = NULL;
	}

	// delete on-line class input pin 
	if (m_ZXKTInputPin)
	{
		delete m_ZXKTInputPin;
		m_ZXKTInputPin = NULL;
	}

	// delete electronic piano input pin 
	if (m_DZGQInputPin)
	{
		delete m_DZGQInputPin;
		m_DZGQInputPin = NULL;
	}

	// delete mike input pins 
	if (!_ResetMicInputPinsList())
	{
		AUDIO_PROCESSING_PRINTF("_ResetMicInputPinsList() failed!");
	}

	// delete on-line class output pin 
	if (m_ZXKTOutputPin)
	{
		delete m_ZXKTOutputPin;
		m_ZXKTOutputPin = NULL;
	}

	// delete speaker output pin 
	if (m_YYYXOutputPin)
	{
		delete m_YYYXOutputPin;
		m_YYYXOutputPin = NULL;
	}

	// delete recorder output pin 
	if (m_LZOutputPin)
	{
		delete m_LZOutputPin;
		m_LZOutputPin = NULL;
	}
#endif

	// release audio process parameter
	__ReleaseStreaming();

	return true;
}

//! initial streaming
HRESULT CAudioProcessFilter::__InitStreaming(void)
{
	HRESULT hr = NOERROR;

	hr = __GetPinInfo();
	if (FAILED(hr))
	{
		AUDIO_PROCESSING_PRINTF("input or output pins are incorrect connected!");
		return E_FAIL;
	}

	hr = __InitWavFile();
	if (FAILED(hr))
	{
		AUDIO_PROCESSING_PRINTF("init wave file error!");
		return E_FAIL;
	}

	if (NULL != m_pAudioEngine)
	{
		IAudioEngine::Delete(m_pAudioEngine);
	}

	m_pAudioEngine = IAudioEngine::Create(m_sAudio_type_num, 1, FRAME_TIME_MS, m_sSampleRate, REAL_FLOAT_DATA);
	if (NULL == m_pAudioEngine)
	{
		return E_FAIL;
	}
	if (!m_pAudioEngine->Init(m_bAecSwitch, m_pFolderName))
	{
		AUDIO_PROCESSING_PRINTF("init Audio Engine failed");
		return E_FAIL;
	}

#if FILTER_MODE_1_IN_2_OUT
	// register transport to network
	if (!m_pAudioEngine->RegisterAudioTransport(m_pOnlineClassOutputPin, TP2Network))
	{
		DbgLog((LOG_TRACE, 0, TEXT("register transport failed!")));
		return false;
	}
	// register transport to recording dev
	if (m_bRecordOutputPin)
	{
		if (!m_pAudioEngine->RegisterAudioTransport(m_pRecordOutputPin, TP2RecordingDev))
		{
			DbgLog((LOG_TRACE, 0, TEXT("register transport failed!")));
			return false;
		}
	}
#else
	// register transport to network
	if (!m_pAudioEngine->RegisterAudioTransport(m_ZXKTOutputPin, TP2Network))
	{
		DbgLog((LOG_TRACE, 0, TEXT("register transport failed!")));
		return false;
	}

	// register transport to render dev
	if (m_bZXKTInputPin)
	{
		if (!m_pAudioEngine->RegisterAudioTransport(m_YYYXOutputPin, TP2RenderDev))
		{
			DbgLog((LOG_TRACE, 0, TEXT("register transport failed!")));
			return false;
		}
	}

	// register transport to recording dev
	if (m_bLZOutputPin)
	{
		if (!m_pAudioEngine->RegisterAudioTransport(m_LZOutputPin, TP2RecordingDev))
		{
			DbgLog((LOG_TRACE, 0, TEXT("register transport failed!")));
			return false;
		}
	}
#endif

	// register property page
	if (!m_pAudioEngine->RegisterPropertyPage(this))
	{
		AUDIO_PROCESSING_PRINTF("register property page!");
		return E_FAIL;
	}


	// begin audio pump
	if (m_pAudioEngine)
	{
		if (Audio_Module_State_Running != m_pAudioEngine->Start())
		{
			AUDIO_PROCESSING_PRINTF("start Audio Engine failed");
			return E_FAIL;
		}
	}
	//merged from v0.5
	if (NULL != m_pEOSFlag)
	{
		delete m_pEOSFlag;
		m_pEOSFlag = NULL;
	}
	m_pEOSFlag = new bool[m_nInputPinFactNum - 1];
	if (NULL == m_pEOSFlag)
	{
		return false;
	}
	for (CAUDIO_S32_t i = 0; i < m_nInputPinFactNum - 1; ++i)
	{
		m_pEOSFlag[i] = false;
	}
	//merged from v0.5 end
	return hr;
}

//! release streaming
HRESULT CAudioProcessFilter::__ReleaseStreaming(void)
{
	HRESULT hr = NOERROR;

	hr = __ReleaseWavFile();
	if (FAILED(hr))
	{
		AUDIO_PROCESSING_PRINTF("release wave file failed!");
		return E_FAIL;
	}

	if (NULL != m_pAudioEngine)
	{
		IAudioEngine::Delete(m_pAudioEngine);
	}

	if (NULL != m_pInBuffer_FeedData)
	{
		delete[] m_pInBuffer_FeedData;
		m_pInBuffer_FeedData = NULL;
	}


	if (NULL != m_pEOSFlag)
	{
		delete m_pEOSFlag;
		m_pEOSFlag = NULL;
	}

	m_mReceivedSampleInfo.clear();

	return true;
}


HRESULT CAudioProcessFilter::__InitWavFile(void)
{
	HRESULT hr = NOERROR;

#ifdef _DEBUG
	std::string Dir;
	std::string File;
	char folder_name[256];
	char file_name[256];
	char trace_name[256];
	tm *local;
	time_t t;

	// create directory 
	Dir.clear();
	Dir = "C:\\audio_wave_file";
	_mkdir(Dir.c_str());

	// create folder
	t = time(NULL);
	local = localtime(&t);
	m_pFolderName.clear();
	memset(folder_name, 0, sizeof(char)* 256);
	sprintf_s(folder_name, 256, "%s\\%04d_%02d_%02d_%02d_%02d_%02d",
		Dir.data(), local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	m_pFolderName = folder_name;
	_mkdir(m_pFolderName.c_str());
#endif

#ifdef AUDIO_WAVE_DEBUG
	// new wave information for each input pin
	if (!m_mWaveFileInfo.empty())
	{
		m_mWaveFileInfo.clear();
	}

	#if FILTER_MODE_1_IN_2_OUT
		if (m_sAudio_type_num.nOnLineClass_in_ > 0)
		{
			POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;

			for (CAUDIO_U32_t index = 0; index < m_sAudio_type_num.nOnLineClass_in_; ++index)
			{
				char name[256];
				sprintf_s(name, 256, "OnlineClass%dIn", m_vOnlineClassInputPinIndex_StoreUsed[index]);

				if (NULL != pos)
				{
					pInputPin = m_onlineClassInputPinList.GetNext(pos);
					if (FAILED(hr = __CreateWavFile(m_pFolderName.data(), name, pInputPin)))
						return false;
				}
			}
		}
		// new wave information for each output pin
		if (m_bOnlineClassOutputPin)
		{
			memset(file_name, 0, sizeof(char) * 256);
			File.clear();
			sprintf_s(file_name, 256, "%s\\OnlineClassOut.wav", m_pFolderName.data());
			File = file_name;
			m_pOnlineClassOutputPin->InitWavFile(File, m_sSampleRate.nSampleRate_OnlineClassOut);
		}
		if (m_bRecordOutputPin)
		{
			memset(file_name, 0, sizeof(char) * 256);
			File.clear();
			sprintf_s(file_name, 256, "%s\\RecorderOut.wav", m_pFolderName.data());
			File = file_name;
			m_pRecordOutputPin->InitWavFile(File, m_sSampleRate.nSampleRate_RecorderOut);
		}
	#else

		if (m_bBBTInputPin)
		{
			if (FAILED(hr = __CreateWavFile(m_pFolderName.data(), "ClassToClassIn", m_BBTInputPin)))
				return false;
		}
		if (m_bZXKTInputPin)
		{
			if (FAILED(hr = __CreateWavFile(m_pFolderName.data(), "OnlineClassIn", m_ZXKTInputPin)))
				return false;
		}
		if (m_bDZGQInputPin)
		{
			if (FAILED(hr = __CreateWavFile(m_pFolderName.data(), "ElectronicPianoIn", m_DZGQInputPin)))
				return false;
		}
		if (m_sAudio_type_num.nMic_in_ > 0)
		{
			POSITION pos = m_MicInputPinList.GetHeadPosition();
			CTransformExtandInputPin *pInputPin = NULL;

			for (CAUDIO_U32_t mic_index = 0; mic_index < m_sAudio_type_num.nMic_in_; ++mic_index)
			{
				char mic_name[256];
				sprintf_s(mic_name, 256, "Mic%dIn", m_vMicInputPinIndex_StoreUsed[mic_index]);

				if (NULL != pos)
				{
					pInputPin = m_MicInputPinList.GetNext(pos);
					if (FAILED(hr = __CreateWavFile(m_pFolderName.data(), mic_name, pInputPin)))
						return false;
				}
			}
		}
		// new wave information for each output pin
		if (m_bZXKTOutputPin)
		{
			memset(file_name, 0, sizeof(char) * 256);
			File.clear();
			sprintf_s(file_name, 256, "%s\\OnlineClassOut.wav", m_pFolderName.data());
			File = file_name;
			m_ZXKTOutputPin->InitWavFile(File, m_sSampleRate.nSampleRate_OnlineClassOut);
		}
		if (m_bYYYZOutputPin)
		{
			memset(file_name, 0, sizeof(char) * 256);
			File.clear();
			sprintf_s(file_name, 256, "%s\\SpeakerOut.wav", m_pFolderName.data());
			File = file_name;
			m_YYYXOutputPin->InitWavFile(File, m_sSampleRate.nSampleRate_SpeakerOut);
		}
		if (m_bLZOutputPin)
		{
			memset(file_name, 0, sizeof(char) * 256);
			File.clear();
			sprintf_s(file_name, 256, "%s\\RecorderOut.wav", m_pFolderName.data());
			File = file_name;
			m_LZOutputPin->InitWavFile(File, m_sSampleRate.nSampleRate_RecorderOut);
		}
	#endif

#endif

#ifdef 	AUDIO_TRACE_DEBUG
	// add trace
	memset(trace_name, 0, sizeof(char)* 256);
	sprintf_s(trace_name, 256, "%s\\audio_engine_log.txt", m_pFolderName.data());
	CreateTrace(trace_name);
#endif

	return hr;
}


HRESULT CAudioProcessFilter::__ReleaseWavFile(void)
{
	HRESULT hr = NOERROR;

#ifdef AUDIO_WAVE_DEBUG

	if (!m_mWaveFileInfo.empty())
	{
		for (MapWavFileInfo::iterator iter = m_mWaveFileInfo.begin(); iter != m_mWaveFileInfo.end(); ++iter)
		{
			SWavFile wave_file = iter->second;
			if (wave_file.m_pWavFileHead && wave_file.m_pWavFileOp)
			{
				wave_file.m_pWavFileOp->UpdateHeader(wave_file.m_pWavFileHead->NChannels, wave_file.m_nWaveCounter / wave_file.m_pWavFileHead->NChannels);
				delete wave_file.m_pWavFileOp;
				delete wave_file.m_pWavFileHead;
				wave_file.m_pWavFileOp = NULL;
				wave_file.m_pWavFileHead = NULL;
			}
		}
		m_mWaveFileInfo.clear();
	}

#if FILTER_MODE_1_IN_2_OUT
	if (m_pOnlineClassOutputPin)
	{
		m_pOnlineClassOutputPin->ReleaseWavFile();
	}
	if (m_pRecordOutputPin)
	{
		m_pRecordOutputPin->ReleaseWavFile();
	}
#else
	if (m_bZXKTOutputPin)
	{
		m_ZXKTOutputPin->ReleaseWavFile();
	}
	if (m_bYYYZOutputPin)
	{
		m_YYYXOutputPin->ReleaseWavFile();
	}
	if (m_bLZOutputPin)
	{
		m_LZOutputPin->ReleaseWavFile();
	}
#endif

#endif

#ifdef 	AUDIO_TRACE_DEBUG
	ReturnTrace();
#endif

	return hr;
}

HRESULT CAudioProcessFilter::__CreateWavFile(const char *folder_name, const char *file_name, CTransformOriginInputPin *input_pin)
{
	HRESULT hr = NOERROR;

	SWavFile wave_file;
	CAUDIO_U32_t sample_rate = 0;

	MapSampleRateInfo::iterator iter = m_mInputSampleRate.find(file_name);
	if (m_mInputSampleRate.end() != iter)
	{
		sample_rate = iter->second;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("can not find file_name");
		return E_FAIL;
	}

	// save file wave
	sprintf_s(wave_file.m_pFileName, 256, "%s\\%s.wav", folder_name, file_name);
	wave_file.m_pWavFileOp = new CWavFileOp(wave_file.m_pFileName, "wb");
	wave_file.m_pWavFileHead = new SWavFileHead;
	if (-2 == wave_file.m_pWavFileOp->m_FileStatus)
	{
		delete wave_file.m_pWavFileOp;
		delete wave_file.m_pWavFileHead;
		wave_file.m_pWavFileOp = NULL;
		wave_file.m_pWavFileHead = NULL;
		AUDIO_PROCESSING_PRINTF("new wave file failed ");
		return E_FAIL;
	}
	wave_file.m_pWavFileHead->NChannels = 2;
	wave_file.m_pWavFileHead->RIFF[0] = 'R';
	wave_file.m_pWavFileHead->RIFF[1] = 'I';
	wave_file.m_pWavFileHead->RIFF[2] = 'F';
	wave_file.m_pWavFileHead->RIFF[3] = 'F';
	wave_file.m_pWavFileHead->data[0] = 'd';
	wave_file.m_pWavFileHead->data[1] = 'a';
	wave_file.m_pWavFileHead->data[2] = 't';
	wave_file.m_pWavFileHead->data[3] = 'a';
	wave_file.m_pWavFileHead->WAVEfmt_[0] = 'W';
	wave_file.m_pWavFileHead->WAVEfmt_[1] = 'A';
	wave_file.m_pWavFileHead->WAVEfmt_[2] = 'V';
	wave_file.m_pWavFileHead->WAVEfmt_[3] = 'E';
	wave_file.m_pWavFileHead->WAVEfmt_[4] = 'f';
	wave_file.m_pWavFileHead->WAVEfmt_[5] = 'm';
	wave_file.m_pWavFileHead->WAVEfmt_[6] = 't';
	wave_file.m_pWavFileHead->WAVEfmt_[7] = ' ';

	wave_file.m_pWavFileHead->noUse = 0x00000010;
	wave_file.m_pWavFileHead->FormatCategory = 1;
	wave_file.m_pWavFileHead->SampleRate = sample_rate;
	wave_file.m_pWavFileHead->SampleBytes = sample_rate * 4;
	wave_file.m_pWavFileHead->BytesPerSample = 4;
	wave_file.m_pWavFileHead->NBitsPersample = 16;

	wave_file.m_nWaveCounter = 0;
	if (wave_file.m_pWavFileOp)
		wave_file.m_pWavFileOp->WriteHeader(*wave_file.m_pWavFileHead);

	m_mWaveFileInfo.insert(std::make_pair(file_name, wave_file));

	return hr;
}

HRESULT CAudioProcessFilter::__CreateWavFile(const char *folder_name, const char *file_name, CTransformExtandInputPin *input_pin)
{
	HRESULT hr = NOERROR;

	SWavFile wave_file;
	CAUDIO_U32_t sample_rate = 0;

	MapSampleRateInfo::iterator iter = m_mInputSampleRate.find(file_name);
	if (m_mInputSampleRate.end() != iter)
	{
		sample_rate = iter->second;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("can not find file_name");
		return E_FAIL;
	}

	// save wave file 
	sprintf_s(wave_file.m_pFileName, 256, "%s\\%s.wav", folder_name, file_name);
	wave_file.m_pWavFileOp = new CWavFileOp(wave_file.m_pFileName, "wb");
	wave_file.m_pWavFileHead = new SWavFileHead;
	if (-2 == wave_file.m_pWavFileOp->m_FileStatus)
	{
		delete wave_file.m_pWavFileOp;
		delete wave_file.m_pWavFileHead;
		wave_file.m_pWavFileOp = NULL;
		wave_file.m_pWavFileHead = NULL;
		return false;
	}
	wave_file.m_pWavFileHead->NChannels = 2;
	wave_file.m_pWavFileHead->RIFF[0] = 'R';
	wave_file.m_pWavFileHead->RIFF[1] = 'I';
	wave_file.m_pWavFileHead->RIFF[2] = 'F';
	wave_file.m_pWavFileHead->RIFF[3] = 'F';
	wave_file.m_pWavFileHead->data[0] = 'd';
	wave_file.m_pWavFileHead->data[1] = 'a';
	wave_file.m_pWavFileHead->data[2] = 't';
	wave_file.m_pWavFileHead->data[3] = 'a';
	wave_file.m_pWavFileHead->WAVEfmt_[0] = 'W';
	wave_file.m_pWavFileHead->WAVEfmt_[1] = 'A';
	wave_file.m_pWavFileHead->WAVEfmt_[2] = 'V';
	wave_file.m_pWavFileHead->WAVEfmt_[3] = 'E';
	wave_file.m_pWavFileHead->WAVEfmt_[4] = 'f';
	wave_file.m_pWavFileHead->WAVEfmt_[5] = 'm';
	wave_file.m_pWavFileHead->WAVEfmt_[6] = 't';
	wave_file.m_pWavFileHead->WAVEfmt_[7] = ' ';

	wave_file.m_pWavFileHead->noUse = 0x00000010;
	wave_file.m_pWavFileHead->FormatCategory = 1;
	wave_file.m_pWavFileHead->SampleRate = sample_rate;
	wave_file.m_pWavFileHead->SampleBytes = sample_rate * 4;
	wave_file.m_pWavFileHead->BytesPerSample = 4;
	wave_file.m_pWavFileHead->NBitsPersample = 16;

	wave_file.m_nWaveCounter = 0;
	if (wave_file.m_pWavFileOp)
		wave_file.m_pWavFileOp->WriteHeader(*wave_file.m_pWavFileHead);

	m_mWaveFileInfo.insert(std::make_pair(file_name, wave_file));

	return hr;
}

HRESULT CAudioProcessFilter::__WriteWavFile(CAUDIO_S16_t *data, CAUDIO_S32_t size, std::string name)
{
	HRESULT hr = NOERROR;

	if (m_State == State_Stopped)
	{
		AUDIO_PROCESSING_PRINTF("m_State == State_Stopped");
		return E_FAIL;
	}

	MapWavFileInfo::iterator iter = m_mWaveFileInfo.find(name);
	if (iter != m_mWaveFileInfo.end() && iter->second.m_pWavFileOp)
	{
		iter->second.m_pWavFileOp->WriteSample(data, size);
		iter->second.m_nWaveCounter += size;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("can not find pin name");
		return E_FAIL;
	}

	return hr;
}

HRESULT CAudioProcessFilter::__GetOriginalSampleRate(std::string name, CAUDIO_U32_t &sample_rate)
{
	HRESULT hr = NOERROR;

	MapSampleRateInfo::iterator iter = m_mInputSampleRate.find(name);
	if (m_mInputSampleRate.end() != iter)
	{
		sample_rate = iter->second;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("can not find pin name");
		return E_FAIL;
	}

	return hr;
}

//! get input/output pin information
HRESULT CAudioProcessFilter::__GetPinInfo(void)
{
	HRESULT hr = NOERROR;

	memset(&m_sAudio_type_num, 0, sizeof(AUDIO_TYPE_NUM));
	m_mReceivedSampleInfo.clear();
	CAUDIO_S32_t nomic_inputpin_count = 0; // count of input pin except mic 
	ALLOCATOR_PROPERTIES pProp;
	CTransformExtandInputPin *pInputPin = NULL;

#if FILTER_MODE_1_IN_2_OUT
	POSITION pos = m_onlineClassInputPinList.GetHeadPosition();
	for (CAUDIO_S32_t index = 0; index < m_nOnlineClassInputPinCount_CalcUsed - 1; ++index)
	{
		m_mReceivedSampleInfo.insert(std::make_pair(m_vOnlineClassInputPinIndex_StoreUsed[index], SReceivedSampleInfo(index, RenderCh)));

		pInputPin = m_onlineClassInputPinList.GetNext(pos);
		memset(&pProp, 0, sizeof(ALLOCATOR_PROPERTIES));
		if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
		{
			if (FAILED(pInputPin->PeekAllocator()->GetProperties(&pProp)))
			{
				return E_FAIL;
			}
			else
			{
				if (FAILED(__CheckProperties(&pProp)))
				{
					return false;
				}
			}
		}
	}
	m_nInputPinFactNum = m_nOnlineClassInputPinCount_CalcUsed;
	m_sAudio_type_num.nOnLineClass_in_ = m_nOnlineClassInputPinCount_CalcUsed - 1;

	if (m_sAudio_type_num.nOnLineClass_in_ <= 0 || !m_bOnlineClassOutputPin)
	{
		return false;
	}

	if (m_bOnlineClassOutputPin)
	{
		m_sAudio_type_num.nOnLineClass_out_ = 1;
	}

	if (m_bRecordOutputPin)
	{
		m_sAudio_type_num.nRecording_out_ = 1;
	}

#else
	if (m_bBBTInputPin)
	{
		m_mReceivedSampleInfo.insert(std::make_pair(-3, SReceivedSampleInfo(0, SharedCh)));
		m_sAudio_type_num.nClassToClass_in_ = 1;
		++nomic_inputpin_count;

		memset(&pProp, 0, sizeof(ALLOCATOR_PROPERTIES));
		if (FAILED(m_BBTInputPin->PeekAllocator()->GetProperties(&pProp)))
		{
			return false;
		}
		else
		{
			if (FAILED(__CheckProperties(&pProp)))
			{
				return false;
			}
		}
	}
	if (m_bZXKTInputPin)
	{
		m_mReceivedSampleInfo.insert(std::make_pair(-2, SReceivedSampleInfo(0, RenderCh)));
		m_sAudio_type_num.nOnLineClass_in_ = 1;
		++nomic_inputpin_count;

		memset(&pProp, 0, sizeof(ALLOCATOR_PROPERTIES));
		if (FAILED(m_ZXKTInputPin->PeekAllocator()->GetProperties(&pProp)))
		{
			return false;
		}
		else
		{
			if (FAILED(__CheckProperties(&pProp)))
			{
				return false;
			}
		}
}
	if (m_bDZGQInputPin)
	{
		if (m_bBBTInputPin)
		{
			m_mReceivedSampleInfo.insert(std::make_pair(-1, SReceivedSampleInfo(1, SharedCh)));
		}
		else
		{
			m_mReceivedSampleInfo.insert(std::make_pair(-1, SReceivedSampleInfo(0, SharedCh)));
		}
		m_sAudio_type_num.nElectronicPiano_in_ = 1;
		++nomic_inputpin_count;

		memset(&pProp, 0, sizeof(ALLOCATOR_PROPERTIES));
		if (FAILED(m_DZGQInputPin->PeekAllocator()->GetProperties(&pProp)))
		{
			return false;
		}
		else
		{
			if (FAILED(__CheckProperties(&pProp)))
			{
				return false;
			}
		}
	}

	POSITION pos = m_MicInputPinList.GetHeadPosition();
	for (CAUDIO_S32_t index = 0; index < m_nMicInputPinCount_CalcUsed - 1; ++index)
	{
		m_mReceivedSampleInfo.insert(std::make_pair(m_vMicInputPinIndex_StoreUsed[index], SReceivedSampleInfo(index, CaptureCh)));

		pInputPin = m_MicInputPinList.GetNext(pos);
		memset(&pProp, 0, sizeof(ALLOCATOR_PROPERTIES));
		if (NULL != pInputPin && TRUE == pInputPin->IsConnected())
		{
			if (FAILED(pInputPin->PeekAllocator()->GetProperties(&pProp)))
			{
				return E_FAIL;
			}
			else
			{
				if (FAILED(__CheckProperties(&pProp)))
				{
					return false;
				}
			}
		}
	}
	m_nInputPinFactNum = m_nMicInputPinCount_CalcUsed + nomic_inputpin_count;
	m_sAudio_type_num.nMic_in_ = m_nMicInputPinCount_CalcUsed - 1;
	m_sAudio_type_num.nTotal_in_ = m_nMicInputPinCount_CalcUsed + nomic_inputpin_count - 1;

#if CAPTURE_PLAYBACK_INTEGRATED
	if (0 == m_sAudio_type_num.nOnLineClass_in_ || !m_bZXKTOutputPin)
	{
		return false;
	}
#else
	// The audio engine should never be started when there is no mike input pin be connected or no output pin be connected.
	if (0 == m_sAudio_type_num.nMic_in_ || !m_bZXKTOutputPin)
	{
		return false;
	}
#endif

	if (m_bLZOutputPin)
	{
		m_sAudio_type_num.nRecording_out_ = 1;
	}

	if (m_bZXKTOutputPin)
	{
		m_sAudio_type_num.nOnLineClass_out_ = 1;
	}

	if (m_bYYYZOutputPin)
	{
		m_sAudio_type_num.nSpeaker_out_ = 1;
	}
#endif

	m_sAudio_type_num.nTotal_out_ = m_sAudio_type_num.nRecording_out_ + m_sAudio_type_num.nOnLineClass_out_
		+ m_sAudio_type_num.nSpeaker_out_;
	m_sAudio_type_num.nTotal_in_ = m_sAudio_type_num.nClassToClass_in_ + m_sAudio_type_num.nElectronicPiano_in_
		+ m_sAudio_type_num.nMic_in_ + m_sAudio_type_num.nOnLineClass_in_;
	return hr;
}

HRESULT CAudioProcessFilter::__Fix2Float(const CAUDIO_S16_t* buffer_in, const CAUDIO_U32_t len_in)
{
	if (NULL == m_pInBuffer_FeedData || len_in / 2 != m_nInBufferSize_FeedData)
	{
		if (m_pInBuffer_FeedData)
		{
			delete m_pInBuffer_FeedData;
			m_pInBuffer_FeedData = NULL;
		}
		m_pInBuffer_FeedData = new AUDIO_DATA_TYPE[len_in / 2];
		if (NULL == m_pInBuffer_FeedData)
		{
			return false;
		}
		m_nInBufferSize_FeedData = len_in / 2;
	}
	for (CAUDIO_U32_t i = 0; i < len_in / 2; ++i)
	{
		m_pInBuffer_FeedData[i] = buffer_in[i*m_nChannelNum] / 32768.f;
	}
	return true;
}


HRESULT CAudioProcessFilter::__CheckProperties(ALLOCATOR_PROPERTIES *pProp)
{
	HRESULT hr = NOERROR;

	AUDIO_DATA_TYPE buffer_size = static_cast<AUDIO_DATA_TYPE>(pProp->cbBuffer) / 4;
	if (buffer_size != (CAUDIO_U32_t)buffer_size)
	{
		AUDIO_PROCESSING_PRINTF("buffer_size != (CAUDIO_U32_t)buffer_size");
		return E_FAIL;
	}

	return hr;
}

//! Basic COM - used here to reveal our own interfaces
STDMETHODIMP CAudioProcessFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv, E_POINTER);

	if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface((ISpecifyPropertyPages *) this, ppv);
	}
	else if (riid == IID_IAUDIOPROCESS)
	{
		return GetInterface((IAudioProcess *) this, ppv);
	}
	else
	{
		return CTransformExtandFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}
//merged from v0.5
//! get channel number
STDMETHODIMP CAudioProcessFilter::GetAudioChannelNumber(bool &nomic_inputpin)
{
	if (m_State != State_Paused)
	{
		return E_UNEXPECTED;
	}
	
#if FILTER_MODE_1_IN_2_OUT
	nomic_inputpin = false;
#else
	nomic_inputpin = (TRUE == m_bBBTInputPin) ? true : ((TRUE == m_bDZGQInputPin) ? true : false);
#endif

	return NOERROR;
}

//! ISpecifyPropertyPages
STDMETHODIMP CAudioProcessFilter::GetPages(CAUUID *pPages)
{
	pPages->cElems = 1;
	pPages->pElems = (GUID *)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
	{
		return E_OUTOFMEMORY;
	}
	*(pPages->pElems) = CLSID_AudioProcessProp;
	return NOERROR;
}

//! get parameters to corresponding channel
STDMETHODIMP CAudioProcessFilter::GetPropertyPage(AUDIO_PROPERTY_PAGE &property_page)
{
	if (m_State != State_Paused)
	{
		AUDIO_PROCESSING_PRINTF("m_State != State_Paused");
		return E_UNEXPECTED;
	}

	// TODO: optimize this logic
	if (NULL != m_pPropertyPage)
	{
		memcpy(&property_page, m_pPropertyPage, sizeof(AUDIO_PROPERTY_PAGE));
	}

	return NOERROR;
}

//! get channel number
STDMETHODIMP CAudioProcessFilter::GetMixState(bool &nomic_inputpin)
{
	if (m_State != State_Paused)
	{
		AUDIO_PROCESSING_PRINTF("m_State != State_Paused");
		return E_UNEXPECTED;
	}

#if FILTER_MODE_1_IN_2_OUT
	nomic_inputpin = false;
#else
	nomic_inputpin = m_bBBTInputPin ? true : false;
#endif

	return NOERROR;
}

STDMETHODIMP CAudioProcessFilter::SetAecState(bool aec_switch)
{
	m_bAecSwitch = aec_switch;

	return NOERROR;
}

STDMETHODIMP CAudioProcessFilter::SetSampleRate(SSampleRate sample_rate)
{
	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_ClassToClassIn))
	{
		m_sSampleRate.nSampleRate_ClassToClassIn = sample_rate.nSampleRate_ClassToClassIn;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_OnLineClassIn))
	{
		m_sSampleRate.nSampleRate_OnLineClassIn = sample_rate.nSampleRate_OnLineClassIn;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_ElectronicPianoIn))
	{
		m_sSampleRate.nSampleRate_ElectronicPianoIn = sample_rate.nSampleRate_ElectronicPianoIn;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_MicIn))
	{
		m_sSampleRate.nSampleRate_MicIn = sample_rate.nSampleRate_MicIn;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_InterProcessUsed))
	{
		m_sSampleRate.nSampleRate_InterProcessUsed = sample_rate.nSampleRate_InterProcessUsed;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_OnlineClassOut))
	{
		m_sSampleRate.nSampleRate_OnlineClassOut = sample_rate.nSampleRate_OnlineClassOut;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_RecorderOut))
	{
		m_sSampleRate.nSampleRate_RecorderOut = sample_rate.nSampleRate_RecorderOut;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	if (SAMPLE_RATE_VALID_CHECK(sample_rate.nSampleRate_SpeakerOut))
	{
		m_sSampleRate.nSampleRate_SpeakerOut = sample_rate.nSampleRate_SpeakerOut;
	}
	else
	{
		AUDIO_PROCESSING_PRINTF("sample rate is invalid");
		return E_FAIL;
	}

	return NOERROR;
}

//! stop 
STDMETHODIMP CAudioProcessFilter::Stop()
{
	// stop audio pump
	if (m_pAudioEngine)
	{
		m_pAudioEngine->Stop();
	}
	return CTransformExtandFilter::Stop();
}

STDMETHODIMP CAudioProcessFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{

	CheckPointer(ppPin, E_POINTER);
	ValidateReadWritePtr(ppPin, sizeof(IPin *));

#if FILTER_MODE_1_IN_2_OUT
	if (0 == lstrcmpW(Id, L"OnlineClassOut")) {
		*ppPin = GetPin(m_nOnlineClassInputPinCount);
	}
	else if (0 == lstrcmpW(Id, L"RecorderOut")) {
		*ppPin = GetPin(m_nOnlineClassInputPinCount + 1);
	}
	else{
		std::vector<CAUDIO_S32_t>::iterator iter = m_vOnlineClassInputPinIndex_StoreUsed.begin();
		WCHAR name[16] = { 0 };
		CAUDIO_U32_t index = 0;

		while (iter != m_vOnlineClassInputPinIndex_StoreUsed.end())
		{
			swprintf(name, sizeof(name), L"OnlineClass%dIn", *iter);
			if (0 == lstrcmpW(Id, name))
			{
				*ppPin = GetPin(index);
				break;
			}
			++iter;
			++index;
		}

		if (iter == m_vOnlineClassInputPinIndex_StoreUsed.end())
		{
			*ppPin = NULL;
			return VFW_E_NOT_FOUND;
		}
	}

#else

	if (0 == lstrcmpW(Id, L"ClassToClassIn")) {
		*ppPin = GetPin(0);
	}
	else if (0 == lstrcmpW(Id, L"OnlineClassIn")) {
		*ppPin = GetPin(1);
	}
	else if (0 == lstrcmpW(Id, L"ElectronicPianoIn")) {
		*ppPin = GetPin(2);
	}
	else if (0 == lstrcmpW(Id, L"OnlineClassOut")) {
		*ppPin = GetPin(m_nMicInputPinCount + 3);
	}
	else if (0 == lstrcmpW(Id, L"SpeakerOut")) {
		*ppPin = GetPin(m_nMicInputPinCount + 4);
	}
	else if (0 == lstrcmpW(Id, L"RecorderOut")) {
		*ppPin = GetPin(m_nMicInputPinCount + 5);
	}
	else{
		std::vector<CAUDIO_S32_t>::iterator iter = m_vMicInputPinIndex_StoreUsed.begin();
		WCHAR name[16] = { 0 };
		CAUDIO_U32_t mike_index = 3;

		while (iter != m_vMicInputPinIndex_StoreUsed.end())
		{
			swprintf(name, sizeof(name), L"Mic%dIn", *iter);
			if (0 == lstrcmpW(Id, name))
			{
				*ppPin = GetPin(mike_index);
				break;
			}
			++iter;
			++mike_index;
		}

		if (iter == m_vMicInputPinIndex_StoreUsed.end())
		{
			*ppPin = NULL;
			return VFW_E_NOT_FOUND;
		}
	}
#endif

	HRESULT hr = NOERROR;
	//  AddRef() returned pointer - but GetPin could fail if memory is low.
	if (*ppPin) {
		(*ppPin)->AddRef();
	}
	else {
		hr = E_OUTOFMEMORY;  // probably.  There's no pin anyway.
	}
	return hr;
}

STDMETHODIMP CAudioProcessFilter::GetFilterState(void)
{
	if (m_State != State_Paused)
	{
		AUDIO_PROCESSING_PRINTF("m_State != State_Paused");
		return E_UNEXPECTED;
	}
	else
	{
		return NOERROR;
	}
}





/* ---------------------------------------------------------------- *
default function of dll
* ---------------------------------------------------------------- */
//! Registered com components
STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}
//! Unregistered com components
STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}
//! the entry point function of dll 
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  dwReason,
	LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);

}