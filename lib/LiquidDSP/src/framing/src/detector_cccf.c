/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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
// detector_cccf.c
//
// Pre-demodulation detector
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <float.h>

#include "liquid.internal.h"

#define DEBUG_DETECTOR              0
#define DEBUG_DETECTOR_PRINT        0
#define DEBUG_DETECTOR_BUFFER_LEN   (1600)
#define DEBUG_DETECTOR_FILENAME     "detector_cccf_debug.m"

// 
// internal method declarations
//

// update sum{ |x|^2 }
void detector_cccf_update_sumsq(detector_cccf _q,
                                float complex _x);

// compute all dot product outputs
void detector_cccf_compute_dotprods(detector_cccf _q);

// estimate carrier and timing offsets
void detector_cccf_estimate_offsets(detector_cccf _q,
                                    float *       _tau_hat,
                                    float *       _dphi_hat);

// print debugging information
int detector_cccf_debug_print(detector_cccf _q,
                              const char *  _filename);

struct detector_cccf_s {
    float complex * s;      // sequence
    unsigned int n;         // sequence length
    float threshold;        // detection threshold
    
    // derived values
    float n_inv;            // 1/n (pre-computed for speed)
    
    windowcf buffer;        // input buffer

    // internal correlators
    dotprod_cccf * dp;      // vector dot products (pre-spun)
    unsigned int m;         // number of correlators
    float   dphi_step;      // step size for each correlator
    float   dphi_max;       // maximum carrier offset
    float * dphi;           // correlator frequencies [size: m x 1]
    float * rxy;            // correlator outputs [size: m x 1]
    float * rxy0;           // buffered correlator outputs [size: m x 1]
    float * rxy1;           // buffered correlator outputs [size: m x 1]
    unsigned int imax;      // index of maximum
    unsigned int idetect;   // index of detection

    // estimation of E{|x|^2}
    wdelayf x2;             // buffer of |x|^2 values
    float x2_sum;           // sum{ |x|^2 }
    float x2_hat;           // estimate of E{|x|^2}

    // counters/states
    enum {
        DETECTOR_STATE_SEEK=0,  // seek sequence
        DETECTOR_STATE_FINDMAX, // find maximum
    } state;
    unsigned int timer;         // sample timer

#if DEBUG_DETECTOR
    windowcf debug_x;
    windowf debug_x2;
    windowf debug_rxy;
#endif
};

// create detector_cccf object
//  _s          :   sequence
//  _n          :   sequence length
//  _threshold  :   detection threshold (default: 0.7)
//  _dphi_max   :   maximum carrier offset
detector_cccf detector_cccf_create(float complex * _s,
                                   unsigned int    _n,
                                   float           _threshold,
                                   float           _dphi_max)
{
    // validate input
    if (_n == 0)
        return liquid_error_config("detector_cccf_create(), sequence length cannot be zero");
    if (_threshold <= 0.0f)
        return liquid_error_config("detector_cccf_create(), threshold must be greater than zero (0.6 recommended)");
    
    // allocate memory for main object
    detector_cccf q = (detector_cccf) malloc(sizeof(struct detector_cccf_s));
    unsigned int i;

    // set internal properties
    q->n         = _n;
    q->threshold = _threshold;
    q->dphi_max  = _dphi_max;

    // derived values
    q->n_inv = 1.0f / (float)(q->n);    // 1/n for faster processing
    q->dphi_step = 0.8f * M_PI / (float)(q->n);

    // compute number of correlators
    q->m = (int) ceilf( fabsf(_dphi_max / q->dphi_step) );
    
    // ensure at least two correlators
    if (q->m < 2)
        q->m = 2;

    // re-compute maximum carrier offset
    q->dphi_max = q->m * q->dphi_step;

    // allocate memory for sequence and copy
    q->s = (float complex*) malloc((q->n)*sizeof(float complex));
    memmove(q->s, _s, q->n*sizeof(float complex));

    // create internal buffer
    q->buffer = windowcf_create(q->n);
    q->x2     = wdelayf_create(q->n);

    // create internal correlators (dot products)
    q->dp   = (dotprod_cccf*) malloc((q->m)*sizeof(dotprod_cccf));
    q->dphi = (float*)        malloc((q->m)*sizeof(float));
    q->rxy0 = (float*)        malloc((q->m)*sizeof(float));
    q->rxy1 = (float*)        malloc((q->m)*sizeof(float));
    q->rxy  = (float*)        malloc((q->m)*sizeof(float));
    unsigned int k;
    float complex sconj[q->n];
    for (k=0; k<q->m; k++) {
        // pre-spin sequence (slightly over-sampled in frequency)
        q->dphi[k] = ((float)k - (float)(q->m-1)/2) * q->dphi_step;
        for (i=0; i<q->n; i++)
            sconj[i] = conjf(q->s[i]) * cexpf(-_Complex_I*q->dphi[k]*i);
        q->dp[k] = dotprod_cccf_create(sconj, q->n);
    }

    // reset state
    detector_cccf_reset(q);

#if DEBUG_DETECTOR
    q->debug_x   = windowcf_create(DEBUG_DETECTOR_BUFFER_LEN);
    q->debug_x2  = windowf_create(DEBUG_DETECTOR_BUFFER_LEN);
    q->debug_rxy = windowf_create(DEBUG_DETECTOR_BUFFER_LEN);
#endif
    // return object
    return q;
}

