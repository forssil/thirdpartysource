//
//  Amp_compute.c
//  audio_common
//
//  Created by Robin on 4/15/16.
//  Copyright © 2016 1. All rights reserved.
//
#include <math.h>
#include "Amp_compute.h"


#define LOW_LIMIT  0.75f
#define HIGH_LIMIT (2 - LOW_LIMIT)
#define SIGN(x) (signbit(x)?-1.f:1.f)

void Clipping( float * sample_value)
{
    float abs_value = fabs( * sample_value);
    if (LOW_LIMIT >= abs_value) {
        return;
    }
    else if (HIGH_LIMIT >= abs_value) {
        float temp1 = abs_value - LOW_LIMIT;
        
        *sample_value =SIGN(*sample_value)* (LOW_LIMIT + temp1/(1+temp1*temp1/(HIGH_LIMIT-LOW_LIMIT)/(HIGH_LIMIT-LOW_LIMIT)));
    }
    else {
        *sample_value /= abs_value;
    }
    
}
// Get sum of sample's absolute value.
float GetPowerLevel(float *x, int len) {
    int   i;
    float sum = 0.0;
    
    for (i = 0; i < len; ++i)
        sum += fabs(x[i]);
    
    if(len>0)
        sum /=(float) len;
    return sum;
}

// Get sum of sample's energe.
float GetPowerEnergy(float *x, int len) {
    int   i;
    float sum = 0.0;

    if(len ==0)
        return 0;
    for (i = 0; i < len; ++i)
        sum += x[i] * x[i];

    if(len>0)
        sum /=(float) len;
    return sum;
}

// Get float data dB.
float GetPowerdB(float *x, int len, float zero_db_value) {
    double Energy_6464 = 0;
    double min_zero = 0;
    int i = 0;
    if (0 == len || 1e-10f > zero_db_value) {
        return  -127;
    }
    min_zero = (double)(zero_db_value/100000);
    for (i=0; i<len; i++)
        Energy_6464 += (double)x[i] * (double)x[i];
    if(len>0)
       Energy_6464 /= len;
    
    if (Energy_6464 <= min_zero)
        return -127;
    else
        Energy_6464 = 10*log10(Energy_6464/((double)zero_db_value*(double)zero_db_value));
    
    return (float)Energy_6464;
}

// Get short data dB.
float GetPowerdBShort(short *x, int len, int zero_db_value) {
    long long Energy_6464 = 0;
    long long min_zero = 0;
    float res = -127.f;
    int i = 0;
    float temp = 0.f;
    if (0 == len || 0 >= zero_db_value) {
        return  -127;
    }
    temp = (float)len;
    temp *= zero_db_value;
    temp *= zero_db_value;
    //min_zero = (double)(zero_db_value/100000);
    for (i=0; i<len; i++)
        Energy_6464 += (long long)(x[i] * x[i]);
    if (len > 0)
        res = (float)(Energy_6464) / temp;
    if (Energy_6464 <= min_zero)
        return -127;
    else
        res = 10*log10(res);
    return res;
}

int ChangeDbToWebExLevel(float db)
{
    if (db>-0.000001f) {
        db  = 0.;
    }
    int level = 0;
    float volume = -1 * db;
    
    if (volume > 60)
    {
        level = 0;
    }else if(volume > 45)
    {
        level = (int)(10  - (volume - 45) / 3 * 2);
    } else if(volume > 40)
    {
        level = (int)(20 - (volume - 40) * 2);
    } else if(volume > 35)
    {
        level = (int)(30 - (volume - 35) * 2);
    } else if(volume > 30)
    {
        level = (int)(40- (volume - 30) * 2);
    } else if(volume > 25)
    {
        level = (int)(50- (volume - 25) * 2);
    } else if(volume > 20)
    {
        level = (int)(60- (volume - 20) * 2);
    } else if(volume > 15)
    {
        level = (int)(70- (volume - 15) * 2);
    }
    else if(volume > 10)
    {
        level = (int)(80- (volume - 10) * 2);
    }
    else if(volume > 5)
    {
        level = (int)(90- (volume - 5) * 2);
    }
    else
    {
        level = (int)(100- volume * 2);
    }
    
    return level;
}

void ShortToFloat(short* src, float* dst, int numSamples)
{
    int i = 0;
    for( i = 0; i < numSamples; i++)
    {
        if (*src > 32767)
        {
            *src = 32767;
        }
        else if (*src < -32768)
        {
            *src = -32768;
        }
        *dst = (*src) / 32768.0f;
        dst++;
        src++;
    }
}

void FloatToShort(float* src, short*dst, int numSamples)
{
    int i = 0;
    for ( i = 0; i < numSamples; i++)
    {
        if (*src > 1)
        {
            *src = 1;
        }
        else if (*src < -1)
        {
            *src = -1;
        }
        *dst = (*src) * 32767.0;
        dst++;
        src++;
        
    }
}

