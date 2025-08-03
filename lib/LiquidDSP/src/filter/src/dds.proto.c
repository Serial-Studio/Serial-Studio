/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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

// direct digital synthesizer (up/down-converter)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

struct DDS(_s) {
    // user-defined parameters
    unsigned int    num_stages;         // number of halfband stages
    float           fc0;                // high-rate center frequency (-0.5,0.5)
    float           bw0;                // low-rate bandwidth (range?)
    float           as0;                // filter stop-band attenuation [dB]

    // derived values
    unsigned int    rate;               // re-sampling rate (2^num_stages)

    // halfband decimation/interpolation stages
    RESAMP2() *     halfband_resamp;    // array of half-band resamplers
    float *         fc;                 // filter center frequency
    float *         ft;                 // filter transition bandwidth
    float *         as;                 // filter stop-band attenuation [dB] array
    unsigned int *  m;                  // filter semi-length

    // internal buffers
    unsigned int    buffer_len;
    T *             buffer0;
    T *             buffer1;

    // low-rate mixing stage
    NCO()           ncox;

    // down-converter scaling factor
    float           zeta;
    TC              scale;
};

// create dds object
//  _num_stages     :   number of halfband stages
//  _fc             :   input carrier
//  _bw             :   input signal bandwidth
//  _as             :   stop-band attenuation
DDS() DDS(_create)(unsigned int _num_stages,
                   float        _fc,
                   float        _bw,
                   float        _as)
{
    // error checking
    if (_num_stages > 20)
        return liquid_error_config("dds_%s_create(), number of stages %u exceeds reasonable maximum (20)", EXTENSION_FULL, _num_stages);
    if (_fc > 0.5f || _fc < -0.5f)
        return liquid_error_config("dds_%s_create(), frequency %12.4e is out of range [-0.5,0.5]", EXTENSION_FULL, _fc);
    if (_bw <= 0.0f || _bw >= 1.0f)
        return liquid_error_config("dds_%s_create(), bandwidth %12.4e is out of range (0,1)", EXTENSION_FULL, _bw);
    if (_as < 0.0f)
        return liquid_error_config("dds_%s_create(), stop-band suppression %12.4e must be greater than zero", EXTENSION_FULL, _as);

    // create object
    DDS() q = (DDS()) malloc(sizeof(struct DDS(_s)));
    q->num_stages = _num_stages;
    q->rate = 1<<(q->num_stages);
    q->fc0 = _fc;
    q->bw0 = _bw;
    q->as0 = _as;

    // allocate memory for filter properties
    q->fc    = (float*) malloc((q->num_stages)*sizeof(float));
    q->ft    = (float*) malloc((q->num_stages)*sizeof(float));
    q->as    = (float*) malloc((q->num_stages)*sizeof(float));
    q->m     = (unsigned int*) malloc((q->num_stages)*sizeof(unsigned int));
    unsigned int i;
    float fc, bw;
    fc = 0.5*(1<<q->num_stages)*q->fc0; // filter center frequency
    bw = q->bw0;                        // signal bandwidth
    // TODO : compute/set filter bandwidths, lengths appropriately
    for (i=0; i<q->num_stages; i++) {
        q->fc[i] = fc;
        while (q->fc[i] >  0.5f) q->fc[i] -= 1.0f;
        while (q->fc[i] < -0.5f) q->fc[i] += 1.0f;

        // compute transition bandwidth
        q->ft[i] = 0.5f*(1.0f - bw);
        if (q->ft[i] > 0.45) q->ft[i] = 0.45f; // set maximum bandwidth
        q->as[i] = q->as0;

        // compute (estimate) required filter length
        //q->m[i] = i==0 ? 37 : q->h_m[i-1]*0.7;
        q->m[i] = estimate_req_filter_len(q->ft[i], q->as[i]);

        // update carrier, bandwidth parameters
        fc *= 0.5f;
        bw *= 0.5f;
    }

    // allocate memory for buffering
    q->buffer_len = q->rate;
    q->buffer0 = (T*) malloc((q->buffer_len)*sizeof(T));
    q->buffer1 = (T*) malloc((q->buffer_len)*sizeof(T));

    // allocate memory for resampler pointers and create objects
    q->halfband_resamp = (RESAMP2()*) malloc((q->num_stages)*sizeof(RESAMP()*));
    for (i=0; i<q->num_stages; i++) {
        q->halfband_resamp[i] = RESAMP2(_create)(q->m[i],
                                                 q->fc[i],
                                                 q->as[i]);
    }

    // set down-converter scaling factor
    q->zeta  = 1.0f / ((float)(q->rate));
    q->scale = 1.0f;

    // create NCO and set frequency
    q->ncox = NCO(_create)(LIQUID_VCO);
    // TODO : ensure range is in [-pi,pi]
    NCO(_set_frequency)(q->ncox, 2*M_PI*(q->rate)*(q->fc0));

    return q;
}

