#ifndef SAMPLE_RATE_CONVERT_H
#define SAMPLE_RATE_CONVERT_H

#include <stdio.h>
#include <stdlib.h> 
#include "samplerate.h"

typedef struct
{
	const float		*fpInBuffer;
	float			*fpOutBuffer;
	unsigned int	nInputFrameLength;
	unsigned int	nOutputFrameLength;
	unsigned int	nInputSampleRate;
	unsigned int	nOutputSampleRate;
} SRC_Parameter;


class CSampleRateConvert
{
public:
	CSampleRateConvert(unsigned int nChannelNum);
	~CSampleRateConvert();

public:
	//todo: at the first time, process output less data(delay).   should add some 0's at the begining of the data 
	int Process(SRC_Parameter parameter, unsigned int& nGeneratedOutputLength);
	int Reset();

private:
	SRC_DATA SRCdata;
	SRC_STATE *SRCstate;
};

#endif
