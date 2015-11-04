/*! \file		AudioProcessingImpl.cpp
*   \author		Keil
*   \date		2015/3/3
*   \brief		Audio Processing Implement class
*   \history:	add class PlaybackAudioProcessing	ZhongYaozhu	03/26/2015
*				add another mixer in AudioCaptureProcessing for recording function	ZhongYaozhu	04/02/2015
*/

#include "AudioProcessingImpl.h"  
#include "AudioCaptureChannel.h"
#include "echocancellation_interface.h"

CAudioProcessingImpl::CAudioProcessingImpl(
	IN AudioChannelBase *_audio_channel_,
	IN CAUDIO_U8_t  _thread_num_,
	IN CAUDIO_U8_t  _max_channel_num_,
	IN AUDIO_DATA_TYPE _frame_time_ms_,
	IN CAUDIO_U32_t _fs_,
	IN DATA_TYPE    _input_data_type_,
	IN ChannelId_e _channel_id_,
	IN AUDIO_PROPERTY_PAGE *_property_page_)
	: m_pAudioChannel(_audio_channel_)
	, m_nThreadNum(_thread_num_)
	, m_nMaxChannelNum(_max_channel_num_)
	, m_nFrameTimeMs(_frame_time_ms_)
	, m_nFs(_fs_)
	, m_eInputDataType(_input_data_type_)
	, m_nFrameSizePerChannel(FRAM_LEN(_frame_time_ms_, _fs_))
	, m_nProcessBufferSize(FRAM_LEN(_frame_time_ms_, _fs_)*_thread_num_*_max_channel_num_)
	, m_nOutBufferSize(_fs_)
	, m_bIsInitSuccess(false)
	, m_pProcessBuffer(NULL)
	, m_pOutBuffer(NULL)
	, m_aAudioProcDataShared(NULL)
	, m_aAudioMixImpl(NULL)
	, m_cDCRemoverImpl(NULL)
	, m_eChannelId(_channel_id_)
	, m_pPropertyPage(_property_page_)
{
}

CAudioProcessingImpl::~CAudioProcessingImpl()
{
	if (m_pProcessBuffer)
	{
		delete[] m_pProcessBuffer;
		m_pProcessBuffer = NULL;
	}
	if (m_pOutBuffer)
	{
		delete[] m_pOutBuffer;
		m_pOutBuffer = NULL;
	}
	if (m_aAudioMixImpl)
	{
		delete m_aAudioMixImpl;
		m_aAudioMixImpl = NULL;
	}
	if (m_cDCRemoverImpl)
	{
		delete m_cDCRemoverImpl;
		m_cDCRemoverImpl = NULL;
	}
	if (m_aAudioProcDataShared)
	{
		delete m_aAudioProcDataShared;
		m_aAudioProcDataShared = NULL;
	}
	m_bIsInitSuccess = false;
}

bool CAudioProcessingImpl::__Init()
{
	if (m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("%s init operation failed", LOG_PROC_IMPL);
		return false;
	}
	if (m_pProcessBuffer)
	{
		delete[] m_pProcessBuffer;
		m_pProcessBuffer = NULL;
	}
	m_pProcessBuffer = new AUDIO_DATA_TYPE[m_nProcessBufferSize];
	memset(m_pProcessBuffer, 0, m_nProcessBufferSize*sizeof(AUDIO_DATA_TYPE));
	if (NULL == m_pProcessBuffer)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_PROC_IMPL);
		return false;
	}
	if (m_pOutBuffer)
	{
		delete[] m_pOutBuffer;
		m_pOutBuffer = NULL;
	}
	m_pOutBuffer = new AUDIO_DATA_TYPE[m_nOutBufferSize];
	memset(m_pOutBuffer, 0, m_nOutBufferSize*sizeof(AUDIO_DATA_TYPE));
	if (NULL == m_pOutBuffer)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_PROC_IMPL);
		return false;
	}
	if (m_aAudioProcDataShared)
	{
		delete m_aAudioProcDataShared;
		m_aAudioProcDataShared = NULL;
	}
	m_aAudioProcDataShared = new audio_pro_share;
	memset(m_aAudioProcDataShared, 0, sizeof(audio_pro_share));
	if (NULL == m_aAudioProcDataShared)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_PROC_IMPL);
		return false;
	}
	if (m_aAudioMixImpl)
	{
		delete m_aAudioMixImpl;
		m_aAudioMixImpl = NULL;
	}
	m_aAudioMixImpl = new CAudioMixImpl(m_nThreadNum, m_nFs, m_nFrameTimeMs);

	if (NULL == m_aAudioMixImpl)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed: m_aAudioMixImpl", LOG_PROC_IMPL);
		return false;
	}

	// alloc CDCRemoverImpl
	if (m_cDCRemoverImpl)
	{
		delete m_cDCRemoverImpl;
		m_cDCRemoverImpl = NULL;
	}
	m_cDCRemoverImpl = new CDCRemoverImpl(m_nFs, m_nFrameSizePerChannel, m_nThreadNum);
	if (NULL == m_cDCRemoverImpl)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed: m_cDCRemoverImpl", LOG_PROC_IMPL);
		return false;
	}

	m_bIsInitSuccess = true;
	return true;
}

bool CAudioProcessingImpl::Reset(void)
{
	if (m_pProcessBuffer)
	{
		memset(m_pProcessBuffer, 0, m_nProcessBufferSize*sizeof(AUDIO_DATA_TYPE));
	}

	if (m_pOutBuffer)
	{
		memset(m_pOutBuffer, 0, m_nOutBufferSize*sizeof(AUDIO_DATA_TYPE));
	}

	//if (m_aAudioProcDataShared)
	//{
	//	memset(m_aAudioProcDataShared, 0, sizeof(audio_pro_share)); 
	//}

	if (m_aAudioMixImpl)
	{
		m_aAudioMixImpl->Reset();
	}

	return true;
}

