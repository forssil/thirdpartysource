/*! \file   AudioMixerDefs.h
*   \author Keil 
*   \date   2014/12/17
*   \brief  definition of audio mix
*/

#ifndef _AUDIOMIXER_AUDIOMIXERDEFS_H_
#define _AUDIOMIXER_AUDIOMIXERDEFS_H_

#include "audiotypedef.h"
#include "audiotrace.h"

#define AUDIO_MIXER_COMPANDOR_NUM                 (MAX_THREAD_NUM+1) // additional one for output channel

#define K                                         5
#define MIX_GAINSMOOTH_UP_TIME                    30 //ms        
#define MIX_GAINSMOOTH_DOWN_TIME                  600 //ms
#define MIX_ENVELOPESMOOTH_TIME                   100 //ms
#define COMPANDOR_LEVELSMOOTH_UP_TIME             1 //ms        
#define COMPANDOR_LEVELSMOOTH_DOWN_TIME           150 //ms
#define COMPANDOR_GAINSMOOTH_UP_TIME              3 //ms
#define COMPANDOR_GAINSMOOTH_DOWN_TIME            60 //ms
#define COMPANDOR_MODERATE_LEVEL_GAIN_USED        2 // 6db
#define COMPANDOR_CONCEAL_LEVEL_USED              0.005f //-50db
#define COMPANDOR_UP_THRESHOLD_AMPLITUDE_USED     0.1f //-20db
#define COMPANDOR_DOWN_THRESHOLD_AMPLITUDE_USED   0.01f //-40db 
#define COMPANDOR_UP_GAIN_RATIO_USED              2.5
#define COMPANDOR_DOWN_GAIN_RATIO_USED            0.625
#define COMPANDOR_ATTACK_TIME_USED                3
#define COMPANDOR_RELEASE_TIME_USED               60
#define CLIP_LIMIT                                0.92

#define MIX_MODE_USED                 MIX_MODE_ADAPTIVE_WEIGHT
#define CLIP_MODE_USED                CLIP_MODE_2
#define COMPANDOR_MODE_USED           COMPANDOR_MODE_HCLE
#define COMPANDOR_ENVELOP_MODE_USED   COMPANDOR_ENVELOP_MODE_ENERGY
#define COMPANDOR_LEVEL_MODE_USED     COMPANDOR_LEVEL_MODE_SAMPLE
#define COMPANDOE_SMOOTH_MODE_USED    COMPANDOE_SMOOTH_MODE_SILIDE_WIN
#define ADAPTIVE_WEIGHT_MIX_CALC_MODE ADAPTIVE_WEIGHT_MIX_CALC_MODE_BY_FRAME


//compandor mode
enum 
{
	COMPANDOR_MODE_NCNE     = 0, //not compressing, not expanding
	COMPANDOR_MODE_HELE     = 1, //high expand, low expand 
	COMPANDOR_MODE_HELC     = 2, //high expand, low compress
	COMPANDOR_MODE_HCLE     = 3, //high compress, low expand 
	COMPANDOR_MODE_HCLC     = 4  //high compress, low compress
};
//compandor amplifier mode
enum
{
	COMPANDOR_ENVELOP_MODE_ENERGY	   = 0, // energy tracking
	COMPANDOR_ENVELOP_MODE_AMPLITUDE   = 1  //amplitude tracking
};
//compandor level mode
enum
{
	COMPANDOR_LEVEL_MODE_SAMPLE        = 0, // sample by sample processing  
	COMPANDOR_LEVEL_MODE_FRAME         = 1  // frame by frame processing, this way should be abandoned
};
//compandor smooth mode
enum
{
	COMPANDOE_SMOOTH_MODE_RECURSIVE    = 0, //recursive way
	COMPANDOE_SMOOTH_MODE_SILIDE_WIN   = 1  //slide window way
};
//audio mix mode
enum 
{
	MIX_MODE_LINEAR_SUPER              = 0, //Linear superposition method
	MIX_MODE_NO_RELATED_LINEAR_SUPER   = 1, //No-related linear superposition method
	MIX_MODE_AVERAGE                   = 2, //Average method
	MIX_MODE_ATTENUATION_FACTOR        = 3, //Attenuation factor method
	MIX_MODE_ADAPTIVE_WEIGHT           = 4, //Adaptive weighting method
	MIX_MODE_HIGH_THRESHOLD_ADAPTIVE   = 5, //High threshold adaptive weighting method
	MIX_MODE_TRUNCATION                = 6, //Truncation method
	MIX_MODE_ALIGNMENT                 = 7  //Alignment method
};
//audio clip mode
enum 
{
	CLIP_MODE_0   = 0, //mode 0
	CLIP_MODE_1   = 1, //mode 1
	CLIP_MODE_2   = 2, //mode 2
	CLIP_MODE_3   = 3  //mode 3
};

