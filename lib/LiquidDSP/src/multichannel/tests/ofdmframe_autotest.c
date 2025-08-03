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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <assert.h>

#include "autotest/autotest.h"
#include "liquid.h"


// internal callback
//  _X          :   subcarrier symbols
//  _p          :   subcarrier allocation
//  _M          :   number of subcarriers
//  _userdata   :   user-defined data structure
int ofdmframesync_autotest_callback(float complex * _X,
                                    unsigned char * _p,
                                    unsigned int    _M,
                                    void * _userdata)
{
    if (liquid_autotest_verbose)
        printf("******** callback invoked!\n");

    // type cast _userdata as complex float array
    float complex * X = (float complex *)_userdata;

    // copy values and return
    memmove(X, _X, _M*sizeof(float complex));

    // return
    return 0;
}


// Helper function to keep code base small
//  _num_subcarriers    :   number of subcarriers
//  _cp_len             :   cyclic prefix length
//  _taper_len          :   taper length
void ofdmframesync_acquire_test(unsigned int _num_subcarriers,
                                unsigned int _cp_len,
                                unsigned int _taper_len)
{
    // options
    unsigned int M         = _num_subcarriers;  // number of subcarriers
    unsigned int cp_len    = _cp_len;           // cyclic prefix length
    unsigned int taper_len = _taper_len;        // taper length
    float tol              = 1e-2f;             // error tolerance

    //
    float dphi = 1.0f / (float)M;       // carrier frequency offset

    // subcarrier allocation (initialize to default)
    unsigned char p[M];
    ofdmframe_init_default_sctype(M, p);

    // derived values
    unsigned int num_samples = (3 + 1)*(M + cp_len);

    // create synthesizer/analyzer objects
    ofdmframegen fg = ofdmframegen_create(M, cp_len, taper_len, p);
    //ofdmframegen_print(fg);

    float complex X[M];         // original data sequence
    float complex X_test[M];    // recovered data sequence
    ofdmframesync fs = ofdmframesync_create(M,cp_len,taper_len,p,ofdmframesync_autotest_callback,(void*)X_test);

    unsigned int i;
    float complex y[num_samples];   // frame samples

    // assemble full frame
    unsigned int n=0;

    // write first S0 symbol
    ofdmframegen_write_S0a(fg, &y[n]);
    n += M + cp_len;

    // write second S0 symbol
    ofdmframegen_write_S0b(fg, &y[n]);
    n += M + cp_len;

    // write S1 symbol
    ofdmframegen_write_S1( fg, &y[n]);
    n += M + cp_len;

    // generate data symbol (random)
    for (i=0; i<M; i++) {
        X[i]      = cexpf(_Complex_I*2*M_PI*randf());
        X_test[i] = 0.0f;
    }

    // write data symbol
    ofdmframegen_writesymbol(fg, X, &y[n]);
    n += M + cp_len;

    // validate frame length
    assert(n == num_samples);

    // add carrier offset
    for (i=0; i<num_samples; i++)
        y[i] *= cexpf(_Complex_I*dphi*i);

    // run receiver
    ofdmframesync_execute(fs,y,num_samples);

    // check output
    for (i=0; i<M; i++) {
        if (p[i] == OFDMFRAME_SCTYPE_DATA) {
            float e = crealf( (X[i] - X_test[i])*conjf(X[i] - X_test[i]) );
            CONTEND_DELTA( fabsf(e), 0.0f, tol );
        }
    }

    // destroy objects
    ofdmframegen_destroy(fg);
    ofdmframesync_destroy(fs);
}

//
void autotest_ofdmframesync_acquire_n64()   { ofdmframesync_acquire_test(64,  8,  0); }
void autotest_ofdmframesync_acquire_n128()  { ofdmframesync_acquire_test(128, 16, 0); }
void autotest_ofdmframesync_acquire_n256()  { ofdmframesync_acquire_test(256, 32, 0); }
void autotest_ofdmframesync_acquire_n512()  { ofdmframesync_acquire_test(512, 64, 0); }