bool CAudioProcessingImpl::__AudioFrame2AudioDataType(
	const AudioFrame **_frame_buffer_, 
	CAUDIO_U8_t a_nThreadNum, 
	AUDIO_DATA_TYPE *_process_buffer_[])
{
	CAUDIO_S16_t *frame_buffer_begin_pos   = NULL; 
	AUDIO_DATA_TYPE *process_buffer_begin_pos = NULL; 
	CAUDIO_U8_t frame_buffer_channel_num = 0; 
	CAUDIO_U8_t thread_ind  = 0; 
	CAUDIO_U32_t sample_per_channel = 0; 
	CAUDIO_U32_t i = 0; 

	for (; thread_ind < a_nThreadNum; ++thread_ind)
	{
		sample_per_channel = 0;
		if (NULL == _frame_buffer_[thread_ind])
		{
			AUDIO_PROCESSING_PRINTF("%s input para error", LOG_PROC_IMPL);
			return false;
		}

		sample_per_channel = _frame_buffer_[thread_ind]->AudioPara_.samples_per_channel_;
		assert(sample_per_channel == m_nFrameSizePerChannel);

		frame_buffer_begin_pos   = (CAUDIO_S16_t*)_frame_buffer_[thread_ind]->data_;
		process_buffer_begin_pos = m_pProcessBuffer + sample_per_channel*m_nMaxChannelNum*thread_ind;
		frame_buffer_channel_num = _frame_buffer_[thread_ind]->AudioPara_.num_channels_;

		if(REAL_FIX_DATA == _frame_buffer_[thread_ind]->AudioPara_.datatype_)
		{
			for (i = 0; i < sample_per_channel; ++i)
			{
				// NOTE: only mix left channel data
				process_buffer_begin_pos[i] = (AUDIO_DATA_TYPE)(frame_buffer_begin_pos[i*frame_buffer_channel_num] / 32768.f);
			}
		}
		else if(REAL_FLOAT_DATA == _frame_buffer_[thread_ind]->AudioPara_.datatype_)
		{
			memcpy_s(process_buffer_begin_pos, 
				sample_per_channel*m_nMaxChannelNum*DataType2Byte(REAL_FLOAT_DATA), 
				frame_buffer_begin_pos, 
				DataType2Byte(REAL_FLOAT_DATA)*sample_per_channel);
		}
		else
		{
			AUDIO_PROCESSING_PRINTF("unreasonable data type in CAudioProcessingImpl::__AudioFrame2AudioDataType");
			return false;
		}

		_process_buffer_[thread_ind] = process_buffer_begin_pos;
	}
	return true;
}

/*virtual */bool CAudioProcessingImpl::processData(IN const AudioFrame **a_AudioFrameArray, IN CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("%s init operation failed", LOG_PROC_IMPL);
		return false;
	}

	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("%s input para error", LOG_PROC_IMPL);
		return false;
	}

	AUDIO_DATA_TYPE* audio_buffer[MAX_THREAD_NUM];
	if (!__AudioFrame2AudioDataType(a_AudioFrameArray, m_nThreadNum, audio_buffer))
	{
		AUDIO_PROCESSING_PRINTF("%s __AudioFrame2AudioDataType operation failed!", LOG_PROC_IMPL);
		return false;
	}

	if (static_cast<CAUDIO_U32_t>(m_nFrameSizePerChannel) > m_nOutBufferSize)
	{
		AUDIO_PROCESSING_PRINTF("%s memory size error!", LOG_PROC_IMPL);
		return false;
	}

	m_aAudioProcDataShared->pCapture_ = audio_buffer;
	m_aAudioProcDataShared->nChannelsInCapture_ = m_nThreadNum;
	m_aAudioProcDataShared->nSamplesPerCaptureChannel_ = m_nFrameSizePerChannel;

	if (m_cDCRemoverImpl)
	{
		int ret = m_cDCRemoverImpl->process(*m_aAudioProcDataShared);
		if (0 != ret)
		{
			AUDIO_PROCESSING_PRINTF("%s m_cDCRemoverImpl process operation failed!", LOG_PROC_IMPL);
			return false;
		}
	}
	m_aAudioProcDataShared->pMixIn_ = audio_buffer;
	m_aAudioProcDataShared->nChannelsInMixIn_ = m_nThreadNum;
	m_aAudioProcDataShared->nSamplesPerMixInChannel_ = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pMixOut_ = m_pOutBuffer;
	m_aAudioProcDataShared->nLenOfMixOut_ = m_nOutBufferSize;
	if (m_aAudioMixImpl)
	{
		m_aAudioMixImpl->process(*m_aAudioProcDataShared);
	}

	if (!m_pAudioChannel->Transport(m_pOutBuffer, REAL_FLOAT_DATA, m_nFrameSizePerChannel, InvalidTP))
	{
		AUDIO_PROCESSING_PRINTF("%s Send operation error", LOG_PROC_IMPL);
		return false;
	}

	return true;
}

PlaybackAudioProcessing::PlaybackAudioProcessing(
	AudioChannelBase *_audio_channel_,
	CAUDIO_U8_t  _thread_num_,
	CAUDIO_U8_t  _max_channel_num_,
	AUDIO_DATA_TYPE _frame_time_ms_,
	CAUDIO_U32_t _fs_,
	DATA_TYPE    _input_data_type_,
	ChannelId_e _channel_id_,
	CAUDIO_U8_t _NetCaptureThreadNum_)
	:
	CAudioProcessingImpl(
	_audio_channel_,
	_thread_num_,
	_max_channel_num_,
	_frame_time_ms_,
	_fs_,
	_input_data_type_,
	_channel_id_),
	m_nNetCaptureThreadNum(_NetCaptureThreadNum_),
	m_nShareThreadNum(_thread_num_ - _NetCaptureThreadNum_),
	m_pNetCaptureDataMix(NULL),
	m_pNetFrameDCRemover(NULL)
{
}

