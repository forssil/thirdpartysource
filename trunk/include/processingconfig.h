/***********************************************************************
 *  Author
;*      Gao Hua
;*      
;*     
;*
;*  History
;*      2/7/2015 Created
;*
;*
;*************************************************************************/


#ifndef _PROCESSING_CONFIG_
#define _PROCESSING_CONFIG_


#define  SUBBANDUSE22k 240
#define  SUBBANDUSE44k 400
#define  FILTER_LEN  10  
#define  minvalue 1e-26f
#define  maxvalue 1e26f
#ifndef PI
#define PI  3.14159265358979F
#endif
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define MAXIT 100
#define EULER 0.5772156649f
#define FPMIN 1.0e-30f
#define EPS 1.0e-7f
#define _fabsf(x) ((x)>=0.f?(x):(-1.f*(x)))

#endif //_PROCESSING_CONFIG_