void autotest_ofdmframe_common_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping ofdmframe common config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check invalid function calls
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_init_default_sctype(0, NULL)) // too few subcarriers

    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_init_sctype_range( 0, -0.4f, +0.4f, NULL)) // too few subcarriers
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_init_sctype_range(64, -0.7f, +0.4f, NULL)) // frequency out of range
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_init_sctype_range(64, -0.4f, +0.7f, NULL)) // frequency out of range
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_init_sctype_range(64, -0.2f, -0.3f, NULL)) // frequency out of range
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_init_sctype_range(64,  0.3f,  0.2f, NULL)) // frequency out of range
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_init_sctype_range(64, -0.02f,+0.02f,NULL)) // too few effective subcarriers

    // generate valid subcarrier allocation
    unsigned int M = 120;
    unsigned char p[M];

    // default subcarrier allocation
    CONTEND_EQUALITY(LIQUID_OK, ofdmframe_init_default_sctype(M, p))
    CONTEND_EQUALITY(LIQUID_OK, ofdmframe_validate_sctype(p, M, NULL, NULL, NULL))

    // subcarrier allocation within an occupied frequency range
    CONTEND_EQUALITY(LIQUID_OK, ofdmframe_init_sctype_range(M, -0.4f,+0.4f,p))
    CONTEND_EQUALITY(LIQUID_OK, ofdmframe_validate_sctype(p, M, NULL, NULL, NULL))

    // invalid subcarrier allocations
    unsigned int i;
    for (i=0; i<M; i++)
        p[i] = OFDMFRAME_SCTYPE_NULL;
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_validate_sctype(p, M, NULL, NULL, NULL))

    p[0] = OFDMFRAME_SCTYPE_PILOT;
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_validate_sctype(p, M, NULL, NULL, NULL))

    p[1] = OFDMFRAME_SCTYPE_DATA;
    CONTEND_INEQUALITY(LIQUID_OK, ofdmframe_validate_sctype(p, M, NULL, NULL, NULL))
}

void autotest_ofdmframegen_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping ofdmframegen config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check invalid function calls
    //CONTEND_ISNULL(ofdmframegen_copy(NULL));
    CONTEND_ISNULL(ofdmframegen_create( 0, 16, 4, NULL)) // too few subcarriers
    CONTEND_ISNULL(ofdmframegen_create( 7, 16, 4, NULL)) // too few subcarriers
    CONTEND_ISNULL(ofdmframegen_create(65, 16, 4, NULL)) // odd-length subcarriers
    CONTEND_ISNULL(ofdmframegen_create(64, 66, 4, NULL)) // cyclic prefix length too large
    CONTEND_ISNULL(ofdmframegen_create(64, 16,24, NULL)) // taper length greater than cyclic prefix

    // create proper object and test configurations
    ofdmframegen q = ofdmframegen_create(64, 16, 4, NULL);

    CONTEND_EQUALITY(LIQUID_OK, ofdmframegen_print(q))

    ofdmframegen_destroy(q);
}

void autotest_ofdmframesync_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping ofdmframesync config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check invalid function calls
    //CONTEND_ISNULL(ofdmframesync_copy(NULL));
    CONTEND_ISNULL(ofdmframesync_create( 0, 16, 4, NULL, NULL, NULL)) // too few subcarriers
    CONTEND_ISNULL(ofdmframesync_create( 7, 16, 4, NULL, NULL, NULL)) // too few subcarriers
    CONTEND_ISNULL(ofdmframesync_create(65, 16, 4, NULL, NULL, NULL)) // odd-length subcarriers
    CONTEND_ISNULL(ofdmframesync_create(64, 66, 4, NULL, NULL, NULL)) // cyclic prefix length too large
    CONTEND_ISNULL(ofdmframesync_create(64, 16,24, NULL, NULL, NULL)) // taper length greater than cyclic prefix

    // create proper object and test configurations
    ofdmframesync q = ofdmframesync_create(64, 16, 4, NULL, NULL, NULL);

    CONTEND_EQUALITY(LIQUID_OK, ofdmframesync_print(q))
    CONTEND_EQUALITY(        0, ofdmframesync_is_frame_open(q))
    CONTEND_EQUALITY(LIQUID_OK, ofdmframesync_set_cfo(q,0.0f))

    ofdmframesync_destroy(q);
}

