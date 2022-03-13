#pragma once
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include "AudioProcessingFramework_interface.h"

float *data_in_f, *data_out_f;
float *data_in_f2, *data_out_f2, *data_out_f3;
audio_pro_share sharedata;
CAudioProcessingFrameworkInterface* pAPFInterface;
int mics_num = 4;
int fremaelen = 480;