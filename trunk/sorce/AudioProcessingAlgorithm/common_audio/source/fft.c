#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "fft.h"
int FFT2(float *real,float *imag,int len)
{
	
	int i,level;

	if((NULL==real)||(NULL==imag))
		return 1;

	level = 1;
	for(i=1;level<32;level++)
	{
		i <<= 1;
		if(i==len) break;       
		if(i>len) return 1;
	}

	if(level%2==0)
	{
		if(RadixFourFFT(real,imag,len)!=0)
			return 1;
	}
	else
	{
		if(splitRadixFFT(real,imag,len)!=0)
			return 1;
	}

	return 0;
	
}


//out_put is real. imag will be modified.
int InvFFT2(float *real,float *imag,int len)
{
	int i;
	float tmp;
	float *imfp=imag;
	if ((real==NULL) || (imag==NULL)|| (len<=0))
	{
		return 1;
	}
	for (i=0;i<len;i++)
	{
		*imfp*=-1.f;
		imfp++;
	}
	FFT2(real,imag,len);
	imfp=real;
	tmp=1.f/(float)len;
	for (i=0;i<len;i++)
	{
		*imfp*=tmp;
		imfp++;
	}
	return 0;
}


/* fft in radix 4 for len = 4^n */
int RadixFourFFT(float *real,float *imag,int len)
{
	int i,j,level,i1,i2,i3,n1,n2,levelNum;
	double nk1,nk2,nk3,e,r0,r1,r2,r3,s0,s1,s2,s3;
	double co1,co2,co3,si1,si2,si3;

	if((NULL==real)||(NULL==imag))
		return 1;

	level = 1;
	for(i=1;level<16;level++)
	{
		i <<= 2;
		if(i==len) break;       /* len=4^level */
		if(i>len) return 1;
	}

	n2 = len;
	for(levelNum=1;levelNum<=level;levelNum++)
	{
		n1 = n2;						/* n1 = len/4^(levelNum-1) */
		n2 >>= 2;						/* n2 = len/4^levelNum */
		e = 6.28318530718/n1;			/* e = 2*pi/4^levelNum */

		for(i=0;i<n2;i++)
		{
			/* calculate twiddle factor */
			nk1 = e*i;
			nk2 = nk1 + nk1;
			nk3 = nk1 + nk2;

			co1 = cos(nk1);
			co2 = cos(nk2);
			co3 = cos(nk3);
			si1 = sin(nk1);
			si2 = sin(nk2);
			si3 = sin(nk3);

			for(j=i;j<len;j+=n1)
			{
				i1 = j + n2;
				i2 = i1 + n2;
				i3 = i2 + n2;
				
				/* 4 points DFT */
				r0 = real[j] + real[i1] + real[i2] + real[i3];
				s0 = imag[j] + imag[i1] + imag[i2] + imag[i3];
				r1 = real[j] + imag[i1] - real[i2] - imag[i3];
				s1 = imag[j] - real[i1] - imag[i2] + real[i3];
				r2 = real[j] - real[i1] + real[i2] - real[i3];
				s2 = imag[j] - imag[i1] + imag[i2] - imag[i3];
				r3 = real[j] - imag[i1] - real[i2] + imag[i3];
				s3 = imag[j] + real[i1] - imag[i2] - real[i3];

				real[j] = (float)r0;
				imag[j] = (float)s0;
				real[i1] = (float)(co1*r1 + si1*s1);
				imag[i1] = (float)(co1*s1 - si1*r1);
				real[i2] = (float)(co2*r2 + si2*s2);
				imag[i2] = (float)(co2*s2 - si2*r2);
				real[i3] = (float)(co3*r3 + si3*s3);
				imag[i3] = (float)(co3*s3 - si3*r3);
			}
		}
	}

	/* reorder */
	for(i=0,j=0;i<len-1;i++)
	{
		if(i<=j)
		{
			r0 = real[i];
			real[i] = real[j];
			real[j] = (float)r0;
			r0 = imag[i];
			imag[i] = imag[j];
			imag[j] = (float)r0;
		}
		n1 = len/4;
		while(3*n1<(j+1))
		{
			j = j - 3 * n1;
			n1 = n1/4;
		}
		j = j + n1;
	}
	return 0;
}




