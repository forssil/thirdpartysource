#include "MVDR.h"

CMVDR::CMVDR(int fft_len, int fs, int bins, int channels, float interval, float DOA) :
	m_nbins(bins),
	m_nFs(fs),
	m_nFftLen(fft_len) {
	m_nChannels = 0;
	m_fInterval = 0;
	m_fDOA = 0;
	init_with_config(channels, interval, DOA);
}
CMVDR::~CMVDR(){
	if (m_pfH) {
		delete[] m_pfH;
		m_pfH = nullptr;
	}
}
void CMVDR::init_with_config(int channels, float interval, float DOA) {
	if (m_nChannels == channels && m_fInterval == interval && m_fDOA == DOA) {
		return;
	}
	m_ncounter = 0;
	m_nChannels = channels;
	m_fInterval = interval;
	m_fDOA = DOA;
	m_fc = 340;
	m_pfH = new float[(2 * m_nChannels + 2 * m_nChannels * m_nChannels) * m_nbins * 2 + 2 * m_nChannels * m_nChannels * 2 + m_nChannels * 2];
	m_pfSteerVector = m_pfH + m_nChannels * m_nbins * 2;
	m_pfNoiseMtx = m_pfSteerVector + m_nChannels * m_nbins * 2;
	m_pfNoiseMtxInv = m_pfNoiseMtx + m_nChannels * m_nChannels * m_nbins * 2;
	m_pfnum = m_pfNoiseMtxInv + m_nChannels * m_nChannels * m_nbins * 2;
	m_pfnum2 = m_pfnum + m_nChannels * m_nChannels * 2;
	m_pfx = m_pfnum2 + m_nChannels * m_nChannels * 2;
	memset(m_pfH, 0, ((2 * m_nChannels + 2 * m_nChannels * m_nChannels) * m_nbins + 2 * m_nChannels * m_nChannels + m_nChannels) *2 * sizeof(float));
	for (int i = 0; i < m_nbins; i++) {
		for (int j = 0; j < m_nChannels; j++) {
			int idx = (i * m_nChannels + j) * 2;
			float theta, omega;
			omega = 2 * PI * (float)m_nFs / (float)m_nFftLen * i;
			theta = -1 * j * omega * cos(m_fDOA) * m_fInterval / m_fc;
			m_pfSteerVector[idx] = cos(theta);
			m_pfSteerVector[idx + 1] = sin(theta);
			m_pfNoiseMtxInv[(i * m_nChannels * m_nChannels + j * j) * 2] = 1e-4;
		} 
	}
	//memcpy(m_pfNoiseMtxInv+ 0 * m_nChannels * m_nChannels * 2, noiseInv, 50 * sizeof(float));
}

