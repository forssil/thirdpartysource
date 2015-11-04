/*! \file		AudioProcessingImpl.h
*   \author		Keil
*   \date		2015/3/3
*   \brief		Audio Processing Implement class
*	\history:	add class PlaybackAudioProcessing	ZhongYaozhu	03/26/2015
*				add another mixer in AudioCaptureProcessing for recording function	ZhongYaozhu	04/02/2015
*
*/

#ifndef _AUDIOPROCESS_AUDIOPROCESSINGIMPL_H_
#define _AUDIOPROCESS_AUDIOPROCESSINGIMPL_H_

#include "AudioModuleImplBase.h" 
#include "AudioMixImpl.h"       
#include "DCRemoverImpl.h"
#include "AudiotransportImpl.h"
#include "AudioChannelBase.h"
#include "WaveIO.h"

#define LOG_PROC_IMPL "[AudioProcessingImpl.h]"
class CEchoCancellationInterface;
enum TRANSPORT_MODE
{
	TRANSPORT2CAPTURE = 0,
	TRANSPORT2RENDER  = 1,
	NOTRANSPORT       = 2 
};

class CAudioProcessingImpl
{
public:

	CAudioProcessingImpl(
		IN AudioChannelBase *_audio_channel_,
		IN CAUDIO_U8_t  _thread_num_,  /* microphone number*/
		IN CAUDIO_U8_t  _max_channel_num_,
		IN AUDIO_DATA_TYPE _frame_time_ms_,
		IN CAUDIO_U32_t _fs_,
		IN DATA_TYPE    _input_data_type_,
		IN ChannelId_e _channel_id_,
		IN AUDIO_PROPERTY_PAGE *_property_page_ = NULL);

	virtual ~CAudioProcessingImpl();

public:
	
	virtual bool processData(IN const AudioFrame **a_AudioFrameArray, IN CAUDIO_U8_t a_nArrayLen);

	//! reset function
	virtual bool Reset(void);
	virtual bool __Init();
	virtual bool StopProcess(void){ return true; }
	

protected:
	//! data type conversion
	virtual bool __AudioFrame2AudioDataType(
		const AudioFrame **_frame_buffer_, 
		CAUDIO_U8_t a_nThreadNum,
		AUDIO_DATA_TYPE *_process_buffer_[]);

protected:
	AUDIO_DATA_TYPE* m_ppAudio_buffer[MAX_THREAD_NUM];
	/*---external para---*/
	AudioChannelBase *m_pAudioChannel; 
	CAUDIO_U8_t  m_nThreadNum; 
	CAUDIO_U8_t  m_nMaxChannelNum; 
	DATA_TYPE    m_eInputDataType; 
	CAUDIO_U32_t m_nFs; 
	AUDIO_DATA_TYPE m_nFrameTimeMs; 

	/*---internal para---*/
	CAudioMixImpl   *m_aAudioMixImpl; 
	CDCRemoverImpl  *m_cDCRemoverImpl;
	audio_pro_share *m_aAudioProcDataShared; 
	AUDIO_DATA_TYPE *m_pProcessBuffer; 
	AUDIO_DATA_TYPE *m_pOutBuffer; 
	CAUDIO_U16_t  m_nFrameSizePerChannel; 
	CAUDIO_U32_t  m_nProcessBufferSize; 
	CAUDIO_U32_t  m_nOutBufferSize; 
	bool m_bIsInitSuccess; 
	ChannelId_e m_eChannelId;
	AUDIO_PROPERTY_PAGE *m_pPropertyPage;
};


class PlaybackAudioProcessing : public CAudioProcessingImpl
{
public:

	PlaybackAudioProcessing(
		AudioChannelBase *_audio_channel_,
		CAUDIO_U8_t  _thread_num_,
		CAUDIO_U8_t  _max_channel_num_,
		AUDIO_DATA_TYPE _frame_time_ms_,
		CAUDIO_U32_t _fs_,
		DATA_TYPE    _input_data_type_,
		ChannelId_e _channel_id_,
		CAUDIO_U8_t _NetCaptureThreadNum_);

	virtual ~PlaybackAudioProcessing()
	{
		if(NULL != m_pNetCaptureDataMix)
		{
			delete m_pNetCaptureDataMix;
			m_pNetCaptureDataMix = NULL;
		}
		if (NULL != m_pNetFrameDCRemover)
		{
			delete m_pNetFrameDCRemover;
			m_pNetFrameDCRemover = NULL;
		}
	};

	virtual bool __Init(void);
	virtual bool Reset(void);
	virtual bool processData(IN const AudioFrame **a_AudioFrameArray, IN CAUDIO_U8_t a_nArrayLen);

#if CAPTURE_PLAYBACK_INTEGRATED
	bool processData(IN AudioFrame a_AudioFrameArray[], IN CAUDIO_U8_t a_nArrayLen);
#endif

private:
	CAUDIO_U8_t m_nNetCaptureThreadNum;
	CAUDIO_U8_t m_nShareThreadNum;
	CAudioMixImpl* m_pNetCaptureDataMix;
	CDCRemoverImpl  *m_pNetFrameDCRemover;
};

class CaputureAudioProcessing : public CAudioProcessingImpl
{
public:
	CaputureAudioProcessing(
		AudioChannelBase *_audio_channel_,
		CAUDIO_U8_t  _thread_num_,
		CAUDIO_U8_t  _max_channel_num_,
		AUDIO_DATA_TYPE _frame_time_ms_,
		CAUDIO_U32_t _fs_,
		DATA_TYPE    _input_data_type_,
		ChannelId_e _channel_id_,
		AUDIO_PROPERTY_PAGE *_property_page_,
		bool _bShareEnable_,
		bool _bPlayEnable_,
		bool _bRecordingEnable_,
		std::string _sFolderName_
		);

	virtual ~CaputureAudioProcessing();

	virtual bool __Init(void);

	virtual bool Reset(void);

	virtual bool processData(IN const AudioFrame **a_AudioFrameArray, IN CAUDIO_U8_t a_nArrayLen);

	virtual bool __AudioFrame2AudioDataType(
		const AudioFrame **_frame_buffer_, 
		CAUDIO_U8_t a_nThreadNum,
		AUDIO_DATA_TYPE *_process_buffer_[]);

	bool StopProcess(void);

private:
	void CaptureMix();//sum microphone data
	bool __InitWavFile(IN CAUDIO_U32_t _fs, IN std::string _folder_name);
	bool __ReleaseWavFile(void);
	bool __WriteWavFile(IN audio_pro_share &aShareData, IN CAUDIO_U32_t process_mode);

private:
	bool   m_bShareChEnable;
	bool   m_bPlayChEnalbe;
	bool   m_bRecordingEnable;
	bool   m_bStop;
	
	CEchoCancellationInterface *m_cAEC;

	// for local recording
	CAudioMixImpl   *m_pLocalRecordingMixer;
	AUDIO_DATA_TYPE *m_pRecordingMixOutBuff;
	CAUDIO_U32_t  m_nRrdMixOutBufferSize;
	audio_pro_share* m_pRrdMixShareData;

	CWavFileOp   *m_pWavFileOp;
	SWavFileHead *m_pWavFileHead;
	CAUDIO_S32_t  m_nWaveCounter;

	CWavFileOp   *m_pWavFileOp2;
	SWavFileHead *m_pWavFileHead2;
	CAUDIO_S32_t  m_nWaveCounter2;

	std::string m_sFolderName;
	CAUDIO_U32_t m_fs;

};
#endif //_AUDIOPROCESS_AUDIOPROCESSINGIMPL_H_
