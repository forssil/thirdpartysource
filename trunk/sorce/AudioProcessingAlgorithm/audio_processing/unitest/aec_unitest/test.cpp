

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

#define framesize 10.f
#define floatfile



int main(int argc , char *argv[ ])
{
	
	SWavFileHead readwavhead,writewavhead;
	short *data_in_s,*data_out_s;
	float *data_in_f,*data_out_f;
	float *data_in_f2, *data_out_f2,*data_out_f3;
	char infile[200];
	char outfile1[200];
	char outfile2[200];
	CWavFileOp *readfile;
	CWavFileOp **writefile; // for out
	CWavFileOp **writefile1; // for est
	audio_pro_share sharedata;
	long filelen,chanlelen;
	long i,k,micNum;
	long chanle_spos;
	long outfileleng;
	long counttime=0;
	//infile=argv[1];
	// infile: [near1, near2, ..., far]
	// outfile1(i): [near1i, outi]
	// outfile2(i): [near2i, outi] 
	char ROOT[200] = "D:/program/thirdpartysource/trunk/sorce/AudioProcessingAlgorithm/audio_processing/unitest/aec_unitest/";
	memcpy(infile, ROOT, 200 * sizeof(char));
	strcat(infile, "example.wav");
	memcpy(outfile1, ROOT, 200 * sizeof(char));
	strcat(outfile1, "Err");
	memcpy(outfile2, ROOT, 200 * sizeof(char));
	strcat(outfile2, "LinearErr");
	i=0;
	
	// read input wav
	readfile=new CWavFileOp(infile,"rb");
	if (readfile->m_FileStatus==-2)
	{
		delete readfile;
		printf("open infile failed!\n");
		return 0;
	}
	readfile->ReadHeader(&readwavhead);
	micNum = readwavhead.NChannels - 1;

	// prepare output wav
	writefile = new CWavFileOp*[micNum];
	writefile1 = new CWavFileOp*[micNum];
	for (int i = 0; i < micNum; i++) {
		char tmp[4];
		sprintf(tmp, "%d", i);
		strcat(outfile1, tmp);
		strcat(outfile1, ".wav");
		writefile[i] = new CWavFileOp(outfile1, "wb");
		if (writefile[i]->m_FileStatus == -2)
		{
			delete writefile[i];
			return 0;
		}
		writewavhead = readwavhead;
		writewavhead.NChannels = 2;
		writefile[i]->WriteHeader(writewavhead);
		strcat(outfile2, tmp);
		strcat(outfile2, ".wav");
		writefile1[i] = new CWavFileOp(outfile2, "wb");
		writefile1[i]->WriteHeader(writewavhead);
		if (writefile1[i]->m_FileStatus == -2)
		{
			delete writefile1[i];
			return 0;
		}
	}

	//int fremaelen = int(framesize*readwavhead.SampleRate/1000);
	int fremaelen = 512;
//create AEC
	// TODO: currently fft len = frame len * 2
	CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(micNum, readwavhead.SampleRate, 2 * fremaelen, fremaelen);
	pAPFInterface->Init();
	// prepare share data
	

	//if (readwavhead.NChannels>2)
	//{
	//	return 0;
	//}
	filelen=readwavhead.RawDataFileLength/readwavhead.BytesPerSample*readwavhead.NChannels;

//write file
	data_in_s=new short[fremaelen*(readwavhead.NChannels+writewavhead.NChannels)];
	data_out_s=data_in_s+fremaelen*readwavhead.NChannels;   
	memset(data_in_s,0,fremaelen*(readwavhead.NChannels+writewavhead.NChannels)*sizeof(short));

	
	data_in_f=new float[fremaelen*writewavhead.NChannels];
	data_out_f=data_in_f+fremaelen;
	memset(data_in_f,0,(fremaelen*writewavhead.NChannels)*sizeof(float));

	data_in_f2 = new float[fremaelen * 3];
	data_out_f2 = data_in_f2 + fremaelen;
	data_out_f3 = data_out_f2 + fremaelen;
	memset(data_in_f2, 0, (fremaelen * 3)*sizeof(float));
	//c
	counttime=clock();
	outfileleng = 0;
	LONGLONG elapseTimeCount = 0;
	int cycleNum = 1;
	int insideCycleNum = 0;
	// prepare share data
	sharedata.ppCapture_ = new float*[micNum];
	sharedata.ppProcessOut_ = new float*[micNum];
	for (int j = 0; j < micNum; j++) {
		sharedata.ppCapture_[j] = new float[fremaelen];
		sharedata.ppProcessOut_[j] = new float[fremaelen];
	}
	//memset(&sharedata, 0, sizeof(audio_pro_share));
	sharedata.bAECOn_=true;
	sharedata.bNROn_= false;
	sharedata.bNRCNGOn_=false;
    sharedata.nChannelsInCapture_ = 1;
	for (int i = 0; i < cycleNum; i++)
	{
		while (outfileleng < (filelen - fremaelen*readwavhead.NChannels))
		{
			LARGE_INTEGER startTime;
			LARGE_INTEGER finishTime;
			insideCycleNum++;

			outfileleng += readfile->ReadSample(data_in_s, fremaelen*readwavhead.NChannels);
			if (outfileleng>=3826* fremaelen*readwavhead.NChannels)
				outfileleng*=1;
			// link input data pointers
			for (int j = 0; j < micNum; j++) {
				for (i = 0; i < fremaelen; i++){
					data_in_f[i] = float(data_in_s[i*readwavhead.NChannels + j]) * 3 / 32768.f;
				}
				sharedata.ppCapture_[j] = data_in_f;
			}
			for (i = 0; i < fremaelen; i++) {
				data_in_f2[i] = float(data_in_s[i*readwavhead.NChannels + micNum]) * 3 / 32768.f;
			}
			sharedata.pReffer_ = data_in_f2;
			//sharedata.pDesire_=data_in_f;

			// process
			QueryPerformanceCounter(&startTime);
			pAPFInterface->process(sharedata);
			QueryPerformanceCounter(&finishTime);
			
			// link input data pointers
			//sharedata.pError_ = data_out_f;
			sharedata.pErrorBeforeNR_ = data_out_f3;
			/*memcpy_s(data_out_f2, fremaelen * sizeof(float), data_in_f, fremaelen * sizeof(float));*/
			for (int j = 0; j < micNum; j++) {
				//TODO: Linear Process Result is mono now
				memcpy_s(data_out_f, fremaelen * sizeof(float), sharedata.ppProcessOut_[j], fremaelen * sizeof(float));
				memcpy_s(data_out_f2, fremaelen * sizeof(float), sharedata.ppCapture_[j], fremaelen * sizeof(float));
				//DCRemover dcremover(0,0,0);
			    //dcremover.findLevelAndDcRemove(data_in_f, 0);
				for (int i = 0; i < (fremaelen); i++)
				{
					//data_out_s[i*writewavhead.NChannels  ]=data_in_s[i*readwavhead.NChannels];
					data_out_f[i] *= 32767.f / 3;
					if (data_out_f[i] > 32767.f)
					{
						data_out_s[i*writewavhead.NChannels] = 32767;
					}
					else if (data_out_f[i] < -32768.f)
					{
						data_out_s[i*writewavhead.NChannels] = -32768;
					}
					else
						data_out_s[i*writewavhead.NChannels] = short(data_out_f[i]);//*32768.f

					data_out_f2[i] *= 32767.f / 3;
					if (data_out_f2[i] > 32767.f)
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
				writefile[j]->WriteSample(data_out_s, (fremaelen*writewavhead.NChannels));//
				//////////////////
				for (int i = 0; i < (fremaelen); i++)
				{
					//data_out_s[i*writewavhead.NChannels  ]=data_in_s[i*readwavhead.NChannels];
					data_out_f3[i] *= 32767 / 3;
					if (data_out_f3[i] > 32767.f)
					{
						data_out_s[i*writewavhead.NChannels] = 32767;
					}
					else if (data_out_f3[i] < -32768.f)
					{
						data_out_s[i*writewavhead.NChannels] = -32768;
					}
					else
						data_out_s[i*writewavhead.NChannels] = short(data_out_f3[i]);//*32768.f

					//data_out_f2[i] *= 32767 / 3;
					//if (data_out_f2[i] > 32767.f)
					//{
					//	data_out_s[i*writewavhead.NChannels + 1] = 32767;
					//}
					//else if (data_out_f2[i] < -32768.f)
					//{
					//	data_out_s[i*writewavhead.NChannels + 1] = -32768;
					//}
					//else
					//	data_out_s[i*writewavhead.NChannels + 1] = short(data_out_f2[i]);//*32768.f
				}
				writefile1[j]->WriteSample(data_out_s, (fremaelen*writewavhead.NChannels));//
			}
			elapseTimeCount = elapseTimeCount + (finishTime.QuadPart - startTime.QuadPart);
			//	printf("elapseTimeCount is %d\n", elapseTimeCount);
		}

	}
	for (int j = 0; j < micNum; j++) {
		writefile1[j]->UpdateHeader(writewavhead.NChannels, outfileleng / writewavhead.NChannels);
		writefile[j]->UpdateHeader(writewavhead.NChannels, outfileleng / writewavhead.NChannels);
	}

	//LARGE_INTEGER fqOfCPU;
	//QueryPerformanceFrequency(&fqOfCPU);
	//printf("Frequency: %u\n", fqOfCPU.QuadPart);
	//counttime-=clock();
	//double elapseTime2 = (double)elapseTimeCount / fqOfCPU.QuadPart / insideCycleNum*1000;
	//printf("elapseTime: %fms\n", elapseTime2);

	//getchar();


	
	delete readfile;
	delete writefile;
	delete writefile1;
	delete data_in_f;
	delete data_in_s;
	delete data_in_f2;
	return 1;
}