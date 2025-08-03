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

// peak callback function; value nearest {p, p, p, ...} where p = 1/sqrt(2)
float gasearch_autotest_peak_callback(void * _userdata, chromosome _c)
{
    unsigned int i, n = chromosome_get_num_traits(_c);
    float u     = 1.0f;
    float sig   = 0.2f;
    float p     = M_SQRT1_2;
    for (i=0; i<n; i++) {
        // extract chromosome values
        float v = chromosome_valuef(_c,i);
        float e = v - p;
        u *= exp(-e*e/(2*sig*sig));
    }
    return u;
}

// find values which maximize function
void autotest_gasearch_peak()
{
    unsigned int num_parameters     = 16;   // dimensionality of search (minimum 1)
    unsigned int bits_per_parameter =  6;   // parameter resolution
    unsigned int num_iterations     = 8000; // number of iterations to run
    unsigned int population_size    = 16;   // GA population size
    float        mutation_rate      = 0.2f; // GA mutation rate
    float        tol                = 0.1f; // error tolerance

    unsigned int i;
    float optimum_utility;

    // create prototype chromosome
    chromosome prototype = chromosome_create_basic(num_parameters, bits_per_parameter);

    // create gasearch object
    gasearch ga = gasearch_create_advanced(gasearch_autotest_peak_callback, NULL, prototype,
                    LIQUID_OPTIM_MAXIMIZE, population_size, mutation_rate);

    // execute search at once
    optimum_utility = gasearch_run(ga, num_iterations, 1e6f);

    // get optimum utility
    gasearch_getopt(ga, prototype, &optimum_utility);
    float v_opt[num_parameters];
    for (i=0; i<num_parameters; i++)
        v_opt[i] = chromosome_valuef(prototype, i);

    if (liquid_autotest_verbose) {
        gasearch_print(ga);
        printf(" opt: [  %6.4f] ", optimum_utility);
        chromosome_printf(prototype);
    }

    // destroy search object
    chromosome_destroy(prototype);
    gasearch_destroy(ga);

    // test results, optimum at {p, p, p, ...} where p = 1/sqrt(2)
    for (i=0; i<num_parameters; i++)
        CONTEND_DELTA(v_opt[i], M_SQRT1_2, tol)

    // test value of utility (should be nearly 1)
    CONTEND_GREATER_THAN( optimum_utility, 0.70f )
}

// test chromosome configuration
void autotest_chromosome_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping chromosome config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // test chromosome
    unsigned int bits_per_trait_invalid[8] = {6,6,6,6,6,6,6,1000};
    unsigned int bits_per_trait_valid  [8] = {6,6,6,6,6,6,6,  32};
    CONTEND_ISNULL(chromosome_create(bits_per_trait_invalid, 8))
    CONTEND_ISNULL(chromosome_create(bits_per_trait_valid,   0))
    CONTEND_ISNULL(chromosome_create_basic(0, 12)) // too few traits
    CONTEND_ISNULL(chromosome_create_basic(8,  0)) // bits per trait too small
    CONTEND_ISNULL(chromosome_create_basic(8, 99)) // bits per trait too large

    // create prototype chromosome using basic method
    chromosome prototype = chromosome_create_basic(20, 5);
    CONTEND_EQUALITY(LIQUID_OK, chromosome_print(prototype))
    chromosome_destroy(prototype);

    // create prototype chromosome using more specific method
    prototype = chromosome_create(bits_per_trait_valid, 8);
    CONTEND_EQUALITY  (LIQUID_OK, chromosome_print    (prototype))
    CONTEND_EQUALITY  (LIQUID_OK, chromosome_reset    (prototype))

    // test initialization
    unsigned int values_invalid[] = {999,12,11,13,63,17, 3,123456789}; // invalid because first trait is only 6 bits
    unsigned int values_valid  [] = {  0,12,11,13,63,17, 3,123456789};
    CONTEND_INEQUALITY(LIQUID_OK, chromosome_init (prototype, values_invalid))
    CONTEND_EQUALITY  (LIQUID_OK, chromosome_init (prototype, values_valid  ))
    CONTEND_EQUALITY  (        0, chromosome_value    (prototype,999))
    CONTEND_EQUALITY  (     0.0f, chromosome_valuef   (prototype,999))
    CONTEND_INEQUALITY(LIQUID_OK, chromosome_mutate   (prototype,999))
    CONTEND_INEQUALITY(LIQUID_OK, chromosome_crossover(prototype,prototype,prototype,999))

    // check individual values
    CONTEND_EQUALITY( chromosome_value(prototype, 0),         0)
    CONTEND_EQUALITY( chromosome_value(prototype, 1),        12)
    CONTEND_EQUALITY( chromosome_value(prototype, 2),        11)
    CONTEND_EQUALITY( chromosome_value(prototype, 3),        13)
    CONTEND_EQUALITY( chromosome_value(prototype, 4),        63)
    CONTEND_EQUALITY( chromosome_value(prototype, 5),        17)
    CONTEND_EQUALITY( chromosome_value(prototype, 6),         3)
    CONTEND_EQUALITY( chromosome_value(prototype, 7), 123456789)

    // test initialization (float values)
    float valuesf_invalid[] = {0.0,0.1,0.2,0.3,0.4,0.5,0.6,999,};
    float valuesf_valid  [] = {0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,};
    CONTEND_INEQUALITY(LIQUID_OK, chromosome_initf(prototype, valuesf_invalid))
    CONTEND_EQUALITY  (LIQUID_OK, chromosome_initf(prototype, valuesf_valid  ))

    // check individual values
    CONTEND_DELTA( chromosome_valuef(prototype, 0), 0.0f, 0.02f )
    CONTEND_DELTA( chromosome_valuef(prototype, 1), 0.1f, 0.02f )
    CONTEND_DELTA( chromosome_valuef(prototype, 2), 0.2f, 0.02f )
    CONTEND_DELTA( chromosome_valuef(prototype, 3), 0.3f, 0.02f )
    CONTEND_DELTA( chromosome_valuef(prototype, 4), 0.4f, 0.02f )
    CONTEND_DELTA( chromosome_valuef(prototype, 5), 0.5f, 0.02f )
    CONTEND_DELTA( chromosome_valuef(prototype, 6), 0.6f, 0.02f )
    CONTEND_DELTA( chromosome_valuef(prototype, 7), 0.7f, 0.02f )

    // destroy objects
    chromosome_destroy(prototype);
}

