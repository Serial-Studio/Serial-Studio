/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/resource.h>
#include "liquid.internal.h"

// 
void benchmark_gmskmodem_modulate(struct rusage *_start,
                                  struct rusage *_finish,
                                  unsigned long int *_num_iterations)
{
    // options
    unsigned int k=2;   // filter samples/symbol
    unsigned int m=3;   // filter delay (symbols)
    float BT=0.3f;      // bandwidth-time product

    // create modem object
    gmskmod mod   = gmskmod_create(k, m, BT);

    float complex x[k];
    unsigned int symbol_in = 0;
    
    unsigned long int i;
    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        gmskmod_modulate(mod, symbol_in, x);
        gmskmod_modulate(mod, symbol_in, x);
        gmskmod_modulate(mod, symbol_in, x);
        gmskmod_modulate(mod, symbol_in, x);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // destroy modem objects
    gmskmod_destroy(mod);
}

// 
void benchmark_gmskmodem_demodulate(struct rusage *_start,
                                    struct rusage *_finish,
                                    unsigned long int *_num_iterations)
{
    // options
    unsigned int k=2;   // filter samples/symbol
    unsigned int m=3;   // filter delay (symbols)
    float BT=0.3f;      // bandwidth-time product

    // create modem object
    gmskdem demod = gmskdem_create(k, m, BT);

    float complex x[k];
    unsigned int symbol_out = 0;
    
    unsigned long int i;
    for (i=0; i<k; i++)
        x[i] = randnf()*cexpf(_Complex_I*2*M_PI*randf());

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        gmskdem_demodulate(demod, x, &symbol_out);
        gmskdem_demodulate(demod, x, &symbol_out);
        gmskdem_demodulate(demod, x, &symbol_out);
        gmskdem_demodulate(demod, x, &symbol_out);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // destroy modem objects
    gmskdem_destroy(demod);
}

