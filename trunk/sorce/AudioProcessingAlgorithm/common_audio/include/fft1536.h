#ifndef WBX_FFT_H
#define WBX_FFT_H
#ifdef __cplusplus
extern "C" {
#endif
 
	/*Unified interface*/
	/*this is FFT for real-signal input.
  Attention:
  it support 768, 256, and 2^N points samples;
  other points will cause erro output
  */
	int wbxFFT(float *real,float *imag,int len);
	int wbxInvFFT(float *real,float *imag,int len);

	/* 128 points split-radix FFT */
	int splitFFT128(float *real,float *imag);

	/* 256 points FFT in radix 4 */
	int FFT256(float *real,float *imag);
	//out_put is real. imag will be modified.
	int InvFFT256(float *real,float *imag);

	/* 384 points FFT,len=3*128  */
	int FFT384(float *real,float *imag);

	/* 768 points FFT for real data */
	/* real part of the first 384 ponits result placed in data[0] ~ data[383] */
	/* image part of the first 384 ponits result placed in data[384] ~ data[767] */
	int realFFT768(float *data);

	/* 768 points FFT,can be used for complex data */
	int FFT768(float *real,float *imag);
	//out_put is real. imag will be modified.
	int InvFFT768(float *real,float *imag);

	/* fft for len = 2^n */
	/* do fft in radix-4 if len = 4^n */
	/* do fft in split radix if len is not 4^n */
	int FFT2pw(float *real,float *imag,int len);
	//out_put is real. imag will be modified.
	int InvFFT2pw(float *real,float *imag,int len);

	/* fft in radix 4 for len = 4^n */
	int wbxRadixFourFFT(float *real,float *imag,int len);

	/* fft in split-radix for len = 2^n */
	int wbxsplitRadixFFT(float *real,float *imag,int len);

#ifdef __cplusplus
}
#endif
 
#endif