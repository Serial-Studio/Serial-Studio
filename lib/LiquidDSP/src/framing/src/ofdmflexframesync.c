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

#define DEBUG_OFDMFLEXFRAMESYNC 0

#define OFDMFLEXFRAME_P_SOFT (1)

// 
// ofdmflexframesync
//

// internal callback
int ofdmflexframesync_internal_callback(float complex * _X,
                                        unsigned char * _p,
                                        unsigned int    _M,
                                        void * _userdata);

// receive header data
int ofdmflexframesync_rxheader(ofdmflexframesync _q,
                               float complex * _X);

// decode header
int ofdmflexframesync_decode_header(ofdmflexframesync _q);

// receive payload data
int ofdmflexframesync_rxpayload(ofdmflexframesync _q,
                                float complex * _X);

static ofdmflexframegenprops_s ofdmflexframesyncprops_header_default = {
    OFDMFLEXFRAME_H_CRC,
    OFDMFLEXFRAME_H_FEC0,
    OFDMFLEXFRAME_H_FEC1,
    OFDMFLEXFRAME_H_MOD,
};

struct ofdmflexframesync_s {
    unsigned int M;         // number of subcarriers
    unsigned int cp_len;    // cyclic prefix length
    unsigned int taper_len; // taper length
    unsigned char * p;      // subcarrier allocation (null, pilot, data)

    // constants
    unsigned int M_null;    // number of null subcarriers
    unsigned int M_pilot;   // number of pilot subcarriers
    unsigned int M_data;    // number of data subcarriers
    unsigned int M_S0;      // number of enabled subcarriers in S0
    unsigned int M_S1;      // number of enabled subcarriers in S1

    // header
    int header_soft;                    // perform soft demod of header
    modemcf mod_header;                 // header modulator
    packetizer p_header;                // header packetizer
    unsigned char * header;             // header data (uncoded)
    unsigned char * header_enc;         // header data (encoded)
    unsigned char * header_mod;         // header symbols
    unsigned int header_user_len;       // header length (user)
    unsigned int header_dec_len;        // header length (uncoded)
    unsigned int header_enc_len;        // header length (encoded)
    unsigned int header_sym_len;        // header length (symbols)
    int header_valid;                   // valid header flag

    ofdmflexframegenprops_s header_props; // header properties

    // header properties
    modulation_scheme ms_payload;       // payload modulation scheme
    unsigned int bps_payload;           // payload modulation depth (bits/symbol)
    unsigned int payload_len;           // payload length (number of bytes)
    crc_scheme check;                   // payload validity check
    fec_scheme fec0;                    // payload FEC (inner)
    fec_scheme fec1;                    // payload FEC (outer)

    // payload
    int payload_soft;                   // perform soft demod of payload
    packetizer p_payload;               // payload packetizer
    modemcf mod_payload;                // payload demodulator
    unsigned char * payload_enc;        // payload data (encoded bytes)
    unsigned char * payload_dec;        // payload data (decoded bytes)
    unsigned int payload_enc_len;       // length of encoded payload
    unsigned int payload_mod_len;       // number of payload modem symbols
    int payload_valid;                  // valid payload flag
    float complex * payload_syms;       // received payload symbols

    // callback
    framesync_callback callback;        // user-defined callback function
    void * userdata;                    // user-defined data structure
    framesyncstats_s    framesyncstats; // frame statistic object (synchronizer, evm, etc.)
    framedatastats_s    framedatastats; // frame statistic object (packet statistics)
    float evm_hat;                      // average error vector magnitude

    // internal synchronizer objects
    ofdmframesync fs;                   // internal OFDM frame synchronizer

    // counters/states
    unsigned int symbol_counter;        // received symbol number
    enum {
        OFDMFLEXFRAMESYNC_STATE_HEADER, // extract header
        OFDMFLEXFRAMESYNC_STATE_PAYLOAD // extract payload symbols
    } state;
    unsigned int header_symbol_index;   // number of header symbols received
    unsigned int payload_symbol_index;  // number of payload symbols received
    unsigned int payload_buffer_index;  // bit-level index of payload (pack array)
};

