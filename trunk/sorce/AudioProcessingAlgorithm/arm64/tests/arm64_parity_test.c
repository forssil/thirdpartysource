#define _POSIX_C_SOURCE 200809L

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Amp_compute.h"
#include "celt_lpc.h"
#include "kiss_fft.h"
#include "rnn.h"
#include "rnn_data.h"
#include "../../common_audio/SampleRateConvert/include/common.h"

#define DENOISE_BANDS 22
#define DENOISE_FREQ_SIZE 481

void scalar_Clipping(float *sample_value);
float scalar_GetPowerLevel(float *x, int len);
float scalar_GetPowerEnergy(float *x, int len);
float scalar_GetPowerdB(float *x, int len, float zero_db_value);
float scalar_GetPowerdBShort(short *x, int len, int zero_db_value);
int scalar_ChangeDbToWebExLevel(float db);
void scalar_ShortToFloat(short *src, float *dst, int num_samples);
void scalar_FloatToShort(float *src, short *dst, int num_samples);
void scalar_compute_dense(const DenseLayer *layer,
                          float *output,
                          const float *input);
void scalar_compute_gru(const GRULayer *gru,
                        float *state,
                        const float *input);
void scalar_compute_rnn(RNNState *rnn,
                        float *gains,
                        float *vad,
                        const float *input);
void scalar_compute_band_energy(float *bandE, const kiss_fft_cpx *X);
void scalar_compute_band_corr(float *bandE,
                              const kiss_fft_cpx *X,
                              const kiss_fft_cpx *P);
void scalar_interp_band_gain(float *g, const float *bandE);
void scalar_pitch_filter(kiss_fft_cpx *X,
                         const kiss_fft_cpx *P,
                         const float *Ex,
                         const float *Ep,
                         const float *Exp,
                         const float *g);
void compute_band_energy(float *bandE, const kiss_fft_cpx *X);
void compute_band_corr(float *bandE,
                       const kiss_fft_cpx *X,
                       const kiss_fft_cpx *P);
void interp_band_gain(float *g, const float *bandE);
void pitch_filter(kiss_fft_cpx *X,
                  const kiss_fft_cpx *P,
                  const float *Ex,
                  const float *Ep,
                  const float *Exp,
                  const float *g);
void scalar_celt_fir(const opus_val16 *x,
                     const opus_val16 *num,
                     opus_val16 *y,
                     int N,
                     int ord);
int scalar_celt_autocorr(const opus_val16 *x,
                         opus_val32 *ac,
                         const opus_val16 *window,
                         int overlap,
                         int lag,
                         int n);
int scalar_linear_set_converter(SRC_PRIVATE *psrc, int src_enum);

void celt_pitch_xcorr(const opus_val16 *x,
                      const opus_val16 *y,
                      opus_val32 *xcorr,
                      int len,
                      int max_pitch)
{
    int i;
    int j;
    for (i = 0; i < max_pitch; ++i) {
        opus_val32 sum = 0;
        for (j = 0; j < len; ++j)
            sum = MAC16_16(sum, x[j], y[i + j]);
        xcorr[i] = sum;
    }
}

static uint32_t random_state = 0x12345678u;

static uint32_t next_random(void)
{
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}

static float random_float(float scale)
{
    int32_t value = (int32_t)(next_random() >> 1);
    return ((float)value / 1073741824.0f - 1.0f) * scale;
}

static int close_float(float lhs, float rhs, float tolerance)
{
    if (isnan(lhs) || isnan(rhs))
        return isnan(lhs) && isnan(rhs);
    float scale = fmaxf(1.0f, fmaxf(fabsf(lhs), fabsf(rhs)));
    return fabsf(lhs - rhs) <= tolerance * scale;
}

static void print_error(const char *name,
                        float max_absolute,
                        float max_relative)
{
    printf("%-24s max_abs=%-12.6g max_rel=%.6g\n",
           name,
           max_absolute,
           max_relative);
}

static int check_vector(const char *name,
                        const float *expected,
                        const float *actual,
                        int length,
                        float tolerance,
                        int report)
{
    int i;
    float max_absolute = 0.0f;
    float max_relative = 0.0f;
    for (i = 0; i < length; ++i) {
        float absolute = fabsf(expected[i] - actual[i]);
        float scale = fmaxf(1.0f, fmaxf(fabsf(expected[i]), fabsf(actual[i])));
        float relative = absolute / scale;
        max_absolute = fmaxf(max_absolute, absolute);
        max_relative = fmaxf(max_relative, relative);
        if (!close_float(expected[i], actual[i], tolerance)) {
            fprintf(stderr,
                    "%s mismatch at %d: expected %.9g, got %.9g\n",
                    name,
                    i,
                    expected[i],
                    actual[i]);
            return 1;
        }
    }
    if (report)
        print_error(name, max_absolute, max_relative);
    return 0;
}

