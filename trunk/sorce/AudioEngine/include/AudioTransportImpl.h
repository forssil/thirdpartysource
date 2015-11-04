/*	
 *	Name:			AudioTransportImpl.h
 *	
 *	Author:			Zhong Yaozhu
 *  
 *	Description:	for now, mainly used to type cast from engine data type to 
 *					data type of IAudioTransport. 
 *  
 *	History:
 *					03/03/2015 Created
 *
 *
 */

#ifndef _AUDIO_TRANSPORT_IMPL_
#define _AUDIO_TRANSPORT_IMPL_

#include "IAudioTransport.h"
#include "ReSampleImpl.h"

// for now , this Transport interface is only used for CAUDIO_S16_t type
class AudioTransportImpl
{
public:
	AudioTransportImpl();

	AudioTransportImpl(CAUDIO_U32_t sample_rate_in, CAUDIO_U32_t sample_rate_out);

	~AudioTransportImpl();

public:
	bool Transport(
		const void* a_pData,
		DATA_TYPE a_eDataType,
		CAUDIO_U32_t a_nSize,
		TransportId_e a_eTPId);

	bool RegisterAudioSink(IAudioTransport* a_pAudioSink);

	bool DeregisterAudioSink();

private:
	bool AllocTypeCastBuff(CAUDIO_U32_t a_nBufferLen);

	bool AllocReSampleBuff(CAUDIO_U32_t a_nReSampleLen);

	bool AudioDataTypeToShort(const AUDIO_DATA_TYPE* a_pData, CAUDIO_U32_t m_nSize);

private:
	TransportId    m_eTransportID;
	IAudioTransport* m_pAudioSink; // todo : modify this to be a list so that transporting to multiple destinations
	CAUDIO_S16_t* m_pTypeCastBuff;
	CAUDIO_U32_t m_nTypeCastBuffSize;
	CAUDIO_U32_t m_nActualOutBufferSize;

	CAUDIO_U32_t m_nSampleRate_in;
	CAUDIO_U32_t m_nSampleRate_out;
	CAUDIO_U32_t m_nReSampleBufferSize;
	AUDIO_DATA_TYPE* m_pReSampleBuffer;
	ReSampleImpl *m_cReSample;
};


#endif  // _AUDIO_TRANSPORT_IMPL_