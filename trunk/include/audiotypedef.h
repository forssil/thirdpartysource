/*! \file   audiotypedef.h
*   \author Gao Hua
*   \date   2014/10/15
*   \brief  Audio Type Definition
*/

#ifndef _INCLUDE_AUDIOTYPEDEF_H_
#define _INCLUDE_AUDIOTYPEDEF_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
//#include <iostream>
#include <assert.h>
#include <memory.h>
#include "typedefs.h"

#ifdef _DEBUG
	#ifndef AUDIO_TRACE_DEBUG
		#define AUDIO_TRACE_DEBUG
	#endif
	#ifndef AUDIO_WAVE_DEBUG
		#define AUDIO_WAVE_DEBUG
	#endif
#endif 

#ifdef NDEBUG
	#ifndef AUDIO_WAVE_RELEASE
		#define AUDIO_WAVE_RELEASE
	#endif
#endif 

//NOTE: change filter pins to (1 OnlineClassIn + 1 OnlineClassOut + 1 SpeakerOut) 
#define FILTER_MODE_1_IN_2_OUT           1

//NOTE: control capture and playback of the sound card by self
#if  FILTER_MODE_1_IN_2_OUT
	#define CAPTURE_PLAYBACK_INTEGRATED  1
#else
	#define CAPTURE_PLAYBACK_INTEGRATED  1
#endif


#define IN  
#define OUT 

#ifndef PI
	#define PI                         3.1415926 
#endif

#if  FILTER_MODE_1_IN_2_OUT
	#define MAX_THREAD_NUM                 4  
#else 
	#define MAX_THREAD_NUM                 11  
#endif

#define MINIMUM                        1e-26f 
#define FRAME_TIME_MS                  10.f
#define SAMPLERATE_INTERPROCESSUSED    48000
#define SAMPLERATE_ONLINECLASSOUT      48000
#define SAMPLERATE_SPEAKEROUT          48000
#define SAMPLERATE_RECORDEROUT         48000
#define SAMPLERATE_CLASSTOCLASSIN      48000
#define SAMPLERATE_ONLINECLASSIN       48000
#define SAMPLERATE_ELECTRONICPIANOIN   48000
#define SAMPLERATE_MICIN               48000

#define SAMPLE_RATE_VALID_CHECK(x) ((x)==8000 || (x)==16000 || (x)==22050 || (x)==24000  \
								    || (x) == 32000 || (x) == 44100 || (x) == 48000 || (x) == 96000)

// data type definition
typedef float    AUDIO_DATA_TYPE;
typedef int8_t   CAUDIO_S8_t;
typedef int16_t  CAUDIO_S16_t;
typedef int32_t  CAUDIO_S32_t;
typedef int64_t  CAUDIO_S64_t;
typedef uint8_t  CAUDIO_U8_t;
typedef uint16_t CAUDIO_U16_t;
typedef uint32_t CAUDIO_U32_t;
typedef uint64_t CAUDIO_U64_t;

// define id for channel, transport and multipleSync
typedef enum ChannelId
{
	CaptureCh,
	RenderCh,
	SharedCh,
	InvalidCh,
}ChannelId_e;

typedef enum TransportId
{
	TP2CaptureCh,
	TP2RenderCh,
	TP2SharedCh,
	TP2Network,
	TP2RenderDev,
	TP2RecordingDev,
	InvalidTP,
}TransportId_e;

typedef enum DataSyncId
{
	DSFromCaptureCh,
	DSFromRenderCh,
	DSFromSharedCh,
	DSFromNetwork,
	DSFromCaptureDev,
	InvalidDS,
}DataSyncId_e;

typedef enum tag_HardwareDeviceType
{
	RECORD_HARDWARE_DEVICE,
	PLAYOUT_HARDWARE_DEVICE,
	NO_HARDWARE_DEVICE
}HardwareDeviceType;

#define FRAM_LEN(frame_ms,Fs) ( CAUDIO_S16_t( (frame_ms)*(Fs)/1000) )