static int test_amplitude(void)
{
    enum { SAMPLE_COUNT = 257 };
    float samples[SAMPLE_COUNT];
    short pcm[SAMPLE_COUNT];
    float scalar_float[SAMPLE_COUNT];
    float neon_float[SAMPLE_COUNT];
    float scalar_input[SAMPLE_COUNT];
    float neon_input[SAMPLE_COUNT];
    short scalar_pcm[SAMPLE_COUNT];
    short neon_pcm[SAMPLE_COUNT];
    const int lengths[] = {-3, 0, 1, 3, 4, 7, 8, 15, 64, SAMPLE_COUNT};
    int i;
    int length_index;
    float max_level_abs = 0.0f;
    float max_energy_abs = 0.0f;
    float max_db_abs = 0.0f;
    float max_db_short_abs = 0.0f;
    const float clip_inputs[] = {
        -INFINITY, -1.5f, -1.25f, -1.249f, -0.751f, -0.75f, -0.0f,
        0.0f, 0.75f, 0.751f, 1.249f, 1.25f, 1.5f, INFINITY
    };
    const float db_inputs[] = {
        -100.0f, -60.0f, -59.9f, -45.0f, -40.0f, -35.0f, -30.0f,
        -25.0f, -20.0f, -15.0f, -10.0f, -5.0f, -0.000001f, 0.0f, 3.0f
    };

    for (i = 0; i < SAMPLE_COUNT; ++i) {
        samples[i] = random_float(1.25f);
        pcm[i] = (short)next_random();
    }
    samples[0] = -1.5f;
    samples[1] = -1.0f;
    samples[2] = 0.0f;
    samples[3] = 1.0f;
    samples[4] = 1.5f;
    pcm[0] = -32768;
    pcm[1] = 32767;

    for (i = 0; i < (int)(sizeof(clip_inputs) / sizeof(clip_inputs[0])); ++i) {
        float scalar_value = clip_inputs[i];
        float neon_value = clip_inputs[i];
        scalar_Clipping(&scalar_value);
        Clipping(&neon_value);
        if (!close_float(scalar_value, neon_value, 0.0f)) {
            fprintf(stderr,
                    "Clipping mismatch for %.9g: expected %.9g, got %.9g\n",
                    clip_inputs[i],
                    scalar_value,
                    neon_value);
            return 1;
        }
    }
    print_error("Clipping", 0.0f, 0.0f);

    for (i = 0; i < (int)(sizeof(db_inputs) / sizeof(db_inputs[0])); ++i) {
        int scalar_level = scalar_ChangeDbToWebExLevel(db_inputs[i]);
        int neon_level = ChangeDbToWebExLevel(db_inputs[i]);
        if (scalar_level != neon_level) {
            fprintf(stderr,
                    "ChangeDbToWebExLevel mismatch for %.9g: %d != %d\n",
                    db_inputs[i],
                    scalar_level,
                    neon_level);
            return 1;
        }
    }
    print_error("ChangeDbToWebExLevel", 0.0f, 0.0f);

    for (length_index = 0;
         length_index < (int)(sizeof(lengths) / sizeof(lengths[0]));
         ++length_index) {
        int length = lengths[length_index];
        float scalar_level = scalar_GetPowerLevel(samples, length);
        float neon_level = GetPowerLevel(samples, length);
        float scalar_energy = scalar_GetPowerEnergy(samples, length);
        float neon_energy = GetPowerEnergy(samples, length);
        float scalar_db = scalar_GetPowerdB(samples, length, 1.0f);
        float neon_db = GetPowerdB(samples, length, 1.0f);
        float scalar_db_short =
            scalar_GetPowerdBShort(pcm, length, 32768);
        float neon_db_short = GetPowerdBShort(pcm, length, 32768);

        max_level_abs =
            fmaxf(max_level_abs, fabsf(scalar_level - neon_level));
        max_energy_abs =
            fmaxf(max_energy_abs, fabsf(scalar_energy - neon_energy));
        max_db_abs = fmaxf(max_db_abs, fabsf(scalar_db - neon_db));
        max_db_short_abs =
            fmaxf(max_db_short_abs,
                  fabsf(scalar_db_short - neon_db_short));
        if (!close_float(scalar_level, neon_level, 2e-6f) ||
            !close_float(scalar_energy, neon_energy, 2e-6f) ||
            !close_float(scalar_db, neon_db, 2e-6f) ||
            !close_float(scalar_db_short, neon_db_short, 2e-6f)) {
            fprintf(stderr, "amplitude reduction mismatch at length %d\n", length);
            return 1;
        }
    }
    print_error("GetPowerLevel", max_level_abs, max_level_abs);
    print_error("GetPowerEnergy", max_energy_abs, max_energy_abs);
    print_error("GetPowerdB", max_db_abs, max_db_abs);
    print_error("GetPowerdBShort", max_db_short_abs, max_db_short_abs);

    scalar_ShortToFloat(pcm, scalar_float, SAMPLE_COUNT);
    ShortToFloat(pcm, neon_float, SAMPLE_COUNT);
    if (check_vector("ShortToFloat",
                     scalar_float,
                     neon_float,
                     SAMPLE_COUNT,
                     0.0f,
                     1))
        return 1;

    memcpy(scalar_input, samples, sizeof(samples));
    memcpy(neon_input, samples, sizeof(samples));
    scalar_FloatToShort(scalar_input, scalar_pcm, SAMPLE_COUNT);
    FloatToShort(neon_input, neon_pcm, SAMPLE_COUNT);
    if (memcmp(scalar_pcm, neon_pcm, sizeof(scalar_pcm)) != 0) {
        fprintf(stderr, "FloatToShort PCM output mismatch\n");
        return 1;
    }
    print_error("FloatToShort PCM", 0.0f, 0.0f);
    return check_vector("FloatToShort input",
                        scalar_input,
                        neon_input,
                        SAMPLE_COUNT,
                        0.0f,
                        1);
}

