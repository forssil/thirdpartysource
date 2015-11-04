/*! \file   AudioProcessFilter.h
*   \author Keil 
*   \brief  The filter is used here as a data transceiver. It can receive audio stream from input pins, 
			such as ClassToClass¡¢Mic, and then transmit them to internal
			audio process engine, which really performs audio process tasks. At last, the filter will 
			send processed audio steams to different output pins.
*   \history   2014/12/3 created
*/

#ifndef _AUDIO_PROCESS_FILTER_H_
#define _AUDIO_PROCESS_FILTER_H_

#include "IAudioProcess.h"      
#include "IAudioTransport.h" 
#include "IAudioPropertyPage.h"
#include "transfrmextand.h"      
#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <cstdio>
#include <direct.h>

class  IAudioEngine;
class  CWavFileOp;
struct SWavFileHead;

// Specify which channel and which index in that channel the input audio stream should be assigned to.
struct SReceivedSampleInfo
{
	SReceivedSampleInfo(CAUDIO_U8_t thread_id, ChannelId_e channel_id)
	{
		thread_id_  = thread_id;
		channel_id_ = channel_id;
	}
	CAUDIO_U8_t thread_id_;
	ChannelId_e channel_id_;
};


// To save each input/output audio stream all the way to wave files
struct SWavFile
{
	SWavFile()
	{
		m_pWavFileOp = NULL;
		m_pWavFileHead = NULL;
		memset(m_pFileName, 0, sizeof(char)* 256);
		m_nWaveCounter = 0;
	}

	CWavFileOp   *m_pWavFileOp;        // wave file operate class
	SWavFileHead *m_pWavFileHead;      // wave file info head
	char   m_pFileName[256];           // wave file name
	CAUDIO_U32_t  m_nWaveCounter;      // wave counter
};

typedef std::map<CAUDIO_S8_t, SReceivedSampleInfo> MapReceivedSampleInfo;
typedef std::map<std::string, SWavFile> MapWavFileInfo;
typedef std::map<std::string, CAUDIO_U32_t> MapSampleRateInfo;

class CAudioProcessFilter 
	: public CTransformExtandFilter // the filter base class
	, public ISpecifyPropertyPages  // support properties page
	, public IAudioProcess          
	, public IAudioTransport    
	, public IAudioPropertyPage
{	
public:
	//! Factory function, disable constructor
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
	~CAudioProcessFilter(void);

	virtual int GetPinCount();
	virtual CBasePin * GetPin(int n);

	//! We can receive audio streams from input pin, and then transmit them to audio engine.
	virtual HRESULT Receive(CAUDIO_S32_t a_nIndex, IMediaSample* a_pSample);

	//! These four functions must be rewritten.
	virtual HRESULT CheckInputType(const CMediaType* mtIn, CAUDIO_S32_t nIndex); //PURE
	virtual HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut, CAUDIO_S32_t nIndex); //PURE
	virtual HRESULT GetMediaType(int iPosition, CMediaType *pmt, CAUDIO_S32_t nIndex); //PURE

	//! These two functions used here to connect input and output pins to our filter.
	virtual HRESULT CompleteConnect(PIN_DIRECTION direction, CAUDIO_S32_t nIndex, IPin *pReceivePin);
	virtual HRESULT BreakConnect(PIN_DIRECTION dir, CAUDIO_S32_t nIndex);

	//! These four functions used here to control stream.
	virtual HRESULT StartStreaming(void);
	virtual HRESULT StopStreaming(void);
	virtual HRESULT EndOfStream(CAUDIO_S32_t nIndex);
	virtual HRESULT EndFlush();

	//! IAudioPropertyPage API
	virtual bool RegisterProperty(AUDIO_PROPERTY_PAGE *property_page);
	virtual bool UnRegisterProperty();

	DECLARE_IUNKNOWN;
	//! Basic COM - used here to reveal our own interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	STDMETHODIMP Stop();
	STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

	//! ISpecifyPropertyPages API
	STDMETHODIMP GetPages(CAUUID *pPages);

	//! IAudioProcess API
	STDMETHODIMP GetPropertyPage(AUDIO_PROPERTY_PAGE &property_page);
	STDMETHODIMP GetMixState(bool &nomic_inputpin);
	STDMETHODIMP SetAecState(bool aec_switch = true);
	STDMETHODIMP SetSampleRate(SSampleRate sample_rate);
	STDMETHODIMP GetFilterState(void);
	//merged from v0.5
	STDMETHODIMP GetAudioChannelNumber(bool &nomic_inputpin);
	
private:
	HRESULT __InitAudioProcessFilter(void);
	HRESULT __ReleaseAudioProcessFilter(void);
	HRESULT __InitStreaming(void);
	HRESULT __ReleaseStreaming(void);
	HRESULT __InitWavFile(void);
	HRESULT __CreateWavFile(const char *folder_name, const char *file_name, CTransformOriginInputPin *input_pin);
	HRESULT __CreateWavFile(const char *folder_name, const char *file_name, CTransformExtandInputPin *input_pin);
	HRESULT __ReleaseWavFile(void);
	HRESULT __WriteWavFile(CAUDIO_S16_t *data, CAUDIO_S32_t size, std::string name);
	HRESULT __GetOriginalSampleRate(std::string name, CAUDIO_U32_t &sample_rate);
	HRESULT __GetPinInfo(void);
	HRESULT __Fix2Float(const CAUDIO_S16_t* buffer_in, const CAUDIO_U32_t len_in);
	HRESULT __CheckProperties(ALLOCATOR_PROPERTIES *pProp);

	//! Disable the constructor, copy constructor, assignment function
	CAudioProcessFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
	CAudioProcessFilter(IN const CAudioProcessFilter &rhs);
	CAudioProcessFilter& operator= (IN const CAudioProcessFilter &rhs);

	std::string m_pFolderName;
	CAUDIO_S32_t m_nChannelNum;
	CAUDIO_S32_t m_nInputPinFactNum; 

	IAudioEngine* m_pAudioEngine; 
	AUDIO_PROPERTY_PAGE *m_pPropertyPage;
	bool m_bAecSwitch;
	//merged from v0.5
	bool *m_pEOSFlag;

	AUDIO_TYPE_NUM_t m_sAudio_type_num;
	SSampleRate m_sSampleRate;
	MapSampleRateInfo m_mInputSampleRate;
	MapWavFileInfo m_mWaveFileInfo;
	MapReceivedSampleInfo m_mReceivedSampleInfo;

	AUDIO_DATA_TYPE *m_pInBuffer_FeedData;
	CAUDIO_U32_t m_nInBufferSize_FeedData;

};
#endif //_AUDIO_PROCESS_FILTER_H_

