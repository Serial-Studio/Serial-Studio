// test group delay for windowed filter design
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME "firpfb_rrrf_example.m"

void apply_delay(float *      _h,
                 float *      _h_prime,
                 unsigned int _h_len,
                 float        _dt)
{
    unsigned int L = _h_len % 2;
    unsigned int m = (_h_len - L)/2;
    float complex buf_time[_h_len];
    float complex buf_freq[_h_len];
    unsigned int i;
    // copy to buffer, applying appropriate shift
    for (i=0; i<_h_len; i++)
        buf_time[i] = _h[(i + m)%_h_len];
    fft_run(_h_len, buf_time, buf_freq, LIQUID_FFT_FORWARD, 0);

    // apply phase shift
    for (i=0; i<_h_len; i++) {
        float v = (float)i - (float)m;
        buf_freq[(i+m+L)%_h_len] *= cexpf(-_Complex_I*_dt*2.*M_PI*v/(float)_h_len);;
    }

#if 0
    for (i=0; i<_h_len; i++)
        printf("H(%3u) = %12.8f + %12.8fi;\n", i+1, crealf(buf_freq[i]), cimagf(buf_freq[i]));
    printf("\n");
#endif
    fft_run(_h_len, buf_freq, buf_time, LIQUID_FFT_BACKWARD, 0);

#if 0
    for (i=0; i<_h_len; i++)
        printf("g(%3u) = %12.8f + %12.8fi;\n", i+1, crealf(buf_time[i]), cimagf(buf_time[i]));
    printf("\n");
#endif
    // copy result, apply shift, retain real component
    for (i=0; i<_h_len; i++) {
        _h_prime[(i+m)%_h_len] = crealf( buf_time[i] );
    }
#if 0
    for (i=0; i<_h_len; i++)
        printf("h(%3u) = %12.8f;\n", i+1, _h_prime[i]);
#endif
}

int main(int argc, char*argv[]) {
    // options
    unsigned int m  =    12; // filter delay (samples)
    float        fc = 0.25f; // filter cut-off frequency

    // derived values and buffers
    unsigned int h_len = 2*m + 1;
    float        h[h_len];  // filter impulse response
    float        g[h_len];  // transformed
    unsigned int i;
    for (i=0; i<h_len; i++) {
        float t = (float)i - (float)m;
        h[i] = sincf(fc*t) * liquid_hamming(i,h_len);
    }

    float dt = -2.0f;
    printf("# fractional sample offset: specified vs. actual\n");
    printf("#\n");
    printf("# %12s %12s %12s\n", "specified", "actual", "error");
    while (dt <= 2.0f) {
        // apply delay
        apply_delay(h, g, h_len, dt);

        // evaluate group delay at DC and compensate for nominal delay
        float dt_actual = fir_group_delay(g,h_len,0.0f) - (float)m;
        printf("  %12.9f %12.9f %12.9f\n", dt, dt_actual, dt-dt_actual);

        // 
        dt += 0.125f;
    }
    return 0;
}
