#include "AdapFilterGroup.h"

#include <cstring>

#ifdef APF_SCALAR_ADAP_ADAPTER
#define ADAPTER(name) scalar_adap_##name
#else
#define ADAPTER(name) neon_adap_##name
#endif

extern "C" void *ADAPTER(create)(int banks,
                                  int *taps,
                                  float mu,
                                  float delta_gain)
{
    return new CAdapFilterGroup(banks, taps, mu, delta_gain);
}

extern "C" void ADAPTER(process)(void *instance,
                                  const float *reference,
                                  const float *desired,
                                  float *estimate,
                                  float *error,
                                  int update)
{
    static_cast<CAdapFilterGroup *>(instance)->process(
        reference, desired, estimate, error, update);
}
