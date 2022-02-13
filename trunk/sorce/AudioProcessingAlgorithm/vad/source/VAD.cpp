/**************************************************
*         Copyright 2014 GaoH Inc.
*         Author:  Gu Cheng
*         History: 10/15/2014
*         Content: Voice Activity Detector
***************************************************/


#include <cstring>
#include <cmath>
#include "SilenceDetector.h"
#include "VAD.h"

const float win256[]= {

	0.000149421078453171f,
	0.000597595007177931f,
	0.00134425391964726f,
	0.00238895154954133f,
	0.00373106349747415f,
	0.00536978760418699f,
	0.00730414442998667f,
	0.00953297784014107f,
	0.0120549556958828f,
	0.0148685706506078f,
	0.0179721410507924f,
	0.0213638119410917f,
	0.0250415561730168f,
	0.0290031756165303f,
	0.0332463024738333f,
	0.0377684006945618f,
	0.0425667674915437f,
	0.0476385349562126f,
	0.0529806717727114f,
	0.0585899850296627f,
	0.0644631221285217f,
	0.0705965727873714f,
	0.0769866711389634f,
	0.0836295979217494f,
	0.0905213827625935f,
	0.097657906549802f,
	0.105034903895052f,
	0.112647965682748f,
	0.120492541705279f,
	0.128563943382607f,
	0.13685734656456f,
	0.145367794414147f,
	0.154090200370186f,
	0.163019351187458f,
	0.172149910052584f,
	0.181476419773753f,
	0.190993306042403f,
	0.200694880764894f,
	0.210575345462196f,
	0.220628794735546f,
	0.230849219796014f,
	0.24123051205586f,
	0.251766466779543f,
	0.262450786792195f,
	0.273277086243339f,
	0.284238894423618f,
	0.295329659632231f,
	0.306542753092784f,
	0.317871472915207f,
	0.329309048101366f,
	0.340848642591985f,
	0.352483359352449f,
	0.364206244495054f,
	0.376010291435238f,
	0.387888445079305f,
	0.399833606041146f,
	0.411838634885427f,
	0.423896356394721f,
	0.435999563858022f,
	0.448141023378083f,
	0.460313478195001f,
	0.472509653023471f,
	0.484722258401111f,
	0.496943995045254f,
	0.509167558215622f,
	0.521385642080249f,
	0.533590944082063f,
	0.545776169303514f,
	0.557934034826625f,
	0.570057274085884f,
	0.582138641211355f,
	0.594170915359414f,
	0.606146905028547f,
	0.618059452357584f,
	0.629901437403849f,
	0.641665782398637f,
	0.653345455977481f,
	0.664933477382692f,
	0.67642292063565f,
	0.687806918676346f,
	0.699078667467724f,
	0.710231430062342f,
	0.721258540628942f,
	0.73215340843651f,
	0.742909521793458f,
	0.753520451939554f,
	0.763979856888295f,
	0.774281485217411f,
	0.784419179805244f,
	0.794386881510759f,
	0.804178632795004f,
	0.813788581281829f,
	0.82321098325577f,
	0.832440207094966f,
	0.841470736637101f,
	0.850297174476321f,
	0.858914245189185f,
	0.867316798487695f,
	0.875499812297549f,
	0.883458395759753f,
	0.891187792153812f,
	0.898683381740745f,
	0.905940684524234f,
	0.912955362928245f,
	0.919723224389528f,
	0.926240223863451f,
	0.932502466241655f,
	0.938506208680101f,
	0.94424786283611f,
	0.949723997013056f,
	0.954931338211443f,
	0.959866774085119f,
	0.96452735480148f,
	0.96891029480454f,
	0.97301297447981f,
	0.976832941720003f,
	0.980367913390621f,
	0.983615776694547f,
	0.98657459043483f,
	0.98924258617491f,
	0.991618169295584f,
	0.993699919948085f,
	0.995486593902703f,
	0.99697712329244f,
	0.998170617251262f,
	0.99906636244655f,
	0.999663823505453f,
	0.999962643334866f,
	0.999962643334866f,
	0.999663823505453f,
	0.99906636244655f,
	0.998170617251262f,
	0.99697712329244f,
	0.995486593902703f,
	0.993699919948085f,
	0.991618169295584f,
	0.98924258617491f,
	0.98657459043483f,
	0.983615776694547f,
	0.980367913390621f,
	0.976832941720003f,
	0.97301297447981f,
	0.96891029480454f,
	0.96452735480148f,
	0.959866774085119f,
	0.954931338211443f,
	0.949723997013056f,
	0.94424786283611f,
	0.938506208680101f,
	0.932502466241655f,
	0.926240223863451f,
	0.919723224389528f,
	0.912955362928245f,
	0.905940684524234f,
	0.898683381740745f,
	0.891187792153812f,
	0.883458395759753f,
	0.875499812297549f,
	0.867316798487695f,
	0.858914245189185f,
	0.850297174476321f,
	0.841470736637101f,
	0.832440207094966f,
	0.82321098325577f,
	0.813788581281829f,
	0.804178632795004f,
	0.794386881510759f,
	0.784419179805244f,
	0.774281485217411f,
	0.763979856888295f,
	0.753520451939554f,
	0.742909521793458f,
	0.73215340843651f,
	0.721258540628942f,
	0.710231430062342f,
	0.699078667467724f,
	0.687806918676346f,
	0.67642292063565f,
	0.664933477382692f,
	0.653345455977481f,
	0.641665782398637f,
	0.629901437403849f,
	0.618059452357584f,
	0.606146905028547f,
	0.594170915359414f,
	0.582138641211355f,
	0.570057274085884f,
	0.557934034826625f,
	0.545776169303514f,
	0.533590944082063f,
	0.521385642080249f,
	0.509167558215622f,
	0.496943995045254f,
	0.484722258401111f,
	0.472509653023471f,
	0.460313478195001f,
	0.448141023378083f,
	0.435999563858022f,
	0.423896356394721f,
	0.411838634885427f,
	0.399833606041146f,
	0.387888445079305f,
	0.376010291435238f,
	0.364206244495054f,
	0.352483359352449f,
	0.340848642591985f,
	0.329309048101366f,
	0.317871472915207f,
	0.306542753092784f,
	0.295329659632231f,
	0.284238894423618f,
	0.273277086243339f,
	0.262450786792195f,
	0.251766466779543f,
	0.24123051205586f,
	0.230849219796014f,
	0.220628794735546f,
	0.210575345462196f,
	0.200694880764894f,
	0.190993306042403f,
	0.181476419773753f,
	0.172149910052584f,
	0.163019351187458f,
	0.154090200370186f,
	0.145367794414147f,
	0.13685734656456f,
	0.128563943382607f,
	0.120492541705279f,
	0.112647965682748f,
	0.105034903895052f,
	0.097657906549802f,
	0.0905213827625935f,
	0.0836295979217494f,
	0.0769866711389634f,
	0.0705965727873714f,
	0.0644631221285217f,
	0.0585899850296627f,
	0.0529806717727114f,
	0.0476385349562126f,
	0.0425667674915437f,
	0.0377684006945618f,
	0.0332463024738333f,
	0.0290031756165303f,
	0.0250415561730168f,
	0.0213638119410917f,
	0.0179721410507924f,
	0.0148685706506078f,
	0.0120549556958828f,
	0.00953297784014107f,
	0.00730414442998667f,
	0.00536978760418699f,
	0.00373106349747415f,
	0.00238895154954133f,
	0.00134425391964726f,
	0.000597595007177931f,
	0.000149421078453171f
};

