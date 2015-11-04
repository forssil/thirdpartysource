#include "cstring"
#include "ReSampleImpl.h"
#include "SampleRateConvert.h"

ReSampleImpl::ReSampleImpl(unsigned int nChannelNum)
	: m_bIsInit(false)
	, m_nChannelNum(nChannelNum)
	, m_cSampleRateConvert(NULL)
{
	init();
}


ReSampleImpl::~ReSampleImpl()
{
	release();
}

bool ReSampleImpl::init()
{
	if (m_cSampleRateConvert)
	{
		delete m_cSampleRateConvert;
		m_cSampleRateConvert = NULL;
	}
	m_cSampleRateConvert = new CSampleRateConvert(m_nChannelNum);
	if (NULL == m_cSampleRateConvert)
	{
		return -1;
	}

	m_bIsInit = true;

	return 0;
}

bool ReSampleImpl::release()
{
	if (m_cSampleRateConvert)
	{
		delete m_cSampleRateConvert;
		m_cSampleRateConvert = NULL;
	}

	m_bIsInit = false;

	return 0;
}

bool ReSampleImpl::reset()
{
	return 0;
}

int ReSampleImpl::Process(
	const float* inbuffer,
	float* outbuffer,
	const unsigned int	inbuffer_length,
	const unsigned int	outbuffer_length,
	const unsigned int	inbuffer_samplerate,
	const unsigned int	outbuffer_samplerate,
	unsigned int & outbuffer_actuallength
	)
{
	if (inbuffer_samplerate != outbuffer_samplerate)
	{
		SRC_Parameter src_para;
		src_para.fpInBuffer = inbuffer;
		src_para.fpOutBuffer = outbuffer;
		src_para.nInputFrameLength = inbuffer_length;
		src_para.nOutputFrameLength = outbuffer_length;
		src_para.nInputSampleRate = inbuffer_samplerate;
		src_para.nOutputSampleRate = outbuffer_samplerate;

		return m_cSampleRateConvert->Process(src_para, outbuffer_actuallength);
	}
	else
	{
		memcpy_s(outbuffer, sizeof(float)*outbuffer_length, inbuffer, sizeof(float)*inbuffer_length);
		outbuffer_actuallength = inbuffer_length;
		return 0;
	}

}
