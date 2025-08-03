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
//  [harris:2005] f. harris, C. Dick, S. Seshagiri, K. Moerder, "An
//      Improved Square-Root Nyquist Shaping Filter," Proceedings of
//      the Software-Defined Radio Forum, 2005
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

#define DEBUG_hM3   0

// Design root-Nyquist harris-Moerder filter using Parks-McClellan
// algorithm
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter [size: 2*_k*_m+1]
int liquid_firdes_hM3(unsigned int _k,
                      unsigned int _m,
                      float _beta,
                      float _dt,
                      float * _h)
{
    if ( _k < 2 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_hM3(): k must be greater than 1");
    if ( _m < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_hM3(): m must be greater than 0");
    if ( (_beta < 0.0f) || (_beta > 1.0f) )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_hM3(): beta must be in [0,1]");

    unsigned int n=2*_k*_m+1;       // filter length

    //
    float fc = 1.0 / (float)(2*_k); // filter cutoff
    float fp = fc*(1.0 - _beta);    // pass-band
    float fs = fc*(1.0 + _beta);    // stop-band

    // root nyquist
    unsigned int num_bands = 3;
    float bands[6]   = {0.0f, fp, fc, fc, fs, 0.5f};
    float des[3]     = {1.0f, 1.0f/sqrtf(2.0f), 0.0f};
    float weights[3] = {1.0f, 1.0f, 1.0f};

    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    liquid_firdespm_wtype wtype[3] = {LIQUID_FIRDESPM_FLATWEIGHT,
                                      LIQUID_FIRDESPM_FLATWEIGHT,
                                      LIQUID_FIRDESPM_EXPWEIGHT};

    //unsigned int i;
    float h[n];
    firdespm_run(n,num_bands,bands,des,weights,wtype,btype,h);
    // copy results
    memmove(_h, h, n*sizeof(float));

    float isi_max;
    float isi_rms;
    liquid_filter_isi(h,_k,_m,&isi_rms,&isi_max);

    // iterate...
    float isi_rms_min = isi_rms;
    unsigned int p, pmax=100;
    for (p=0; p<pmax; p++) {
        // increase pass-band edge
        fp = fc*(1.0 - _beta * p / (float)(pmax) );
        bands[1] = fp;

        // execute filter design
        firdespm_run(n,num_bands,bands,des,weights,wtype,btype,h);

        // compute inter-symbol interference (MSE, max)
        liquid_filter_isi(h,_k,_m,&isi_rms,&isi_max);

#if DEBUG_hM3
        printf("  isi mse : %20.8e (min: %20.8e)\n", isi_rms, isi_rms_min);
#endif
        if (isi_rms > isi_rms_min) {
            // search complete
            break;
        } else {
            isi_rms_min = isi_rms;
            // copy results
            memmove(_h, h, n*sizeof(float));
        }
    };

    // normalize
    float e2 = 0.0f;
    unsigned int i;
    for (i=0; i<n; i++) { e2 += _h[i]*_h[i];     }
    for (i=0; i<n; i++) { _h[i] *= sqrtf(_k/e2); }
    
    return LIQUID_OK;
}

