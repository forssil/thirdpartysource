#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <stdbool.h>
#include "AudioAecCpp.h"
#include <time.h>
//#include "timecounter.h"
#ifdef WIN32
#include <windows.h>
#endif

#ifndef _CLOCK_T_DEFINED 
typedef long clock_t;
#define _CLOCK_T_DEFINED 
#endif 

#define framesize 04.f
#define floatfile

//void aec_processing(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out);
//void aec_processing_init(void  **p_aec);
//void aec_processing_deinit(void *h_aec);
void getconfigs(const char* input, Toggle3A *output) {
	FILE* fp = NULL;
	char mode[8];
	char tmp;
  	fp = fopen(input, "rb+");
	if (NULL == fp) {
		printf("open config file fail");
		return;  // cann't open the file
	}

	while (!feof(fp)) {
		auto size = fread(mode, 4, 1, fp);
		if(size == 0){
			break;
		}
		mode[4] = '\0';
		if (strcmp(mode, "AEC ") == 0) {
			fread(&tmp, 1, 1, fp);
			output->bAECOn_ = tmp - '0';
			fread(&tmp, 1, 1, fp);
		}
		else if (strcmp(mode, "AGC ") == 0) {
			fread(&tmp, 1, 1, fp);
			output->bAGCOn_ = tmp - '0';
			fread(&tmp, 1, 1, fp);
		}
		else if (strcmp(mode, "CNG ") == 0) {
			fread(&tmp, 1, 1, fp);
			output->bNRCNGOn_ = tmp - '0';
			fread(&tmp, 1, 1, fp);
		}
		else if (strcmp(mode, "NR_ ") == 0) {
			fread(&tmp, 1, 1, fp);
			output->bNROn_ = tmp - '0';
			fread(&tmp, 1, 1, fp);
		}
		else if (strcmp(mode, "RNN ") == 0) {
			fread(&tmp, 1, 1, fp);
			output->bRNNOISEOn_ = tmp - '0';
			fread(&tmp, 1, 1, fp);
			fread(mode, 7, 1, fp);
			mode[7] = '\0';
			if (strcmp(mode, "PRERNN ") == 0) {
				fread(&tmp, 1, 1, fp);
				output->bPreRnnOn_ = tmp - '0';
				fread(&tmp, 1, 1, fp);
			}
		}
	}
}

