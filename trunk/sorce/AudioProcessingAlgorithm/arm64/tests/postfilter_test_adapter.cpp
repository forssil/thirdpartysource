#define private public
#include "postfilter.h"
#undef private

#include <cstring>

#ifdef APF_SCALAR_POSTFILTER_ADAPTER
#define ADAPTER(name) scalar_postfilter_##name
#else
#define ADAPTER(name) neon_postfilter_##name
#endif

extern "C" void ADAPTER(update_power)(int length,
                                       float *input,
                                       float *estimate_input,
                                       float *desired_input,
                                       float *power,
                                       float *after,
                                       float *estimate_power,
                                       float *estimate,
                                       float *before,
                                       float speed)
{
    alignas(CPostFilter) unsigned char storage[sizeof(CPostFilter)] = {};
    CPostFilter *filter = reinterpret_cast<CPostFilter *>(storage);
    audio_pro_share data;
    std::memset(&data, 0, sizeof(data));
    filter->m_nHalfFFTLen = length;
    filter->m_pfPwr = power;
    filter->m_pfAft = after;
    filter->m_pfEstPwr = estimate_power;
    filter->m_pfEst = estimate;
    filter->m_pfBef = before;
    data.pNRInput_ = input;
    data.pNRDynamicRefer_ = estimate_input;
    data.pNRInputRefer_ = desired_input;
    data.pEstimationFFT_ = estimate_input;
    data.pDesireFFT_ = desired_input;
    filter->UpdatePwr(&data, speed);
}

extern "C" void ADAPTER(update_reference)(int length,
                                           float *input,
                                           float *power)
{
    alignas(CPostFilter) unsigned char storage[sizeof(CPostFilter)] = {};
    CPostFilter *filter = reinterpret_cast<CPostFilter *>(storage);
    audio_pro_share data;
    std::memset(&data, 0, sizeof(data));
    filter->m_nHalfFFTLen = length;
    filter->m_pfReferPwr = power;
    data.pRefferFFT_ = input;
    filter->UpdateReferPwr(&data);
}

extern "C" void ADAPTER(limit)(int length,
                                float *after,
                                float *gain,
                                float *output,
                                float min_gain,
                                float threshold)
{
    alignas(CPostFilter) unsigned char storage[sizeof(CPostFilter)] = {};
    CPostFilter *filter = reinterpret_cast<CPostFilter *>(storage);
    filter->m_nHalfFFTLen = length;
    filter->m_pfAft = after;
    filter->m_pfGain = gain;
    filter->m_pfGaintemp = output;
    filter->m_fMinGain = min_gain;
    filter->m_fLowLevelThreashold = threshold;
    filter->Spe_Limiter(nullptr);
}