typedef enum MODULE_STATE
{	
	Audio_Module_State_Unknown   =-1,
	Audio_Module_State_Created   = 0,
	Audio_Module_State_Inited   ,
	Audio_Module_State_Stopped,	
	Audio_Module_State_Paused	= ( Audio_Module_State_Stopped + 1 ) ,
	Audio_Module_State_Running	= ( Audio_Module_State_Paused + 1 ) ,
	Audio_Module_State_Dead
} 	ModuleState;

enum DATA_TYPE
{
	REAL_FIX_DATA    = 0,
	COMPLEX_FIX_DATA ,
	REAL_FLOAT_DATA  ,
	COMPLEX_FLOAT_DATA 
};

inline CAUDIO_S32_t DataType2Byte(DATA_TYPE atype)
{

	CAUDIO_S32_t ret=0;
	switch(atype)
	{
	case REAL_FIX_DATA:
		ret= sizeof(CAUDIO_S16_t);
		break;
	case  COMPLEX_FIX_DATA:
		ret= sizeof(CAUDIO_S16_t)*2;
		break;
	case  REAL_FLOAT_DATA:
		ret= sizeof(AUDIO_DATA_TYPE);
		break;
	case COMPLEX_FLOAT_DATA:
		ret= sizeof(AUDIO_DATA_TYPE)*2;
		break;
	default:
		ret =0;
	}

	return ret;
}

// Audio Frame
enum VADActivity
{
	kVadActive  = 0,
	kVadPassive = 1,
	kVadUnknown = 2
};
enum SpeechType
{
	kNormalSpeech = 0,
	kPLC          = 1,
	kCNG          = 2,
	kPLCCNG       = 3,
	kUndefined    = 4
};

struct AUDIO_FRAME_PARA
{
	CAUDIO_S32_t id_;
	CAUDIO_U32_t timestamp_;	
	CAUDIO_S32_t samples_per_channel_;
	CAUDIO_S32_t sample_rate_hz_;
	CAUDIO_S32_t num_channels_;
	SpeechType speech_type_;
	VADActivity vad_activity_;
	CAUDIO_U32_t energy_;
	DATA_TYPE   datatype_;
	DataSyncId_e dataSyncId_;
};

class AudioFrame
{
public:
	// Stereo, 32 kHz, 60 ms (2 * 32 * 60) of complex float
	static const CAUDIO_S32_t kMaxDataSizeSamples = 3840*sizeof(AUDIO_DATA_TYPE)*2;
	
	AudioFrame();
	virtual ~AudioFrame();

	void UpdateFrame(
		CAUDIO_S32_t id,
		CAUDIO_U32_t timestamp,
		const CAUDIO_U8_t* data,
		CAUDIO_S32_t samples_per_channel,
		CAUDIO_S32_t sample_rate_hz,
		SpeechType speech_type,
		VADActivity vad_activity,
		CAUDIO_S32_t num_channels = 1,
		CAUDIO_U32_t energy = -1,
		DATA_TYPE   datatype=REAL_FIX_DATA,
		DataSyncId_e dataSyncId=InvalidDS);

	void UpdateFrame(AUDIO_FRAME_PARA aPara,const CAUDIO_U8_t* data);
	//AudioFrame& Append(const AudioFrame& rhs);

	void CopyFrom(const AudioFrame& src);
	void Reset();
	void Mute();

// 	AudioFrame& operator>>=(const CAUDIO_S32_t rhs);
// 	AudioFrame& operator+=(const AudioFrame& rhs);
// 	AudioFrame& operator-=(const AudioFrame& rhs);

	
public:
	CAUDIO_U8_t data_[kMaxDataSizeSamples];
	AUDIO_FRAME_PARA AudioPara_;
private:
	//DISALLOW_COPY_AND_ASSIGN(AudioFrame);
	 void operator=(const AudioFrame&);
	 AudioFrame(const AudioFrame&);  
};

inline void AudioFrame::UpdateFrame(AUDIO_FRAME_PARA aPara,const CAUDIO_U8_t* data)
{
	memcpy(&AudioPara_, &aPara,sizeof(AUDIO_FRAME_PARA));
	const CAUDIO_S32_t length = AudioPara_.samples_per_channel_ * AudioPara_.num_channels_;
	assert(length <= kMaxDataSizeSamples && length >= 0);


	CAUDIO_S32_t databytes = DataType2Byte(AudioPara_.datatype_);
	if(data != NULL)
	{
		memcpy(data_, data, databytes * length);
	}
	else
	{
		memset(data_, 0, databytes * length);
	}
}

