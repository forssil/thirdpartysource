#include "SampleRateConvert.h"
#include "samplerate.h"

CSampleRateConvert::CSampleRateConvert(unsigned int nChannelNum)
{
	int error;

	if ((SRCstate = src_new(SRC_SINC_FASTEST, nChannelNum, &error)) == NULL)
	{
		printf("\n\nLine %d : src_new failed : %s.\n\n", __LINE__, src_strerror(error));
		exit(1);
	};

}

CSampleRateConvert::~CSampleRateConvert()
{
	SRCstate = src_delete(SRCstate);
}


int CSampleRateConvert::Process(SRC_Parameter parameter, unsigned int& nGeneratedOutputLength)
{

	float ratio;
	float *InBuffer;
	int ret;

	InBuffer = (float *)parameter.fpInBuffer;

	//calculation ratio
	ratio = (float)parameter.nOutputSampleRate/ (float)parameter.nInputSampleRate;;

	//set SRCdata
	SRCdata.data_in = InBuffer;
	SRCdata.input_frames = parameter.nInputFrameLength;
	SRCdata.data_out = parameter.fpOutBuffer;
	SRCdata.output_frames = parameter.nOutputFrameLength;
	SRCdata.end_of_input = 0;
	SRCdata.src_ratio = ratio;

	nGeneratedOutputLength = SRCdata.output_frames_gen;

	ret = src_process(SRCstate, &SRCdata);

	nGeneratedOutputLength = SRCdata.output_frames_gen;

	return ret;

	//if it was the first time call this function,  the out value should add 0s
}

int CSampleRateConvert::Reset()
{
	return src_reset(SRCstate);
}
