//
// (Fast) discrete cosine transforms and equivalent FFTs
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"

int main() {
    unsigned int n=16;
    float x[n]; // time-domain 'signal'
    float y[n]; // fft(x)
    float z[n]; // ifft(y)

    unsigned int i;
    for (i=0; i<n; i++)
        x[i] = (float)i;

    // execute dcts
    dct(x,y,n);
    idct(y,z,n);

    // run equivalent ffts
    float complex x0[2*n];
    float complex y0[2*n];
    float complex y1[2*n];
    float complex z0[2*n];

    // create the plan
    int method = 0;
    for (i=0; i<n; i++) {
        x0[i] = x0[2*n-i-1] = x[i];
    }
    fftplan pf = fft_create_plan(2*n, x0, y0, LIQUID_FFT_FORWARD,  method);
    fftplan pr = fft_create_plan(2*n, y1, z0, LIQUID_FFT_BACKWARD, method);

    // execute forward plan and scale appropriately
    fft_execute(pf);
    for (i=0; i<2*n; i++)
        y0[i] *= 0.5f*cexpf(-_Complex_I*M_PI*i/(float)(2*n));

    // scale reverse plan appropriately and execute
    for (i=0; i<2*n; i++)
        y1[i] = y0[i]*cexpf(_Complex_I*M_PI*i/(float)(2*n)) / n;
    fft_execute(pr);
    
    // destroy fft plans
    fft_destroy_plan(pf);
    fft_destroy_plan(pr);

    // normalize inverse
    for (i=0; i<n; i++)
        z[i] *= 2.0f / (float) n;

    // print results
    printf("----------------\n");
    printf("original signal, x[n]:\n");
    for (i=0; i<n; i++)
        printf("  x[%3u] = %8.4f\n", i, x[i]);

    printf("----------------\n");
    printf("  y[n]   = dct(x)[n]   : fft( [x fliplr(x)] ).*(1/2)*exp(-j*pi*[0:n-1]/n)\n");
    for (i=0; i<n; i++)
        printf("  y[%3u] = %8.4f    : %12.8f + j*%12.8f\n", i, y[i], crealf(y0[i]), cimagf(y0[i]));

    printf("----------------\n");
    printf("  z[n]   = idct(x)[n]  : ifft( [y fliplr(y)] ).*(1/n)*exp(j*pi*[0:n-1]/n)\n");
    for (i=0; i<n; i++)
        printf("  z[%3u] = %8.4f    : %12.8f + j*%12.8f\n", i, z[i], crealf(z0[i]), cimagf(z0[i]));

    printf("done.\n");
    return 0;
}

