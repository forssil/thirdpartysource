#include <math.h>
#include "basemath.h"
#include "processingconfig.h"
extern "C"
{
float sinc_fun(float x)
{
	float y;
	x=_fabsf(x);
	if (x>0.01)
	{
		y=sin(x)/x;
	}
	else
		y=cos(x);
	return y;
}

float expint(float x)
{

	int i;
	float a,b,c,d,del,fact,h,ans;


	if (x>10)
	{
		ans=0.f;
	}
	else
	{
		if (x <= 1e-10) ans=maxvalue;
		else {
			if (x > 1.0) {
				b=x+1.f;
				c=1.0f/FPMIN;
				d=1.0f/b;
				h=d;
				for (i=1;i<=MAXIT;i++) {
					a =float(-i*(i));
					b += 2.0f;
					d=1.0f/(a*d+b);
					c=b+a/c;
					del=c*d;
					h *= del;
					if (_fabsf(del-1.0f) < EPS) {
						ans=h*exp(-x);
						return ans;
					}
				}
				ans=h*exp(-x);

			} else {
				ans = (-log(x)-EULER);
				fact=1.0f;
				for (i=1;i<=MAXIT;i++) {
					fact *= -x/i;
					del = -fact/(i);					
					ans += del;
					if (_fabsf(del) < _fabsf(ans)*EPS) return ans;
				}

			}
		}	
	}
	return ans;
}
}