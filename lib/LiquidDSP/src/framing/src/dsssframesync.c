/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
// dsssframesync.c
//

#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

int dsssframesync_execute_seekpn    (dsssframesync _q, float complex _x);
int dsssframesync_step              (dsssframesync _q, float complex _x, float complex * _y);
int dsssframesync_execute_rxpreamble(dsssframesync _q, float complex _x);
int dsssframesync_decode_header     (dsssframesync _q);
int dsssframesync_decode_payload    (dsssframesync _q);
int dsssframesync_execute_rxheader  (dsssframesync _q, float complex _x);
int dsssframesync_execute_rxpayload (dsssframesync _q, float complex _x);
int dsssframesync_configure_payload (dsssframesync _q);
static dsssframegenprops_s dsssframesyncprops_header_default = {
    DSSSFRAME_H_CRC,
    DSSSFRAME_H_FEC0,
    DSSSFRAME_H_FEC1,
};

enum state {
    DSSSFRAMESYNC_STATE_DETECTFRAME = 0,
    DSSSFRAMESYNC_STATE_RXPREAMBLE,
    DSSSFRAMESYNC_STATE_RXHEADER,
    DSSSFRAMESYNC_STATE_RXPAYLOAD,
};

struct dsssframesync_s {
    framesync_callback  callback;
    void *              userdata;
    framesyncstats_s    framesyncstats;
    framedatastats_s    framedatastats;

    unsigned int        k;
    unsigned int        m;
    float               beta;
    qdetector_cccf      detector;
    float               tau_hat;
    float               dphi_hat;
    float               phi_hat;
    float               gamma_hat;
    nco_crcf            mixer;
    nco_crcf            pll;

    firpfb_crcf         mf;
    unsigned int        npfb;
    int                 mf_counter;
    unsigned int        pfb_index;

    float complex *     preamble_pn;
    float complex *     preamble_rx;
    synth_crcf          header_synth;
    synth_crcf          payload_synth;

    int                 header_soft;
    flexframegenprops_s header_props;
    float complex *     header_spread;
    unsigned int        header_spread_len;
    qpacketmodem        header_decoder;
    unsigned int        header_user_len;
    unsigned int        header_dec_len;
    unsigned char *     header_dec;
    int                 header_valid;

    int                 payload_soft;
    float complex *     payload_spread;
    unsigned int        payload_spread_len;
    qpacketmodem        payload_decoder;
    unsigned int        payload_dec_len;
    unsigned char *     payload_dec;
    int                 payload_valid;

    unsigned int        preamble_counter;
    unsigned int        symbol_counter;
    enum state          state;
};

dsssframesync dsssframesync_create(framesync_callback _callback, void * _userdata)
{
    dsssframesync q = (dsssframesync)calloc(1, sizeof(struct dsssframesync_s));
    q->callback     = _callback;
    q->userdata     = _userdata;

    q->k    = 2;
    q->m    = 7;
    q->beta = 0.3f;

    unsigned int i;
    q->preamble_pn = (float complex *)calloc(64, sizeof(float complex));
    q->preamble_rx = (float complex *)calloc(64, sizeof(float complex));
    msequence ms   = msequence_create(7, 0x0089, 1);
    for (i = 0; i < 64; i++) {
        q->preamble_pn[i] = (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2);
        q->preamble_pn[i] += (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2) * _Complex_I;
    }
    msequence_destroy(ms);

    float complex * pn = (float complex *)calloc(64, sizeof(float complex));
    ms                        = msequence_create(7, 0x00cb, 0x53);
    for (i = 0; i < 64; i++) {
        pn[i] = (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2);
        pn[i] += (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2) * _Complex_I;
    }
    q->header_synth  = synth_crcf_create(pn, 64);
    q->payload_synth = synth_crcf_create(pn, 64);
    synth_crcf_pll_set_bandwidth(q->header_synth, 1e-4f);
    synth_crcf_pll_set_bandwidth(q->payload_synth, 1e-4f);
    free(pn);
    msequence_destroy(ms);

    q->detector = qdetector_cccf_create_linear(
        q->preamble_pn, 64, LIQUID_FIRFILT_ARKAISER, q->k, q->m, q->beta);
    qdetector_cccf_set_threshold(q->detector, 0.5f);

    q->npfb = 32;
    q->mf   = firpfb_crcf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, q->npfb, q->k, q->m, q->beta);

    q->mixer = nco_crcf_create(LIQUID_NCO);
    q->pll   = nco_crcf_create(LIQUID_NCO);
    nco_crcf_pll_set_bandwidth(q->pll, 1e-4f); // very low bandwidth

    q->header_decoder  = qpacketmodem_create();
    q->header_user_len = DSSSFRAME_H_USER_DEFAULT;
    dsssframesync_set_header_props(q, NULL);

    q->payload_decoder    = qpacketmodem_create();
    q->payload_spread_len = 64;
    q->payload_spread
        = (float complex *)malloc(q->payload_spread_len * sizeof(float complex));

    dsssframesync_reset_framedatastats(q);
    dsssframesync_reset(q);

    return q;
}

