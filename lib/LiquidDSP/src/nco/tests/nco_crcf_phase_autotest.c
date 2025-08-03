/*
 * Copyright (c) 2007 - 2018 Joseph Gaeddert
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

// autotest helper function
//  _theta  :   input phase
//  _cos    :   expected output: cos(_theta)
//  _sin    :   expected output: sin(_theta)
//  _type   :   NCO type (e.g. LIQUID_NCO)
//  _tol    :   error tolerance
void nco_crcf_phase_test(float _theta,
                         float _cos,
                         float _sin,
                         int   _type,
                         float _tol)
{
    // create object
    nco_crcf nco = nco_crcf_create(_type);

    // set phase
    nco_crcf_set_phase(nco, _theta);

    // compute cosine and sine outputs
    float c = nco_crcf_cos(nco);
    float s = nco_crcf_sin(nco);

    if (liquid_autotest_verbose) {
        printf("cos(%8.5f) = %8.5f (%8.5f) e:%8.5f, sin(%8.5f) = %8.5f (%8.5f) e:%8.5f\n",
                _theta, _cos, c, _cos-c, _theta, _sin, s, _sin-s);
    }

    // run tests
    CONTEND_DELTA( c, _cos, _tol );
    CONTEND_DELTA( s, _sin, _tol );

    // destroy object
    nco_crcf_destroy(nco);
}


// test floating point precision nco phase
void autotest_nco_crcf_phase()
{
    // error tolerance (higher for NCO)
    float tol = 0.02f;

    nco_crcf_phase_test(-6.283185307f,  1.000000000f,  0.000000000f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-6.195739393f,  0.996179042f,  0.087334510f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-5.951041106f,  0.945345356f,  0.326070787f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-5.131745978f,  0.407173250f,  0.913350943f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-4.748043551f,  0.035647016f,  0.999364443f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-3.041191113f, -0.994963998f, -0.100232943f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-1.947799864f, -0.368136099f, -0.929771914f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-1.143752030f,  0.414182352f, -0.910193924f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-1.029377689f,  0.515352252f, -0.856978446f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-0.174356887f,  0.984838307f, -0.173474811f, LIQUID_NCO, tol);
    nco_crcf_phase_test(-0.114520496f,  0.993449692f, -0.114270338f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 0.000000000f,  1.000000000f,  0.000000000f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 1.436080000f,  0.134309213f,  0.990939471f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 2.016119855f, -0.430749878f,  0.902471353f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 2.996498473f, -0.989492293f,  0.144585621f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 3.403689755f, -0.965848729f, -0.259106603f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 3.591162483f, -0.900634128f, -0.434578148f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 5.111428476f,  0.388533479f, -0.921434607f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 5.727585681f,  0.849584319f, -0.527452828f, LIQUID_NCO, tol);
    nco_crcf_phase_test( 6.283185307f,  1.000000000f, -0.000000000f, LIQUID_NCO, tol);
}

//
// test floating point precision nco
//
void autotest_nco_basic() {
    nco_crcf p = nco_crcf_create(LIQUID_NCO);

    unsigned int i;     // loop index
    float s, c;         // sine/cosine result
    float tol=1e-4f;    // error tolerance
    float f=0.0f;       // frequency to test

    nco_crcf_set_phase( p, 0.0f);
    CONTEND_DELTA( nco_crcf_cos(p), 1.0f, tol );
    CONTEND_DELTA( nco_crcf_sin(p), 0.0f, tol );
    nco_crcf_sincos(p, &s, &c);
    CONTEND_DELTA( s, 0.0f, tol );
    CONTEND_DELTA( c, 1.0f, tol );

    nco_crcf_set_phase(p, M_PI/2);
    CONTEND_DELTA( nco_crcf_cos(p), 0.0f, tol );
    CONTEND_DELTA( nco_crcf_sin(p), 1.0f, tol );
    nco_crcf_sincos(p, &s, &c);
    CONTEND_DELTA( s, 1.0f, tol );
    CONTEND_DELTA( c, 0.0f, tol );

    // cycle through one full period in 64 steps
    nco_crcf_set_phase(p, 0.0f);
    f = 2.0f * M_PI / 64.0f;
    nco_crcf_set_frequency(p, f);
    for (i=0; i<128; i++) {
        nco_crcf_sincos(p, &s, &c);
        CONTEND_DELTA( s, sinf(i*f), tol );
        CONTEND_DELTA( c, cosf(i*f), tol );
        nco_crcf_step(p);
    }

    // double frequency: cycle through one full period in 32 steps
    nco_crcf_set_phase(p, 0.0f);
    f = 2.0f * M_PI / 32.0f;
    nco_crcf_set_frequency(p, f);
    for (i=0; i<128; i++) {
        nco_crcf_sincos(p, &s, &c);
        CONTEND_DELTA( s, sinf(i*f), tol );
        CONTEND_DELTA( c, cosf(i*f), tol );
        nco_crcf_step(p);
    }

    // destroy NCO object
    nco_crcf_destroy(p);
}

//
// test nco mixing
//
void autotest_nco_mixing() {
    // frequency, phase
    float f = 0.1f;
    float phi = M_PI;

    // error tolerance (high for NCO)
    float tol = 0.05f;

    // initialize nco object
    nco_crcf p = nco_crcf_create(LIQUID_NCO);
    nco_crcf_set_frequency(p, f);
    nco_crcf_set_phase(p, phi);

    unsigned int i;
    float nco_i, nco_q;
    for (i=0; i<64; i++) {
        // generate sin/cos
        nco_crcf_sincos(p, &nco_q, &nco_i);

        // mix back to zero phase
        complex float nco_cplx_in = nco_i + _Complex_I*nco_q;
        complex float nco_cplx_out;
        nco_crcf_mix_down(p, nco_cplx_in, &nco_cplx_out);

        // assert mixer output is correct
        CONTEND_DELTA(crealf(nco_cplx_out), 1.0f, tol);
        CONTEND_DELTA(cimagf(nco_cplx_out), 0.0f, tol);
        //printf("%3u : %12.8f + j*%12.8f\n", i, crealf(nco_cplx_out), cimagf(nco_cplx_out));

        // step nco
        nco_crcf_step(p);
    }

    // destroy NCO object
    nco_crcf_destroy(p);
}

//
// test nco block mixing
//
void autotest_nco_block_mixing()
{
    // frequency, phase
    float f = 0.1f;
    float phi = M_PI;

    // error tolerance (high for NCO)
    float tol = 0.05f;

    unsigned int i;

    // number of samples
    unsigned int num_samples = 1024;

    // store samples
    float complex * x = (float complex*)malloc(num_samples*sizeof(float complex));
    float complex * y = (float complex*)malloc(num_samples*sizeof(float complex));

    // generate complex sin/cos
    for (i=0; i<num_samples; i++)
        x[i] = cexpf(_Complex_I*(f*i + phi));

    // initialize nco object
    nco_crcf p = nco_crcf_create(LIQUID_NCO);
    nco_crcf_set_frequency(p, f);
    nco_crcf_set_phase(p, phi);

    // mix signal back to zero phase (in pieces)
    unsigned int num_remaining = num_samples;
    i = 0;
    while (num_remaining > 0) {
        unsigned int n = 7 < num_remaining ? 7 : num_remaining;
        nco_crcf_mix_block_down(p, &x[i], &y[i], n);

        i += n;
        num_remaining -= n;
    }

    // assert mixer output is correct
    for (i=0; i<num_samples; i++) {
        CONTEND_DELTA( crealf(y[i]), 1.0f, tol );
        CONTEND_DELTA( cimagf(y[i]), 0.0f, tol );
    }

    // free those buffers
    free(x);
    free(y);

    // destroy NCO object
    nco_crcf_destroy(p);
}

