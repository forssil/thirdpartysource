

#include <memory>
#include <stdio.h>
#include "AudioAec.h"

#include <time.h>
//#include "timecounter.h"
#ifdef WIN32
#include <windows.h>
#endif

#include "AudioProcessingFramework_interface.h"
#include "agc.h"
#include "agc_new.h"
#include "WaveIO.h"
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
	//audio_pro_share sharedata;
	long filelen,chanlelen;
	long i,k;
	long chanle_spos;
	long outfileleng;
	long counttime=0;
	int mics_num = 0;
	struct AGCSTATE* pAgc;
	//struct AGCSTATE_NEW* pAgc_new;

	//memset(&sharedata,0,sizeof(audio_pro_share));

	int fremaelen=480;//int(framesize*readwavhead.SampleRate/1000);

   {
	    
	   //infile=argv[1];
		/*infile = "D:\\works\\chenan\\3308_mca_dump\\5channel.wav";
		outfile=   "D:\\works\\chenan\\3308_mca_dump\\out\\5channel_out.wav";
		outfile1 = "D:\\works\\chenan\\3308_mca_dump\\out\\5channel_out1.wav";*/
	   i=0;
	   infile = "C:/Users/carlzhang/Downloads/5channel.wav";
	   outfile = "C:/Users/carlzhang/Downloads/5channel_out_agc.wav";
	   outfile1 = "C:/Users/carlzhang/Downloads/5channel_out1.wav";

   }

	
	readfile=new CWavFileOp(infile,"rb");
	if (readfile->m_FileStatus==-2)
	{
		delete readfile;
		printf("open infile failed!\n");
		return 0;
	}
	readfile->ReadHeader(&readwavhead);
	mics_num = readwavhead.NChannels - 1;

	//writefile1 = new CWavFileOp(outfile1, "wb");
	//if (writefile1->m_FileStatus == -2)
	//{
	//	delete writefile1;
	//	return 0;
	//}
	writefile=new CWavFileOp(outfile,"wb");
	if (writefile->m_FileStatus==-2)
	{
		delete writefile;
		return 0;
	}

	

	filelen=readwavhead.RawDataFileLength/readwavhead.BytesPerSample*readwavhead.NChannels;



//write file
	writewavhead=readwavhead;
	//writewavhead.NChannels=2;
	writewavhead1 = readwavhead;
	//writewavhead1.NChannels = 2;
//buffer
	data_in_s=new short[fremaelen*(readwavhead.NChannels+writewavhead.NChannels)];
	data_out_s=data_in_s+fremaelen*readwavhead.NChannels;
	memset(data_in_s,0,fremaelen*(readwavhead.NChannels+writewavhead.NChannels)*sizeof(short));

	
	data_in_f=new float[fremaelen*(2 + 2*mics_num)];
	data_out_f=data_in_f+fremaelen;
	memset(data_in_f,0,(fremaelen*(2 + 2*mics_num))*sizeof(float));
	writefile->WriteHeader(writewavhead);

	data_in_f2 = new float[fremaelen * 3];
	data_out_f2 = data_in_f2 + fremaelen;
	data_out_f3 = data_out_f2 + fremaelen;
	memset(data_in_f2, 0, (fremaelen * 3)*sizeof(float));

	//create AEC
	//CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(mics_num, readwavhead.SampleRate, 2 * fremaelen, fremaelen);
	//pAPFInterface->Init();
	//create AGC
	pAgc = agc_create();
	agc_reset(pAgc);
	//pAgc_new = agc_new_create();
	//agc_new_reset(pAgc_new);

    //sharedata init
	//sharedata.ppCapture_ = new float*[mics_num];
	//sharedata.nChannelsInCapture_ = mics_num;
	//sharedata.nSamplesPerCaptureChannel_ = fremaelen;
	//sharedata.ppProcessOut_ = new float*[mics_num];
	//sharedata.nChannelsInProcessOut_ = mics_num;
	//sharedata.nSamplesPerProcessOutChannel_ = fremaelen;

	//for (int i = 0; i < mics_num; i++) {
	//	sharedata.ppCapture_[i] = data_out_f + i * fremaelen;
	//	sharedata.ppProcessOut_[i] = data_out_f + i * fremaelen+ mics_num* fremaelen;
	//}
	//sharedata.pReffer_ = data_in_f2;
	//sharedata.nSamplesInReffer_ = fremaelen;

	//sharedata.bAECOn_=true;
	//sharedata.bNROn_= true;
	//sharedata.bNRCNGOn_=false;

    short mic[6][480] = { 0 };
    short **micin = new short*[4];
    for (int i = 0; i < 4; i++) {
        micin[i] = new short[480];
    }
    short farin[480] = { 0 };
    short errout[480] = { 0 };
    aec_processing_init(nullptr);
	//c
	counttime = clock();
	outfileleng = 0;
