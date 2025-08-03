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
// ofdmframe data, methods common to both generator/synchronizer
// objects
//  - physical layer convergence procedure (PLCP)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

// generate short sequence symbols
//  _p      :   subcarrier allocation array
//  _M      :   total number of subcarriers
//  _S0     :   output symbol (freq)
//  _s0     :   output symbol (time)
//  _M_S0   :   total number of enabled subcarriers in S0
int ofdmframe_init_S0(unsigned char * _p,
                      unsigned int    _M,
                      float complex * _S0,
                      float complex * _s0,
                      unsigned int *  _M_S0)
{
    unsigned int i;

    // compute m-sequence length
    unsigned int m = liquid_nextpow2(_M);
    if (m < 4)      m = 4;
    else if (m > 8) m = 8;

    // generate m-sequence generator object
    msequence ms = msequence_create_default(m);

    unsigned int s;
    unsigned int M_S0 = 0;

    // short sequence
    for (i=0; i<_M; i++) {
        // generate symbol
        //s = msequence_generate_symbol(ms,1);
        s = msequence_generate_symbol(ms,3) & 0x01;

        if (_p[i] == OFDMFRAME_SCTYPE_NULL) {
            // NULL subcarrier
            _S0[i] = 0.0f;
        } else {
            if ( (i%2) == 0 ) {
                // even subcarrer
                _S0[i] = s ? 1.0f : -1.0f;
                M_S0++;
            } else {
                // odd subcarrer (ignore)
                _S0[i] = 0.0f;
            }
        }
    }

    // destroy objects
    msequence_destroy(ms);

    // ensure at least one subcarrier was enabled
    if (M_S0 == 0)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_S0(), no subcarriers enabled; check allocation");

    // set return value(s)
    *_M_S0 = M_S0;

    // run inverse fft to get time-domain sequence
    fft_run(_M, _S0, _s0, LIQUID_FFT_BACKWARD, 0);

    // normalize time-domain sequence level
    float g = 1.0f / sqrtf(M_S0);
    for (i=0; i<_M; i++)
        _s0[i] *= g;
    return LIQUID_OK;
}


// generate long sequence symbols
//  _p      :   subcarrier allocation array
//  _M      :   total number of subcarriers
//  _S1     :   output symbol (freq)
//  _s1     :   output symbol (time)
//  _M_S1   :   total number of enabled subcarriers in S1
int ofdmframe_init_S1(unsigned char * _p,
                      unsigned int    _M,
                      float complex * _S1,
                      float complex * _s1,
                      unsigned int *  _M_S1)
{
    unsigned int i;

    // compute m-sequence length
    unsigned int m = liquid_nextpow2(_M);
    if (m < 4)      m = 4;
    else if (m > 8) m = 8;

    // increase m such that the resulting S1 sequence will
    // differ significantly from S0 with the same subcarrier
    // allocation array
    m++;

    // generate m-sequence generator object
    msequence ms = msequence_create_default(m);

    unsigned int s;
    unsigned int M_S1 = 0;

    // long sequence
    for (i=0; i<_M; i++) {
        // generate symbol
        //s = msequence_generate_symbol(ms,1);
        s = msequence_generate_symbol(ms,3) & 0x01;

        if (_p[i] == OFDMFRAME_SCTYPE_NULL) {
            // NULL subcarrier
            _S1[i] = 0.0f;
        } else {
            _S1[i] = s ? 1.0f : -1.0f;
            M_S1++;
        }
    }

    // destroy objects
    msequence_destroy(ms);

    // ensure at least one subcarrier was enabled
    if (M_S1 == 0)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_S1(), no subcarriers enabled; check allocation");

    // set return value(s)
    *_M_S1 = M_S1;

    // run inverse fft to get time-domain sequence
    fft_run(_M, _S1, _s1, LIQUID_FFT_BACKWARD, 0);

    // normalize time-domain sequence level
    float g = 1.0f / sqrtf(M_S1);
    for (i=0; i<_M; i++)
        _s1[i] *= g;
    return LIQUID_OK;
}

// initialize default subcarrier allocation
//  _M      :   number of subcarriers
//  _p      :   output subcarrier allocation array, [size: _M x 1]
//
// key: '.' (null), 'P' (pilot), '+' (data)
// .+++P+++++++P.........P+++++++P+++
//
int ofdmframe_init_default_sctype(unsigned int    _M,
                                  unsigned char * _p)
{
    // validate input
    if (_M < 6)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_default_sctype(), less than 6 subcarriers");

    unsigned int i;
    unsigned int M2 = _M/2;

    // compute guard band
    unsigned int G = _M / 10;
    if (G < 2) G = 2;

    // designate pilot spacing
    unsigned int P = (_M > 34) ? 8 : 4;
    unsigned int P2 = P/2;

    // initialize as NULL
    for (i=0; i<_M; i++)
        _p[i] = OFDMFRAME_SCTYPE_NULL;

    // upper band
    for (i=1; i<M2-G; i++) {
        if ( ((i+P2)%P) == 0 )
            _p[i] = OFDMFRAME_SCTYPE_PILOT;
        else
            _p[i] = OFDMFRAME_SCTYPE_DATA;
    }

    // lower band
    for (i=1; i<M2-G; i++) {
        unsigned int k = _M - i;
        if ( ((i+P2)%P) == 0 )
            _p[k] = OFDMFRAME_SCTYPE_PILOT;
        else
            _p[k] = OFDMFRAME_SCTYPE_DATA;
    }
    return LIQUID_OK;
}