int main(int argc , char *argv[ ])
{
	FILE* input = NULL, *output = NULL;
	short *data_in_s,*data_out_s;
	float *data_in_f;
	float *data_in_f2, *data_out_f2;
	char *infile,*outfile,*configfile = NULL;
	long filelen;
	long outfileleng;
	int mics_num = 4;
    int channel_num = 5;
	Toggle3A config_;
	int fremaelen=480;//int(framesize*readwavhead.SampleRate/1000);
	printf("Usage: [infile.pcm] [outfile.pcm] [length] [config.txt(can be defaulted)]\n");
    
	if(argc < 3){
		return 0;
	}
	infile=argv[1];
	outfile = argv[2];
	long length = strtol(argv[3], NULL, 10);
	if(argc > 3){
		configfile = argv[4];
	}
	
	// infile = (char*)"test_short.pcm";
	// outfile=   (char*)"pcmout.pcm";
    
	filelen=(int)length*100*480 * channel_num;
    input = fopen(infile, "rb+");
    output = fopen(outfile, "wb+");
	if(NULL == input){
		printf("open infile failed!\n");
		return 0;
	}
	if(NULL == output){
		printf("open outfile failed!\n");
		return 0;
	}
	if(NULL == configfile){
		printf("use default config!\n");
		config_.bAECOn_ = true;
		config_.bAGCOn_ = true;
		config_.bNRCNGOn_ = false;
		config_.bNROn_ = true;
		config_.bPreRnnOn_ = true;
		config_.bRNNOISEOn_ = false;
	}
	else{
		printf("get external config!");
		getconfigs(configfile, &config_);
		printf("configs: AEC: %d AGC: %d NR_: %d CNG: %d RNN: %d PRERNN: %d\n",
				config_.bAECOn_,config_.bAGCOn_,config_.bNROn_,config_.bNRCNGOn_,config_.bPreRnnOn_,config_.bRNNOISEOn_);
	}

	data_in_s=new short[fremaelen*(channel_num+1)];
	data_out_s=data_in_s+fremaelen*channel_num;
	memset(data_in_s,0,fremaelen*(channel_num+1)*sizeof(short));

	
	data_in_f=new float[fremaelen*(1 + mics_num)];
	memset(data_in_f,0,(fremaelen*(1 + mics_num))*sizeof(float));

	data_in_f2 = new float[fremaelen * 2];
	data_out_f2 = data_in_f2 + fremaelen;
	memset(data_in_f2, 0, (fremaelen * 2)*sizeof(float));

    short **micin = new short*[mics_num];
    for (int i = 0; i < mics_num; i++) {
        micin[i] = new short[512];
    }
    short farin[512] = { 0 };
    short errout[512] = { 0 };
    aec_processing_init_cpp(nullptr, (void*)&config_);
	outfileleng = 0;
	
	int cycleNum = 1;
	int insideCycleNum = 0;
	for (int i = 0; i < cycleNum; i++)
	{
		while (outfileleng < (filelen - fremaelen * channel_num))
		{
			//printf("outfileleng: %ld\n", outfileleng);
			int read_len = fread(data_in_s,sizeof(short),fremaelen * channel_num,input);
			outfileleng += read_len;
			//if (outfileleng>=3826* fremaelen*readwavhead.NChannels)
			//	outfileleng*=1;
			for (i = 0; i < fremaelen; i++)
			{
				for (size_t channel = 0; channel < (size_t)mics_num; channel++)
				{
                    micin[channel][i] = data_in_s[i*channel_num + channel];
				}
                farin[i] = data_in_s[i*channel_num + mics_num];
			}
			insideCycleNum++;
            aec_processing_cpp(nullptr, micin, farin, nullptr, 0, errout, 1, 0);
#ifdef WIN32
			QueryPerformanceCounter(&finishTime);
			elapseTimeCount = elapseTimeCount + (finishTime.QuadPart - startTime.QuadPart);		
#endif	

			memcpy(data_out_f2, data_in_f, fremaelen*sizeof(float));

			/////////////
			int channel = 0;
			for (int i = 0; i<(fremaelen); i++)
			{
				//for (size_t channel = 0; channel < (size_t)mics_num; channel++)
				
				{                    
					if (errout[i] > 32767.f)
					{
						data_out_s[i + channel] = 32767;
					}
					else if (errout[i] < -32768.f)
					{
						data_out_s[i + channel] = -32768;
					}
                    else {
                        data_out_s[i + channel] = errout[i];
                    }
				}				
			}
			fwrite(data_out_s,sizeof(short),fremaelen,output);
		}
	}
	
	// writefile->UpdateHeader(writewavhead.NChannels, outfileleng / writewavhead.NChannels);
#ifdef WIN32
	LARGE_INTEGER fqOfCPU;
	QueryPerformanceFrequency(&fqOfCPU);
	printf("Frequency: %u\n", fqOfCPU.QuadPart);
	//counttime-=clock();
	double elapseTime2 = (double)elapseTimeCount / fqOfCPU.QuadPart / insideCycleNum*1000;
	printf("elapseTime: %fms\n", elapseTime2);

	//getchar();

#endif
	//agc_destroy(pAgc);
	//agc_new_destroy(pAgc_new);
	// delete readfile;
	// delete writefile;
	fclose(input);
	fclose(output);
	//delete data_in_f;
	delete data_in_s;
    if (micin) {
        for (int i = 0; i < mics_num; i++) {
            delete[] micin[i];
        }
        delete[] micin;
    }
	//delete data_in_f2;
	//delete sharedata.ppCapture_ ;
	//delete sharedata.ppProcessOut_;
    aec_processing_deinit_cpp(nullptr);
    //aec_processing_deinit(nullptr);
	return 1;
}
