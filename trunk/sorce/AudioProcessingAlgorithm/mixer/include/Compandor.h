/*! \file   Compandor.h
*   \brief  Audio Compandor Algorithm
*/

#ifndef _AUDIOMIXER_COMPANDOR_H_
#define _AUDIOMIXER_COMPANDOR_H_

#define K                                         5
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



//audio compandor parameter
struct COMPANDOR_PARAMETER
{
	float upThresholdOfAmplitude; //amplitude up threshold
	float downThresholdOfAmplitude; //amplitude down threshold
	float upRatioOfGainUpdate; //gaupdate up ratio
	float downRatioOfGainUpdate; //gaupdate down ratio
    float gain_smooth_up_time;
	float upThresholdOfGainSmooth; //gasmooth up threshold
    float gain_smooth_down_time;
	float downThresholdOfGainSmooth; //gasmooth down threshold
    float level_smooth_up_time;
	float upThresholdOfLevelSmooth; //level smooth up threshold
    float level_smooth_down_time;
	float downThresholdOfLevelSmooth; //level smooth down threshold
	float concealLevel;
	float moderateLevelGain;
};
struct CHANNELINFO
{
    float fGain_; // gain
    COMPANDOR_PARAMETER compandor_para; //compandor mode(0~4)
};
class CCompandor
{
public:
///////////////////////////////////////////////////////////////////////constructor/destructor
	CCompandor(const int fs,
			   const int lengthofchannel, 
			   CHANNELINFO mChannelInfo);
	~CCompandor(void);

public:
///////////////////////////////////////////////////////////////////////////////public methods
	//! deal with audio compandor 
	bool AudioCompandor(float* memory, int len);
    float AudioCompandorEnergy(float en_in);
	COMPANDOR_PARAMETER *m_compandorParameter; //CompandorParameter

    void SetChannelInfo(CHANNELINFO info){
        m_pChannelInfo = info;
        m_compandorParameter->upThresholdOfGainSmooth = K/(K+m_pChannelInfo.compandor_para.gain_smooth_up_time *m_nFs/1000.f);
        m_compandorParameter->downThresholdOfGainSmooth = K/(K+m_pChannelInfo.compandor_para.gain_smooth_down_time *m_nFs/1000.f);
    };
private:
///////////////////////////////////////////////////////////////////////////////private methods
     float __PerformCompandorEnergy(float idata);
	//! initial compandor class.
	bool __Init(void);
	//! perform compandor
	void __PerformCompandor(float* idata);
	//! level smooth
	void __LevelSmooth(const float data);
	//! gaupdate
	float __GainUpdate();
	//! frame gasmooth
	void __FrameGainSmooth(const float gain);
	//! recursive way
	inline float __Recursive( float recursivedata, 
										const float newdata, 
										const float ratio1, 
										const float ratio2);
private:
////////////////////////////////////////////////////////////////////////////////////attribute

    int m_nFs; //samples per second
	int m_nSize; //length of data	
	//long m_lIndexOfDelayMemory; //index of delay memory
	//float *m_pDelayMemory; //memory which to store delay frames
	bool m_bIsInitial; //flag of initialization
	float m_fEnergy; //energy
	float m_fSmoothGainOfCompandor; //smooth gaof compandor 
	
	float m_fDelta; //amplitude of gavariation
	CHANNELINFO m_pChannelInfo;
	

};
#endif //_AUDIOMIXER_COMPANDOR_H_