inline void AudioFrame::Reset()
{
	AudioPara_.id_= (-1);
	AudioPara_.timestamp_=(0);
	AudioPara_.samples_per_channel_=(0);
	AudioPara_.sample_rate_hz_=0;
	AudioPara_.num_channels_=1;
	AudioPara_.speech_type_=kUndefined;
	AudioPara_.vad_activity_=kVadUnknown;
	AudioPara_.energy_=0xffffffff;
	AudioPara_.datatype_=REAL_FIX_DATA;
	memset(data_, 0, kMaxDataSizeSamples);
	AudioPara_.dataSyncId_ = InvalidDS;
}

inline
	AudioFrame::AudioFrame()

{
	AudioPara_.id_= (-1);
	AudioPara_.timestamp_=(0);
	AudioPara_.samples_per_channel_=(0);
	AudioPara_.sample_rate_hz_=0;
	AudioPara_.num_channels_=1;
	AudioPara_.speech_type_=kUndefined;
	AudioPara_.vad_activity_=kVadUnknown;
	AudioPara_.energy_=0xffffffff;
	AudioPara_.datatype_=REAL_FIX_DATA;
	AudioPara_.dataSyncId_ = InvalidDS;
}

inline
	AudioFrame::~AudioFrame()
{
}

inline
	void
	AudioFrame::UpdateFrame(
	CAUDIO_S32_t id,
	CAUDIO_U32_t timestamp,
	const CAUDIO_U8_t* data,
	CAUDIO_S32_t samples_per_channel,
	CAUDIO_S32_t sample_rate_hz,
	SpeechType speech_type,
	VADActivity vad_activity,
	CAUDIO_S32_t num_channels,
	CAUDIO_U32_t energy,
	DATA_TYPE   datatype,
	DataSyncId_e dataSyncId)
{
	AudioPara_.id_            = id;
	AudioPara_.timestamp_     = timestamp;
	AudioPara_.samples_per_channel_ = samples_per_channel;
	AudioPara_.sample_rate_hz_ = sample_rate_hz;
	AudioPara_.speech_type_    = speech_type;
	AudioPara_.vad_activity_   = vad_activity;
	AudioPara_.num_channels_  = num_channels;
	AudioPara_.energy_        = energy;
	AudioPara_.datatype_	   = datatype;
	AudioPara_.dataSyncId_	= dataSyncId;

	
	const CAUDIO_S32_t length = samples_per_channel * num_channels;
	assert(length <= kMaxDataSizeSamples && length >= 0);

	CAUDIO_S32_t databytes = DataType2Byte(AudioPara_.datatype_);
	if(data != NULL)
	{
		memcpy(data_, data, databytes * length);
	}
	else
	{
		memset(data_, 0, databytes * length);
	}
}

inline void AudioFrame::CopyFrom(const AudioFrame& src)
{
	if(this == &src)
	{
		return;
	}
	AudioPara_.id_               = src.AudioPara_.id_;
	AudioPara_.timestamp_        = src.AudioPara_.timestamp_;
	AudioPara_.samples_per_channel_ = src.AudioPara_.samples_per_channel_;
	AudioPara_.sample_rate_hz_    = src.AudioPara_.sample_rate_hz_;
	AudioPara_.speech_type_       = src.AudioPara_.speech_type_;
	AudioPara_.vad_activity_      = src.AudioPara_.vad_activity_;
	AudioPara_.num_channels_     = src.AudioPara_.num_channels_;
	AudioPara_.energy_           = src.AudioPara_.energy_;
	AudioPara_.datatype_		  = src.AudioPara_.datatype_;
	AudioPara_.dataSyncId_		 = src.AudioPara_.dataSyncId_;
	const CAUDIO_S32_t length = AudioPara_.samples_per_channel_ * AudioPara_.num_channels_;
	
	assert(length <= kMaxDataSizeSamples && length >= 0);

	CAUDIO_S32_t databytes = DataType2Byte(AudioPara_.datatype_);
	memcpy(data_, src.data_, databytes* length);
}