void detector_cccf_destroy(detector_cccf _q)
{
#if DEBUG_DETECTOR
    detector_cccf_debug_print(_q, DEBUG_DETECTOR_FILENAME);
    windowcf_destroy(_q->debug_x);
    windowf_destroy(_q->debug_x2);
    windowf_destroy(_q->debug_rxy);
#endif
    // destroy input buffer
    windowcf_destroy(_q->buffer);

    // destroy internal correlators (dot products)
    unsigned int k;
    for (k=0; k<_q->m; k++)
        dotprod_cccf_destroy(_q->dp[k]);
    free(_q->dp);
    free(_q->dphi);
    free(_q->rxy);
    free(_q->rxy0);
    free(_q->rxy1);

    // destroy |x|^2 buffer
    wdelayf_destroy(_q->x2);

    // free internal buffers/arrays
    free(_q->s);

    // free main object memory
    free(_q);
}

void detector_cccf_print(detector_cccf _q)
{
    printf("<liquid.detector_cccf:\n");
    printf(", seq=%u", _q->n);
    printf(", thresh=%g", _q->threshold);
    printf(", dphi_max=%8.4f", _q->dphi_max);
    printf(", ncorrs=%u", _q->m);
    printf(">\n");
}

void detector_cccf_reset(detector_cccf _q)
{
    // reset internal state
    windowcf_reset(_q->buffer);
    wdelayf_reset(_q->x2);

    // reset internal state
    _q->timer   = _q->n;                // reset timer
    _q->state   = DETECTOR_STATE_SEEK;  // set state to seek threshold
    _q->imax    = 0;                    // index of maximum rxy value
    _q->idetect = 0;                    // index of detected maximum
    _q->x2_sum  = 0.0f;                 // sum{ |x|^2 }
    
    // clear cross-correlator outputs
    //memset(_q->rxy, 0x00, sizeof(_q->rxy));
    memset(_q->rxy0, 0x00, _q->m*sizeof(float));
    memset(_q->rxy1, 0x00, _q->m*sizeof(float));
}

