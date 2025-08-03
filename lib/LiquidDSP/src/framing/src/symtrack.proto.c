/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
// Symbol tracker/synchronizer
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_SYMTRACK           0
#define DEBUG_SYMTRACK_PRINT     0
#define DEBUG_SYMTRACK_FILENAME  "symtrack_internal_debug.m"
#define DEBUG_BUFFER_LEN        (1024)

//
// forward declaration of internal methods
//

// internal structure
struct SYMTRACK(_s) {
    // parameters
    int             filter_type;        // filter type (e.g. LIQUID_FIRFILT_RRC)
    unsigned int    k;                  // samples/symbol
    unsigned int    m;                  // filter semi-length
    float           beta;               // filter excess bandwidth
    int             mod_scheme;         // demodulator
    float           bw;                 // tracking bandwidth

    // automatic gain control
    AGC()           agc;                // agc object
    float           agc_bandwidth;      // agc bandwidth

    // symbol timing recovery
    SYMSYNC()       symsync;            // symbol timing recovery object
    float           symsync_bandwidth;  // symsync loop bandwidth
    TO              symsync_buf[8];     // symsync output buffer
    unsigned int    symsync_index;      // symsync output sample index

    // equalizer/decimator
    EQLMS()         eq;                 // equalizer (LMS)
    unsigned int    eq_len;             // equalizer length
    float           eq_bandwidth;       // equalizer bandwidth
    enum {
                    SYMTRACK_EQ_CM,     // equalizer strategy: constant modulus
                    SYMTRACK_EQ_DD,     // equalizer strategy: decision directed
                    SYMTRACK_EQ_OFF,    // equalizer strategy: disabled
    }               eq_strategy;

    // nco/phase-locked loop
    NCO()           nco;                // nco (carrier recovery)
    float           pll_bandwidth;      // phase-locked loop bandwidth

    // demodulator
    MODEM()         demod;              // linear modem demodulator

    // state and counters
    unsigned int    num_syms_rx;        // number of symbols recovered
};

