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

#include "autotest/autotest.h"
#include "liquid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "liquid.internal.h"

void autotest_qnsearch_rosenbrock()
{
    float tol = 1e-2f;                  // error tolerance
    unsigned int num_parameters = 6;    // dimensionality of search (minimum 2)
    unsigned int num_iterations = 4000; // number of iterations to run

    // initialize vector for optimization
    float v_opt[num_parameters];
    unsigned int i;
    for (i=0; i<num_parameters; i++)
        v_opt[i] = 0.0f;

    // create qnsearch object
    qnsearch gs = qnsearch_create(NULL,v_opt,
        num_parameters, liquid_rosenbrock, LIQUID_OPTIM_MINIMIZE);

    // execute search
    float u_opt = qnsearch_execute(gs, num_iterations, 0.0f);

    // destroy gradient descent search object
    qnsearch_destroy(gs);

    // test results, optimum at [1, 1, 1, ... 1];
    for (i=0; i<num_parameters; i++)
        CONTEND_DELTA(v_opt[i], 1.0f, tol);

    // test value of utility (should be nearly 0)
    CONTEND_DELTA( liquid_rosenbrock(NULL, v_opt, num_parameters), 0.0f, tol );
    CONTEND_LESS_THAN( u_opt, tol );
}

// test configuration
void autotest_qnsearch_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping qnsearch config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif

    // test configurations
    float v[8] = {0,0,0,0,0,0,0,0};
    CONTEND_ISNULL(qnsearch_create(NULL, v, 0, liquid_rosenbrock, LIQUID_OPTIM_MINIMIZE)) // no parameters
    CONTEND_ISNULL(qnsearch_create(NULL, v, 8,              NULL, LIQUID_OPTIM_MINIMIZE)) // utility is null

    // create proper object and test configurations
    qnsearch q = qnsearch_create(NULL, v, 8, liquid_rosenbrock, LIQUID_OPTIM_MINIMIZE);
    CONTEND_EQUALITY(LIQUID_OK, qnsearch_print(q))

    // destroy objects
    qnsearch_destroy(q);
}

