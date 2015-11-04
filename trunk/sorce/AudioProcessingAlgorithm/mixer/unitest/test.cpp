#include "WaveIO.h"
#include "AudioMixerInterface.h"
#include <stdio.h>
#include <memory.h>

int main(int argc , char *argv[ ])
{
	SWavFileHead readwavhead;
	SWavFileHead readwavhead2;
	SWavFileHead writewavhead;
	short *data_in_s, *data_out_s;
	float *data_in_f, *data_out_f;
	char *infile;
	char *infile2;
	char outfile[256];
	CWavFileOp *readfile;
	CWavFileOp *readfile2;
	CWavFileOp *writefile;
	
	long filelen,filelen2,minfilelen;
	long i,k;
	long chanle_spos;
	long outfileleng,outfileleng2;

	infile  = "test1.wav";
	infile2 = "test2.wav";
	i=0;
	while((infile[i]!='.')&&(i<240))
	{
		outfile[i]=infile[i];
		i++;
	}
	outfile[i+0]='-';
	outfile[i+1]='o';
	outfile[i+2]='u';
	outfile[i+3]='t';
	outfile[i+4]='.';
	outfile[i+5]='w';
	outfile[i+6]='a';
	outfile[i+7]='v';
	outfile[i+8]='\0';
	
	readfile=new CWavFileOp(infile,"rb");
	if (readfile->m_FileStatus==-2)
	{
		delete readfile;
		printf("open infile failed!\n");
		return 0;
	}
	readfile2=new CWavFileOp(infile2,"rb");
	if (readfile2->m_FileStatus==-2)
	{
		delete readfile2;
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
	readfile2->ReadHeader(&readwavhead2);

	filelen=readwavhead.RawDataFileLength/readwavhead.BytesPerSample*readwavhead.NChannels;
	filelen2=readwavhead2.RawDataFileLength/readwavhead2.BytesPerSample*readwavhead2.NChannels;
	minfilelen = filelen<filelen2 ? filelen : filelen2;

	int fremaelen=440;
	//create AudioMixer
	AUDIO_CHANNEL_INFO channelInfo[3];
	channelInfo[0].mChannelDelay = 0;
	channelInfo[0].mGain = 1;
	channelInfo[0].mCompandorMode = 0;
	channelInfo[1].mChannelDelay = 0;
	channelInfo[1].mGain = 1;
	channelInfo[1].mCompandorMode = 0;
	channelInfo[2].mChannelDelay = 0;
	channelInfo[2].mGain = 1;
	channelInfo[2].mCompandorMode = 0;
	AUDIO_CHANNEL_INFO outChannelInfo;
	outChannelInfo.mChannelDelay = 0;
	outChannelInfo.mGain = 1;
	outChannelInfo.mCompandorMode = 0;
	AudioMixerInterface* pAudioMixerInterface = CreateIAudioMixerInst(channelInfo, outChannelInfo, 3, readwavhead.SampleRate);

//write file
	writewavhead=readwavhead;
	writewavhead.NChannels=2;
	data_in_s=new short[fremaelen*(readwavhead.NChannels+readwavhead2.NChannels+writewavhead.NChannels)];
	data_out_s=data_in_s+fremaelen*(readwavhead.NChannels+readwavhead2.NChannels);
	memset(data_in_s,0,fremaelen*(readwavhead.NChannels+readwavhead2.NChannels+writewavhead.NChannels)*sizeof(short));

	data_in_f=new float[fremaelen*4];
	data_out_f=data_in_f+fremaelen*3;
	memset(data_in_f,0,(fremaelen*4)*sizeof(float));
	writefile->WriteHeader(writewavhead);

	outfileleng = 0;
	outfileleng2 = 0;
	int number=0;
	while (outfileleng < (minfilelen - fremaelen*readwavhead.NChannels))
	{
		if (910==number)
		{
			number *= 1;
		}
		++number;
		outfileleng += readfile->ReadSample(data_in_s, fremaelen*readwavhead.NChannels);
		outfileleng2 +=readfile2->ReadSample(data_in_s+fremaelen*readwavhead.NChannels, fremaelen*readwavhead.NChannels);
		for (i = 0; i < fremaelen; i++)
		{
			data_in_f[i] = float(data_in_s[i*readwavhead.NChannels]) / 32768.f;//left channele
			data_in_f[i+fremaelen] = float(data_in_s[i*readwavhead.NChannels + 1]) / 32768.f;//right channele
			data_in_f[i+fremaelen*2] = float(data_in_s[i*readwavhead.NChannels +fremaelen*readwavhead.NChannels]) / 32768.f;//right channele
		}
		const float* ptrs[3];
		ptrs[0] = data_in_f;
		ptrs[1] = data_in_f+fremaelen;
		ptrs[2] = data_in_f+fremaelen*2;
		pAudioMixerInterface->Process(data_out_f, fremaelen, ptrs, fremaelen);
		for (i = 0; i<(fremaelen); i++)
		{
			data_out_f[i] *= 32767;
			if (data_out_f[i]>32767.f)
			{
				data_out_s[i*writewavhead.NChannels] = 32767;
				data_out_s[i*writewavhead.NChannels + 1] = 32767;
			}
			else if (data_out_f[i]<-32768.f)
			{
				data_out_s[i*writewavhead.NChannels] = -32768;
				data_out_s[i*writewavhead.NChannels + 1] = -32768;
			}
			else
			{
				data_out_s[i*writewavhead.NChannels] = short(data_out_f[i]);
				data_out_s[i*writewavhead.NChannels + 1] = short(data_out_f[i]);
			}		
		}
		writefile->WriteSample(data_out_s, (fremaelen*writewavhead.NChannels));
	}
	writefile->UpdateHeader(writewavhead.NChannels,outfileleng/writewavhead.NChannels);

	printf("done");
	getchar();
	DeleteIAudioMixerInst(pAudioMixerInterface);
	delete readfile;
	delete readfile2;
	delete writefile;
	delete data_in_f;
	delete data_in_s;
	return 1;
}