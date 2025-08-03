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

#include <complex.h>
#include "autotest/autotest.h"
#include "liquid.h"

//
// AUTOTEST: regular phase-unwrapping
//
void autotest_nco_unwrap_phase()
{
    unsigned int n=32;  // number of steps
    float tol = 1e-6f;  // error tolerance
    
    // initialize data arrays
    float phi[n];       // original array
    float theta[n];     // wrapped array
    float phi_hat[n];   // unwrapped array

    float phi0 = 3.0f;  // initial phase
    float dphi = 0.1f;  // phase step

    unsigned int i;
    for (i=0; i<n; i++) {
        // phase input
        phi[i] = phi0 + i*dphi;

        // wrapped array
        theta[i] = phi[i];
        while (theta[i] >  M_PI) theta[i] -= 2*M_PI;
        while (theta[i] < -M_PI) theta[i] += 2*M_PI;

        // initialize output
        phi_hat[i] = theta[i];
    }

    // unwrap phase
    liquid_unwrap_phase(phi_hat, n);

    // compare input to output
    for (i=0; i<n; i++)
        CONTEND_DELTA( phi[i], phi_hat[i], tol );
}