// test configuration
void autotest_gasearch_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping gasearch config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // create prototype chromosome
    chromosome prototype = chromosome_create_basic(8, 12);

    // check invalid function calls
    CONTEND_ISNULL(gasearch_create_advanced(                           NULL, NULL, prototype, LIQUID_OPTIM_MAXIMIZE, 16, 0.1f)) // bad utility function
    CONTEND_ISNULL(gasearch_create_advanced(gasearch_autotest_peak_callback, NULL,      NULL, LIQUID_OPTIM_MAXIMIZE,  0, 0.1f)) // bad parent chromosome
    CONTEND_ISNULL(gasearch_create_advanced(gasearch_autotest_peak_callback, NULL, prototype, LIQUID_OPTIM_MAXIMIZE,  0, 0.1f)) // bad population size
    CONTEND_ISNULL(gasearch_create_advanced(gasearch_autotest_peak_callback, NULL, prototype, LIQUID_OPTIM_MAXIMIZE, -1, 0.1f)) // bad population size
    CONTEND_ISNULL(gasearch_create_advanced(gasearch_autotest_peak_callback, NULL, prototype, LIQUID_OPTIM_MAXIMIZE, 16,-1.0f)) // bad mutation rate

    // create proper object and test configurations
    gasearch ga = gasearch_create(gasearch_autotest_peak_callback, NULL, prototype, LIQUID_OPTIM_MAXIMIZE);
    CONTEND_EQUALITY(LIQUID_OK, gasearch_print(ga))

    // test configurations
    CONTEND_INEQUALITY(LIQUID_OK, gasearch_set_population_size(ga, 0, 8)) // population size too small
    CONTEND_INEQUALITY(LIQUID_OK, gasearch_set_population_size(ga,-1, 8)) // population size too large
    CONTEND_INEQUALITY(LIQUID_OK, gasearch_set_population_size(ga,24, 0)) // selection size too small
    CONTEND_INEQUALITY(LIQUID_OK, gasearch_set_population_size(ga,24,24)) // selection size too large
    CONTEND_EQUALITY  (LIQUID_OK, gasearch_set_population_size(ga,24,12)) // ok
    CONTEND_INEQUALITY(LIQUID_OK, gasearch_set_mutation_rate  (ga,-1.0f)) // mutation rate out of range
    CONTEND_INEQUALITY(LIQUID_OK, gasearch_set_mutation_rate  (ga, 2.0f)) // mutation rate out of range
    CONTEND_EQUALITY  (LIQUID_OK, gasearch_set_mutation_rate  (ga, 0.1f)) // ok

    // destroy objects
    chromosome_destroy(prototype);
    gasearch_destroy(ga);
}