static int run_linear_case(int channels, double ratio, int iterations)
{
    enum { INPUT_FRAMES = 513, OUTPUT_FRAMES = 769, MAX_CHANNELS = 8 };
    SRC_PRIVATE scalar_state;
    SRC_PRIVATE neon_state;
    SRC_DATA scalar_data;
    SRC_DATA neon_data;
    float input[INPUT_FRAMES * MAX_CHANNELS];
    float scalar_output[OUTPUT_FRAMES * MAX_CHANNELS];
    float neon_output[OUTPUT_FRAMES * MAX_CHANNELS];
    int sample_count = INPUT_FRAMES * channels;
    int output_count = OUTPUT_FRAMES * channels;
    int iteration;
    int i;

    memset(&scalar_state, 0, sizeof(scalar_state));
    memset(&neon_state, 0, sizeof(neon_state));
    scalar_state.channels = channels;
    neon_state.channels = channels;
    if (scalar_linear_set_converter(&scalar_state, SRC_LINEAR) ||
        linear_set_converter(&neon_state, SRC_LINEAR)) {
        fprintf(stderr, "linear converter setup failed\n");
        return 1;
    }
    scalar_state.last_ratio = neon_state.last_ratio = ratio;

    for (i = 0; i < sample_count; ++i)
        input[i] = random_float(2.0f);

    for (iteration = 0; iteration < iterations; ++iteration) {
        memset(scalar_output, 0, output_count * sizeof(float));
        memset(neon_output, 0, output_count * sizeof(float));
        memset(&scalar_data, 0, sizeof(scalar_data));
        memset(&neon_data, 0, sizeof(neon_data));
        scalar_data.data_in = neon_data.data_in = input;
        scalar_data.data_out = scalar_output;
        neon_data.data_out = neon_output;
        scalar_data.input_frames = neon_data.input_frames = INPUT_FRAMES;
        scalar_data.output_frames = neon_data.output_frames = OUTPUT_FRAMES;
        scalar_data.src_ratio = neon_data.src_ratio =
            ratio + iteration * 0.0001;

        if (scalar_state.vari_process(&scalar_state, &scalar_data) ||
            neon_state.vari_process(&neon_state, &neon_data)) {
            fprintf(stderr, "linear converter processing failed\n");
            return 1;
        }
        if (scalar_data.input_frames_used != neon_data.input_frames_used ||
            scalar_data.output_frames_gen != neon_data.output_frames_gen ||
            !close_float((float)scalar_state.last_position,
                         (float)neon_state.last_position,
                         0.0f) ||
            check_vector("src_linear",
                         scalar_output,
                         neon_output,
                         (int)(scalar_data.output_frames_gen * channels),
                         2e-6f,
                         iteration + 1 == iterations)) {
            fprintf(stderr,
                    "src_linear state mismatch for %d channels at iteration %d\n",
                    channels,
                    iteration);
            return 1;
        }
    }

    free(scalar_state.private_data);
    free(neon_state.private_data);
    return 0;
}

static int test_src_linear(void)
{
    const int channels[] = {1, 2, 4, 8};
    int i;
    for (i = 0; i < (int)(sizeof(channels) / sizeof(channels[0])); ++i) {
        if (run_linear_case(channels[i], 1.37, 4))
            return 1;
    }
    return 0;
}

static void fill_weights(rnn_weight *weights, int length)
{
    int i;
    for (i = 0; i < length; ++i)
        weights[i] = (rnn_weight)((next_random() % 255u) - 127);
}

