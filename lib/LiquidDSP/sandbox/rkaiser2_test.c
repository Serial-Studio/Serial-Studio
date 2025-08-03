/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// Design root-Nyquist Kaiser filter
//
// References
//  [Vaidyanathan:1993] Vaidyanathan, P. P., "Multirate Systems and
//      Filter Banks," 1993, Prentice Hall, Section 3.2.1
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

#define DEBUG 0
#define OUTPUT_FILENAME "rkaiser2_test.m"

struct gsuserdata_s {
    unsigned int k; // samples/symbol
    unsigned int m; // filter delay (symbols)
    float beta;     // excess bandwidth factor
    float dt;       // fractional delay

    float w0;       // weighting for ISI
    float w1;       // weighting for stop-band attenuation
};

// Design frequency-shifted root-Nyquist filter based on
// the Kaiser-windowed sinc.
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter [size: 2*_k*_m+1]
void liquid_firdes_rkaiser_filter2(unsigned int _k,
                                   unsigned int _m,
                                   float        _beta,
                                   float        _dt,
                                   float *      _h);

// gradient search utility
float gs_utility(void * _userdata,
                 float * _h,
                 unsigned int _n);

int main() {
    // options
    unsigned int k=2;
    unsigned int m=5;
    float beta = 0.3;
    float dt = 0.0;

    // derived values
    unsigned int h_len = 2*k*m+1;
    float h1[h_len];
    float h2[h_len];

    liquid_firdes_rkaiser(k,m,beta,dt,h1);
    liquid_firdes_rkaiser_filter2(k,m,beta,dt,h2);

    // print filters

    // export results
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"\n");
    fprintf(fid,"k  = %u;\n", k);
    fprintf(fid,"m  = %u;\n", m);
    fprintf(fid,"beta = %12.8f;\n", beta);
    fprintf(fid,"n = 2*k*m+1;\n");
    fprintf(fid,"h1 = zeros(1,n);\n");
    fprintf(fid,"h2 = zeros(1,n);\n");

    unsigned int i;
    for (i=0; i<h_len; i++) {
        fprintf(fid,"  h1(%3u) = %12.4e;\n", i+1, h1[i]);
        fprintf(fid,"  h2(%3u) = %12.4e;\n", i+1, h2[i]);
    }
    fprintf(fid,"\n");
    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H1 = 20*log10(abs(fftshift(fft(h1/k,nfft))));\n");
    fprintf(fid,"H2 = 20*log10(abs(fftshift(fft(h2/k,nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,H1, f,H2);\n");
    fprintf(fid,"axis([-0.5 0.5 -100 20]);\n");
    fprintf(fid,"grid on\n");
    fprintf(fid,"legend('r-Kaiser','r-Kaiser(2)',1);\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

// liquid_firdes_rkaiser()
//
// Design frequency-shifted root-Nyquist filter based on
// the Kaiser-windowed sinc.
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter [size: 2*_k*_m+1]
void liquid_firdes_rkaiser_filter2(unsigned int _k,
                                   unsigned int _m,
                                   float        _beta,
                                   float        _dt,
                                   float *      _h)
{
    // validate input
    if (_k < 2) {
        fprintf(stderr,"error: liquid_firdes_rkaiser(), k must be at least 2\n");
        exit(1);
    } else if (_m < 1) {
        fprintf(stderr,"error: liquid_firdes_rkaiser(), m must be at least 1\n");
        exit(1);
    } else if (_beta <= 0.0f || _beta >= 1.0f) {
        fprintf(stderr,"error: liquid_firdes_rkaiser(), beta must be in (0,1)\n");
        exit(1);
    } else if (_dt < -1.0f || _dt > 1.0f) {
        fprintf(stderr,"error: liquid_firdes_rkaiser(), dt must be in [-1,1]\n");
        exit(1);
    }

    unsigned int h_len = 2*_k*_m + 1;

    // 
#if 0
    float fc = 0.5*(1.0 + _beta)/(float)(_k);
    float as = 40.0f;
#else
    float rho_hat = rkaiser_approximate_rho(_m,_beta);
    float fc = 0.5f*(1 + _beta*(1.0f-rho_hat))/(float)_k; // filter cutoff
    float del = _beta*rho_hat / (float)_k;                // transition bandwidth
    float as = estimate_req_filter_As(del, h_len);  // stop-band suppression

    // reduce slightly to help ensure solution isn't stuck
    // (not really necessary)
    fc *= 0.995f;
    as -= 1.0f;
#endif
    float v[2] = {fc, as/1000};

    struct gsuserdata_s q = {
        .k    = _k,
        .m    = _m,
        .beta = _beta,
        .dt   = _dt,
        .w0   = 0.5f,
        .w1   = 0.5f};

    // create gradsearch object
    gradsearch gs = gradsearch_create((void*)&q,
                                      v, 2,       // vector optimizer
                                      gs_utility,
                                      LIQUID_OPTIM_MINIMIZE);

    // run search
    unsigned int i;
    unsigned int num_iterations = 100;
    for (i=0; i<num_iterations; i++) {
        // compute utility for plotting
        //float utility = gs_utility((void*)&q,v,2);

        gradsearch_step(gs);

        gradsearch_print(gs);
    }

    // destroy gradient search object
    gradsearch_destroy(gs);

    // re-design filter and return
    fc = v[0];
    as = v[1]*1000;
    liquid_firdes_kaiser(h_len,fc,as,_dt,_h);

    // normalize coefficients
    float e2 = 0.0f;
    for (i=0; i<h_len; i++) e2 += _h[i]*_h[i];
    for (i=0; i<h_len; i++) _h[i] *= sqrtf(_k/e2);
}

// gradient search utility
float gs_utility(void * _userdata,
                 float * _v,
                 unsigned int _n)
{
    // get options from _userdata object
    struct gsuserdata_s * q = (struct gsuserdata_s*)_userdata;
    float w0        = q->w0;
    float w1        = q->w1;
    unsigned int k  = q->k;
    unsigned int m  = q->m;
    float beta      = q->beta;
    float dt        = q->dt;
    unsigned int nfft=512;

    // parameters
    float fc = _v[0];
    float as = _v[1]*1000;

    // derived values
    unsigned int h_len = 2*k*m+1;

    // utilities
    float u0 = 0.0;
    float u1 = 0.0;

    // compute filter
    float h[h_len];
    liquid_firdes_kaiser(h_len,fc,as,dt,h);

    // normalize coefficients
    float e2 = 0.0f;
    unsigned int i;
    for (i=0; i<h_len; i++) e2 += h[i]*h[i];
    for (i=0; i<h_len; i++) h[i] *= sqrtf(k/e2);

    // compute filter ISI
    float isi_max;
    float isi_rms;
    liquid_filter_isi(h,k,m,&isi_rms,&isi_max);
    u0 = 20*log10f(isi_rms);

    // compute relative out-of-band energy
    float e = liquid_filter_energy(h, h_len, 0.5*(1.0+beta)/(float)k, nfft);
    u1 = 20*log10f(e);

#if DEBUG
    printf("  ISI (rms) : %12.8f, energy : %12.8f\n", u0, u1);
#endif

    // combine results
    return w0*u0 + w1*u1;
}

