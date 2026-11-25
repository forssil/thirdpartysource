#include "arm64_optimization.h"

#include <arm_neon.h>
#include <memory.h>

#include "DCRemover.h"

DCRemover::DCRemover(int Fs, int frameLen, int chn)
{
    (void)Fs;
    m_nChn = chn;
    m_nFRAMESIZE = frameLen;
    PrePosAnaInit();
    m_nPreEnhance = 1;
}

DCRemover::~DCRemover()
{
    if (m_fpDCRemov != NULL) {
        delete[] m_fpDCRemov;
        m_fpDCRemov = NULL;
    }
    if (m_fpDCRemovPre != NULL) {
        delete[] m_fpDCRemovPre;
        m_fpDCRemovPre = NULL;
    }
}

void DCRemover::PrePosAnaInit()
{
    m_fInvFRAMESIZE = 1.f / (float)m_nFRAMESIZE;
    m_fpDCRemov = new float[m_nChn];
    memset(m_fpDCRemov, 0, m_nChn * sizeof(float));
    m_fpDCRemovPre = new float[m_nChn];
    memset(m_fpDCRemovPre, 0, m_nChn * sizeof(float));
}

void DCRemover::findLevelAndDcRemove(float *pData, int ch)
{
    int i = 0;
    float even_sum = 0.0f;
    float odd_sum = 0.0f;
    const float previous = m_fpDCRemovPre[ch];
    const float step =
        (m_fpDCRemov[ch] - previous) * m_fInvFRAMESIZE;
    const float step_x2 = 2.0f * step;
    float even_offset = previous + step;
    float odd_offset = previous + step_x2;

    for (; i + 4 <= m_nFRAMESIZE; i += 4) {
        float32x4_t samples = vld1q_f32(pData + i);
        float next_even_offset = even_offset + step_x2;
        float next_odd_offset = odd_offset + step_x2;
        float32x4_t offsets = {
            even_offset,
            odd_offset,
            next_even_offset,
            next_odd_offset
        };
        even_sum += vgetq_lane_f32(samples, 0);
        odd_sum += vgetq_lane_f32(samples, 1);
        even_sum += vgetq_lane_f32(samples, 2);
        odd_sum += vgetq_lane_f32(samples, 3);
        vst1q_f32(pData + i, vsubq_f32(samples, offsets));
        even_offset = next_even_offset + step_x2;
        odd_offset = next_odd_offset + step_x2;
    }

    for (; i + 2 <= m_nFRAMESIZE; i += 2) {
        float even_sample = pData[i];
        float odd_sample = pData[i + 1];
        even_sum += even_sample;
        odd_sum += odd_sample;
        pData[i] = even_sample - even_offset;
        pData[i + 1] = odd_sample - odd_offset;
        even_offset += step_x2;
        odd_offset += step_x2;
    }

    float mean_level = (even_sum + odd_sum) * m_fInvFRAMESIZE;
    m_fpDCRemovPre[ch] = m_fpDCRemov[ch];
    m_fpDCRemov[ch] +=
        (mean_level - m_fpDCRemov[ch]) * DCREMOVERATE;
}