bool PlaybackAudioProcessing::__Init(void)
{
	if (m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init failed");
		return false;
	}

	if (NULL != m_pNetCaptureDataMix)
	{
		delete m_pNetCaptureDataMix;
		m_pNetCaptureDataMix = NULL;
	}
	m_pNetCaptureDataMix = new CAudioMixImpl(m_nNetCaptureThreadNum, m_nFs, m_nFrameTimeMs);
	if (NULL == m_pNetCaptureDataMix)
	{
		AUDIO_PROCESSING_PRINTF("new m_pNetCaptureDataMix failed");
		return false;
	}

	if (NULL != m_pNetFrameDCRemover)
	{
		delete m_pNetFrameDCRemover;
		m_pNetFrameDCRemover = NULL;
	}
	m_pNetFrameDCRemover = new CDCRemoverImpl(m_nFs, m_nFrameSizePerChannel, m_nNetCaptureThreadNum);
	if (NULL == m_pNetFrameDCRemover)
	{
		AUDIO_PROCESSING_PRINTF("new m_pNetRemover failed");
		return false;
	}

	return CAudioProcessingImpl::__Init();
}

bool PlaybackAudioProcessing::Reset(void)
{
	if (NULL != m_pNetCaptureDataMix)
	{
		m_pNetCaptureDataMix->Reset();
	}

	return CAudioProcessingImpl::Reset();
}

bool PlaybackAudioProcessing::processData(IN const AudioFrame **a_AudioFrameArray, IN CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init failed");
		return false;
	}

	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("sanity check failed");
		return false;
	}

	const AudioFrame *networkAudioFrameArray[MAX_THREAD_NUM];
	const AudioFrame *renderAudioFrameArray[MAX_THREAD_NUM];
	AUDIO_DATA_TYPE* audio_buffer[MAX_THREAD_NUM];
	CAUDIO_U8_t netAudioFrameIdx = 0;
	CAUDIO_U8_t renderAudioFrameIdx = 0;
	for (CAUDIO_U8_t idx = 0; idx < m_nThreadNum; ++idx)
	{
		const AudioFrame * pFrame = a_AudioFrameArray[idx];
		if (NULL != pFrame)
		{
			if (DSFromNetwork == pFrame->AudioPara_.dataSyncId_)
			{
				networkAudioFrameArray[netAudioFrameIdx] = pFrame;
				netAudioFrameIdx++;
			}
			renderAudioFrameArray[renderAudioFrameIdx] = pFrame;
			renderAudioFrameIdx++;

		}
	}
	assert(netAudioFrameIdx == m_nNetCaptureThreadNum);
	
	/* 2015-9-15 by Keil. 
	TODO: for now we transmit mixing data that hybrid playback channel signal and share channel signal to capture channel. 

	if (!__AudioFrame2AudioDataType(networkAudioFrameArray, m_nNetCaptureThreadNum, audio_buffer))
	{
		AUDIO_PROCESSING_PRINTF("audio data type casting failed");
		return false;
	}

	if (static_cast<CAUDIO_U32_t>(m_nFrameSizePerChannel) > m_nOutBufferSize)
	{
		AUDIO_PROCESSING_PRINTF("frame size per channel is too large");
		return false;
	}

	m_aAudioProcDataShared->pRender_ = audio_buffer;
	m_aAudioProcDataShared->nChannelsInRender_ = m_nNetCaptureThreadNum;
	m_aAudioProcDataShared->nSamplesPerRenderChannel_ = m_nFrameSizePerChannel;
	if (NULL != m_pNetFrameDCRemover)
	{
		if (0 != m_pNetFrameDCRemover->process(*m_aAudioProcDataShared))
		{
			AUDIO_PROCESSING_PRINTF("remove DC failed");
			return false;
		}
	}

	// Mix data from net side together, then transport it to capture channel for AEC
	m_aAudioProcDataShared->pMixIn_ = audio_buffer;
	m_aAudioProcDataShared->nChannelsInMixIn_ = m_nNetCaptureThreadNum;
	m_aAudioProcDataShared->nSamplesPerMixInChannel_ = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pMixOut_ = m_pOutBuffer;
	m_aAudioProcDataShared->nLenOfMixOut_ = m_nOutBufferSize;

	if (NULL != m_pNetCaptureDataMix)
	{
		m_pNetCaptureDataMix->process(*m_aAudioProcDataShared);
	}
	
	*/

	// Mix data from net side and share side together, then transport it to render to playback.
	if (!__AudioFrame2AudioDataType(renderAudioFrameArray, m_nNetCaptureThreadNum + m_nShareThreadNum, audio_buffer))
	{
		AUDIO_PROCESSING_PRINTF("%s __AudioFrame2AudioDataType operation failed!", LOG_PROC_IMPL);
		return false;
	}
	if (static_cast<CAUDIO_U32_t>(m_nFrameSizePerChannel) > m_nOutBufferSize)
	{
		AUDIO_PROCESSING_PRINTF("frame size per channel is too large");
		return false;
	}
	m_aAudioProcDataShared->pRender_ = audio_buffer;
	m_aAudioProcDataShared->nSamplesPerRenderChannel_ = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->nChannelsInRender_ = m_nNetCaptureThreadNum + m_nShareThreadNum;

	if (m_cDCRemoverImpl)
	{
		int ret = m_cDCRemoverImpl->process(*m_aAudioProcDataShared);
		if (0 != ret)
		{
			AUDIO_PROCESSING_PRINTF("%s m_cDCRemoverImpl process operation failed!", LOG_PROC_IMPL);
			return false;
		}
	}
	// Mix data from net side together, then transport it to capture channel for AEC
	m_aAudioProcDataShared->pMixIn_ = audio_buffer;
	m_aAudioProcDataShared->nSamplesPerMixInChannel_ = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pMixOut_ = m_pOutBuffer;
	m_aAudioProcDataShared->nLenOfMixOut_ = m_nOutBufferSize;
	m_aAudioProcDataShared->nChannelsInMixIn_ = m_nNetCaptureThreadNum + m_nShareThreadNum;
	if (NULL != m_aAudioMixImpl)
	{
		m_aAudioMixImpl->process(*m_aAudioProcDataShared);
	}

	if (!m_pAudioChannel->Transport(m_pOutBuffer, REAL_FLOAT_DATA, m_nFrameSizePerChannel, TP2CaptureCh))
	{
		AUDIO_PROCESSING_PRINTF("transport data to capture channel failed");
		return false;
	}
	if (!m_pAudioChannel->Transport(m_pOutBuffer, REAL_FLOAT_DATA, m_nFrameSizePerChannel, TP2RenderDev))
	{
		AUDIO_PROCESSING_PRINTF("Send operation error");
		return false;
	}

	return true;
}

