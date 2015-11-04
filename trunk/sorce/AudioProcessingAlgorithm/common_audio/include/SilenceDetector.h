#ifndef _AUDIO_PROCESSING_SILENCE_DETECTOR_H_
#define _AUDIO_PROCESSING_SILENCE_DETECTOR_H_

#include <string.h>

class SilenceDetector
{
public:
	SilenceDetector()
	{
		mPWRthresholdEchoDB = 0;
		mLatestMean = 1;
		mPrePwr = 0;
		mPWRtmp = 0;
		mPWRratio = 0;
		mMaxRatio = 0;
		mMinRatio = 0;
		mPwrThresholdEcho = 0;
		memset(mBufferOfPWRratio, 0, sizeof(float) * 3);
	};

	~SilenceDetector(){};

	void init();
private:
	void GetPWRvalue(const float* idata, int size);

public:
	bool isCurrentSilence(float* idata, int size);


private:
	float mPWRthresholdEchoDB;
	float mPwrThresholdEcho;     // pwrThresholdEcho = pow(10,(mPWRthresholdEchoDB /10))

private:
	float mLatestMean;	// the latest mean
	float mPrePwr;	// deviation of the front frame
	float mPWRtmp;    // deviation of data
	float mPWRratio;  // deviation ratio (deviation / original deviation)
	float mBufferOfPWRratio[3]; // store m_fPWRratio
	float mMaxRatio;  // max deviation in m_fBufferOfPWRratio
	float mMinRatio;  // min deviation in m_fBufferOfPWRratio
};



#endif