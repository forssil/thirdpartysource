#ifndef TTMINMAX_H
#define TTMINMAX_H

#ifndef __cplusplus
#ifndef max
   #define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
   #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif

static inline float ttclampf(float val, float lower, float upper) 
{
    if (val < lower)
        return lower;
    else if (val > upper)
        return upper;
    else
        return val;
}
    
static inline int ttclampi(int val, int lower, int upper) 
{
    if (val < lower)
        return lower;
    else if (val > upper)
        return upper;
    else
        return val;
}
    

#endif

