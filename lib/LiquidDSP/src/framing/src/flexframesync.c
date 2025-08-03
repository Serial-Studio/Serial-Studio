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
// flexframesync.c
//
// basic frame synchronizer
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FLEXFRAMESYNC         1
#define DEBUG_FLEXFRAMESYNC_PRINT   0
#define DEBUG_FILENAME              "flexframesync_internal_debug.m"
#define DEBUG_BUFFER_LEN            (2000)

#define FLEXFRAMESYNC_ENABLE_EQ     0

// push samples through detection stage
int flexframesync_execute_seekpn(flexframesync _q,
                                 float complex _x);

// step receiver mixer, matched filter, decimator
//  _q      :   frame synchronizer
//  _x      :   input sample
//  _y      :   output symbol
int flexframesync_step(flexframesync   _q,
                       float complex   _x,
                       float complex * _y);

// push samples through synchronizer, saving received p/n symbols
int flexframesync_execute_rxpreamble(flexframesync _q,
                                     float complex _x);

// decode header and reconfigure payload
int flexframesync_decode_header(flexframesync _q);

// receive header symbols
int flexframesync_execute_rxheader(flexframesync _q,
                                   float complex _x);

// receive payload symbols
int flexframesync_execute_rxpayload(flexframesync _q,
                                    float complex _x);

static flexframegenprops_s flexframesyncprops_header_default = {
   FLEXFRAME_H_CRC,
   FLEXFRAME_H_FEC0,
   FLEXFRAME_H_FEC1,
   FLEXFRAME_H_MOD,
};

// flexframesync object structure
struct flexframesync_s {
    // callback
    framesync_callback  callback;       // user-defined callback function
    void *              userdata;       // user-defined data structure
    framesyncstats_s    framesyncstats; // frame statistic object (synchronizer)
    framedatastats_s    framedatastats; // frame statistic object (packet statistics)
    
    // synchronizer objects
    unsigned int    m;                  // filter delay (symbols)
    float           beta;               // filter excess bandwidth factor
    qdetector_cccf  detector;           // pre-demod detector
    float           tau_hat;            // fractional timing offset estimate
    float           dphi_hat;           // carrier frequency offset estimate
    float           phi_hat;            // carrier phase offset estimate
    float           gamma_hat;          // channel gain estimate
    nco_crcf        mixer;              // carrier frequency recovery (coarse)
    nco_crcf        pll;                // carrier frequency recovery (fine)

    // timing recovery objects, states
    firpfb_crcf     mf;                 // matched filter decimator
    unsigned int    npfb;               // number of filters in symsync
    int             mf_counter;         // matched filter output timer
    unsigned int    pfb_index;          // filterbank index
#if FLEXFRAMESYNC_ENABLE_EQ
    eqlms_cccf      equalizer;          // equalizer (trained on p/n sequence)
#endif

    // preamble
    float complex * preamble_pn;        // known 64-symbol p/n sequence
    float complex * preamble_rx;        // received p/n symbols
    
    // header
    int             header_soft;        // header performs soft demod
    float complex * header_sym;         // header symbols with pilots (received)
    unsigned int    header_sym_len;     // header symbols with pilots (length)
    qpilotsync      header_pilotsync;   // header demodulator/decoder
    float complex * header_mod;         // header symbols (received)
    unsigned int    header_mod_len;     // header symbols (length)
    qpacketmodem    header_decoder;     // header demodulator/decoder
    unsigned int    header_user_len;    // length of user-defined array
    unsigned int    header_dec_len;     // length of header (decoded)
    unsigned char * header_dec;         // header bytes (decoded)
    int             header_valid;       // header CRC flag

    flexframegenprops_s header_props;   // header properties

    // payload
    int             payload_soft;       // payload performs soft demod
    modemcf         payload_demod;      // payload demod (for phase recovery only)
    float complex * payload_sym;        // payload symbols (received)
    unsigned int    payload_sym_len;    // payload symbols (length)
    qpacketmodem    payload_decoder;    // payload demodulator/decoder
    unsigned char * payload_dec;        // payload data (bytes)
    unsigned int    payload_dec_len;    // payload data (length)
    int             payload_valid;      // payload CRC flag
    
