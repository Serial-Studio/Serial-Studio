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

#include "autotest/autotest.h"
#include "liquid.internal.h"

// 
// AUTOTESTS: basic sparse matrix functionality
//

// test sparse integer vector multiplication
void autotest_smatrixi_vmul()
{
    // A = [
    //  0 0 0 0 4
    //  0 0 0 0 0
    //  0 0 0 3 0
    //  2 0 0 0 1

    // create sparse matrix and set values
    smatrixi A = smatrixi_create(4, 5);
    smatrixi_set(A, 0,4, 4);
    smatrixi_set(A, 2,3, 3);
    smatrixi_set(A, 3,0, 2);
    smatrixi_set(A, 3,4, 0);
    smatrixi_set(A, 3,4, 1);

    // initialize input vector
    short int x[5] = {7, 1, 5, 2, 2};

    short int y_test[4] = {8, 0, 6, 16};
    short int y[4];

    // multiply and run test
    smatrixi_vmul(A,x,y);

    // check values
    CONTEND_EQUALITY( y[0], y_test[0] );
    CONTEND_EQUALITY( y[1], y_test[1] );
    CONTEND_EQUALITY( y[2], y_test[2] );
    CONTEND_EQUALITY( y[3], y_test[3] );

    smatrixi_destroy(A);
}

// test sparse integer matrix multiplication
void autotest_smatrixi_mul()
{
    // initialize matrices
    smatrixi a = smatrixi_create(4, 5);
    smatrixi b = smatrixi_create(5, 3);
    smatrixi c = smatrixi_create(4, 3);

    // initialize 'a'
    // 0 0 0 0 4
    // 0 0 0 0 0
    // 0 0 0 3 0
    // 2 0 0 0 1
    smatrixi_set(a, 0,4, 4);
    smatrixi_set(a, 2,3, 3);
    smatrixi_set(a, 3,0, 2);
    smatrixi_set(a, 3,4, 0);
    smatrixi_set(a, 3,4, 1);

    // initialize 'b'
    // 7 6 0
    // 0 0 0
    // 0 0 0
    // 0 5 0
    // 2 0 0
    smatrixi_set(b, 0,0, 7);
    smatrixi_set(b, 0,1, 6);
    smatrixi_set(b, 3,1, 5);
    smatrixi_set(b, 4,0, 2);

    // compute 'c'
    //  8   0   0
    //  0   0   0
    //  0  15   0
    // 16  12   0
    smatrixi_mul(a,b,c);

    short int c_test[12] = {
         8,   0,   0,
         0,   0,   0,
         0,  15,   0,
        16,  12,   0};

    // check values
    unsigned int i;
    unsigned int j;
    for (i=0; i<4; i++) {
        for (j=0; j<3; j++) {
            CONTEND_EQUALITY(smatrixi_get(c,i,j),
                             matrixf_access(c_test,4,3,i,j))
        }
    }

    smatrixi_destroy(a);
    smatrixi_destroy(b);
    smatrixi_destroy(c);
}
