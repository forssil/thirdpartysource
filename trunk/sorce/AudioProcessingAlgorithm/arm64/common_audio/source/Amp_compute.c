#include "arm64_optimization.h"

#include <arm_neon.h>
#include <math.h>

#include "Amp_compute.h"

#define LOW_LIMIT 0.75f
#define HIGH_LIMIT (2 - LOW_LIMIT)
#define SIGN(x) (signbit(x) ? -1.f : 1.f)

void Clipping(float *sample_value)
{
    float abs_value = fabs(*sample_value);
    if (LOW_LIMIT >= abs_value) {
        return;
    } else if (HIGH_LIMIT >= abs_value) {
        float temp1 = abs_value - LOW_LIMIT;
        *sample_value =
            SIGN(*sample_value) *
            (LOW_LIMIT + temp1 / (1 + temp1 * temp1 /
                                        (HIGH_LIMIT - LOW_LIMIT) /
                                        (HIGH_LIMIT - LOW_LIMIT)));
    } else {
        *sample_value /= abs_value;
    }
}

float GetPowerLevel(float *x, int len)
{
    int i = 0;
    float32x4_t sum = vdupq_n_f32(0.0f);

    for (; i + 4 <= len; i += 4)
        sum = vaddq_f32(sum, vabsq_f32(vld1q_f32(x + i)));

    float total = vaddvq_f32(sum);
    for (; i < len; ++i)
        total += fabsf(x[i]);

    return len > 0 ? total / (float)len : total;
}

float GetPowerEnergy(float *x, int len)
{
    int i = 0;
    float32x4_t sum = vdupq_n_f32(0.0f);

    if (len <= 0)
        return 0;

    for (; i + 4 <= len; i += 4) {
        float32x4_t samples = vld1q_f32(x + i);
        sum = vfmaq_f32(sum, samples, samples);
    }

    float total = vaddvq_f32(sum);
    for (; i < len; ++i)
        total += x[i] * x[i];

    return total / (float)len;
}

float GetPowerdB(float *x, int len, float zero_db_value)
{
    int i = 0;
    float64x2_t sum_lo = vdupq_n_f64(0.0);
    float64x2_t sum_hi = vdupq_n_f64(0.0);
    double energy;
    double min_zero;

    if (len == 0 || zero_db_value < 1e-10f)
        return -127;

    min_zero = (double)(zero_db_value / 100000);
    for (; i + 4 <= len; i += 4) {
        float32x4_t samples = vld1q_f32(x + i);
        float64x2_t low = vcvt_f64_f32(vget_low_f32(samples));
        float64x2_t high = vcvt_high_f64_f32(samples);
        sum_lo = vfmaq_f64(sum_lo, low, low);
        sum_hi = vfmaq_f64(sum_hi, high, high);
    }

    energy = vaddvq_f64(vaddq_f64(sum_lo, sum_hi));
    for (; i < len; ++i)
        energy += (double)x[i] * (double)x[i];

    energy /= len;
    if (energy <= min_zero)
        return -127;

    energy = 10 * log10(energy / ((double)zero_db_value *
                                  (double)zero_db_value));
    return (float)energy;
}

float GetPowerdBShort(short *x, int len, int zero_db_value)
{
    int i = 0;
    int64x2_t sum0 = vdupq_n_s64(0);
    int64x2_t sum1 = vdupq_n_s64(0);
    long long energy;
    float denominator;
    float result;

    if (len == 0 || zero_db_value <= 0)
        return -127;

    for (; i + 8 <= len; i += 8) {
        int16x8_t samples = vld1q_s16(x + i);
        int32x4_t square_lo =
            vmull_s16(vget_low_s16(samples), vget_low_s16(samples));
        int32x4_t square_hi =
            vmull_s16(vget_high_s16(samples), vget_high_s16(samples));
        sum0 = vaddq_s64(sum0, vpaddlq_s32(square_lo));
        sum1 = vaddq_s64(sum1, vpaddlq_s32(square_hi));
    }

    energy = vaddvq_s64(vaddq_s64(sum0, sum1));
    for (; i < len; ++i)
        energy += (long long)(x[i] * x[i]);

    if (energy <= 0)
        return -127;

    denominator = (float)len * (float)zero_db_value *
                  (float)zero_db_value;
    result = (float)energy / denominator;
    return 10 * log10f(result);
}

int ChangeDbToWebExLevel(float db)
{
    int level = 0;
    float volume;

    if (db > -0.000001f)
        db = 0.f;
    volume = -db;

    if (volume > 60)
        level = 0;
    else if (volume > 45)
        level = (int)(10 - (volume - 45) / 3 * 2);
    else if (volume > 40)
        level = (int)(20 - (volume - 40) * 2);
    else if (volume > 35)
        level = (int)(30 - (volume - 35) * 2);
    else if (volume > 30)
        level = (int)(40 - (volume - 30) * 2);
    else if (volume > 25)
        level = (int)(50 - (volume - 25) * 2);
    else if (volume > 20)
        level = (int)(60 - (volume - 20) * 2);
    else if (volume > 15)
        level = (int)(70 - (volume - 15) * 2);
    else if (volume > 10)
        level = (int)(80 - (volume - 10) * 2);
    else if (volume > 5)
        level = (int)(90 - (volume - 5) * 2);
    else
        level = (int)(100 - volume * 2);

    return level;
}

void ShortToFloat(short *src, float *dst, int numSamples)
{
    int i = 0;
    const float32x4_t scale = vdupq_n_f32(1.0f / 32768.0f);

    for (; i + 8 <= numSamples; i += 8) {
        int16x8_t input = vld1q_s16(src + i);
        float32x4_t low = vcvtq_f32_s32(vmovl_s16(vget_low_s16(input)));
        float32x4_t high = vcvtq_f32_s32(vmovl_high_s16(input));
        vst1q_f32(dst + i, vmulq_f32(low, scale));
        vst1q_f32(dst + i + 4, vmulq_f32(high, scale));
    }

    for (; i < numSamples; ++i)
        dst[i] = src[i] / 32768.0f;
}

void FloatToShort(float *src, short *dst, int numSamples)
{
    int i = 0;
    const float32x4_t min_value = vdupq_n_f32(-1.0f);
    const float32x4_t max_value = vdupq_n_f32(1.0f);
    const float32x4_t scale = vdupq_n_f32(32767.0f);

    for (; i + 8 <= numSamples; i += 8) {
        float32x4_t low = vld1q_f32(src + i);
        float32x4_t high = vld1q_f32(src + i + 4);
        low = vmaxq_f32(min_value, vminq_f32(max_value, low));
        high = vmaxq_f32(min_value, vminq_f32(max_value, high));
        vst1q_f32(src + i, low);
        vst1q_f32(src + i + 4, high);
        vst1q_s16(dst + i,
                  vcombine_s16(vqmovn_s32(vcvtq_s32_f32(vmulq_f32(low, scale))),
                               vqmovn_s32(vcvtq_s32_f32(vmulq_f32(high, scale)))));
    }

    for (; i < numSamples; ++i) {
        if (src[i] > 1)
            src[i] = 1;
        else if (src[i] < -1)
            src[i] = -1;
        dst[i] = (short)(src[i] * 32767.0f);
    }
}
