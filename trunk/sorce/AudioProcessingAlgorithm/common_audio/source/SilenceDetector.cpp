#include "SilenceDetector.h"
#include <cmath>
#include <stdio.h>

void SilenceDetector::init()
{
	mPWRthresholdEchoDB = -58;
	mPwrThresholdEcho = pow(10, (mPWRthresholdEchoDB / 10));
}


//128 samples per frame
bool SilenceDetector::isCurrentSilence(float* idata, int size)
{
	if (128 != size)
	{
		printf("input size should be 128!/n");
		return false;
	}

	// Get deviation and mean of data
	GetPWRvalue(idata, size);

	if (mPWRtmp < mPwrThresholdEcho)
	{
		return true;
	}

	return false;
}


/*********************************************************************************\
Get some value (m_pwrtmp, m_pwrratio, m_maxratio, m_minratio)
\*********************************************************************************/
void SilenceDetector::GetPWRvalue(const float* idata, int size)
{
	float fDeviation = 0.f; // deviation of data 

	// calculate mean
	float mean = 0;
	for (int j = 0; j < size; j++)
		mean += idata[j];
	mean /= size;
	mLatestMean = mean;

	// calculate deviation
	for (int k = 0; k<size; k++)
		fDeviation += pow((idata[k] - size), 2);
	fDeviation /= size;

	// smooth deviation
	if (fDeviation > mPrePwr)
		mPWRtmp = 0.6f*fDeviation + 0.4f*mPrePwr;
	else
		mPWRtmp = 1.f * fDeviation + 0.0f*mPrePwr;
	mPWRratio = mPWRtmp / (mPrePwr + 0.00000000000001f);
	mPrePwr = mPWRtmp;

	// store to m_fBufferOfPWRratio, so we can get max/min 
	mBufferOfPWRratio[0] = mBufferOfPWRratio[1];
	mBufferOfPWRratio[1] = mBufferOfPWRratio[2];
	mBufferOfPWRratio[2] = mPWRratio;
	mMaxRatio = mBufferOfPWRratio[0] > mBufferOfPWRratio[1] ? mBufferOfPWRratio[0] : mBufferOfPWRratio[1];
	mMaxRatio = mBufferOfPWRratio[2] > mMaxRatio ? mBufferOfPWRratio[2] : mMaxRatio;
	mMinRatio = mBufferOfPWRratio[0] < mBufferOfPWRratio[1] ? mBufferOfPWRratio[0] : mBufferOfPWRratio[1];
	mMinRatio = mBufferOfPWRratio[2] < mMinRatio ? mBufferOfPWRratio[2] : mMinRatio;
}