    // status variables
    unsigned int    preamble_counter;   // counter: num of p/n syms received
    unsigned int    symbol_counter;     // counter: num of symbols received
    enum {
        FLEXFRAMESYNC_STATE_DETECTFRAME=0,  // detect frame (seek p/n sequence)
        FLEXFRAMESYNC_STATE_RXPREAMBLE,     // receive p/n sequence
        FLEXFRAMESYNC_STATE_RXHEADER,       // receive header data
        FLEXFRAMESYNC_STATE_RXPAYLOAD,      // receive payload data
    }               state;                  // receiver state

#if DEBUG_FLEXFRAMESYNC
    int         debug_enabled;          // debugging enabled?
    int         debug_objects_created;  // debugging objects created?
    int         debug_qdetector_flush;  // debug: flag to set if we are flushing detector
    windowcf    debug_x;                // debug: raw input samples
#endif
};

// create flexframesync object
//  _callback       :   callback function invoked when frame is received
//  _userdata       :   user-defined data object passed to callback
flexframesync flexframesync_create(framesync_callback _callback,
                                   void *             _userdata)
{
    flexframesync q = (flexframesync) malloc(sizeof(struct flexframesync_s));
    q->callback = _callback;
    q->userdata = _userdata;
    q->m        = 7;    // filter delay (symbols)
    q->beta     = 0.3f; // excess bandwidth factor

    unsigned int i;

    // generate p/n sequence
    q->preamble_pn = (float complex*) malloc(64*sizeof(float complex));
    q->preamble_rx = (float complex*) malloc(64*sizeof(float complex));
    msequence ms = msequence_create(7, 0x0089, 1);
    for (i=0; i<64; i++) {
        q->preamble_pn[i] = (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2);
        q->preamble_pn[i] += (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2) * _Complex_I;
    }
    msequence_destroy(ms);

    // create frame detector
    unsigned int k = 2; // samples/symbol
    q->detector = qdetector_cccf_create_linear(q->preamble_pn, 64, LIQUID_FIRFILT_ARKAISER, k, q->m, q->beta);
    qdetector_cccf_set_threshold(q->detector, 0.5f);

    // create symbol timing recovery filters
    q->npfb = 32;   // number of filters in the bank
    q->mf   = firpfb_crcf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, q->npfb,k,q->m,q->beta);

#if FLEXFRAMESYNC_ENABLE_EQ
    // create equalizer
    unsigned int p = 3;
    q->equalizer = eqlms_cccf_create_lowpass(2*k*p+1, 0.4f);
    eqlms_cccf_set_bw(q->equalizer, 0.05f);
#endif

    // create down-coverters for carrier phase tracking
    q->mixer = nco_crcf_create(LIQUID_NCO);
    q->pll   = nco_crcf_create(LIQUID_NCO);
    nco_crcf_pll_set_bandwidth(q->pll, 1e-4f); // very low bandwidth
    
    // header demodulator/decoder
    q->header_sym = NULL;
    q->header_mod = NULL;
    q->header_dec = NULL;
    q->header_pilotsync = NULL;
    q->header_decoder = NULL;
    q->header_user_len = FLEXFRAME_H_USER_DEFAULT;
    q->header_soft = 0;
    flexframesync_set_header_props(q, NULL);

    // payload demodulator for phase recovery
    q->payload_demod = modemcf_create(LIQUID_MODEM_QPSK);

    // create payload demodulator/decoder object
    q->payload_dec_len = 64;
    int check      = LIQUID_CRC_24;
    int fec0       = LIQUID_FEC_NONE;
    int fec1       = LIQUID_FEC_GOLAY2412;
    int mod_scheme = LIQUID_MODEM_BPSK;
    q->payload_decoder = qpacketmodem_create();
    qpacketmodem_configure(q->payload_decoder, q->payload_dec_len, check, fec0, fec1, mod_scheme);
    //qpacketmodem_print(q->payload_decoder);
    //assert( qpacketmodem_get_frame_len(q->payload_decoder)==600 );
    q->payload_sym_len = qpacketmodem_get_frame_len(q->payload_decoder);

    // allocate memory for payload symbols and recovered data bytes
    q->payload_sym = (float complex*) malloc(q->payload_sym_len*sizeof(float complex));
    q->payload_dec = (unsigned char*) malloc(q->payload_dec_len*sizeof(unsigned char));
    q->payload_soft = 0;

    // reset global data counters
    flexframesync_reset_framedatastats(q);

#if DEBUG_FLEXFRAMESYNC
    // set debugging flags, objects to NULL
    q->debug_enabled         = 0;
    q->debug_objects_created = 0;
    q->debug_qdetector_flush = 0;
    q->debug_x               = NULL;
#endif

    // reset state and return
    flexframesync_reset(q);
    return q;
}

