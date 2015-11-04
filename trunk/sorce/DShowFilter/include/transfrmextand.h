/*! \file   transfrmextand.h
*   \author Keil 
*   \date   2015/1/7
*   \brief  base class of AudioProcessFilter
*/

#ifndef _TRANSFRMEXTAND_H_
#define _TRANSFRMEXTAND_H_

#include <vector>
#include "AudioProcessFilterDefs.h"   
#include "IAudioTransport.h" 
#include "WaveIO.h"

class CTransformExtandFilter;

//merged from v0.5 begin
class CTransformExtandInputPin : public CBaseInputPin
{
    friend class CTransformExtandFilter;

public:
    CTransformExtandInputPin(TCHAR *pObjectName, CTransformExtandFilter *pTransformFilter,  HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t nIndex);
#ifdef UNICODE
	CTransformExtandInputPin(CHAR *pObjectName, CTransformExtandFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t  nIndex);
#endif

	// Override since the life time of pins and filters are not the same
	STDMETHODIMP_(ULONG) NonDelegatingAddRef();
	STDMETHODIMP_(ULONG) NonDelegatingRelease();

    STDMETHODIMP QueryId(LPWSTR * Id)
    {
        return AMGetWideString(L"In", Id);
    }

    // Grab and release extra interfaces if required
    HRESULT CheckConnect(IPin *pPin);
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);

    // check that we can support this output type
    HRESULT CheckMediaType(const CMediaType* mtIn);

    // set the connection media type
    HRESULT SetMediaType(const CMediaType* mt);

    // --- IMemInputPin -----

    // here's the next block of data from the stream.
    // AddRef it yourself if you need to hold it beyond the end of this call.
    STDMETHODIMP Receive(IMediaSample * pSample);

    // provide EndOfStream that passes straight downstream 
    STDMETHODIMP EndOfStream(void);

    // passes it to CTransformFilter::BeginFlush
    STDMETHODIMP BeginFlush(void);

    // passes it to CTransformFilter::EndFlush
    STDMETHODIMP EndFlush(void);

    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    // Check if it's OK to process samples
    virtual HRESULT CheckStreaming();

	// Allow the filter to see what allocator we have
	// N.B. This does NOT AddRef
	__out IMemAllocator * PeekAllocator() const
	{  return m_pAllocator; }

    // Media type
    CMediaType& CurrentMediaType() { return m_mt; };

protected:
	CTransformExtandFilter *m_pTransformFilter; 
	CAUDIO_S32_t	 m_nIndex;  
	LONG             m_cOurRef; // We maintain reference counting
	bool             m_bIsDataComeIn;

};
//merged from v0.5 end

class CTransformOriginInputPin : public CBaseInputPin
{
	friend class CTransformExtandFilter;

public:
	CTransformOriginInputPin(TCHAR *pObjectName, CTransformExtandFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t nIndex);
#ifdef UNICODE
	CTransformOriginInputPin(CHAR *pObjectName, CTransformExtandFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t  nIndex);
#endif

	STDMETHODIMP QueryId(LPWSTR * Id)
	{
		return AMGetWideString(L"In", Id);
	}

	// Grab and release extra interfaces if required
	HRESULT CheckConnect(IPin *pPin);
	HRESULT BreakConnect();
	HRESULT CompleteConnect(IPin *pReceivePin);

	// check that we can support this output type
	HRESULT CheckMediaType(const CMediaType* mtIn);
	// set the connection media type
	HRESULT SetMediaType(const CMediaType* mt);

	// --- IMemInputPin -----

	// here's the next block of data from the stream.
	// AddRef it yourself if you need to hold it beyond the end of this call.
	STDMETHODIMP Receive(IMediaSample * pSample);

	// provide EndOfStream that passes straight downstream 
	STDMETHODIMP EndOfStream(void);

	// passes it to CTransformFilter::BeginFlush
	STDMETHODIMP BeginFlush(void);