static int test_dense(void)
{
    enum { INPUTS = 42, NEURONS = 22 };
    rnn_weight bias[NEURONS];
    rnn_weight weights[INPUTS * NEURONS];
    float input[INPUTS];
    float scalar_output[NEURONS];
    float neon_output[NEURONS];
    DenseLayer layer = {bias, weights, INPUTS, NEURONS, ACTIVATION_TANH};
    int i;

    fill_weights(bias, NEURONS);
    fill_weights(weights, INPUTS * NEURONS);
    for (i = 0; i < INPUTS; ++i)
        input[i] = random_float(2.0f);

    scalar_compute_dense(&layer, scalar_output, input);
    compute_dense(&layer, neon_output, input);
    return check_vector("compute_dense",
                        scalar_output,
                        neon_output,
                        NEURONS,
                        2e-5f,
                        1);
}

static int test_gru(void)
{
    enum { INPUTS = 90, NEURONS = 48 };
    rnn_weight bias[3 * NEURONS];
    rnn_weight input_weights[3 * INPUTS * NEURONS];
    rnn_weight recurrent_weights[3 * NEURONS * NEURONS];
    float input[INPUTS];
    float scalar_state[NEURONS];
    float neon_state[NEURONS];
    GRULayer layer = {
        bias,
        input_weights,
        recurrent_weights,
        INPUTS,
        NEURONS,
        ACTIVATION_TANH
    };
    int i;

    fill_weights(bias, 3 * NEURONS);
    fill_weights(input_weights, 3 * INPUTS * NEURONS);
    fill_weights(recurrent_weights, 3 * NEURONS * NEURONS);
    for (i = 0; i < INPUTS; ++i)
        input[i] = random_float(2.0f);
    for (i = 0; i < NEURONS; ++i)
        scalar_state[i] = neon_state[i] = random_float(1.0f);

    scalar_compute_gru(&layer, scalar_state, input);
    compute_gru(&layer, neon_state, input);
    return check_vector("compute_gru",
                        scalar_state,
                        neon_state,
                        NEURONS,
                        3e-5f,
                        1);
}

