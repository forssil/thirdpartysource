#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#define SPEst ScalarSPEst
#include "SPEst.h"
#undef SPEst
#undef AECHALFDUPLEX_SPEST_H_
#include "SPEst.h"

#define DCRemover ScalarDCRemover
#include "DCRemover.h"
#undef DCRemover
#undef DC_REMOVER_
#include "DCRemover.h"

extern "C" float sinc_fun(float x)
{
    return std::fabs(x) > 0.01f ? std::sin(x) / x : std::cos(x);
}

extern "C" void *scalar_psd_create(int fft_length, int sample_rate);
extern "C" int scalar_psd_length(void *instance);
extern "C" void scalar_psd_set_mode(void *instance, int mode);
extern "C" void scalar_psd_init_frame(void *instance, float *input);
extern "C" void scalar_psd_process(void *instance, float *input, float *noise);
extern "C" float *scalar_psd_frequency(void *instance);
extern "C" float *scalar_psd_temporal(void *instance);
extern "C" float *scalar_psd_alpha(void *instance);
extern "C" void *neon_psd_create(int fft_length, int sample_rate);
extern "C" int neon_psd_length(void *instance);
extern "C" void neon_psd_set_mode(void *instance, int mode);
extern "C" void neon_psd_init_frame(void *instance, float *input);
extern "C" void neon_psd_process(void *instance, float *input, float *noise);
extern "C" float *neon_psd_frequency(void *instance);
extern "C" float *neon_psd_temporal(void *instance);
extern "C" float *neon_psd_alpha(void *instance);
extern "C" void scalar_postfilter_update_power(
    int, float *, float *, float *, float *, float *, float *, float *, float *, float);
extern "C" void neon_postfilter_update_power(
    int, float *, float *, float *, float *, float *, float *, float *, float *, float);
extern "C" void scalar_postfilter_update_reference(int, float *, float *);
extern "C" void neon_postfilter_update_reference(int, float *, float *);
extern "C" void scalar_postfilter_limit(
    int, float *, float *, float *, float, float);
extern "C" void neon_postfilter_limit(
    int, float *, float *, float *, float, float);
extern "C" void apf_postfilter_apply_gain(
    float *, const float *, int, int, int);
extern "C" void *scalar_adap_create(int, int *, float, float);
extern "C" void scalar_adap_process(
    void *, const float *, const float *, float *, float *, int);
extern "C" void *neon_adap_create(int, int *, float, float);
extern "C" void neon_adap_process(
    void *, const float *, const float *, float *, float *, int);

static uint32_t random_state = 0x87654321u;

static float random_float(float scale)
{
    random_state = random_state * 1664525u + 1013904223u;
    int32_t value = static_cast<int32_t>(random_state >> 1);
    return (static_cast<float>(value) / 1073741824.0f - 1.0f) * scale;
}

static bool close_float(float lhs, float rhs, float tolerance)
{
    if (std::isnan(lhs) || std::isnan(rhs))
        return std::isnan(lhs) && std::isnan(rhs);
    float scale = std::max(1.0f, std::max(std::fabs(lhs), std::fabs(rhs)));
    return std::fabs(lhs - rhs) <= tolerance * scale;
}

static bool check_vector(const char *name,
                         const std::vector<float> &expected,
                         const std::vector<float> &actual,
                         float tolerance,
                         bool report)
{
    float max_absolute = 0.0f;
    float max_relative = 0.0f;
    for (size_t i = 0; i < expected.size(); ++i) {
        float absolute = std::fabs(expected[i] - actual[i]);
        float scale =
            std::max(1.0f, std::max(std::fabs(expected[i]), std::fabs(actual[i])));
        max_absolute = std::max(max_absolute, absolute);
        max_relative = std::max(max_relative, absolute / scale);
        if (!close_float(expected[i], actual[i], tolerance)) {
            std::fprintf(stderr,
                         "%s mismatch at %zu: expected %.9g, got %.9g\n",
                         name,
                         i,
                         expected[i],
                         actual[i]);
            return false;
        }
    }
    if (report) {
        std::printf("%-24s max_abs=%-12.6g max_rel=%.6g\n",
                    name,
                    max_absolute,
                    max_relative);
    }
    return true;
}

