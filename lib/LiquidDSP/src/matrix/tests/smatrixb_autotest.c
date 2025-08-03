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

// test sparse binary matrix methods
void autotest_smatrixb_vmul()
{
    // A = [
    //  1   0   0   0   0   0   0   0   0   0   0   0
    //  0   0   0   1   0   0   0   0   0   0   0   0
    //  1   0   0   0   0   0   0   0   1   1   0   0
    //  0   0   1   0   0   0   1   1   0   0   0   0
    //  0   0   0   0   0   0   0   0   0   0   0   0
    //  0   0   0   0   0   0   1   0   1   0   1   0
    //  1   0   1   0   0   0   0   0   0   0   1   1
    //  0   0   1   0   0   1   1   0   0   0   0   0]
    //
    // x = [   1   1   0   0   1   0   0   1   1   1   0   1 ]
    // y = [   1   0   1   1   0   1   0   0 ]
    //
    
    // create sparse matrix and set values
    smatrixb A = smatrixb_create(8,12);
    smatrixb_set(A,0,0,  1);
    smatrixb_set(A,2,0,  1);
    smatrixb_set(A,6,0,  1);
    smatrixb_set(A,3,2,  1);
    smatrixb_set(A,6,2,  1);
    smatrixb_set(A,7,2,  1);
    smatrixb_set(A,1,3,  1);
    smatrixb_set(A,7,5,  1);
    smatrixb_set(A,3,6,  1);
    smatrixb_set(A,5,6,  1);
    smatrixb_set(A,7,6,  1);
    smatrixb_set(A,3,7,  1);
    smatrixb_set(A,2,8,  1);
    smatrixb_set(A,5,8,  1);
    smatrixb_set(A,2,9,  1);
    smatrixb_set(A,5,10, 1);
    smatrixb_set(A,6,10, 1);
    smatrixb_set(A,6,11, 1);

    // generate vectors
    unsigned char x[12]     = {1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1};
    unsigned char y_test[8] = {1, 0, 1, 1, 0, 1, 0, 0};
    unsigned char y[8];

    // multiply and run test
    smatrixb_vmul(A,x,y);

    CONTEND_EQUALITY( y[0], y_test[0] );
    CONTEND_EQUALITY( y[1], y_test[1] );
    CONTEND_EQUALITY( y[2], y_test[2] );
    CONTEND_EQUALITY( y[3], y_test[3] );
    CONTEND_EQUALITY( y[4], y_test[4] );
    CONTEND_EQUALITY( y[5], y_test[5] );
    CONTEND_EQUALITY( y[6], y_test[6] );
    CONTEND_EQUALITY( y[7], y_test[7] );

    // print results (verbose)
    if (liquid_autotest_verbose) {
        printf("\ncompact form:\n");
        smatrixb_print(A);

        printf("\nexpanded form:\n");
        smatrixb_print_expanded(A);

        unsigned int i;
        unsigned int j;

        printf("x = [");
        for (j=0; j<12; j++) printf("%2u", x[j]);
        printf(" ];\n");

        printf("y      = [");
        for (i=0; i<8; i++) printf("%2u", y[i]);
        printf(" ];\n");

        printf("y_test = [");
        for (i=0; i<8; i++) printf("%2u", y_test[i]);
        printf(" ];\n");
    }

    // destroy matrix object
    smatrixb_destroy(A);
}

