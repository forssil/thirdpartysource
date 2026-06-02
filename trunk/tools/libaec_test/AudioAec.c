#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include    <stdbool.h>

#include   "AudioAecCpp.h"

#ifdef __cplusplus 
extern "C" {
#endif

    char *gs_acallocptr = NULL;
    int gs_s32LicState = 1;


    unsigned int getAudioAngle()
    {
        return 230;
    }

    void getEncryptedPlaintex(unsigned char* plaintexId)
    {
        if (plaintexId) {
            printf("%s-%d: creat plaintex ok \n", __func__, __LINE__);
            memcpy(plaintexId, "123456789abcdefghijklmnopqrstuvw", 32);
            gs_s32LicState = 2;
        }
    }

    void checkLicense(const unsigned char*dstLic)
    {
        if (dstLic) {
            printf("%s-%d: license ok \n", __func__, __LINE__);
            gs_s32LicState = 4;
        }
    }

    int getLicState()
    {
        return gs_s32LicState;
    }


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
    void aec_processing(void *h_aec, short *date_in[], short *ref_spk, short *ref_mic, int mode, short *data_out)
    {
        if (gs_acallocptr == NULL) {
            printf("%s-%d: malloc fail \n", __func__, __LINE__);
            return;
        }
        //printf("%s-%d: mode =%d %p %p \n",__func__,__LINE__,mode,date_in[3],&date_in[3][0]);   
        //memcpy((void *)data_out, (void *)(&date_in[3][0]), 960 * 2);

        aec_processing_cpp(h_aec, date_in, ref_spk, ref_mic, mode, data_out, 2, 3);
    }

    void aec_processing_init(void  **p_aec)
    {
        if (gs_acallocptr == NULL) {
            printf("%s-%d: ca test libaec.so date:%s time:%s \n", __func__, __LINE__, __DATE__, __TIME__);
            gs_acallocptr = (char *)malloc(24);
            if (gs_acallocptr != NULL) {
                *p_aec = (void *)gs_acallocptr;
                printf("%s-%d: malloc succeed \n", __func__, __LINE__);
            }
            else {
                printf("%s-%d: malloc fail \n", __func__, __LINE__);
            }
        }
        Toggle3A aec_para;
        aec_para.mics_num = 4;
        aec_para.fremaelen = 480;
        aec_para.samplerate = 48000;
        aec_para.bAECOn_ = true;
		aec_para.bNROn_ = false;
		aec_para.bNRCNGOn_ = false;
		aec_para.bAGCOn_ = true;
		aec_para.bRNNOISEOn_ = false;
		aec_para.bPreRnnOn_ = false;
        aec_processing_init_cpp(p_aec, &aec_para);
    }

    void aec_processing_deinit(void *h_aec)
    {
        printf("%s-%d: ca test libaec.so date:%s time:%s \n", __func__, __LINE__, __DATE__, __TIME__);
        aec_processing_deinit_cpp(h_aec);
    }

    unsigned int aec_processing_get_lib_version()
    {
        return 196;
    }

#ifdef __cplusplus 
}
#endif