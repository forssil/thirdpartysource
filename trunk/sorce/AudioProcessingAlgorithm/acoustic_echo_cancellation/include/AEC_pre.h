#ifndef _H_AEC_PRE_
#define _H_AEC_PRE_

#define  SUBBANDUSE22k 240
#define  SUBBANDUSE44k 400
#define  FILTER_LEN  48  
struct  aecdata
{
	// 	float * mic_in;
	// 	int len_mic;
	// 	float * ref_in;
	// 	int len_ref;
	float * mic_fft;
	int len_fft;
	float * est_fft;	
	float * err_fft;
	float * ref_fft;
	float * maxsubtaps;
	int len_subband;
	float maxtaps;
	float *gain;
	int gainlen;
	int offset; //AEC start bin in fft
	bool *AECon;

	/*shell*/
	float *noise;
	float *noisebef;
	float *noisetotl;
	float shellgain;
	float Noiselevel;
	int farVAD;
	float DTD_corr;
	float DTD_gain;
	float far_pwr;


};
typedef aecdata Aec_data;//transfer data
#endif

