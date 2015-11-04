

#include <memory>
#include <stdio.h>


#include <time.h>
//#include "timecounter.h"
#include <windows.h>
#include "echocancellation_interface.h"
#include "WaveIO.h"
#include "fft.h"
#include "T2Ftransformer.h"
#include "F2Ttransformer.h"
#ifndef _CLOCK_T_DEFINED 
typedef long clock_t;
#define _CLOCK_T_DEFINED 
#endif 

#define framesize 04.f
#define floatfile



int main(int argc , char *argv[ ])
{
	
	SWavFileHead readwavhead,writewavhead;
	short *data_in_s,*data_out_s;
	float *data_in_f,*data_out_f;
	float *data_in_f2, *data_out_f2;
	char *infile;
	char *outfile;
	CWavFileOp *readfile;
	CWavFileOp *writefile;
	audio_pro_share sharedata;
	long filelen,chanlelen;
	long i,k;
	long chanle_spos;
	long outfileleng;
	long counttime=0;

	memset(&sharedata,0,sizeof(audio_pro_share));

		int fremaelen=512;//int(framesize*readwavhead.SampleRate/1000);
	//create AEC
	CEchoCancellationInterface* pAECInterface = CreateIAECInst_int(48000, 2*fremaelen, fremaelen);
	pAECInterface->Init();

   {
	    
	   //infile=argv[1];
		infile = "D:\\work\\work\\samplelib\\handmicsample.wav";
		outfile="D:\\work\\work\\samplelib\\handmicsample_err.wav";
	   i=0;
// 	   while((infile[i]!='.')&&(i<240))
// 	   {
// 		   outfile[i]=infile[i];
// 		   i++;
// 	   }
// 	   
// 	   outfile[i+0]='1';
// 	   outfile[i+1]='o';
// 	   outfile[i+2]='u';
// 	   outfile[i+3]='t';
// 
// 	  outfile[i+4]='.';
// 	  outfile[i+5]='w';
// 	  outfile[i+6]='a';
// 	  outfile[i+7]='v';
// 	  outfile[i+8]='\0';
   }

	
	readfile=new CWavFileOp(infile,"rb");
	if (readfile->m_FileStatus==-2)
	{
		delete readfile;
		printf("open infile failed!\n");
		return 0;
	}
	writefile=new CWavFileOp(outfile,"wb");
	if (writefile->m_FileStatus==-2)
	{
		delete writefile;
		return 0;
	}

	
	readfile->ReadHeader(&readwavhead);

	if (readwavhead.NChannels>2)
	{
		return 0;
	}
	filelen=readwavhead.RawDataFileLength/readwavhead.BytesPerSample*readwavhead.NChannels;



//write file
	writewavhead=readwavhead;
	writewavhead.NChannels=2;
	data_in_s=new short[fremaelen*(readwavhead.NChannels+writewavhead.NChannels)];
	data_out_s=data_in_s+fremaelen*readwavhead.NChannels;
	memset(data_in_s,0,fremaelen*(readwavhead.NChannels+writewavhead.NChannels)*sizeof(short));

	
	data_in_f=new float[fremaelen*2];
	data_out_f=data_in_f+fremaelen;
	memset(data_in_f,0,(fremaelen*2)*sizeof(float));
	writefile->WriteHeader(writewavhead);

	data_in_f2 = new float[fremaelen * 2];
	data_out_f2 = data_in_f2 + fremaelen;
	memset(data_in_f2, 0, (fremaelen * 2)*sizeof(float));
	//c
	counttime=clock();
	outfileleng = 0;
	LONGLONG elapseTimeCount = 0;
	int cycleNum = 1;
	int insideCycleNum = 0;
	sharedata.bAECOn_=true;
	sharedata.bNROn_= true;
	sharedata.bNRCNGOn_=true;
	for (int i = 0; i < cycleNum; i++)
	{
		while (outfileleng < (filelen - fremaelen*readwavhead.NChannels))
		{
			outfileleng += readfile->ReadSample(data_in_s, fremaelen*readwavhead.NChannels);
			if (outfileleng>=367000*readwavhead.NChannels)
				outfileleng*=1;
			for (i = 0; i < fremaelen; i++)
			{
				data_in_f[i] = float(data_in_s[i*readwavhead.NChannels]) / 32768.f;//left channele
				data_in_f2[i] = float(data_in_s[i*readwavhead.NChannels + 1]) / 32768.f;//right channele
			}
			//////////////////////////////////////////////////////////////////////////


			LARGE_INTEGER startTime;
			LARGE_INTEGER finishTime;
			insideCycleNum++;
			sharedata.pDesire_=data_in_f;
			sharedata.pReffer_ = data_in_f2;
			sharedata.pError_ = data_out_f;
			QueryPerformanceCounter(&startTime);
			pAECInterface->process(sharedata);
			QueryPerformanceCounter(&finishTime);
			
			elapseTimeCount = elapseTimeCount + (finishTime.QuadPart - startTime.QuadPart);

		//	printf("elapseTimeCount is %d\n", elapseTimeCount);
			
			memcpy_s(data_out_f2, fremaelen*sizeof(float), data_in_f, fremaelen*sizeof(float));

			//DCRemover dcremover(0,0,0);
			//dcremover.findLevelAndDcRemove(data_in_f, 0);

			/////////////
			for (i = 0; i<(fremaelen); i++)
			{
				//data_out_s[i*writewavhead.NChannels  ]=data_in_s[i*readwavhead.NChannels];
				data_out_f[i] *= 32767;
				if (data_out_f[i]>32767.f)
				{
					data_out_s[i*writewavhead.NChannels] = 32767;
				}
				else if (data_out_f[i]<-32768.f)
				{
					data_out_s[i*writewavhead.NChannels] = -32768;
				}
				else
					data_out_s[i*writewavhead.NChannels] = short(data_out_f[i]);//*32768.f

				data_out_f2[i] *= 32767;
				if (data_out_f2[i]>32767.f)
				{
					data_out_s[i*writewavhead.NChannels + 1] = 32767;
				}
				else if (data_out_f2[i] < -32768.f)
				{
					data_out_s[i*writewavhead.NChannels + 1] = -32768;
				}
				else
					data_out_s[i*writewavhead.NChannels + 1] = short(data_out_f2[i]);//*32768.f
			}
			writefile->WriteSample(data_out_s, (fremaelen*writewavhead.NChannels));//

		}
	}

	LARGE_INTEGER fqOfCPU;
	QueryPerformanceFrequency(&fqOfCPU);
	printf("Frequency: %u\n", fqOfCPU.QuadPart);
	counttime-=clock();
	double elapseTime2 = (double)elapseTimeCount / fqOfCPU.QuadPart / insideCycleNum*1000;
	printf("elapseTime: %fms\n", elapseTime2);

	getchar();

	writefile->UpdateHeader(writewavhead.NChannels,outfileleng/writewavhead.NChannels);

	
	delete readfile;
	delete writefile;
	delete data_in_f;
	delete data_in_s;
	delete data_in_f2;
	return 1;
}