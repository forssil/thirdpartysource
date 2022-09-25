#ifndef _INC_AUDIOAECCPP
#define _INC_AUDIOAECCPP

// #include	<stdio.h>
// #include	<stdlib.h>
// #include	<string.h>

//#include    "agc.h"

#ifdef __cplusplus
extern "C" {
#endif
//unsigned int getAudioAngle();
//void getEncryptedPlaintex(unsigned char* plaintexId);
//void checkLicense(const unsigned char*dstLic);
//int getLicState();
extern void aec_processing_cpp(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out, int cycle_num, int index_tmp);
extern void aec_processing_init_cpp(void  **p_aec, void* config);
extern void aec_processing_deinit_cpp(void *h_aec);
//unsigned int aec_processing_get_lib_version();

//struct TagCAudioProcessingFrameworkInterface;
//extern struct TagCAudioProcessingFrameworkInterface *GetAudioProcessingFrameworkInterface(void);
struct AEC_parameter {
    float *data_in_f, *data_out_f;
    float *data_in_f2, *data_out_f2, *data_out_f3, *data_out_f4, *data_out_f5, *data_out_f6, *data_out_f7;
    void* sharedata;
    void* pAPFInterface;
    //CAudioProcessingFrameworkInterface* pAPFInterface = CreateIApfInst_int(4, 48000, 2 * 512, 512);
    int mics_num;
    int fremaelen;
    //short **buffer = new short*[5];
    //struct AGCSTATE* pAgc;
    //struct AGCSTATE_NEW* pAgc_new;
    void * pAgc_new;
    void* pRnnoise;
    void* pSUBThread;
};
struct Toggle3A{
    bool bAECOn_;
    bool bNROn_;
    bool bNRCNGOn_;
    bool bAGCOn_;
    bool bRNNOISEOn_;
    bool bPreRnnOn_;
};

#ifdef __cplusplus
};
#endif

#endif