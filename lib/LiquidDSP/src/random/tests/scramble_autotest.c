/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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
#include <string.h>
#include <math.h>

#include "autotest/autotest.h"
#include "liquid.internal.h"

// compute basic entropy metric
float liquid_scramble_test_entropy(unsigned char * _x,
                                   unsigned int _n)
{
    unsigned int i;

    // count ones
    unsigned int num_ones=0;
    for (i=0; i<_n; i++)
        num_ones += liquid_c_ones[_x[i]];

    // compute probabilities (add small value to prevent
    // possible log(0))
    float p1 = (float)num_ones / (float)(8*_n) + 1e-12f;
    float p0 = 1.0 - p1;
    return -p0*log2f(p0) - p1*log2f(p1);
}

// helper function to keep code base small
void liquid_scramble_test(unsigned int _n)
{
    unsigned char x[_n];    // input data
    unsigned char y[_n];    // scrambled data
    unsigned char z[_n];    // unscrambled data

    unsigned int i;

    // initialize data array
    for (i=0; i<_n; i++)
        x[i] = 0x00;

    // scramble input
    memmove(y,x,_n);
    scramble_data(y,_n);

    // unscramble result
    memmove(z,y,_n);
    unscramble_data(z,_n);

    // ensure data are equivalent
    CONTEND_SAME_DATA(x,z,_n*sizeof(unsigned char));

    // compute entropy metric
    float H = liquid_scramble_test_entropy(y,_n);
    CONTEND_EXPRESSION( H > 0.8f );
}


// test unscrambling of soft bits (helper function to keep code base small)
void liquid_scramble_soft_test(unsigned int _n)
{
    unsigned char msg_org[_n];      // input data
    unsigned char msg_enc[_n];      // scrambled data 
    unsigned char msg_soft[8*_n];   // scrambled data (soft bits)
    unsigned char msg_dec[_n];      // unscrambled data

    unsigned int i;

    // initialize data array (random)
    for (i=0; i<_n; i++)
        msg_org[i] = rand() & 0xff;

    // scramble input
    memmove(msg_enc, msg_org, _n);
    scramble_data(msg_enc,_n);

    // convert to soft bits
    for (i=0; i<_n; i++)
        liquid_unpack_soft_bits(msg_enc[i], 8, &msg_soft[8*i]);

    // unscramble result
    unscramble_data_soft(msg_soft, _n);

    // unpack soft bits
    for (i=0; i<_n; i++) {
        unsigned int sym_out;
        liquid_pack_soft_bits(&msg_soft[8*i], 8, &sym_out);
        msg_dec[i] = sym_out;
    }

    // ensure data are equivalent
    CONTEND_SAME_DATA(msg_org, msg_dec, _n);
}

//
// AUTOTESTS : simple data scrambling
//
void autotest_scramble_n16()     { liquid_scramble_test(16);  };
void autotest_scramble_n64()     { liquid_scramble_test(64);  };
void autotest_scramble_n256()    { liquid_scramble_test(256); };

void autotest_scramble_n11()     { liquid_scramble_test(11);  };
void autotest_scramble_n33()     { liquid_scramble_test(33);  };
void autotest_scramble_n277()    { liquid_scramble_test(277); };

//
// AUTOTESTS : soft data scrambling
//
void autotest_scramble_soft_n16()     { liquid_scramble_soft_test(16);  };
void autotest_scramble_soft_n64()     { liquid_scramble_soft_test(64);  };
void autotest_scramble_soft_n256()    { liquid_scramble_soft_test(256); };

void autotest_scramble_soft_n11()     { liquid_scramble_soft_test(11);  };
void autotest_scramble_soft_n33()     { liquid_scramble_soft_test(33);  };
void autotest_scramble_soft_n277()    { liquid_scramble_soft_test(277); };