int dsssframesync_destroy(dsssframesync _q)
{
    if (_q == NULL)
        return liquid_error(LIQUID_EIOBJ,"dsssframesync_destroy(), NULL pointer passed");

    free(_q->preamble_pn);
    free(_q->preamble_rx);
    free(_q->header_spread);
    free(_q->header_dec);
    free(_q->payload_spread);
    free(_q->payload_dec);

    qpacketmodem_destroy(_q->header_decoder);
    qpacketmodem_destroy(_q->payload_decoder);
    qdetector_cccf_destroy(_q->detector);
    firpfb_crcf_destroy(_q->mf);
    nco_crcf_destroy(_q->mixer);
    nco_crcf_destroy(_q->pll);
    synth_crcf_destroy(_q->header_synth);
    synth_crcf_destroy(_q->payload_synth);

    free(_q);
    return LIQUID_OK;
}

int dsssframesync_print(dsssframesync _q)
{
    printf("<liquid.dsssframesync>\n");
    return LIQUID_OK;
}

int dsssframesync_reset(dsssframesync _q)
{
    qdetector_cccf_reset(_q->detector);

    nco_crcf_reset(_q->mixer);
    nco_crcf_reset(_q->pll);

    firpfb_crcf_reset(_q->mf);

    _q->state            = DSSSFRAMESYNC_STATE_DETECTFRAME;
    _q->preamble_counter = 0;
    _q->symbol_counter   = 0;

    _q->framesyncstats.evm = 0.f;
    return LIQUID_OK;
}

int dsssframesync_is_frame_open(dsssframesync _q)
{
    return (_q->state == DSSSFRAMESYNC_STATE_DETECTFRAME) ? 0 : 1;
}

int dsssframesync_set_header_len(dsssframesync _q, unsigned int _len)
{
    _q->header_user_len = _len;
    _q->header_dec_len  = DSSSFRAME_H_DEC + _q->header_user_len;
    _q->header_dec
        = (unsigned char *)realloc(_q->header_dec, _q->header_dec_len * sizeof(unsigned char));
    qpacketmodem_configure(_q->header_decoder,
                           _q->header_dec_len,
                           _q->header_props.check,
                           _q->header_props.fec0,
                           _q->header_props.fec1,
                           LIQUID_MODEM_BPSK);

    _q->header_spread_len = synth_crcf_get_length(_q->header_synth);
    _q->header_spread     = (float complex *)realloc(
        _q->header_spread, _q->header_spread_len * sizeof(float complex));
    return LIQUID_OK;
}

int dsssframesync_decode_header_soft(dsssframesync _q, int _soft)
{
    _q->header_soft = _soft;
    return LIQUID_OK;
}

int dsssframesync_decode_payload_soft(dsssframesync _q, int _soft)
{
    _q->payload_soft = _soft;
    return LIQUID_OK;
}

int dsssframesync_set_header_props(dsssframesync _q, dsssframegenprops_s * _props)
{
    if (_props == NULL)
        _props = &dsssframesyncprops_header_default;

    if (_props->check == LIQUID_CRC_UNKNOWN || _props->check >= LIQUID_CRC_NUM_SCHEMES)
        return liquid_error(LIQUID_EIMODE,"dsssframesync_set_header_props(), invalid/unsupported CRC scheme");
    if (_props->fec0 == LIQUID_FEC_UNKNOWN || _props->fec1 == LIQUID_FEC_UNKNOWN)
        return liquid_error(LIQUID_EIMODE,"dsssframesync_set_header_props(), invalid/unsupported FEC scheme");

    // copy properties to internal structure
    memmove(&_q->header_props, _props, sizeof(dsssframegenprops_s));

    // reconfigure payload buffers (reallocate as necessary)
    return dsssframesync_set_header_len(_q, _q->header_user_len);
}

