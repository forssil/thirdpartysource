//
//  Amp_compute.h
//  audio_common
//
//  Created by Robin on 4/15/16.
//  Copyright © 2016 1. All rights reserved.
//

#ifndef AMP_COMPUTE_H
#define AMP_COMPUTE_H

#ifdef __cplusplus
extern "C" {
#endif
void Clipping( float * sample_value);
// Get sum of sample's absolute value.
float GetPowerLevel(float *x, int len);
// Get sum of sample's energe.
float GetPowerEnergy(float *x, int len);
// Get float data dB.
float GetPowerdB(float *x, int len, float zero_db_value);
//Get short data dB
float GetPowerdBShort(short *x, int len, int zero_db_value);
//convert db to WebEx level
int ChangeDbToWebExLevel(float db);
//convert short to float, numSamples will be protected by caller
void ShortToFloat(short* src, float* dst, int numSamples);
//convert float to short
void FloatToShort(float* src, short* dst, int numSamples);
#ifdef __cplusplus
}
#endif
#endif
