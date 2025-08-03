//
// elliptest.c
//
// gcc -I. -I./include -lm libliquid.a elliptest.c -o elliptest
//

#include <stdio.h>
#include <stdlib.h>

#include "include/liquid.h"
#include "src/filter/src/ellip.c"

int main() {
    // filter specifications
    float fp = 4.0f;
    float fs = 4.5f;
    float Gp = 0.95f;
    float Gs = 0.05f;

    unsigned int n=7;   // number of iterations

    float Wp = 2*M_PI*fp;
    float Ws = 2*M_PI*fs;

    // ripples passband, stopband
    float ep = sqrtf(1.0f/(Gp*Gp) - 1.0f);
    float es = sqrtf(1.0f/(Gs*Gs) - 1.0f);
    printf("ep, es      : %12.8f, %12.8f\n", ep, es);

    float k  = Wp/Ws;           // 0.8889f;
    float k1 = ep/es;           // 0.0165f;

    float K,  Kp;
    float K1, K1p;
    ellipkf(k, n, &K,  &Kp);    // K  = 2.23533416, Kp  = 1.66463780
    ellipkf(k1,n, &K1, &K1p);   // K1 = 1.57090271, K1p = 5.49361753
    printf("K,  Kp      : %12.8f, %12.8f\n", K,  Kp);
    printf("K1, K1p     : %12.8f, %12.8f\n", K1, K1p);

    float Nexact = (K1p/K1)/(Kp/K); // 4.69604063
    float N = ceilf(Nexact);        // 5
    printf("N (exact)   : %12.8f\n", Nexact);
    printf("N           : %12.8f\n", N);

    k = ellipdegf(N,k1,n);      // 0.91427171
    printf("k           : %12.8f\n", k);

    float fs_new = fp/k;        // 4.37506723
    printf("fs_new      : %12.8f\n", fs_new);

    unsigned int L = (unsigned int)(floorf(N/2.0f)); // 2
    //unsigned int r = ((unsigned int)N) % 2;
    float u[L];
    unsigned int i;
    for (i=0; i<L; i++) {
        float t = (float)i + 1.0f;
        u[i] = (2.0f*t - 1.0f)/N;
        printf("u[%3u]      : %12.8f\n", i, u[i]);
    }
    float complex zeta[L];
    for (i=0; i<L; i++) {
        zeta[i] = ellip_cdf(u[i],k,n);
        printf("zeta[%3u]   : %12.8f + j*%12.8f\n", i, crealf(zeta[i]), cimagf(zeta[i]));
    }

    // compute filter zeros
    float complex za[L];
    for (i=0; i<L; i++) {
        za[i] = _Complex_I * Wp / (k*zeta[i]);
        printf("za[%3u]     : %12.8f + j*%12.8f\n", i, crealf(za[i]), cimagf(za[i]));
    }

    float complex v0 = -_Complex_I*ellip_asnf(_Complex_I/ep, k1, n)/N;
    printf("v0          : %12.8f + j*%12.8f\n", crealf(v0), cimagf(v0));

    float complex pa[L];
    for (i=0; i<L; i++) {
        pa[i] = Wp*_Complex_I*ellip_cdf(u[i]-_Complex_I*v0, k, n);
        printf("pa[%3u]     : %12.8f + j*%12.8f\n", i, crealf(pa[i]), cimagf(pa[i]));
    }
    float complex pa0 = Wp * _Complex_I*ellip_snf(_Complex_I*v0, k, n);
    printf("pa0         : %12.8f + j*%12.8f\n", crealf(pa0), cimagf(pa0));


    printf("done.\n");
    return 0;
}