int dsssframesync_execute(dsssframesync _q, float complex * _x, unsigned int _n)
{
    unsigned int i;
    for (i = 0; i < _n; i++) {
        switch (_q->state) {
        case DSSSFRAMESYNC_STATE_DETECTFRAME:
            // detect frame (look for p/n sequence)
            dsssframesync_execute_seekpn(_q, _x[i]);
            break;
        case DSSSFRAMESYNC_STATE_RXPREAMBLE:
            // receive p/n sequence symbols
            dsssframesync_execute_rxpreamble(_q, _x[i]);
            break;
        case DSSSFRAMESYNC_STATE_RXHEADER:
            // receive header symbols
            dsssframesync_execute_rxheader(_q, _x[i]);
            break;
        case DSSSFRAMESYNC_STATE_RXPAYLOAD:
            // receive payload symbols
            dsssframesync_execute_rxpayload(_q, _x[i]);
            break;
        default:
            return liquid_error(LIQUID_EINT,"dsssframesync_execute(), invalid internal state");
        }
    }
    return LIQUID_OK;
}

// execute synchronizer, seeking p/n sequence
//  _q      :   frame synchronizer object
//  _x      :   input sample
//  _sym    :   demodulated symbol
int dsssframesync_execute_seekpn(dsssframesync _q, float complex _x)
{
    // push through pre-demod synchronizer
    float complex * v = qdetector_cccf_execute(_q->detector, _x);

    // check if frame has been detected
    if (v == NULL)
        return LIQUID_OK;

    // get estimates
    _q->tau_hat   = qdetector_cccf_get_tau(_q->detector);
    _q->gamma_hat = qdetector_cccf_get_gamma(_q->detector);
    _q->dphi_hat  = qdetector_cccf_get_dphi(_q->detector);
    _q->phi_hat   = qdetector_cccf_get_phi(_q->detector);

    // set appropriate filterbank index
    if (_q->tau_hat > 0) {
        _q->pfb_index  = (unsigned int)(_q->tau_hat * _q->npfb) % _q->npfb;
        _q->mf_counter = 0;
    } else {
        _q->pfb_index  = (unsigned int)((1.0f + _q->tau_hat) * _q->npfb) % _q->npfb;
        _q->mf_counter = 1;
    }

    // output filter scale (gain estimate, scaled by 1/2 for k=2 samples/symbol)
    firpfb_crcf_set_scale(_q->mf, 0.5f / _q->gamma_hat);

    // set frequency/phase of mixer
    nco_crcf_set_frequency(_q->mixer, _q->dphi_hat);
    nco_crcf_set_phase(_q->mixer, _q->phi_hat);

    // update state
    _q->state = DSSSFRAMESYNC_STATE_RXPREAMBLE;

    // run buffered samples through synchronizer
    unsigned int buf_len = qdetector_cccf_get_buf_len(_q->detector);
    return dsssframesync_execute(_q, v, buf_len);
}

int dsssframesync_step(dsssframesync _q, float complex _x, float complex * _y)
{
    // mix sample down
    float complex v;
    nco_crcf_mix_down(_q->mixer, _x, &v);
    nco_crcf_step(_q->mixer);

    // push sample into filterbank
    firpfb_crcf_push(_q->mf, v);
    firpfb_crcf_execute(_q->mf, _q->pfb_index, &v);

    // increment counter to determine if sample is available
    _q->mf_counter++;
    int sample_available = (_q->mf_counter >= 1) ? 1 : 0;

    // set output sample if available
    if (sample_available) {
        // set output
        *_y = v;

        // decrement counter by k=2 samples/symbol
        _q->mf_counter -= _q->k;
    }

    // return flag
    return sample_available;
}

