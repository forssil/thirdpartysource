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

/*
 * AEC processing function
 * h_aec: AEC handle pointer
 * date_in: Input audio data pointer
 * date_in : [(channel+index_tmp)*mic nubmer, cycle*480]
 * mic数据读取规则 date_in[index_tmp + channel][i + 480 * cycle])
 * ref_spk: 扬声器回采数据，没有设置为NULL
 * ref_mic: 参考麦克风数据，可用于远场噪声抑制，没有可以设置为NULL
 * mode: AEC mode，预留接口
 * data_out: Output audio data pointer， 1声道输出
 * cycle_num: Number of cycles， 输入数据为480的倍数
 * index_tmp: Index of the current cycle，每个mic数据所在行数，debug信息放在对应通道之前，没有置0
 *
 */
extern void aec_processing_cpp(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out, int cycle_num, int index_tmp);
/*
 * AEC init function
 * p_aec: AEC handle pointer
 * config: Toggle3A
 */
extern void aec_processing_init_cpp(void  **p_aec, void* config);
extern void aec_processing_deinit_cpp(void *h_aec);
//unsigned int aec_processing_get_lib_version();

//struct TagCAudioProcessingFrameworkInterface;
//extern struct TagCAudioProcessingFrameworkInterface *GetAudioProcessingFrameworkInterface(void);
struct AEC_parameter {
    float *data_in_f, *data_out_f;
    float *data_in_f2, *data_out_f2, *data_out_f3, *data_out_f4, *data_out_f5, *data_out_f6, *data_out_f7;
    void* sharedata;
    int mics_num;
    int fremaelen;
    int samplerate;
    void* pSUBThread;
};
struct Toggle3A{
    bool bAECOn_;
    bool bNROn_;
    bool bNRCNGOn_;
    bool bAGCOn_;
    bool bRNNOISEOn_;
    bool bPreRnnOn_;
    int mics_num;   // number of microphones
    int fremaelen;  //480  not used currently
    int samplerate; //48000 not used currently
};

#ifdef __cplusplus
};
#endif

#endif