#if CAPTURE_PLAYBACK_INTEGRATED
bool PlaybackAudioProcessing::processData(IN AudioFrame a_AudioFrameArray[], IN CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("init failed");
		return false;
	}
	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("sanity check failed");
		return false;
	}

	const AudioFrame *networkAudioFrameArray[MAX_THREAD_NUM];
	const AudioFrame *renderAudioFrameArray[MAX_THREAD_NUM];
	AUDIO_DATA_TYPE* audio_buffer[MAX_THREAD_NUM];
	CAUDIO_U8_t netAudioFrameIdx = 0;
	CAUDIO_U8_t renderAudioFrameIdx = 0;
	for (CAUDIO_U8_t idx = 0; idx < m_nThreadNum; ++idx)
	{
		if (DSFromNetwork == a_AudioFrameArray[idx].AudioPara_.dataSyncId_)
		{
			networkAudioFrameArray[netAudioFrameIdx] = &a_AudioFrameArray[idx];
			netAudioFrameIdx++;
		}
		renderAudioFrameArray[renderAudioFrameIdx] = &a_AudioFrameArray[idx];
		renderAudioFrameIdx++;
	}
	assert(netAudioFrameIdx == m_nNetCaptureThreadNum);

	// Mix data from net side and share side together, then transport it to render to playback.
	if (!__AudioFrame2AudioDataType(renderAudioFrameArray, m_nNetCaptureThreadNum + m_nShareThreadNum, audio_buffer))
	{
		AUDIO_PROCESSING_PRINTF("%s __AudioFrame2AudioDataType operation failed!", LOG_PROC_IMPL);
		return false;
	}
	if (static_cast<CAUDIO_U32_t>(m_nFrameSizePerChannel) > m_nOutBufferSize)
	{
		AUDIO_PROCESSING_PRINTF("frame size per channel is too large");
		return false;
	}
	m_aAudioProcDataShared->pRender_ = audio_buffer;
	m_aAudioProcDataShared->nSamplesPerRenderChannel_ = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->nChannelsInRender_ = m_nNetCaptureThreadNum + m_nShareThreadNum;

	if (m_cDCRemoverImpl)
	{
		int ret = m_cDCRemoverImpl->process(*m_aAudioProcDataShared);
		if (0 != ret)
		{
			AUDIO_PROCESSING_PRINTF("%s m_cDCRemoverImpl process operation failed!", LOG_PROC_IMPL);
			return false;
		}
	}
	// Mix data from net side together, then transport it to capture channel for AEC
	m_aAudioProcDataShared->pMixIn_ = audio_buffer;
	m_aAudioProcDataShared->nSamplesPerMixInChannel_ = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pMixOut_ = m_pOutBuffer;
	m_aAudioProcDataShared->nLenOfMixOut_ = m_nOutBufferSize;
	m_aAudioProcDataShared->nChannelsInMixIn_ = m_nNetCaptureThreadNum + m_nShareThreadNum;
	if (NULL != m_aAudioMixImpl)
	{
		m_aAudioMixImpl->process(*m_aAudioProcDataShared);
	}

	// transport data to capture channel 
	if (!m_pAudioChannel->Transport(m_pOutBuffer, REAL_FLOAT_DATA, m_nFrameSizePerChannel, TP2CaptureCh))
	{
		AUDIO_PROCESSING_PRINTF("transport data to capture channel failed");
		return false;
	}

	// TODO: remove copy operation if we want to optimize
	// copy data back to a_AudioFrameArray[0]
	memcpy_s(a_AudioFrameArray[0].data_, 
		AudioFrame::kMaxDataSizeSamples, 
		m_pOutBuffer, 
		m_nFrameSizePerChannel*DataType2Byte(m_eInputDataType));
	a_AudioFrameArray[0].AudioPara_.num_channels_ = 1;
	a_AudioFrameArray[0].AudioPara_.samples_per_channel_ = m_nFrameSizePerChannel;

	return true;
}
#endif

CaputureAudioProcessing::CaputureAudioProcessing(
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
	std::string _sFolderName_)
	: 
	CAudioProcessingImpl(
		_audio_channel_,
		_thread_num_,
		_max_channel_num_,
		_frame_time_ms_,
		_fs_,
		_input_data_type_,
		_channel_id_,
		_property_page_)

	, m_bShareChEnable(_bShareEnable_)
	, m_bPlayChEnalbe(_bPlayEnable_)
	, m_cAEC(NULL)
	, m_pRecordingMixOutBuff(NULL)
	, m_nRrdMixOutBufferSize(m_nFrameSizePerChannel)
	, m_pRrdMixShareData(NULL)
	, m_pLocalRecordingMixer(NULL)
	, m_bStop(false)
	, m_pWavFileOp(NULL)
	, m_pWavFileHead(NULL)
	, m_nWaveCounter(0)
	, m_pWavFileOp2(NULL)
	, m_pWavFileHead2(NULL)
	, m_nWaveCounter2(0)
	, m_bRecordingEnable(_bRecordingEnable_)
	, m_sFolderName(_sFolderName_)
	, m_fs(_fs_)
{
}

CaputureAudioProcessing::~CaputureAudioProcessing()
{
	if(NULL != m_aAudioProcDataShared->pMixIn_)	
	{
		delete m_aAudioProcDataShared->pMixIn_;
	}
	if(NULL != m_cAEC)
	{
		DeleteIAECInst(m_cAEC);
		m_cAEC=NULL;
	}
	if(NULL != m_pLocalRecordingMixer)
	{
		delete m_pLocalRecordingMixer;
		m_pLocalRecordingMixer = NULL;
	}
	if(NULL != m_pRecordingMixOutBuff)
	{
		delete[] m_pRecordingMixOutBuff;
		m_pRecordingMixOutBuff = NULL;
	}
	if(NULL != m_pRrdMixShareData)
	{
		if(NULL != m_pRrdMixShareData->pMixIn_)
		{
			delete[] m_pRrdMixShareData;
			m_pRrdMixShareData = NULL;
		}
		delete m_pRrdMixShareData;
		m_pRrdMixShareData = NULL;
	}
}