/* fft in split-radix for len = 2^n */
int splitRadixFFT(float *real,float *imag,int len)
{
	int	n,i,j,step=1,pos1,pos2;
	int *isThisLevel,level,levelNum;
	int LStart,LFourthStart,LHalfStart,LThreeFourthStart,LEnd;
	int LSize=len<<1,LHalfSize=len,LForthSize=len>>1;
	float *temr,*temi;
	float temr1,temr2,temr3,temr4,temr5,temr6,temi1,temi2,temi3,temi4,temi5,temi6;
	float temrS,temiS,temrF,temiF,temrH,temiH,temrT,temiT;
	float *twiddleFactor,e,nk;

	if(NULL==real||NULL==imag)
		return 1;

	twiddleFactor = (float*)calloc(len,sizeof(float));
	if(NULL==twiddleFactor)
         return 1;
	temr = (float*)calloc(len,sizeof(float));
	if(NULL==temr)
         return 1;
	temi = (float*)calloc(len,sizeof(float));
	if(NULL==temi)
         return 1;
	isThisLevel = (int*)calloc(len,sizeof(int));
	if(NULL==isThisLevel)
         return 1;

	/* calculate twiddle factors */
	e = (float)6.28318530718/len;
	for(n=0;n<len;n++)
	{
		nk = e * n;
		twiddleFactor[n] = (float)sin(nk); 
	}

	levelNum = 1;
	for(i=1;levelNum<32;levelNum++)
	{
		i <<= 1;
		if(i==len) break;       /* len=2^levelNum */
		if(i>len) return 1;
	}

	for(n=0;n<len;n++)
	{
		temr[n] = real[n];
		temi[n] = imag[n];
	}

	isThisLevel[0] = 1;
	for(level=1;level<levelNum;level++)
	{ 
		LSize >>= 1;
		LHalfSize = LSize >> 1;
        LForthSize = LHalfSize >> 1;
		LEnd = LSize;
		for(LStart=0;LStart<len;LStart=LEnd)
		{
			LEnd = LStart + LSize;
			if(isThisLevel[LStart]==level)
			{
				pos1 = 0;
		        LFourthStart = LStart + LForthSize; 
		        LHalfStart = LStart + LHalfSize;
		        LThreeFourthStart = LHalfStart + LForthSize;

				/* update level value */
				isThisLevel[LStart] = level + 1;
				isThisLevel[LThreeFourthStart] = isThisLevel[LHalfStart] = level + 2;

				/* calculate L */
				for(n=0;n<LForthSize;n++)
				{				
					if(level!=levelNum)
					{
						temrS = temr[LStart];
						temiS = temi[LStart];
						temrF = temr[LFourthStart];
						temiF = temi[LFourthStart];
						temrH = temr[LHalfStart];
						temiH = temi[LHalfStart];
						temrT = temr[LThreeFourthStart];
						temiT = temi[LThreeFourthStart];
						

						temr1 = temrS + temrH;
						temi1 = temiS + temiH;
						temr2 = temrF + temrT;
						temi2 = temiF + temiT;
						temr3 = temrS  - temrH;
						temi3 = temiF - temiT;
						temr4 = -temrF + temrT;
						temi4 = temiS - temiH;

						temr5 = temr3 + temi3;
   			    		temi5 = temr4 + temi4;
						temr6 = temr3 - temi3;
						temi6 = temi4 - temr4;

						/* multiply the twiddle factor */
						pos2 = pos1*3;
						temr[LHalfStart] = temr5*twiddleFactor[pos1+len/4] + temi5*twiddleFactor[pos1];
						temi[LHalfStart] = temi5*twiddleFactor[pos1+len/4] - temr5*twiddleFactor[pos1];
						temr[LThreeFourthStart] = temr6*twiddleFactor[pos2+len/4] + temi6*twiddleFactor[pos2];
						temi[LThreeFourthStart] = temi6*twiddleFactor[pos2+len/4] - temr6*twiddleFactor[pos2];
						
						pos1 += step;

					}else{

						temrS = temr[LStart];
						temiS = temi[LStart];
						temrF = temr[LFourthStart];
						temiF = temi[LFourthStart];
						temrH = temr[LHalfStart];
						temiH = temi[LHalfStart];
						temrT = temr[LThreeFourthStart];
						temiT = temi[LThreeFourthStart];

						temr1 = temrS + temrH;
						temi1 = temiS + temiH;
						temr2 = temrF + temrT;
						temi2 = temiF + temiT;

						temr3 = temrS - temrH;
						temi3 = temiF - temiT;
						temr4 = - temrF + temrT;
						temi4 = temiS - temiH;

						temr[LHalfStart] = temr3 + temi3;
   			    		temi[LHalfStart] = temr4 + temi4;
						temr[LThreeFourthStart] =  temr3 - temi3;
						temi[LThreeFourthStart] =  temi4 - temr4;
					}

					temr[LStart] = temr1;
					temi[LStart] = temi1;
					temr[LFourthStart] = temr2;
					temi[LFourthStart] = temi2;

					LStart++;
					LFourthStart++;
					LHalfStart++;
					LThreeFourthStart++;
				}
			}
		}
		step <<= 1;
	}

	for(LStart=0;LStart<len;LStart+=2)
	{
		//calculate 2 points DFT
		if(isThisLevel[LStart]==levelNum)
		{
			temr1 = temr[LStart] + temr[LStart+1];
			temi1 = temi[LStart] + temi[LStart+1];
			temr[LStart+1] = temr[LStart] - temr[LStart+1];
			temi[LStart+1] = temi[LStart] - temi[LStart+1];
			temr[LStart] = temr1;
			temi[LStart] = temi1;
		}
	}

	/* reorder */
	for(i=0,j=0;i<len-1;i++)
	{
		if(i<=j)
		{
			real[i] = temr[j];
			imag[i] = temi[j];
			real[j] = temr[i];
			imag[j] = temi[i];
		}
		n = len >> 1;
		while(n<j+1)
		{
			j = j - n;
			n = n >> 1;
		}
		j = j + n;
	}
	real[len-1] = temr[len-1];
	imag[len-1] = temi[len-1];

	free(twiddleFactor);
	twiddleFactor = NULL;
	free(temr);
	temr = NULL;
	free(temi);
	temi = NULL;
	free(isThisLevel);
	isThisLevel = NULL;

	return 0;
}
/*Unified interface*/

int FFT(float *real,float *imag,int len)
{
	int i,j,k;
	
	
		memset(imag ,0,sizeof(float)*len);
		FFT2(real,imag,len);
		k=len/2;
		for (j=0;j<k;j+=2)
		{
			imag[j+k]=real[j]/len;
			imag[j+k+1]=real[j+1]/len;	

		}
		for (j=0;j<k;j++)
		{
			i=2*j;
			real[i]=imag[j+k];
			real[i+1]=imag[j]/len;
		}
		return i;
	
}

int InvFFT(float *real,float *imag,int len)
{
	int i,j,k;	
	k=len/2;
	for (j=0;j<k;j++)
	{
		i=2*j;
		imag[j]=real[i+1]*len;		
		real[j]=real[i]*len;		
	}
	imag[k]=0.f;
	real[k]=0.f;

	for (j=1;j<k;j++)
	{		
		real[len-j]=real[j];
		imag[len-j]=-imag[j];
	}
	return InvFFT2(real,imag,len);

}