AEC_VAD::AEC_VAD(void):
m_band_power(NULL),
m_noisepower(NULL),
m_noise_est(NULL),
m_noise_min(NULL),
m_segSpecPow(NULL),
m_freq_index(NULL),
m_flag(true),
m_fs(0),
m_fftlen(0),
m_framelen(0),
m_i_whole(0),
m_i_sub(0),
m_vad_est(NULL),
m_vad_full(0),
m_vad_full_pre(0),
m_speech_env_pre(0),
m_noise_temp_env_pre(0),
m_noise_env_pre(0),
m_band_num(3),
m_sub_time(15),
m_whole_time(10),
m_bias(3),
alpha_s(0.9f),
alpha_nt(0.5f),
alpha_n(0.99f),
threshold1(4.f),
threshold2(2.f),
_noise_offset(0.000002f),
m_fFrameBuffer(NULL),
m_frame_buffer_count(5),
m_nFrameBufferCount(-5),
//m_silencedetector(NULL),
m_flagofCurrentSilence(false),
m_fTempout(0.f)
{

}


AEC_VAD::~AEC_VAD(void) {
	if (m_band_power) delete []m_band_power;
	if (m_noisepower) delete []m_noisepower;
	if (m_noise_est)  delete []m_noise_est;
	if (m_noise_min)  delete []m_noise_min;
	if (m_freq_index) delete []m_freq_index;
	if (m_fFrameBuffer)    delete []m_fFrameBuffer;
	//if (m_silencedetector) delete []m_silencedetector;
	if (m_vad_est)    delete []m_vad_est;
}

