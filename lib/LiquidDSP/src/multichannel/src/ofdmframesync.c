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

// OFDM frame synchronizer

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_OFDMFRAMESYNC             0
#define DEBUG_OFDMFRAMESYNC_PRINT       0
#define DEBUG_OFDMFRAMESYNC_FILENAME    "ofdmframesync_internal_debug.m"
#define DEBUG_OFDMFRAMESYNC_BUFFER_LEN  (2048)

#define OFDMFRAMESYNC_ENABLE_SQUELCH    0

// forward declaration of internal methods

int ofdmframesync_execute_seekplcp(ofdmframesync _q);
int ofdmframesync_execute_S0a(ofdmframesync _q);
int ofdmframesync_execute_S0b(ofdmframesync _q);
int ofdmframesync_execute_S1( ofdmframesync _q);
int ofdmframesync_execute_rxsymbols(ofdmframesync _q);

int ofdmframesync_S0_metrics(ofdmframesync   _q,
                             float complex * _G,
                             float complex * _s_hat);

// estimate short sequence gain
//  _q      :   ofdmframesync object
//  _x      :   input array (time)
//  _G      :   output gain (freq)
int ofdmframesync_estimate_gain_S0(ofdmframesync   _q,
                                   float complex * _x,
                                   float complex * _G);

// estimate long sequence gain
//  _q      :   ofdmframesync object
//  _x      :   input array (time)
//  _G      :   output gain (freq)
int ofdmframesync_estimate_gain_S1(ofdmframesync _q,
                                   float complex * _x,
                                   float complex * _G);

// estimate complex equalizer gain from G0 and G1
//  _q      :   ofdmframesync object
//  _ntaps  :   number of time-domain taps for smoothing
//int ofdmframesync_estimate_eqgain(ofdmframesync _q, unsigned int _ntaps);

// estimate complex equalizer gain from G0 and G1 using polynomial fit
//  _q      :   ofdmframesync object
//  _order  :   polynomial order
int ofdmframesync_estimate_eqgain_poly(ofdmframesync _q,
                                       unsigned int _order);

// recover symbol, correcting for gain, pilot phase, etc.
int ofdmframesync_rxsymbol(ofdmframesync _q);

struct ofdmframesync_s {
    unsigned int M;         // number of subcarriers
    unsigned int M2;        // number of subcarriers (divided by 2)
    unsigned int cp_len;    // cyclic prefix length
    unsigned char * p;      // subcarrier allocation (null, pilot, data)
    unsigned int taper_len; // number of samples in tapering window/overlap

    // constants
    unsigned int M_null;    // number of null subcarriers
    unsigned int M_pilot;   // number of pilot subcarriers
    unsigned int M_data;    // number of data subcarriers
    unsigned int M_S0;      // number of enabled subcarriers in S0
    unsigned int M_S1;      // number of enabled subcarriers in S1

    // scaling factors
    float g_data;           // data symbols gain
    float g_S0;             // S0 training symbols gain
    float g_S1;             // S1 training symbols gain

    // transform object
    FFT_PLAN fft;           // ifft object
    float complex * X;      // frequency-domain buffer
    float complex * x;      // time-domain buffer
    windowcf input_buffer;  // input sequence buffer

    // PLCP sequences
    float complex * S0;     // short sequence (freq)
    float complex * s0;     // short sequence (time)
    float complex * S1;     // long sequence (freq)
    float complex * s1;     // long sequence (time)

    // gain
    float g0;               // nominal gain (coarse initial estimate)
    float complex * G0a;    // complex subcarrier gain estimate, S0[a]
    float complex * G0b;    // complex subcarrier gain estimate, S0[b]
    float complex * G1;     // complex subcarrier gain estimate, S1
    float complex * G;      // complex subcarrier gain estimate
    float complex * B;      // subcarrier phase rotation due to backoff
    float complex * R;      // 

    // receiver state
    enum {
        OFDMFRAMESYNC_STATE_SEEKPLCP=0,   // seek initial PLCP
        OFDMFRAMESYNC_STATE_PLCPSHORT0,   // seek first PLCP short sequence
        OFDMFRAMESYNC_STATE_PLCPSHORT1,   // seek second PLCP short sequence
        OFDMFRAMESYNC_STATE_PLCPLONG,     // seek PLCP long sequence
        OFDMFRAMESYNC_STATE_RXSYMBOLS     // receive payload symbols
    } state;

    // synchronizer objects
    nco_crcf nco_rx;        // numerically-controlled oscillator
    msequence ms_pilot;     // pilot sequence generator
    float phi_prime;        // ...
    float p1_prime;         // filtered pilot phase slope

#if OFDMFRAMESYNC_ENABLE_SQUELCH
    // coarse signal detection
    float squelch_threshold;
    int squelch_enabled;
#endif

    // timing
    unsigned int timer;         // input sample timer
    unsigned int num_symbols;   // symbol counter
    unsigned int backoff;       // sample timing backoff
    float complex s_hat_0;      // first S0 symbol metrics estimate
    float complex s_hat_1;      // second S0 symbol metrics estimate

    // detection thresholds
    float plcp_detect_thresh;   // plcp detection threshold, nominally 0.35
    float plcp_sync_thresh;     // long symbol threshold, nominally 0.30

    // callback
    ofdmframesync_callback callback;
    void * userdata;

#if DEBUG_OFDMFRAMESYNC
    int debug_enabled;
    int debug_objects_created;
    windowcf debug_x;
    windowf  debug_rssi;
    windowcf debug_framesyms;
    float complex * G_hat;  // complex subcarrier gain estimate, S1
    float * px;             // pilot x-value
    float * py;             // pilot y-value
    float p_phase[2];       // pilot polyfit
    windowf debug_pilot_0;  // pilot polyfit history, p[0]
    windowf debug_pilot_1;  // pilot polyfit history, p[1]
#endif
};

