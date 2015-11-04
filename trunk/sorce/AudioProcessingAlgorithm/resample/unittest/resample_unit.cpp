// resample_unit.cpp : Defines the entry point for the console application.
//


#include "WaveIO.h"
#include <windows.h>
#include <time.h>
#include "ReSampleImpl.h"



#define  framesize  10
#define  samplerate_out 44100

int main(int argc, char *argv[])
{
	SWavFileHead readwavhead, writewavhead;
	short *data_in_s, *data_out_s;
	float *data_in_f, *data_out_f;
	char *infile;
	char outfile[256];
	CWavFileOp *readfile;
	CWavFileOp *writefile;
	ReSampleImpl *ReSample;

	long filelen;
	long i;
	long outfileleng;
	long write_outfileleng;
	long samplerate_in = 0;
	int cycleNum = 1;
	int insideCycleNum = 0;

	LONGLONG elapseTimeCount = 0;

	infile = "test1_22050.wav";
	i = 0;
	while ((infile[i] != '.') && (i < 240))
	{
		outfile[i] = infile[i];
		i++;
	}

	outfile[i + 0] = '-';
	outfile[i + 1] = 'o';
	outfile[i + 2] = 'u';
	outfile[i + 3] = 't';

	outfile[i + 4] = '.';
	outfile[i + 5] = 'w';
	outfile[i + 6] = 'a';
	outfile[i + 7] = 'v';
	outfile[i + 8] = '\0';

	for (int i = 0; i < cycleNum; i++)
	{



		readfile = new CWavFileOp(infile, "rb");
		if (readfile->m_FileStatus == -2)
		{
			delete readfile;
			printf("open infile failed!\n");
			return 0;
		}
		writefile = new CWavFileOp(outfile, "wb");
		if (writefile->m_FileStatus == -2)
		{
			delete writefile;
			return 0;
		}

		ReSample = new ReSampleImpl(1);

		readfile->ReadHeader(&readwavhead);

		if (readwavhead.NChannels > 2)
		{
			return 0;
		}
		filelen = readwavhead.RawDataFileLength / readwavhead.BytesPerSample*readwavhead.NChannels;

		int fremaelen = int(framesize*readwavhead.SampleRate / 1000);
		int writeframelen = int(framesize*samplerate_out / 1000);

		//write file
		writewavhead = readwavhead;
		samplerate_in = readwavhead.SampleRate;
		writewavhead.SampleRate = samplerate_out;
		writewavhead.SampleBytes = readwavhead.SampleBytes / readwavhead.SampleRate*samplerate_out;
		data_in_s = new short[fremaelen*readwavhead.NChannels + writeframelen*writewavhead.NChannels];
		data_out_s = data_in_s + fremaelen*readwavhead.NChannels;
		memset(data_in_s, 0, (fremaelen*readwavhead.NChannels + writeframelen*writewavhead.NChannels)*sizeof(short));


		data_in_f = new float[fremaelen*readwavhead.NChannels + writeframelen*writewavhead.NChannels];
		data_out_f = data_in_f + fremaelen*readwavhead.NChannels;
		memset(data_in_f, 0, (fremaelen*readwavhead.NChannels + writeframelen*writewavhead.NChannels)*sizeof(float));

		writefile->WriteHeader(writewavhead);


		outfileleng = 0;
		write_outfileleng = 0;
		
		

		while (outfileleng < (filelen - fremaelen*readwavhead.NChannels))
		{
			outfileleng += readfile->ReadSample(data_in_s, fremaelen*readwavhead.NChannels);
			if (outfileleng >= 9273 * readwavhead.NChannels)
				outfileleng *= 1;

			for (int j = 0; j < fremaelen; j++)
			{
				data_in_f[j] = float(data_in_s[j*readwavhead.NChannels]) / 32768.f;//left channele
			}

			insideCycleNum++;

			unsigned int actual_leng = 0;
			LARGE_INTEGER startTime;
			LARGE_INTEGER finishTime;
			QueryPerformanceCounter(&startTime);

			ReSample->Process(data_in_f, data_out_f, fremaelen, writeframelen, samplerate_in, samplerate_out, actual_leng);

			QueryPerformanceCounter(&finishTime);

			elapseTimeCount = elapseTimeCount + (finishTime.QuadPart - startTime.QuadPart);


			for (int k = 0; k<(actual_leng); k++)
			{
				data_out_f[k] *= 32767;
				if (data_out_f[k]>32767.f)
				{
					data_out_s[k*writewavhead.NChannels] = 32767;
					data_out_s[k*writewavhead.NChannels+1] = 32767;
				}
				else if (data_out_f[k]<-32768.f)
				{
					data_out_s[k*writewavhead.NChannels] = -32768;
					data_out_s[k*writewavhead.NChannels+1] = -32768;
				}
				else
				{
					data_out_s[k*writewavhead.NChannels] = short(data_out_f[k]);//*32768.f
					data_out_s[k*writewavhead.NChannels+1] = short(data_out_f[k]);//*32768.f
				}
			}
			writefile->WriteSample(data_out_s, (actual_leng*writewavhead.NChannels));//
			write_outfileleng += actual_leng*writewavhead.NChannels;
		}

		writefile->UpdateHeader(writewavhead.NChannels, write_outfileleng / writewavhead.NChannels);


		delete readfile;
		delete writefile;
		delete data_in_f;
		delete data_in_s;
		delete ReSample;



		if (0 == i % 10)
		{
			printf("cycle num : %d\n", i);
			LARGE_INTEGER fqOfCPU;
			QueryPerformanceFrequency(&fqOfCPU);
			printf("Frequency: %u\n", fqOfCPU.QuadPart);
			double elapseTime2 = (double)elapseTimeCount / fqOfCPU.QuadPart / insideCycleNum * 1000;
			printf("elapseTime: %fms\n", elapseTime2);
			elapseTimeCount = 0.f;
			insideCycleNum = 0;
			

		}

	}



	getchar();
	return 0;
}