// destroy frame synchronizer object, freeing all internal memory
int flexframesync_destroy(flexframesync _q)
{
#if DEBUG_FLEXFRAMESYNC
    // clean up debug objects (if created)
    if (_q->debug_objects_created)
        windowcf_destroy(_q->debug_x);
#endif

    // free allocated arrays
    free(_q->preamble_pn);
    free(_q->preamble_rx);
    free(_q->header_sym);
    free(_q->header_mod);
    free(_q->header_dec);
    free(_q->payload_sym);
    free(_q->payload_dec);

    // destroy synchronization objects
    qpilotsync_destroy    (_q->header_pilotsync); // header demodulator/decoder
    qpacketmodem_destroy  (_q->header_decoder);   // header demodulator/decoder
    modemcf_destroy         (_q->payload_demod);    // payload demodulator (for PLL)
    qpacketmodem_destroy  (_q->payload_decoder);  // payload demodulator/decoder
    qdetector_cccf_destroy(_q->detector);         // frame detector
    firpfb_crcf_destroy   (_q->mf);               // matched filter
    nco_crcf_destroy      (_q->mixer);            // oscillator (coarse)
    nco_crcf_destroy      (_q->pll);              // oscillator (fine)
#if FLEXFRAMESYNC_ENABLE_EQ
    eqlms_cccf_destroy    (_q->equalizer);        // LMS equalizer
#endif

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print frame synchronizer object internals
int flexframesync_print(flexframesync _q)
{
    printf("<liquid.flexframesync>\n");
    return LIQUID_OK;
}

// reset frame synchronizer object
int flexframesync_reset(flexframesync _q)
{
    // reset binary pre-demod synchronizer
    qdetector_cccf_reset(_q->detector);

    // reset carrier recovery objects
    nco_crcf_reset(_q->mixer);
    nco_crcf_reset(_q->pll);

    // reset symbol timing recovery state
    firpfb_crcf_reset(_q->mf);
        
    // reset state
    _q->state           = FLEXFRAMESYNC_STATE_DETECTFRAME;
    _q->preamble_counter= 0;
    _q->symbol_counter  = 0;
    
    // reset frame statistics
    _q->framesyncstats.evm = 0.0f;
    return LIQUID_OK;
}

int flexframesync_is_frame_open(flexframesync _q)
{
    return (_q->state == FLEXFRAMESYNC_STATE_DETECTFRAME) ? 0 : 1;
}

int flexframesync_set_header_len(flexframesync _q,
                                  unsigned int  _len)
{
    _q->header_user_len = _len;
    _q->header_dec_len = FLEXFRAME_H_DEC + _q->header_user_len;
    _q->header_dec     = (unsigned char *) realloc(_q->header_dec, _q->header_dec_len*sizeof(unsigned char));
    if (_q->header_decoder) {
        qpacketmodem_destroy(_q->header_decoder);
    }
    _q->header_decoder = qpacketmodem_create();
    qpacketmodem_configure(_q->header_decoder,
                           _q->header_dec_len,
                           _q->header_props.check,
                           _q->header_props.fec0,
                           _q->header_props.fec1,
                           _q->header_props.mod_scheme);
    _q->header_mod_len = qpacketmodem_get_frame_len(_q->header_decoder);
    _q->header_mod     = (float complex*) realloc(_q->header_mod, _q->header_mod_len*sizeof(float complex));

    // header pilot synchronizer
    if (_q->header_pilotsync) {
        qpilotsync_destroy(_q->header_pilotsync);
    }
    _q->header_pilotsync = qpilotsync_create(_q->header_mod_len, 16);
    _q->header_sym_len   = qpilotsync_get_frame_len(_q->header_pilotsync);
    _q->header_sym       = (float complex*) realloc(_q->header_sym, _q->header_sym_len*sizeof(float complex));
    return LIQUID_OK;
}

int flexframesync_decode_header_soft(flexframesync _q,
                                     int           _soft)
{
    _q->header_soft = _soft;
    return LIQUID_OK;
}

int flexframesync_decode_payload_soft(flexframesync _q,
                                      int           _soft)
{
    _q->payload_soft = _soft;
    return LIQUID_OK;
}

int flexframesync_set_header_props(flexframesync          _q,
                                   flexframegenprops_s * _props)
{
    if (_props == NULL)
        _props = &flexframesyncprops_header_default;

    // validate input
    if (_props->check == LIQUID_CRC_UNKNOWN || _props->check >= LIQUID_CRC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"flexframesync_set_header_props(), invalid/unsupported CRC scheme");
    if (_props->fec0 == LIQUID_FEC_UNKNOWN || _props->fec1 == LIQUID_FEC_UNKNOWN)
        return liquid_error(LIQUID_EICONFIG,"flexframesync_set_header_props(), invalid/unsupported FEC scheme");
    if (_props->mod_scheme == LIQUID_MODEM_UNKNOWN )
        return liquid_error(LIQUID_EICONFIG,"flexframesync_set_header_props(), invalid/unsupported modulation scheme");

    // copy properties to internal structure
    memmove(&_q->header_props, _props, sizeof(flexframegenprops_s));

    // reconfigure payload buffers (reallocate as necessary)
    return flexframesync_set_header_len(_q, _q->header_user_len);
}

// execute frame synchronizer
//  _q  :   frame synchronizer object
//  _x  :   input sample array [size: _n x 1]
//  _n  :   number of input samples
int flexframesync_execute(flexframesync   _q,
                          float complex * _x,
                          unsigned int    _n)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
#if DEBUG_FLEXFRAMESYNC
        // write samples to debug buffer
        // NOTE: the debug_qdetector_flush prevents samples from being written twice
        if (_q->debug_enabled && !_q->debug_qdetector_flush)
            windowcf_push(_q->debug_x, _x[i]);
#endif
        switch (_q->state) {
        case FLEXFRAMESYNC_STATE_DETECTFRAME:
            // detect frame (look for p/n sequence)
            flexframesync_execute_seekpn(_q, _x[i]);
            break;
        case FLEXFRAMESYNC_STATE_RXPREAMBLE:
            // receive p/n sequence symbols
            flexframesync_execute_rxpreamble(_q, _x[i]);
            break;
        case FLEXFRAMESYNC_STATE_RXHEADER:
            // receive header symbols
            flexframesync_execute_rxheader(_q, _x[i]);
            break;
        case FLEXFRAMESYNC_STATE_RXPAYLOAD:
            // receive payload symbols
            flexframesync_execute_rxpayload(_q, _x[i]);
            break;
        default:
            return liquid_error(LIQUID_EINT,"flexframesync_exeucte(), unknown/unsupported internal state");
        }
    }
    return LIQUID_OK;
}