/***************************************************
name:    CreateVAD_int
para:    fs       (IN)
	     fftlen   (IN)
		 framelen (IN)
content: initial parameter
***************************************************/
void AEC_VAD::CreateVAD_int(int fs, int fftlen, int framelen) {
	//load parameter
	m_fs = fs;
	m_fftlen = fftlen;
	m_framelen = framelen;


	alpha_s  = 0.85f;
	alpha_nt = 0.5f;
	alpha_n  = 0.9998f;
	
	//fixed parameter
	m_band_threshold[0][0] = 8;
	m_band_threshold[0][1] = 3;
	m_band_threshold[1][0] = 8;
	m_band_threshold[1][1] = 3;
	m_band_threshold[2][0] = 8;
	m_band_threshold[2][1] = 3;

	m_band_weight[0] = 0.6f;
	m_band_weight[1] = 0.2f;
	m_band_weight[2] = 0.2f;

	m_band_freq[0] = 300;
	m_band_freq[1] = 800;
	m_band_freq[2] = 1600;
	m_band_freq[3] = 3200;

	m_band_power = new float[m_band_num];
	m_noisepower = new float[m_band_num];
	m_noise_est  = new float[m_band_num*m_whole_time];
	m_noise_min  = new float[m_band_num];
	m_freq_index = new int[m_band_num+1];
	
	memset(m_band_power, 0, sizeof(float)*m_band_num);
	memset(m_noisepower, 0, sizeof(float)*m_band_num);
	memset(m_noise_est,  0, sizeof(float)*m_band_num*m_whole_time);
	memset(m_noise_min,  0, sizeof(float)*m_band_num);
	memset(m_freq_index, 0, sizeof(float)*(m_band_num+1));
	
	//initial
	for (int i = 0; i < m_band_num; ++i)
	{
		m_noise_min[i] = 1e8;
		for (int j = 0; j < m_whole_time; ++j)
			m_noise_est[i*m_whole_time + j] = 1e8;
	}
	for (int i = 0; i < m_band_num+1; ++i)
		m_freq_index[i] = static_cast<int>(m_band_freq[i]*m_fftlen/m_fs + 0.5);



	m_fFrameBuffer      = new float[m_frame_buffer_count*m_band_num];
	memset(m_fFrameBuffer, 0, sizeof(float)*m_frame_buffer_count*m_band_num);
	//SilenceDetector
	//m_silencedetector = new SilenceDetector();
	//m_silencedetector->init();

	m_vad_est = new float[m_band_num];
	memset(m_vad_est, 0, sizeof(float)*m_band_num);
}

/***************************************************
name:    NoiseEst
content: noise estimation
***************************************************/
void AEC_VAD::NoiseEst() {
	 if (true == m_flagofCurrentSilence)
	 {
		 m_nFrameBufferCount  = -m_frame_buffer_count;
		 memset(m_fFrameBuffer,0,sizeof(float) *m_frame_buffer_count *m_band_num);
	 } 
	 else 
	 {
		 memcpy(m_fFrameBuffer+((m_nFrameBufferCount +m_frame_buffer_count) %m_frame_buffer_count)*m_band_num, m_band_power, sizeof(float)*m_band_num);
		 m_nFrameBufferCount++;

		 if (m_nFrameBufferCount >= 0)
		 {
			 const int height = m_band_num;
			 float temp = 0.f;
			 float min  = 1e8;

			 for (int j = 0; j < height; ++j) {
				 temp = m_bias*m_fFrameBuffer[(m_nFrameBufferCount%m_frame_buffer_count)*m_band_num + j];

				 if (m_i_sub == m_sub_time)
					 m_noise_est[j*m_whole_time + m_i_whole] = temp;
				 else if (temp < m_noise_est[j*m_whole_time + m_i_whole])
					 m_noise_est[j*m_whole_time + m_i_whole] = temp;

				 if (m_noise_est[j*m_whole_time + m_i_whole] < m_noise_min[j])
					 m_noise_min[j] = m_noise_est[j*m_whole_time + m_i_whole];

				 if (m_i_sub == m_sub_time-1) {	 
					 min  = 1e8;
					 for (int i=0; i<m_whole_time; ++i)
						 if (min > m_noise_est[j*m_whole_time + i])
							 min = m_noise_est[j*m_whole_time + i];
					 m_noise_min[j] = min;
				 }
				 m_noisepower[j] = m_noise_min[j];
			 }

			 if (m_i_sub == m_sub_time)
				 m_i_sub = 0;

			 if (m_i_sub == m_sub_time-1) {
				 ++m_i_whole;
				 if (m_i_whole == m_whole_time-1)
					 m_i_whole = 0;
			 }
			 ++m_i_sub;
		 }
	 }
}

