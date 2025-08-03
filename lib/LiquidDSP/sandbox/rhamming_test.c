// Design (approximate) square-root Nyquist Hamming filter
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

#define OUTPUT_FILENAME "rhamming_test.m"

float bisect_search(int _wtype, unsigned int _k, unsigned int _m, float _arg, float * _h, int _verbose);
float bisect_eval  (int _wtype, unsigned int _k, unsigned int _m, float _arg, float * _h, float _gamma);

int main() {
    // options
    unsigned int k     =  2;
    unsigned int m     = 12;
    int          wtype = LIQUID_WINDOW_HAMMING;
    float        arg   = 0;
    int          verbose = 1;

    // derived values
    unsigned int h_len = 2*k*m+1;
    float h[h_len];

    float gamma_opt = bisect_search(wtype, k, m, arg, h, verbose);
    printf(" k:%2u, m:%2u, gamma:%12.8f\n", k, m, gamma_opt);

    // re-design with optimum and save to file
    //printf("optimum RMS ISI of %.2f dB with gamma=%.6f\n", 20*log10(isi_opt), gamma_opt);
    liquid_firdes_windowf(wtype,h_len,gamma_opt*0.5f/(float)k,arg,h);
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"k = %u; m = %u; h = zeros(2*k*m+1,1);\n", k, m);
    fprintf(fid,"h = [");
    unsigned int i;
    for (i=0; i<h_len; i++)
        fprintf(fid,"%g,", h[i]);
    fprintf(fid,"];\n");
    fprintf(fid,"g = conv(h,h);\n");
    fprintf(fid,"th = [(-k*m):(k*m)]/k;\n");
    fprintf(fid,"tg = [(-2*k*m):(2*k*m)]/k;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1), plot(th,h,'.-'); grid on; ylabel('h');\n");
    fprintf(fid,"subplot(2,1,2), plot(tg,g,'.-'); grid on; ylabel('h * h');\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);
    return 0;
}

float bisect_search(int _wtype, unsigned int _k, unsigned int _m,
                    float _arg, float * _h, int _verbose)
{
    // design filter
    float g0 = 1.00f; float u0 = bisect_eval(_wtype,_k,_m,_arg,_h,g0);
    float g1 = 1.05f; float u1 = bisect_eval(_wtype,_k,_m,_arg,_h,g1);
    float g2 = 1.50f; float u2 = bisect_eval(_wtype,_k,_m,_arg,_h,g2);

    // assume u1 < {u0,u2}
    // { g0, ga, g1, gb, g2 }
    unsigned int num_steps = 16;
    unsigned int i = 0;
    for (i=0; i<num_steps; i++) {
        float ga = sqrtf(g0*g1); float ua = bisect_eval(_wtype,_k,_m,_arg,_h,ga);
        float gb = sqrtf(g1*g2); float ub = bisect_eval(_wtype,_k,_m,_arg,_h,gb);
        if (_verbose) {
            printf("g:{%8.6f, %8.6f, %8.6f, %8.6f, %8.6f}, isi:{%6.3f,%6.3f,%6.3f,%6.3f,%6.3f}\n",
                    g0, ga, g1, gb, g2, u0, ua, u1, ub, u2);
        }

        // find minimum of {ua, u1, ub}
        if (ua < u1 && ua < ub) {
            g2 = g1; u2 = u1;
            g1 = ga; u1 = ua;
        } else if (u1 < ua && u1 < ub) {
            g0 = ga; u0 = ua;
            g2 = gb; u2 = ub;
        } else {
            g0 = g1; u0 = u1;
            g1 = gb; u1 = ub;
        }
    }
    return g1;
}

float bisect_eval(int _wtype, unsigned int _k, unsigned int _m,
                  float _arg, float * _h, float _gamma)
{
    // design filter
    liquid_firdes_windowf(_wtype,2*_k*_m+1,_gamma*0.5f/(float)_k,_arg,_h);

    // compute filter ISI
    float isi_rms, isi_max;
    liquid_filter_isi(_h,_k,_m,&isi_rms,&isi_max);
    return 20*log10(isi_rms);
}

