#ifndef _INC_AUDIOAECCPP
#define _INC_AUDIOAECCPP

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    "audiotypedef.h"
#include    "AudioProcessingFramework_interface.h"
#ifdef __cplusplus
extern "C" {
#endif
//unsigned int getAudioAngle();
//void getEncryptedPlaintex(unsigned char* plaintexId);
//void checkLicense(const unsigned char*dstLic);
//int getLicState();
extern void aec_processing_cpp(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out);
extern void aec_processing_init_cpp(void  **p_aec);
extern void aec_processing_deinit_cpp(void *h_aec);
//unsigned int aec_processing_get_lib_version();

//struct TagCAudioProcessingFrameworkInterface;
//extern struct TagCAudioProcessingFrameworkInterface *GetAudioProcessingFrameworkInterface(void);
struct AEC_parameter {
    float *data_in_f, *data_out_f;
    float *data_in_f2, *data_out_f2, *data_out_f3;
    audio_pro_share sharedata;
    CAudioProcessingFrameworkInterface* pAPFInterface;
    //CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(4, 48000, 2 * 512, 512);
    int mics_num;
    int fremaelen;
    //short **buffer = new short*[5];
};

static AEC_parameter aec_para;

#ifdef __cplusplus
};
#endif

#endif