static int test_rnn(void)
{
    enum {
        INPUT_SIZE = 42,
        INPUT_DENSE_SIZE = 8,
        VAD_GRU_SIZE = 6,
        NOISE_GRU_SIZE = 7,
        DENOISE_GRU_SIZE = 9,
        GAIN_SIZE = 22,
        NOISE_INPUT_SIZE = INPUT_DENSE_SIZE + VAD_GRU_SIZE + INPUT_SIZE,
        DENOISE_INPUT_SIZE = VAD_GRU_SIZE + NOISE_GRU_SIZE + INPUT_SIZE
    };
    rnn_weight input_dense_bias[INPUT_DENSE_SIZE];
    rnn_weight input_dense_weights[INPUT_SIZE * INPUT_DENSE_SIZE];
    rnn_weight vad_gru_bias[3 * VAD_GRU_SIZE];
    rnn_weight vad_gru_input[3 * INPUT_DENSE_SIZE * VAD_GRU_SIZE];
    rnn_weight vad_gru_recurrent[3 * VAD_GRU_SIZE * VAD_GRU_SIZE];
    rnn_weight vad_output_bias[1];
    rnn_weight vad_output_weights[VAD_GRU_SIZE];
    rnn_weight noise_gru_bias[3 * NOISE_GRU_SIZE];
    rnn_weight noise_gru_input[3 * NOISE_INPUT_SIZE * NOISE_GRU_SIZE];
    rnn_weight noise_gru_recurrent[3 * NOISE_GRU_SIZE * NOISE_GRU_SIZE];
    rnn_weight denoise_gru_bias[3 * DENOISE_GRU_SIZE];
    rnn_weight denoise_gru_input[3 * DENOISE_INPUT_SIZE * DENOISE_GRU_SIZE];
    rnn_weight denoise_gru_recurrent[3 * DENOISE_GRU_SIZE * DENOISE_GRU_SIZE];
    rnn_weight denoise_output_bias[GAIN_SIZE];
    rnn_weight denoise_output_weights[DENOISE_GRU_SIZE * GAIN_SIZE];
    float input[INPUT_SIZE];
    float scalar_vad_state[VAD_GRU_SIZE];
    float neon_vad_state[VAD_GRU_SIZE];
    float scalar_noise_state[NOISE_GRU_SIZE];
    float neon_noise_state[NOISE_GRU_SIZE];
    float scalar_denoise_state[DENOISE_GRU_SIZE];
    float neon_denoise_state[DENOISE_GRU_SIZE];
    float scalar_gains[GAIN_SIZE];
    float neon_gains[GAIN_SIZE];
    float scalar_vad;
    float neon_vad;
    DenseLayer input_dense = {
        input_dense_bias, input_dense_weights,
        INPUT_SIZE, INPUT_DENSE_SIZE, ACTIVATION_TANH
    };
    GRULayer vad_gru = {
        vad_gru_bias, vad_gru_input, vad_gru_recurrent,
        INPUT_DENSE_SIZE, VAD_GRU_SIZE, ACTIVATION_TANH
    };
    DenseLayer vad_output = {
        vad_output_bias, vad_output_weights,
        VAD_GRU_SIZE, 1, ACTIVATION_SIGMOID
    };
    GRULayer noise_gru = {
        noise_gru_bias, noise_gru_input, noise_gru_recurrent,
        NOISE_INPUT_SIZE, NOISE_GRU_SIZE, ACTIVATION_RELU
    };
    GRULayer denoise_gru = {
        denoise_gru_bias, denoise_gru_input, denoise_gru_recurrent,
        DENOISE_INPUT_SIZE, DENOISE_GRU_SIZE, ACTIVATION_TANH
    };
    DenseLayer denoise_output = {
        denoise_output_bias, denoise_output_weights,
        DENOISE_GRU_SIZE, GAIN_SIZE, ACTIVATION_SIGMOID
    };
    RNNModel model = {
        INPUT_DENSE_SIZE, &input_dense,
        VAD_GRU_SIZE, &vad_gru,
        NOISE_GRU_SIZE, &noise_gru,
        DENOISE_GRU_SIZE, &denoise_gru,
        GAIN_SIZE, &denoise_output,
        1, &vad_output
    };
    RNNState scalar_state = {
        &model, scalar_vad_state, scalar_noise_state, scalar_denoise_state
    };
    RNNState neon_state = {
        &model, neon_vad_state, neon_noise_state, neon_denoise_state
    };
    int i;
    int iteration;

#define FILL(array) fill_weights((array), (int)(sizeof(array) / sizeof((array)[0])))
    FILL(input_dense_bias);
    FILL(input_dense_weights);
    FILL(vad_gru_bias);
    FILL(vad_gru_input);
    FILL(vad_gru_recurrent);
    FILL(vad_output_bias);
    FILL(vad_output_weights);
    FILL(noise_gru_bias);
    FILL(noise_gru_input);
    FILL(noise_gru_recurrent);
    FILL(denoise_gru_bias);
    FILL(denoise_gru_input);
    FILL(denoise_gru_recurrent);
    FILL(denoise_output_bias);
    FILL(denoise_output_weights);
#undef FILL

    for (i = 0; i < VAD_GRU_SIZE; ++i)
        scalar_vad_state[i] = neon_vad_state[i] = random_float(0.25f);
    for (i = 0; i < NOISE_GRU_SIZE; ++i)
        scalar_noise_state[i] = neon_noise_state[i] = random_float(0.25f);
    for (i = 0; i < DENOISE_GRU_SIZE; ++i)
        scalar_denoise_state[i] =
            neon_denoise_state[i] = random_float(0.25f);

    for (iteration = 0; iteration < 12; ++iteration) {
        for (i = 0; i < INPUT_SIZE; ++i)
            input[i] = random_float(2.0f);
        scalar_compute_rnn(&scalar_state,
                           scalar_gains,
                           &scalar_vad,
                           input);
        compute_rnn(&neon_state, neon_gains, &neon_vad, input);
        if (check_vector("compute_rnn gains",
                         scalar_gains,
                         neon_gains,
                         GAIN_SIZE,
                         2e-4f,
                         0) ||
            check_vector("compute_rnn vad state",
                         scalar_vad_state,
                         neon_vad_state,
                         VAD_GRU_SIZE,
                         2e-4f,
                         0) ||
            check_vector("compute_rnn noise state",
                         scalar_noise_state,
                         neon_noise_state,
                         NOISE_GRU_SIZE,
                         2e-4f,
                         0) ||
            check_vector("compute_rnn denoise state",
                         scalar_denoise_state,
                         neon_denoise_state,
                         DENOISE_GRU_SIZE,
                         2e-4f,
                         0) ||
            !close_float(scalar_vad, neon_vad, 2e-4f)) {
            fprintf(stderr,
                    "compute_rnn mismatch at iteration %d: vad %.9g != %.9g\n",
                    iteration,
                    scalar_vad,
                    neon_vad);
            return 1;
        }
    }
    check_vector("compute_rnn gains",
                 scalar_gains,
                 neon_gains,
                 GAIN_SIZE,
                 2e-4f,
                 1);
    check_vector("compute_rnn vad state",
                 scalar_vad_state,
                 neon_vad_state,
                 VAD_GRU_SIZE,
                 2e-4f,
                 1);
    check_vector("compute_rnn noise state",
                 scalar_noise_state,
                 neon_noise_state,
                 NOISE_GRU_SIZE,
                 2e-4f,
                 1);
    check_vector("compute_rnn denoise state",
                 scalar_denoise_state,
                 neon_denoise_state,
                 DENOISE_GRU_SIZE,
                 2e-4f,
                 1);
    print_error("compute_rnn vad",
                fabsf(scalar_vad - neon_vad),
                fabsf(scalar_vad - neon_vad));
    return 0;
}