// 
// internal methods
//

// execute synchronizer, seeking p/n sequence
//  _q      :   frame synchronizer object
//  _x      :   input sample
//  _sym    :   demodulated symbol
int flexframesync_execute_seekpn(flexframesync _q,
                                  float complex _x)
{
    // push through pre-demod synchronizer
    float complex * v = qdetector_cccf_execute(_q->detector, _x);

    // check if frame has been detected
    if (v == NULL)
        return LIQUID_OK;

    // get estimates
    _q->tau_hat   = qdetector_cccf_get_tau  (_q->detector);
    _q->gamma_hat = qdetector_cccf_get_gamma(_q->detector);
    _q->dphi_hat  = qdetector_cccf_get_dphi (_q->detector);
    _q->phi_hat   = qdetector_cccf_get_phi  (_q->detector);

#if DEBUG_FLEXFRAMESYNC_PRINT
    printf("***** frame detected! tau-hat:%8.4f, dphi-hat:%8.4f, gamma:%8.2f dB\n",
            _q->tau_hat, _q->dphi_hat, 20*log10f(_q->gamma_hat));
#endif

    // set appropriate filterbank index
    if (_q->tau_hat > 0) {
        _q->pfb_index = (unsigned int)(      _q->tau_hat  * _q->npfb) % _q->npfb;
        _q->mf_counter = 0;
    } else {
        _q->pfb_index = (unsigned int)((1.0f+_q->tau_hat) * _q->npfb) % _q->npfb;
        _q->mf_counter = 1;
    }
    
    // output filter scale (gain estimate, scaled by 1/2 for k=2 samples/symbol)
    firpfb_crcf_set_scale(_q->mf, 0.5f / _q->gamma_hat);

    // set frequency/phase of mixer
    nco_crcf_set_frequency(_q->mixer, _q->dphi_hat);
    nco_crcf_set_phase    (_q->mixer, _q->phi_hat );

    // update state
    _q->state = FLEXFRAMESYNC_STATE_RXPREAMBLE;

#if DEBUG_FLEXFRAMESYNC
    // the debug_qdetector_flush prevents samples from being written twice
    _q->debug_qdetector_flush = 1;
#endif
    // run buffered samples through synchronizer
    unsigned int buf_len = qdetector_cccf_get_buf_len(_q->detector);
    flexframesync_execute(_q, v, buf_len);
#if DEBUG_FLEXFRAMESYNC
    _q->debug_qdetector_flush = 0;
#endif
    return LIQUID_OK;
}

