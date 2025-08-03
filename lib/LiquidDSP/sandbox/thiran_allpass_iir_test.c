//
// thiran_allpass_iir_test.c
//
// Test thiran allpass interpolator
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

void compute_thiran_allpass(unsigned int _n,
                            float _mu,
                            float * _b,
                            float * _a);

int main()
{
    unsigned int n = 4;
    float mu = 0.1f;

    float b[n+1];
    float a[n+1];

    // compute filter coefficients
    compute_thiran_allpass(n,mu,b,a);

    // print coefficients to screen
    unsigned int i;
    for (i=0; i<=n; i++)
        printf("a(%3u) = %12.8f;   b(%3u) = %12.8f;\n", i+1, a[i], i+1, b[i]);

    // compute group delay
    float g = iir_group_delay(b,n+1,a,n+1,0.0f);

    printf(" g = %f\n", g);
    
    printf("done.\n");
    return 0;
}

void compute_thiran_allpass(unsigned int _n,
                            float _mu,
                            float * _b,
                            float * _a)
{
    float nf = (float)_n;

    unsigned int k;
    for (k=0; k<=_n; k++) {
        _a[k] = ((k%2)==0 ? 1.0f : -1.0f) * liquid_nchoosek(_n, k);

        // check condition when mu is very small (eliminate divide-by-zero)
        if ( fabsf(_mu) < 1e-6f ) {
            _a[k] = _a[k]*_a[k];
            continue;
        }

        // TODO : expand as sum of logarithms
        unsigned int n;
        for (n=0; n<=_n; n++) {
            _a[k] *= (_mu - nf + (float)n) / (_mu - nf + (float)(n+k));
        }
    }

    for (k=0; k<=_n; k++)
        _b[k] = _a[_n-k];
}

