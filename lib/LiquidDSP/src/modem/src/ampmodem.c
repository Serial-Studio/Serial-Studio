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
// Amplitude modulator/demodulator
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "liquid.internal.h"

// internal methods for specific demodulation methods
int ampmodem_demod_dsb_peak_detect(ampmodem _q, float complex _x, float * _m);
int ampmodem_demod_dsb_pll_carrier(ampmodem _q, float complex _x, float * _m);
int ampmodem_demod_dsb_pll_costas (ampmodem _q, float complex _x, float * _m);
int ampmodem_demod_ssb_pll_carrier(ampmodem _q, float complex _x, float * _m);
int ampmodem_demod_ssb            (ampmodem _q, float complex _x, float * _m);

struct ampmodem_s {
    // modulation index
    float mod_index;

    // modulation type (e.g. LIQUID_AMPMODEM_DSB)
    liquid_ampmodem_type type;

    // suppressed carrier flag
    int suppressed_carrier;

    // demod and helper objects
    unsigned int    m;          // filter semi-length for all objects
    nco_crcf        mixer;      // mixer and phase-locked loop
    firfilt_rrrf    dcblock;    // carrier suppression filter
    firhilbf        hilbert;    // hilbert transform (single side-band)
    firfilt_crcf    lowpass;    // low-pass filter for SSB PLL
    wdelaycf        delay;      // delay buffer to align to low-pass filter delay

    // demodulation function pointer
    int (*demod)(ampmodem _q, float complex _x, float * _m);
};

// create ampmodem object
//  _mod_index          :   modulation index
//  _type               :   AM type (e.g. LIQUID_AMPMODEM_DSB)
//  _suppressed_carrier :   carrier suppression flag
ampmodem ampmodem_create(float                _mod_index,
                         liquid_ampmodem_type _type,
                         int                  _suppressed_carrier)
{
    // validate input
    switch (_type) {
    case LIQUID_AMPMODEM_DSB:
    case LIQUID_AMPMODEM_USB:
    case LIQUID_AMPMODEM_LSB:
        break;
    default:
        return liquid_error_config("ampmodem_create(), invalid modem type: %d", (int)_type);
    }

    // create main object
    ampmodem q = (ampmodem) malloc(sizeof(struct ampmodem_s));

    // set internal properties
    q->type      = _type;
    q->mod_index = _mod_index;
    q->suppressed_carrier = (_suppressed_carrier != 0);
    q->m         = 25;

    // create nco, pll objects
    q->mixer = nco_crcf_create(LIQUID_NCO);
    nco_crcf_pll_set_bandwidth(q->mixer,0.001f);

    // carrier suppression filter
    q->dcblock = firfilt_rrrf_create_dc_blocker(q->m, 20.0f);

    // Hilbert transform for single side-band recovery
    q->hilbert = firhilbf_create(q->m, 60.0f);

    // carrier admittance filter for phase-locked loop
    q->lowpass = firfilt_crcf_create_kaiser(2*q->m+1, 0.01f, 40.0f, 0.0f);

    // delay buffer
    q->delay = wdelaycf_create(q->m);

    // set appropriate demod function pointer
    q->demod = NULL;
    if (q->type == LIQUID_AMPMODEM_DSB) {
        // double side-band
        q->demod = q->suppressed_carrier ?
            ampmodem_demod_dsb_pll_costas :
            //ampmodem_demod_dsb_peak_detect;
            ampmodem_demod_dsb_pll_carrier;
    } else {
        // single side-band
        q->demod = q->suppressed_carrier ? ampmodem_demod_ssb : ampmodem_demod_ssb_pll_carrier;
    }

    // reset object and return
    ampmodem_reset(q);
    return q;
}