static bool test_spest()
{
    const int sizes[] = {1, 3, 4, 7, 64, 257};
    for (int size : sizes) {
        ScalarSPEst scalar;
        SPEst neon;
        std::vector<float> input(2 * size);
        std::vector<float> scalar_smooth(size);
        std::vector<float> neon_smooth(size);
        std::vector<float> scalar_raw(size);
        std::vector<float> neon_raw(size);

        scalar.InitPara(size);
        neon.InitPara(size);
        for (float &sample : input)
            sample = random_float(4.0f);

        for (int iteration = 0; iteration < 8; ++iteration) {
            scalar.PwrEnergy(input.data(),
                             scalar_smooth.data(),
                             scalar_raw.data());
            neon.PwrEnergy(input.data(), neon_smooth.data(), neon_raw.data());
            if (!check_vector("SPEst smooth",
                              scalar_smooth,
                              neon_smooth,
                              3e-6f,
                              iteration == 7) ||
                !check_vector("SPEst raw",
                              scalar_raw,
                              neon_raw,
                              2e-6f,
                              iteration == 7))
                return false;
        }
    }
    return true;
}

static bool test_dc_remover()
{
    const int sizes[] = {2, 4, 6, 64, 258};
    for (int size : sizes) {
        // Construction calls PrePosAnaInit; scope exit exercises destruction.
        ScalarDCRemover scalar(48000, size, 1);
        DCRemover neon(48000, size, 1);
        std::vector<float> scalar_data(size);
        std::vector<float> neon_data(size);

        for (int iteration = 0; iteration < 16; ++iteration) {
            for (float &sample : scalar_data)
                sample = random_float(2.0f) + 0.25f;
            neon_data = scalar_data;
            scalar.findLevelAndDcRemove(scalar_data.data(), 0);
            neon.findLevelAndDcRemove(neon_data.data(), 0);
            if (!check_vector("DCRemover",
                              scalar_data,
                              neon_data,
                              0.0f,
                              iteration == 15))
                return false;
        }
    }
    std::puts("DCRemover constructor   covered");
    std::puts("DCRemover PrePosAnaInit covered via constructor");
    std::puts("DCRemover destructor    covered via scope exit");
    return true;
}

static bool test_psd_smooth()
{
    constexpr int fft_length = 1024;
    void *scalar = scalar_psd_create(fft_length, 16000);
    void *neon = neon_psd_create(fft_length, 16000);
    std::vector<float> input(fft_length / 2);
    int length = scalar_psd_length(scalar);
    std::vector<float> noise(length);

    scalar_psd_set_mode(scalar, 1);
    neon_psd_set_mode(neon, 1);
    for (float &value : input)
        value = std::fabs(random_float(4.0f)) + 1e-4f;
    for (float &value : noise)
        value = std::fabs(random_float(2.0f)) + 1e-4f;

    scalar_psd_init_frame(scalar, input.data());
    neon_psd_init_frame(neon, input.data());
    for (int iteration = 0; iteration < 16; ++iteration) {
        for (float &value : input)
            value = std::fabs(random_float(4.0f)) + 1e-4f;
        scalar_psd_process(scalar, input.data(), noise.data());
        neon_psd_process(neon, input.data(), noise.data());
        std::vector<float> scalar_fd(scalar_psd_frequency(scalar),
                                     scalar_psd_frequency(scalar) + length);
        std::vector<float> neon_fd(neon_psd_frequency(neon),
                                   neon_psd_frequency(neon) + length);
        std::vector<float> scalar_td(scalar_psd_temporal(scalar),
                                     scalar_psd_temporal(scalar) + length);
        std::vector<float> neon_td(neon_psd_temporal(neon),
                                   neon_psd_temporal(neon) + length);
        std::vector<float> scalar_alpha(scalar_psd_alpha(scalar),
                                        scalar_psd_alpha(scalar) + length);
        std::vector<float> neon_alpha(neon_psd_alpha(neon),
                                      neon_psd_alpha(neon) + length);
        bool report = iteration == 15;
        if (!check_vector("PSDsmooth frequency",
                          scalar_fd,
                          neon_fd,
                          3e-6f,
                          report) ||
            !check_vector("PSDsmooth alpha",
                          scalar_alpha,
                          neon_alpha,
                          5e-6f,
                          report) ||
            !check_vector("PSDsmooth temporal",
                          scalar_td,
                          neon_td,
                          5e-6f,
                          report))
            return false;
    }
    return true;
}