inline
	void
	AudioFrame::Mute()
{
	memset(data_, 0, kMaxDataSizeSamples);
}
/*
inline
	AudioFrame&
	AudioFrame::operator>>=(const CAUDIO_S32_t rhs)
{
	assert((num_channels_ > 0) && (num_channels_ < 3));
	if((num_channels_ > 2) ||
		(num_channels_ < 1))
	{
		return *this;
	}
	for(CAUDIO_S32_t i = 0; i < samples_per_channel_ * num_channels_; i++)
	{
		data_[i] = static_cast<CAUDIO_U8_t>(data_[i] >> rhs);
	}
	return *this;
}

inline
	AudioFrame&
	AudioFrame::Append(const AudioFrame& rhs)
{
	// Sanity check
	assert((num_channels_ > 0) && (num_channels_ < 3));
	if((num_channels_ > 2) ||
		(num_channels_ < 1))
	{
		return *this;
	}
	if(num_channels_ != rhs.num_channels_)
	{
		return *this;
	}
	if((vad_activity_ == kVadActive) ||
		rhs.vad_activity_ == kVadActive)
	{
		vad_activity_ = kVadActive;
	}
	else if((vad_activity_ == kVadUnknown) ||
		rhs.vad_activity_ == kVadUnknown)
	{
		vad_activity_ = kVadUnknown;
	}
	if(speech_type_ != rhs.speech_type_)
	{
		speech_type_ = kUndefined;
	}

	CAUDIO_S32_t offset = samples_per_channel_ * num_channels_;
	for(CAUDIO_S32_t i = 0;
		i < rhs.samples_per_channel_ * rhs.num_channels_;
		i++)
	{
		data_[offset+i] = rhs.data_[i];
	}
	samples_per_channel_ += rhs.samples_per_channel_;
	return *this;
}

// merge vectors
inline
	AudioFrame&
	AudioFrame::operator+=(const AudioFrame& rhs)
{
	// Sanity check
	assert((num_channels_ > 0) && (num_channels_ < 3));
	if((num_channels_ > 2) ||
		(num_channels_ < 1))
	{
		return *this;
	}
	if(num_channels_ != rhs.num_channels_)
	{
		return *this;
	}
	bool noPrevData = false;
	if(samples_per_channel_ != rhs.samples_per_channel_)
	{
		if(samples_per_channel_ == 0)
		{
			// special case we have no data to start with
			samples_per_channel_ = rhs.samples_per_channel_;
			noPrevData = true;
		} else
		{
			return *this;
		}
	}

	if((vad_activity_ == kVadActive) ||
		rhs.vad_activity_ == kVadActive)
	{
		vad_activity_ = kVadActive;
	}
	else if((vad_activity_ == kVadUnknown) ||
		rhs.vad_activity_ == kVadUnknown)
	{
		vad_activity_ = kVadUnknown;
	}

	if(speech_type_ != rhs.speech_type_)
	{
		speech_type_ = kUndefined;
	}

	if(noPrevData)
	{
		memcpy(data_, rhs.data_,
			sizeof(CAUDIO_U8_t) * rhs.samples_per_channel_ * num_channels_);
	} else
	{
		// IMPROVEMENT this can be done very fast in assembly
		for(CAUDIO_S32_t i = 0; i < samples_per_channel_ * num_channels_; i++)
		{
			int32_t wrapGuard = static_cast<int32_t>(data_[i]) +
				static_cast<int32_t>(rhs.data_[i]);
			if(wrapGuard < -32768)
			{
				data_[i] = -32768;
			}else if(wrapGuard > 32767)
			{
				data_[i] = 32767;
			}else
			{
				data_[i] = (CAUDIO_U8_t)wrapGuard;
			}
		}
	}
	energy_ = 0xffffffff;
	return *this;
}

inline
	AudioFrame&
	AudioFrame::operator-=(const AudioFrame& rhs)
{
	// Sanity check
	assert((num_channels_ > 0) && (num_channels_ < 3));
	if((num_channels_ > 2)||
		(num_channels_ < 1))
	{
		return *this;
	}
	if((samples_per_channel_ != rhs.samples_per_channel_) ||
		(num_channels_ != rhs.num_channels_))
	{
		return *this;
	}
	if((vad_activity_ != kVadPassive) ||
		rhs.vad_activity_ != kVadPassive)
	{
		vad_activity_ = kVadUnknown;
	}
	speech_type_ = kUndefined;

	for(CAUDIO_S32_t i = 0; i < samples_per_channel_ * num_channels_; i++)
	{
		int32_t wrapGuard = static_cast<int32_t>(data_[i]) -
			static_cast<int32_t>(rhs.data_[i]);
		if(wrapGuard < -32768)
		{
			data_[i] = -32768;
		}
		else if(wrapGuard > 32767)
		{
			data_[i] = 32767;
		}
		else
		{
			data_[i] = (CAUDIO_U8_t)wrapGuard;
		}
	}
	energy_ = 0xffffffff;
	return *this;
}

inline bool IsNewerSequenceNumber(uint16_t sequence_number,
	uint16_t prev_sequence_number) {
		return sequence_number != prev_sequence_number &&
			static_cast<uint16_t>(sequence_number - prev_sequence_number) < 0x8000;
}

inline bool IsNewerTimestamp(CAUDIO_U32_t timestamp, CAUDIO_U32_t prev_timestamp) {
	return timestamp != prev_timestamp &&
		static_cast<CAUDIO_U32_t>(timestamp - prev_timestamp) < 0x80000000;
}

inline uint16_t LatestSequenceNumber(uint16_t sequence_number1,
	uint16_t sequence_number2) {
		return IsNewerSequenceNumber(sequence_number1, sequence_number2) ?
sequence_number1 : sequence_number2;
}

inline CAUDIO_U32_t LatestTimestamp(CAUDIO_U32_t timestamp1, CAUDIO_U32_t timestamp2) {
	return IsNewerTimestamp(timestamp1, timestamp2) ? timestamp1 :
		timestamp2;
}
*/
/////////////////Audio Frame