	// passes it to CTransformFilter::EndFlush
	STDMETHODIMP EndFlush(void);

	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	// Check if it's OK to process samples
	virtual HRESULT CheckStreaming();

	// Allow the filter to see what allocator we have
	// N.B. This does NOT AddRef
	__out IMemAllocator * PeekAllocator() const
	{
		return m_pAllocator;
	}

	// Media type
	CMediaType& CurrentMediaType() { return m_mt; };

protected:
	CTransformExtandFilter *m_pTransformFilter; 
	CAUDIO_S32_t	 m_nIndex; 
	LONG             m_cOurRef; // We maintain reference counting
	bool             m_bIsDataComeIn;

};


class CTransformExtandOutputPin : public CBaseOutputPin,
	public IAudioTransport  
{
    friend class CTransformExtandFilter;

public:
    // implement IMediaPosition by passing upstream
    IUnknown * m_pPosition;

    CTransformExtandOutputPin(TCHAR *pObjectName, CTransformExtandFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t nIndex);
#ifdef UNICODE
    CTransformExtandOutputPin(CHAR *pObjectName, CTransformExtandFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName, CAUDIO_S32_t nIndex);
#endif
    ~CTransformExtandOutputPin();

    // override to expose IMediaPosition
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    // --- CBaseOutputPin ------------
    STDMETHODIMP QueryId(LPWSTR * Id)
    {
        return AMGetWideString(L"Out", Id);
    }

    // Grab and release extra interfaces if required
    HRESULT CheckConnect(IPin *pPin);
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);
    // check that we can support this output type
    HRESULT CheckMediaType(const CMediaType* mtOut);
    // set the connection media type
    HRESULT SetMediaType(const CMediaType *pmt);

    // called from CBaseOutputPin during connection to ask for
    // the count and size of buffers we need.
    HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES *pProp);
	HRESULT SetAllocaterProp(ALLOCATOR_PROPERTIES *pProp);
    // returns the preferred formats for a pin
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

    // inherited from IQualityControl via CBasePin
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

	//! IAudioTransport API
	bool Transport(const void* a_pData, DATA_TYPE a_eDataType, CAUDIO_U32_t a_nSize, TransportId_e a_eTPId);

	HRESULT InitializeOutputSample(IMediaSample **ppOutSample, TransportId_e a_eTPId);

    // Media type
public:
    CMediaType& CurrentMediaType() { return m_mt; }
	HRESULT InitWavFile(std::string file_name, CAUDIO_U32_t sample_rate);
	HRESULT ReleaseWavFile(void);
	HRESULT WriteWavFile(CAUDIO_S16_t *data, CAUDIO_S32_t size);
	CAUDIO_U32_t Ceil(AUDIO_DATA_TYPE data);
	bool IsDataComeIn();

protected:

	short *m_pOutBuffer; 
	int m_nOutBufferSize; 
	int m_nOutBufferWritePtr;

	CTransformExtandFilter *m_pTransformFilter; 
	CAUDIO_S32_t  m_nIndex; 
	ALLOCATOR_PROPERTIES m_pProp;

	CWavFileOp   *m_pWavFileOp;
	SWavFileHead *m_pWavFileHead;
	char         m_pFileName[256];
	CAUDIO_S32_t  m_nWaveCounter;

public:
	CAUDIO_U32_t m_nSampleRate_in;
	CAUDIO_U32_t m_nSampleRate_out;
	CAUDIO_U32_t m_nSampleSize;
	TransportId_e m_eTPId;
};


// Input Pin List
typedef CGenericList <CTransformExtandInputPin> CInputList;

class AM_NOVTABLE CTransformExtandFilter : public CBaseFilter
{
public:
	friend class CTransformExtandInputPin;  //merged from v0.5
	friend class CTransformOriginInputPin;
	friend class CTransformExtandOutputPin;