using PowerUpdate = void (*)(int,
                             float *,
                             float *,
                             float *,
                             float *,
                             float *,
                             float *,
                             float *,
                             float *,
                             float);

static bool test_postfilter()
{
    const int lengths[] = {3, 4, 7, 64, 257};
    for (int length : lengths) {
        std::vector<float> input(2 * length);
        std::vector<float> estimate_input(2 * length);
        std::vector<float> desired_input(2 * length);
        std::vector<float> scalar_power(length);
        std::vector<float> neon_power(length);
        std::vector<float> scalar_after(length);
        std::vector<float> neon_after(length);
        std::vector<float> scalar_estimate_power(length);
        std::vector<float> neon_estimate_power(length);
        std::vector<float> scalar_estimate(length);
        std::vector<float> neon_estimate(length);
        std::vector<float> scalar_before(length);
        std::vector<float> neon_before(length);
        std::vector<float> scalar_reference(length);
        std::vector<float> neon_reference(length);

        for (float &value : input)
            value = random_float(2.0f);
        for (float &value : estimate_input)
            value = random_float(2.0f);
        for (float &value : desired_input)
            value = random_float(2.0f);
        for (int i = 0; i < length; ++i) {
            scalar_after[i] = neon_after[i] = random_float(1.0f);
            scalar_estimate[i] = neon_estimate[i] = random_float(1.0f);
            scalar_before[i] = neon_before[i] = random_float(1.0f);
        }

        scalar_postfilter_update_power(length,
                                       input.data(),
                                       estimate_input.data(),
                                       desired_input.data(),
                                       scalar_power.data(),
                                       scalar_after.data(),
                                       scalar_estimate_power.data(),
                                       scalar_estimate.data(),
                                       scalar_before.data(),
                                       0.3f);
        neon_postfilter_update_power(length,
                                     input.data(),
                                     estimate_input.data(),
                                     desired_input.data(),
                                     neon_power.data(),
                                     neon_after.data(),
                                     neon_estimate_power.data(),
                                     neon_estimate.data(),
                                     neon_before.data(),
                                     0.3f);
        scalar_postfilter_update_reference(
            length, input.data(), scalar_reference.data());
        neon_postfilter_update_reference(
            length, input.data(), neon_reference.data());

        if (!check_vector("PostFilter power",
                          scalar_power,
                          neon_power,
                          2e-6f,
                          length == 257) ||
            !check_vector("PostFilter after",
                          scalar_after,
                          neon_after,
                          2e-6f,
                          length == 257) ||
            !check_vector("PostFilter estimate power",
                          scalar_estimate_power,
                          neon_estimate_power,
                          2e-6f,
                          length == 257) ||
            !check_vector("PostFilter estimate",
                          scalar_estimate,
                          neon_estimate,
                          2e-6f,
                          length == 257) ||
            !check_vector("PostFilter before",
                          scalar_before,
                          neon_before,
                          2e-6f,
                          length == 257) ||
            !check_vector("PostFilter reference",
                          scalar_reference,
                          neon_reference,
                          2e-6f,
                          length == 257))
            return false;

        std::vector<float> gain(length);
        std::vector<float> scalar_limited(length);
        std::vector<float> neon_limited(length);
        for (int i = 0; i < length; ++i) {
            gain[i] = std::fabs(random_float(1.0f));
            scalar_after[i] = neon_after[i] =
                length == 3 ? 1e-10f : std::fabs(random_float(2.0f));
        }
        scalar_postfilter_limit(length,
                                scalar_after.data(),
                                gain.data(),
                                scalar_limited.data(),
                                0.01f,
                                1e-5f);
        neon_postfilter_limit(length,
                              neon_after.data(),
                              gain.data(),
                              neon_limited.data(),
                              0.01f,
                              1e-5f);
        if (!check_vector("PostFilter limiter",
                          scalar_limited,
                          neon_limited,
                          0.0f,
                          length == 257))
            return false;
    }

    constexpr int length = 257;
    std::vector<float> scalar_error(2 * length);
    std::vector<float> neon_error(2 * length);
    std::vector<float> gain(length);
    for (float &value : scalar_error)
        value = random_float(2.0f);
    neon_error = scalar_error;
    for (float &value : gain)
        value = std::fabs(random_float(1.0f));
    int cutoff = 11;
    for (int i = 0; i < length; ++i) {
        if (i < cutoff) {
            scalar_error[2 * i] = 0.0f;
            scalar_error[2 * i + 1] = 0.0f;
        } else {
            scalar_error[2 * i] *= gain[i];
            scalar_error[2 * i + 1] *= gain[i];
        }
    }
    apf_postfilter_apply_gain(
        neon_error.data(), gain.data(), length, cutoff, 1);
    return check_vector(
        "PostFilter apply gain", scalar_error, neon_error, 0.0f, true);
}