// step receiver mixer, matched filter, decimator
//  _q      :   frame synchronizer
//  _x      :   input sample
//  _y      :   output symbol
int flexframesync_step(flexframesync   _q,
                       float complex   _x,
                       float complex * _y)
{
    // mix sample down
    float complex v;
    nco_crcf_mix_down(_q->mixer, _x, &v);
    nco_crcf_step    (_q->mixer);
    
    // push sample into filterbank
    firpfb_crcf_push   (_q->mf, v);
    firpfb_crcf_execute(_q->mf, _q->pfb_index, &v);

#if FLEXFRAMESYNC_ENABLE_EQ
    // push sample through equalizer
    eqlms_cccf_push(_q->equalizer, v);
#endif

    // increment counter to determine if sample is available
    _q->mf_counter++;
    int sample_available = (_q->mf_counter >= 1) ? 1 : 0;
    
    // set output sample if available
    if (sample_available) {
#if FLEXFRAMESYNC_ENABLE_EQ
        // compute equalizer output
        eqlms_cccf_execute(_q->equalizer, &v);
#endif

        // set output
        *_y = v;

        // decrement counter by k=2 samples/symbol
        _q->mf_counter -= 2;
    }

    // return flag
    return sample_available;
}

// execute synchronizer, receiving p/n sequence
//  _q     :   frame synchronizer object
//  _x      :   input sample
//  _sym    :   demodulated symbol
int flexframesync_execute_rxpreamble(flexframesync _q,
                                     float complex _x)
{
    // step synchronizer
    float complex mf_out = 0.0f;
    int sample_available = flexframesync_step(_q, _x, &mf_out);

    // compute output if timeout
    if (sample_available) {

        // save output in p/n symbols buffer
#if FLEXFRAMESYNC_ENABLE_EQ
        unsigned int delay = 2*_q->m + 3; // delay from matched filter and equalizer
#else
        unsigned int delay = 2*_q->m;     // delay from matched filter
#endif
        if (_q->preamble_counter >= delay) {
            unsigned int index = _q->preamble_counter-delay;

            _q->preamble_rx[index] = mf_out;
        
#if FLEXFRAMESYNC_ENABLE_EQ
            // train equalizer
            eqlms_cccf_step(_q->equalizer, _q->preamble_pn[index], mf_out);
#endif
        }

        // update p/n counter
        _q->preamble_counter++;

        // update state
        if (_q->preamble_counter == 64 + delay)
            _q->state = FLEXFRAMESYNC_STATE_RXHEADER;
    }
    return LIQUID_OK;
}

// execute synchronizer, receiving header
//  _q      :   frame synchronizer object
//  _x      :   input sample
//  _sym    :   demodulated symbol
int flexframesync_execute_rxheader(flexframesync _q,
                                   float complex _x)
{
    // step synchronizer
    float complex mf_out = 0.0f;
    int sample_available = flexframesync_step(_q, _x, &mf_out);

    // compute output if timeout
    if (sample_available) {
        // save payload symbols (modem input/output)
        _q->header_sym[_q->symbol_counter] = mf_out;

        // increment counter
        _q->symbol_counter++;

        if (_q->symbol_counter == _q->header_sym_len) {
            // decode header
            flexframesync_decode_header(_q);

            if (_q->header_valid) {
                // continue on to decoding payload
                _q->symbol_counter = 0;
                _q->state = FLEXFRAMESYNC_STATE_RXPAYLOAD;
                return LIQUID_OK;
            }

            // update statistics
            _q->framedatastats.num_frames_detected++;

            // header invalid: invoke callback
            if (_q->callback != NULL) {
                // set framestats internals
                _q->framesyncstats.evm           = 0.0f; //20*log10f(sqrtf(_q->framesyncstats.evm / 600));
                _q->framesyncstats.rssi          = 20*log10f(_q->gamma_hat);
                _q->framesyncstats.cfo           = nco_crcf_get_frequency(_q->mixer);
                _q->framesyncstats.framesyms     = NULL;
                _q->framesyncstats.num_framesyms = 0;
                _q->framesyncstats.mod_scheme    = LIQUID_MODEM_UNKNOWN;
                _q->framesyncstats.mod_bps       = 0;
                _q->framesyncstats.check         = LIQUID_CRC_UNKNOWN;
                _q->framesyncstats.fec0          = LIQUID_FEC_UNKNOWN;
                _q->framesyncstats.fec1          = LIQUID_FEC_UNKNOWN;

                // invoke callback method
                _q->callback(_q->header_dec,
                             _q->header_valid,
                             NULL,  // payload
                             0,     // payload length
                             0,     // payload valid,
                             _q->framesyncstats,
                             _q->userdata);
            }

            // reset frame synchronizer
            return flexframesync_reset(_q);
        }
    }
    return LIQUID_OK;
}