// create OFDM framing synchronizer object
//  _M          :   number of subcarriers, >10 typical
//  _cp_len     :   cyclic prefix length
//  _taper_len  :   taper length (OFDM symbol overlap), TODO: remove? not used in this object
//  _p          :   subcarrier allocation (null, pilot, data), [size: _M x 1]
//  _callback   :   user-defined callback function
//  _userdata   :   user-defined data pointer
ofdmframesync ofdmframesync_create(unsigned int           _M,
                                   unsigned int           _cp_len,
                                   unsigned int           _taper_len,
                                   unsigned char *        _p,
                                   ofdmframesync_callback _callback,
                                   void *                 _userdata)
{
    // validate input
    if (_M < 8)
        return liquid_error_config("ofdmframesync_create(), number of subcarriers must be at least 8");
    if (_M % 2)
        return liquid_error_config("ofdmframesync_create(), number of subcarriers must be even");
    if (_cp_len > _M)
        return liquid_error_config("ofdmframesync_create(), cyclic prefix length cannot exceed number of subcarriers");
    if (_taper_len > _cp_len)
        return liquid_error_config("ofdmframesync_create(), taper length cannot exceed cyclic prefix");

    // allocate object and set parameters
    ofdmframesync q = (ofdmframesync) malloc(sizeof(struct ofdmframesync_s));
    q->M = _M;
    q->cp_len = _cp_len;
    q->taper_len = _taper_len;

    // derived values
    q->M2 = _M/2;

    // subcarrier allocation
    q->p = (unsigned char*) malloc((q->M)*sizeof(unsigned char));
    if (_p == NULL) {
        ofdmframe_init_default_sctype(q->M, q->p);
    } else {
        memmove(q->p, _p, q->M*sizeof(unsigned char));
    }

    // validate and count subcarrier allocation
    if (ofdmframe_validate_sctype(q->p, q->M, &q->M_null, &q->M_pilot, &q->M_data))
        return liquid_error_config("ofdmframesync_create(), invalid subcarrier allocation");

    // create transform object
    q->X = (float complex*) FFT_MALLOC((q->M)*sizeof(float complex));
    q->x = (float complex*) FFT_MALLOC((q->M)*sizeof(float complex));
    q->fft = FFT_CREATE_PLAN(q->M, q->x, q->X, FFT_DIR_FORWARD, FFT_METHOD);
 
    // create input buffer the length of the transform
    q->input_buffer = windowcf_create(q->M + q->cp_len);

    // allocate memory for PLCP arrays
    q->S0 = (float complex*) malloc((q->M)*sizeof(float complex));
    q->s0 = (float complex*) malloc((q->M)*sizeof(float complex));
    q->S1 = (float complex*) malloc((q->M)*sizeof(float complex));
    q->s1 = (float complex*) malloc((q->M)*sizeof(float complex));
    ofdmframe_init_S0(q->p, q->M, q->S0, q->s0, &q->M_S0);
    ofdmframe_init_S1(q->p, q->M, q->S1, q->s1, &q->M_S1);

    // compute scaling factor
    q->g_data = sqrtf(q->M) / sqrtf(q->M_pilot + q->M_data);
    q->g_S0   = sqrtf(q->M) / sqrtf(q->M_S0);
    q->g_S1   = sqrtf(q->M) / sqrtf(q->M_S1);

    // gain
    q->g0 = 1.0f;
    q->G0a = (float complex*) malloc((q->M)*sizeof(float complex));
    q->G0b = (float complex*) malloc((q->M)*sizeof(float complex));
    q->G   = (float complex*) malloc((q->M)*sizeof(float complex));
    q->B   = (float complex*) malloc((q->M)*sizeof(float complex));
    q->R   = (float complex*) malloc((q->M)*sizeof(float complex));

#if 1
    memset(q->G0a, 0x00, q->M*sizeof(float complex));
    memset(q->G0b, 0x00, q->M*sizeof(float complex));
    memset(q->G ,  0x00, q->M*sizeof(float complex));
    memset(q->B,   0x00, q->M*sizeof(float complex));
#endif

    // timing backoff
    q->backoff = q->cp_len < 2 ? q->cp_len : 2;
    float phi = (float)(q->backoff)*2.0f*M_PI/(float)(q->M);
    unsigned int i;
    for (i=0; i<q->M; i++)
        q->B[i] = liquid_cexpjf(i*phi);

    // set callback data
    q->callback = _callback;
    q->userdata = _userdata;

    // 
    // synchronizer objects
    //

    // numerically-controlled oscillator
    q->nco_rx = nco_crcf_create(LIQUID_NCO);

    // set pilot sequence
    q->ms_pilot = msequence_create_default(8);

#if OFDMFRAMESYNC_ENABLE_SQUELCH
    // coarse detection
    q->squelch_threshold = -25.0f;
    q->squelch_enabled = 0;
#endif

    // reset object
    ofdmframesync_reset(q);

#if DEBUG_OFDMFRAMESYNC
    q->debug_enabled = 0;
    q->debug_objects_created = 0;

    q->debug_x =        NULL;
    q->debug_rssi =     NULL;
    q->debug_framesyms =NULL;
    
    q->G_hat = NULL;
    q->px    = NULL;
    q->py    = NULL;
    
    q->debug_pilot_0 = NULL;
    q->debug_pilot_1 = NULL;
#endif

    // return object
    return q;
}