// create ofdmflexframesync object
//  _M          :   number of subcarriers
//  _cp_len     :   length of cyclic prefix [samples]
//  _taper_len  :   taper length (OFDM symbol overlap)
//  _p          :   subcarrier allocation (PILOT/NULL/DATA) [size: _M x 1]
//  _callback   :   user-defined callback function
//  _userdata   :   user-defined data structure passed to callback
ofdmflexframesync ofdmflexframesync_create(unsigned int       _M,
                                           unsigned int       _cp_len,
                                           unsigned int       _taper_len,
                                           unsigned char *    _p,
                                           framesync_callback _callback,
                                           void *             _userdata)
{
    // validate input
    if (_M < 8)
        return liquid_error_config("ofdmflexframesync_create(), number of subcarriers must be at least 8");
    if (_M % 2)
        return liquid_error_config("ofdmflexframesync_create(), number of subcarriers must be even");
    if (_cp_len > _M)
        return liquid_error_config("ofdmflexframesync_create(), cyclic prefix length cannot exceed number of subcarriers");

    // allocate memory for object and set internal properties
    ofdmflexframesync q = (ofdmflexframesync) malloc(sizeof(struct ofdmflexframesync_s));
    q->M         = _M;
    q->cp_len    = _cp_len;
    q->taper_len = _taper_len;
    q->callback  = _callback;
    q->userdata  = _userdata;

    // allocate memory for subcarrier allocation IDs
    q->p = (unsigned char*) malloc((q->M)*sizeof(unsigned char));
    if (_p == NULL) {
        // initialize default subcarrier allocation
        ofdmframe_init_default_sctype(q->M, q->p);
    } else {
        // copy user-defined subcarrier allocation
        memmove(q->p, _p, q->M*sizeof(unsigned char));
    }

    // validate and count subcarrier allocation
    ofdmframe_validate_sctype(q->p, q->M, &q->M_null, &q->M_pilot, &q->M_data);

    // create internal framing object
    q->fs = ofdmframesync_create(_M, _cp_len, _taper_len, _p, ofdmflexframesync_internal_callback, (void*)q);

    // create header objects
    q->header = NULL;
    q->p_header = NULL;
    q->header_enc = NULL;
    q->header_mod = NULL;
    q->mod_header = NULL;
    q->header_user_len = OFDMFLEXFRAME_H_USER_DEFAULT;
    q->header_soft = 0;
    ofdmflexframesync_set_header_props(q, NULL);

    // frame properties (default values to be overwritten when frame
    // header is received and properly decoded)
    q->ms_payload   = LIQUID_MODEM_QPSK;
    q->bps_payload  = 2;
    q->payload_len  = 1;
    q->check        = LIQUID_CRC_NONE;
    q->fec0         = LIQUID_FEC_NONE;
    q->fec1         = LIQUID_FEC_NONE;

    // create payload objects (initially QPSK, etc but overridden by received properties)
    q->mod_payload = modemcf_create(q->ms_payload);
    q->payload_soft = 0;
    q->p_payload   = packetizer_create(q->payload_len, q->check, q->fec0, q->fec1);
    q->payload_enc_len = packetizer_get_enc_msg_len(q->p_payload);
    q->payload_enc = (unsigned char*) malloc(q->payload_enc_len*sizeof(unsigned char));
    q->payload_dec = (unsigned char*) malloc(q->payload_len*sizeof(unsigned char));
    q->payload_syms = (float complex *) malloc(q->payload_len*sizeof(float complex));
    q->payload_mod_len = 0;

    // reset state
    ofdmflexframesync_reset_framedatastats(q);
    ofdmflexframesync_reset(q);

    // return object
    return q;
}