// decode header
int flexframesync_decode_header(flexframesync _q)
{
    // recover data symbols from pilots
    qpilotsync_execute(_q->header_pilotsync, _q->header_sym, _q->header_mod);

    // decode payload
    if (_q->header_soft) {
        _q->header_valid = qpacketmodem_decode_soft(_q->header_decoder,
                                                    _q->header_mod,
                                                    _q->header_dec);
    } else {
        _q->header_valid = qpacketmodem_decode(_q->header_decoder,
                                               _q->header_mod,
                                               _q->header_dec);
    }

    if (!_q->header_valid)
        return LIQUID_OK;

    // set fine carrier frequency and phase
    float dphi_hat = qpilotsync_get_dphi(_q->header_pilotsync);
    float  phi_hat = qpilotsync_get_phi (_q->header_pilotsync);
    //printf("residual offset: dphi=%12.8f, phi=%12.8f\n", dphi_hat, phi_hat);
    nco_crcf_set_frequency(_q->pll, dphi_hat);
    nco_crcf_set_phase    (_q->pll, phi_hat + dphi_hat * _q->header_sym_len);

    // first several bytes of header are user-defined
    unsigned int n = _q->header_user_len;

    // first byte is for expansion/version validation
    unsigned int protocol = _q->header_dec[n+0];
    if (protocol != FLEXFRAME_PROTOCOL) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EICONFIG,"flexframesync_decode_header(), invalid framing protocol %u (expected %u)", protocol, FLEXFRAME_PROTOCOL);
    }

    // strip off payload length
    unsigned int payload_dec_len = (_q->header_dec[n+1] << 8) | (_q->header_dec[n+2]);
    _q->payload_dec_len = payload_dec_len;

    // strip off modulation scheme/depth
    unsigned int mod_scheme = _q->header_dec[n+3];

    // strip off CRC, forward error-correction schemes
    //  CRC     : most-significant 3 bits of [n+4]
    //  fec0    : least-significant 5 bits of [n+4]
    //  fec1    : least-significant 5 bits of [n+5]
    unsigned int check = (_q->header_dec[n+4] >> 5 ) & 0x07;
    unsigned int fec0  = (_q->header_dec[n+4]      ) & 0x1f;
    unsigned int fec1  = (_q->header_dec[n+5]      ) & 0x1f;

    // validate properties
    if (mod_scheme == 0 || mod_scheme >= LIQUID_MODEM_NUM_SCHEMES) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EICONFIG,"flexframesync_decode_header(), invalid modulation scheme");
    } else if (check == LIQUID_CRC_UNKNOWN || check >= LIQUID_CRC_NUM_SCHEMES) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EICONFIG,"flexframesync_decode_header(), decoded CRC exceeds available");
    } else if (fec0 == LIQUID_FEC_UNKNOWN || fec0 >= LIQUID_FEC_NUM_SCHEMES) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EICONFIG,"flexframesync_decode_header(), decoded FEC (inner) exceeds available");
    } else if (fec1 == LIQUID_FEC_UNKNOWN || fec1 >= LIQUID_FEC_NUM_SCHEMES) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EICONFIG,"flexframesync_decode_header(), decoded FEC (outer) exceeds available");
    }

    // re-create payload demodulator for phase-locked loop
    _q->payload_demod = modemcf_recreate(_q->payload_demod, mod_scheme);

    // reconfigure payload demodulator/decoder
    qpacketmodem_configure(_q->payload_decoder,
                           payload_dec_len, check, fec0, fec1, mod_scheme);

    // set length appropriately
    _q->payload_sym_len = qpacketmodem_get_frame_len(_q->payload_decoder);

    // re-allocate buffers accordingly
    _q->payload_sym = (float complex*) realloc(_q->payload_sym, (_q->payload_sym_len)*sizeof(float complex));
    _q->payload_dec = (unsigned char*) realloc(_q->payload_dec, (_q->payload_dec_len)*sizeof(unsigned char));

    if (_q->payload_sym == NULL || _q->payload_dec == NULL) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EIMEM,"flexframesync_decode_header(), could not re-allocate payload arrays");
    }

