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
// framesyncstats.c
//
// Default and generic frame statistics
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "liquid.internal.h"

framesyncstats_s framesyncstats_default = {
    // signal quality
    0.0f,                   // error vector magnitude
    0.0f,                   // rssi
    0.0f,                   // carrier frequency offset

    // demodulated frame symbols
    NULL,                   // framesyms
    0,                      // num_framesyms

    // modulation/coding scheme, etc.
    LIQUID_MODEM_UNKNOWN,   // mod_scheme
    0,                      // mod_bps
    LIQUID_CRC_UNKNOWN,     // check
    LIQUID_FEC_UNKNOWN,     // fec0
    LIQUID_FEC_UNKNOWN      // fec1
};

// initialize framesyncstats object on default
int framesyncstats_init_default(framesyncstats_s * _stats)
{
    memmove(_stats, &framesyncstats_default, sizeof(framesyncstats_s));
    return LIQUID_OK;
}
// print framesyncstats object
int framesyncstats_print(framesyncstats_s * _stats)
{
    // validate statistics object
    if (_stats->mod_scheme >= LIQUID_MODEM_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"framesyncstats_print(), invalid modulation scheme");
     if (_stats->check >= LIQUID_CRC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"framesyncstats_print(), invalid CRC scheme");
     if (_stats->fec0 >= LIQUID_FEC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"framesyncstats_print(), invalid FEC scheme (inner)");
     if (_stats->fec1 >= LIQUID_FEC_NUM_SCHEMES)
        return liquid_error(LIQUID_EICONFIG,"framesyncstats_print(), invalid FEC scheme (outer)");

    printf("    EVM                 :   %12.8f dB\n", _stats->evm);
    printf("    rssi                :   %12.8f dB\n", _stats->rssi);
    printf("    carrier offset      :   %12.8f Fs\n", _stats->cfo);
    printf("    num symbols         :   %u\n", _stats->num_framesyms);
    printf("    mod scheme          :   %s (%u bits/symbol)\n",
            modulation_types[_stats->mod_scheme].name,
            _stats->mod_bps);
    printf("    validity check      :   %s\n", crc_scheme_str[_stats->check][0]);
    printf("    fec (inner)         :   %s\n", fec_scheme_str[_stats->fec0][0]);
    printf("    fec (outer)         :   %s\n", fec_scheme_str[_stats->fec1][0]);
    return LIQUID_OK;
}

