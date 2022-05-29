#include "MVDR.h"
#include "basemath.h"

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


CAdaptiveBeamForming::CAdaptiveBeamForming(int fft_len, int fs, int bins, int channels):
	m_nbins(bins),
	m_nFs(fs),
	m_nFftLen(fft_len),
	m_nChannels(channels){


}
CAdaptiveBeamForming::~CAdaptiveBeamForming() {
	if (m_fppAutoCorr[0]) {
		delete[] m_fppAutoCorr[0];
	}
	if (m_fppAutoCorr) {
		delete[] m_fppAutoCorr;
	}

}

void CAdaptiveBeamForming::init() {

	int  inter_channels = m_nChannels - 1;
	///mem allocate
	m_fppAutoCorr = new float*[3 * inter_channels];
	m_fppCrossCorr = m_fppAutoCorr + inter_channels;
	m_fppTransFilter = m_fppCrossCorr + inter_channels;
	int mem_size = inter_channels * (m_nFftLen * 5 / 2)+ m_nFftLen;
	m_fppAutoCorr[0] = new float[mem_size];	 
	memset(m_fppAutoCorr[0], 0, mem_size * sizeof(float));
	m_fppCrossCorr[0] = m_fppAutoCorr[0] + inter_channels* m_nFftLen/2;
	m_fppTransFilter[0] = m_fppCrossCorr[0] + inter_channels * m_nFftLen ;
	for (int i = 1; i < inter_channels; i++) {
		m_fppAutoCorr[i] = m_fppAutoCorr[0] + i * m_nFftLen / 2;
		m_fppCrossCorr[i] = m_fppCrossCorr[0] + i * m_nFftLen ;
		m_fppTransFilter[i] = m_fppTransFilter[0] + i * m_nFftLen;
	}
	m_fpOutPut = m_fppTransFilter[inter_channels - 1] + m_nFftLen;
}
void  CAdaptiveBeamForming::process(audio_pro_share* input, int input_len, audio_pro_share& output, int main_channel)
{
	//update corr
	m_nMainChannel = (main_channel<0 || main_channel>m_nChannels) ? 0 : main_channel;
	size_t loopmax = input_len == (m_nChannels) ? (m_nChannels) : input_len;
	float alpha = 1.f - m_fLamda;
	float * fp_input = nullptr;
	float* fp_output_auto = *m_fppAutoCorr;
	float* fp_output_corss = *m_fppCrossCorr;
	float* fp_main_channel = input[m_nMainChannel].pErrorFFT_;
	float *fp_tf = NULL;
	int channels = 0;
	float tem_pwr = 0.f;
	memset(m_fpOutPut,0, m_nFftLen * sizeof(float));
	for (size_t channelind = 0; channelind < loopmax; channelind++) {
		if (channelind == m_nMainChannel) {
			continue;
		}
		fp_input = input[channelind].pErrorFFT_;
		fp_output_auto = m_fppAutoCorr[channels];
		fp_tf = m_fppTransFilter[channels];
		fp_output_corss = m_fppCrossCorr[channels++];
		for (int bin = 0; bin < m_nbins; bin++) {
			int bin_in_complex = 2 * bin;
			tem_pwr = fp_input[bin_in_complex] * fp_input[bin_in_complex] + fp_input[bin_in_complex + 1] * fp_input[bin_in_complex + 1];
			//if (tem_pwr <= m_fActiveThreashold) {
			//	continue;
			//}
			///auto
			fp_output_auto[bin] *= alpha;
			fp_output_auto[bin] += m_fLamda *(tem_pwr);
			//cross
			fp_output_corss[bin_in_complex] *= alpha;
			fp_output_corss[bin_in_complex] += m_fLamda * (fp_main_channel[bin_in_complex] * fp_input[bin_in_complex] + fp_main_channel[bin_in_complex + 1] * fp_input[bin_in_complex + 1]);
			fp_output_corss[bin_in_complex +1] *= alpha;
			fp_output_corss[bin_in_complex + 1] += m_fLamda * (fp_main_channel[bin_in_complex + 1] * fp_input[bin_in_complex] - fp_main_channel[bin_in_complex] * fp_input[bin_in_complex + 1]);
	
			//output
			float H_re = fp_output_corss[bin_in_complex] / (fp_output_auto[bin] + m_fActiveThreashold);
			float H_im = fp_output_corss[bin_in_complex +1] / (fp_output_auto[bin] + m_fActiveThreashold);

			if (fp_output_auto[bin] > m_fActiveThreashold &&
				H_re<2.f &&
				H_im< 2.f) {
				fp_tf[bin_in_complex] *= alpha;
				fp_tf[bin_in_complex] += m_fLamda * H_re;
				fp_tf[bin_in_complex+1] *= alpha;
				fp_tf[bin_in_complex+1] += m_fLamda * H_im;
			}
			float tem_re = fp_tf[bin_in_complex] * fp_input[bin_in_complex] - fp_tf[bin_in_complex + 1] * fp_input[bin_in_complex + 1];
			float tem_im = fp_tf[bin_in_complex] * fp_input[bin_in_complex + 1] + fp_tf[bin_in_complex + 1] * fp_input[bin_in_complex];
			if ((tem_re *tem_re + tem_im * tem_im) >( fp_main_channel[bin_in_complex] * fp_main_channel[bin_in_complex ] + fp_main_channel[bin_in_complex + 1]*fp_main_channel[bin_in_complex + 1]) * 2.5f) {
				tem_re = fp_main_channel[bin_in_complex];
				tem_im = fp_main_channel[bin_in_complex + 1];
			}
			m_fpOutPut[bin_in_complex] += tem_re;
			m_fpOutPut[bin_in_complex + 1] += tem_im;
		}
	}
	int bin = 0;
	for ( bin = 0; bin < m_nbins; bin++) {
		int bin_in_complex = 2 * bin;
		output.pErrorFFT_[bin_in_complex] = (fp_main_channel[bin_in_complex] + m_fpOutPut[bin_in_complex])/m_nChannels;
		output.pErrorFFT_[bin_in_complex + 1] = (fp_main_channel[bin_in_complex + 1] + m_fpOutPut[bin_in_complex + 1])/m_nChannels;
	}
	for (; bin < m_nFftLen / 2; bin++) {
		int bin_in_complex = 2 * bin;
		output.pErrorFFT_[bin_in_complex] = (fp_main_channel[bin_in_complex] ) ;
		output.pErrorFFT_[bin_in_complex + 1] = (fp_main_channel[bin_in_complex + 1] ) ;
	}
}