int ampmodem_destroy(ampmodem _q)
{
    // destroy objects
    nco_crcf_destroy    (_q->mixer);
    firfilt_rrrf_destroy(_q->dcblock);
    firhilbf_destroy    (_q->hilbert);
    firfilt_crcf_destroy(_q->lowpass);
    wdelaycf_destroy    (_q->delay);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

int ampmodem_print(ampmodem _q)
{
    printf("<liquid.ampmodem");
    switch (_q->type) {
    case LIQUID_AMPMODEM_DSB: printf(", type=\"DSB\""); break;
    case LIQUID_AMPMODEM_USB: printf(", type=\"USB\""); break;
    case LIQUID_AMPMODEM_LSB: printf(", type=\"LSB\""); break;
    default:                  printf(", type=\"?\"");
    }
    printf(", carrier_suppressed=%s", _q->suppressed_carrier ? "true" : "false");
    printf(", mod_index=%g", _q->mod_index);
    printf(">\n");
    return LIQUID_OK;
}

// reset all internal objects
int ampmodem_reset(ampmodem _q)
{
    nco_crcf_reset    (_q->mixer);
    firfilt_rrrf_reset(_q->dcblock);
    firhilbf_reset    (_q->hilbert);
    firfilt_crcf_reset(_q->lowpass);
    wdelaycf_reset    (_q->delay);
    return LIQUID_OK;
}

// accessor methods
unsigned int ampmodem_get_delay_mod(ampmodem _q)
{
    switch (_q->type) {
    case LIQUID_AMPMODEM_DSB:
        return 0;
    case LIQUID_AMPMODEM_USB:
    case LIQUID_AMPMODEM_LSB:
        return 2*_q->m;
    default:
        liquid_error(LIQUID_EINT,"ampmodem_get_delay_mod(), internal error, invalid mod type");
    }
    return 0;
}

unsigned int ampmodem_get_delay_demod(ampmodem _q)
{
    switch (_q->type) {
    case LIQUID_AMPMODEM_DSB:
        return _q->suppressed_carrier ? 0 : 2*_q->m;
    case LIQUID_AMPMODEM_USB:
    case LIQUID_AMPMODEM_LSB:
        return _q->suppressed_carrier ? 2*_q->m : 4*_q->m;
    default:
        liquid_error(LIQUID_EINT,"ampmodem_get_delay_demod(), internal error, invalid mod type");
    }
    return 0;
}

int ampmodem_modulate(ampmodem        _q,
                      float           _x,
                      float complex * _y)
{
    float complex x_hat = 0.0f;

    if (_q->type == LIQUID_AMPMODEM_DSB) {
        x_hat = _x;
    } else {
        // push through Hilbert transform
        // LIQUID_AMPMODEM_USB:
        // LIQUID_AMPMODEM_LSB: conjugate Hilbert transform output
        firhilbf_r2c_execute(_q->hilbert, _x, &x_hat);

        if (_q->type == LIQUID_AMPMODEM_LSB)
            x_hat = conjf(x_hat);
    }

    // save result
    *_y = x_hat * _q->mod_index + (_q->suppressed_carrier ? 0.0f : 1.0f);
    return LIQUID_OK;
}

// modulate block of samples
//  _q      :   ampmodem object
//  _m      :   message signal m(t), [size: _n x 1]
//  _n      :   number of input, output samples
//  _s      :   complex baseband signal s(t) [size: _n x 1]
int ampmodem_modulate_block(ampmodem        _q,
                            float *         _m,
                            unsigned int    _n,
                            float complex * _s)
{
    // TODO: implement more efficient method
    unsigned int i;
    for (i=0; i<_n; i++)
        ampmodem_modulate(_q, _m[i], &_s[i]);
    return LIQUID_OK;
}

// demodulate
int ampmodem_demodulate(ampmodem      _q,
                        float complex _y,
                        float *       _x)
{
    // invoke internal type-specific method
    return _q->demod(_q, _y, _x);
}

// demodulate block of samples
//  _q      :   frequency demodulator object
//  _y      :   received signal r(t) [size: _n x 1]
//  _n      :   number of input, output samples
//  _x      :   message signal m(t), [size: _n x 1]
int ampmodem_demodulate_block(ampmodem        _q,
                              float complex * _y,
                              unsigned int    _n,
                              float *         _x)
{
    unsigned int i;
    int rc;
    for (i=0; i<_n; i++) {
        // invoke internal type-specific method
        //ampmodem_demodulate(_q, _y[i], &_x[i]);
        if ( (rc = _q->demod(_q, _y[i], &_x[i])) )
            return rc;
    }
    return LIQUID_OK;
}

//
// internal methods
//

int ampmodem_demod_dsb_peak_detect(ampmodem      _q,
                                   float complex _x,
                                   float *       _y)
{
    // compute signal magnitude
    float v = cabsf(_x);

    // apply DC blocking filter
    firfilt_rrrf_push   (_q->dcblock, v);
    firfilt_rrrf_execute(_q->dcblock, &v);

    // set output
    *_y = v / _q->mod_index;
    return LIQUID_OK;
}

int ampmodem_demod_dsb_pll_carrier(ampmodem      _q,
                                   float complex _x,
                                   float *       _y)
{
    // split signal into two branches:
    //   0. low-pass filter for carrier recovery and
    //   1. delay to align signal output
    float complex x0, x1;
    firfilt_crcf_push   (_q->lowpass, _x);
    firfilt_crcf_execute(_q->lowpass, &x0);
    wdelaycf_push       (_q->delay,   _x);
    wdelaycf_read       (_q->delay,   &x1);

    // mix each signal down
    float complex v0, v1;
    nco_crcf_mix_down(_q->mixer, x0, &v0);
    nco_crcf_mix_down(_q->mixer, x1, &v1);

    // compute phase error
    float phase_error = cargf(v0);

    // adjust nco, pll objects
    nco_crcf_pll_step(_q->mixer, phase_error);

    // step nco
    nco_crcf_step(_q->mixer);

    // keep in-phase component
    float m = crealf(v1) / _q->mod_index;

    // apply DC block, writing directly to output
    firfilt_rrrf_push   (_q->dcblock, m);
    firfilt_rrrf_execute(_q->dcblock, _y);
    return LIQUID_OK;
}

int ampmodem_demod_dsb_pll_costas(ampmodem      _q,
                                  float complex _x,
                                  float *       _y)
{
    // mix signal down
    float complex v;
    nco_crcf_mix_down(_q->mixer, _x, &v);

    // compute phase error
    //float phase_error = crealf(v)*cimagf(v);
    float phase_error = cimagf(v) * (crealf(v) > 0 ? 1 : -1);

    // adjust nco, pll objects
    nco_crcf_pll_step(_q->mixer, phase_error);

    // step nco
    nco_crcf_step(_q->mixer);

    // keep in-phase component (ignore modulation index)
    *_y = crealf(v) / _q->mod_index;
    return LIQUID_OK;
}

int ampmodem_demod_ssb_pll_carrier(ampmodem      _q,
                                   float complex _x,
                                   float *       _y)
{
    // split signal into two branches:
    //   0. low-pass filter for carrier recovery and
    //   1. delay to align signal output
    float complex x0, x1;
    firfilt_crcf_push   (_q->lowpass, _x);
    firfilt_crcf_execute(_q->lowpass, &x0);
    wdelaycf_push       (_q->delay,   _x);
    wdelaycf_read       (_q->delay,   &x1);

    // mix each signal down
    float complex v0, v1;
    nco_crcf_mix_down(_q->mixer, x0, &v0);
    nco_crcf_mix_down(_q->mixer, x1, &v1);

    // compute phase error
    float phase_error = cimagf(v0);

    // adjust nco, pll objects
    nco_crcf_pll_step(_q->mixer, phase_error);

    // step nco
    nco_crcf_step(_q->mixer);

    // apply hilbert transform and retrieve both upper and lower side-bands
    float m_lsb, m_usb;
    firhilbf_c2r_execute(_q->hilbert, v1, &m_lsb, &m_usb);

    // recover message
    float m = 0.5f * (_q->type == LIQUID_AMPMODEM_USB ? m_usb : m_lsb) / _q->mod_index;

    // apply DC block, writing directly to output
    firfilt_rrrf_push   (_q->dcblock, m);
    firfilt_rrrf_execute(_q->dcblock, _y);
    return LIQUID_OK;
}

int ampmodem_demod_ssb(ampmodem      _q,
                       float complex _x,
                       float *       _y)
{
    // apply hilbert transform and retrieve both upper and lower side-bands
    float m_lsb, m_usb;
    firhilbf_c2r_execute(_q->hilbert, _x, &m_lsb, &m_usb);

    // recover message
    *_y = 0.5f * (_q->type == LIQUID_AMPMODEM_USB ? m_usb : m_lsb) / _q->mod_index;
    return LIQUID_OK;
}