void CMVDR::process(bool VADflag, audio_pro_share* input, audio_pro_share* output) {
	m_ncounter++;
	if (!VADflag && m_ncounter < 200) {
		update_noise_matrix(input);
	}
	if (m_ncounter == 2382) {
		m_ncounter *= 1;
	}
	float denorm_re, denorm_im;
	for (int i = 0; i < m_nbins; i++) {
		denorm_re = 0;
		denorm_im = 0;
		memset(m_pfnum, 0, m_nChannels * m_nChannels * 2 * sizeof(float));
		for (int row = 0; row < m_nChannels; row++) {
			for (int col = 0; col < m_nChannels; col++) {
				int idxi = 2 * (i * m_nChannels + row);
				int idxj = 2 * (i * m_nChannels + col);
				int idxij = 2 * (i * m_nChannels * m_nChannels + row * m_nChannels + col);
				denorm_re += (m_pfSteerVector[idxi] * m_pfNoiseMtxInv[idxij] * m_pfSteerVector[idxj]
							+ m_pfSteerVector[idxi + 1] * m_pfNoiseMtxInv[idxij + 1] * m_pfSteerVector[idxj]
							- m_pfSteerVector[idxi] * m_pfNoiseMtxInv[idxij + 1] * m_pfSteerVector[idxj + 1]
							+ m_pfSteerVector[idxi + 1] * m_pfNoiseMtxInv[idxij] * m_pfSteerVector[idxj + 1]);
				denorm_im += (m_pfSteerVector[idxi] * m_pfNoiseMtxInv[idxij] * m_pfSteerVector[idxj + 1]
							+ m_pfSteerVector[idxi + 1] * m_pfNoiseMtxInv[idxij + 1] * m_pfSteerVector[idxj + 1]
							+ m_pfSteerVector[idxi] * m_pfNoiseMtxInv[idxij + 1] * m_pfSteerVector[idxj]
							- m_pfSteerVector[idxi + 1] * m_pfNoiseMtxInv[idxij] * m_pfSteerVector[idxj]);
				m_pfnum[2 * row] += m_pfNoiseMtxInv[idxij] * m_pfSteerVector[idxj] - m_pfNoiseMtxInv[idxij + 1] * m_pfSteerVector[idxj + 1];
				m_pfnum[2 * row + 1] += m_pfNoiseMtxInv[idxij + 1] * m_pfSteerVector[idxj] + m_pfNoiseMtxInv[idxij] * m_pfSteerVector[idxj + 1];
			}
		}
		float tmp_real = 0, tmp_imag = 0;
		for (int row = 0; row < m_nChannels; row++) {
			int idx = 2 * (i * m_nChannels + row);
			m_pfH[idx]     = (m_pfnum[2 * row] * denorm_re + m_pfnum[2 * row + 1] * denorm_im) / (denorm_re * denorm_re + denorm_im * denorm_im);
			m_pfH[idx + 1] = (m_pfnum[2 * row + 1] * denorm_re - m_pfnum[2 * row] * denorm_im) / (denorm_re * denorm_re + denorm_im * denorm_im);
			tmp_real += input[row].pErrorFFT_[i * 2] * m_pfH[idx] - input[row].pErrorFFT_[i * 2 + 1] * m_pfH[idx + 1];
			tmp_imag += input[row].pErrorFFT_[i * 2 + 1] * m_pfH[idx] + input[row].pErrorFFT_[i * 2] * m_pfH[idx + 1];
		}
		output->pErrorFFT_[i * 2] = tmp_real;
		output->pErrorFFT_[i * 2 + 1] = tmp_imag;
	}
}
void CMVDR::update_noise_matrix(audio_pro_share* sharedata) {
	float denorm_re, denorm_im;
	for (int i = 0; i < m_nbins; i++) {
		denorm_re = 0;
		denorm_im = 0;
		memset(m_pfnum, 0, m_nChannels * m_nChannels * 2 * sizeof(float));
		for (int row = 0; row < m_nChannels; row++) {
			m_pfx[row * 2] = sharedata[row].pErrorFFT_[i * 2];
			m_pfx[row * 2 + 1] = sharedata[row].pErrorFFT_[i * 2 + 1];
		}
		for (int row = 0; row < m_nChannels; row++) {
			for (int col = 0; col < m_nChannels; col++) {
				int idxij = (row * m_nChannels + col) * 2;
				m_pfnum[idxij] = m_pfx[row * 2] * m_pfx[col * 2] + m_pfx[row * 2 + 1] * m_pfx[col * 2 + 1];
				m_pfnum[idxij + 1] = m_pfx[row * 2 + 1] * m_pfx[col * 2] - m_pfx[row * 2] * m_pfx[col * 2 + 1];
				int idxi = 2 * row;
				int idxj = 2 * col;
				idxij = 2 * (i * m_nChannels * m_nChannels + row * m_nChannels + col);
				denorm_re += (m_pfx[idxi] * m_pfNoiseMtxInv[idxij] * m_pfx[idxj]
							+ m_pfx[idxi + 1] * m_pfNoiseMtxInv[idxij + 1] * m_pfx[idxj]
							- m_pfx[idxi] * m_pfNoiseMtxInv[idxij + 1] * m_pfx[idxj + 1]
							+ m_pfx[idxi + 1] * m_pfNoiseMtxInv[idxij] * m_pfx[idxj + 1]);
				denorm_im += (m_pfx[idxi] * m_pfNoiseMtxInv[idxij] * m_pfx[idxj + 1]
							+ m_pfx[idxi + 1] * m_pfNoiseMtxInv[idxij + 1] * m_pfx[idxj + 1]
							+ m_pfx[idxi] * m_pfNoiseMtxInv[idxij + 1] * m_pfx[idxj]
							- m_pfx[idxi + 1] * m_pfNoiseMtxInv[idxij] * m_pfx[idxj]);
			}
		}
		matrix_multiply(m_pfNoiseMtxInv + i * m_nChannels * m_nChannels * 2, m_pfnum, m_pfnum2, m_nChannels);
		matrix_multiply(m_pfnum2, m_pfNoiseMtxInv + i * m_nChannels * m_nChannels * 2, m_pfnum, m_nChannels);
		for (int row = 0; row < m_nChannels; row++) {
			for (int col = 0; col < m_nChannels; col++) {
				int idx_inv = 2 * (i * m_nChannels * m_nChannels + row * m_nChannels + col);
				int idx_num = 2 * (row * m_nChannels + col);
				float num_denorm_re = (m_pfnum[idx_num] * (m_fLamda + denorm_re) + m_pfnum[idx_num + 1] * denorm_im) / ((m_fLamda + denorm_re) * (m_fLamda + denorm_re) + denorm_im * denorm_im);
				float num_denorm_im = (m_pfnum[idx_num + 1] * (m_fLamda + denorm_re) - m_pfnum[idx_num] * denorm_im) / ((m_fLamda + denorm_re) * (m_fLamda + denorm_re) + denorm_im * denorm_im);
				m_pfNoiseMtxInv[idx_inv] = 1 / m_fLamda * (m_pfNoiseMtxInv[idx_inv] - num_denorm_re);
				m_pfNoiseMtxInv[idx_inv + 1] = 1 / m_fLamda * (m_pfNoiseMtxInv[idx_inv + 1] - num_denorm_im);
			}
		}
	}
}
void CMVDR::matrix_multiply(float* A, float* B, float* C, int length) {
	// C = A * B, A and B are [length * length * 2]
	// C(i, j) = sigma(A(i,k) * B(k,j))
	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			for (int k = 0; k < length; k++) {
				int idxA = (i * length + k) * 2;
				int idxB = (k * length + j) * 2;
				int idxC = (i * length + j) * 2;
				C[idxC] += A[idxA] * B[idxB] - A[idxA + 1] * B[idxB + 1];
				C[idxC + 1] += A[idxA + 1] * B[idxB] + A[idxA] * B[idxB + 1];
			}
		}
	}
}
float* CMVDR::get_weight() {
	return m_pfH;
}