    CTransformExtandFilter(TCHAR *, LPUNKNOWN, REFCLSID clsid);
#ifdef UNICODE
    CTransformExtandFilter(CHAR *, LPUNKNOWN, REFCLSID clsid);
#endif
    ~CTransformExtandFilter();

    // =================================================================
    // ----- override these bits ---------------------------------------
    // =================================================================
	// map getpin/getpincount for base enum of pins to owner
	// override this to return more specialised pin objects

	virtual int GetPinCount();
	virtual CBasePin * GetPin(int n);
	STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

	// override state changes to allow derived transform filter to control streaming start/stop
	STDMETHODIMP Stop();
	STDMETHODIMP Pause();

    // check if you can support mtIn
	virtual HRESULT CheckInputType(const CMediaType* mtIn,CAUDIO_S32_t nIndex) PURE;
    // check if you can support the transform from this input to this output
    virtual HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut, CAUDIO_S32_t nIndex) PURE;
	// call the SetProperties function with appropriate arguments
    //virtual HRESULT DecideBufferSize( IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES *pprop) PURE;
    // override to suggest OUTPUT pin media types
    virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType, CAUDIO_S32_t nIndex) PURE;

    // =================================================================
    // ----- Optional Override Methods           -----------------------
    // =================================================================

    // you can also override these if you want to know about streaming
    virtual HRESULT StartStreaming();
    virtual HRESULT StopStreaming();

    // override if you can do anything constructive with quality notifications
    virtual HRESULT AlterQuality(Quality q);

    // override this to know when the media type is actually set
    virtual HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt, CAUDIO_S32_t nIndex=0);

    // chance to grab extra interfaces on connection
    virtual HRESULT CheckConnect(PIN_DIRECTION dir,IPin *pPin);
    virtual HRESULT BreakConnect(PIN_DIRECTION dir, CAUDIO_S32_t nIndex);
    virtual HRESULT CompleteConnect(PIN_DIRECTION direction, CAUDIO_S32_t nIndex, IPin *pReceivePin);

    // chance to customize the receive and storage process
	virtual HRESULT Receive(CAUDIO_S32_t nIndex, IMediaSample *pSample);

    // if you override Receive, you may need to override these three too
    virtual HRESULT EndOfStream(CAUDIO_S32_t nIndex);
    virtual HRESULT BeginFlush(void);
    virtual HRESULT EndFlush(void);
    virtual HRESULT NewSegment( REFERENCE_TIME tStart,  REFERENCE_TIME tStop, double dRate);

	BOOL GetSampleProps(AM_SAMPLE2_PROPERTIES** a_ppProps, TransportId_e a_eTPId);


#ifdef PERF
    // Override to register performance measurement with a less generic string
    // You should do this to avoid confusion with other filters
    virtual void RegisterPerfId()
         {m_idTransform = MSR_REGISTER(TEXT("Transform"));}
#endif // PERF


protected:
	// =================================================================
	// ----- Input Pin Processing Function           -------------------
	// =================================================================

	//! check connection status of input pin 
	virtual BOOL _ConnectStatusOfInputPin(CAUDIO_S32_t nIndex = -1);
	//! check connection status of output pin 
	virtual BOOL _ConnectStatusOfOutputPin(void);

#if FILTER_MODE_1_IN_2_OUT
	//! reset on-line class input pin list
	virtual BOOL _ResetOnlineClassInputPinsList(void);
	//! get pin which index is n from on-line class input pins list
	virtual CTransformExtandInputPin* _GetOnlineClassInputPinFromList(CAUDIO_S32_t n);
	//! create a new on-line class input pin
	virtual BOOL _CreateNewOnlineClassInputPin(CTransformExtandFilter *pFilter);
	//! delete a on-line class input pin
	virtual BOOL _DeleteOnlineClassInputPin(CTransformExtandInputPin *pPin);
	//! get free on-line class input pin's count
	virtual CAUDIO_S32_t _GetFreeOnlineClassInputPinCount(void);