bool CaputureAudioProcessing::__Init(void)
{
	if (m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("%s init operation failed", LOG_PROC_IMPL);
		return false;
	}

	m_nProcessBufferSize=m_nFrameSizePerChannel*(MAX_THREAD_NUM)*m_nMaxChannelNum;
	if (m_pProcessBuffer)
	{
		delete[] m_pProcessBuffer;
		m_pProcessBuffer = NULL;
	}
	m_pProcessBuffer = new AUDIO_DATA_TYPE[m_nProcessBufferSize];
	memset(m_pProcessBuffer, 0, m_nProcessBufferSize*sizeof(AUDIO_DATA_TYPE));
	if (NULL == m_pProcessBuffer)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_PROC_IMPL);
		return false;
	}

	for(int i=0; i<MAX_THREAD_NUM;i++)
	{
		m_ppAudio_buffer[i]=m_pProcessBuffer+i*m_nFrameSizePerChannel*m_nMaxChannelNum;
	}

	if (m_pOutBuffer)
	{
		delete[] m_pOutBuffer;
		m_pOutBuffer = NULL;
	}
	m_pOutBuffer = new AUDIO_DATA_TYPE[m_nOutBufferSize];
	memset(m_pOutBuffer, 0, m_nOutBufferSize*sizeof(AUDIO_DATA_TYPE));
	if (NULL == m_pOutBuffer)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_PROC_IMPL);
		return false;
	}
////////////////////////////////
	
	if (m_aAudioProcDataShared)
	{
		delete m_aAudioProcDataShared;
		m_aAudioProcDataShared = NULL;
	}
	m_aAudioProcDataShared = new audio_pro_share;
	memset(m_aAudioProcDataShared, 0, sizeof(audio_pro_share));

	if (NULL == m_aAudioProcDataShared)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed", LOG_PROC_IMPL);
		return false;
	}

	m_aAudioProcDataShared->pCapture_		           = m_ppAudio_buffer;
	m_aAudioProcDataShared->nChannelsInCapture_        = m_nThreadNum;
	m_aAudioProcDataShared->nSamplesPerCaptureChannel_ = m_nFrameSizePerChannel;
	if (m_bPlayChEnalbe)
	{
			m_aAudioProcDataShared->nChannelsInRender_ = 1;
			m_aAudioProcDataShared->bAECOn_ = m_pPropertyPage->pAudioAECInfo_->bAECOn_;
			m_aAudioProcDataShared->bNROn_ = m_pPropertyPage->pAudioAECInfo_->bNROn_;
	}
	else
	{
			m_aAudioProcDataShared->nChannelsInRender_ = 0;
			m_aAudioProcDataShared->bAECOn_ = false;
			m_aAudioProcDataShared->bNROn_ = false;
	}
	m_aAudioProcDataShared->pRender_				   = m_ppAudio_buffer + MAX_THREAD_NUM - 2;
	m_aAudioProcDataShared->nSamplesPerRenderChannel_  = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pShare_                    = m_aAudioProcDataShared->pRender_+1;

	if (m_bShareChEnable)
	{
		m_aAudioProcDataShared->nChannelsInShare_         = 1;
	}
	else
	{
		m_aAudioProcDataShared->nChannelsInShare_         = 0;
	}
	m_aAudioProcDataShared->nSamplesPerShareChannel_   = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pDesire_                   = m_aAudioProcDataShared->pCapture_[0];
	m_aAudioProcDataShared->nSamplesInDesire_          = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pReffer_                   = m_aAudioProcDataShared->pRender_[0];
	m_aAudioProcDataShared->nSamplesInReffer_          = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pError_                    = m_aAudioProcDataShared->pDesire_ ;
	m_aAudioProcDataShared->pMixIn_                    = new AUDIO_DATA_TYPE*[1+m_aAudioProcDataShared->nChannelsInShare_];
	m_aAudioProcDataShared->pMixIn_[0]                 = m_aAudioProcDataShared->pError_ ;
	for(CAUDIO_U8_t i=0; i<m_aAudioProcDataShared->nChannelsInShare_;i++)
		m_aAudioProcDataShared->pMixIn_[i+1] = m_aAudioProcDataShared->pShare_[i];
	m_aAudioProcDataShared->nChannelsInMixIn_ = 1+m_aAudioProcDataShared->nChannelsInShare_;
	m_aAudioProcDataShared->nSamplesPerMixInChannel_ = m_nFrameSizePerChannel;
	m_aAudioProcDataShared->pMixOut_                 = m_pOutBuffer;
	m_aAudioProcDataShared->nLenOfMixOut_            = m_nOutBufferSize;

#ifdef AUDIO_WAVE_DEBUG
	m_aAudioProcDataShared->pErrorBeforeNR_ = m_aAudioProcDataShared->pCapture_[1];
