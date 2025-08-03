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

// continuous phase frequency-shift keying demodulator

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "liquid.internal.h"

#define DEBUG_CPFSKDEM()  0

// initialize coherent demodulator
int CPFSKDEM(_init_coherent)(CPFSKDEM() _q);

// initialize non-coherent demodulator
int CPFSKDEM(_init_noncoherent)(CPFSKDEM() _q);

// demodulate array of samples (coherent)
unsigned int CPFSKDEM(_demodulate_coherent)(CPFSKDEM() _q, TC * _y);

// demodulate array of samples (non-coherent)
unsigned int CPFSKDEM(_demodulate_noncoherent)(CPFSKDEM() _q, TC * _y);

// cpfskdem
struct CPFSKDEM(_s) {
    // common
    unsigned int bps;           // bits per symbol
    unsigned int k;             // samples per symbol
    unsigned int m;             // filter delay (symbols)
    float        beta;          // filter bandwidth parameter
    float        h;             // modulation index
    int          type;          // filter type (e.g. LIQUID_CPFSK_GMSK)
    unsigned int M;             // constellation size
    unsigned int symbol_delay;  // receiver filter delay [symbols]

    // demodulator type
    enum {
        CPFSKDEM_COHERENT=0,    // coherent demodulator
        CPFSKDEM_NONCOHERENT    // non-coherent demodulator
    } demod_type;

    // demodulation function pointer
#if 0
    void (*demodulate)(CPFSKDEM()     _q,
                       TC             _y,
                       unsigned int * _s,
                       unsigned int * _nw);
#else
    unsigned int (*demodulate)(CPFSKDEM() _q, TC * _y);
#endif

    // common data structure shared between coherent and non-coherent
    // demodulator receivers
    union {
        // coherent demodulator
        struct {
            //nco_crcf nco;       // oscillator/phase-locked loop
            //firpfb_crcf mf;     // matched filter
            //firpfb_crcf dmf;    // matched filter (derivative)
            //eqlms_rrrf equalizer;
        } coherent;

        // non-coherent demodulator
        struct {
            firfilt_crcf mf;    // matched filter
        } noncoherent;
    } demod;

    // state variables
    unsigned int  index;    // debug
    unsigned int  counter;  // sample counter
    float complex z_prime;  // (coherent only)
};

// create CPFSKDEM() object (frequency demodulator)
//  _bps    :   bits per symbol, _bps > 0
//  _h      :   modulation index, _h > 0
//  _k      :   samples/symbol, _k > 1, _k even
//  _m      :   filter delay (symbols), _m > 0
//  _beta   :   filter bandwidth parameter, _beta > 0
//  _type   :   filter type (e.g. LIQUID_CPFSK_SQUARE)
CPFSKDEM() CPFSKDEM(_create)(unsigned int _bps,
                             float        _h,
                             unsigned int _k,
                             unsigned int _m,
                             float        _beta,
                             int          _type)
{
    // validate input
    if (_bps == 0)
        return liquid_error_config("cpfskdem_create(), bits/symbol must be greater than 0");
    if (_h <= 0.0f)
        return liquid_error_config("cpfskdem_create(), modulation index must be greater than 0");
    if (_k < 2 || (_k%2))
        return liquid_error_config("cpfskdem_create(), samples/symbol must be greater than 2 and even");
    if (_m == 0)
        return liquid_error_config("cpfskdem_create(), filter delay must be greater than 0");
    if (_beta <= 0.0f || _beta > 1.0f)
        return liquid_error_config("cpfskdem_create(), filter roll-off must be in (0,1]");

    switch(_type) {
    case LIQUID_CPFSK_SQUARE:
    case LIQUID_CPFSK_RCOS_FULL:
    case LIQUID_CPFSK_RCOS_PARTIAL:
    case LIQUID_CPFSK_GMSK:
        break;
    default:
        return liquid_error_config("cpfskdem_create(), invalid filter type '%d'", _type);
    }

    // create main object memory
    CPFSKDEM() q = (cpfskdem) malloc(sizeof(struct CPFSKDEM(_s)));

    // set basic internal properties
    q->bps  = _bps;     // bits per symbol
    q->h    = _h;       // modulation index
    q->k    = _k;       // samples per symbol
    q->m    = _m;       // filter delay (symbols)
    q->beta = _beta;    // filter roll-off factor (only for certain filters)
    q->type = _type;    // filter type

    // derived values
    q->M = 1 << q->bps; // constellation size

    // coherent or non-coherent?
    // TODO: allow user to specify
    if (q->h > 0.66667f) {
        //cpfskdem_init_noncoherent(q);
        fprintf(stderr,"warning: cpfskdem_create(), coherent demodulation with h > 2/3 not recommended\n");
    }
    CPFSKDEM(_init_noncoherent)(q);

    // reset modem object
    CPFSKDEM(_reset)(q);
#if DEBUG_CPFSKDEM
    printf("clear all; close all; y=[]; z=[];\n");
#endif
    return q;
}