// copy object
DDS() DDS(_copy)(DDS() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("dds_%s_create(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy internal memory
    DDS() q_copy = (DDS()) malloc(sizeof(struct DDS(_s)));
    memmove(q_copy, q_orig, sizeof(struct DDS(_s)));

    // copy array of half-band resamplers
    q_copy->halfband_resamp = (RESAMP2()*) malloc((q_copy->num_stages)*sizeof(RESAMP()*));
    unsigned int i;
    for (i=0; i<q_copy->num_stages; i++)
        q_copy->halfband_resamp[i] = RESAMP2(_copy)(q_orig->halfband_resamp[i]);

    // copy arrays of parameters
    q_copy->fc = (float *      ) liquid_malloc_copy(q_orig->fc, q_copy->num_stages, sizeof(float       ));
    q_copy->ft = (float *      ) liquid_malloc_copy(q_orig->ft, q_copy->num_stages, sizeof(float       ));
    q_copy->as = (float *      ) liquid_malloc_copy(q_orig->as, q_copy->num_stages, sizeof(float       ));
    q_copy->m  = (unsigned int*) liquid_malloc_copy(q_orig->m,  q_copy->num_stages, sizeof(unsigned int));

    // copy buffers
    q_copy->buffer0 = (T*) liquid_malloc_copy(q_orig->buffer0, q_orig->buffer_len, sizeof(T));
    q_copy->buffer1 = (T*) liquid_malloc_copy(q_orig->buffer1, q_orig->buffer_len, sizeof(T));

    // copy mixer
    q_copy->ncox = NCO(_copy)(q_orig->ncox);

    // return new object
    return q_copy;
}

// destroy dds object, freeing all internally-allocated memory
int DDS(_destroy)(DDS() _q)
{
    // free filter parameter arrays
    free(_q->m);
    free(_q->as);
    free(_q->fc);
    free(_q->ft);

    // destroy buffers
    free(_q->buffer0);
    free(_q->buffer1);

    // destroy halfband resampler objects
    unsigned int i;
    for (i=0; i<_q->num_stages; i++)
        RESAMP2(_destroy)(_q->halfband_resamp[i]);
    free(_q->halfband_resamp);

    // destroy NCO object
    NCO(_destroy)(_q->ncox);

    // destroy DDS object
    free(_q);
    return LIQUID_OK;
}

// print dds object internals
int DDS(_print)(DDS() _q)
{
    printf("<liquid.dds, rate=%u, fc=%g, bw=%g, nco=%g, as=%g, n=%u, stages=[",
        _q->rate,
        _q->fc0,
        _q->bw0,
        nco_crcf_get_frequency(_q->ncox) / (2.0f*M_PI),
        _q->as0,
        _q->num_stages);
    unsigned int i;
    for (i=0; i<_q->num_stages; i++)
        printf("{fc=%.5f, ft=%.5f, m=%u},",_q->fc[i],_q->ft[i],_q->m[i]);
    printf("]>\n");
    return LIQUID_OK;
}

// reset dds object internals, reset filters and nco phase
int DDS(_reset)(DDS() _q)
{
    // reset internal filter state variables
    unsigned int i;
    for (i=0; i<_q->num_stages; i++) {
        RESAMP2(_reset)(_q->halfband_resamp[i]);
    }

    NCO(_set_phase)(_q->ncox,0.0f);
    return LIQUID_OK;
}

// Set output scaling for filter
//  _q      : synthesizer object
//  _scale  : scaling factor to apply to each output sample
int DDS(_set_scale)(DDS() _q,
                    TC    _scale)
{
    _q->scale = _scale;
    return LIQUID_OK;
}

// Get output scaling for filter
//  _q      : synthesizer object
//  _scale  : scaling factor to apply to each output sample
int DDS(_get_scale)(DDS() _q,
                    TC *  _scale)
{
    *_scale = _q->scale;
    return LIQUID_OK;
}

// Get number of half-band states in DDS object
unsigned int DDS(_get_num_stages)(DDS() _q)
{
    return _q->num_stages;
}

// Get delay (samples) when running as interpolator
unsigned int DDS(_get_delay_interp)(DDS() _q)
{
    unsigned int i, delay=0;
    for (i=0; i<_q->num_stages; i++) {
        delay *= 2;
        delay += 2*_q->m[i];
    }
    return delay;
}

// Get delay (samples) when running as decimator
float DDS(_get_delay_decim)(DDS() _q)
{
    float delay = 0.0f;
    unsigned int i;
    for (i=0; i<_q->num_stages; i++) {
        delay *= 0.5f;
        delay += _q->m[_q->num_stages-i-1] - 0.5f;
    }
    return delay;
}

// execute decimator
//  _q      :   dds object
//  _x      :   input sample array [size: 2^num_stages x 1]
//  _y      :   output sample
int DDS(_decim_execute)(DDS() _q,
                        T *   _x,
                        T *   _y)
{
    // copy input data
    memmove(_q->buffer0, _x, (_q->rate)*sizeof(T));

    unsigned int k=_q->rate;// number of inputs for this stage
    unsigned int s;         // stage counter
    unsigned int i;         // input counter
    unsigned int g;         // halfband resampler stage index (reversed)
    T * b0 = NULL;          // input buffer pointer
    T * b1 = _q->buffer0;   // output buffer pointer

    // iterate through each stage
    for (s=0; s<_q->num_stages; s++) {
        // length halves with each iteration
        k >>= 1;

        // set buffer pointers
        b0 = s%2 == 0 ? _q->buffer0 : _q->buffer1;
        b1 = s%2 == 1 ? _q->buffer0 : _q->buffer1;

        // execute halfband decimator
        g = _q->num_stages - s - 1;
        for (i=0; i<k; i++)
            RESAMP2(_decim_execute)(_q->halfband_resamp[g], &b0[2*i], &b1[i]);
    }

    // output value
    T y = b1[0];

    // increment NCO
    NCO(_mix_down)(_q->ncox, y, &y);
    NCO(_step)(_q->ncox);

    // set output, normalizing by scaling factor
    *_y = y * _q->zeta * _q->scale;
    return LIQUID_OK;
}

// execute interpolator
//  _q      :   dds object
//  _x      :   input sample
//  _y      :   output sample array [size: 2^num_stages x 1]
int DDS(_interp_execute)(DDS() _q,
                         T     _x,
                         T *   _y)
{
    // increment NCO
    _x *= _q->scale;
    NCO(_mix_up)(_q->ncox, _x, &_x);
    NCO(_step)(_q->ncox);

    unsigned int s;         // stage counter
    unsigned int i;         // input counter
    unsigned int k=1;       // number of inputs for this stage
    T * b0 = NULL;          // input buffer pointer
    T * b1 = _q->buffer0;   // output buffer pointer

    // set initial buffer value
    _q->buffer0[0] = _x;

    // iterate through each stage
    for (s=0; s<_q->num_stages; s++) {

        // set buffer pointers
        b0 = s%2 == 0 ? _q->buffer0 : _q->buffer1;
        b1 = s%2 == 1 ? _q->buffer0 : _q->buffer1;

        // execute halfband interpolator
        for (i=0; i<k; i++)
            RESAMP2(_interp_execute)(_q->halfband_resamp[s], b0[i], &b1[2*i]);
        
        // length doubles with each iteration
        k <<= 1;
    }

    // copy output data
    memmove(_y, b1, (_q->rate)*sizeof(T));
    return LIQUID_OK;
}