static bool test_adaptive_filter()
{
    constexpr int banks = 96;
    std::vector<int> taps(banks);
    std::vector<float> reference(2 * banks);
    std::vector<float> desired(2 * banks);
    std::vector<float> scalar_estimate(2 * banks);
    std::vector<float> neon_estimate(2 * banks);
    std::vector<float> scalar_error(2 * banks);
    std::vector<float> neon_error(2 * banks);
    for (int i = 0; i < banks; ++i)
        taps[i] = 5 + i % 20;

    void *scalar = scalar_adap_create(banks, taps.data(), 0.08f, 1.0f);
    void *neon = neon_adap_create(banks, taps.data(), 0.08f, 1.0f);
    for (int frame = 0; frame < 48; ++frame) {
        for (float &value : reference)
            value = random_float(0.5f);
        for (float &value : desired)
            value = random_float(0.5f);
        scalar_adap_process(scalar,
                            reference.data(),
                            desired.data(),
                            scalar_estimate.data(),
                            scalar_error.data(),
                            1);
        neon_adap_process(neon,
                          reference.data(),
                          desired.data(),
                          neon_estimate.data(),
                          neon_error.data(),
                          1);
        bool report = frame == 47;
        if (!check_vector("AdapFilter estimate",
                          scalar_estimate,
                          neon_estimate,
                          2e-5f,
                          report) ||
            !check_vector("AdapFilter error",
                          scalar_error,
                          neon_error,
                          2e-5f,
                          report))
            return false;
    }
    return true;
}

template <typename Function>
static uint64_t benchmark_ns(Function function)
{
    auto start = std::chrono::steady_clock::now();
    function();
    auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               finish - start)
        .count();
}

static void print_benchmark(const char *name,
                            uint64_t scalar_ns,
                            uint64_t neon_ns)
{
    std::printf("%s: scalar %.3f ms, NEON %.3f ms, speedup %.2fx\n",
                name,
                scalar_ns / 1000000.0,
                neon_ns / 1000000.0,
                neon_ns ? static_cast<double>(scalar_ns) / neon_ns : 0.0);
}

static void benchmark_dc_remover()
{
    constexpr int size = 1024;
    constexpr int iterations = 30000;
    ScalarDCRemover scalar(48000, size, 1);
    DCRemover neon(48000, size, 1);
    std::vector<float> scalar_data(size);
    std::vector<float> neon_data(size);
    for (int i = 0; i < size; ++i)
        scalar_data[i] = neon_data[i] = random_float(2.0f) + 0.25f;

    uint64_t scalar_ns = benchmark_ns([&] {
        for (int i = 0; i < iterations; ++i)
            scalar.findLevelAndDcRemove(scalar_data.data(), 0);
    });
    uint64_t neon_ns = benchmark_ns([&] {
        for (int i = 0; i < iterations; ++i)
            neon.findLevelAndDcRemove(neon_data.data(), 0);
    });
    print_benchmark("DCRemover", scalar_ns, neon_ns);
}