// Copy object including all internal objects and state
CPFSKDEM() CPFSKDEM(_copy)(CPFSKDEM() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("cpfskdem_copy(), object cannot be NULL");

    // create filter object and copy base parameters
    CPFSKDEM() q_copy = (CPFSKDEM()) malloc(sizeof(struct CPFSKDEM(_s)));
    memmove(q_copy, q_orig, sizeof(struct CPFSKDEM(_s)));

    // copy objects
    if (q_orig->demod_type == CPFSKDEM_COHERENT) {
        //return liquid_error_config("cpfskdem_copy(), coherent mode not supported");
        liquid_error(LIQUID_EINT,"cpfskdem_copy(), coherent mode not supported");
        return NULL;
    } else {
        q_copy->demod.noncoherent.mf = firfilt_crcf_copy(q_orig->demod.noncoherent.mf);
    }

    // return new object
    return q_copy;
}

// create demodulator object for minimum-shift keying
//  _k      : samples/symbol, _k > 1, _k even
CPFSKDEM() CPFSKDEM(_create_msk)(unsigned int _k)
{
    return CPFSKDEM(_create)(1, 0.5f, _k, 1, 1.0f, LIQUID_CPFSK_SQUARE);
}

// create demodulator object for Gauss minimum-shift keying
//  _k      : samples/symbol, _k > 1, _k even
//  _m      : filter delay (symbols), _m > 0
//  _BT     : bandwidth-time factor, 0 < _BT < 1
CPFSKDEM() CPFSKDEM(_create_gmsk)(unsigned int _k,
                                  unsigned int _m,
                                  float        _BT)
{
    return CPFSKDEM(_create)(1, 0.5f, _k, _m, _BT, LIQUID_CPFSK_GMSK);
}


// initialize non-coherent demodulator
int CPFSKDEM(_init_coherent)(CPFSKDEM() _q)
{
    return liquid_error(LIQUID_EUMODE,"cpfskdem_init_coherent(), unsupported mode");
}

// initialize noncoherent demodulator
int CPFSKDEM(_init_noncoherent)(CPFSKDEM() _q)
{
    // specify coherent receiver
    _q->demod_type = CPFSKDEM_NONCOHERENT;

    // set demodulate function pointer
    _q->demodulate = cpfskdem_demodulate_noncoherent;

    // create object depending upon input type
    float bw = 0.0f;
    float beta = 0.0f;
    float gmsk_bt = _q->beta;
    switch(_q->type) {
    case LIQUID_CPFSK_SQUARE:
        //bw = 0.9f / (float)k;
        bw = 0.4f;
        _q->symbol_delay = _q->m;
        _q->demod.noncoherent.mf = firfilt_crcf_create_kaiser(2*_q->k*_q->m+1, bw, 60.0f, 0.0f);
        firfilt_crcf_set_scale(_q->demod.noncoherent.mf, 2.0f * bw);
        break;
    case LIQUID_CPFSK_RCOS_FULL:
        if (_q->M==2) {
            _q->demod.noncoherent.mf = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,_q->k,_q->m,0.5f,0);
            firfilt_crcf_set_scale(_q->demod.noncoherent.mf, 1.33f / (float)_q->k);
            _q->symbol_delay = _q->m;
        } else {
            _q->demod.noncoherent.mf = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,_q->k/2,2*_q->m,0.9f,0);
            firfilt_crcf_set_scale(_q->demod.noncoherent.mf, 3.25f / (float)_q->k);
            _q->symbol_delay = 0; // TODO: fix this value
        }
        break;
    case LIQUID_CPFSK_RCOS_PARTIAL:
        if (_q->M==2) {
            _q->demod.noncoherent.mf = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,_q->k,_q->m,0.3f,0);
            firfilt_crcf_set_scale(_q->demod.noncoherent.mf, 1.10f / (float)_q->k);
            _q->symbol_delay = _q->m;
        } else {
            _q->demod.noncoherent.mf = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,_q->k/2,2*_q->m,0.27f,0);
            firfilt_crcf_set_scale(_q->demod.noncoherent.mf, 2.90f / (float)_q->k);
            _q->symbol_delay = 0; // TODO: fix this value
        }
        break;
    case LIQUID_CPFSK_GMSK:
        bw = 0.5f / (float)_q->k;
        // TODO: figure out beta value here
        beta = (_q->M == 2) ? 0.8*gmsk_bt : 1.0*gmsk_bt;
        _q->demod.noncoherent.mf = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,_q->k,_q->m,beta,0);
        firfilt_crcf_set_scale(_q->demod.noncoherent.mf, 2.0f * bw);
        _q->symbol_delay = _q->m;
        break;
    default:
        return liquid_error(LIQUID_EICONFIG,"cpfskdem_init_noncoherent(), invalid tx filter type");
    }
    return LIQUID_OK;
}

