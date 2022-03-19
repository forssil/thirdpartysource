#ifndef AGC_H_
#define AGC_H_

/* Automatic gain controller */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define AGC_OLD 0

struct AGCSTATE;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Allocates memory for struct.
 * Returns pointer to allocated struct
 */
struct AGCSTATE * agc_create(void);

/* Frees memory for struct.  */
void agc_destroy(struct AGCSTATE * self);

/* Initialises agcState */
void agc_reset(struct AGCSTATE * self);

/* Performs Automatic Gain Control.  */
void agc_process(struct AGCSTATE * self,
                 const uint32_t channels,
                 const float * const absLevel,
                 float * gain,
                 uint32_t trace);

/* Turn agc on or off */
void agc_set_on_off(struct AGCSTATE * self, bool val);

/* Tell if agc is on or off */
bool agc_get_state(struct AGCSTATE * self);

/* Set debug mode on or off */
void agc_set_debug(struct AGCSTATE * self, bool onoff);

/* Set robust mode on or off */
void agc_set_robust_mode(struct AGCSTATE * self, bool onoff);

/* Return string with parameters to be plotted in the MATLAB GUI */
void agc_get_gui_string(struct AGCSTATE * self, char *guiString, const size_t max_string_size);

float agc_get_gain(struct AGCSTATE * self);

#ifdef __cplusplus
}
#endif

#endif