#if DEBUG_FLEXFRAMESYNC_PRINT
    // print results
    printf("flexframesync_decode_header():\n");
    printf("    header crc      : %s\n", _q->header_valid ? "pass" : "FAIL");
    printf("    check           : %s\n", crc_scheme_str[check][1]);
    printf("    fec (inner)     : %s\n", fec_scheme_str[fec0][1]);
    printf("    fec (outer)     : %s\n", fec_scheme_str[fec1][1]);
    printf("    mod scheme      : %s\n", modulation_types[mod_scheme].name);
    printf("    payload sym len : %u\n", _q->payload_sym_len);
    printf("    payload dec len : %u\n", _q->payload_dec_len);
    printf("    user data       :");
    unsigned int i;
    for (i=0; i<_q->header_user_len; i++)
        printf(" %.2x", _q->header_dec[i]);
    printf("\n");
#endif
    return LIQUID_OK;
}


// execute synchronizer, receiving payload
//  _q      :   frame synchronizer object
//  _x      :   input sample
//  _sym    :   demodulated symbol
int flexframesync_execute_rxpayload(flexframesync _q,
                                    float complex _x)
{
    // step synchronizer
    float complex mf_out = 0.0f;
    int sample_available = flexframesync_step(_q, _x, &mf_out);

    // compute output if timeout
    if (sample_available) {
        // TODO: clean this up
        // mix down with fine-tuned oscillator
        nco_crcf_mix_down(_q->pll, mf_out, &mf_out);
        // track phase, accumulate error-vector magnitude
        unsigned int sym;
        modemcf_demodulate(_q->payload_demod, mf_out, &sym);
        float phase_error = modemcf_get_demodulator_phase_error(_q->payload_demod);
        float evm         = modemcf_get_demodulator_evm        (_q->payload_demod);
        nco_crcf_pll_step(_q->pll, phase_error);
        nco_crcf_step(_q->pll);
        _q->framesyncstats.evm += evm*evm;

        // save payload symbols (modem input/output)
        _q->payload_sym[_q->symbol_counter] = mf_out;

        // increment counter
        _q->symbol_counter++;

        if (_q->symbol_counter == _q->payload_sym_len) {
            // decode payload
            if (_q->payload_soft) {
                _q->payload_valid = qpacketmodem_decode_soft(_q->payload_decoder,
                                                             _q->payload_sym,
                                                             _q->payload_dec);
            } else {
                _q->payload_valid = qpacketmodem_decode(_q->payload_decoder,
                                                        _q->payload_sym,
                                                        _q->payload_dec);
            }

            // update statistics
            _q->framedatastats.num_frames_detected++;
            _q->framedatastats.num_headers_valid++;
            if (_q->payload_valid) {
                _q->framedatastats.num_payloads_valid += 1;
                _q->framedatastats.num_bytes_received += _q->payload_dec_len;
            }

            // invoke callback
            if (_q->callback != NULL) {
                // set framestats internals
                int ms = qpacketmodem_get_modscheme(_q->payload_decoder);
                _q->framesyncstats.evm           = 10*log10f(_q->framesyncstats.evm / (float)_q->payload_sym_len);
                _q->framesyncstats.rssi          = 20*log10f(_q->gamma_hat);
                _q->framesyncstats.cfo           = nco_crcf_get_frequency(_q->mixer);
                _q->framesyncstats.framesyms     = _q->payload_sym;
                _q->framesyncstats.num_framesyms = _q->payload_sym_len;
                _q->framesyncstats.mod_scheme    = ms;
                _q->framesyncstats.mod_bps       = modulation_types[ms].bps;
                _q->framesyncstats.check         = qpacketmodem_get_crc(_q->payload_decoder);
                _q->framesyncstats.fec0          = qpacketmodem_get_fec0(_q->payload_decoder);
                _q->framesyncstats.fec1          = qpacketmodem_get_fec1(_q->payload_decoder);

                // invoke callback method
                _q->callback(_q->header_dec,
                             _q->header_valid,
                             _q->payload_dec,
                             _q->payload_dec_len,
                             _q->payload_valid,
                             _q->framesyncstats,
                             _q->userdata);
            }

            // reset frame synchronizer
            return flexframesync_reset(_q);
        }
    }
    return LIQUID_OK;
}

// reset frame data statistics
int flexframesync_reset_framedatastats(flexframesync _q)
{
    return framedatastats_reset(&_q->framedatastats);
}