int dsssframesync_execute_rxpreamble(dsssframesync _q, float complex _x)
{
    // step synchronizer
    float complex mf_out           = 0.0f;
    int           sample_available = dsssframesync_step(_q, _x, &mf_out);

    // compute output if timeout
    if (!sample_available)
        return LIQUID_OK;

    // save output in p/n symbols buffer
    unsigned int delay = _q->k * _q->m; // delay from matched filter
    if (_q->preamble_counter >= delay) {
        unsigned int index     = _q->preamble_counter - delay;
        _q->preamble_rx[index] = mf_out;
    }

    // update p/n counter
    _q->preamble_counter++;

    // update state
    if (_q->preamble_counter == 64 + delay)
        _q->state = DSSSFRAMESYNC_STATE_RXHEADER;

    return LIQUID_OK;
}

int dsssframesync_execute_rxheader(dsssframesync _q, float complex _x)
{
    float complex mf_out           = 0.f;
    int           sample_available = dsssframesync_step(_q, _x, &mf_out);

    if (!sample_available)
        return LIQUID_OK;

    _q->header_spread[_q->symbol_counter % synth_crcf_get_length(_q->header_synth)] = mf_out;
    ++_q->symbol_counter;

    if (_q->symbol_counter % synth_crcf_get_length(_q->header_synth))
        return LIQUID_OK;

    int header_complete = dsssframesync_decode_header(_q);

    if (!header_complete)
        return LIQUID_OK;

    if (_q->header_valid) {
        _q->symbol_counter = 0;
        _q->state = DSSSFRAMESYNC_STATE_RXPAYLOAD;
        return LIQUID_OK;
    }

    // if not taken, header is NOT valid

    ++_q->framedatastats.num_frames_detected;

    if (_q->callback != NULL) {
        _q->framesyncstats.evm           = 0.f;
        _q->framesyncstats.rssi          = 20 * log10f(_q->gamma_hat);
        _q->framesyncstats.cfo           = nco_crcf_get_frequency(_q->mixer);
        _q->framesyncstats.framesyms     = NULL;
        _q->framesyncstats.num_framesyms = 0;
        _q->framesyncstats.check         = LIQUID_CRC_UNKNOWN;
        _q->framesyncstats.fec0          = LIQUID_FEC_UNKNOWN;
        _q->framesyncstats.fec1          = LIQUID_FEC_UNKNOWN;

        _q->callback(
            _q->header_dec, _q->header_valid, NULL, 0, 0, _q->framesyncstats, _q->userdata);
    }

    return dsssframesync_reset(_q);
}

int dsssframesync_decode_header(dsssframesync _q)
{
    float complex prev_corr, corr, next_corr;
    nco_crcf_mix_block_down(
        _q->pll, _q->header_spread, _q->header_spread, synth_crcf_get_length(_q->header_synth));
    synth_crcf_despread_triple(_q->header_synth, _q->header_spread, &prev_corr, &corr, &next_corr);

    int   complete    = qpacketmodem_decode_soft_sym(_q->header_decoder, corr);
    float phase_error = qpacketmodem_get_demodulator_phase_error(_q->header_decoder);

    nco_crcf_pll_step(_q->pll, synth_crcf_get_length(_q->header_synth) * phase_error);

    if (!complete) {
        return 0;
    }

    dsssframesync_configure_payload(_q);
    return 1;
}