// test sparse binary matrix multiplication
void autotest_smatrixb_mul()
{
    // a: [8 x 12]
    unsigned char a_test[96] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0};

    // b: [12 x 5]
    unsigned char b_test[60] = {
        1, 1, 0, 0, 0,
        0, 0, 0, 0, 1,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 0, 0,
        0, 1, 0, 0, 1,
        1, 0, 0, 1, 0,
        0, 1, 0, 0, 0};

    // output: [8 x 5]
    unsigned char c_test[40] = {
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0};

    smatrixb a = smatrixb_create_array(a_test,  8,12);
    smatrixb b = smatrixb_create_array(b_test, 12, 5);
    smatrixb c = smatrixb_create(8, 5);

    // compute output
    smatrixb_mul(a,b,c);

    // print results (verbose)
    if (liquid_autotest_verbose) {
        printf("a:\n"); smatrixb_print_expanded(a);
        printf("b:\n"); smatrixb_print_expanded(b);
        printf("c:\n"); smatrixb_print_expanded(c);
    }

    unsigned int i;
    unsigned int j;
    for (i=0; i<8; i++) {
        for (j=0; j<5; j++) {
            CONTEND_EQUALITY( smatrixb_get(c,i,j), c_test[i*5+j]);
        }
    }
    
    // destroy objects
    smatrixb_destroy(a);
    smatrixb_destroy(b);
    smatrixb_destroy(c);
}

// 
void autotest_smatrixb_mulf()
{
    // A = [
    //  1   0   0   0   0   0   0   0   0   0   0   0
    //  0   0   0   1   0   0   0   0   0   0   0   0
    //  1   0   0   0   0   0   0   0   1   1   0   0
    //  0   0   1   0   0   0   1   1   0   0   0   0
    //  0   0   0   0   0   0   0   0   0   0   0   0
    //  0   0   0   0   0   0   1   0   1   0   1   0
    //  1   0   1   0   0   0   0   0   0   0   1   1
    //  0   0   1   0   0   1   1   0   0   0   0   0]
    //
    
    float tol = 1e-6f;

    // create sparse matrix and set values
    smatrixb A = smatrixb_create(8,12);
    smatrixb_set(A,0,0,  1);
    smatrixb_set(A,2,0,  1);
    smatrixb_set(A,6,0,  1);
    smatrixb_set(A,3,2,  1);
    smatrixb_set(A,6,2,  1);
    smatrixb_set(A,7,2,  1);
    smatrixb_set(A,1,3,  1);
    smatrixb_set(A,7,5,  1);
    smatrixb_set(A,3,6,  1);
    smatrixb_set(A,5,6,  1);
    smatrixb_set(A,7,6,  1);
    smatrixb_set(A,3,7,  1);
    smatrixb_set(A,2,8,  1);
    smatrixb_set(A,5,8,  1);
    smatrixb_set(A,2,9,  1);
    smatrixb_set(A,5,10, 1);
    smatrixb_set(A,6,10, 1);
    smatrixb_set(A,6,11, 1);

    // generate vectors
    float x[36] = {
      -4.3,  -0.7,   3.7,
      -1.7,   2.8,   4.3,
       2.0,   1.9,   0.6,
       3.6,   1.0,  -3.7,
       4.3,   0.7,   2.1,
       4.6,   0.5,   0.8,
       1.6,  -3.8,  -0.8,
      -1.9,  -2.1,   2.8,
      -1.5,   2.5,   0.8,
       8.4,   1.5,  -3.1,
      -5.8,   0.0,   2.5,
      -4.9,  -2.1,  -1.5};

    float y_test[24] = {
       -4.3,   -0.7,    3.7,
        3.6,    1.0,   -3.7,
        2.6,    3.3,    1.4,
        1.7,   -4.0,    2.6,
        0.0,    0.0,    0.0,
       -5.7,   -1.3,    2.5,
      -13.0,   -0.9,    5.3,
        8.2,   -1.4,    0.6};

    float y[24];

    // multiply and run test
    smatrixb_mulf(A,
                  x,12,3,
                  y, 8,3);

    unsigned int i;
    for (i=0; i<24; i++)
        CONTEND_DELTA( y[i], y_test[i], tol );

    // print results (verbose)
    if (liquid_autotest_verbose) {
        printf("A:\n");
        smatrixb_print_expanded(A);

        printf("x = [\n");
        for (i=0; i<36; i++) printf("%6.2f%s", x[i], ((i+1)%3)==0 ? "\n" : "");
        printf(" ];\n");

        printf("y = [\n");
        for (i=0; i<24; i++) printf("%6.2f%s", y[i], ((i+1)%3)==0 ? "\n" : "");
        printf(" ];\n");

        printf("y_test = [\n");
        for (i=0; i<24; i++) printf("%6.2f%s", y_test[i], ((i+1)%3)==0 ? "\n" : "");
        printf(" ];\n");
    }

    // destroy matrix object
    smatrixb_destroy(A);
}