static int test_denoise_primitives(void)
{
    kiss_fft_cpx x[DENOISE_FREQ_SIZE];
    kiss_fft_cpx p[DENOISE_FREQ_SIZE];
    kiss_fft_cpx scalar_filtered[DENOISE_FREQ_SIZE];
    kiss_fft_cpx neon_filtered[DENOISE_FREQ_SIZE];
    float scalar_energy[DENOISE_BANDS];
    float neon_energy[DENOISE_BANDS];
    float scalar_corr[DENOISE_BANDS];
    float neon_corr[DENOISE_BANDS];
    float band_gain[DENOISE_BANDS];
    float ep[DENOISE_BANDS];
    float exp[DENOISE_BANDS];
    float scalar_gain[DENOISE_FREQ_SIZE];
    float neon_gain[DENOISE_FREQ_SIZE];
    int i;

    for (i = 0; i < DENOISE_FREQ_SIZE; ++i) {
        x[i].r = random_float(2.0f);
        x[i].i = random_float(2.0f);
        p[i].r = random_float(2.0f);
        p[i].i = random_float(2.0f);
        scalar_gain[i] = neon_gain[i] = random_float(1.0f);
    }
    for (i = 0; i < DENOISE_BANDS; ++i) {
        band_gain[i] = 0.1f + 0.8f * fabsf(random_float(1.0f));
        ep[i] = 0.1f + fabsf(random_float(3.0f));
        exp[i] = 0.8f * fabsf(random_float(1.0f));
    }

    scalar_compute_band_energy(scalar_energy, x);
    compute_band_energy(neon_energy, x);
    if (check_vector("compute_band_energy",
                     scalar_energy,
                     neon_energy,
                     DENOISE_BANDS,
                     3e-6f,
                     1))
        return 1;

    scalar_compute_band_corr(scalar_corr, x, p);
    compute_band_corr(neon_corr, x, p);
    if (check_vector("compute_band_corr",
                     scalar_corr,
                     neon_corr,
                     DENOISE_BANDS,
                     3e-6f,
                     1))
        return 1;

    scalar_interp_band_gain(scalar_gain, band_gain);
    interp_band_gain(neon_gain, band_gain);
    if (check_vector("interp_band_gain",
                     scalar_gain,
                     neon_gain,
                     DENOISE_FREQ_SIZE,
                     1e-6f,
                     1))
        return 1;

    memcpy(scalar_filtered, x, sizeof(x));
    memcpy(neon_filtered, x, sizeof(x));
    scalar_pitch_filter(scalar_filtered,
                        p,
                        scalar_energy,
                        ep,
                        exp,
                        band_gain);
    pitch_filter(neon_filtered,
                 p,
                 scalar_energy,
                 ep,
                 exp,
                 band_gain);
    return check_vector("pitch_filter",
                        (const float *)scalar_filtered,
                        (const float *)neon_filtered,
                        2 * DENOISE_FREQ_SIZE,
                        1e-5f,
                        1);
}

static int test_celt_lpc_primitives(void)
{
    enum { N = 257, ORDER = 24, LAG = 48, OVERLAP = 80 };
    opus_val16 history[N + ORDER];
    opus_val16 coefficients[ORDER];
    opus_val16 scalar_fir[N];
    opus_val16 neon_fir[N];
    opus_val16 window[OVERLAP];
    opus_val32 scalar_ac[LAG + 1];
    opus_val32 neon_ac[LAG + 1];
    int scalar_shift;
    int neon_shift;
    int i;

    for (i = 0; i < N + ORDER; ++i)
        history[i] = random_float(1.0f);
    for (i = 0; i < ORDER; ++i)
        coefficients[i] = random_float(0.08f);
    for (i = 0; i < OVERLAP; ++i)
        window[i] = (float)(i + 1) / (float)(OVERLAP + 1);

    scalar_celt_fir(history + ORDER,
                    coefficients,
                    scalar_fir,
                    N,
                    ORDER);
    celt_fir(history + ORDER, coefficients, neon_fir, N, ORDER);
    if (check_vector("celt_fir",
                     scalar_fir,
                     neon_fir,
                     N,
                     3e-6f,
                     1))
        return 1;

    scalar_shift = scalar_celt_autocorr(history + ORDER,
                                        scalar_ac,
                                        window,
                                        OVERLAP,
                                        LAG,
                                        N);
    neon_shift = _celt_autocorr(history + ORDER,
                                neon_ac,
                                window,
                                OVERLAP,
                                LAG,
                                N);
    if (scalar_shift != neon_shift) {
        fprintf(stderr,
                "_celt_autocorr shift mismatch: %d != %d\n",
                scalar_shift,
                neon_shift);
        return 1;
    }
    return check_vector("_celt_autocorr",
                        scalar_ac,
                        neon_ac,
                        LAG + 1,
                        3e-6f,
                        1);
}