// destroy modem object
int CPFSKDEM(_destroy)(CPFSKDEM() _q)
{
#if DEBUG_CPFSKDEM
    printf("figure('position',[100 100 400 400]);\n");
    printf("n=length(z); i=1:%u:n; plot(z,'-',z(i),'o');\n", _q->k);
    printf("axis square; axis([-1 1 -1 1]*1.5); grid on;\n");
#endif
    switch(_q->demod_type) {
    case CPFSKDEM_COHERENT:
        return liquid_error(LIQUID_EINT,"cpfskdem_destroy(), coherent mode not supported");
        break;
    case CPFSKDEM_NONCOHERENT:
        firfilt_crcf_destroy(_q->demod.noncoherent.mf);
        break;
    }

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print modulation internals
int CPFSKDEM(_print)(CPFSKDEM() _q)
{
    printf("<liquid.cpfskdem, bps=%u, h=%g, sps=%u, m=%u, beta=%g",
        _q->bps, _q->h, _q->k, _q->m, _q->beta);
    switch(_q->type) {
    case LIQUID_CPFSK_SQUARE:       printf(", type=\"square\"");       break;
    case LIQUID_CPFSK_RCOS_FULL:    printf(", type=\"rcos-full\"");    break;
    case LIQUID_CPFSK_RCOS_PARTIAL: printf(", type=\"rcos-partial\""); break;
    case LIQUID_CPFSK_GMSK:         printf(", type=\"gmsk\"");         break;
    default:;
    }
    printf(">\n");
    return LIQUID_OK;
}

// reset modem object
int CPFSKDEM(_reset)(CPFSKDEM() _q)
{
    switch(_q->demod_type) {
    case CPFSKDEM_COHERENT:
        firfilt_crcf_reset(_q->demod.noncoherent.mf);
        break;
    case CPFSKDEM_NONCOHERENT:
        break;
    default:
        break;
    }

    _q->index   = 0;
    _q->counter = _q->k-1;
    _q->z_prime = 0;
    return LIQUID_OK;
}

// Get demodulator's number of bits per symbol
unsigned int CPFSKDEM(_get_bits_per_symbol)(CPFSKDEM() _q)
{
    return _q->bps;
}

// Get demodulator's modulation index
float CPFSKDEM(_get_modulation_index)(CPFSKDEM() _q)
{
    return _q->h;
}

// Get demodulator's number of samples per symbol
unsigned int CPFSKDEM(_get_samples_per_symbol)(CPFSKDEM() _q)
{
    return _q->k;
}

// Get demodulator's filter delay [symbols]
unsigned int CPFSKDEM(_get_delay)(CPFSKDEM() _q)
{
    return _q->symbol_delay;
}

// Get demodulator's bandwidth parameter
float CPFSKDEM(_get_beta)(CPFSKDEM() _q)
{
    return _q->beta;
}

// Get demodulator's filter type
int CPFSKDEM(_get_type)(CPFSKDEM() _q)
{
    return _q->type;
}


#if 0
// demodulate array of samples
//  _q      :   continuous-phase frequency demodulator object
//  _y      :   input sample array [size: _n x 1]
//  _n      :   input sample array length
//  _s      :   output symbol array
//  _nw     :   number of output symbols written
int CPFSKDEM(_demodulate)(CPFSKDEM()     _q,
                          TC           * _y,
                          unsigned int   _n,
                          unsigned int * _s,
                          unsigned int * _nw)
{
    // iterate through each sample calling type-specific demodulation function
    unsigned int i;
    unsigned int num_written = 0;
    for (i=0; i<_n; i++) {
        unsigned int nw;
        _q->demodulate(_q, _y[i], &_s[num_written], &nw);

        // update number of symbols written
        num_written += nw;
    }

    // set output number of bits written
    *_nw = num_written;
    return LIQUID_OK;
}

// demodulate array of samples (coherent)
int CPFSKDEM(_demodulate_coherent)(CPFSKDEM()     _q,
                                   TC             _y,
                                   unsigned int * _s,
                                   unsigned int * _nw)
{
    // clear output counter
    *_nw = 0;

    // push input sample through filter
    firfilt_crcf_push(_q->demod.noncoherent.mf, _y);

#if DEBUG_CPFSKDEM
    // compute output sample
    float complex zp;
    firfilt_crcf_execute(_q->demod.noncoherent.mf, &zp);
    printf("y(end+1) = %12.8f + 1i*%12.8f;\n", crealf(_y), cimagf(_y));
    printf("z(end+1) = %12.8f + 1i*%12.8f;\n", crealf(zp), cimagf(zp));
#endif

    // decimate output
    _q->counter++;
    if ( (_q->counter % _q->k)==0 ) {
        // reset sample counter
        _q->counter = 0;

        // compute output sample
        float complex z;
        firfilt_crcf_execute(_q->demod.noncoherent.mf, &z);

        // compute instantaneous frequency scaled by modulation index
        // TODO: pre-compute scaling factor
        float phi_hat = cargf(conjf(_q->z_prime) * z) / (_q->h * M_PI);

        // estimate transmitted symbol
        float v = (phi_hat + (_q->M-1.0))*0.5f;
        unsigned int sym_out = ((int) roundf(v)) % _q->M;

        // save current point
        _q->z_prime = z;

#if 1
        // print result to screen
        printf("  %3u : %12.8f + j%12.8f, <f=%8.4f : %8.4f> (%1u)\n",
                _q->index++, crealf(z), cimagf(z), phi_hat, v, sym_out);
#endif

        // save output
        *_s  = sym_out;
        *_nw = 1;
    }
    return LIQUID_OK;
}

// demodulate array of samples (non-coherent)
int CPFSKDEM(_demodulate_noncoherent)(CPFSKDEM()        _q,
                                    float complex   _y,
                                    unsigned int  * _s,
                                    unsigned int  * _nw)
{
    *_nw = 0;
    return LIQUID_OK;
}

#else

// demodulate array of samples
//  _q      :   continuous-phase frequency demodulator object
//  _y      :   input sample array [size: _k x 1]
unsigned int CPFSKDEM(_demodulate)(CPFSKDEM() _q,
                                   TC *       _y)
{
    return _q->demodulate(_q, _y);
}

// demodulate array of samples (coherent)
unsigned int CPFSKDEM(_demodulate_coherent)(CPFSKDEM() _q,
                                            TC *       _y)
{
    liquid_error(LIQUID_EINT,"cpfskdem_demodulate_coherent(), coherent mode not supported");
    return 0;
}

// demodulate array of samples (non-coherent)
unsigned int CPFSKDEM(_demodulate_noncoherent)(CPFSKDEM() _q,
                                               TC *       _y)
{
    unsigned int i;
    unsigned int sym_out = 0;

    for (i=0; i<_q->k; i++) {
        // push input sample through filter
        firfilt_crcf_push(_q->demod.noncoherent.mf, _y[i]);

#if DEBUG_CPFSKDEM
        // compute output sample
        float complex zp;
        firfilt_crcf_execute(_q->demod.noncoherent.mf, &zp);
        printf("y(end+1) = %12.8f + %12.8fj;\n", crealf(_y[i]), cimagf(_y[i]));
        printf("z(end+1) = %12.8f + %12.8fj;\n", crealf(   zp), cimagf(   zp));
#endif

        // decimate output
        if ( i == 0 ) {
            // compute output sample
            float complex z;
            firfilt_crcf_execute(_q->demod.noncoherent.mf, &z);

            // compute instantaneous frequency scaled by modulation index
            // TODO: pre-compute scaling factor
            float phi_hat = cargf(conjf(_q->z_prime) * z) / (_q->h * M_PI);

            // estimate transmitted symbol
            float v = (phi_hat + (_q->M-1.0))*0.5f;
            sym_out = ((int) roundf(v)) % _q->M;

            // save current point
            _q->z_prime = z;

#if DEBUG_CPFSKDEM
            // print result to screen
            printf("%%  %3u : %12.8f + j%12.8f, <f=%8.4f : %8.4f> (%1u)\n",
                    _q->index++, crealf(z), cimagf(z), phi_hat, v, sym_out);
#endif
        }
    }
    return sym_out;
}

#endif