enum 
{
	ADAPTIVE_WEIGHT_MIX_CALC_MODE_BY_FRAME  = 0,
	ADAPTIVE_WEIGHT_MIX_CALC_MODE_BY_SAMPLE = 1
};

enum COMPANDOR_PARAMETER_TYPE
{
	COMPANDOR_NONE                        = -1, //none
	COMPANDOR_LEVEL_MODE                  = 0,  //0 for sample mode , 1 for frame mode
	COMPANDOR_AMPLITUDE_MODE              = 1,  //0 for energy , 1 for envelope
	COMPANDOE_SMOOTH_MODE                 = 2,  //0 for recursive , 1 for slide window
	COMPANDOR_UP_THRESHOLD_AMPLITUDE      = 3,  //amplitude up threshold
	COMPANDOR_DOWN_THRESHOLD_AMPLITUDE    = 4,  //amplitude down threshold
	COMPANDOR_UP_RATIO_GAIN_UPDATE        = 5,  //gain update up ratio
	COMPANDOR_DOWN_RATIO_GAIN_UPDATE      = 6,  //gain update down ratio
	COMPANDOR_UP_THRESHOLD_GAIN_SMOOTH    = 7,  //gain smooth up threshold
	COMPANDOR_DOWN_THRESHOLD_GAIN_SMOOTH  = 8,  //gain smooth down threshold
	COMPANDOR_UP_THRESHOLD_LEVEL_SMOOTH   = 9,  //level smooth up threshold
	COMPANDOR_DOWN_THRESHOLD_LEVEL_SMOOTH = 10, //level smooth down threshold
	COMPANDOR_ATTACK_TIME                 = 11,	//attack time
	COMPANDOR_RELEASE_TIME                = 12, //release time
	COMPANDOR_CONCEAL_LEVEL               = 13,
	COMPANDOR_MODERATE_LEVEL_GAIN         = 14,
	COMPANDOR_GAIN                        = 15, //gain
	COMPANDOR_DELAY                       = 16, //delay time(ms)
	COMPANDOR_COMPANDOR_MODE              = 17  //compandor mode(0~4)
};
//audio clip parameter 
enum CLIP_PARAMETER_TYPE
{
	CLIP_TYPE_NONE     = -1, // none
	CLIP_TYPE_LIMIT    = 1,  // clip limit
	CLIP_TYPE_MODE     = 2   // clip mode
};
//audio mix parameter
enum MIX_PARAMETER_TYPE
{
	MIX_TYPE_NONE        = -1, // none
	MIX_TYPE_MODE        = 0,  // mix mode
	MIX_TYPE_ALPHA       = 1,  // parameter alpha 
	MIX_TYPE_BETA        = 2,  // parameter beta
	MIX_CLIP_TYPE_LIMIT  = 3,  // clip limit
	MIX_CLIP_TYPE_MODE   = 4   // clip mode
};
//audio compandor parameter
struct COMPANDOR_PARAMETER
{
	CAUDIO_S32_t levelMode; //0 for sample mode , 1 for frame mode
	CAUDIO_S32_t amplitudeMode; //0 for energy , 1 for envelope
	CAUDIO_S32_t smoothMode; //0 for recursive , 1 for slide window
	AUDIO_DATA_TYPE upThresholdOfAmplitude; //amplitude up threshold
	AUDIO_DATA_TYPE downThresholdOfAmplitude; //amplitude down threshold
	AUDIO_DATA_TYPE upRatioOfGainUpdate; //gain update up ratio
	AUDIO_DATA_TYPE downRatioOfGainUpdate; //gain update down ratio
	AUDIO_DATA_TYPE upThresholdOfGainSmooth; //gain smooth up threshold
	AUDIO_DATA_TYPE downThresholdOfGainSmooth; //gain smooth down threshold
	AUDIO_DATA_TYPE upThresholdOfLevelSmooth; //level smooth up threshold
	AUDIO_DATA_TYPE downThresholdOfLevelSmooth; //level smooth down threshold
	AUDIO_DATA_TYPE attackTime; //attack time
	AUDIO_DATA_TYPE releaseTime; //release time
	AUDIO_DATA_TYPE concealLevel;
	AUDIO_DATA_TYPE moderateLevelGain;
};

#endif //_AUDIOMIXER_AUDIOMIXERDEFS_H_