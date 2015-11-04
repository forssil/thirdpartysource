#include "AudioBufferManager.h"
#include "AudioProcessorBase.h"
#include "WaveIO.h"
#include <stdlib.h>

int main()
{
	int fs = 22050;
	int frameTime = 10; //ms
	int minFrameSize = fs*frameTime/1000;

	AudioBufferParam_t param;
	param.mLenOfSample = sizeof(float);
	param.mInitDelaySize = minFrameSize;
	param.mPostProcessBufferLen 
		= 100 * fs / 1000 * param.mLenOfSample;
	param.mFrameShiftSize = minFrameSize;
	param.mInChannelNum = 1;
	param.mOutChannelNum = 1;

	AudioBufferManager audioBufferManager;
	audioBufferManager.init(param);

	SWavFileHead readwavhead, writewavhead;
	short *data_in_s, *data_out_s;
	float *data_in_f, *data_out_f;
	float *data_in_f2, *data_out_f2;
	char *infile;
	char outfile[256];
	CWavFileOp *readfile;
	CWavFileOp *writefile;

	long filelen, chanlelen;
	long i, k;
	long chanle_spos;
	long outfileleng;
	long counttime = 0;
	
	infile = "test.wav";
	i = 0;
	while ((infile[i] != '.') && (i<240))
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


	readfile->ReadHeader(&readwavhead);

	if (readwavhead.NChannels>2)
	{
		return 0;
	}
	filelen = readwavhead.RawDataFileLength / readwavhead.BytesPerSample*readwavhead.NChannels;


	int fremaelen = minFrameSize;
	int maxMultipleOfMinFrameSize = 4;
	//write file
	writewavhead = readwavhead;
	writewavhead.NChannels = 2;
	data_in_s = new short[maxMultipleOfMinFrameSize*fremaelen*(readwavhead.NChannels + writewavhead.NChannels)];
	data_out_s = data_in_s + maxMultipleOfMinFrameSize*fremaelen*readwavhead.NChannels;
	memset(data_in_s, 0, maxMultipleOfMinFrameSize*fremaelen*(readwavhead.NChannels + writewavhead.NChannels)*sizeof(short));


	data_in_f = new float[maxMultipleOfMinFrameSize*fremaelen * 2];
	data_out_f = data_in_f + maxMultipleOfMinFrameSize*fremaelen;
	memset(data_in_f, 0, (maxMultipleOfMinFrameSize*fremaelen * 2)*sizeof(float));
	writefile->WriteHeader(writewavhead);

	data_in_f2 = new float[maxMultipleOfMinFrameSize*fremaelen * 2];
	data_out_f2 = data_in_f2 + maxMultipleOfMinFrameSize*fremaelen;
	memset(data_in_f2, 0, (maxMultipleOfMinFrameSize*fremaelen * 2)*sizeof(float));
	//c
	outfileleng = 0;
	while (outfileleng < (filelen - maxMultipleOfMinFrameSize*minFrameSize*readwavhead.NChannels))
	{
		int multiple = (rand()%maxMultipleOfMinFrameSize) + 1;
		fremaelen = multiple * minFrameSize;

		outfileleng += readfile->ReadSample(data_in_s, fremaelen*readwavhead.NChannels);
		if (outfileleng >= 73000 * readwavhead.NChannels)
			outfileleng *= 1;
		for (i = 0; i < fremaelen; i++)
		{
			data_in_f[i] = float(data_in_s[i*readwavhead.NChannels]) / 32768.f;//left channele
			data_in_f2[i] = float(data_in_s[i*readwavhead.NChannels + 1]) / 32768.f;//right channele
		}

		//////////////////////////////////////////////////////////////////////////
		//processing

		const float* headBuff = NULL;
		const float* srcBuff = data_in_f;
		unsigned int headDataSize = 0;
		unsigned int dataSize = fremaelen;
		unsigned int outBufferLen = 0;
		audioBufferManager.bufferSrcData((const void**)&srcBuff, dataSize,
			(const void**)&headBuff, headDataSize, outBufferLen);

		memcpy_s(data_out_f2, fremaelen*sizeof(float), headBuff, headDataSize*sizeof(float));
		memcpy_s(data_out_f2+headDataSize, (fremaelen-headDataSize)*sizeof(float), data_in_f, dataSize*sizeof(float));

		fremaelen = dataSize + headDataSize;
		memcpy_s(data_out_f, fremaelen*sizeof(float), data_out_f2, fremaelen*sizeof(float));


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

	
	writefile->UpdateHeader(writewavhead.NChannels, outfileleng / writewavhead.NChannels);
	printf("done\n");

	delete readfile;
	delete writefile;
	delete data_in_f;
	delete data_in_s;
	delete data_in_f2;
	getchar();
	return 1;
}