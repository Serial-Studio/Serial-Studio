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

// declare external arrays
extern float complex nco_sincos_fsqrt1_2[];
extern float complex nco_sincos_fsqrt1_3[];
extern float complex nco_sincos_fsqrt1_5[];
extern float complex nco_sincos_fsqrt1_7[];

// autotest helper function
//  _type       :   NCO type (e.g. LIQUID_NCO)
//  _phase      :   initial phase
//  _frequency  :   initial frequency
//  _sincos     :   external sin/cosine data
//  _num_samples:   number of samples to test
//  _tol        :   error tolerance
void nco_crcf_frequency_test(int             _type,
                             float           _phase,
                             float           _frequency,
                             float complex * _sincos,
                             unsigned int    _num_samples,
                             float           _tol)
{
    // create object
    nco_crcf nco = nco_crcf_create(_type);

    // set phase and frequency
    nco_crcf_set_phase(nco, _phase);
    nco_crcf_set_frequency(nco, _frequency);

    // run trials
    unsigned int i;
    for (i=0; i<_num_samples; i++) {
        // compute complex output
        float complex y_test;
        nco_crcf_cexpf(nco, &y_test);

        // compare to expected output
        float complex y = _sincos[i];

        // run tests
        CONTEND_DELTA( crealf(y_test), crealf(y), _tol );
        CONTEND_DELTA( cimagf(y_test), cimagf(y), _tol );

        // step oscillator
        nco_crcf_step(nco);
    }

    // destroy object
    nco_crcf_destroy(nco);
}


// test floating point precision nco phase
void autotest_nco_crcf_frequency()
{
    // error tolerance (higher for NCO)
    float tol = 0.04f;

    // test frequencies with irrational values
    nco_crcf_frequency_test(LIQUID_NCO, 0.0f, 0.707106781186547, nco_sincos_fsqrt1_2, 256, tol); // 1/sqrt(2)
    nco_crcf_frequency_test(LIQUID_NCO, 0.0f, 0.577350269189626, nco_sincos_fsqrt1_3, 256, tol); // 1/sqrt(3)
    nco_crcf_frequency_test(LIQUID_NCO, 0.0f, 0.447213595499958, nco_sincos_fsqrt1_5, 256, tol); // 1/sqrt(5)
    nco_crcf_frequency_test(LIQUID_NCO, 0.0f, 0.377964473009227, nco_sincos_fsqrt1_7, 256, tol); // 1/sqrt(7)
}