#endif

	if (m_aAudioMixImpl)
	{
		delete m_aAudioMixImpl;
		m_aAudioMixImpl = NULL;
	}
	m_aAudioMixImpl = new CAudioMixImpl(1 + m_aAudioProcDataShared->nChannelsInShare_, m_nFs, m_nFrameTimeMs, m_pPropertyPage);
	if (NULL == m_aAudioMixImpl)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed: m_aAudioMixImpl", LOG_PROC_IMPL);
		return false;
	}

	// alloc CDCRemoverImpl
	if (m_cDCRemoverImpl)
	{
		delete m_cDCRemoverImpl;
		m_cDCRemoverImpl = NULL;
	}
	m_cDCRemoverImpl = new CDCRemoverImpl(m_nFs, m_nFrameSizePerChannel, m_aAudioProcDataShared->nChannelsInCapture_+m_aAudioProcDataShared->nChannelsInRender_+m_aAudioProcDataShared->nChannelsInShare_);
	if (NULL == m_cDCRemoverImpl)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed: m_cDCRemoverImpl", LOG_PROC_IMPL);
		return false;
	}

	m_cAEC= CreateIAECInst_int(m_nFs,2*m_nFrameSizePerChannel,m_nFrameSizePerChannel);

	if(NULL == m_cAEC)
	{
		AUDIO_PROCESSING_PRINTF("%s new operation failed: m_cAEC", LOG_PROC_IMPL);
		return false;
	}
	m_cAEC->Init();
	m_cAEC->SetDelay(0);

	// alloc buffer for mixing for recording
	if(m_bRecordingEnable)
	{
		if (m_pRecordingMixOutBuff)
		{
			delete[] m_pRecordingMixOutBuff;
			m_pRecordingMixOutBuff = NULL;
		}
		m_pRecordingMixOutBuff = new AUDIO_DATA_TYPE[m_nRrdMixOutBufferSize];
		memset(m_pRecordingMixOutBuff, 0, m_nRrdMixOutBufferSize*sizeof(AUDIO_DATA_TYPE));
		if (NULL == m_pRecordingMixOutBuff)
		{
			AUDIO_PROCESSING_PRINTF("alloc recording mix out buffer failed");
			return false;
		}


		//share data for recording mix
		if (NULL != m_pRrdMixShareData)
		{
			if(NULL != m_pRrdMixShareData->pMixIn_)
			{
				delete[] m_pRrdMixShareData->pMixIn_;
				m_pRrdMixShareData->pMixIn_ = NULL;
			}

			delete m_pRrdMixShareData;
			m_pRrdMixShareData = NULL;
		}
		m_pRrdMixShareData = new audio_pro_share;
		if(NULL == m_pRrdMixShareData)
		{
			AUDIO_PROCESSING_PRINTF("new recording shared data failed");
			return false;
		}
		memset(m_pRrdMixShareData, 0, sizeof(audio_pro_share));

		CAUDIO_U8_t recordingMixChannelNum = 1 + m_aAudioProcDataShared->nChannelsInRender_;
		m_pRrdMixShareData->pMixIn_ = new AUDIO_DATA_TYPE*[recordingMixChannelNum];
		if(NULL == m_pRrdMixShareData->pMixIn_)
		{
			AUDIO_PROCESSING_PRINTF("new mix in buffer in recording shared data failed");
			return false;
		}
		memset(m_pRrdMixShareData->pMixIn_, 0, sizeof(AUDIO_DATA_TYPE*)*recordingMixChannelNum);

		// assign accordingly mix params
		m_pRrdMixShareData->nSamplesPerMixInChannel_ = m_aAudioProcDataShared->nSamplesPerMixInChannel_;
		m_pRrdMixShareData->nChannelsInMixIn_ = m_aAudioProcDataShared->nChannelsInRender_ + 1;
		m_pRrdMixShareData->nLenOfMixOut_ = m_nRrdMixOutBufferSize;
		m_pRrdMixShareData->pMixOut_ = m_pRecordingMixOutBuff;
		//m_pRrdMixShareData->pMixIn_[0] = m_pOutBuffer;
		//TODO : mix refer and error in recording.
		m_pRrdMixShareData->pMixIn_[0] = m_aAudioProcDataShared->pError_;
		m_pRrdMixShareData->pMixIn_[1] = m_aAudioProcDataShared->pReffer_;
		//for(CAUDIO_U8_t i=0; i<m_aAudioProcDataShared->nChannelsInRender_; ++i)
		//{
		//	m_pRrdMixShareData->pMixIn_[i+1] = m_aAudioProcDataShared->pRender_[i];
		//}
	
		// alloc another mixer for local recording
		if (NULL != m_pLocalRecordingMixer)
		{
			delete m_pLocalRecordingMixer;
			m_pLocalRecordingMixer = NULL;
		}

		m_pLocalRecordingMixer = new CAudioMixImpl(recordingMixChannelNum, m_nFs, m_nFrameTimeMs);
		if (NULL == m_pLocalRecordingMixer)
		{
			AUDIO_PROCESSING_PRINTF("alloc local recording mixer failed");
			return false;
		}
	}

#ifdef AUDIO_WAVE_DEBUG
	__InitWavFile(m_fs, m_sFolderName);
#endif

	m_bIsInitSuccess = true;
	return true;
}

bool CaputureAudioProcessing::Reset(void)
{
	return CAudioProcessingImpl::Reset();
}

void CaputureAudioProcessing::CaptureMix()
{ 
	int i=0;
	int j=0;
	float fWeight= 1.f/m_aAudioProcDataShared->nChannelsInCapture_;
	AUDIO_DATA_TYPE* fpout=m_aAudioProcDataShared->pCapture_[0];
	AUDIO_DATA_TYPE** fpin=m_aAudioProcDataShared->pCapture_;
	for(;i<m_aAudioProcDataShared->nSamplesPerCaptureChannel_;i++)
	{
		fpout[i] *= fWeight;
		for(j=1;j<m_aAudioProcDataShared->nChannelsInCapture_; j++)
		{			
			fpout[i] +=fWeight*fpin[j][i];
		}
	}
}