/*audio mix parameter definition*/
// audio channel information
struct AUDIO_MIX_INFO
{
	AUDIO_DATA_TYPE fGain_; // gain
	CAUDIO_U32_t nChannelDelay_; //delay time(ms)
	CAUDIO_U32_t nCompandorMode_; //compandor mode(0~4)
};

struct AUDIO_AEC_INFO
{
	bool bAECOn_;
	bool bNROn_;
};

struct AUDIO_PROPERTY_PAGE
{
	AUDIO_AEC_INFO *pAudioAECInfo_;
	AUDIO_MIX_INFO **pAudioMixInfo_;
};

// audio mix property struct 
struct AUDIO_MIX_PROPERTY
{
	//-----------------------compandor module-------------------------//
	CAUDIO_S32_t   nLevelMode; //0 for sample , 1 for frame
	CAUDIO_S32_t   nAmplitudeMode; //0 for energy , 1 for envelope
	CAUDIO_S32_t   nSmoothMode; //0 for recursive , 1 for slide window
	CAUDIO_S32_t   nCompandorMode; //compression mode 0~4
	CAUDIO_S32_t   nDelayTime; //delay time(multiple of frames) , if m_fDelayTime = 1 , then length of delay is m_nSize*1
	AUDIO_DATA_TYPE fUpThresholdOfAmplitude; //amplitude up threshold
	AUDIO_DATA_TYPE fDownThresholdOfAmplitude; //amplitude down threshold
	AUDIO_DATA_TYPE fUpRatioOfGainUpdate; //gain update up ratio
	AUDIO_DATA_TYPE fDownRatioOfGainUpdate; //gain update down ratio
	AUDIO_DATA_TYPE fUpThresholdOfGainSmooth; //gain smooth up threshold
	AUDIO_DATA_TYPE fDownThresholdOfGainSmooth; //gain smooth down threshold
	AUDIO_DATA_TYPE fUpThresholdOfLevelSmooth; //level smooth up threshold
	AUDIO_DATA_TYPE fDownThresholdOfLevelSmooth; //level smooth down threshold
	AUDIO_DATA_TYPE fAttackTime; //attack time
	AUDIO_DATA_TYPE fReleaseTime; //release time
	AUDIO_DATA_TYPE fAutomaticGain; //automatic gain
	AUDIO_DATA_TYPE fConcealLevel; //starting position in compandor mode 3 
	AUDIO_DATA_TYPE fModerateLevelGain; //ratio in compandor mode 3 