int ofdmflexframesync_destroy(ofdmflexframesync _q)
{
    // destroy internal objects
    ofdmframesync_destroy(_q->fs);
    packetizer_destroy(_q->p_header);
    modemcf_destroy(_q->mod_header);
    packetizer_destroy(_q->p_payload);
    modemcf_destroy(_q->mod_payload);

    // free internal buffers/arrays
    free(_q->p);
    free(_q->payload_enc);
    free(_q->payload_dec);
    free(_q->payload_syms);
    free(_q->header);
    free(_q->header_enc);
    free(_q->header_mod);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

int ofdmflexframesync_print(ofdmflexframesync _q)
{
    printf("<liquid.ofdmflexframesync");
    printf(", subcarriers=%u", _q->M);
    printf(", null=%u", _q->M_null);
    printf(", pilot=%u", _q->M_pilot);
    printf(", data=%u", _q->M_data);
    printf(", cp=%u", _q->cp_len);
    printf(", taper=%u", _q->taper_len);
    printf(">\n");
    return LIQUID_OK;
}

int ofdmflexframesync_set_header_len(ofdmflexframesync _q,
                                     unsigned int     _len)
{
    _q->header_user_len = _len;
    _q->header_dec_len = OFDMFLEXFRAME_H_DEC + _q->header_user_len;
    _q->header = realloc(_q->header, _q->header_dec_len*sizeof(unsigned char));

    if (_q->p_header) {
        packetizer_destroy(_q->p_header);
    }
    _q->p_header = packetizer_create(_q->header_dec_len,
                                     _q->header_props.check,
                                     _q->header_props.fec0,
                                     _q->header_props.fec1);
    if (_q->header_soft) {
        _q->header_enc_len = 8*packetizer_get_enc_msg_len(_q->p_header);
        _q->header_sym_len = _q->header_enc_len;
    } else {
        _q->header_enc_len = packetizer_get_enc_msg_len(_q->p_header);
        unsigned int bps = modulation_types[_q->header_props.mod_scheme].bps;
        div_t bps_d = div(_q->header_enc_len*8, bps);
        _q->header_sym_len = bps_d.quot + (bps_d.rem ? 1 : 0);
    }
    _q->header_enc = realloc(_q->header_enc, _q->header_enc_len*sizeof(unsigned char));

    _q->header_mod = realloc(_q->header_mod, _q->header_sym_len*sizeof(unsigned char));
    // create header objects
    if (_q->mod_header) {
        modemcf_destroy(_q->mod_header);
    }
    _q->mod_header = modemcf_create(_q->header_props.mod_scheme);
    return LIQUID_OK;
}

int ofdmflexframesync_decode_header_soft(ofdmflexframesync _q,
                                         int _soft)
{
    _q->header_soft = _soft;
    return ofdmflexframesync_set_header_len(_q, _q->header_user_len);
}

int ofdmflexframesync_decode_payload_soft(ofdmflexframesync _q,
                                          int _soft)
{
    _q->payload_soft = _soft;
    return LIQUID_OK;
}

int ofdmflexframesync_set_header_props(ofdmflexframesync _q,
                                       ofdmflexframegenprops_s * _props)
{
    // if properties object is NULL, initialize with defaults
    if (_props == NULL) {
        _props = &ofdmflexframesyncprops_header_default;
    }

    // validate input
    if (_props->check == LIQUID_CRC_UNKNOWN || _props->check >= LIQUID_CRC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_set_header_props(), invalid/unsupported CRC scheme");
    if (_props->fec0 == LIQUID_FEC_UNKNOWN || _props->fec1 == LIQUID_FEC_UNKNOWN)
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_set_header_props(), invalid/unsupported FEC scheme");
    if (_props->mod_scheme == LIQUID_MODEM_UNKNOWN )
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_set_header_props(), invalid/unsupported modulation scheme");

    // copy properties to internal structure
    memmove(&_q->header_props, _props, sizeof(ofdmflexframegenprops_s));

    // reconfigure internal buffers, objects, etc.
    return ofdmflexframesync_set_header_len(_q, _q->header_user_len);
}

int ofdmflexframesync_reset(ofdmflexframesync _q)
{
    // reset internal state
    _q->state = OFDMFLEXFRAMESYNC_STATE_HEADER;

    // reset internal counters
    _q->symbol_counter=0;
    _q->header_symbol_index=0;
    _q->payload_symbol_index=0;
    _q->payload_buffer_index=0;
    
    // reset error vector magnitude estimate
    _q->evm_hat = 1e-12f;   // slight offset to ensure no log(0)

    // reset global data counters
    framesyncstats_init_default(&_q->framesyncstats);

    // reset internal OFDM frame synchronizer object
    return ofdmframesync_reset(_q->fs);
}

// set the callback
int ofdmflexframesync_set_callback(ofdmflexframesync  _q,
                                   framesync_callback _callback)
{
    _q->callback = _callback;
    return LIQUID_OK;
}

// set the user-defined data field (context)
int ofdmflexframesync_set_userdata(ofdmflexframesync _q,
                                   void *            _userdata)
{
    _q->userdata = _userdata;
    return LIQUID_OK;
}

int ofdmflexframesync_is_frame_open(ofdmflexframesync _q)
{
    return ofdmframesync_is_frame_open(_q->fs);
}

// execute synchronizer object on buffer of samples
int ofdmflexframesync_execute(ofdmflexframesync _q,
                              float complex *   _x,
                              unsigned int      _n)
{
    // push samples through ofdmframesync object
    return ofdmframesync_execute(_q->fs, _x, _n);
}

// 
// query methods
//

// received signal strength indication
float ofdmflexframesync_get_rssi(ofdmflexframesync _q)
{
    return ofdmframesync_get_rssi(_q->fs);
}

// received carrier frequency offset
float ofdmflexframesync_get_cfo(ofdmflexframesync _q)
{
    return ofdmframesync_get_cfo(_q->fs);
}

// reset frame data statistics
int ofdmflexframesync_reset_framedatastats(ofdmflexframesync _q)
{
    return framedatastats_reset(&_q->framedatastats);
}

// retrieve frame data statistics
framedatastats_s ofdmflexframesync_get_framedatastats(ofdmflexframesync _q)
{
    return _q->framedatastats;
}

//
// set methods
//

// received carrier frequency offset
int ofdmflexframesync_set_cfo(ofdmflexframesync _q, float _cfo)
{
    return ofdmframesync_set_cfo(_q->fs, _cfo);
}

// 
// debugging methods
//

// enable debugging for internal ofdm frame synchronizer
int ofdmflexframesync_debug_enable(ofdmflexframesync _q)
{
    return ofdmframesync_debug_enable(_q->fs);
}

// disable debugging for internal ofdm frame synchronizer
int ofdmflexframesync_debug_disable(ofdmflexframesync _q)
{
    return ofdmframesync_debug_disable(_q->fs);
}

// print debugging file for internal ofdm frame synchronizer
int ofdmflexframesync_debug_print(ofdmflexframesync _q,
                                   const char *      _filename)
{
    return ofdmframesync_debug_print(_q->fs, _filename);
}

//
// internal methods
//

// internal callback
//  _X          :   subcarrier symbols
//  _p          :   subcarrier allocation
//  _M          :   number of subcarriers
//  _userdata   :   user-defined data structure
int ofdmflexframesync_internal_callback(float complex * _X,
                                        unsigned char * _p,
                                        unsigned int    _M,
                                        void *          _userdata)
{
#if DEBUG_OFDMFLEXFRAMESYNC
    printf("******* ofdmflexframesync callback invoked!\n");
#endif
    // type-cast userdata as ofdmflexframesync object
    ofdmflexframesync _q = (ofdmflexframesync) _userdata;

    _q->symbol_counter++;

#if DEBUG_OFDMFLEXFRAMESYNC
    printf("received symbol %u\n", _q->symbol_counter);
#endif

    // extract symbols
    switch (_q->state) {
    case OFDMFLEXFRAMESYNC_STATE_HEADER:  return ofdmflexframesync_rxheader (_q, _X);
    case OFDMFLEXFRAMESYNC_STATE_PAYLOAD: return ofdmflexframesync_rxpayload(_q, _X);
    default:;
    }

    // return
    return liquid_error(LIQUID_EINT,"ofdmflexframesync_internal_callback(), invalid internal state");
}

// receive header data
int ofdmflexframesync_rxheader(ofdmflexframesync _q,
                               float complex * _X)
{
#if DEBUG_OFDMFLEXFRAMESYNC
    printf("  ofdmflexframesync extracting header...\n");
#endif

    // demodulate header symbols
    unsigned int i;
    int sctype;
    for (i=0; i<_q->M; i++) {
        // subcarrier type (PILOT/NULL/DATA)
        sctype = _q->p[i];

        // ignore pilot and null subcarriers
        if (sctype == OFDMFRAME_SCTYPE_DATA) {
            // unload header symbols
            // demodulate header symbol
            unsigned int sym;
            if (_q->header_soft) {
                unsigned int bps = modulation_types[_q->header_props.mod_scheme].bps;
                modemcf_demodulate_soft(_q->mod_header, _X[i], &sym, &_q->header_mod[bps*_q->header_symbol_index]);
            } else {
                modemcf_demodulate(_q->mod_header, _X[i], &sym);
                _q->header_mod[_q->header_symbol_index] = sym;
            }
            _q->header_symbol_index++;
            //printf("  extracting symbol %3u / %3u (x = %8.5f + j%8.5f)\n", _q->header_symbol_index, _q->header_sym_len, crealf(_X[i]), cimagf(_X[i]));

            // get demodulator error vector magnitude
            float evm = modemcf_get_demodulator_evm(_q->mod_header);
            _q->evm_hat += evm*evm;

            // header extracted
            if (_q->header_symbol_index == _q->header_sym_len) {
                // decode header
                ofdmflexframesync_decode_header(_q);
            
                // update statistics
                _q->framesyncstats.evm = 10*log10f( _q->evm_hat/_q->header_sym_len );
                _q->framedatastats.num_frames_detected++;

                // invoke callback if header is invalid
                if (_q->header_valid) {
                    _q->state = OFDMFLEXFRAMESYNC_STATE_PAYLOAD;
                    _q->framedatastats.num_headers_valid++;
                } else {
                    //printf("**** header invalid!\n");
                    // set framesyncstats internals
                    _q->framesyncstats.rssi          = ofdmframesync_get_rssi(_q->fs);
                    _q->framesyncstats.cfo           = ofdmframesync_get_cfo(_q->fs);
                    _q->framesyncstats.framesyms     = NULL;
                    _q->framesyncstats.num_framesyms = 0;
                    _q->framesyncstats.mod_scheme    = LIQUID_MODEM_UNKNOWN;
                    _q->framesyncstats.mod_bps       = 0;
                    _q->framesyncstats.check         = LIQUID_CRC_UNKNOWN;
                    _q->framesyncstats.fec0          = LIQUID_FEC_UNKNOWN;
                    _q->framesyncstats.fec1          = LIQUID_FEC_UNKNOWN;

                    // ignore callback if set to NULL
                    if (_q->callback == NULL) {
                        ofdmflexframesync_reset(_q);
                        break;
                    }

                    // invoke callback method
                    _q->callback(_q->header,
                                 _q->header_valid,
                                 NULL,
                                 0,
                                 0,
                                 _q->framesyncstats,
                                 _q->userdata);

                    ofdmflexframesync_reset(_q);
                }
                break;
            }
        }
    }
    return LIQUID_OK;
}

// decode header
int ofdmflexframesync_decode_header(ofdmflexframesync _q)
{
    _q->header_valid = 0;
    int header_ok = 0;
    if (_q->header_soft) {
        // TODO: ensure lengths are the same
        memmove(_q->header_enc, _q->header_mod, _q->header_enc_len);

        // unscramble header using soft bits
        unscramble_data_soft(_q->header_enc, _q->header_enc_len/8);

        // run packet decoder
        header_ok = packetizer_decode_soft(_q->p_header, _q->header_enc, _q->header);
    } else {
        // pack 1-bit header symbols into 8-bit bytes
        unsigned int num_written;
        unsigned int bps = modulation_types[_q->header_props.mod_scheme].bps;
        liquid_repack_bytes(_q->header_mod, bps, _q->header_sym_len,
                            _q->header_enc, 8,   _q->header_enc_len,
                            &num_written);
        assert(num_written==_q->header_enc_len);

        // unscramble header
        unscramble_data(_q->header_enc, _q->header_enc_len);

        // run packet decoder
        header_ok = packetizer_decode(_q->p_header, _q->header_enc, _q->header);
    }

#if 0
    // print header
    printf("header rx (enc) : ");
    for (i=0; i<_q->header_enc_len; i++)
        printf("%.2X ", _q->header_enc[i]);
    printf("\n");

    // print header
    printf("header rx (dec) : ");
    for (i=0; i<_q->header_dec_len; i++)
        printf("%.2X ", _q->header[i]);
    printf("\n");
#endif

#if DEBUG_OFDMFLEXFRAMESYNC
    printf("****** header extracted [%s]\n", header_ok ? "valid" : "INVALID!");
#endif
    if (!header_ok)
        return LIQUID_OK;

    unsigned int n = _q->header_user_len;

    // first byte is for expansion/version validation
    if (_q->header[n+0] != OFDMFLEXFRAME_PROTOCOL)
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_decode_header(), invalid framing version");

    // strip off payload length
    unsigned int payload_len = (_q->header[n+1] << 8) | (_q->header[n+2]);

    // strip off modulation scheme/depth
    unsigned int mod_scheme = _q->header[n+3];
    if (mod_scheme == 0 || mod_scheme >= LIQUID_MODEM_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_decode_header(), invalid modulation scheme");

    // strip off CRC, forward error-correction schemes
    //  CRC     : most-significant 3 bits of [n+4]
    //  fec0    : least-significant 5 bits of [n+4]
    //  fec1    : least-significant 5 bits of [n+5]
    unsigned int check = (_q->header[n+4] >> 5 ) & 0x07;
    unsigned int fec0  = (_q->header[n+4]      ) & 0x1f;
    unsigned int fec1  = (_q->header[n+5]      ) & 0x1f;

    // validate properties
    if (check >= LIQUID_CRC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_decode_header(), decoded CRC exceeds available");
    if (fec0 >= LIQUID_FEC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_decode_header(), decoded FEC (inner) exceeds available");
    if (fec1 >= LIQUID_FEC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"ofdmflexframesync_decode_header(), decoded FEC (outer) exceeds available");

    // print results
#if DEBUG_OFDMFLEXFRAMESYNC
    printf("    properties:\n");
    printf("      * mod scheme      :   %s\n", modulation_types[mod_scheme].fullname);
    printf("      * fec (inner)     :   %s\n", fec_scheme_str[fec0][1]);
    printf("      * fec (outer)     :   %s\n", fec_scheme_str[fec1][1]);
    printf("      * CRC scheme      :   %s\n", crc_scheme_str[check][1]);
    printf("      * payload length  :   %u bytes\n", payload_len);
#endif

    // at this point, header is valid
    _q->header_valid = 1;

    // configure modem
    if (mod_scheme != _q->ms_payload) {
        // set new properties
        _q->ms_payload  = mod_scheme;
        _q->bps_payload = modulation_types[mod_scheme].bps;

        // recreate modem (destroy/create)
        _q->mod_payload = modemcf_recreate(_q->mod_payload, _q->ms_payload);
    }

    // set new packetizer properties
    _q->payload_len = payload_len;
    _q->check       = check;
    _q->fec0        = fec0;
    _q->fec1        = fec1;
    
    // recreate packetizer object
    _q->p_payload = packetizer_recreate(_q->p_payload,
                                        _q->payload_len,
                                        _q->check,
                                        _q->fec0,
                                        _q->fec1);

    // re-compute payload encoded message length
    if (_q->payload_soft) {
        int packetizer_msg_len = packetizer_get_enc_msg_len(_q->p_payload);
        div_t d = div((int)8*packetizer_msg_len, (int)_q->bps_payload);
        _q->payload_mod_len = d.quot + (d.rem ? 1 : 0);
        _q->payload_enc_len = _q->bps_payload*_q->payload_mod_len;
    } else {
        _q->payload_enc_len = packetizer_get_enc_msg_len(_q->p_payload);
        // re-compute number of modulated payload symbols
        div_t d = div(8*_q->payload_enc_len, _q->bps_payload);
        _q->payload_mod_len = d.quot + (d.rem ? 1 : 0);
    }

#if DEBUG_OFDMFLEXFRAMESYNC
    printf("      * payload encoded :   %u bytes\n", _q->payload_enc_len);
#endif

    // re-allocate buffers accordingly
    _q->payload_enc = (unsigned char*) realloc(_q->payload_enc, _q->payload_enc_len*sizeof(unsigned char));
    _q->payload_dec = (unsigned char*) realloc(_q->payload_dec, _q->payload_len*sizeof(unsigned char));
    _q->payload_syms = (float complex*) realloc(_q->payload_syms, _q->payload_mod_len*sizeof(float complex));
#if DEBUG_OFDMFLEXFRAMESYNC
    printf("      * payload mod syms:   %u symbols\n", _q->payload_mod_len);
#endif
    return LIQUID_OK;
}

// receive payload data
int ofdmflexframesync_rxpayload(ofdmflexframesync _q,
                                float complex * _X)
{
    // demodulate paylod symbols
    unsigned int i;
    int sctype;
    for (i=0; i<_q->M; i++) {
        // subcarrier type (PILOT/NULL/DATA)
        sctype = _q->p[i];

        // ignore pilot and null subcarriers
        if (sctype == OFDMFRAME_SCTYPE_DATA) {
            // unload payload symbols
            unsigned int sym;
            // store received symbol
            _q->payload_syms[_q->payload_symbol_index] = _X[i];

            if (_q->payload_soft) {
                modemcf_demodulate_soft(_q->mod_payload, _X[i], &sym, &_q->payload_enc[_q->bps_payload*_q->payload_symbol_index]);
            } else {
                modemcf_demodulate(_q->mod_payload, _X[i], &sym);

                // pack decoded symbol into array
                liquid_pack_array(_q->payload_enc,
                                  _q->payload_enc_len,
                                  _q->payload_buffer_index,
                                  _q->bps_payload,
                                  sym);

                // increment...
                _q->payload_buffer_index += _q->bps_payload;
            }

            // increment symbol counter
            _q->payload_symbol_index++;

            if (_q->payload_symbol_index == _q->payload_mod_len) {
                // payload extracted
                if (_q->payload_soft) {
                    _q->payload_valid = packetizer_decode_soft(_q->p_payload, _q->payload_enc, _q->payload_dec);
                } else {
                    // decode payload
                    _q->payload_valid = packetizer_decode(_q->p_payload, _q->payload_enc, _q->payload_dec);
                }
#if DEBUG_OFDMFLEXFRAMESYNC
                printf("****** payload extracted [%s]\n", _q->payload_valid ? "valid" : "INVALID!");
#endif
                // update statistics
                _q->framedatastats.num_payloads_valid += _q->payload_valid;
                _q->framedatastats.num_bytes_received += _q->payload_len;

                // set framesyncstats internals
                _q->framesyncstats.rssi          = ofdmframesync_get_rssi(_q->fs);
                _q->framesyncstats.cfo           = ofdmframesync_get_cfo(_q->fs);
                _q->framesyncstats.framesyms     = _q->payload_syms;
                _q->framesyncstats.num_framesyms = _q->payload_mod_len;
                _q->framesyncstats.mod_scheme    = _q->ms_payload;
                _q->framesyncstats.mod_bps       = _q->bps_payload;
                _q->framesyncstats.check         = _q->check;
                _q->framesyncstats.fec0          = _q->fec0;
                _q->framesyncstats.fec1          = _q->fec1;

                // ignore callback if set to NULL
                if (_q->callback == NULL) {
                    ofdmflexframesync_reset(_q);
                    break;
                }

                // invoke callback method
                _q->callback(_q->header,
                             _q->header_valid,
                             _q->payload_dec,
                             _q->payload_len,
                             _q->payload_valid,
                             _q->framesyncstats,
                             _q->userdata);


                // reset object
                ofdmflexframesync_reset(_q);
                break;
            }
        }
    }
    return LIQUID_OK;
}