// Run sample through pre-demod detector's correlator.
// Returns '1' if signal was detected, '0' otherwise
//  _q          :   pre-demod detector
//  _x          :   input sample
//  _tau_hat    :   fractional sample offset estimate (set when detected)
//  _dphi_hat   :   carrier frequency offset estimate (set when detected)
//  _gamma_hat  :   channel gain estimate (set when detected)
int detector_cccf_correlate(detector_cccf _q,
                            float complex _x,
                            float *       _tau_hat,
                            float *       _dphi_hat,
                            float *       _gamma_hat)
{
    // push sample into buffer
    windowcf_push(_q->buffer, _x);

    // update sum{|x|^2}
    detector_cccf_update_sumsq(_q, _x);

#if DEBUG_DETECTOR
    windowcf_push(_q->debug_x, _x);
    windowf_push(_q->debug_x2, _q->x2_hat);
#endif
    // return if no timeout
    if (_q->timer) {
        // hasn't timed out yet
        //printf("timer = %u\n", _q->timer);
        _q->timer--;
#if DEBUG_DETECTOR
        windowf_push(_q->debug_rxy, 0.0f);
#endif
        return 0;
    }

    // save previous correlator outputs
    memmove(_q->rxy0, _q->rxy1, _q->m*sizeof(float));
    memmove(_q->rxy1, _q->rxy,  _q->m*sizeof(float));

    // compute vector dot products
    detector_cccf_compute_dotprods(_q);

    // find max{rxy}
    float rxy_abs = _q->rxy[ _q->imax ];

#if DEBUG_DETECTOR
    windowf_push(_q->debug_rxy, rxy_abs);
#endif
    
    if (_q->state == DETECTOR_STATE_SEEK) {
        // check to see if value exceeds threshold
        if (rxy_abs > _q->threshold) {
#if DEBUG_DETECTOR_PRINT
            printf("threshold exceeded:      rxy = %8.4f\n", rxy_abs);
#endif
            _q->idetect = _q->imax;
            _q->state = DETECTOR_STATE_FINDMAX;
        }
    } else if (_q->state == DETECTOR_STATE_FINDMAX) {
        // see if this new value exceeds maximum
        if ( _q->rxy[_q->imax] > _q->rxy1[_q->idetect] ) {
#if DEBUG_DETECTOR_PRINT
            printf("maximum not yet reached: rxy = %8.4f\n", rxy_abs);
#endif
            // set new index of maximum
            _q->idetect = _q->imax;
        } else {
            // peak was found last time; run estimates, reset values,
            // and return
#if DEBUG_DETECTOR_PRINT
            printf("maximum found:           rxy = %8.4f\n", rxy_abs);
#endif
            
            // estimate timing and carrier offsets
            detector_cccf_estimate_offsets(_q, _tau_hat, _dphi_hat);

            *_gamma_hat = sqrtf(_q->x2_hat);

            // soft state reset
            _q->state = DETECTOR_STATE_SEEK;
            // set timer to allow signal to settle
            _q->timer = _q->n/4;

            return 1;
        }
    } else {
        liquid_error(LIQUID_EINT,"detector_cccf_correlate(), unknown/unsupported internal state");
        return 0;
    }

    return 0;
}

// 
// internal methods
//

// compute sum{ |x|^2 }
void detector_cccf_update_sumsq(detector_cccf _q,
                                float complex _x)
{
    // update estimate of signal magnitude
    float x2_n = crealf(_x * conjf(_x));    // |x[n-1]|^2 (input sample)
    float x2_0;                             // |x[0]  |^2 (oldest sample)
    wdelayf_push(_q->x2, x2_n);             // push newest sample
    wdelayf_read(_q->x2, &x2_0);            // read oldest sample
    _q->x2_sum = _q->x2_sum + x2_n - x2_0;  // update sum( |x|^2 ) of last 'n' input samples
    if (_q->x2_sum < FLT_EPSILON) {
        _q->x2_sum = FLT_EPSILON;
    }
#if 0
    // filtered estimate of E{ |x|^2 }
    _q->x2_hat = 0.8f*_q->x2_hat + 0.2f*_q->x2_sum*_q->n_inv;
#else
    // unfiltered estimate of E{ |x|^2 }
    _q->x2_hat = _q->x2_sum * _q->n_inv;
#endif

}

// compute all dot product outputs
void detector_cccf_compute_dotprods(detector_cccf _q)
{
    // read buffer
    float complex * r;
    windowcf_read(_q->buffer, &r);

    // compute dot products
    // TODO: compute conjugate as well
    unsigned int k;
    float complex rxy;
#if DEBUG_DETECTOR_PRINT
    printf("  rxy : ");
#endif
    float rxy_max = 0;
    // TODO: peridically re-compute scaling factor)
    for (k=0; k<_q->m; k++) {
        // execute vector dot product
        dotprod_cccf_execute(_q->dp[k], r, &rxy);

        // save scaled magnitude
        // TODO: compute scaled squared magnitude so as not to have
        //       to compute square root
        _q->rxy[k] = cabsf(rxy) * _q->n_inv / sqrtf(_q->x2_hat);
#if DEBUG_DETECTOR_PRINT
        printf("%6.4f (%6.4f) ", _q->rxy[k], _q->dphi[k]);
#endif

        // find index of maximum
        if (_q->rxy[k] > rxy_max) {
            rxy_max = _q->rxy[k];
            _q->imax = k;
        }
    }
#if DEBUG_DETECTOR_PRINT
    printf("\n");
#endif
}

