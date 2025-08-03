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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid.h"

// structured data type
struct firdespm_halfband_s {
    // top-level filter design parameters
    unsigned int    m;          // filter semi-length
    unsigned int    h_len;      // filter length, 4*m+1
    float           ft;         // desired transition band
    float *         h;          // resulting filter coefficients

    // utility calculation
    unsigned int    nfft;       // transform size for analysis
    float complex * buf_time;   // time buffer
    float complex * buf_freq;   // frequency buffer
    fftplan         fft;        // transform object
    unsigned int    n;          // number of points to evaluate
};

float firdespm_halfband_utility(float _gamma, void * _userdata)
{
    // type-cast input structure as pointer
    struct firdespm_halfband_s * q = (struct firdespm_halfband_s*)_userdata;

    // design filter
    float f0 = 0.25f - 0.5f*q->ft*_gamma;
    float f1 = 0.25f + 0.5f*q->ft;
    float bands[4]   = {0.00f, f0, f1, 0.50f};
    float des[2]     = {1.0f, 0.0f};
    float weights[2] = {1.0f, 1.0f}; // best with {1, 1}
    liquid_firdespm_wtype wtype[2] = { // best with {flat, flat}
        LIQUID_FIRDESPM_FLATWEIGHT, LIQUID_FIRDESPM_FLATWEIGHT,};
    firdespm_run(q->h_len, 2, bands, des, weights, wtype,
        LIQUID_FIRDESPM_BANDPASS, q->h);

    // compute utility; copy ideal non-zero coefficients and compute transform
    unsigned int i;
    // force zeros for even coefficients
    for (i=0; i<q->m; i++) {
        q->h[           2*i] = 0;
        q->h[q->h_len-2*i-1] = 0;
    }
    // copy coefficients to input buffer
    for (i=0; i<q->nfft; i++) {
        q->buf_time[i] = i < q->h_len ? q->h[i] : 0.0f;
    }
    // compute transform
    fft_execute(q->fft);

    // compute metric: power in stop-band
    float u = 0.0f;
    for (i=0; i<q->n; i++) {
        unsigned int idx = q->nfft/2 - i;
        float u_test = cabsf(q->buf_freq[idx]);
        //printf(" %3u : %12.8f : %12.3f\n", i, (float)(idx) / (float)(q->nfft), 20*log10f(u_test));
        u += u_test * u_test;
    }

    // return utility in dB
    return 10*log10f(u / (float)(q->n));
}

// perform search to find optimal coefficients given transition band
int liquid_firdespm_halfband_ft(unsigned int _m, float _ft, float * _h)
{
    // create and initialize object
    struct firdespm_halfband_s q;
    q.m     = _m;
    q.h_len = 4*_m+1;
    q.ft    = _ft;
    q.h     = (float*)malloc(q.h_len*sizeof(float));

    // initialize values for utility calculation
    q.nfft = 1200;
    while (q.nfft < 20*q.m)
        q.nfft <<= 1;
    q.buf_time = (float complex*) fft_malloc(q.nfft*sizeof(float complex));
    q.buf_freq = (float complex*) fft_malloc(q.nfft*sizeof(float complex));
    q.fft      = fft_create_plan(q.nfft, q.buf_time, q.buf_freq, LIQUID_FFT_FORWARD, 0);

    // compute indices of stop-band analysis
    q.n = (unsigned int)(q.nfft * (0.25f - 0.5f*_ft));
    //printf("firdespm.halfband, m=%u, ft=%.3f, n=%u, nfft=%u, n=%u\n",
    //    q.m, q.ft, q.h_len, q.nfft, q.n);

    // create and run search
    qs1dsearch optim = qs1dsearch_create(firdespm_halfband_utility, &q, LIQUID_OPTIM_MINIMIZE);
    qs1dsearch_init_bounds(optim, 1.0f, 0.9f);
    unsigned int i;
    for (i=0; i<32; i++) {
        qs1dsearch_step (optim);
        //qs1dsearch_print(optim);
    }
    qs1dsearch_destroy(optim);

    // copy optimal coefficients
    memmove(_h, q.h, q.h_len*sizeof(float));

    // destroy objects
    free(q.h);
    fft_destroy_plan(q.fft);
    fft_free(q.buf_time);
    fft_free(q.buf_freq);

    //
    return LIQUID_OK;
}

// perform search to find optimal coefficients given stop-band suppression
int liquid_firdespm_halfband_as(unsigned int _m, float _as, float * _h)
{
    // estimate transition band given other parameters
    float ft = estimate_req_filter_df(_as, 4*_m+1);

    // return filter design
    return liquid_firdespm_halfband_ft(_m, ft, _h);
}

