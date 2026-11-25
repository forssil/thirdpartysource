#include "PSDsmooth.h"

#ifdef APF_SCALAR_PSD_ADAPTER
#define ADAPTER(name) scalar_psd_##name
#else
#define ADAPTER(name) neon_psd_##name
#endif

extern "C" void *ADAPTER(create)(int fft_length, int sample_rate)
{
    return new CPSDsmooth(fft_length, sample_rate);
}

extern "C" int ADAPTER(length)(void *instance)
{
    return static_cast<CPSDsmooth *>(instance)->m_nAllBand;
}

extern "C" void ADAPTER(set_mode)(void *instance, int mode)
{
    static_cast<CPSDsmooth *>(instance)->m_nTDMode =
        static_cast<TDMode>(mode);
}

extern "C" void ADAPTER(init_frame)(void *instance, float *input)
{
    static_cast<CPSDsmooth *>(instance)->SetInitFrame(input);
}

extern "C" void ADAPTER(process)(void *instance,
                                  float *input,
                                  float *noise)
{
    static_cast<CPSDsmooth *>(instance)->processing(input, noise);
}

extern "C" float *ADAPTER(frequency)(void *instance)
{
    return static_cast<CPSDsmooth *>(instance)->GetCQPsdFd();
}

extern "C" float *ADAPTER(temporal)(void *instance)
{
    return static_cast<CPSDsmooth *>(instance)->GetCQPsd();
}

extern "C" float *ADAPTER(alpha)(void *instance)
{
    return static_cast<CPSDsmooth *>(instance)->GetAlpha();
}