bool CaputureAudioProcessing::processData(IN const AudioFrame **a_AudioFrameArray, IN CAUDIO_U8_t a_nArrayLen)
{
	if (!m_bIsInitSuccess)
	{
		AUDIO_PROCESSING_PRINTF("%s init operation failed", LOG_PROC_IMPL);
		return false;
	}
	if (NULL == a_AudioFrameArray || a_nArrayLen < m_nThreadNum)
	{
		AUDIO_PROCESSING_PRINTF("%s input para error", LOG_PROC_IMPL);
		return false;
	}
	// switch on/off AEC¡¢NR when external parameters have changes.
	if (m_bPlayChEnalbe)
	{
		m_aAudioProcDataShared->bAECOn_ = m_pPropertyPage->pAudioAECInfo_->bAECOn_;
		m_aAudioProcDataShared->bNROn_ = m_pPropertyPage->pAudioAECInfo_->bNROn_;
	}
	else
	{
		m_aAudioProcDataShared->bAECOn_ = false;
		m_aAudioProcDataShared->bNROn_ = false;
	}

	int nAllCh= m_aAudioProcDataShared->nChannelsInCapture_+m_aAudioProcDataShared->nChannelsInRender_+m_aAudioProcDataShared->nChannelsInShare_;
	if (!__AudioFrame2AudioDataType(a_AudioFrameArray, nAllCh, m_ppAudio_buffer))
	{
		AUDIO_PROCESSING_PRINTF("%s __AudioFrame2AudioDataType operation failed!", LOG_PROC_IMPL);
		return false;
	}

	if (static_cast<CAUDIO_U32_t>(m_nFrameSizePerChannel) > m_nOutBufferSize)
	{
		AUDIO_PROCESSING_PRINTF("%s memory size error!", LOG_PROC_IMPL);
		return false;
	}

	if (m_cDCRemoverImpl)
	{
		int ret = m_cDCRemoverImpl->process(*m_aAudioProcDataShared);
		if (0 != ret)
		{
			AUDIO_PROCESSING_PRINTF("%s m_cDCRemoverImpl process operation failed!", LOG_PROC_IMPL);
			return false;
		}
	}

	CaptureMix();

#ifdef AUDIO_WAVE_DEBUG
	__WriteWavFile(*m_aAudioProcDataShared, 0);
#endif

	if (m_aAudioProcDataShared->bAECOn_ || m_aAudioProcDataShared->bNROn_)
	{
		if(NULL != m_cAEC)
		{
			int ret = m_cAEC->process(*m_aAudioProcDataShared);
			if (0 != ret)
			{
				AUDIO_PROCESSING_PRINTF("%s m_cAEC process operation failed!", LOG_PROC_IMPL);
				return false;
			}
		}
	}

#ifdef AUDIO_WAVE_DEBUG
	__WriteWavFile(*m_aAudioProcDataShared, 1);
#endif

	if (m_aAudioMixImpl)
	{
		m_aAudioMixImpl->process(*m_aAudioProcDataShared);
	}

	// transport data to network
	if (!m_pAudioChannel->Transport(m_pOutBuffer, REAL_FLOAT_DATA, m_nFrameSizePerChannel, TP2Network))
	{
		AUDIO_PROCESSING_PRINTF("%s Send operation error", LOG_PROC_IMPL);
		return false;
	}
	if(!m_bRecordingEnable)
	{
		return true;
	}

	// mix data for recording
	if (NULL != m_pLocalRecordingMixer)
	{
		m_pLocalRecordingMixer->process(*m_pRrdMixShareData);
	}

	// transport mix data for recording
	if (!m_pAudioChannel->Transport(m_pRecordingMixOutBuff, REAL_FLOAT_DATA, m_nFrameSizePerChannel, TP2RecordingDev))
	{
		AUDIO_PROCESSING_PRINTF("transport data to recording failed");
		return false;
	}

	return true;
}

bool CaputureAudioProcessing::__AudioFrame2AudioDataType(
	const AudioFrame **_frame_buffer_, 
	CAUDIO_U8_t a_nThreadNum, 
	AUDIO_DATA_TYPE *_process_buffer_[])
{
	CAUDIO_S16_t *frame_buffer_begin_pos   = NULL; 
	AUDIO_DATA_TYPE *process_buffer_begin_pos = NULL; 
	CAUDIO_U8_t frame_buffer_channel_num = 0; 
	CAUDIO_U8_t thread_ind  = 0; 
	CAUDIO_U32_t sample_per_channel = 0; 
	CAUDIO_U32_t i = 0; 
	const AudioFrame* pFrame = NULL;

	for (; thread_ind < a_nThreadNum; ++thread_ind)
	{
		sample_per_channel = 0;
		pFrame=_frame_buffer_[thread_ind];
		if (NULL == pFrame)
		{
			AUDIO_PROCESSING_PRINTF("%s input para error", LOG_PROC_IMPL);
			return false;
		}

		sample_per_channel = pFrame->AudioPara_.samples_per_channel_;
		assert(sample_per_channel == m_nFrameSizePerChannel);

		frame_buffer_begin_pos   = (CAUDIO_S16_t*)pFrame->data_;

		switch(pFrame->AudioPara_.dataSyncId_)  
		{
		case DSFromRenderCh:
			process_buffer_begin_pos = m_aAudioProcDataShared->pRender_[0];
			break;
		case DSFromSharedCh:
			process_buffer_begin_pos = m_aAudioProcDataShared->pShare_[0];
			break;
		default:
			process_buffer_begin_pos = m_ppAudio_buffer[thread_ind];
			break;
		}
		
		frame_buffer_channel_num = pFrame->AudioPara_.num_channels_;

		if(REAL_FIX_DATA == pFrame->AudioPara_.datatype_)
		{
			for (i = 0; i < sample_per_channel; ++i)
			{
				// NOTE: only mix left channel data
				process_buffer_begin_pos[i] = (AUDIO_DATA_TYPE)(frame_buffer_begin_pos[i*frame_buffer_channel_num] / 32768.f);
			}
		}
		else if(REAL_FLOAT_DATA == pFrame->AudioPara_.datatype_)
		{
			memcpy_s(process_buffer_begin_pos, 
				sample_per_channel*m_nMaxChannelNum*DataType2Byte(REAL_FLOAT_DATA), 
				frame_buffer_begin_pos, 
				DataType2Byte(REAL_FLOAT_DATA)*sample_per_channel);
		}
		else
		{
			AUDIO_PROCESSING_PRINTF("unreasonable data type in CAudioProcessingImpl::__AudioFrame2AudioDataType");
			return false;
		}
	}
	return true;
}

bool CaputureAudioProcessing::StopProcess(void)
{

#ifdef AUDIO_WAVE_DEBUG
	return __ReleaseWavFile();
#endif

	return true;
}

