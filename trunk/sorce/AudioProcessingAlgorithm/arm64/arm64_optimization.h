#ifndef APF_ARM64_OPTIMIZATION_H
#define APF_ARM64_OPTIMIZATION_H

#if !defined(__aarch64__)
#error "ARM64 sources require an AArch64 compiler"
#endif

#if !defined(__ARM_NEON)
#error "ARM64 sources require Advanced SIMD support"
#endif

#endif
