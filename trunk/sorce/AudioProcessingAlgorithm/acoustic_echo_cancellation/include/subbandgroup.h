#ifndef _H_subbandgroup_
#define _H_subbandgroup_

#define DECAY_NUMBER_OF_USED_ATTACK_TAPS_DEFAULT 7

float DECAY_ATTACK_TAPS_DEFAULT[]= {0.2f, 0.5f, 3.0f, 3.0f, 3.0f, 2.8125f, 2.6367f, 2.4719f, 2.3174f, 2.1725f}; /* default value must be  NUMBER_OF_ATTACK_TAPS long */
#define DECAY_LARGEST_TAP_AT_MAX 8 /* largest value of echo filter (total power of subbands) occur at maximum this 10ms packet */
#define DECAY_LARGEST_TAP_AT_MIN 3 /* largest value of echo filter (total power of subbands) occur at minimum this 10ms packet */
#define DECAY_PRE_ROOM_DOMINACE_TIME 2 /* number of 10ms packets from largest value of echo filter occur until the room dominates the echo filter */
#define NUMBER_OF_ATTACK_TAPS (DECAY_LARGEST_TAP_AT_MAX + DECAY_PRE_ROOM_DOMINACE_TIME)/* maximum number of individual tapweights */
/* impulse response decay estiamtion */
//#define SUBUSED_FINDDECAY48 159 /* The finddecay routine requires at least 20 filtertaps */ //48kHz 


#endif