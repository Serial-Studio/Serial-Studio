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

//
// Symbol synchronizer
//
// References:
//  [Mengali:1997] Umberto Mengali and Aldo N. D'Andrea,
//      "Synchronization Techniques for Digital Receivers,"
//      Plenum Press, New York & London, 1997.
//  [harris:2001] frederic j. harris and Michael Rice,
//      "Multirate Digital Filters for Symbol Timing Synchronization
//      in Software Defined Radios," IEEE Journal on Selected Areas
//      of Communications, vol. 19, no. 12, December, 2001, pp.
//      2346-2357.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//
// forward declaration of internal methods
//

// step synchronizer
//  _q      : symsync object
//  _x      : input sample
//  _y      : output sample array pointer
//  _ny     : number of output samples written
int SYMSYNC(_step)(SYMSYNC()      _q,
                   TI             _x,
                   TO *           _y,
                   unsigned int * _ny);

// advance synchronizer's internal loop filter
//  _q      : synchronizer object
//  _mf     : matched-filter output
//  _dmf    : derivative matched-filter output
int SYMSYNC(_advance_internal_loop)(SYMSYNC() _q,
                                    TO        _mf,
                                    TO        _dmf);

// internal structure
struct SYMSYNC(_s) {
    unsigned int h_len;         // matched filter length
    unsigned int k;             // samples/symbol (input)
    unsigned int k_out;         // samples/symbol (output)

    unsigned int decim_counter; // decimation counter
    int is_locked;              // synchronizer locked flag

    float rate;                 // internal resampling rate
    float del;                  // fractional delay step

    // floating-point timing phase
    float tau;                  // accumulated timing phase (0 <= tau <= 1)
    float tau_decim;            // timing phase, retained for get_tau() method
    float bf;                   // soft filterbank index
    int   b;                    // filterbank index

    // loop filter
    float q;                    // instantaneous timing error
    float q_hat;                // filtered timing error
    float B[3];                 // loop filter feed-forward coefficients
    float A[3];                 // loop filter feed-back coefficients
    iirfiltsos_rrrf pll;        // loop filter object (iir filter)
    float rate_adjustment;      // internal rate adjustment factor

    unsigned int npfb;          // number of filters in the bank
    FIRPFB()      mf;           // matched filter
    FIRPFB()     dmf;           // derivative matched filter
};

// create synchronizer object from external coefficients
//  _k      : samples per symbol
//  _M      : number of filters in the bank
//  _h      : matched filter coefficients
//  _h_len  : length of matched filter
SYMSYNC() SYMSYNC(_create)(unsigned int _k,
                           unsigned int _M,
                           TC *         _h,
                           unsigned int _h_len)
{
    // validate input
    if (_k < 2)
        return liquid_error_config("symsync_%s_create(), input sample rate must be at least 2", EXTENSION_FULL);
    if (_M == 0)
        return liquid_error_config("symsync_%s_create(), number of filter banks must be greater than zero", EXTENSION_FULL);
    if (_h_len == 0)
        return liquid_error_config("symsync_%s_create(), filter length must be greater than zero", EXTENSION_FULL);
    if ( (_h_len-1) % _M )
        return liquid_error_config("symsync_%s_create(), filter length must be of the form: h_len = m*_k*_M + 1 ", EXTENSION_FULL);

    // create main object
    SYMSYNC() q = (SYMSYNC()) malloc(sizeof(struct SYMSYNC(_s)));

    // set internal properties
    q->k    = _k;  // input samples per symbol
    q->npfb = _M;  // number of filters in the polyphase filter bank

    // set output rate (nominally 1, full decimation)
    SYMSYNC(_set_output_rate)(q, 1);

    // set internal sub-filter length
    q->h_len = (_h_len-1)/q->npfb;

    // compute derivative filter
    TC dh[_h_len];
    float hdh_max = 0.0f;
    unsigned int i;
    for (i=0; i<_h_len; i++) {
        if (i==0) {
            dh[i] = _h[i+1] - _h[_h_len-1];
        } else if (i==_h_len-1) {
            dh[i] = _h[0]   - _h[i-1];
        } else {
            dh[i] = _h[i+1] - _h[i-1];
        }

        // find maximum of h*dh
        if ( fabsf(_h[i]*dh[i]) > hdh_max || i==0 )
            hdh_max = fabsf(_h[i]*dh[i]);
    }

    // apply scaling factor for normalized response
    for (i=0; i<_h_len; i++)
        dh[i] *= 0.06f / hdh_max;

    q->mf  = FIRPFB(_create)(q->npfb, _h, _h_len);
    q->dmf = FIRPFB(_create)(q->npfb, dh, _h_len);

    // reset state and initialize loop filter
    q->A[0] = 1.0f;     q->B[0] = 0.0f;
    q->A[1] = 0.0f;     q->B[1] = 0.0f;
    q->A[2] = 0.0f;     q->B[2] = 0.0f;
    q->pll = iirfiltsos_rrrf_create(q->B, q->A);
    SYMSYNC(_reset)(q);
    SYMSYNC(_set_lf_bw)(q, 0.01f);

    // unlock loop control
    SYMSYNC(_unlock)(q);

    // return main object
    return q;
}