#ifdef WIN32
	LONGLONG elapseTimeCount = 0;
#endif
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
					//sharedata.ppCapture_[channel][i] = float(data_in_s[i*readwavhead.NChannels+ channel])/32768.f;
                    micin[channel][i] = data_in_s[i*readwavhead.NChannels + channel];
				}
				
				//sharedata.pReffer_[i] = float(data_in_s[i*readwavhead.NChannels + mics_num])/32768.f;
                farin[i] = data_in_s[i*readwavhead.NChannels + mics_num];
			}
			//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
			LARGE_INTEGER startTime;
			LARGE_INTEGER finishTime;
#endif
			insideCycleNum++;
			//sharedata.pDesire_=data_in_f;
			//sharedata.pReffer_ = data_in_f2;
			//sharedata.pError_ = data_out_f;
#ifdef AUDIO_WAVE_DEBUG
			//sharedata.pErrorBeforeNR_ = data_out_f3;
#endif
#ifdef WIN32
			QueryPerformanceCounter(&startTime);
#endif
			//pAPFInterface->process(sharedata);
            
            aec_processing(nullptr, micin, farin, nullptr, 0, errout);
#ifdef WIN32
			QueryPerformanceCounter(&finishTime);
			elapseTimeCount = elapseTimeCount + (finishTime.QuadPart - startTime.QuadPart);		
#endif	

			memcpy(data_out_f2, data_in_f, fremaelen*sizeof(float));


			// do agc for every output channel
			for (size_t channel = 0; channel < mics_num; channel++) {
				float gain = 1, power = 0;
				for (int i = 0; i < fremaelen; i++) {
					//power += abs(sharedata.ppProcessOut_[channel][i]);
                    power += abs((float)errout[i]/32768);
				}
				power /= fremaelen;
				agc_process(pAgc, 1, &power, &gain, 0);
				//agc_new_process(pAgc_new, 1, &power, &gain, 0);
				for (int i = 0; i < fremaelen; i++) {
					//sharedata.ppProcessOut_[channel][i] *= gain;
                    errout[i] *= gain;
				}
			}

			/////////////
			for (int i = 0; i<(fremaelen); i++)
			{
				for (size_t channel = 0; channel < mics_num; channel++)
				{
					//sharedata.ppProcessOut_[channel][i] *= 32767.f;
                    errout[i] *= 32767;
					if (errout[i] > 32767.f)
					{
						data_out_s[i*writewavhead.NChannels + channel] = 32767;
					}
					else if (errout[i] < -32768.f)
					{
						data_out_s[i*writewavhead.NChannels + channel] = -32768;
					}
                    else {
                        //data_out_s[i*writewavhead.NChannels + channel] = short(sharedata.ppProcessOut_[channel][i]);//*32768.f
                        data_out_s[i*writewavhead.NChannels + channel] = errout[i];
                    }
						
				}

				//data_out_s[i*writewavhead.NChannels + mics_num] = (data_in_s[i*writewavhead.NChannels + mics_num]);
				
			}
			writefile->WriteSample(data_out_s, (fremaelen*writewavhead.NChannels));//
		}
	}
	
	writefile->UpdateHeader(writewavhead.NChannels, outfileleng / writewavhead.NChannels);
#ifdef WIN32
	LARGE_INTEGER fqOfCPU;
	QueryPerformanceFrequency(&fqOfCPU);
	printf("Frequency: %u\n", fqOfCPU.QuadPart);
	counttime-=clock();
	double elapseTime2 = (double)elapseTimeCount / fqOfCPU.QuadPart / insideCycleNum*1000;
	printf("elapseTime: %fms\n", elapseTime2);

	//getchar();

#endif
	agc_destroy(pAgc);
	//agc_new_destroy(pAgc_new);
	delete readfile;
	delete writefile;
	
	//delete data_in_f;
	delete data_in_s;
	//delete data_in_f2;
	//delete sharedata.ppCapture_ ;
	//delete sharedata.ppProcessOut_;
    aec_processing_deinit(nullptr);
	return 1;
}
