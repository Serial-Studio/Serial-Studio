/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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

// basic frame synchronizer with 8 bytes header and 64 bytes payload

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <assert.h>

#include "liquid.internal.h"

// synchronization callback, return 0:continue, 1:reset
int framesync64_callback_internal(float complex * _buf,
                                  unsigned int    _buf_len,
                                  void *          _context);

// export debugging based on return value
//  0   : do not write file
//  >0  : write specific number (hex)
//  -1  : number of packets detected
//  -2  : id using first 4 bytes of header
//  -3  : write with random extension
int framesync64_debug_export(framesync64 _q, int _code, float complex * _payload_rx);

// framesync64 object structure
struct framesync64_s {
    // callback
    framesync_callback  callback;       // user-defined callback function
    void *              userdata;       // user-defined data structure
    framesyncstats_s    framesyncstats; // frame statistic object (synchronizer)
    framedatastats_s    framedatastats; // frame statistic object (packet statistics)

    // synchronizer objects
    unsigned int        m;              // filter delay (symbols)
    float               beta;           // filter excess bandwidth factor
    qdsync_cccf         sync;           // sequence detector

    // preamble
    float complex preamble_pn[64];      // known 64-symbol p/n sequence
    float complex preamble_rx[64];      // received p/n symbols

    // payload decoder
    float complex payload_sym[600];     // received payload symbols
    unsigned char payload_dec[ 72];     // decoded payload bytes
    qpacketmodem  dec;                  // packet demodulator/decoder
    qpilotsync    pilotsync;            // pilot extraction, carrier recovery
    int           payload_valid;        // did payload pass crc?

    windowcf     buf_debug;             // debug: raw input samples
    char *       prefix;                // debug: filename prefix
    char *       filename;              // debug: filename buffer
    unsigned int num_files_exported;    // debug: number of files exported
};

// create framesync64 object
//  _callback       :   callback function invoked when frame is received
//  _userdata       :   user-defined data object passed to callback
framesync64 framesync64_create(framesync_callback _callback,
                               void *             _userdata)
{
    framesync64 q = (framesync64) malloc(sizeof(struct framesync64_s));
    q->callback = _callback;
    q->userdata = _userdata;
    q->m        = 7;    // filter delay (symbols)
    q->beta     = 0.3f; // excess bandwidth factor

    unsigned int i;

    // generate p/n sequence
    msequence ms = msequence_create(7, 0x0089, 1);
    for (i=0; i<64; i++) {
        q->preamble_pn[i]  = (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2);
        q->preamble_pn[i] += (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2)*_Complex_I;
    }
    msequence_destroy(ms);

    // create frame detector with callback to run frame sync in one step
    q->sync = qdsync_cccf_create_linear(q->preamble_pn, 64,
        LIQUID_FIRFILT_ARKAISER, 2, q->m, q->beta, framesync64_callback_internal, q);
    qdsync_cccf_set_buf_len(q->sync, 64 + 630);

    // create payload demodulator/decoder object
    int check      = LIQUID_CRC_24;
    int fec0       = LIQUID_FEC_NONE;
    int fec1       = LIQUID_FEC_GOLAY2412;
    int mod_scheme = LIQUID_MODEM_QPSK;
    q->dec         = qpacketmodem_create();
    qpacketmodem_configure(q->dec, 72, check, fec0, fec1, mod_scheme);
    assert( qpacketmodem_get_frame_len(q->dec)==600 );

    // create pilot synchronizer
    q->pilotsync   = qpilotsync_create(600, 21);
    assert( qpilotsync_get_frame_len(q->pilotsync)==630);

    // reset global data counters
    framesync64_reset_framedatastats(q);

    // set debugging fields
    q->buf_debug= windowcf_create(LIQUID_FRAME64_LEN);
    q->prefix   = NULL;
    q->filename = NULL;
    q->num_files_exported = 0;
    framesync64_set_prefix(q, "framesync64");

    // reset state and return
    framesync64_reset(q);
    return q;
}

// copy object
framesync64 framesync64_copy(framesync64 q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("framesync64_copy(), object cannot be NULL");

    // allocate memory for new object
    framesync64 q_copy = (framesync64) malloc(sizeof(struct framesync64_s));

    // copy entire memory space over and overwrite values as needed
    memmove(q_copy, q_orig, sizeof(struct framesync64_s));

    // set callback and userdata fields
    //q_copy->callback = q_orig->callback;
    //q_copy->userdata = q_orig->userdata;

    // copy objects
    q_copy->sync     = qdsync_cccf_copy (q_orig->sync);
    q_copy->dec      = qpacketmodem_copy(q_orig->dec);
    q_copy->pilotsync= qpilotsync_copy  (q_orig->pilotsync);
    q_copy->buf_debug= windowcf_copy    (q_orig->buf_debug);

    // set prefix value
    q_copy->prefix   = NULL;
    q_copy->filename = NULL;
    framesync64_set_prefix(q_copy, q_orig->prefix);

    // update the context for the sync object to that detected frames will
    // apply to this new frame synchronizer object
    qdsync_cccf_set_context(q_copy->sync, q_copy);

    return q_copy;
}