// create square-root Nyquist symbol synchronizer
//  _type   : filter type (e.g. LIQUID_FIRFILT_RRC)
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _M      : number of filters in the bank
SYMSYNC() SYMSYNC(_create_rnyquist)(int          _type,
                                    unsigned int _k,
                                    unsigned int _m,
                                    float        _beta,
                                    unsigned int _M)
{
    // validate input
    if (_k < 2)
        return liquid_error_config("symsync_%s_create_rnyquist(), samples/symbol must be at least 2", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("symsync_%s_create_rnyquist(), filter delay (m) must be greater than zero", EXTENSION_FULL);
    if (_beta < 0.0f || _beta > 1.0f)
        return liquid_error_config("symsync_%s_create_rnyquist(), filter excess bandwidth must be in [0,1]", EXTENSION_FULL);
    if (_M == 0)
        return liquid_error_config("symsync_%s_create_rnyquist(), number of filters in bnak must be greater than zero", EXTENSION_FULL);

    // allocate memory for filter coefficients
    unsigned int H_len = 2*_M*_k*_m + 1;
    float Hf[H_len];

    // design square-root Nyquist pulse-shaping filter
    liquid_firdes_prototype(_type, _k*_M, _m, _beta, 0, Hf);

    // copy coefficients to type-specific array
    TC H[H_len];
    unsigned int i;
    for (i=0; i<H_len; i++)
        H[i] = Hf[i];

    // create object and return
    return SYMSYNC(_create)(_k, _M, H, H_len);
}

// create symsync using Kaiser filter interpolator; useful
// when the input signal has matched filter applied already
//  _k      : input samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor, beta in (0,1]
//  _M      : number of filters in the bank
SYMSYNC() SYMSYNC(_create_kaiser)(unsigned int _k,
                                  unsigned int _m,
                                  float        _beta,
                                  unsigned int _M)
{
    // validate input
    if (_k < 2)
        return liquid_error_config("symsync_%s_create_kaiser(), samples/symbol must be at least 2", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("symsync_%s_create_kaiser(), filter delay (m) must be greater than zero", EXTENSION_FULL);
    if (_beta < 0.0f || _beta > 1.0f)
        return liquid_error_config("symsync_%s_create_kaiser(), filter excess bandwidth must be in [0,1]", EXTENSION_FULL);
    if (_M == 0)
        return liquid_error_config("symsync_%s_create_kaiser(), number of filters in bnak must be greater than zero", EXTENSION_FULL);

    // allocate memory for filter coefficients
    unsigned int H_len = 2*_M*_k*_m + 1;
    float Hf[H_len];

    // design interpolating filter whose bandwidth is outside the cut-off
    // frequency of input signal
    // TODO: use _beta to compute more accurate cut-off frequency
    float fc = 0.75f;   // filter cut-off frequency (nominal)
    float as = 40.0f;   // filter stop-band attenuation
    liquid_firdes_kaiser(H_len, fc / (float)(_k*_M), as, 0.0f, Hf);

    // copy coefficients to type-specific array, adjusting to relative
    // filter gain
    unsigned int i;
    TC H[H_len];
    for (i=0; i<H_len; i++)
        H[i] = Hf[i] * 2.0f * fc;

    // create object and return
    return SYMSYNC(_create)(_k, _M, H, H_len);
}

// copy object
SYMSYNC() SYMSYNC(_copy)(SYMSYNC() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("symsync_%s_copy(), object cannot be NULL", EXTENSION_FULL);

    // create object, copy internal memory, overwrite with specific values
    SYMSYNC() q_copy = (SYMSYNC()) malloc(sizeof(struct SYMSYNC(_s)));
    memmove(q_copy, q_orig, sizeof(struct SYMSYNC(_s)));

    // copy phased-locked loop
    q_copy->pll = iirfiltsos_rrrf_copy(q_orig->pll);

    // copy filter banks
    q_copy->mf  = FIRPFB(_copy)(q_orig->mf);
    q_copy->dmf = FIRPFB(_copy)(q_orig->dmf);

    // return object
    return q_copy;
}

// destroy symsync object, freeing all internal memory
int SYMSYNC(_destroy)(SYMSYNC() _q)
{
    // destroy filterbank objects
    FIRPFB(_destroy)(_q->mf);
    FIRPFB(_destroy)(_q->dmf);

    // destroy timing phase-locked loop filter
    iirfiltsos_rrrf_destroy(_q->pll);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print symsync object's parameters
int SYMSYNC(_print)(SYMSYNC() _q)
{
    printf("<liquid.symsync_%s, rate=%g, k_in=%u, k_out=%u>\n",
        EXTENSION_FULL, _q->rate, _q->k, _q->k_out);
    return FIRPFB(_print)(_q->mf);
}

// reset symsync internal state
int SYMSYNC(_reset)(SYMSYNC() _q)
{
    // reset polyphase filterbank
    FIRPFB(_reset)(_q->mf);

    // reset counters, etc.
    _q->rate          = (float)_q->k / (float)_q->k_out;
    _q->del           = _q->rate;
    _q->b             =   0;    // filterbank index
    _q->bf            = 0.0f;   // filterbank index (soft value)
    _q->tau           = 0.0f;   // instantaneous timing estimate
    _q->tau_decim     = 0.0f;   // instantaneous timing estaimte (decimated)
    _q->q             = 0.0f;   // timing error
    _q->q_hat         = 0.0f;   // filtered timing error
    _q->decim_counter = 0;      // decimated output counter

    // reset timing phase-locked loop filter
    return iirfiltsos_rrrf_reset(_q->pll);
}

// lock synchronizer object
int SYMSYNC(_lock)(SYMSYNC() _q)
{
    _q->is_locked = 1;
    return LIQUID_OK;
}

// unlock synchronizer object
int SYMSYNC(_unlock)(SYMSYNC() _q)
{
    _q->is_locked = 0;
    return LIQUID_OK;
}

// check lock state
int SYMSYNC(_is_locked)(SYMSYNC() _q)
{
    return _q->is_locked;
}

// set synchronizer output rate (samples/symbol)
//  _q      :   synchronizer object
//  _k_out  :   output samples/symbol
int SYMSYNC(_set_output_rate)(SYMSYNC()    _q,
                              unsigned int _k_out)
{
    // validate input
    if (_k_out == 0)
        return liquid_error(LIQUID_EICONFIG,"symsync_%s_output_rate(), output rate must be greater than 0", EXTENSION_FULL);

    // set output rate
    _q->k_out = _k_out;

    _q->rate = (float)_q->k / (float)_q->k_out;
    _q->del  = _q->rate;
    return LIQUID_OK;
}

// set synchronizer loop filter bandwidth
//  _q      :   synchronizer object
//  _bt     :   loop bandwidth
int SYMSYNC(_set_lf_bw)(SYMSYNC() _q,
                        float     _bt)
{
    // validate input
    if (_bt < 0.0f || _bt > 1.0f)
        return liquid_error(LIQUID_EICONFIG,"symsync_%s_set_lf_bt(), bandwidth must be in [0,1]", EXTENSION_FULL);

    // compute filter coefficients from bandwidth
    float alpha = 1.000f - _bt;
    float beta  = 0.220f * _bt;
    float a     = 0.500f;
    float b     = 0.495f;

    _q->B[0] = beta;
    _q->B[1] = 0.00f;
    _q->B[2] = 0.00f;

    _q->A[0] = 1.00f - a*alpha;
    _q->A[1] = -b*alpha;
    _q->A[2] = 0.00f;

    // set internal parameters of 2nd-order IIR filter
    iirfiltsos_rrrf_set_coefficients(_q->pll, _q->B, _q->A);

    // update rate adjustment factor
    _q->rate_adjustment = 0.5*_bt;
    return LIQUID_OK;
}

// return instantaneous fractional timing offset estimate
float SYMSYNC(_get_tau)(SYMSYNC() _q)
{
    return _q->tau_decim;
}

// execute synchronizer on input data array
//  _q      : synchronizer object
//  _x      : input data array
//  _nx     : number of input samples
//  _y      : output data array
//  _ny     : number of samples written to output buffer
int SYMSYNC(_execute)(SYMSYNC()      _q,
                      TI *           _x,
                      unsigned int   _nx,
                      TO *           _y,
                      unsigned int * _ny)
{
    unsigned int i, ny=0, k=0;
    for (i=0; i<_nx; i++) {
        SYMSYNC(_step)(_q, _x[i], &_y[ny], &k);
        ny += k;
        //printf("%u\n",k);
    }
    *_ny = ny;
    return LIQUID_OK;
}

//
// internal methods
//

// step synchronizer with single input sample
//  _q      : symsync object
//  _x      : input sample
//  _y      : output sample array pointer
//  _ny     : number of output samples written
int SYMSYNC(_step)(SYMSYNC()      _q,
                   TI             _x,
                   TO *           _y,
                   unsigned int * _ny)
{
    // push sample into MF and dMF filterbanks
    FIRPFB(_push)(_q->mf,  _x);
    FIRPFB(_push)(_q->dmf, _x);

    // matched and derivative matched-filter outputs
    TO  mf; // matched filter output
    TO dmf; // derivative matched filter output

    unsigned int n=0;

    // continue loop until filterbank index rolls over
    while (_q->b < _q->npfb) {

        // compute filterbank output
        FIRPFB(_execute)(_q->mf, _q->b, &mf);

        // scale output by samples/symbol
        _y[n] = mf / (float)(_q->k);

        // check output count and determine if this is 'ideal' timing output
        if (_q->decim_counter == _q->k_out) {
            // reset counter
            _q->decim_counter = 0;

            // if synchronizer is locked, don't update internal timing offset
            if (_q->is_locked)
                continue;

            // compute dMF output
            FIRPFB(_execute)(_q->dmf, _q->b, &dmf);

            // update internal state
            SYMSYNC(_advance_internal_loop)(_q, mf, dmf);
            _q->tau_decim = _q->tau;    // save return value
        }

        // increment decimation counter
        _q->decim_counter++;

        // update states
        _q->tau += _q->del;                     // instantaneous fractional offset
        _q->bf  = _q->tau * (float)(_q->npfb);  // filterbank index (soft)
        _q->b   = (int)roundf(_q->bf);          // filterbank index
        n++;                                    // number of output samples
    }

    // filterbank index rolled over; update states
    _q->tau -= 1.0f;                // instantaneous fractional offset
    _q->bf  -= (float)(_q->npfb);   // filterbank index (soft)
    _q->b   -= _q->npfb;            // filterbank index

    // set output number of samples written
    *_ny = n;
    return LIQUID_OK;
}

// advance synchronizer's internal loop filter
//  _q      : synchronizer object
//  _mf     : matched-filter output
//  _dmf    : derivative matched-filter output
int SYMSYNC(_advance_internal_loop)(SYMSYNC() _q,
                                    TO        _mf,
                                    TO        _dmf)
{
    //  1. compute timing error signal, clipping large levels
#if 0
    _q->q = crealf(_mf)*crealf(_dmf) + cimagf(_mf)*cimagf(_dmf);
#else
    // TODO : use more efficient method to compute this
    _q->q = crealf( conjf(_mf)*_dmf );  // [Mengali:1997] Eq.~(8.3.5)
#endif
    // constrain timing error
    if      (_q->q >  1.0f) _q->q =  1.0f;  // clip large positive values
    else if (_q->q < -1.0f) _q->q = -1.0f;  // clip large negative values

    //  2. filter error signal through timing loop filter: retain large
    //     portion of old estimate and small percent of new estimate
    iirfiltsos_rrrf_execute(_q->pll, _q->q, &_q->q_hat);

    // 3. update rate and timing phase
    _q->rate += _q->rate_adjustment * _q->q_hat;
    _q->del   = _q->rate + _q->q_hat;

    return LIQUID_OK;
}