/***************************************************
name:    VADEst
content: obtain VADEst
***************************************************/
void AEC_VAD::VADEst() {
	const int height = m_band_num;
	float tempVAD  = 0.f;
	float rate     = 0.f;
	float pre_prob = 0.f;

	for (int i = 0; i < height; ++i) {
		//tempVAD  = 0.f;
		//rate     = 0.f;
		//pre_prob = 0.f;

		rate = static_cast<float>(m_band_power[i]/(m_noisepower[i] + (1e-12)));
		pre_prob = (rate - m_band_threshold[i][1])/(m_band_threshold[i][0] - m_band_threshold[i][1]);

		if (pre_prob < 0)
			pre_prob = 0;
		else if(pre_prob > 1)
			pre_prob = 1;

		//tempVAD = tempVAD>pre_prob?tempVAD:pre_prob;	

		if (pre_prob > m_vad_est[i]){
			m_vad_est[i] *= 0.3f;
			m_vad_est[i] += 0.7f*pre_prob;

		} else {
			m_vad_est[i] *= 0.7f;
			m_vad_est[i] += 0.3f*pre_prob;
		}
	} 

}

/***************************************************
name:    VADFull
content: obtain VADFull
***************************************************/
void AEC_VAD::VADFull() {
	float speech_env     = 0.f;
	float noise_temp_env = 0.f;
	float noise_env      = 0.f;
	float power          = 0.f;

	//(1-0.95*win256[i]) : Inhibition of low-frequency noise
	for (int i = m_freq_index[0]; i < m_freq_index[m_band_num]; ++i)
		power += m_segSpecPow[i]*(1.f-0.95f*win256[i]);

	if (true == m_flag) {
		m_speech_env_pre     = power;
		m_noise_temp_env_pre = power;
		m_noise_env_pre      = power;
		m_flag               = false;

	} else {
		if(m_flagofCurrentSilence==false){
		speech_env     = (m_speech_env_pre<power) ?  power : ((1-alpha_s)*power + alpha_s*m_speech_env_pre);
		noise_temp_env = (m_noise_temp_env_pre<power) ? power : ((1-alpha_nt)*power + alpha_nt*m_noise_temp_env_pre);
		noise_env      = (m_noise_env_pre<noise_temp_env) ? ((1-alpha_n)*noise_temp_env + alpha_n*m_noise_env_pre) : noise_temp_env;

		//noise offset
		noise_env=( noise_env>_noise_offset)?noise_env:_noise_offset;

		if(speech_env > threshold1*noise_env)
			m_vad_full = 1;
		else if(speech_env < threshold2*noise_env)
			m_vad_full = 0;
		else
			m_vad_full = m_vad_full_pre;

		m_speech_env_pre     = speech_env;
		m_noise_temp_env_pre = noise_temp_env;
		m_noise_env_pre      = noise_env;
		m_vad_full_pre       = m_vad_full;
		}
		else
			m_vad_full = 0;
	}
}

/***************************************************
name:    SubbandPwr
content: obtain sub-band power
***************************************************/
void AEC_VAD::SubbandPwr() {
	for (int i = 0; i < m_band_num; ++i) {
		float sum = 0.f;
		for (int j = m_freq_index[i]; j < m_freq_index[i+1]; ++j)
			sum += m_segSpecPow[j];
		sum /= (m_freq_index[i+1] - m_freq_index[i]);
		m_band_power[i] = sum;
	}
}

/***************************************************
name:    GetNoisePower
content: get noise power
***************************************************/
float AEC_VAD::GetNoisePower()
{
	float noisepower = 0;
	for (int j = 0; j < m_band_num; ++j) {		
		noisepower += m_noisepower[j];
	}
	return noisepower;
}

/***************************************************
name:    GetFlag
content: get flag
***************************************************/
float   AEC_VAD::GetFlag()
{
	m_fTempout=m_flagofCurrentSilence?1.f:0.f;//m_silencedetector->GetRatio();
	return m_fTempout;
}

/***************************************************
name:    GetVAD
para:    segSpecPow  (IN)
	     vad_est     (OUT)
	     vad_full    (OUT)
content: obtain VAD
***************************************************/
void AEC_VAD::GetVAD(const float* smooth, const float* nonsmooth, float* vad_est, float& vad_full) {
	m_segSpecPow = smooth;
	SubbandPwr();
	float noisepower = GetNoisePower();
	//m_flagofCurrentSilence = m_silencedetector->isCurrentSilence(noisepower, nonsmooth, m_framelen);
	NoiseEst();
	VADEst();
	VADFull();
	memcpy(vad_est, m_vad_est, sizeof(float)*m_band_num);
	vad_full = m_vad_full;
}