// destroy frame synchronizer object, freeing all internal memory
int framesync64_destroy(framesync64 _q)
{
    // destroy synchronization objects
    qdsync_cccf_destroy (_q->sync);      // frame detector/synchronizer
    qpacketmodem_destroy(_q->dec);       // payload demodulator
    qpilotsync_destroy  (_q->pilotsync); // pilot synchronizer
    windowcf_destroy    (_q->buf_debug);

    // free allocated buffers
    free(_q->prefix);
    free(_q->filename);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print frame synchronizer object internals
int framesync64_print(framesync64 _q)
{
    printf("<liquid.framesync64>\n");
    return LIQUID_OK;
}

// reset frame synchronizer object
int framesync64_reset(framesync64 _q)
{
    // reset synchronizer
    qdsync_cccf_reset(_q->sync);
    return LIQUID_OK;
}

// set the callback function
int framesync64_set_callback(framesync64        _q,
                             framesync_callback _callback)
{
    _q->callback = _callback;
    return LIQUID_OK;
}

// set the user-defined data field (context)
int framesync64_set_userdata(framesync64 _q,
                             void *      _userdata)
{
    _q->userdata = _userdata;
    return LIQUID_OK;
}

// execute frame synchronizer
//  _q     :   frame synchronizer object
//  _x      :   input sample array [size: _n x 1]
//  _n      :   number of input samples
int framesync64_execute(framesync64     _q,
                        float complex * _x,
                        unsigned int    _n)
{
    return qdsync_cccf_execute(_q->sync, _x, _n);
}

//
// internal methods
//

// synchronization callback, return 0:continue, 1:reset
int framesync64_callback_internal(float complex * _buf,
                                  unsigned int    _buf_len,
                                  void *          _context)
{
    // type cast context input as frame synchronizer object
    framesync64 _q = (framesync64)_context;

    // recover data symbols from pilots (input buffer with 64-symbol preamble offset)
    qpilotsync_execute(_q->pilotsync, _buf + 64, _q->payload_sym);

    // decode payload
    _q->payload_valid = qpacketmodem_decode(_q->dec,
                                            _q->payload_sym,
                                            _q->payload_dec);

    // update statistics
    _q->framedatastats.num_frames_detected++;
    _q->framedatastats.num_headers_valid  += _q->payload_valid;
    _q->framedatastats.num_payloads_valid += _q->payload_valid;
    _q->framedatastats.num_bytes_received += _q->payload_valid ? 64 : 0;

    // set framesyncstats internals
    _q->framesyncstats.evm           = qpacketmodem_get_demodulator_evm(_q->dec);
    _q->framesyncstats.rssi          = 20*log10f( qdsync_cccf_get_gamma(_q->sync) );
    _q->framesyncstats.cfo           = qdsync_cccf_get_dphi(_q->sync) + qpilotsync_get_dphi(_q->pilotsync) / 2.0f;
    _q->framesyncstats.framesyms     = _q->payload_sym;
    _q->framesyncstats.num_framesyms = 600;
    _q->framesyncstats.mod_scheme    = LIQUID_MODEM_QPSK;
    _q->framesyncstats.mod_bps       = 2;
    _q->framesyncstats.check         = LIQUID_CRC_24;
    _q->framesyncstats.fec0          = LIQUID_FEC_NONE;
    _q->framesyncstats.fec1          = LIQUID_FEC_GOLAY2412;

    // invoke callback
    if (_q->callback != NULL) {
        int rc =
        _q->callback(&_q->payload_dec[0],   // header is first 8 bytes
                     _q->payload_valid,
                     &_q->payload_dec[8],   // payload is last 64 bytes
                     64,
                     _q->payload_valid,
                     _q->framesyncstats,
                     _q->userdata);

        // export debugging based on return value
        framesync64_debug_export(_q, rc, _buf+64);
    }

    // reset frame synchronizer
    return framesync64_reset(_q);
}

// DEPRECATED: enable debugging
int framesync64_debug_enable(framesync64 _q)
{
    return LIQUID_OK;
}

// DEPRECATED: disable debugging
int framesync64_debug_disable(framesync64 _q)
{
    return LIQUID_OK;
}

// DEPRECATED: print debugging information
int framesync64_debug_print(framesync64   _q,
                             const char * _filename)
{
    return LIQUID_OK;
}

// get detection threshold
float framesync64_get_threshold(framesync64 _q)
{
    return qdsync_cccf_get_threshold(_q->sync);
}

// set detection threshold
int framesync64_set_threshold(framesync64 _q,
                              float       _threshold)
{
    return qdsync_cccf_set_threshold(_q->sync, _threshold);
}

// set prefix for exporting debugging files, default: "framesync64"
//  _q      : frame sync object
//  _prefix : string with valid file path
int framesync64_set_prefix(framesync64  _q,
                           const char * _prefix)
{
    // skip if input is NULL pointer
    if (_prefix == NULL)
        return LIQUID_OK;

    // sanity check
    unsigned int n = strlen(_prefix);
    if (n > 1<<14)
        return liquid_error(LIQUID_EICONFIG,"framesync64_set_prefix(), input string size exceeds reasonable limits");

    // reallocate memory, copy input, and return
    _q->prefix   = (char*) realloc(_q->prefix,   n+ 1);
    _q->filename = (char*) realloc(_q->filename, n+15);
    memmove(_q->prefix, _prefix, n);
    _q->prefix[n] = '\0';
    return LIQUID_OK;
}

// get prefix for exporting debugging files
const char * framesync64_get_prefix(framesync64  _q)
{
    return (const char*) _q->prefix;
}

// get number of files exported
unsigned int framesync64_get_num_files_exported(framesync64  _q)
{
    return _q->num_files_exported;
}

// get name of last debugging file written
const char * framesync64_get_filename(framesync64  _q)
{
    return _q->num_files_exported == 0 ? NULL : (const char*) _q->filename;
}

// reset frame data statistics
int framesync64_reset_framedatastats(framesync64 _q)
{
    return framedatastats_reset(&_q->framedatastats);
}

// retrieve frame data statistics
framedatastats_s framesync64_get_framedatastats(framesync64 _q)
{
    return _q->framedatastats;
}

// export debugging samples to file
int framesync64_debug_export(framesync64     _q,
                             int             _code,
                             float complex * _payload_rx)
{
    // determine what to do based on callback return code
    if (_code == 0) {
        // do not export file
        return LIQUID_OK;
    } else if (_code > 0) {
        // user-defined value
        sprintf(_q->filename,"%s_u%.8x.dat", _q->prefix, _code);
    } else if (_code == -1) {
        // based on number of packets detected
        sprintf(_q->filename,"%s_n%.8x.dat", _q->prefix,
                _q->framedatastats.num_frames_detected);
    } else if (_code == -2) {
        // decoded header (first 4 bytes)
        sprintf(_q->filename,"%s_h", _q->prefix);
        char * p = _q->filename + strlen(_q->prefix) + 2;
        for (unsigned int i=0; i<4; i++) {
            sprintf(p,"%.2x", _q->payload_dec[i]);
            p += 2;
        }
        sprintf(p,".dat");
    } else if (_code == -3) {
        // random extension
        sprintf(_q->filename,"%s_r%.8x.dat", _q->prefix, rand() & 0xffffffff);
    } else {
        return liquid_error(LIQUID_EICONFIG,"framesync64_debug_export(), invalid return code %d", _code);
    }

    FILE * fid = fopen(_q->filename,"wb");
    if (fid == NULL)
        return liquid_error(LIQUID_EIO,"framesync64_debug_export(), could not open %s for writing", _q->filename);

    // TODO: write file header?

    // write debug buffer
    float complex * rc;
    windowcf_read(_q->buf_debug, &rc);
    fwrite(rc, sizeof(float complex), LIQUID_FRAME64_LEN, fid);

    // export framesync stats
    //framesyncstats_export(_q->framesyncstats, fid);

    // export measured offsets
    float tau_hat = 0.0f;
    float phi_hat = 0.0f;
    fwrite(&tau_hat,                       sizeof(float), 1, fid);
    fwrite(&(_q->framesyncstats.cfo),      sizeof(float), 1, fid);
    fwrite(&phi_hat,                       sizeof(float), 1, fid);
    fwrite(&(_q->framesyncstats.rssi),     sizeof(float), 1, fid);
    fwrite(&(_q->framesyncstats.evm),      sizeof(float), 1, fid);

    // export payload values
    fwrite(_payload_rx,     sizeof(float complex), 630, fid);
    fwrite(_q->payload_sym, sizeof(float complex), 600, fid);
    fwrite(_q->payload_dec, sizeof(unsigned char),  72, fid);

    fclose(fid);
    _q->num_files_exported++;
    printf("framesync64_debug_export(), results written to %s (%u total)\n",
        _q->filename, _q->num_files_exported);
    return LIQUID_OK;
}

