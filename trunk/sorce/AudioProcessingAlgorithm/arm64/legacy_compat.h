#ifndef APF_ARM64_LEGACY_COMPAT_H
#define APF_ARM64_LEGACY_COMPAT_H

#include "arm64_optimization.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

static inline int apf_arm64_memcpy_s(void *destination,
                                    size_t destination_size,
                                    const void *source,
                                    size_t count)
{
    if (count > destination_size)
        return ERANGE;
    memcpy(destination, source, count);
    return 0;
}

#define memcpy_s(destination, destination_size, source, count) \
    apf_arm64_memcpy_s((destination), (destination_size), (source), (count))

#endif
