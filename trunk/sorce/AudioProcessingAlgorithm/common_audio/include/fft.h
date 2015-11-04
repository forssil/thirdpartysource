#ifndef _FFT_H
#define _FFT_H
#ifdef __cplusplus
extern "C" {
#endif
 
	/*Unified interface*/	
	int FFT(float *real,float *imag,int len);
	int InvFFT(float *real,float *imag,int len);	
	/* fft for len = 2^n */	
	int FFT2(float *real,float *imag,int len);
	//out_put is real. imag will be modified.
	int InvFFT2(float *real,float *imag,int len);
	/* fft in radix 4 for len = 4^n */
	int RadixFourFFT(float *real,float *imag,int len);

	int splitRadixFFT(float *real,float *imag,int len);
#ifdef __cplusplus
}
#endif
 
#endif