	//-----------------------mix module------------------------------//
	CAUDIO_S32_t   nMixMode; //mix mode 0~7
	AUDIO_DATA_TYPE fMixAlpha; //mix mode3 , pincers factor
	AUDIO_DATA_TYPE fMixBeta; //mix mode5 , high-threshold

	//-----------------------clip module-----------------------------//
	CAUDIO_S32_t   nClipMode; //clip mode, 0~3
	AUDIO_DATA_TYPE fClipLimit; //0.f<= limit <=1.f
};

/* data shared in audio processing */
typedef struct  AUDIO_PROCESSING_DATA_SHARE
{
	/////////////// unprocessed data
	//time domain
	AUDIO_DATA_TYPE		**ppCapture_;
	CAUDIO_U32_t	    nChannelsInCapture_;
	CAUDIO_U32_t		nSamplesPerCaptureChannel_; 
	AUDIO_DATA_TYPE		**ppRender_;
	CAUDIO_U32_t		nChannelsInRender_;
	CAUDIO_U32_t		nSamplesPerRenderChannel_;  
	AUDIO_DATA_TYPE		**ppShare_;
	CAUDIO_U32_t		nChannelsInShare_;
	CAUDIO_U32_t		nSamplesPerShareChannel_;  
	AUDIO_DATA_TYPE     **ppProcessOut_;
	CAUDIO_U32_t	    nChannelsInProcessOut_;
	CAUDIO_U32_t		nSamplesPerProcessOutChannel_;
	//spectrum domain
	CAUDIO_U32_t		nLengthFFT_;  
	AUDIO_DATA_TYPE		**ppCapureFFT_;
	CAUDIO_U32_t	    nChannelsInCaptureFFT_;
	AUDIO_DATA_TYPE		**ppRenderFFT_;
	CAUDIO_U32_t		nChannelsInRenderFFT_;
	AUDIO_DATA_TYPE     **ppProcessOutFFT_;
	CAUDIO_U32_t	    nChannelsInProcessOutFFT_;

	//mix process
	AUDIO_DATA_TYPE     **pMixIn_;
	CAUDIO_U32_t	    nChannelsInMixIn_;
	CAUDIO_U32_t		nSamplesPerMixInChannel_;
	AUDIO_DATA_TYPE     *pMixOut_;            // output buffer of audio mix
	CAUDIO_U32_t		nLenOfMixOut_;        // length of mix out buffer
	AUDIO_MIX_PROPERTY  *pAudioMixProperty_;  // structure of audio mix property

	//aec process
	CAUDIO_U32_t        nSamplesInDesire_;
	AUDIO_DATA_TYPE     *pDesire_;
	CAUDIO_U32_t        nSamplesInReffer_;
	AUDIO_DATA_TYPE     *pReffer_;
	AUDIO_DATA_TYPE		*pDesireFFT_;
	AUDIO_DATA_TYPE		*pEstimationFFT_;
	AUDIO_DATA_TYPE		*pErrorFFT_;
	AUDIO_DATA_TYPE		*pErrorSpectrumPower_;
	AUDIO_DATA_TYPE		*pRefferFFT_;
    AUDIO_DATA_TYPE		*pRNNERRORFFT_;
	AUDIO_DATA_TYPE		*pMaxSubfilterTaps_;
	AUDIO_DATA_TYPE		*pError_;
	CAUDIO_U32_t 		nOffsetBin_; //AEC start bin in fft
	bool				bAECOn_;
	CAUDIO_S32_t		nFarVAD_;
    CAUDIO_S32_t		nNearVAD_;
    AUDIO_DATA_TYPE		fErle_;
    bool                bRnnon_;

#ifdef AUDIO_WAVE_DEBUG
	AUDIO_DATA_TYPE		*pErrorBeforeNR_;
#endif

    //NR
	bool                bNROn_;
	AUDIO_DATA_TYPE     *pNRInput_;         //  
	AUDIO_DATA_TYPE		*pNRDynamicRefer_;  //  echo estimation 
	AUDIO_DATA_TYPE		*pNRInputRefer_;    //  before adaptive filter
	AUDIO_DATA_TYPE		*pGain_;
	CAUDIO_U32_t		nGainLength_;
	AUDIO_DATA_TYPE		fNoisePwr_;
	AUDIO_DATA_TYPE		fSpeechProbability_;
	AUDIO_DATA_TYPE		fProiSNR_;
	AUDIO_DATA_TYPE		fNoisePwrBefor_;
	AUDIO_DATA_TYPE*    pNoiseSPwr;
	//CNG
	bool                bNRCNGOn_;
	AUDIO_DATA_TYPE*    pNRCNGBuffer_;
	//AUDIO_DATA_TYPE*    pNoiseSPwrBef;
    //Delay
	AUDIO_DATA_TYPE*    pDTD;
	AUDIO_DATA_TYPE     fDTDTotal;
	AUDIO_DATA_TYPE		fDTDgain;
	//VAD

	//RES
	bool                IsResEcho_;

    //AGC
    bool                bAGCOn_;
    AUDIO_DATA_TYPE		fAGCgain_;

    //RNNOISE
    bool                bRNNOISEOn_;
    bool                bPreRnnOn_;
    AUDIO_DATA_TYPE     *pRNNERROR_;
    AUDIO_DATA_TYPE     *pRNNPOWER_;
    AUDIO_DATA_TYPE     *pRNNBuffer_; // 10ms 480 points delay
    AUDIO_DATA_TYPE     *pRNNBufferDiff_; // 1.13ms 64 points delay
    bool                bRNNOISEVad_ = true;
    bool                bRNNOISEVad_enhance_ = true;
    int                 RNNCounter_;
    int                 RNNCounter_enhance_;
    int                 ChannelIndex_;
    CAUDIO_U32_t        FrameCounter_;
    float               RnnVad_;
    AUDIO_DATA_TYPE     *RnnGain_;

} audio_pro_share;

