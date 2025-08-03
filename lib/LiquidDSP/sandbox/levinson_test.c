//
// levinson_test.c
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"


int main() {
    unsigned int p=10;

    // derived values
    float r[p+1];
    float a_matrixinv[p+1];
    float a_levinson[p+1];

    unsigned int i;
    unsigned int j;

    for (i=0; i<p+1; i++) {
        r[i] = randnf()*expf(-0.6f*i) + 2.00f*randnf();

        printf("r[%2u] = %12.8f\n", i, r[i]);
    }

    // 
    // matrix inversion
    //

    // use low inversion method
    float R[p*p];
    for (i=0; i<p; i++) {
        for (j=0; j<p; j++)
            matrix_access(R,p,p,i,j) = r[(i>j) ? (i-j) : (j-i)]; // abs(i-j)
    }

    // invert matrix (using Gauss-Jordan elimination)
    matrixf_inv(R,p,p);

    float rt[p];
    float a_hat[p];

    for (i=0; i<p; i++)
        rt[i] = -r[i+1];

    // multiply R_inv with r to get _a vector
    matrixf_mul(R,     p, p,
                rt,    p, 1,
                a_hat, p, 1);

    a_matrixinv[0] = 1.0f;
    memmove(&a_matrixinv[1], a_hat, p*sizeof(float));

    // 
    // Levinson-Durbin algorithm
    //

    float a0[p+1];  // ...
    float a1[p+1];  // ...
    float e[p+1];     // prediction error
    float k[p+1];     // reflection coefficients

    // initialize
    k[0] = 1.0f;
    e[0] = r[0];

    for (i=0; i<p+1; i++) {
        a0[i] = (i==0) ? 1.0f : 0.0f;
        a1[i] = (i==0) ? 1.0f : 0.0f;
    }

    unsigned int n;
    for (n=1; n<p+1; n++) {

        float q = 0.0f;
        for (i=0; i<n; i++)
            q += a0[i]*r[n-i];

        k[n] = -q/e[n-1];
        e[n] = e[n-1]*(1.0f - k[n]*k[n]);

        // compute new coefficients
        for (i=0; i<n; i++)
            a1[i] = a0[i] + k[n]*a0[n-i];

        a1[n] = k[n];
#if 0
        printf("iteration [n=%u]\n", n);
        for (i=0; i<n; i++)
            printf("  ** i = %3u, n-i = %3u\n", i, n-i);

        // print results
        printf("  k   = %12.8f\n", k[n]);
        printf("  q   = %12.8f\n", q);
        printf("  e   = %12.8f\n", e[n]);

        printf("  a_%u = {", n-1);
        for (i=0; i<n; i++)
            printf("%6.3f, ", a0[i]);
        printf("}\n");

        printf("  a_%u = {", n);
        for (i=0; i<n+1; i++)
            printf("%6.3f, ", a1[i]);
        printf("}\n");
#endif

        // copy temporary vector a1 to a0
        memmove(a0, a1, (p+1)*sizeof(float));

    }

#if 0
    // copy result
    for (i=0; i<p+1; i++)
        a_levinson[i] = a1[i];
#else
    memmove(a_levinson, a1, (p+1)*sizeof(float));
#endif

#if 0
    //
    // test using liquid_levinson
    //
    liquid_levinson(r,p,a_levinson,k);
#endif

    // 
    // print results
    //

    for (i=0; i<p+1; i++)
        printf("%3u : %12.8f (%12.8f)\n", i, a_matrixinv[i], a_levinson[i]);

    return 0;
}