// initialize default subcarrier allocation
//  _M      :   number of subcarriers
//  _f0     :   lower frequency band, _f0 in [-0.5,0.5]
//  _f1     :   upper frequency band, _f1 in [-0.5,0.5]
//  _p      :   output subcarrier allocation array, [size: _M x 1]
int ofdmframe_init_sctype_range(unsigned int    _M,
                                float           _f0,
                                float           _f1,
                                unsigned char * _p)
{
    // validate input
    if (_M < 6)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_sctype_range(), less than 6 subcarriers");
    if (_f0 < -0.5f || _f0 > 0.5f)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_sctype_range(), lower frequency edge must be in [-0.5,0.5]");
    if (_f1 < -0.5f || _f1 > 0.5f)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_sctype_range(), upper frequency edge must be in [-0.5,0.5]");
    if (_f0 >= _f1)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_sctype_range(), lower frequency edge must be below upper edge");

    // get relative edges
    int M0 = (int)((_f0 + 0.5f) * _M); // lower subcarrier index
    int M1 = (int)((_f1 + 0.5f) * _M); // upper subcarrier index
    int Mp = M1 - M0;
    if (Mp > (int)_M) {
        Mp = (int)_M;
    } else if (Mp < 6) {
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_init_sctype_range(), less than 6 subcarriers (effectively)");
    }

    // designate pilot spacing
    unsigned int P = (Mp > 34) ? 8 : 4;

    // upper band
    int i;
    for (i=0; i<(int)_M; i++) {
        // shift
        unsigned int k = ((unsigned int)i + _M/2) % _M;
        if (i < M0 || i > M1) {
            // guard band
            _p[k] = OFDMFRAME_SCTYPE_NULL;
        } else if ( (k%P)==0 ) {
            _p[k] = OFDMFRAME_SCTYPE_PILOT;
        } else {
            _p[k] = OFDMFRAME_SCTYPE_DATA;
        }
    }
    return LIQUID_OK;
}

// validate subcarrier type (count number of null, pilot, and data
// subcarriers in the allocation)
//  _p          :   subcarrier allocation array, [size: _M x 1]
//  _M          :   number of subcarriers
//  _M_null     :   output number of null subcarriers
//  _M_pilot    :   output number of pilot subcarriers
//  _M_data     :   output number of data subcarriers
int ofdmframe_validate_sctype(unsigned char * _p,
                              unsigned int    _M,
                              unsigned int *  _M_null,
                              unsigned int *  _M_pilot,
                              unsigned int *  _M_data)
{
    // clear counters
    unsigned int M_null  = 0;
    unsigned int M_pilot = 0;
    unsigned int M_data  = 0;

    unsigned int i;
    for (i=0; i<_M; i++) {
        // update appropriate counters
        if (_p[i] == OFDMFRAME_SCTYPE_NULL)
            M_null++;
        else if (_p[i] == OFDMFRAME_SCTYPE_PILOT)
            M_pilot++;
        else if (_p[i] == OFDMFRAME_SCTYPE_DATA)
            M_data++;
        else {
            return liquid_error(LIQUID_EICONFIG,"ofdmframe_validate_sctype(), invalid subcarrier type (%u)", _p[i]);
        }
    }

    // validate subcarrier allocation
    if ( (M_pilot + M_data) == 0)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_validate_sctype(), must have at least one enabled subcarrier");
    if (M_data == 0)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_validate_sctype(), must have at least one data subcarrier");
    if (M_pilot < 2)
        return liquid_error(LIQUID_EICONFIG,"ofdmframe_validate_sctype(), must have at least two pilot subcarriers");

    // set outputs if requested
    if (_M_null  != NULL) *_M_null  = M_null;
    if (_M_pilot != NULL) *_M_pilot = M_pilot;
    if (_M_data  != NULL) *_M_data  = M_data;
    return LIQUID_OK;
}

// print subcarrier allocation to screen
//
// key: '.' (null), 'P' (pilot), '+' (data)
// .+++P+++++++P.........P+++++++P+++
//
int ofdmframe_print_sctype(unsigned char * _p,
                           unsigned int    _M)
{
    unsigned int i;

    printf("[");
    for (i=0; i<_M; i++) {
        unsigned int k = (i + _M/2) % _M;

        switch (_p[k]) {
        case OFDMFRAME_SCTYPE_NULL:  printf("."); break;
        case OFDMFRAME_SCTYPE_PILOT: printf("|"); break;
        case OFDMFRAME_SCTYPE_DATA:  printf("+"); break;
        default:
            return liquid_error(LIQUID_EICONFIG,"ofdmframe_print_default_sctype(), invalid subcarrier type");
        }
    }
    printf("]\n");
    return LIQUID_OK;
}