static uint64_t monotonic_ns(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000000u + (uint64_t)now.tv_nsec;
}

static void print_benchmark(const char *name,
                            uint64_t scalar_ns,
                            uint64_t neon_ns)
{
    printf("%s: scalar %.3f ms, NEON %.3f ms, speedup %.2fx\n",
           name,
           scalar_ns / 1000000.0,
           neon_ns / 1000000.0,
           neon_ns ? (double)scalar_ns / (double)neon_ns : 0.0);
}

static void benchmark_amplitude(void)
{
    enum { SAMPLE_COUNT = 1024, ITERATIONS = 20000 };
    float samples[SAMPLE_COUNT];
    volatile float sink = 0;
    uint64_t start;
    uint64_t scalar_ns;
    uint64_t neon_ns;
    int i;

    for (i = 0; i < SAMPLE_COUNT; ++i)
        samples[i] = random_float(1.0f);

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        sink += scalar_GetPowerEnergy(samples, SAMPLE_COUNT);
    scalar_ns = monotonic_ns() - start;

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        sink += GetPowerEnergy(samples, SAMPLE_COUNT);
    neon_ns = monotonic_ns() - start;

    if (sink == -1)
        puts("unreachable");
    print_benchmark("GetPowerEnergy", scalar_ns, neon_ns);
}

static void benchmark_dense(void)
{
    enum { INPUTS = 90, NEURONS = 48, ITERATIONS = 5000 };
    rnn_weight bias[NEURONS];
    rnn_weight weights[INPUTS * NEURONS];
    float input[INPUTS];
    float output[NEURONS];
    DenseLayer layer = {bias, weights, INPUTS, NEURONS, ACTIVATION_TANH};
    uint64_t start;
    uint64_t scalar_ns;
    uint64_t neon_ns;
    int i;

    fill_weights(bias, NEURONS);
    fill_weights(weights, INPUTS * NEURONS);
    for (i = 0; i < INPUTS; ++i)
        input[i] = random_float(2.0f);

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        scalar_compute_dense(&layer, output, input);
    scalar_ns = monotonic_ns() - start;

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        compute_dense(&layer, output, input);
    neon_ns = monotonic_ns() - start;

    if (output[0] == 12345)
        puts("unreachable");
    print_benchmark("compute_dense", scalar_ns, neon_ns);
}

static void benchmark_gru(void)
{
    enum { INPUTS = 90, NEURONS = 48, ITERATIONS = 500 };
    rnn_weight bias[3 * NEURONS];
    rnn_weight input_weights[3 * INPUTS * NEURONS];
    rnn_weight recurrent_weights[3 * NEURONS * NEURONS];
    float input[INPUTS];
    float scalar_state[NEURONS];
    float neon_state[NEURONS];
    GRULayer layer = {
        bias,
        input_weights,
        recurrent_weights,
        INPUTS,
        NEURONS,
        ACTIVATION_TANH
    };
    uint64_t start;
    uint64_t scalar_ns;
    uint64_t neon_ns;
    int i;

    fill_weights(bias, 3 * NEURONS);
    fill_weights(input_weights, 3 * INPUTS * NEURONS);
    fill_weights(recurrent_weights, 3 * NEURONS * NEURONS);
    for (i = 0; i < INPUTS; ++i)
        input[i] = random_float(2.0f);
    for (i = 0; i < NEURONS; ++i)
        scalar_state[i] = neon_state[i] = random_float(1.0f);

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        scalar_compute_gru(&layer, scalar_state, input);
    scalar_ns = monotonic_ns() - start;

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        compute_gru(&layer, neon_state, input);
    neon_ns = monotonic_ns() - start;

    print_benchmark("compute_gru", scalar_ns, neon_ns);
}

static void benchmark_denoise_primitives(void)
{
    enum { ITERATIONS = 50000 };
    kiss_fft_cpx x[DENOISE_FREQ_SIZE];
    kiss_fft_cpx p[DENOISE_FREQ_SIZE];
    float bands[DENOISE_BANDS];
    float output[DENOISE_FREQ_SIZE];
    uint64_t start;
    uint64_t scalar_ns;
    uint64_t neon_ns;
    int i;

    for (i = 0; i < DENOISE_FREQ_SIZE; ++i) {
        x[i].r = random_float(2.0f);
        x[i].i = random_float(2.0f);
        p[i].r = random_float(2.0f);
        p[i].i = random_float(2.0f);
    }
    for (i = 0; i < DENOISE_BANDS; ++i)
        bands[i] = random_float(1.0f);

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        scalar_compute_band_energy(bands, x);
    scalar_ns = monotonic_ns() - start;
    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        compute_band_energy(bands, x);
    neon_ns = monotonic_ns() - start;
    print_benchmark("compute_band_energy", scalar_ns, neon_ns);

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        scalar_compute_band_corr(bands, x, p);
    scalar_ns = monotonic_ns() - start;
    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        compute_band_corr(bands, x, p);
    neon_ns = monotonic_ns() - start;
    print_benchmark("compute_band_corr", scalar_ns, neon_ns);

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        scalar_interp_band_gain(output, bands);
    scalar_ns = monotonic_ns() - start;
    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i)
        interp_band_gain(output, bands);
    neon_ns = monotonic_ns() - start;
    print_benchmark("interp_band_gain", scalar_ns, neon_ns);
    if (output[0] == 12345)
        puts("unreachable");
}

