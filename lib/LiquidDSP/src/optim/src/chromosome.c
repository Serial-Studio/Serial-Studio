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

// chromosome object for genetic algorithm search

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

#define LIQUID_CHROMOSOME_MAX_SIZE (32)

// create chromosome with varying bits/trait
//  _bits_per_trait     :   array of bits/trait [size: _num_traits x 1]
//  _num_traits         :   number of traits in this chromosome
chromosome chromosome_create(unsigned int * _bits_per_trait,
                             unsigned int   _num_traits)
{
    // validate input
    unsigned int i;
    if (_num_traits < 1)
        return liquid_error_config("chromosome_create(), must have at least one trait");
    for (i=0; i<_num_traits; i++) {
        if (_bits_per_trait[i] > LIQUID_CHROMOSOME_MAX_SIZE)
            return liquid_error_config("chromosome_create(), bits/trait cannot exceed %u", LIQUID_CHROMOSOME_MAX_SIZE);
    }

    chromosome q;
    q = (chromosome) malloc( sizeof(struct chromosome_s) );
    q->num_traits = _num_traits;

    // initialize internal arrays
    q->bits_per_trait = (unsigned int *) malloc(q->num_traits*sizeof(unsigned int));
    q->max_value =      (unsigned long*) malloc(q->num_traits*sizeof(unsigned long));
    q->traits =         (unsigned long*) malloc(q->num_traits*sizeof(unsigned long));

    // copy/initialize values
    q->num_bits = 0;
    for (i=0; i<q->num_traits; i++) {
        q->bits_per_trait[i] = _bits_per_trait[i];

        q->max_value[i] = 1LU << q->bits_per_trait[i];
        q->traits[i] = 0LU;

        q->num_bits += q->bits_per_trait[i];
    }

    return q;
}

// create basic chromosome
//  _num_traits         :   number of traits in this chromosome
//  _bits_per_trait     :   number of bits/trait for all traits
chromosome chromosome_create_basic(unsigned int _num_traits,
                                   unsigned int _bits_per_trait)
{
    // validate input
    if (_num_traits == 0)
        return liquid_error_config("chromosome_create_basic(), must have at least one trait");
    if (_bits_per_trait == 0 || _bits_per_trait > 64)
        return liquid_error_config("chromosome_create_basic(), bits per trait out of range");

    unsigned int * bpt = (unsigned int *) malloc(_num_traits*sizeof(unsigned int));
    unsigned int i;
    for (i=0; i<_num_traits; i++)
        bpt[i] = _bits_per_trait;

    // create chromosome
    chromosome q = chromosome_create(bpt, _num_traits);

    // free bits/trait array
    free(bpt);

    return q;
}

// create chromosome cloning a parent
chromosome chromosome_create_clone(chromosome _parent)
{
    // create chromosome
    chromosome q = chromosome_create(_parent->bits_per_trait,
                                     _parent->num_traits);

    // copy internal values
    chromosome_copy(_parent, q);

    return q;
}

// copy chromosome
int chromosome_copy(chromosome _parent,
                    chromosome _child)
{
    // copy internal values
    unsigned int i;
    for (i=0; i<_parent->num_traits; i++)
        _child->traits[i] = _parent->traits[i];
    return LIQUID_OK;
}

int chromosome_destroy(chromosome _q)
{
    free(_q->bits_per_trait);
    free(_q->max_value);
    free(_q->traits);
    free(_q);
    return LIQUID_OK;
}

unsigned int chromosome_get_num_traits(chromosome _q)
{
    return _q->num_traits;
}

int chromosome_print(chromosome _q)
{
    unsigned int i,j;
    printf("<liquid.chromosome, ");
    // print one bit at a time
    for (i=0; i<_q->num_traits; i++) {
        for (j=0; j<_q->bits_per_trait[i]; j++) {
            unsigned int bit = (_q->traits[i] >> (_q->bits_per_trait[i]-j-1) ) & 1;
            printf("%c", bit ? '1' : '0');
        }
        
        if (i != _q->num_traits-1)
            printf(".");
    }
    printf(">\n");
    return LIQUID_OK;
}

int chromosome_printf(chromosome _q)
{
    unsigned int i;
    printf("<liquid.chromosome, ");
    for (i=0; i<_q->num_traits; i++)
        printf("%6.3f", chromosome_valuef(_q,i));
    printf(">\n");
    return LIQUID_OK;
}

// clear chromosome (set traits to zero)
int chromosome_reset(chromosome _q)
{
    unsigned int i;
    for (i=0; i<_q->num_traits; i++)
        _q->traits[i] = 0;
    return LIQUID_OK;
}

