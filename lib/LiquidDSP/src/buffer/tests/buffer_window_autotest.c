/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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

#include "autotest/autotest.h"
#include "liquid.internal.h"

void autotest_window_config_errors()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping window config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    CONTEND_EXPRESSION(windowcf_create(0)==NULL);
    CONTEND_EXPRESSION(windowf_create (0)==NULL);
}

void autotest_windowf()
{
    float v[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    float *r;   // reader pointer
    float x;    // temporary value holder

    float test0[10] = {0,0,0,0,0,0,0,0,0,0};
    float test1[10] = {0,0,0,0,0,0,1,1,1,1};
    float test2[10] = {0,0,1,1,1,1,9,8,7,6};
    float test3[10] = {1,1,9,8,7,6,3,3,3,3};
    float test4[10] = {7,6,3,3,3,3,5,5,5,5};
    float test5[6]  = {3,3,5,5,5,5};
    float test6[6]  = {5,5,5,5,6,7};
    float test7[10] = {0,0,0,0,5,5,5,5,6,7};
    float test8[10] = {0,0,0,0,0,0,0,0,0,0};

    // create window
    // 0 0 0 0 0 0 0 0 0 0
    windowf w = windowf_create(10);

    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test0,10*sizeof(float));

    // push 4 elements
    // 0 0 0 0 0 0 1 1 1 1
    windowf_push(w, 1);
    windowf_push(w, 1);
    windowf_push(w, 1);
    windowf_push(w, 1);

    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test1,10*sizeof(float));

    // push 4 more elements
    // 0 0 1 1 1 1 9 8 7 6
    windowf_write(w, v, 4);

    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test2,10*sizeof(float));

    // push 4 more elements
    // 1 1 9 8 7 6 3 3 3 3
    windowf_push(w, 3);
    windowf_push(w, 3);
    windowf_push(w, 3);
    windowf_push(w, 3);

    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test3,10*sizeof(float));

    // test indexing operation
    windowf_index(w, 0, &x);    CONTEND_EQUALITY(x, 1);
    windowf_index(w, 1, &x);    CONTEND_EQUALITY(x, 1);
    windowf_index(w, 2, &x);    CONTEND_EQUALITY(x, 9);
    windowf_index(w, 3, &x);    CONTEND_EQUALITY(x, 8);
    windowf_index(w, 4, &x);    CONTEND_EQUALITY(x, 7);
    windowf_index(w, 5, &x);    CONTEND_EQUALITY(x, 6);
    windowf_index(w, 6, &x);    CONTEND_EQUALITY(x, 3);
    windowf_index(w, 7, &x);    CONTEND_EQUALITY(x, 3);
    windowf_index(w, 8, &x);    CONTEND_EQUALITY(x, 3);
    windowf_index(w, 9, &x);    CONTEND_EQUALITY(x, 3);
    CONTEND_INEQUALITY( windowf_index(w,999, &x), LIQUID_OK); // out of range

    // push 4 more elements
    // 7 6 3 3 3 3 5 5 5 5
    windowf_push(w, 5);
    windowf_push(w, 5);
    windowf_push(w, 5);
    windowf_push(w, 5);

    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test4,10*sizeof(float));
    if (liquid_autotest_verbose)
        windowf_debug_print(w);

    // recreate window (truncate to last 6 elements)
    // 3 3 5 5 5 5
    w = windowf_recreate(w,6);
    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test5,6*sizeof(float));

    // push 2 more elements
    // 5 5 5 5 6 7
    windowf_push(w, 6);
    windowf_push(w, 7);
    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test6,6*sizeof(float));

    // recreate window (extend to 10 elements)
    // 0 0 0 0 5 5 5 5 6 7
    w = windowf_recreate(w,10);
    windowf_read(w,&r);
    CONTEND_SAME_DATA(r,test7,10*sizeof(float));

    // reset
    // 0 0 0 0 0 0 0 0 0 0
    windowf_reset(w);

    windowf_read(w, &r);
    CONTEND_SAME_DATA(r,test8,10*sizeof(float));

    windowf_destroy(w);
}

void autotest_window_copy()
{
    // create window
    unsigned int wlen = 20;
    windowcf q0 = windowcf_create(wlen);

    // write some values
    unsigned int i;
    for (i=0; i<wlen; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        windowcf_push(q0, v);
    }

    // copy object
    windowcf q1 = windowcf_copy(q0);

    // write a few more values
    for (i=0; i<wlen/2; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        windowcf_push(q0, v);
        windowcf_push(q1, v);
    }

    // read buffers and compare
    float complex * r0, * r1;
    windowcf_read(q0, &r0);
    windowcf_read(q1, &r1);
    CONTEND_SAME_DATA(r0, r1, wlen*sizeof(float complex));

    // destroy objects
    windowcf_destroy(q0);
    windowcf_destroy(q1);
}

