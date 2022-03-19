#ifndef AGC_NEW_H_
#define AGC_NEW_H_

/*
 * Automatic gain controller
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define AGC_NEW 1

#define AGC_NEW_GUI_STRING_SIZE 321

struct AGCSTATE_NEW;
#ifdef __cplusplus
extern "C" {
#endif
struct AGCSTATE_NEW * agc_new_create(void);
bool agc_new_check_on_off(struct AGCSTATE_NEW * agc);
void agc_new_destroy(struct AGCSTATE_NEW * agc);
void agc_new_reset(struct AGCSTATE_NEW * agc);
void agc_new_process(struct AGCSTATE_NEW * agc,
                     const uint32_t channels,
                     const float * const absLevel,
                     float * gain,
                     uint32_t trace);
void agc_new_set_on_off(struct AGCSTATE_NEW * agc, bool val);
void agc_new_set_NFE_on_off(struct AGCSTATE_NEW * agc, bool val);
void agc_new_set_param(struct AGCSTATE_NEW * agc,
                       float attack,
                       float release,
                       int hold_ms,
                       float ratio_comp,
                       float ratio_exp,
                       float attack_gain,
                       float release_gain);
void agc_new_set_gain_char(struct AGCSTATE_NEW *agc,
                           int th_comp,
                           int th_exp,
                           float makeup);
void agc_new_set_nfe_param(struct AGCSTATE_NEW *agc,
                           float attack_nfe,
                           float release_nfe,
                           int hold_nfe_ms,
                           int target_lvl,
                           int th_comp_lvl,
                           int th_exp_lvl);
void agc_new_print_all_param(struct AGCSTATE_NEW *agc);
void agc_new_set_gui_on_off(struct AGCSTATE_NEW *agc, bool val);
bool agc_new_check_gui_on_off(struct AGCSTATE_NEW *agc);
int agc_new_get_file_size(struct AGCSTATE_NEW *agc);
void agc_new_get_gui_string(struct AGCSTATE_NEW *agc, char *guiString, const size_t max_string_size);
float agc_new_get_gain(struct AGCSTATE_NEW *agc);
#ifdef __cplusplus
}
#endif

#endif