bool CaputureAudioProcessing::__InitWavFile(IN CAUDIO_U32_t _fs, IN std::string _folder_name)
{
#ifdef AUDIO_WAVE_DEBUG
	char wave_name[256] = { "" };
	sprintf_s(wave_name, 256, "%s\\pDesire_pReffer_.wav", _folder_name.data());

	m_pWavFileOp = new CWavFileOp(wave_name, "wb");
	m_pWavFileHead = new SWavFileHead;
	if (-2 == m_pWavFileOp->m_FileStatus)
	{
		delete m_pWavFileOp;
		delete m_pWavFileHead;
		m_pWavFileOp = NULL;
		m_pWavFileHead = NULL;
		return false;
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
	m_pWavFileHead->SampleRate = _fs;
	m_pWavFileHead->SampleBytes = _fs * 4;
	m_pWavFileHead->BytesPerSample = 4;
	m_pWavFileHead->NBitsPersample = 16;

	m_nWaveCounter = 0;
	if (m_pWavFileOp)
		m_pWavFileOp->WriteHeader(*m_pWavFileHead);

	memset(wave_name, 0, sizeof(char)* 256);
	sprintf_s(wave_name, 256, "%s\\pError_.wav", _folder_name.data());
	m_pWavFileOp2 = new CWavFileOp(wave_name, "wb");
	m_pWavFileHead2 = new SWavFileHead;
	if (-2 == m_pWavFileOp2->m_FileStatus)
	{
		delete m_pWavFileOp2;
		delete m_pWavFileHead2;
		m_pWavFileOp2 = NULL;
		m_pWavFileHead2 = NULL;
		return false;
	}
	m_pWavFileHead2->NChannels = 2;
	m_pWavFileHead2->RIFF[0] = 'R';
	m_pWavFileHead2->RIFF[1] = 'I';
	m_pWavFileHead2->RIFF[2] = 'F';
	m_pWavFileHead2->RIFF[3] = 'F';
	m_pWavFileHead2->data[0] = 'd';
	m_pWavFileHead2->data[1] = 'a';
	m_pWavFileHead2->data[2] = 't';
	m_pWavFileHead2->data[3] = 'a';
	m_pWavFileHead2->WAVEfmt_[0] = 'W';
	m_pWavFileHead2->WAVEfmt_[1] = 'A';
	m_pWavFileHead2->WAVEfmt_[2] = 'V';
	m_pWavFileHead2->WAVEfmt_[3] = 'E';
	m_pWavFileHead2->WAVEfmt_[4] = 'f';
	m_pWavFileHead2->WAVEfmt_[5] = 'm';
	m_pWavFileHead2->WAVEfmt_[6] = 't';
	m_pWavFileHead2->WAVEfmt_[7] = ' ';
	m_pWavFileHead2->noUse = 0x00000010;
	m_pWavFileHead2->FormatCategory = 1;
	m_pWavFileHead2->SampleRate = _fs;
	m_pWavFileHead2->SampleBytes = _fs * 4;
	m_pWavFileHead2->BytesPerSample = 4;
	m_pWavFileHead2->NBitsPersample = 16;

	m_nWaveCounter2 = 0;
	if (m_pWavFileOp2)
		m_pWavFileOp2->WriteHeader(*m_pWavFileHead2);
	m_bStop = false;

#endif
	return true;
}

bool CaputureAudioProcessing::__ReleaseWavFile(void)
{
#ifdef AUDIO_WAVE_DEBUG
	m_bStop = true;

	if (m_pWavFileOp && m_pWavFileHead)
	{
		m_pWavFileOp->UpdateHeader(m_pWavFileHead->NChannels, m_nWaveCounter / m_pWavFileHead->NChannels);
		delete m_pWavFileOp;
		delete m_pWavFileHead;
		m_pWavFileOp = NULL;
		m_pWavFileHead = NULL;
	}
	if (m_pWavFileOp2 && m_pWavFileHead2)
	{
		m_pWavFileOp2->UpdateHeader(m_pWavFileHead2->NChannels, m_nWaveCounter2 / m_pWavFileHead2->NChannels);
		delete m_pWavFileOp2;
		delete m_pWavFileHead2;
		m_pWavFileOp2 = NULL;
		m_pWavFileHead2 = NULL;
	}
#endif

	return true;
}

bool CaputureAudioProcessing::__WriteWavFile(IN audio_pro_share &aShareData, IN CAUDIO_U32_t process_mode)
{
#ifdef AUDIO_WAVE_DEBUG
	if (m_bStop)
	{
		return false;
	}

	CAUDIO_S16_t *data = NULL;
	float desire_value = 0;
	float refer_value = 0;
	float error_value = 0;
	float error_beforeNR_value = 0;

	data = new CAUDIO_S16_t[aShareData.nSamplesInDesire_ * 2];
	if (0 == process_mode)
	{
		for (CAUDIO_U32_t index = 0; index < aShareData.nSamplesInDesire_; ++index)
		{
			desire_value = aShareData.pDesire_[index] * 32767;
			refer_value = aShareData.pReffer_[index] * 32767;

			if (desire_value > 32767)
			{
				data[index * 2] = 32767;
			}
			else if (desire_value < -32767)
			{
				data[index * 2] = -32767;
			}
			else
			{
				data[index * 2] = static_cast<CAUDIO_S16_t>(desire_value);
			}

			if (refer_value > 32767)
			{
				data[index * 2+1] = 32767;
			}
			else if (refer_value < -32767)
			{
				data[index * 2+1] = -32767;
			}
			else
			{
				data[index * 2 + 1] = static_cast<CAUDIO_S16_t>(refer_value);
			}
		}
		if (m_pWavFileOp)
		{
			m_pWavFileOp->WriteSample(data, aShareData.nSamplesInDesire_*2);
			m_nWaveCounter += aShareData.nSamplesInDesire_ * 2;
		}
	}
	else if (1 == process_mode)
	{
		for (CAUDIO_U32_t index = 0; index < aShareData.nSamplesInDesire_; ++index)
		{
			error_value = aShareData.pError_[index] * 32767;
			error_beforeNR_value = aShareData.pErrorBeforeNR_[index] * 32767;

			if (error_beforeNR_value > 32767)
			{
				data[index * 2] = 32767;
			}
			else if (error_beforeNR_value < -32767)
			{
				data[index * 2] = -32767;
			}
			else
			{
				data[index * 2] = static_cast<CAUDIO_S16_t>(error_beforeNR_value);
			}

			if (error_value > 32767)
			{
				data[index * 2 + 1] = 32767;
			}
			else if (error_value < -32767)
			{
				data[index * 2 + 1] = -32767;
			}
			else
			{
				data[index * 2 + 1] = static_cast<CAUDIO_S16_t>(error_value);
			}
		}
		if (m_pWavFileOp2)
		{
			m_pWavFileOp2->WriteSample(data, aShareData.nSamplesInDesire_ * 2);
			m_nWaveCounter2 += aShareData.nSamplesInDesire_ * 2;
		}
	}
	else
	{
		return false;
	}
	delete []data;

#endif

	return true;
}

