

#include <memory>
#include <stdio.h>


#include <time.h>
//#include "timecounter.h"
#include <windows.h>
#include "AudioProcessingFramework_interface.h"
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
	
	SWavFileHead readwavhead,writewavhead, writewavhead1;
	short *data_in_s,*data_out_s;
	float *data_in_f,*data_out_f;
	float *data_in_f2, *data_out_f2,*data_out_f3;
	char *infile;
	char *outfile;
	char *outfile1;
	CWavFileOp *readfile;
	CWavFileOp *writefile;
	CWavFileOp *writefile1;
	audio_pro_share sharedata;
	long filelen,chanlelen;
	long i,k;
	long chanle_spos;
	long outfileleng;
	long counttime=0;
	int mics_num = 0;

	memset(&sharedata,0,sizeof(audio_pro_share));

	int fremaelen=512;//int(framesize*readwavhead.SampleRate/1000);

   {
	    
	   //infile=argv[1];
		infile = "D:\\hardware\chenan\\3308_mca_dump\\5channel.wav";
		outfile="D:\\hardware\\chenan\\3308_mca_dumpk\\out\\5channel_out.wav";
		outfile1 = "D:\\hardware\\chenan\\3308_mca_dump\\out\\5channel_out1.wav";
	   i=0;

   }

	
	readfile=new CWavFileOp(infile,"rb");
	if (readfile->m_FileStatus==-2)
	{
		delete readfile;
		printf("open infile failed!\n");
		return 0;
	}
	mics_num = readwavhead.NChannels - 1;

	writefile1 = new CWavFileOp(outfile1, "wb");
	if (writefile1->m_FileStatus == -2)
	{
		delete writefile1;
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
	writewavhead1 = readwavhead;
	writewavhead1.NChannels = 2;
//buffer
	data_in_s=new short[fremaelen*(readwavhead.NChannels+writewavhead.NChannels)];
	data_out_s=data_in_s+fremaelen*readwavhead.NChannels;
	memset(data_in_s,0,fremaelen*(readwavhead.NChannels+writewavhead.NChannels)*sizeof(short));

	
	data_in_f=new float[fremaelen*(2 + 2*mics_num)];
	data_out_f=data_in_f+fremaelen;
	memset(data_in_f,0,(fremaelen*(2 + 2*mics_num))*sizeof(float));
	writefile->WriteHeader(writewavhead);
	writefile1->WriteHeader(writewavhead1);

	data_in_f2 = new float[fremaelen * 3];
	data_out_f2 = data_in_f2 + fremaelen;
	data_out_f3 = data_out_f2 + fremaelen;
	memset(data_in_f2, 0, (fremaelen * 3)*sizeof(float));

	//create AEC
	CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(mics_num, readwavhead.SampleRate, 2 * fremaelen, fremaelen);
	pAPFInterface->Init();
    //sharedata init
	sharedata.ppCapture_ = new float*[mics_num];
	sharedata.nChannelsInCapture_ = mics_num;
	sharedata.nSamplesPerCaptureChannel_ = fremaelen;
	sharedata.ppProcessOut_ = new float*[mics_num];
	sharedata.nChannelsInProcessOut_ = mics_num;
	sharedata.nSamplesPerProcessOutChannel_ = fremaelen;

	for (int i = 0; i < mics_num; i++) {
		sharedata.ppCapture_[i] = data_out_f + i * fremaelen;
		sharedata.ppProcessOut_[i] = data_out_f + i * fremaelen+ mics_num* fremaelen;
	}
	sharedata.pReffer_ = data_in_f2;
	sharedata.nSamplesInReffer_ = fremaelen;

	sharedata.bAECOn_=true;
	sharedata.bNROn_= true;
	sharedata.bNRCNGOn_=false;

	//c
	counttime = clock();
	outfileleng = 0;
	LONGLONG elapseTimeCount = 0;
	int cycleNum = 1;
	int insideCycleNum = 0;

	for (int i = 0; i < cycleNum; i++)
	{
		while (outfileleng < (filelen - fremaelen*readwavhead.NChannels))
		{
			outfileleng += readfile->ReadSample(data_in_s, fremaelen*readwavhead.NChannels);
			//if (outfileleng>=3826* fremaelen*readwavhead.NChannels)
			//	outfileleng*=1;
			for (i = 0; i < fremaelen; i++)
			{
				for (size_t channel = 0; channel < mics_num; channel++)
				{
					sharedata.ppCapture_[channel][i] = float(data_in_s[i*readwavhead.NChannels+ channel]);
				}
				
				sharedata.pReffer_[i] = float(data_in_s[i*readwavhead.NChannels + mics_num]);
			}
			//////////////////////////////////////////////////////////////////////////


			LARGE_INTEGER startTime;
			LARGE_INTEGER finishTime;
			insideCycleNum++;
			sharedata.pDesire_=data_in_f;
			sharedata.pReffer_ = data_in_f2;
			sharedata.pError_ = data_out_f;
			sharedata.pErrorBeforeNR_ = data_out_f3;
			QueryPerformanceCounter(&startTime);
			pAPFInterface->process(sharedata);
			QueryPerformanceCounter(&finishTime);
			
			elapseTimeCount = elapseTimeCount + (finishTime.QuadPart - startTime.QuadPart);		
			 
			memcpy_s(data_out_f2, fremaelen*sizeof(float), data_in_f, fremaelen*sizeof(float));

			/////////////
			for (int i = 0; i<(fremaelen); i++)
			{
				//data_out_s[i*writewavhead.NChannels  ]=data_in_s[i*readwavhead.NChannels];
				data_out_f[i] *= 32767.f/3;
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

				data_out_f2[i] *= 32767.f/3;
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
		    //////////////////
			for (int i = 0; i < (fremaelen); i++)
			{
				//data_out_s[i*writewavhead.NChannels  ]=data_in_s[i*readwavhead.NChannels];
				data_out_f3[i] *= 32767 / 3;
				if (data_out_f3[i] > 32767.f)
				{
					data_out_s[i*writewavhead1.NChannels] = 32767;
				}
				else if (data_out_f3[i] < -32768.f)
				{
					data_out_s[i*writewavhead1.NChannels] = -32768;
				}
				else
					data_out_s[i*writewavhead1.NChannels] = short(data_out_f3[i]);//*32768.f

			}
			writefile1->WriteSample(data_out_s, (fremaelen*writewavhead1.NChannels));//
		}
	}
	writefile1->UpdateHeader(writewavhead1.NChannels, outfileleng / writewavhead1.NChannels);
	writefile->UpdateHeader(writewavhead.NChannels, outfileleng / writewavhead.NChannels);
	
	LARGE_INTEGER fqOfCPU;
	QueryPerformanceFrequency(&fqOfCPU);
	printf("Frequency: %u\n", fqOfCPU.QuadPart);
	counttime-=clock();
	double elapseTime2 = (double)elapseTimeCount / fqOfCPU.QuadPart / insideCycleNum*1000;
	printf("elapseTime: %fms\n", elapseTime2);

	getchar();


	
	delete readfile;
	delete writefile;
	delete writefile1;
	delete data_in_f;
	delete data_in_s;
	delete data_in_f2;
	return 1;
}
