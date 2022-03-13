#ifndef _INC_AUDIOAEC
#define _INC_AUDIOAEC

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    "AudioProcessingFramework_interface.h"

unsigned int getAudioAngle();
void getEncryptedPlaintex(unsigned char* plaintexId);
void checkLicense(const unsigned char*dstLic);
int getLicState();
void aec_processing(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out);
void aec_processing_init(void  **p_aec);
void aec_processing_deinit(void *h_aec);
unsigned int aec_processing_get_lib_version();
namespace {
    float *data_in_f1, *data_out_f1;
    float *data_in_f21, *data_out_f21, *data_out_f31;
    audio_pro_share sharedata1;
    CAudioProcessingFrameworkInterface* pAPFInterface1;
    int mics_num1 = 4;
    int fremaelen1 = 480;
}



#endif