// initialize chromosome on integer values
int chromosome_init(chromosome     _c,
                    unsigned int * _v)
{
    unsigned int i;
    for (i=0; i<_c->num_traits; i++) {
        //printf("===> [%3u] bits:%3u, max:%12lu, value:%12lu\n", i, _c->bits_per_trait[i], _c->max_value[i], _v[i]);
        if (_v[i] >= _c->max_value[i])
            return liquid_error(LIQUID_EIRANGE,"chromosome_init(), value exceeds maximum");

        _c->traits[i] = _v[i];
    }
    return LIQUID_OK;
}

// initialize chromosome on floating-point values
int chromosome_initf(chromosome _c,
                     float *    _v)
{
    unsigned int i;
    for (i=0; i<_c->num_traits; i++) {
        if (_v[i] < 0.0f || _v[i] > 1.0f)
            return liquid_error(LIQUID_EIRANGE,"chromosome_initf(), value must be in [0,1]");

        // quantize sample
        unsigned long N = 1LU << _c->bits_per_trait[i];
        _c->traits[i] = (unsigned long) floorf( _v[i] * N );
        //printf("===> [%3u] quantizing %8.2f, bits:%3u, N:%12lu, trait:%12lu/%12lu => %12.8f\n",
        //    i, _v[i], _c->bits_per_trait[i], N, _c->traits[i], _c->max_value[i], chromosome_valuef(_c,i));
    }
    return LIQUID_OK;
}

// mutate bit at _index
int chromosome_mutate(chromosome   _q,
                      unsigned int _index)
{
    if (_index >= _q->num_bits)
        return liquid_error(LIQUID_EIRANGE,"chromosome_mutate(), maximum index exceeded");

    // search for
    unsigned int i;
    unsigned int t=0;
    for (i=0; i<_q->num_traits; i++) {
        unsigned int b = _q->bits_per_trait[i];
        if (t == _index) {
            _q->traits[i] ^= (unsigned long)(1LU << (b-1));
            return LIQUID_OK;
        } else if (t > _index) {
            _q->traits[i-1] ^= (unsigned long)(1LU << (t-_index-1));
            return LIQUID_OK;
        } else {
            t += b;
        }
    }

    _q->traits[i-1] ^= (unsigned long)(1 << (t-_index-1));
    return LIQUID_OK;
}

// crossover parent chromosomes and store in child
//  _p1         :   first parent chromosome
//  _p2         :   second parent chromosome
//  _q          :   child chromosome
//  _threshold  :   crossover point
int chromosome_crossover(chromosome   _p1,
                         chromosome   _p2,
                         chromosome   _q,
                         unsigned int _threshold)
{
    if (_threshold > _q->num_bits)
        return liquid_error(LIQUID_EIRANGE,"chromosome_crossover(), maximum index exceeded");

    // TODO : validate input on all properties of _p1, _p2, and _q

    // find crossover point
    unsigned int i;
    unsigned int t=0;
    for (i=0; i<_q->num_traits; i++) {
        if (t >= _threshold)
            break;
        else
            t += _q->bits_per_trait[i];

        // child gets first parent's traits up until
        // threshold is reached
        _q->traits[i] = _p1->traits[i];
    }

#if 0
    printf("  crossover point   : %u\n", i);
    printf("  accumulator       : %u\n", t);
    printf("  remainder         : %u\n", t - _threshold);
#endif

    // determine if trait is split
    unsigned int rem = t - _threshold;
    if (rem > 0) {
        // split trait on remainder
        unsigned int b = _q->bits_per_trait[i-1];
        unsigned int mask1 = ((1 << (b-rem)) - 1) << rem;
        unsigned int mask2 = ((1 << rem    ) - 1);
        _q->traits[i-1] = (_p1->traits[i-1] & mask1) |
                          (_p2->traits[i-1] & mask2);
#if 0
        printf("  b                 : %u\n", b);
        printf("  mask1             : %.8x\n", mask1);
        printf("  mask2             : %.8x\n", mask2);
#endif
    }

    // finish crossover
    for ( ; i<_q->num_traits; i++) {
        // child gets second parent's traits beyond threshold
        _q->traits[i] = _p2->traits[i];
    }
    return LIQUID_OK;
}
    
int chromosome_init_random(chromosome _q)
{
    unsigned int i;
    for (i=0; i<_q->num_traits; i++)
        _q->traits[i] = rand() & (_q->max_value[i]-1LU);
    return LIQUID_OK;
}

float chromosome_valuef(chromosome   _q,
                        unsigned int _index)
{
    if (_index > _q->num_traits) {
        liquid_error(LIQUID_EIRANGE,"chromosome_valuef(), trait index exceeded");
        return 0.0f;
    }
    return (float) (_q->traits[_index]) / (float)(_q->max_value[_index]-1LU);
}

unsigned int chromosome_value(chromosome   _q,
                              unsigned int _index)
{
    if (_index > _q->num_traits) {
        liquid_error(LIQUID_EIRANGE,"chromosome_value(), trait index exceeded");
        return 0.0f;
    }
    return _q->traits[_index];
}