// 
void autotest_smatrixb_vmulf()
{
    // A = [
    //  1   0   0   0   0   0   0   0   0   0   0   0
    //  0   0   0   1   0   0   0   0   0   0   0   0
    //  1   0   0   0   0   0   0   0   1   1   0   0
    //  0   0   1   0   0   0   1   1   0   0   0   0
    //  0   0   0   0   0   0   0   0   0   0   0   0
    //  0   0   0   0   0   0   1   0   1   0   1   0
    //  1   0   1   0   0   0   0   0   0   0   1   1
    //  0   0   1   0   0   1   1   0   0   0   0   0]
    //
    // x = [3.4,-5.7, 0.3, 2.3, 1.9, 3.9, 2.3,-4.0,-0.5, 1.5,-0.6,-1.0]^T
    // y = [3.4, 2.3, 4.4,-1.4, 0.0, 1.2, 2.1, 6.5]^T
    //
    
    float tol = 1e-6f;

    // create sparse matrix and set values
    smatrixb A = smatrixb_create(8,12);
    smatrixb_set(A,0,0,  1);
    smatrixb_set(A,2,0,  1);
    smatrixb_set(A,6,0,  1);
    smatrixb_set(A,3,2,  1);
    smatrixb_set(A,6,2,  1);
    smatrixb_set(A,7,2,  1);
    smatrixb_set(A,1,3,  1);
    smatrixb_set(A,7,5,  1);
    smatrixb_set(A,3,6,  1);
    smatrixb_set(A,5,6,  1);
    smatrixb_set(A,7,6,  1);
    smatrixb_set(A,3,7,  1);
    smatrixb_set(A,2,8,  1);
    smatrixb_set(A,5,8,  1);
    smatrixb_set(A,2,9,  1);
    smatrixb_set(A,5,10, 1);
    smatrixb_set(A,6,10, 1);
    smatrixb_set(A,6,11, 1);

    // generate vectors
    float x[12] = {
        3.4,  -5.7,   0.3,   2.3,   1.9,   3.9,
        2.3,  -4.0,  -0.5,   1.5,  -0.6,  -1.0};

    float y_test[8] = {
        3.4,   2.3,   4.4,  -1.4,   0.0,   1.2,   2.1,   6.5};

    float y[8];

    // multiply and run test
    smatrixb_vmulf(A,x,y);

    CONTEND_DELTA( y[0], y_test[0], tol );
    CONTEND_DELTA( y[1], y_test[1], tol );
    CONTEND_DELTA( y[2], y_test[2], tol );
    CONTEND_DELTA( y[3], y_test[3], tol );
    CONTEND_DELTA( y[4], y_test[4], tol );
    CONTEND_DELTA( y[5], y_test[5], tol );
    CONTEND_DELTA( y[6], y_test[6], tol );
    CONTEND_DELTA( y[7], y_test[7], tol );

    // print results (verbose)
    if (liquid_autotest_verbose) {
        printf("\ncompact form:\n");
        smatrixb_print(A);

        printf("\nexpanded form:\n");
        smatrixb_print_expanded(A);

        unsigned int i;
        unsigned int j;

        printf("x = [");
        for (j=0; j<12; j++) printf("%8.4f", x[j]);
        printf(" ];\n");

        printf("y      = [");
        for (i=0; i<8; i++) printf("%8.4f", y[i]);
        printf(" ];\n");

        printf("y_test = [");
        for (i=0; i<8; i++) printf("%8.4f", y_test[i]);
        printf(" ];\n");
    }

    // destroy matrix object
    smatrixb_destroy(A);
}

