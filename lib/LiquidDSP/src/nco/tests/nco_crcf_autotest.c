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

#include <stdlib.h>
#include <complex.h>
#include "autotest/autotest.h"
#include "liquid.h"

// forward declaration of internal method to constrain phase
uint32_t nco_crcf_constrain(float _theta);

// compute phase-constrained error
uint32_t nco_crcf_constrain_error(float _theta, uint32_t _expected)
{
    uint32_t phase = nco_crcf_constrain(_theta);
    uint32_t error = phase > _expected ? phase - _expected : _expected - phase;
    return error < 0x80000000 ? error : 0xffffffff - error;
}

// test phase constraint
void autotest_nco_crcf_constrain()
{
    uint32_t tol = 0x00001fff;

    // phase: 0 mod 2 pi
    CONTEND_LESS_THAN( nco_crcf_constrain_error(    0.0f, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(  2*M_PI, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(  4*M_PI, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(  6*M_PI, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( 20*M_PI, 0), tol );

    // phase: 0 mod 2 pi (negative)
    CONTEND_LESS_THAN( nco_crcf_constrain_error(   -0.0f, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( -2*M_PI, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( -4*M_PI, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( -6*M_PI, 0), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(-20*M_PI, 0), tol );

    // phase: pi mod 2 pi
    CONTEND_LESS_THAN( nco_crcf_constrain_error(    M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(  3*M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(  5*M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(  7*M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( 27*M_PI, 0x80000000), tol );

    // phase: pi mod 2 pi (negative)
    CONTEND_LESS_THAN( nco_crcf_constrain_error(   -M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( -3*M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( -5*M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error( -7*M_PI, 0x80000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(-27*M_PI, 0x80000000), tol );

    // check other values
    CONTEND_LESS_THAN( nco_crcf_constrain_error(+0.500*M_PI, 0x40000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(+0.250*M_PI, 0x20000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(+0.125*M_PI, 0x10000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(+0.750*M_PI, 0x60000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(-0.500*M_PI, 0xc0000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(-0.250*M_PI, 0xe0000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(-0.125*M_PI, 0xf0000000), tol );

    // check phase near boundaries
    CONTEND_LESS_THAN( nco_crcf_constrain_error(+0.000001f, 0x00000000), tol );
    CONTEND_LESS_THAN( nco_crcf_constrain_error(-0.000001f, 0x00000000), tol );
}

// test copying object
void autotest_nco_crcf_copy()
{
    // create and initialize object
    nco_crcf nco_0 = nco_crcf_create(LIQUID_VCO);
    nco_crcf_set_phase        (nco_0, 1.23456f);
    nco_crcf_set_frequency    (nco_0, 5.67890f);
    nco_crcf_pll_set_bandwidth(nco_0, 0.011f);

    // copy object
    nco_crcf nco_1 = nco_crcf_copy(nco_0);

    unsigned int i, n = 240;
    float complex v0, v1;
    for (i=0; i<n; i++) {
        // received complex signal
        nco_crcf_cexpf(nco_0,&v0);
        nco_crcf_cexpf(nco_1,&v1);

        // update pll
        nco_crcf_pll_step(nco_0, cargf(v0));
        nco_crcf_pll_step(nco_1, cargf(v1));

        // update nco objects
        nco_crcf_step(nco_0);
        nco_crcf_step(nco_1);

        // check output
        CONTEND_EQUALITY(v0, v1);
    }

    // clean it up
    nco_crcf_destroy(nco_0);
    nco_crcf_destroy(nco_1);
}

void autotest_nco_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping nco config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    //
    CONTEND_INEQUALITY(LIQUID_OK, nco_crcf_destroy(NULL));
    CONTEND_ISNULL(nco_crcf_copy(NULL));

    // create proper NCO object and test configurations
    nco_crcf q_nco = nco_crcf_create(LIQUID_NCO);
    CONTEND_EQUALITY(LIQUID_OK, nco_crcf_print(q_nco))
    nco_crcf_destroy(q_nco);
}

