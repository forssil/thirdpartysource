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
        aec_processing_init_cpp(p_aec, NULL);
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