static void benchmark_psd_smooth()
{
    constexpr int fft_length = 1024;
    constexpr int iterations = 30000;
    void *scalar = scalar_psd_create(fft_length, 16000);
    void *neon = neon_psd_create(fft_length, 16000);
    std::vector<float> input(fft_length / 2);
    for (float &value : input)
        value = std::fabs(random_float(4.0f)) + 1e-4f;

    uint64_t scalar_ns = benchmark_ns([&] {
        for (int i = 0; i < iterations; ++i)
            scalar_psd_process(scalar, input.data(), nullptr);
    });
    uint64_t neon_ns = benchmark_ns([&] {
        for (int i = 0; i < iterations; ++i)
            neon_psd_process(neon, input.data(), nullptr);
    });
    print_benchmark("PSDsmooth", scalar_ns, neon_ns);
}

static uint64_t benchmark_postfilter_power(PowerUpdate update)
{
    constexpr int length = 1024;
    constexpr int iterations = 30000;
    std::vector<float> input(2 * length);
    std::vector<float> estimate_input(2 * length);
    std::vector<float> desired_input(2 * length);
    std::vector<float> power(length);
    std::vector<float> after(length);
    std::vector<float> estimate_power(length);
    std::vector<float> estimate(length);
    std::vector<float> before(length);
    for (float &value : input)
        value = random_float(2.0f);
    estimate_input = desired_input = input;
    return benchmark_ns([&] {
        for (int i = 0; i < iterations; ++i)
            update(length,
                   input.data(),
                   estimate_input.data(),
                   desired_input.data(),
                   power.data(),
                   after.data(),
                   estimate_power.data(),
                   estimate.data(),
                   before.data(),
                   0.3f);
    });
}

static void benchmark_postfilter()
{
    uint64_t scalar_ns =
        benchmark_postfilter_power(scalar_postfilter_update_power);
    uint64_t neon_ns =
        benchmark_postfilter_power(neon_postfilter_update_power);
    print_benchmark("PostFilter UpdatePwr", scalar_ns, neon_ns);
}

using AdapProcess =
    void (*)(void *, const float *, const float *, float *, float *, int);

static uint64_t benchmark_adaptive_filter(AdapProcess process,
                                          void *instance,
                                          const std::vector<float> &reference,
                                          const std::vector<float> &desired)
{
    constexpr int iterations = 10000;
    std::vector<float> estimate(reference.size());
    std::vector<float> error(reference.size());
    return benchmark_ns([&] {
        for (int i = 0; i < iterations; ++i)
            process(instance,
                    reference.data(),
                    desired.data(),
                    estimate.data(),
                    error.data(),
                    1);
    });
}

static void benchmark_adaptive_filter()
{
    constexpr int banks = 96;
    std::vector<int> taps(banks, 16);
    std::vector<float> reference(2 * banks);
    std::vector<float> desired(2 * banks);
    for (float &value : reference)
        value = random_float(0.1f);
    for (float &value : desired)
        value = random_float(0.1f);
    void *scalar = scalar_adap_create(banks, taps.data(), 0.02f, 1.0f);
    void *neon = neon_adap_create(banks, taps.data(), 0.02f, 1.0f);
    uint64_t scalar_ns = benchmark_adaptive_filter(
        scalar_adap_process, scalar, reference, desired);
    uint64_t neon_ns = benchmark_adaptive_filter(
        neon_adap_process, neon, reference, desired);
    print_benchmark("AdapFilter process", scalar_ns, neon_ns);
}

int main()
{
    if (!test_spest() || !test_dc_remover() || !test_psd_smooth() ||
        !test_postfilter() || !test_adaptive_filter())
        return 1;
    std::puts("ARM64 C++ parity tests passed");
    benchmark_dc_remover();
    benchmark_psd_smooth();
    benchmark_postfilter();
    benchmark_adaptive_filter();
    return 0;
}
