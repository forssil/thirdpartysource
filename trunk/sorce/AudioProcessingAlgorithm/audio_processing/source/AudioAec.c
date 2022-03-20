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
        static FILE   *fp = NULL;
        static FILE   *fp1 = NULL;
        static FILE   *fp2 = NULL;
        static FILE   *fp3 = NULL;
        static FILE   *fp_ref = NULL;
        static FILE   *fp_out = NULL;
        static int	 index = 0;
        static int	 logidx = 0;
        static int    size = 0;
#define	MAX_RECORD_TIMES	20000

        if (logidx++ > 500) {
            logidx = 0;
            printf("%s-%d: =============== aduio size =%d ========== \n", __func__, __LINE__, size);
        }
        if (gs_acallocptr == NULL) {
            printf("%s-%d: malloc fail \n", __func__, __LINE__);
            return;
        }
        //printf("%s-%d: mode =%d %p %p \n",__func__,__LINE__,mode,date_in[3],&date_in[3][0]);   
        //memcpy((void *)data_out, (void *)(&date_in[3][0]), 960 * 2);

        aec_processing_cpp(h_aec, date_in, ref_spk, ref_mic, mode, data_out);

        bool valid = (fp == NULL) && (fp1 == NULL) && (fp2 == NULL) && (fp3 == NULL) && (fp_ref == NULL) && (fp_out == NULL);
        if (valid && (index < MAX_RECORD_TIMES)) {
            fp = fopen("/dev/chn0.pcm", "wb");
            fp1 = fopen("/dev/chn1.pcm", "wb");
            fp2 = fopen("/dev/chn2.pcm", "wb");
            fp3 = fopen("/dev/chn3.pcm", "wb");
            fp_ref = fopen("/dev/chnref.pcm", "wb");
            fp_out = fopen("/dev/chnout.pcm", "wb");
        }
        valid = (fp != NULL) && (fp1 != NULL) && (fp2 != NULL) && (fp3 != NULL) && (fp_ref != NULL) && (fp_out != NULL);
        if (valid && (index < MAX_RECORD_TIMES)) {
            fwrite((char *)(&date_in[3][0]), 960 * 2, 1, fp);
            fwrite((char *)(&date_in[4][0]), 960 * 2, 1, fp1);
            fwrite((char *)(&date_in[5][0]), 960 * 2, 1, fp2);
            fwrite((char *)(&date_in[6][0]), 960 * 2, 1, fp3);
            fwrite((char *)(&ref_spk[0]), 960 * 2, 1, fp_ref);
            fwrite((char *)(&data_out[0]), 960 * 2, 1, fp_out);
            index++;
            size = ftell(fp);
            if (index >= MAX_RECORD_TIMES) {
                fclose(fp);
                fclose(fp1);
                fclose(fp2);
                fclose(fp3);
                fclose(fp_ref);
                fclose(fp_out);
                fp = NULL;
                fp1 = NULL;
                fp2 = NULL;
                fp3 = NULL;
                fp_ref = NULL;
                fp_out = NULL;
                printf("%s-%d: ================== stop save =================== \n", __func__, __LINE__);
            }
        }
        if (access("/oem/aserver/debug", F_OK) == 0) {
            printf("%s-%d: ================== start save =================== \n", __func__, __LINE__);
            if (fp != NULL) {
                fclose(fp);
            }
            fp = NULL;
            if (fp1 != NULL) {
                fclose(fp1);
            }
            fp1 = NULL;
            if (fp2 != NULL) {
                fclose(fp2);
            }
            fp2 = NULL;
            if (fp3 != NULL) {
                fclose(fp3);
            }
            fp3 = NULL;
            if (fp_ref != NULL) {
                fclose(fp_ref);
            }
            fp_ref = NULL;
            if (fp_out != NULL) {
                fclose(fp_out);
            }
            fp_out = NULL;

            index = 0;
            size = 0;
            logidx = 0;
            system("rm /oem/aserver/debug -f");
        }
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
        aec_processing_init_cpp(p_aec);
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