typedef struct AUDIO_TYPE_NUM
{
	CAUDIO_U8_t nTotal_in_;
	CAUDIO_U8_t nTotal_out_;

	CAUDIO_U8_t nClassToClass_in_;
	CAUDIO_U8_t nElectronicPiano_in_;
	CAUDIO_U8_t nOnLineClass_in_;
	CAUDIO_U8_t nMic_in_;

	CAUDIO_U8_t nOnLineClass_out_;
	CAUDIO_U8_t nRecording_out_;
	CAUDIO_U8_t nSpeaker_out_;
}AUDIO_TYPE_NUM_t;

// To specify sample rate to internal process and each output audio stream
struct SSampleRate
{
	SSampleRate()
	{
		nSampleRate_InterProcessUsed = SAMPLERATE_INTERPROCESSUSED;
		nSampleRate_OnlineClassOut = SAMPLERATE_ONLINECLASSOUT;
		nSampleRate_SpeakerOut = SAMPLERATE_SPEAKEROUT;
		nSampleRate_RecorderOut = SAMPLERATE_RECORDEROUT;

		// TODO: initial
		nSampleRate_ClassToClassIn = SAMPLERATE_CLASSTOCLASSIN;
		nSampleRate_OnLineClassIn = SAMPLERATE_ONLINECLASSIN;
		nSampleRate_ElectronicPianoIn = SAMPLERATE_ELECTRONICPIANOIN;
		nSampleRate_MicIn = SAMPLERATE_MICIN;

	}

	CAUDIO_U32_t nSampleRate_InterProcessUsed;
	CAUDIO_U32_t nSampleRate_OnlineClassOut;
	CAUDIO_U32_t nSampleRate_SpeakerOut;
	CAUDIO_U32_t nSampleRate_RecorderOut;

	CAUDIO_U32_t nSampleRate_ClassToClassIn;
	CAUDIO_U32_t nSampleRate_OnLineClassIn;
	CAUDIO_U32_t nSampleRate_ElectronicPianoIn;
	CAUDIO_U32_t nSampleRate_MicIn;
};

#endif //_INCLUDE_AUDIOTYPEDEF_H_