#else
	//! reset mic input pin list
	virtual BOOL _ResetMicInputPinsList(void);
	//! get pin which index is n from mic input pins list
	virtual CTransformExtandInputPin* _GetMicInputPinFromList(CAUDIO_S32_t n);
	//! create a new mic input pin
	virtual BOOL _CreateNewMicInputPin(CTransformExtandFilter *pFilter);
	//! delete a mic input pin
	virtual BOOL _DeleteMicInputPin(CTransformExtandInputPin *pPin);
	//! get free mid input pin's count
	virtual CAUDIO_S32_t _GetFreeMicInputPinCount(void);
#endif

	bool IsDataComeIn();

#ifdef PERF
    int							m_idTransform;					// performance measuring id
#endif
    BOOL						m_bEOSDelivered;				// have we sent EndOfStream
    BOOL						m_bSampleSkipped;				// Did we just skip a frame
    BOOL						m_bQualityChanged;				// Have we degraded?

    // critical section protecting filter state.
    CCritSec					m_csFilter;

	// critical section stopping state changes (ie Stop) while we're
	// processing a sample.
	//
	// This critical section is held when processing
	// events that occur on the receive thread - Receive() and EndOfStream().
	//
	// If you want to hold both m_csReceive and m_csFilter then grab
	// m_csFilter FIRST - like CTransformFilter::Stop() does.
	CCritSec					m_csReceive;
// merged from v0.5 begin
 // these hold our input and output pins

#if FILTER_MODE_1_IN_2_OUT
	CTransformExtandOutputPin *m_pOnlineClassOutputPin;  
	CTransformExtandOutputPin *m_pRecordOutputPin;      
	BOOL m_bOnlineClassOutputPin;                          
	BOOL m_bRecordOutputPin;

	CInputList m_onlineClassInputPinList;                
	CAUDIO_S32_t m_nOnlineClassInputPinIndex;            
	CAUDIO_S32_t m_nOnlineClassInputPinCount;	         
	CAUDIO_S32_t m_nOnlineClassInputPinCount_CalcUsed;   
	std::vector<CAUDIO_S32_t> m_vOnlineClassInputPinIndex_StoreUsed;  

#else
	// input - BBT\ZXKT\DZGQ\Mic          output - ZXKT\YYYX\LZ
    CTransformExtandOutputPin *m_ZXKTOutputPin;  // ZXKT output pin
	CTransformExtandOutputPin *m_YYYXOutputPin;  // YYYX output pin
	CTransformExtandOutputPin *m_LZOutputPin;    // LZ output pin
	CTransformOriginInputPin  *m_BBTInputPin;    // BBT input pin
	CTransformOriginInputPin  *m_ZXKTInputPin;   // ZXKT input pin
	CTransformOriginInputPin  *m_DZGQInputPin;   // DZGQ input pin
	BOOL m_bBBTInputPin;                         // connection status of BBT 
	BOOL m_bZXKTInputPin;                        // connection status of ZXKT
	BOOL m_bDZGQInputPin;                        // connection status of DZGQ
	BOOL m_bZXKTOutputPin;
	BOOL m_bYYYZOutputPin;
	BOOL m_bLZOutputPin;
	
	// input - mic 
	CInputList m_MicInputPinList;                // mic input pin list
	CAUDIO_S32_t m_nMicInputPinIndex;            // index of mic input pin 
	CAUDIO_S32_t m_nMicInputPinCount;	         // count of mic input pin
	CAUDIO_S32_t m_nMicInputPinCount_CalcUsed;   // count of mic input pin that only for calculate using
	std::vector<CAUDIO_S32_t> m_vMicInputPinIndex_StoreUsed;   // vector to store index of mic input pins
	/// merged from v0.5 end
#endif
	
};

#endif //_TRANSFRMEXTAND_H_