// retrieve frame data statistics
framedatastats_s flexframesync_get_framedatastats(flexframesync _q)
{
    return _q->framedatastats;
}

// enable debugging
int flexframesync_debug_enable(flexframesync _q)
{
    // create debugging objects if necessary
#if DEBUG_FLEXFRAMESYNC
    if (_q->debug_objects_created)
        return LIQUID_OK;

    // create debugging objects
    _q->debug_x = windowcf_create(DEBUG_BUFFER_LEN);

    // set debugging flags
    _q->debug_enabled = 1;
    _q->debug_objects_created = 1;
    return LIQUID_OK;
#else
    return liquid_error(LIQUID_EICONFIG,"flexframesync_debug_enable(): compile-time debugging disabled");
#endif
}

// disable debugging
int flexframesync_debug_disable(flexframesync _q)
{
    // disable debugging
#if DEBUG_FLEXFRAMESYNC
    _q->debug_enabled = 0;
    return LIQUID_OK;
#else
    return liquid_error(LIQUID_EICONFIG,"flexframesync_debug_enable(): compile-time debugging disabled");
#endif
}

// print debugging information
int flexframesync_debug_print(flexframesync _q,
                              const char *  _filename)
{
#if DEBUG_FLEXFRAMESYNC
    if (!_q->debug_objects_created)
        return liquid_error(LIQUID_EICONFIG,"flexframesync_debug_print(), debugging objects don't exist; enable debugging first");

    unsigned int i;
    float complex * rc;
    FILE* fid = fopen(_filename,"w");
    if (fid==NULL)
        return liquid_error(LIQUID_EIO,"flexframesync_debug_print(), could not open '%s' for writing", _filename);
    fprintf(fid,"%% %s: auto-generated file", _filename);
    fprintf(fid,"\n\n");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"n = %u;\n", DEBUG_BUFFER_LEN);
    
    // main figure
    fprintf(fid,"figure('Color','white','position',[100 100 800 600]);\n");

    // write x
    fprintf(fid,"x = zeros(1,n);\n");
    windowcf_read(_q->debug_x, &rc);
    for (i=0; i<DEBUG_BUFFER_LEN; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));
    fprintf(fid,"\n\n");
    fprintf(fid,"subplot(3,2,1:2);\n");
    fprintf(fid,"plot(1:length(x),real(x), 1:length(x),imag(x));\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('sample index');\n");
    fprintf(fid,"ylabel('received signal, x');\n");

    // write p/n sequence
    fprintf(fid,"preamble_pn = zeros(1,64);\n");
    rc = _q->preamble_pn;
    for (i=0; i<64; i++)
        fprintf(fid,"preamble_pn(%4u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));

    // write p/n symbols
    fprintf(fid,"preamble_rx = zeros(1,64);\n");
    rc = _q->preamble_rx;
    for (i=0; i<64; i++)
        fprintf(fid,"preamble_rx(%4u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));

    // write recovered header symbols (after qpilotsync)
    fprintf(fid,"header_mod = zeros(1,%u);\n", _q->header_mod_len);
    rc = _q->header_mod;
    for (i=0; i<_q->header_mod_len; i++)
        fprintf(fid,"header_mod(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));

    // write raw payload symbols
    fprintf(fid,"payload_sym = zeros(1,%u);\n", _q->payload_sym_len);
    rc = _q->payload_sym;
    for (i=0; i<_q->payload_sym_len; i++)
        fprintf(fid,"payload_sym(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rc[i]), cimagf(rc[i]));

    fprintf(fid,"subplot(3,2,[3 5]);\n");
    fprintf(fid,"plot(real(header_mod),imag(header_mod),'o');\n");
    fprintf(fid,"xlabel('in-phase');\n");
    fprintf(fid,"ylabel('quadrature phase');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"title('Received Header Symbols');\n");

    fprintf(fid,"subplot(3,2,[4 6]);\n");
    fprintf(fid,"plot(real(payload_sym),imag(payload_sym),'+');\n");
    fprintf(fid,"xlabel('in-phase');\n");
    fprintf(fid,"ylabel('quadrature phase');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"title('Received Payload Symbols');\n");

    fprintf(fid,"\n\n");
    fclose(fid);

    printf("flexframesync/debug: results written to %s\n", _filename);
#else
    return liquid_error(LIQUID_EICONFIG,"flexframesync_debug_print(): compile-time debugging disabled");
#endif
    return LIQUID_OK;
}

