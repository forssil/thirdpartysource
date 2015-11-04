/*! \file   ReSampleImpl.h
*   \author Keil
*   \date   2015/5/18
*   \brief  resample algorithm
*/

#ifndef _RESAMPLE_IMPL_
#define _RESAMPLE_IMPL_

class CSampleRateConvert;

class ReSampleImpl
{
public:
	ReSampleImpl(unsigned int nChannelNum);

	~ReSampleImpl();

	bool init();

	bool release();

	bool reset();

	int Process(
		const float* inbuffer,
		float* outbuffer,
		const unsigned int	inbuffer_length,
		const unsigned int	outbuffer_length,
		const unsigned int	inbuffer_samplerate,
		const unsigned int	outbuffer_samplerate,
		unsigned int & outbuffer_actuallength
		);

	bool m_bIsInit;
	unsigned int m_nChannelNum;
	CSampleRateConvert *m_cSampleRateConvert;
};
#endif //_RESAMPLE_IMPL_