int dsssframesync_configure_payload(dsssframesync _q)
{
    _q->header_valid = qpacketmodem_decode_soft_payload(_q->header_decoder, _q->header_dec);

    if (!_q->header_valid)
        return LIQUID_OK;

    unsigned int n = _q->header_user_len;

    unsigned int protocol = _q->header_dec[n + 0];
    if (protocol != DSSSFRAME_PROTOCOL) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EIMODE,"dsssframesync_decode_header(), invalid framing protocol %u (expected %u)\n",protocol,DSSSFRAME_PROTOCOL);
    }

    unsigned int payload_dec_len = (_q->header_dec[n + 1] << 8) | (_q->header_dec[n + 2]);
    _q->payload_dec_len          = payload_dec_len;

    unsigned int check = (_q->header_dec[n + 3] >> 5) & 0x07;
    unsigned int fec0  = (_q->header_dec[n + 3]) & 0x1f;
    unsigned int fec1  = (_q->header_dec[n + 4]) & 0x1f;

    if (check == LIQUID_CRC_UNKNOWN || check >= LIQUID_CRC_NUM_SCHEMES) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EIMODE,"dsssframesync_decode_header(), decoded CRC exceeds available\n");
    } else if (fec0 == LIQUID_FEC_UNKNOWN || fec0 >= LIQUID_FEC_NUM_SCHEMES) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EIMODE,"dsssframesync_decode_header(), decoded FEC (inner) exceeds available\n");
        return LIQUID_OK;
    } else if (fec1 == LIQUID_FEC_UNKNOWN || fec1 >= LIQUID_FEC_NUM_SCHEMES) {
        _q->header_valid = 0;
        return liquid_error(LIQUID_EIMODE,"dsssframesync_decode_header(), decoded FEC (outer) exceeds available\n");
    }

    _q->payload_dec
        = (unsigned char *)realloc(_q->payload_dec, (_q->payload_dec_len) * sizeof(unsigned char));
    qpacketmodem_configure(
        _q->payload_decoder, _q->payload_dec_len, check, fec0, fec1, LIQUID_MODEM_BPSK);

    synth_crcf_set_frequency(_q->payload_synth, synth_crcf_get_frequency(_q->header_synth));
    return LIQUID_OK;
}

int dsssframesync_execute_rxpayload(dsssframesync _q, float complex _x)
{
    float complex mf_out           = 0.f;
    int                  sample_available = dsssframesync_step(_q, _x, &mf_out);

    if (!sample_available)
        return LIQUID_OK;

    _q->payload_spread[_q->symbol_counter % synth_crcf_get_length(_q->payload_synth)] = mf_out;
    ++_q->symbol_counter;

    if (_q->symbol_counter % synth_crcf_get_length(_q->payload_synth) != 0)
        return LIQUID_OK;

    int payload_complete = dsssframesync_decode_payload(_q);

    if (!payload_complete)
        return LIQUID_OK;

    _q->framesyncstats.check = qpacketmodem_get_crc(_q->payload_decoder);
    _q->framesyncstats.fec0  = qpacketmodem_get_fec0(_q->payload_decoder);
    _q->framesyncstats.fec1  = qpacketmodem_get_fec1(_q->payload_decoder);

    if (_q->callback != NULL) {
        _q->callback(_q->header_dec,
                     _q->header_valid,
                     _q->payload_dec,
                     _q->payload_dec_len,
                     _q->payload_valid,
                     _q->framesyncstats,
                     _q->userdata);
    }

    return dsssframesync_reset(_q);
}

int dsssframesync_decode_payload(dsssframesync _q)
{
    float complex prev_corr, corr, next_corr;
    nco_crcf_mix_block_down(
        _q->pll, _q->payload_spread, _q->payload_spread, synth_crcf_get_length(_q->payload_synth));
    synth_crcf_despread_triple(
        _q->payload_synth, _q->payload_spread, &prev_corr, &corr, &next_corr);

    int   complete    = qpacketmodem_decode_soft_sym(_q->payload_decoder, corr);
    float phase_error = qpacketmodem_get_demodulator_phase_error(_q->payload_decoder);

    nco_crcf_pll_step(_q->pll, synth_crcf_get_length(_q->payload_synth) * phase_error);

    if (!complete) {
        return 0;
    }

    _q->payload_valid = qpacketmodem_decode_soft_payload(_q->payload_decoder, _q->payload_dec);

    return 1;
}

int dsssframesync_reset_framedatastats(dsssframesync _q)
{
    return framedatastats_reset(&_q->framedatastats);
}

framedatastats_s dsssframesync_get_framedatastats(dsssframesync _q)
{
    return _q->framedatastats;
}

int dsssframesync_debug_enable(dsssframesync _q)
{
    return liquid_error(LIQUID_EIMODE,"dsssframesync_debug_enable(), mode not supported");
}

int dsssframesync_debug_disable(dsssframesync _q)
{
    return liquid_error(LIQUID_EIMODE,"dsssframesync_debug_disable(), mode not supported");
}

int dsssframesync_debug_print(dsssframesync _q, const char * _filename)
{
    return liquid_error(LIQUID_EIMODE,"dsssframesync_debug_print(), mode not supported");
}
