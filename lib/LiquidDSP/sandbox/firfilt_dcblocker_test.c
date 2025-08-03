// firfilt DC blocker filter design test
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <time.h>

#include "liquid.h"

// initialize DC blocker filter
float firfilt_dcblocker_init(unsigned int _m,
                             float        _fc,
                             float        _as,
                             float *      _h);

int main()
{
    // options
    unsigned int m       = 80;                          // filter semi-length
    float        as      = 40.0f;                       // filter prototype suppression
    const char *filename = "firfilt_dcblocker_test.m";  // output filename

    // derived values
    unsigned int h_len = 2*m+1; // filter length
    float        h[h_len];      // filter coefficients buffer

    // initialize starting conditions
    float p  = 0.1f / fabsf(m * as); // initial spacing
    float g0 = 0, g1 = p, g2 = 2*p;
    float u0 = firfilt_dcblocker_init(m, 0.5f-g0, as, h);
    float u1 = firfilt_dcblocker_init(m, 0.5f-g1, as, h);
    float u2 = firfilt_dcblocker_init(m, 0.5f-g2, as, h);
    // assert u0 > u1 > u2

    // open an initialize output file
    FILE * fid = fopen(filename,"w");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"as= %.3f;\n", as);
    fprintf(fid,"g=[]; u=[];\n");
    fprintf(fid,"g(end+1)=%12.4e; u(end+1)=%12.3f;\n", g0, u0);
    fprintf(fid,"g(end+1)=%12.4e; u(end+1)=%12.3f;\n", g1, u1);
    fprintf(fid,"g(end+1)=%12.4e; u(end+1)=%12.3f;\n", g2, u2);

    // perform initial search while dc value continues to drop
    float g_prime = 3*p;
    while (g_prime < 0.25f) {
        // design filter
        float u_prime = firfilt_dcblocker_init(m, 0.5f-g_prime, as, h);
        fprintf(fid,"g(end+1)=%12.4e; u(end+1)=%12.3f;\n", g_prime, u_prime);
        printf("  gamma : %12.4e, dc : %12.3f dB\n", g_prime, u_prime);

        // shift values
        g0 = g1;      u0 = u1;
        g1 = g2;      u1 = u2;
        g2 = g_prime; u2 = u_prime;

        // break if utility starts to increase
        if (u2 > u1) break;

        // update estimate
        g_prime *= 1.10f;
    }

    // perform bisection:
    // { (g0) .. gm .. (g1) .. gp .. (g2) }
    // { (g0)    ..    (g1)    ..    (g2) }
    unsigned int i;
    for (i=0; i<10; i++) {
        // assert u0 > u1 > u2
        float gm = 0.5f*(g0 + g1);
        float gp = 0.5f*(g1 + g2);
        float um = firfilt_dcblocker_init(m, 0.5f-gm, as, h);
        float up = firfilt_dcblocker_init(m, 0.5f-gp, as, h);
        fprintf(fid,"g(end+1)=%12.4e; u(end+1)=%12.3f;\n", gm, um);
        fprintf(fid,"g(end+1)=%12.4e; u(end+1)=%12.3f;\n", gp, up);

        // save best: should be gm, g1, or gp
        if (um < u1 && um < up) {
            // gm is best
            g2 = g1; u2 = u1;
            g1 = gm; u1 = um;
        } else if (u1 < um && u1 < up) {
            // g1 is best
            g0 = gm; u0 = um;
            g2 = gp; u2 = up;
        } else {
            // gp is best
            g0 = g1; u0 = u1;
            g1 = gp; u1 = up;
        }
        printf("  gamma : %12.4e, dc : %12.3f dB\n", g1, u1);
    }

    // re-design optimum filter and copy coefficients to type-specific array
    firfilt_dcblocker_init(m, 0.5-g1, as, h);

    // save filter coefficients
    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%3u) = %12.4e;\n", i+1, h[i]);

    //
    fprintf(fid,"figure('position',[1 1 600 800]);\n");
    //fprintf(fid,"h = h / sum(h.^2);\n");
    fprintf(fid,"nfft = 1200; f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"subplot(3,1,1)\n");
    fprintf(fid,"  plot(g,u,'x');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Cutoff Frequency Backoff');\n");
    fprintf(fid,"  ylabel('Utility, DC Rejection (dB)');\n");
    fprintf(fid,"subplot(3,1,2)\n");
    fprintf(fid,"  plot([-m:m],h,'-x');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Sample Index');\n");
    fprintf(fid,"  ylabel('Filter Response');\n");
    fprintf(fid,"subplot(3,1,3)\n");
    fprintf(fid,"  plot(f,20*log10(abs(fftshift(fft(h,nfft)))));\n");
    fprintf(fid,"  axis([-0.5 0.5 -80 10]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('PSD (dB)');\n");
    fclose(fid);
    printf("results written to %s\n", filename);
    return 0;
}

// initialize DC blocker filter
float firfilt_dcblocker_init(unsigned int _m,
                             float        _fc,
                             float        _as,
                             float *      _h)
{
    // design filter
    unsigned int h_len = 2*_m+1;
    liquid_firdes_kaiser(h_len, _fc, _as, 0.0f, _h);

    // mix by Fs/2 and evaluate DC term
    float dc = 0.0f;
    unsigned int i;
    for (i=0; i<h_len; i++) {
        _h[i] *= ((i + _m) % 2) == 0 ? 1.0f : -1.0f;
        dc += _h[i];
    }
    return 20*log10(fabsf(dc));
}