// estimate carrier and timing offsets
void detector_cccf_estimate_offsets(detector_cccf _q,
                                    float *       _tau_hat,
                                    float *       _dphi_hat)
{
    // check length of...
    if (_q->m == 1) {
        // compute carrier offset
        //*_dphi_hat = _q->dphi[_q->idetect];
        *_dphi_hat = 0.0f;

        // interpolate to find timing offset
        //*_tau_hat  = 0.5f*(_q->rxy2 - _q->rxy0) / (_q->rxy2 + _q->rxy0 - 2*_q->rxy1);
        *_tau_hat = 0.0f;
        return;
    }

    // _q->rxy0:          [rm0]
    //                      |
    //                      |
    //                      |
    // _q->rxy1:  [r0m]---[r00]---[r0p]-->freq
    //                      |
    //                      |
    //                      |
    //                    [rp0]
    //                      |
    // _q->rxy              V time
    //
    float rm0 = _q->rxy0[_q->idetect];
    float r0m = _q->idetect==0 ? _q->rxy1[_q->idetect+1] : _q->rxy1[_q->idetect-1];
    float r00 = _q->rxy1[_q->idetect];
    float r0p = _q->idetect==_q->m-1 ? _q->rxy1[_q->idetect-1] : _q->rxy1[_q->idetect+1];
    float rp0 = _q->rxy[_q->idetect];

#if DEBUG_DETECTOR_PRINT
    // print values for interpolation
    printf("idetect : %u\n", _q->idetect);
    printf("             [%8.5f]\n", rm0);
    printf("                  |\n");
    printf("                  |\n");
    printf("[%8.5f]---[%8.5f]----[%8.5f]--> freq\n", r0m, r00, r0p);
    printf("              %8.5f\n", _q->dphi[_q->idetect]);
    printf("                  |\n");
    printf("             [%8.5f]\n", rp0);
    printf("                  |\n");
    printf("                  V time\n");
#endif

    // interpolate frequency offset estimate
    float dphi_detect = _q->dphi[_q->idetect];       // frequency from detection
    *_dphi_hat        = dphi_detect - _q->dphi_step * 0.5f*(r0p - r0m) / (r0p + r0m - 2*r00);

    // interpolate timing offset estimate
    *_tau_hat  =  0.5f*(rp0 - rm0) / (rp0 + rm0 - 2*r00);

    // force result to be in proper range
    if (*_tau_hat < -0.499f) *_tau_hat = -0.499f;
    if (*_tau_hat >  0.499f) *_tau_hat =  0.499f;
}

#if 0
float detector_cccf_estimate_dphi(detector_cccf _q)
{
    // read buffer...
    float complex * r;
    windowcf_read(_q->buffer, &r);

    //
    float complex r0 = 0.0f;
    float complex r1 = 0.0f;
    float complex metric = 0.0f;
    unsigned int i;
    for (i=0; i<_q->n; i++) {
        r0 = r1;
        r1 = r[i] * conjf(_q->s[i]);

        metric += r1 * conjf(r0);
    }

    return cargf(metric);
}
#endif

int detector_cccf_debug_print(detector_cccf _q,
                              const char *  _filename)
{
#if DEBUG_DETECTOR
    FILE * fid = fopen(_filename,"w");
    if (!fid)
        return liquid_error(LIQUID_EIO,"detector_cccf_debug_print(), could not open '%s' for writing", _filename);

    fprintf(fid,"%% %s : auto-generated file\n", DEBUG_DETECTOR_FILENAME);
    fprintf(fid,"close all;\n");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"N = %u;\n", DEBUG_DETECTOR_BUFFER_LEN);
    unsigned int i;
    float complex * rc;
    float * r;

    fprintf(fid,"x = zeros(1,N);\n");
    windowcf_read(_q->debug_x, &rc);
    for (i=0; i<DEBUG_DETECTOR_BUFFER_LEN; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));

    fprintf(fid,"rxy = zeros(1,N);\n");
    windowf_read(_q->debug_rxy, &r);
    for (i=0; i<DEBUG_DETECTOR_BUFFER_LEN; i++)
        fprintf(fid,"rxy(%4u) = %12.4e;\n", i+1, r[i]);

    fprintf(fid,"x2 = zeros(1,N);\n");
    windowf_read(_q->debug_x2, &r);
    for (i=0; i<DEBUG_DETECTOR_BUFFER_LEN; i++)
        fprintf(fid,"x2(%4u) = %12.4e;\n", i+1, r[i]);

    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(N-1);\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t,real(x),t,imag(x));\n");
    fprintf(fid,"  ylabel('received signal, x');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(t, rxy);\n");
    fprintf(fid,"  ylabel('rxy');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(t, x2);\n");
    fprintf(fid,"  ylabel('rssi');\n");
    fprintf(fid,"  grid on;\n");

    fclose(fid);
    printf("detector_cccf/debug: results written to '%s'\n", _filename);
#else
    return liquid_error(LIQUID_EUMODE,"detector_cccf_debug_print(): compile-time debugging disabled\n");
#endif
    return LIQUID_OK;
}

