// 
// vectorcf_test.c : test complex floating-point vector operations
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <complex.h>

#include "liquid.h"

int main()
{
    // options
    unsigned int n = 64;
    
    //
    unsigned int i;
    float complex v[n];
    for (i=0; i<n; i++)
        v[i] = cexpf(_Complex_I*(float)i);

    // test l2-norm operation
    float norm = 0.0f;
    for (i=0; i<n; i++)
        norm += crealf( v[i]*conjf(v[i]) );
    norm = sqrtf(norm);
    float norm_test = liquid_vectorcf_norm(v, n);

    printf("norm : %12.8f (expected %12.8f)\n", norm_test, norm);

    printf("done.\n");
    return 0;
}

