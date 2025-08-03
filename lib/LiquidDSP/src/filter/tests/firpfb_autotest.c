/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
#include "autotest/autotest.h"
#include "liquid.h"

void autotest_firpfb_impulse_response()
{
    // Initialize variables
    float tol=1e-4f;

    // k=2, m=3, beta=0.3, npfb=4;
    // h=rrcos(k*npfb,m,beta);
    float h[48] = {
     -0.033116,  -0.024181,  -0.006284,   0.018261, 
      0.045016,   0.068033,   0.080919,   0.078177, 
      0.056597,   0.016403,  -0.038106,  -0.098610, 
     -0.153600,  -0.189940,  -0.194900,  -0.158390, 
     -0.075002,   0.054511,   0.222690,   0.415800, 
      0.615340,   0.800390,   0.950380,   1.048100, 
      1.082000,   1.048100,   0.950380,   0.800390, 
      0.615340,   0.415800,   0.222690,   0.054511, 
     -0.075002,  -0.158390,  -0.194900,  -0.189940, 
     -0.153600,  -0.098610,  -0.038106,   0.016403, 
      0.056597,   0.078177,   0.080919,   0.068033, 
      0.045016,   0.018261,  -0.006284,  -0.024181
    };

    // filter input
    float noise[12] = {
      0.438310,   1.001900,   0.200600,   0.790040, 
      1.134200,   1.592200,  -0.702980,  -0.937560, 
     -0.511270,  -1.684700,   0.328940,  -0.387780
    };

    // expected filter outputs
    float test[4] = {
        2.05558467194397f,
        1.56922189602661f,
        0.998479744645138,
        0.386125857849177
    };

    // Load filter coefficients externally
    firpfb_rrrf f = firpfb_rrrf_create(4, h, 48);
    
    unsigned int i;
    for (i=0; i<12; i++)
        firpfb_rrrf_push(f,noise[i]);

    float y;
    for (i=0; i<4; i++) {
        firpfb_rrrf_execute(f,i,&y);
        CONTEND_DELTA(test[i],y,tol);
    }
    
    firpfb_rrrf_destroy(f);
}

void autotest_firpfb_crcf_copy()
{
    // create base object with irregular parameters
    unsigned int M = 13, m = 7;
    firpfb_crcf q0 = firpfb_crcf_create_default(M, m);

    // run random samples through filter
    unsigned int i, num_samples = 80;
    for (i=0; i<num_samples; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        firpfb_crcf_push(q0, v);
    }

    // copy object
    firpfb_crcf q1 = firpfb_crcf_copy(q0);

    // run random samples through filter
    for (i=0; i<num_samples; i++) {
        // random input channel and index
        float complex v  = randnf() + _Complex_I*randnf();
        unsigned int idx = rand() % M;

        // push sample through each filter
        firpfb_crcf_push(q0, v);
        firpfb_crcf_push(q1, v);

        // compare outputs
        float complex y0, y1;
        firpfb_crcf_execute(q0, idx, &y0);
        firpfb_crcf_execute(q1, idx, &y1);
        CONTEND_EQUALITY(y0, y1);
    }

    // destroy objects
    firpfb_crcf_destroy(q0);
    firpfb_crcf_destroy(q1);
}

