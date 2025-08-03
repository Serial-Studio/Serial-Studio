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

// test sparse floating-point vector multiplication
void autotest_smatrixf_vmul()
{
    float tol = 1e-6f;

    // A = [
    //  0 0 0 0 4
    //  0 0 0 0 0
    //  0 0 0 3 0
    //  2 0 0 0 1

    // create sparse matrix and set values
    smatrixf A = smatrixf_create(4, 5);
    smatrixf_set(A, 0,4, 4);
    smatrixf_set(A, 2,3, 3);
    smatrixf_set(A, 3,0, 2);
    smatrixf_set(A, 3,4, 0);
    smatrixf_set(A, 3,4, 1);

    // initialize input vector
    float x[5] = {7, 1, 5, 2, 2};

    float y_test[4] = {8, 0, 6, 16};
    float y[4];

    // multiply and run test
    smatrixf_vmul(A,x,y);

    // check values
    CONTEND_DELTA( y[0], y_test[0], tol );
    CONTEND_DELTA( y[1], y_test[1], tol );
    CONTEND_DELTA( y[2], y_test[2], tol );
    CONTEND_DELTA( y[3], y_test[3], tol );

    smatrixf_destroy(A);
}
// test sparse floating-point matrix multiplication
void autotest_smatrixf_mul()
{
    float tol = 1e-6f;

    // initialize matrices
    smatrixf a = smatrixf_create(4, 5);
    smatrixf b = smatrixf_create(5, 3);
    smatrixf c = smatrixf_create(4, 3);

    // initialize 'a'
    // 0 0 0 0 4
    // 0 0 0 0 0
    // 0 0 0 3 0
    // 2 0 0 0 1
    smatrixf_set(a, 0,4, 4);
    smatrixf_set(a, 2,3, 3);
    smatrixf_set(a, 3,0, 2);
    smatrixf_set(a, 3,4, 0);
    smatrixf_set(a, 3,4, 1);

    // initialize 'b'
    // 7 6 0
    // 0 0 0
    // 0 0 0
    // 0 5 0
    // 2 0 0
    smatrixf_set(b, 0,0, 7);
    smatrixf_set(b, 0,1, 6);
    smatrixf_set(b, 3,1, 5);
    smatrixf_set(b, 4,0, 2);

    // compute 'c'
    //  8   0   0
    //  0   0   0
    //  0  15   0
    // 16  12   0
    smatrixf_mul(a,b,c);

    float c_test[12] = {
         8,   0,   0,
         0,   0,   0,
         0,  15,   0,
        16,  12,   0};

    // check values
    unsigned int i;
    unsigned int j;
    for (i=0; i<4; i++) {
        for (j=0; j<3; j++) {
            CONTEND_DELTA(smatrixf_get(c,i,j),
                          matrixf_access(c_test,4,3,i,j),
                          tol);
        }
    }

    smatrixf_destroy(a);
    smatrixf_destroy(b);
    smatrixf_destroy(c);
}