int ofdmframesync_destroy(ofdmframesync _q)
{
#if DEBUG_OFDMFRAMESYNC
    // destroy debugging objects
    if (_q->debug_x         != NULL) windowcf_destroy(_q->debug_x);
    if (_q->debug_rssi      != NULL) windowf_destroy(_q->debug_rssi);
    if (_q->debug_framesyms != NULL) windowcf_destroy(_q->debug_framesyms);
    if (_q->G_hat           != NULL) free(_q->G_hat);
    if (_q->px              != NULL) free(_q->px);
    if (_q->py              != NULL) free(_q->py);
    if (_q->debug_pilot_0   != NULL) windowf_destroy(_q->debug_pilot_0);
    if (_q->debug_pilot_1   != NULL) windowf_destroy(_q->debug_pilot_1);
#endif

    // free subcarrier type array memory
    free(_q->p);

    // free transform object
    windowcf_destroy(_q->input_buffer);
    FFT_FREE(_q->X);
    FFT_FREE(_q->x);
    FFT_DESTROY_PLAN(_q->fft);

    // clean up PLCP arrays
    free(_q->S0);
    free(_q->s0);
    free(_q->S1);
    free(_q->s1);

    // free gain arrays
    free(_q->G0a);
    free(_q->G0b);
    free(_q->G);
    free(_q->B);
    free(_q->R);

    // destroy synchronizer objects
    nco_crcf_destroy(_q->nco_rx);           // numerically-controlled oscillator
    msequence_destroy(_q->ms_pilot);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

int ofdmframesync_print(ofdmframesync _q)
{
    printf("<liquid.ofdmframesync");
    printf(", subcarriers=%u", _q->M);
    printf(", null=%u", _q->M_null);
    printf(", pilot=%u", _q->M_pilot);
    printf(", data=%u", _q->M_data);
    printf(", cp=%u", _q->cp_len);
    printf(", taper=%u", _q->taper_len);
    printf(">\n");
    return LIQUID_OK;
}

int ofdmframesync_reset(ofdmframesync _q)
{
#if 0
    // reset gain parameters
    unsigned int i;
    for (i=0; i<_q->M; i++)
        _q->G[i] = 1.0f;
#endif

    // reset synchronizer objects
    nco_crcf_reset(_q->nco_rx);
    msequence_reset(_q->ms_pilot);

    // reset timers
    _q->timer = 0;
    _q->num_symbols = 0;
    _q->s_hat_0 = 0.0f;
    _q->s_hat_1 = 0.0f;
    _q->phi_prime = 0.0f;
    _q->p1_prime = 0.0f;

    // set thresholds (increase for small number of subcarriers)
    _q->plcp_detect_thresh = (_q->M > 44) ? 0.35f : 0.35f + 0.01f*(44 - _q->M);
    _q->plcp_sync_thresh   = (_q->M > 44) ? 0.30f : 0.30f + 0.01f*(44 - _q->M);

    // reset state
    _q->state = OFDMFRAMESYNC_STATE_SEEKPLCP;
    return LIQUID_OK;
}

int ofdmframesync_is_frame_open(ofdmframesync _q)
{
    return (_q->state == OFDMFRAMESYNC_STATE_SEEKPLCP) ? 0 : 1;
}

int ofdmframesync_execute(ofdmframesync   _q,
                          float complex * _x,
                          unsigned int    _n)
{
    unsigned int i;
    float complex x;
    for (i=0; i<_n; i++) {
        x = _x[i];

        // correct for carrier frequency offset
        if (_q->state != OFDMFRAMESYNC_STATE_SEEKPLCP) {
            nco_crcf_mix_down(_q->nco_rx, x, &x);
            nco_crcf_step(_q->nco_rx);
        }

        // save input sample to buffer
        windowcf_push(_q->input_buffer,x);

#if DEBUG_OFDMFRAMESYNC
        if (_q->debug_enabled) {
            windowcf_push(_q->debug_x, x);
            windowf_push(_q->debug_rssi, crealf(x)*crealf(x) + cimagf(x)*cimagf(x));
        }
#endif

        switch (_q->state) {
        case OFDMFRAMESYNC_STATE_SEEKPLCP:
            ofdmframesync_execute_seekplcp(_q);
            break;
        case OFDMFRAMESYNC_STATE_PLCPSHORT0:
            ofdmframesync_execute_S0a(_q);
            break;
        case OFDMFRAMESYNC_STATE_PLCPSHORT1:
            ofdmframesync_execute_S0b(_q);
            break;
        case OFDMFRAMESYNC_STATE_PLCPLONG:
            ofdmframesync_execute_S1(_q);
            break;
        case OFDMFRAMESYNC_STATE_RXSYMBOLS:
            ofdmframesync_execute_rxsymbols(_q);
            break;
        default:;
        }

    } // for (i=0; i<_n; i++)
    return LIQUID_OK;
} // ofdmframesync_execute()

// get receiver RSSI
float ofdmframesync_get_rssi(ofdmframesync _q)
{
    return -10.0f*log10(_q->g0);
}

// get receiver carrier frequency offset estimate
float ofdmframesync_get_cfo(ofdmframesync _q)
{
    return nco_crcf_get_frequency(_q->nco_rx);
}

// set receiver carrier frequency offset estimate
int ofdmframesync_set_cfo(ofdmframesync _q, float _cfo)
{
    return nco_crcf_set_frequency(_q->nco_rx, _cfo);
}

//
// internal methods
//

// frame detection
int ofdmframesync_execute_seekplcp(ofdmframesync _q)
{
    _q->timer++;

    if (_q->timer < _q->M)
        return LIQUID_OK;

    // reset timer
    _q->timer = 0;

    //
    float complex * rc;
    windowcf_read(_q->input_buffer, &rc);

    // estimate gain
    unsigned int i;
    // start with a reasonably small number to avoid division by zero
    float g = 1.0e-9f;
    for (i=_q->cp_len; i<_q->M + _q->cp_len; i++) {
        // compute |rc[i]|^2 efficiently
        g += crealf(rc[i])*crealf(rc[i]) + cimagf(rc[i])*cimagf(rc[i]);
    }
    g = (float)(_q->M) / g;

#if OFDMFRAMESYNC_ENABLE_SQUELCH
    // TODO : squelch here
    if ( -10*log10f( sqrtf(g) ) < _q->squelch_threshold &&
         _q->squelch_enabled)
    {
        printf("squelch\n");
        return LIQUID_OK;
    }
#endif

    // estimate S0 gain
    ofdmframesync_estimate_gain_S0(_q, &rc[_q->cp_len], _q->G0a);

    float complex s_hat;
    ofdmframesync_S0_metrics(_q, _q->G0a, &s_hat);
    s_hat *= g;

    float tau_hat  = cargf(s_hat) * (float)(_q->M2) / (2*M_PI);
#if DEBUG_OFDMFRAMESYNC_PRINT
    printf(" - gain=%12.3f, rssi=%12.8f, s_hat=%12.4f <%12.8f>, tau_hat=%8.3f\n",
            sqrt(g),
            -10*log10(g),
            cabsf(s_hat), cargf(s_hat),
            tau_hat);
#endif

    // save gain (permits dynamic invocation of get_rssi() method)
    _q->g0 = g;

    // 
    if (cabsf(s_hat) > _q->plcp_detect_thresh) {

        int dt = (int)roundf(tau_hat);
        // set timer appropriately...
        _q->timer = (_q->M + dt) % (_q->M2);
        _q->timer += _q->M; // add delay to help ensure good S0 estimate
        _q->state = OFDMFRAMESYNC_STATE_PLCPSHORT0;

#if DEBUG_OFDMFRAMESYNC_PRINT
        printf("********** frame detected! ************\n");
        printf("    s_hat   :   %12.8f <%12.8f>\n", cabsf(s_hat), cargf(s_hat));
        printf("  tau_hat   :   %12.8f\n", tau_hat);
        printf("    dt      :   %12d\n", dt);
        printf("    timer   :   %12u\n", _q->timer);
#endif
    }
    return LIQUID_OK;
}

// frame detection
int ofdmframesync_execute_S0a(ofdmframesync _q)
{
    //printf("t : %u\n", _q->timer);
    _q->timer++;

    if (_q->timer < _q->M2)
        return LIQUID_OK;

    // reset timer
    _q->timer = 0;

    //
    float complex * rc;
    windowcf_read(_q->input_buffer, &rc);

    // TODO : re-estimate nominal gain

    // estimate S0 gain
    ofdmframesync_estimate_gain_S0(_q, &rc[_q->cp_len], _q->G0a);

    float complex s_hat;
    ofdmframesync_S0_metrics(_q, _q->G0a, &s_hat);
    s_hat *= _q->g0;

    _q->s_hat_0 = s_hat;

#if DEBUG_OFDMFRAMESYNC_PRINT
    float tau_hat  = cargf(s_hat) * (float)(_q->M2) / (2*M_PI);
    printf("********** S0[0] received ************\n");
    printf("    s_hat   :   %12.8f <%12.8f>\n", cabsf(s_hat), cargf(s_hat));
    printf("  tau_hat   :   %12.8f\n", tau_hat);
#endif

#if 0
    // TODO : also check for phase of s_hat (should be small)
    if (cabsf(s_hat) < 0.3f) {
        // false alarm
#if DEBUG_OFDMFRAMESYNC_PRINT
        printf("false alarm S0[0]\n");
#endif
        ofdmframesync_reset(_q);
        return;
    }
#endif
    _q->state = OFDMFRAMESYNC_STATE_PLCPSHORT1;
    return LIQUID_OK;
}

// frame detection
int ofdmframesync_execute_S0b(ofdmframesync _q)
{
    //printf("t = %u\n", _q->timer);
    _q->timer++;

    if (_q->timer < _q->M2)
        return LIQUID_OK;

    // reset timer
    _q->timer = _q->M + _q->cp_len - _q->backoff;

    //
    float complex * rc;
    windowcf_read(_q->input_buffer, &rc);

    // estimate S0 gain
    ofdmframesync_estimate_gain_S0(_q, &rc[_q->cp_len], _q->G0b);

    float complex s_hat;
    ofdmframesync_S0_metrics(_q, _q->G0b, &s_hat);
    s_hat *= _q->g0;

    _q->s_hat_1 = s_hat;

#if DEBUG_OFDMFRAMESYNC_PRINT
    float tau_hat  = cargf(s_hat) * (float)(_q->M2) / (2*M_PI);
    printf("********** S0[1] received ************\n");
    printf("    s_hat   :   %12.8f <%12.8f>\n", cabsf(s_hat), cargf(s_hat));
    printf("  tau_hat   :   %12.8f\n", tau_hat);

    // new timing offset estimate
    tau_hat  = cargf(_q->s_hat_0 + _q->s_hat_1) * (float)(_q->M2) / (2*M_PI);
    printf("  tau_hat * :   %12.8f\n", tau_hat);

    printf("**********\n");
#endif

    // re-adjust timer accordingly
    float tau_prime = cargf(_q->s_hat_0 + _q->s_hat_1) * (float)(_q->M2) / (2*M_PI);
    _q->timer -= (int)roundf(tau_prime);

#if 0
    if (cabsf(s_hat) < 0.3f) {
#if DEBUG_OFDMFRAMESYNC_PRINT
        printf("false alarm S0[1]\n");
#endif
        // false alarm
        ofdmframesync_reset(_q);
        return;
    }
#endif

    unsigned int i;
#if 0
    float complex g_hat = 0.0f;
    for (i=0; i<_q->M; i++)
        g_hat += _q->G0b[i] * conjf(_q->G0a[i]);

    // compute carrier frequency offset estimate using freq. domain method
    float nu_hat = 2.0f * cargf(g_hat) / (float)(_q->M);
#else
    // compute carrier frequency offset estimate using ML method
    float complex t0 = 0.0f;
    for (i=0; i<_q->M2; i++) {
        t0 += conjf(rc[i])       *       _q->s0[i] * 
                    rc[i+_q->M2] * conjf(_q->s0[i+_q->M2]);
    }
    float nu_hat = cargf(t0) / (float)(_q->M2);
#endif

#if DEBUG_OFDMFRAMESYNC_PRINT
    printf("   nu_hat   :   %12.8f\n", nu_hat);
#endif

    // set NCO frequency
    nco_crcf_set_frequency(_q->nco_rx, nu_hat);

    _q->state = OFDMFRAMESYNC_STATE_PLCPLONG;
    return LIQUID_OK;
}

int ofdmframesync_execute_S1(ofdmframesync _q)
{
    _q->timer--;

    if (_q->timer > 0)
        return LIQUID_OK;

    // increment number of symbols observed
    _q->num_symbols++;

    // run fft
    float complex * rc;
    windowcf_read(_q->input_buffer, &rc);

    // estimate S1 gain
    // TODO : add backoff in gain estimation
    ofdmframesync_estimate_gain_S1(_q, &rc[_q->cp_len], _q->G);

    // compute detector output
    float complex g_hat = 0.0f;
    unsigned int i;
    for (i=0; i<_q->M; i++) {
        //g_hat += _q->G[(i+1+_q->M)%_q->M]*conjf(_q->G[(i+_q->M)%_q->M]);
        g_hat += _q->G[(i+1)%_q->M]*conjf(_q->G[i]);
    }
    g_hat /= _q->M_S1; // normalize output
    g_hat *= _q->g0;

    // rotate by complex phasor relative to timing backoff
    g_hat *= liquid_cexpjf((float)(_q->backoff)*2.0f*M_PI/(float)(_q->M));

#if DEBUG_OFDMFRAMESYNC_PRINT
    printf("    g_hat   :   %12.4f <%12.8f>\n", cabsf(g_hat), cargf(g_hat));
#endif

    // check conditions for g_hat:
    //  1. magnitude should be large (near unity) when aligned
    //  2. phase should be very near zero (time aligned)
    if (cabsf(g_hat) > _q->plcp_sync_thresh && fabsf(cargf(g_hat)) < 0.1f*M_PI ) {
        //printf("    acquisition\n");
        _q->state = OFDMFRAMESYNC_STATE_RXSYMBOLS;
        // reset timer
        _q->timer = _q->M + _q->cp_len + _q->backoff;
        _q->num_symbols = 0;

        // normalize gain by subcarriers, apply timing backoff correction
        float g = (float)(_q->M) / sqrtf(_q->M_pilot + _q->M_data);
        for (i=0; i<_q->M; i++) {
            _q->G[i] *= g;          // gain due to relative subcarrier allocation
            _q->G[i] *= _q->B[i];   // timing backoff correction
        }

#if 0
        // TODO : choose number of taps more appropriately
        //unsigned int ntaps = _q->M / 4;
        unsigned int ntaps = (_q->M < 8) ? 2 : 8;
        // FIXME : this is by far the most computationally complex part of synchronization
        ofdmframesync_estimate_eqgain(_q, ntaps);
#else
        unsigned int poly_order = 4;
        if (poly_order >= _q->M_pilot + _q->M_data)
            poly_order = _q->M_pilot + _q->M_data - 1;
        ofdmframesync_estimate_eqgain_poly(_q, poly_order);
#endif

#if 1
        // compute composite gain
        unsigned int i;
        for (i=0; i<_q->M; i++)
            _q->R[i] = _q->B[i] / _q->G[i];
#endif
        return LIQUID_OK;
    }

    // check if we are stuck searching for the S1 symbol
    if (_q->num_symbols == 16) {
#if DEBUG_OFDMFRAMESYNC_PRINT
        printf("could not find S1 symbol. bailing...\n");
#endif
        ofdmframesync_reset(_q);
    }

    // 'reset' timer (wait another half symbol)
    _q->timer = _q->M2;
    return LIQUID_OK;
}

int ofdmframesync_execute_rxsymbols(ofdmframesync _q)
{
    // wait for timeout
    _q->timer--;

    if (_q->timer == 0) {

        // run fft
        float complex * rc;
        windowcf_read(_q->input_buffer, &rc);
        memmove(_q->x, &rc[_q->cp_len-_q->backoff], (_q->M)*sizeof(float complex));
        FFT_EXECUTE(_q->fft);

        // recover symbol in internal _q->X buffer
        ofdmframesync_rxsymbol(_q);

#if DEBUG_OFDMFRAMESYNC
        if (_q->debug_enabled) {
            unsigned int i;
            for (i=0; i<_q->M; i++) {
                if (_q->p[i] == OFDMFRAME_SCTYPE_DATA)
                    windowcf_push(_q->debug_framesyms, _q->X[i]);
            }
        }
#endif
        // invoke callback
        if (_q->callback != NULL) {
            int retval = _q->callback(_q->X, _q->p, _q->M, _q->userdata);

            if (retval != 0)
                ofdmframesync_reset(_q);
        }

        // reset timer
        _q->timer = _q->M + _q->cp_len;
    }
    return LIQUID_OK;
}

// compute S0 metrics
int ofdmframesync_S0_metrics(ofdmframesync   _q,
                             float complex * _G,
                             float complex * _s_hat)
{
    // timing, carrier offset correction
    unsigned int i;
    float complex s_hat = 0.0f;

    // compute timing estimate, accumulate phase difference across
    // gains on subsequent pilot subcarriers (note that all the odd
    // subcarriers are NULL)
    for (i=0; i<_q->M; i+=2) {
        s_hat += _G[(i+2)%_q->M]*conjf(_G[i]);
    }
    s_hat /= _q->M_S0; // normalize output

    // set output values
    *_s_hat = s_hat;
    return LIQUID_OK;
}

// estimate short sequence gain
//  _q      :   ofdmframesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
int ofdmframesync_estimate_gain_S0(ofdmframesync   _q,
                                   float complex * _x,
                                   float complex * _G)
{
    // move input array into fft input buffer
    memmove(_q->x, _x, (_q->M)*sizeof(float complex));

    // compute fft, storing result into _q->X
    FFT_EXECUTE(_q->fft);
    
    // compute gain, ignoring NULL subcarriers
    unsigned int i;
    float gain = sqrtf(_q->M_S0) / (float)(_q->M);

    for (i=0; i<_q->M; i++) {
        if (_q->p[i] != OFDMFRAME_SCTYPE_NULL && (i%2)==0) {
            // NOTE : if cabsf(_q->S0[i]) == 0 then we can multiply by conjugate
            //        rather than compute division
            //_G[i] = _q->X[i] / _q->S0[i];
            _G[i] = _q->X[i] * conjf(_q->S0[i]);
        } else {
            _G[i] = 0.0f;
        }

        // normalize gain
        _G[i] *= gain;
    }
    return LIQUID_OK;
}

// estimate long sequence gain
//  _q      :   ofdmframesync object
//  _x      :   input array (time), [size: M x 1]
//  _G      :   output gain (freq)
int ofdmframesync_estimate_gain_S1(ofdmframesync _q,
                                   float complex * _x,
                                   float complex * _G)
{
    // move input array into fft input buffer
    memmove(_q->x, _x, (_q->M)*sizeof(float complex));

    // compute fft, storing result into _q->X
    FFT_EXECUTE(_q->fft);
    
    // compute gain, ignoring NULL subcarriers
    unsigned int i;
    float gain = sqrtf(_q->M_S1) / (float)(_q->M);
    for (i=0; i<_q->M; i++) {
        if (_q->p[i] != OFDMFRAME_SCTYPE_NULL) {
            // NOTE : if cabsf(_q->S1[i]) == 0 then we can multiply by conjugate
            //        rather than compute division
            //_G[i] = _q->X[i] / _q->S1[i];
            _G[i] = _q->X[i] * conjf(_q->S1[i]);
        } else {
            _G[i] = 0.0f;
        }

        // normalize gain
        _G[i] *= gain;
    }   
    return LIQUID_OK;
}

#if 0
// estimate complex equalizer gain from G0 and G1
//  _q      :   ofdmframesync object
//  _ntaps  :   number of time-domain taps for smoothing
int ofdmframesync_estimate_eqgain(ofdmframesync _q,
                                  unsigned int  _ntaps)
{
#if DEBUG_OFDMFRAMESYNC
    if (_q->debug_enabled) {
        // copy pre-smoothed gain
        memmove(_q->G_hat, _q->G, _q->M*sizeof(float complex));
    }
#endif

    // validate input
    if (_ntaps == 0 || _ntaps > _q->M)
        return liquid_error(LIQUID_EICONFIG,"ofdmframesync_estimate_eqgain(), ntaps must be in [1,M]");

    unsigned int i;

    // generate smoothing window (fft of temporal window)
    for (i=0; i<_q->M; i++)
        _q->x[i] = (i < _ntaps) ? 1.0f : 0.0f;
    FFT_EXECUTE(_q->fft);

    memmove(_q->G0a, _q->G, _q->M*sizeof(float complex));

    // smooth complex equalizer gains
    for (i=0; i<_q->M; i++) {
        // set gain to zero for null subcarriers
        if (_q->p[i] == OFDMFRAME_SCTYPE_NULL) {
            _q->G[i] = 0.0f;
            continue;
        }

        float complex w;
        float complex w0 = 0.0f;
        float complex G_hat = 0.0f;

        unsigned int j;
        for (j=0; j<_q->M; j++) {
            if (_q->p[j] == OFDMFRAME_SCTYPE_NULL) continue;

            // select window sample from array
            w = _q->X[(i + _q->M - j) % _q->M];

            // accumulate gain
            //G_hat += w * 0.5f * (_q->G0a[j] + _q->G0b[j]);
            G_hat += w * _q->G0a[j];
            w0 += w;
        }

        // eliminate divide-by-zero issues
        if (cabsf(w0) < 1e-4f) {
            liquid_error(LIQUID_EINT,"ofdmframesync_estimate_eqgain(), weighting factor is zero");
            w0 = 1.0f;
        }
        _q->G[i] = G_hat / w0;
    }
    return LIQUID_OK;
}
#endif

// estimate complex equalizer gain from G0 and G1 using polynomial fit
//  _q      :   ofdmframesync object
//  _order  :   polynomial order
int ofdmframesync_estimate_eqgain_poly(ofdmframesync _q,
                                       unsigned int _order)
{
#if DEBUG_OFDMFRAMESYNC
    if (_q->debug_enabled) {
        // copy pre-smoothed gain
        memmove(_q->G_hat, _q->G, _q->M*sizeof(float complex));
    }
#endif

    // polynomial interpolation
    unsigned int i;
    unsigned int N = _q->M_pilot + _q->M_data;
    if (_order > N-1) _order = N-1;
    if (_order > 10)  _order = 10;
    float x_freq[N];
    float y_abs[N];
    float y_arg[N];
    float p_abs[_order+1];
    float p_arg[_order+1];

    unsigned int n=0;
    unsigned int k;
    for (i=0; i<_q->M; i++) {

        // start at mid-point (effective fftshift)
        k = (i + _q->M2) % _q->M;

        if (_q->p[k] != OFDMFRAME_SCTYPE_NULL) {
            if (n == N)
                return liquid_error(LIQUID_EINT,"ofdmframesync_estimate_eqgain_poly(), pilot subcarrier mismatch");
            // store resulting...
            x_freq[n] = (k > _q->M2) ? (float)k - (float)(_q->M) : (float)k;
            x_freq[n] = x_freq[n] / (float)(_q->M);
            y_abs[n] = cabsf(_q->G[k]);
            y_arg[n] = cargf(_q->G[k]);

            // update counter
            n++;
        }
    }

    if (n != N)
        return liquid_error(LIQUID_EINT,"ofdmframesync_estimate_eqgain_poly(), pilot subcarrier mismatch");

    // try to unwrap phase
    liquid_unwrap_phase(y_arg, N);

    // fit to polynomial
    polyf_fit(x_freq, y_abs, N, p_abs, _order+1);
    polyf_fit(x_freq, y_arg, N, p_arg, _order+1);

    // compute subcarrier gain
    for (i=0; i<_q->M; i++) {
        float freq = (i > _q->M2) ? (float)i - (float)(_q->M) : (float)i;
        freq = freq / (float)(_q->M);
        float A     = polyf_val(p_abs, _order+1, freq);
        float theta = polyf_val(p_arg, _order+1, freq);
        _q->G[i] = (_q->p[i] == OFDMFRAME_SCTYPE_NULL) ? 0.0f : A * liquid_cexpjf(theta);
    }

#if 0
    for (i=0; i<N; i++)
        printf("x(%3u) = %12.8f; y_abs(%3u) = %12.8f; y_arg(%3u) = %12.8f;\n",
                i+1, x_freq[i],
                i+1, y_abs[i],
                i+1, y_arg[i]);

    for (i=0; i<=_order; i++)
        printf("p_abs(%3u) = %12.8f;\n", i+1, p_abs[i]);
    for (i=0; i<=_order; i++)
        printf("p_arg(%3u) = %12.8f;\n", i+1, p_arg[i]);
#endif
    return LIQUID_OK;
}

// recover symbol, correcting for gain, pilot phase, etc.
int ofdmframesync_rxsymbol(ofdmframesync _q)
{
    // apply gain
    unsigned int i;
    for (i=0; i<_q->M; i++)
        _q->X[i] *= _q->R[i];

    // polynomial curve-fit
    float x_phase[_q->M_pilot];
    float y_phase[_q->M_pilot];
    float p_phase[2];

    unsigned int n=0;
    unsigned int k;
    float complex pilot = 1.0f;
    for (i=0; i<_q->M; i++) {

        // start at mid-point (effective fftshift)
        k = (i + _q->M2) % _q->M;

        if (_q->p[k]==OFDMFRAME_SCTYPE_PILOT) {
            if (n == _q->M_pilot)
                return liquid_error(LIQUID_EINT,"ofdmframesync_estimate_eqgain_poly(), pilot subcarrier mismatch");

            pilot = (msequence_advance(_q->ms_pilot) ? 1.0f : -1.0f);
#if 0
            printf("pilot[%3u] = %12.4e + j*%12.4e (expected %12.4e + j*%12.4e)\n",
                    k,
                    crealf(_q->X[k]), cimagf(_q->X[k]),
                    crealf(pilot),    cimagf(pilot));
#endif
            // store resulting...
            x_phase[n] = (k > _q->M2) ? (float)k - (float)(_q->M) : (float)k;
            y_phase[n] = cargf(_q->X[k]*conjf(pilot));

            // update counter
            n++;

        }
    }

    if (n != _q->M_pilot)
        return liquid_error(LIQUID_EINT,"ofdmframesync_estimate_eqgain_poly(), pilot subcarrier mismatch");

    // try to unwrap phase
    liquid_unwrap_phase(y_phase, _q->M_pilot);

    // fit phase to 1st-order polynomial (2 coefficients)
    polyf_fit(x_phase, y_phase, _q->M_pilot, p_phase, 2);

    // filter slope estimate (timing offset)
    float alpha = 0.3f;
    p_phase[1] = alpha*p_phase[1] + (1-alpha)*_q->p1_prime;
    _q->p1_prime = p_phase[1];

#if DEBUG_OFDMFRAMESYNC
    if (_q->debug_enabled) {
        // save pilots
        memmove(_q->px, x_phase, _q->M_pilot*sizeof(float));
        memmove(_q->py, y_phase, _q->M_pilot*sizeof(float));

        // NOTE : swapping values for octave
        _q->p_phase[0] = p_phase[1];
        _q->p_phase[1] = p_phase[0];

        windowf_push(_q->debug_pilot_0, p_phase[0]);
        windowf_push(_q->debug_pilot_1, p_phase[1]);
    }
#endif

    // compensate for phase offset
    // TODO : find more computationally efficient way to do this
    for (i=0; i<_q->M; i++) {
        // only apply to data/pilot subcarriers
        if (_q->p[i] == OFDMFRAME_SCTYPE_NULL) {
            _q->X[i] = 0.0f;
        } else {
            float fx    = (i > _q->M2) ? (float)i - (float)(_q->M) : (float)i;
            float theta = polyf_val(p_phase, 2, fx);
            _q->X[i] *= liquid_cexpjf(-theta);
        }
    }

    // adjust NCO frequency based on differential phase
    if (_q->num_symbols > 0) {
        // compute phase error (unwrapped)
        float dphi_prime = p_phase[0] - _q->phi_prime;
        while (dphi_prime >  M_PI) dphi_prime -= M_2_PI;
        while (dphi_prime < -M_PI) dphi_prime += M_2_PI;

        // adjust NCO proportionally to phase error
        nco_crcf_adjust_frequency(_q->nco_rx, 1e-3f*dphi_prime);
    }
    // set internal phase state
    _q->phi_prime = p_phase[0];
    //printf("%3u : theta : %12.8f, nco freq: %12.8f\n", _q->num_symbols, p_phase[0], nco_crcf_get_frequency(_q->nco_rx));
    
    // increment symbol counter
    _q->num_symbols++;

#if 0
    for (i=0; i<_q->M_pilot; i++)
        printf("x_phase(%3u) = %12.8f; y_phase(%3u) = %12.8f;\n", i+1, x_phase[i], i+1, y_phase[i]);
    printf("poly : p0=%12.8f, p1=%12.8f\n", p_phase[0], p_phase[1]);
#endif
    return LIQUID_OK;
}

// enable debugging
int ofdmframesync_debug_enable(ofdmframesync _q)
{
    // create debugging objects if necessary
#if DEBUG_OFDMFRAMESYNC
    if (_q->debug_objects_created)
        return LIQUID_OK;

    _q->debug_x         = windowcf_create(DEBUG_OFDMFRAMESYNC_BUFFER_LEN);
    _q->debug_rssi      = windowf_create(DEBUG_OFDMFRAMESYNC_BUFFER_LEN);
    _q->debug_framesyms = windowcf_create(DEBUG_OFDMFRAMESYNC_BUFFER_LEN);
    _q->G_hat           = (float complex*) malloc((_q->M)*sizeof(float complex));

    _q->px = (float*) malloc((_q->M_pilot)*sizeof(float));
    _q->py = (float*) malloc((_q->M_pilot)*sizeof(float));

    _q->debug_pilot_0 = windowf_create(DEBUG_OFDMFRAMESYNC_BUFFER_LEN);
    _q->debug_pilot_1 = windowf_create(DEBUG_OFDMFRAMESYNC_BUFFER_LEN);

    _q->debug_enabled   = 1;
    _q->debug_objects_created = 1;
    return LIQUID_OK;
#else
    return liquid_error(LIQUID_EICONFIG,"ofdmframesync_debug_enable(): compile-time debugging disabled");
#endif
}

int ofdmframesync_debug_disable(ofdmframesync _q)
{
    // disable debugging
#if DEBUG_OFDMFRAMESYNC
    _q->debug_enabled = 0;
    return LIQUID_OK;
#else
    return liquid_error(LIQUID_EICONFIG,"ofdmframesync_debug_disable(): compile-time debugging disabled");
#endif
}

int ofdmframesync_debug_print(ofdmframesync _q,
                              const char * _filename)
{
#if DEBUG_OFDMFRAMESYNC
    if (!_q->debug_objects_created)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_debug_print(), debugging objects don't exist; enable debugging first");

    FILE * fid = fopen(_filename,"w");
    if (fid==NULL)
        return liquid_error(LIQUID_EIO,"ofdmframe_debug_print(), could not open '%s' for writing", _filename);

    fprintf(fid,"%% %s : auto-generated file\n", DEBUG_OFDMFRAMESYNC_FILENAME);
    fprintf(fid,"close all;\n");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"n = %u;\n", DEBUG_OFDMFRAMESYNC_BUFFER_LEN);
    fprintf(fid,"M = %u;\n", _q->M);
    fprintf(fid,"M_null  = %u;\n", _q->M_null);
    fprintf(fid,"M_pilot = %u;\n", _q->M_pilot);
    fprintf(fid,"M_data  = %u;\n", _q->M_data);
    unsigned int i;
    float complex * rc;
    float * r;

    // save subcarrier allocation
    fprintf(fid,"p = zeros(1,M);\n");
    for (i=0; i<_q->M; i++)
        fprintf(fid,"p(%4u) = %d;\n", i+1, _q->p[i]);
    fprintf(fid,"i_null  = find(p==%d);\n", OFDMFRAME_SCTYPE_NULL);
    fprintf(fid,"i_pilot = find(p==%d);\n", OFDMFRAME_SCTYPE_PILOT);
    fprintf(fid,"i_data  = find(p==%d);\n", OFDMFRAME_SCTYPE_DATA);

    // short, long, training sequences
    for (i=0; i<_q->M; i++) {
        fprintf(fid,"S0(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(_q->S0[i]), cimagf(_q->S0[i]));
        fprintf(fid,"S1(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(_q->S1[i]), cimagf(_q->S1[i]));
    }

    fprintf(fid,"x = zeros(1,n);\n");
    windowcf_read(_q->debug_x, &rc);
    for (i=0; i<DEBUG_OFDMFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(0:(n-1),real(x),0:(n-1),imag(x));\n");
    fprintf(fid,"xlabel('sample index');\n");
    fprintf(fid,"ylabel('received signal, x');\n");


    fprintf(fid,"s1 = [];\n");
    for (i=0; i<_q->M; i++)
        fprintf(fid,"s1(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(_q->s1[i]), cimagf(_q->s1[i]));


    // write agc_rssi
    fprintf(fid,"\n\n");
    fprintf(fid,"agc_rssi = zeros(1,%u);\n", DEBUG_OFDMFRAMESYNC_BUFFER_LEN);
    windowf_read(_q->debug_rssi, &r);
    for (i=0; i<DEBUG_OFDMFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"agc_rssi(%4u) = %12.4e;\n", i+1, r[i]);
    fprintf(fid,"agc_rssi = filter([0.00362168 0.00724336 0.00362168],[1 -1.82269490 0.83718163],agc_rssi);\n");
    fprintf(fid,"agc_rssi = 10*log10( agc_rssi );\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(agc_rssi)\n");
    fprintf(fid,"ylabel('RSSI [dB]');\n");

    // write short, long symbols
    fprintf(fid,"\n\n");
    fprintf(fid,"S0 = zeros(1,M);\n");
    fprintf(fid,"S1 = zeros(1,M);\n");
    for (i=0; i<_q->M; i++) {
        fprintf(fid,"S0(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(_q->S0[i]), cimagf(_q->S0[i]));
        fprintf(fid,"S1(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(_q->S1[i]), cimagf(_q->S1[i]));
    }


    // write gain arrays
    fprintf(fid,"\n\n");
    fprintf(fid,"G0     = zeros(1,M);\n");
    fprintf(fid,"G1     = zeros(1,M);\n");
    fprintf(fid,"G_hat  = zeros(1,M);\n");
    fprintf(fid,"G      = zeros(1,M);\n");
    for (i=0; i<_q->M; i++) {
        fprintf(fid,"G0(%3u)    = %12.8f + j*%12.8f;\n", i+1, crealf(_q->G0a[i]),  cimagf(_q->G0a[i]));
        fprintf(fid,"G1(%3u)    = %12.8f + j*%12.8f;\n", i+1, crealf(_q->G0b[i]),  cimagf(_q->G0b[i]));
        fprintf(fid,"G_hat(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(_q->G_hat[i]),cimagf(_q->G_hat[i]));
        fprintf(fid,"G(%3u)     = %12.8f + j*%12.8f;\n", i+1, crealf(_q->G[i]),    cimagf(_q->G[i]));
    }
    fprintf(fid,"f = [0:(M-1)];\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(f, fftshift(abs(G_hat)),'sb',...\n");
    fprintf(fid,"       f, fftshift(abs(G)),'-k','LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('subcarrier index');\n");
    fprintf(fid,"  ylabel('gain estimate (mag)');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(f, fftshift(arg(G_hat).*[abs(G0) > 1e-3]),'sb',...\n");
    fprintf(fid,"       f, fftshift(arg(G)),'-k','LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('subcarrier index');\n");
    fprintf(fid,"  ylabel('gain estimate (phase)');\n");

    // write pilot response
    fprintf(fid,"\n\n");
    fprintf(fid,"px = zeros(1,M_pilot);\n");
    fprintf(fid,"py = zeros(1,M_pilot);\n");
    for (i=0; i<_q->M_pilot; i++) {
        fprintf(fid,"px(%3u) = %12.8f;\n", i+1, _q->px[i]);
        fprintf(fid,"py(%3u) = %12.8f;\n", i+1, _q->py[i]);
    }
    fprintf(fid,"p_phase(1) = %12.8f;\n", _q->p_phase[0]);
    fprintf(fid,"p_phase(2) = %12.8f;\n", _q->p_phase[1]);

    // save pilot history
    fprintf(fid,"p0 = zeros(1,M);\n");
    windowf_read(_q->debug_pilot_0, &r);
    for (i=0; i<DEBUG_OFDMFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"p0(%4u) = %12.4e;\n", i+1, r[i]);

    fprintf(fid,"p1 = zeros(1,M);\n");
    windowf_read(_q->debug_pilot_1, &r);
    for (i=0; i<DEBUG_OFDMFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"p1(%4u) = %12.4e;\n", i+1, r[i]);

    fprintf(fid,"figure;\n");
    fprintf(fid,"fp = (-M/2):(M/2);\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(px, py, 'sb',...\n");
    fprintf(fid,"       fp, polyval(p_phase, fp), '-k');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  legend('pilots','polyfit',0);\n");
    fprintf(fid,"  xlabel('subcarrier');\n");
    fprintf(fid,"  ylabel('phase');\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(1:length(p0), p0);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('p0 (phase offset)');\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(1:length(p1), p1);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('p1 (phase slope)');\n");

    // write frame symbols
    fprintf(fid,"framesyms = zeros(1,n);\n");
    windowcf_read(_q->debug_framesyms, &rc);
    for (i=0; i<DEBUG_OFDMFRAMESYNC_BUFFER_LEN; i++)
        fprintf(fid,"framesyms(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(real(framesyms), imag(framesyms), 'x');\n");
    fprintf(fid,"xlabel('I');\n");
    fprintf(fid,"ylabel('Q');\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.6);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("ofdmframesync/debug: results written to '%s'\n", _filename);
    return LIQUID_OK;
#else
    return liquid_error(LIQUID_EICONFIG,"ofdmframesync_debug_print(): compile-time debugging disabled");
#endif
}