// create symtrack object with basic parameters
//  _ftype  : filter type (e.g. LIQUID_FIRFILT_RRC)
//  _k      : samples per symbol
//  _m      : filter delay (symbols)
//  _beta   : filter excess bandwidth
//  _ms     : modulation scheme (e.g. LIQUID_MODEM_QPSK)
SYMTRACK() SYMTRACK(_create)(int          _ftype,
                             unsigned int _k,
                             unsigned int _m,
                             float        _beta,
                             int          _ms)
{
    // validate input
    if (_k < 2)
        return liquid_error_config("symtrack_%s_create(), filter samples/symbol must be at least 2", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("symtrack_%s_create(), filter delay must be greater than zero", EXTENSION_FULL);
    if (_beta <= 0.0f || _beta > 1.0f)
        return liquid_error_config("symtrack_%s_create(), filter excess bandwidth must be in (0,1]", EXTENSION_FULL);
    if (_ms == LIQUID_MODEM_UNKNOWN || _ms >= LIQUID_MODEM_NUM_SCHEMES)
        return liquid_error_config("symtrack_%s_create(), invalid modulation scheme", EXTENSION_FULL);

    // allocate memory for main object
    SYMTRACK() q = (SYMTRACK()) malloc( sizeof(struct SYMTRACK(_s)) );

    // set input parameters
    q->filter_type = _ftype;
    q->k           = _k;
    q->m           = _m;
    q->beta        = _beta;
    q->mod_scheme  = _ms == LIQUID_MODEM_UNKNOWN ? LIQUID_MODEM_BPSK : _ms;

    // create automatic gain control
    q->agc = AGC(_create)();
    
    // create symbol synchronizer (output rate: 2 samples per symbol)
    if (q->filter_type == LIQUID_FIRFILT_UNKNOWN)
        q->symsync = SYMSYNC(_create_kaiser)(q->k, q->m, 0.9f, 16);
    else
        q->symsync = SYMSYNC(_create_rnyquist)(q->filter_type, q->k, q->m, q->beta, 16);
    SYMSYNC(_set_output_rate)(q->symsync, 2);

    // create equalizer as default low-pass filter with integer symbol delay (2 samples/symbol)
    q->eq_len = 2 * 4 + 1;
    q->eq = EQLMS(_create_lowpass)(q->eq_len,0.45f);
    q->eq_strategy = SYMTRACK_EQ_CM;

    // nco and phase-locked loop
    q->nco = NCO(_create)(LIQUID_VCO);

    // demodulator
    q->demod = MODEM(_create)(q->mod_scheme);

    // set default bandwidth
    SYMTRACK(_set_bandwidth)(q, 0.9f);

    // reset and return main object
    SYMTRACK(_reset)(q);
    return q;
}

// create symtrack object using default parameters
SYMTRACK() SYMTRACK(_create_default)()
{
    return SYMTRACK(_create)(LIQUID_FIRFILT_ARKAISER,
                             2,     // samples/symbol
                             7,     // filter delay
                             0.3f,  // filter excess bandwidth
                             LIQUID_MODEM_QPSK);
}


// destroy symtrack object, freeing all internal memory
int SYMTRACK(_destroy)(SYMTRACK() _q)
{
    // destroy objects
    AGC    (_destroy)(_q->agc);
    SYMSYNC(_destroy)(_q->symsync);
    EQLMS  (_destroy)(_q->eq);
    NCO    (_destroy)(_q->nco);
    MODEM  (_destroy)(_q->demod);

    // free main object
    free(_q);
    return LIQUID_OK;
}

// print symtrack object's parameters
int SYMTRACK(_print)(SYMTRACK() _q)
{
#if 0
    printf("symtrack_%s:\n", EXTENSION_FULL);
    printf("  k:%u, m:%u, beta:%.3f, ms:%s\n", _q->k, _q->m, _q->beta,
            modulation_types[_q->mod_scheme].name);
    printf("  equalization strategy: ");
    switch (_q->eq_strategy) {
    case SYMTRACK_EQ_CM:  printf("constant modulus\n");  break;
    case SYMTRACK_EQ_DD:  printf("decision directed\n"); break;
    case SYMTRACK_EQ_OFF: printf("disabled\n");          break;
    default:
        printf("?\n");
        return liquid_error(LIQUID_EINT,"symtrack_%s_print(), invalid equalization strategy");
    }
#else
    printf("<liquid.symtrack_%s\n", EXTENSION_FULL);
    printf(", k=%u, m=%u, beta=%.3f, ms\"%s\">\n",
        _q->k, _q->m, _q->beta, modulation_types[_q->mod_scheme].name);
#endif
    return LIQUID_OK;
}

// reset symtrack internal state
int SYMTRACK(_reset)(SYMTRACK() _q)
{
    // reset objects
    AGC    (_reset)(_q->agc);
    SYMSYNC(_reset)(_q->symsync);
    EQLMS  (_reset)(_q->eq);
    NCO    (_reset)(_q->nco);
    MODEM  (_reset)(_q->demod);

    // reset internal counters
    _q->symsync_index = 0;
    _q->num_syms_rx = 0;
    return LIQUID_OK;
}

// get symtrack filter type
int SYMTRACK(_get_ftype)(SYMTRACK() _q)
{
    return _q->filter_type;
}

// get symtrack samples per symbol
unsigned int SYMTRACK(_get_k)(SYMTRACK() _q)
{
    return _q->k;
}

// get symtrack filter semi-length [symbols]
unsigned int SYMTRACK(_get_m)(SYMTRACK() _q)
{
    return _q->m;
}

// get symtrack filter excess bandwidth factor
float SYMTRACK(_get_beta)(SYMTRACK() _q)
{
    return _q->beta;
}

// get symtrack modulation scheme
int SYMTRACK(_get_modscheme)(SYMTRACK() _q)
{
    return _q->mod_scheme;
}

// set symtrack modulation scheme
int SYMTRACK(_set_modscheme)(SYMTRACK() _q,
                             int        _ms)
{
    // validate input
    if (_ms >= LIQUID_MODEM_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"symtrack_%s_set_modscheme(), invalid/unsupported modulation scheme", EXTENSION_FULL);

    // set internal modulation scheme
    _q->mod_scheme = _ms == LIQUID_MODEM_UNKNOWN ? LIQUID_MODEM_BPSK : _ms;

    // re-create modem
    _q->demod = MODEM(_recreate)(_q->demod, _q->mod_scheme);
    return LIQUID_OK;
}

// set symtrack internal bandwidth
int SYMTRACK(_set_bandwidth)(SYMTRACK() _q,
                             float      _bw)
{
    // validate input
    if (_bw < 0)
        return liquid_error(LIQUID_EICONFIG,"symtrack_%s_set_bandwidth(), bandwidth must be in [0,1]", EXTENSION_FULL);

    _q->bw = _bw;

    // set bandwidths accordingly
    float agc_bandwidth     = 0.02f  * _q->bw;
    float symsync_bandwidth = 0.001f * _q->bw;
    float eq_bandwidth      = 0.02f  * _q->bw;
    float pll_bandwidth     = 0.001f * _q->bw;

    // automatic gain control
    AGC(_set_bandwidth)(_q->agc, agc_bandwidth);

    // symbol timing recovery
    SYMSYNC(_set_lf_bw)(_q->symsync, symsync_bandwidth);

    // equalizer
    EQLMS(_set_bw)(_q->eq, eq_bandwidth);
    
    // phase-locked loop
    NCO(_pll_set_bandwidth)(_q->nco, pll_bandwidth);
    return LIQUID_OK;
}

// get symtrack internal bandwidth
float SYMTRACK(_get_bandwidth)(SYMTRACK() _q)
{
    return _q->bw;
}

// adjust internal nco by requested frequency
int SYMTRACK(_adjust_frequency)(SYMTRACK() _q,
                                T          _dphi)
{
    return NCO(_adjust_frequency)(_q->nco, _dphi);
}

// adjust internal nco by requested phase
int SYMTRACK(_adjust_phase)(SYMTRACK() _q,
                            T          _phi)
{
    return NCO(_adjust_phase)(_q->nco, _phi);
}

// set equalization strategy to constant modulus (default)
int SYMTRACK(_set_eq_cm)(SYMTRACK() _q)
{
    _q->eq_strategy = SYMTRACK_EQ_CM;
    return LIQUID_OK;
}

// set equalization strategy to decision directed
int SYMTRACK(_set_eq_dd)(SYMTRACK() _q)
{
    _q->eq_strategy = SYMTRACK_EQ_DD;
    return LIQUID_OK;
}

// disable equalization
int SYMTRACK(_set_eq_off)(SYMTRACK() _q)
{
    _q->eq_strategy = SYMTRACK_EQ_OFF;
    return LIQUID_OK;
}

// execute synchronizer on single input sample
//  _q      : synchronizer object
//  _x      : input data sample
//  _y      : output data array
//  _ny     : number of samples written to output buffer
int SYMTRACK(_execute)(SYMTRACK()     _q,
                       TI             _x,
                       TO *           _y,
                       unsigned int * _ny)
{
    TO v;   // output sample
    unsigned int i;
    unsigned int num_outputs = 0;

    // run sample through automatic gain control
    AGC(_execute)(_q->agc, _x, &v);

    // symbol synchronizer
    unsigned int nw = 0;
    SYMSYNC(_execute)(_q->symsync, &v, 1, _q->symsync_buf, &nw);

    // process each output sample
    for (i=0; i<nw; i++) {
        // update phase-locked loop
        NCO(_step)(_q->nco);
        nco_crcf_mix_down(_q->nco, _q->symsync_buf[i], &v);

        // equalizer/decimator
        EQLMS(_push)(_q->eq, v);

        // decimate result, noting that symsync outputs at exactly 2 samples/symbol
        _q->symsync_index++;
        if ( !(_q->symsync_index % 2) )
            continue;

        // increment number of symbols received
        _q->num_syms_rx++;

        // compute equalizer filter output; updating coefficients is dependent upon strategy
        TO d_hat;
        EQLMS(_execute)(_q->eq, &d_hat);

        // demodulate result, apply phase correction
        unsigned int sym_out;
        MODEM(_demodulate)(_q->demod, d_hat, &sym_out);
        float phase_error = MODEM(_get_demodulator_phase_error)(_q->demod);

        // update pll
        NCO(_pll_step)(_q->nco, phase_error);

        // update equalizer independent of the signal: estimate error
        // assuming constant modulus signal
        // TODO: check lock conditions of previous object to determine when to run equalizer
        float complex d_prime = 0.0f;
        if (_q->num_syms_rx > 200 && _q->eq_strategy != SYMTRACK_EQ_OFF) {
            switch (_q->eq_strategy) {
            case SYMTRACK_EQ_CM: d_prime = d_hat/cabsf(d_hat); break;
            case SYMTRACK_EQ_DD: MODEM(_get_demodulator_sample)(_q->demod, &d_prime); break;
            default:
                return liquid_error(LIQUID_EINT,"symtrack_%s_execute(), invalid equalizer strategy", EXTENSION_FULL);
            }
            EQLMS(_step)(_q->eq, d_prime, d_hat);
        }

        // save result to output
        _y[num_outputs++] = d_hat;
    }

#if DEBUG_SYMTRACK
    printf("symsync wrote %u samples, %u outputs\n", nw, num_outputs);
#endif

    //
    *_ny = num_outputs;
    return LIQUID_OK;
}

// execute synchronizer on input data array
//  _q      : synchronizer object
//  _x      : input data array
//  _nx     : number of input samples
//  _y      : output data array
//  _ny     : number of samples written to output buffer
int SYMTRACK(_execute_block)(SYMTRACK()     _q,
                             TI *           _x,
                             unsigned int   _nx,
                             TO *           _y,
                             unsigned int * _ny)
{
    //
    unsigned int i;
    unsigned int num_written = 0;

    //
    for (i=0; i<_nx; i++) {
        unsigned int nw = 0;
        SYMTRACK(_execute)(_q, _x[i], &_y[num_written], &nw);

        num_written += nw;
    }

    //
    *_ny = num_written;
    return LIQUID_OK;
}

