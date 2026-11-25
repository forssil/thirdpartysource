#include "arm64_optimization.h"

#include <arm_neon.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "opus_types.h"
#include "common.h"
#include "arch.h"
#include "tansig_table.h"
#include "rnn.h"
#include "rnn_data.h"

static OPUS_INLINE float tansig_approx(float x)
{
    int i;
    float y;
    float dy;
    float sign = 1;

    if (!(x < 8))
        return 1;
    if (!(x > -8))
        return -1;
#ifndef FIXED_POINT
    if (celt_isnan(x))
        return 0;
#endif
    if (x < 0) {
        x = -x;
        sign = -1;
    }
    i = (int)floor(.5f + 25 * x);
    x -= .04f * i;
    y = tansig_table[i];
    dy = 1 - y * y;
    y = y + x * dy * (1 - y * x);
    return sign * y;
}

static OPUS_INLINE float sigmoid_approx(float x)
{
    return .5f + .5f * tansig_approx(.5f * x);
}

static OPUS_INLINE float relu(float x)
{
    return x < 0 ? 0 : x;
}

static OPUS_INLINE float activate(float value, int activation)
{
    if (activation == ACTIVATION_SIGMOID)
        return sigmoid_approx(value);
    if (activation == ACTIVATION_TANH)
        return tansig_approx(value);
    if (activation == ACTIVATION_RELU)
        return relu(value);
    *(int *)0 = 0;
    return 0;
}

static OPUS_INLINE float32x4_t load_weights4(const rnn_weight *weights)
{
    uint32_t packed;
    memcpy(&packed, weights, sizeof(packed));
    int8x8_t bytes = vcreate_s8((uint64_t)packed);
    int16x8_t wide16 = vmovl_s8(bytes);
    return vcvtq_f32_s32(vmovl_s16(vget_low_s16(wide16)));
}

void compute_dense(const DenseLayer *layer, float *output, const float *input)
{
    int i;
    int j;
    const int M = layer->nb_inputs;
    const int N = layer->nb_neurons;
    const int stride = N;
    const float32x4_t scale = vdupq_n_f32(WEIGHTS_SCALE);

    for (i = 0; i + 4 <= N; i += 4) {
        float32x4_t sum = load_weights4(layer->bias + i);
        for (j = 0; j < M; ++j) {
            float32x4_t weights =
                load_weights4(layer->input_weights + j * stride + i);
            sum = vfmaq_n_f32(sum, weights, input[j]);
        }
        vst1q_f32(output + i, vmulq_f32(sum, scale));
    }

    for (; i < N; ++i) {
        float sum = layer->bias[i];
        for (j = 0; j < M; ++j)
            sum += layer->input_weights[j * stride + i] * input[j];
        output[i] = WEIGHTS_SCALE * sum;
    }

    for (i = 0; i < N; ++i)
        output[i] = activate(output[i], layer->activation);
}

static void compute_gru_gate(const GRULayer *gru,
                             float *output,
                             const float *input,
                             const float *recurrent_input,
                             int gate)
{
    int i;
    int j;
    const int M = gru->nb_inputs;
    const int N = gru->nb_neurons;
    const int stride = 3 * N;
    const int offset = gate * N;
    const float32x4_t scale = vdupq_n_f32(WEIGHTS_SCALE);

    for (i = 0; i + 4 <= N; i += 4) {
        float32x4_t sum = load_weights4(gru->bias + offset + i);
        for (j = 0; j < M; ++j) {
            float32x4_t weights =
                load_weights4(gru->input_weights + offset + j * stride + i);
            sum = vfmaq_n_f32(sum, weights, input[j]);
        }
        for (j = 0; j < N; ++j) {
            float32x4_t weights = load_weights4(
                gru->recurrent_weights + offset + j * stride + i);
            sum = vfmaq_n_f32(sum, weights, recurrent_input[j]);
        }
        vst1q_f32(output + i, vmulq_f32(sum, scale));
    }

    for (; i < N; ++i) {
        float sum = gru->bias[offset + i];
        for (j = 0; j < M; ++j)
            sum += gru->input_weights[offset + j * stride + i] * input[j];
        for (j = 0; j < N; ++j)
            sum += gru->recurrent_weights[offset + j * stride + i] *
                   recurrent_input[j];
        output[i] = WEIGHTS_SCALE * sum;
    }
}

void compute_gru(const GRULayer *gru, float *state, const float *input)
{
    int i;
    const int N = gru->nb_neurons;
    float z[MAX_NEURONS];
    float r[MAX_NEURONS];
    float h[MAX_NEURONS];
    float recurrent_input[MAX_NEURONS];

    compute_gru_gate(gru, z, input, state, 0);
    for (i = 0; i < N; ++i)
        z[i] = sigmoid_approx(z[i]);

    compute_gru_gate(gru, r, input, state, 1);
    for (i = 0; i < N; ++i) {
        r[i] = sigmoid_approx(r[i]);
        recurrent_input[i] = state[i] * r[i];
    }

    compute_gru_gate(gru, h, input, recurrent_input, 2);
    for (i = 0; i < N; ++i) {
        float candidate = activate(h[i], gru->activation);
        h[i] = z[i] * state[i] + (1 - z[i]) * candidate;
    }

    memcpy(state, h, N * sizeof(*state));
}

#define INPUT_SIZE 42

void compute_rnn(RNNState *rnn,
                 float *gains,
                 float *vad,
                 const float *input)
{
    int i;
    float dense_out[MAX_NEURONS];
    float noise_input[MAX_NEURONS * 3];
    float denoise_input[MAX_NEURONS * 3];

    compute_dense(rnn->model->input_dense, dense_out, input);
    compute_gru(rnn->model->vad_gru, rnn->vad_gru_state, dense_out);
    compute_dense(rnn->model->vad_output, vad, rnn->vad_gru_state);

    for (i = 0; i < rnn->model->input_dense_size; ++i)
        noise_input[i] = dense_out[i];
    for (i = 0; i < rnn->model->vad_gru_size; ++i)
        noise_input[i + rnn->model->input_dense_size] =
            rnn->vad_gru_state[i];
    for (i = 0; i < INPUT_SIZE; ++i)
        noise_input[i + rnn->model->input_dense_size +
                    rnn->model->vad_gru_size] = input[i];
    compute_gru(rnn->model->noise_gru, rnn->noise_gru_state, noise_input);

    for (i = 0; i < rnn->model->vad_gru_size; ++i)
        denoise_input[i] = rnn->vad_gru_state[i];
    for (i = 0; i < rnn->model->noise_gru_size; ++i)
        denoise_input[i + rnn->model->vad_gru_size] =
            rnn->noise_gru_state[i];
    for (i = 0; i < INPUT_SIZE; ++i)
        denoise_input[i + rnn->model->vad_gru_size +
                      rnn->model->noise_gru_size] = input[i];
    compute_gru(rnn->model->denoise_gru,
                rnn->denoise_gru_state,
                denoise_input);
    compute_dense(rnn->model->denoise_output,
                  gains,
                  rnn->denoise_gru_state);
}
