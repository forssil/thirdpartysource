#pragma once
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
	float *data_in_f, *data_out_f;
	float *data_in_f2, *data_out_f2, *data_out_f3;
	audio_pro_share sharedata;
	CAudioProcessingFrameworkInterface* pAPFInterface;
	int mics_num = 4;
	int fremaelen = 480;
}