static void benchmark_celt_lpc_primitives(void)
{
    enum {
        N = 480,
        ORDER = 24,
        LAG = 48,
        OVERLAP = 80,
        FIR_ITERATIONS = 30000,
        AC_ITERATIONS = 5000
    };
    opus_val16 history[N + ORDER];
    opus_val16 coefficients[ORDER];
    opus_val16 output[N];
    opus_val16 window[OVERLAP];
    opus_val32 ac[LAG + 1];
    uint64_t start;
    uint64_t scalar_ns;
    uint64_t neon_ns;
    int i;

    for (i = 0; i < N + ORDER; ++i)
        history[i] = random_float(1.0f);
    for (i = 0; i < ORDER; ++i)
        coefficients[i] = random_float(0.08f);
    for (i = 0; i < OVERLAP; ++i)
        window[i] = (float)(i + 1) / (float)(OVERLAP + 1);

    start = monotonic_ns();
    for (i = 0; i < FIR_ITERATIONS; ++i)
        scalar_celt_fir(history + ORDER, coefficients, output, N, ORDER);
    scalar_ns = monotonic_ns() - start;
    start = monotonic_ns();
    for (i = 0; i < FIR_ITERATIONS; ++i)
        celt_fir(history + ORDER, coefficients, output, N, ORDER);
    neon_ns = monotonic_ns() - start;
    print_benchmark("celt_fir", scalar_ns, neon_ns);

    start = monotonic_ns();
    for (i = 0; i < AC_ITERATIONS; ++i)
        scalar_celt_autocorr(history + ORDER,
                             ac,
                             window,
                             OVERLAP,
                             LAG,
                             N);
    scalar_ns = monotonic_ns() - start;
    start = monotonic_ns();
    for (i = 0; i < AC_ITERATIONS; ++i)
        _celt_autocorr(history + ORDER,
                       ac,
                       window,
                       OVERLAP,
                       LAG,
                       N);
    neon_ns = monotonic_ns() - start;
    print_benchmark("_celt_autocorr", scalar_ns, neon_ns);
    if (output[0] + ac[0] == 12345)
        puts("unreachable");
}

typedef int (*linear_setter)(SRC_PRIVATE *, int);

static uint64_t time_src_linear(linear_setter setter)
{
    enum { CHANNELS = 8, INPUT_FRAMES = 513, OUTPUT_FRAMES = 769,
           ITERATIONS = 10000 };
    SRC_PRIVATE state;
    SRC_DATA data;
    float input[INPUT_FRAMES * CHANNELS];
    float output[OUTPUT_FRAMES * CHANNELS];
    uint64_t start;
    int i;

    memset(&state, 0, sizeof(state));
    memset(&data, 0, sizeof(data));
    state.channels = CHANNELS;
    setter(&state, SRC_LINEAR);
    for (i = 0; i < INPUT_FRAMES * CHANNELS; ++i)
        input[i] = random_float(2.0f);
    data.data_in = input;
    data.data_out = output;
    data.input_frames = INPUT_FRAMES;
    data.output_frames = OUTPUT_FRAMES;
    data.src_ratio = 1.37;

    start = monotonic_ns();
    for (i = 0; i < ITERATIONS; ++i) {
        state.reset(&state);
        state.last_ratio = data.src_ratio;
        state.last_position = 0.0;
        state.vari_process(&state, &data);
    }
    start = monotonic_ns() - start;
    if (output[0] == 12345.0f)
        puts("unreachable");
    free(state.private_data);
    return start;
}

static void benchmark_src_linear(void)
{
    uint64_t scalar_ns = time_src_linear(scalar_linear_set_converter);
    uint64_t neon_ns = time_src_linear(linear_set_converter);
    print_benchmark("src_linear 8ch", scalar_ns, neon_ns);
}

int main(void)
{
    if (test_amplitude() ||
        test_dense() ||
        test_gru() ||
        test_rnn() ||
        test_denoise_primitives() ||
        test_celt_lpc_primitives() ||
        test_src_linear())
        return 1;
    puts("ARM64 parity tests passed");
    benchmark_amplitude();
    benchmark_dense();
    benchmark_gru();
    benchmark_denoise_primitives();
    benchmark_celt_lpc_primitives();
    benchmark_src_linear();
    return 0;
}
