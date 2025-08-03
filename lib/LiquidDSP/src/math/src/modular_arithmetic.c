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

//
// Modular arithmetic and related integer mathematics
//

#include <stdio.h>
#include <stdlib.h>
#include "liquid.internal.h"

// determine if number is prime (slow, simple method)
// https://en.ikipedia.org/wiki/Primality_test#Pseudocode
// (thanks to K. Rosenberg for the tip)
int liquid_is_prime(unsigned int _n)
{
    // check base cases (0, 1, 2, 3, divisible by 2, divisible by 3)
    if      ( _n <= 1)  return 0;
    else if ( _n <= 3)  return 1;
    else if (!(_n & 1)) return 0; // divisible by 2
    else if (!(_n % 3)) return 0; // divisible by 3

    unsigned int r=5;
    while ( r*r <= _n ) {
        if ( (_n % r) == 0 || (_n % (r+2)) == 0 )
            return 0;
        r += 6;
    }
    return 1;
}

// compute number's prime factors
//  _n          :   number to factor
//  _factors    :   pre-allocated array of factors [size: LIQUID_MAX_FACTORS x 1]
//  _num_factors:   number of factors found, sorted ascending
int liquid_factor(unsigned int   _n,
                  unsigned int * _factors,
                  unsigned int * _num_factors)
{
    unsigned int k;
    unsigned int n = _n;
    unsigned int num_factors = 0;
    do {
        for (k=2; k<=n; k++) {
            if ( (n % k) == 0) {
                // k factors _n; append to list
                _factors[num_factors++] = k;
                n /= k;
                break;
            }
        }
    } while (n > 1 && num_factors < LIQUID_MAX_FACTORS);

    if (n > 1 && num_factors == LIQUID_MAX_FACTORS)
        return liquid_error(LIQUID_EICONFIG,"liquid_factor(), could not factor %u in %u numbers", _n, LIQUID_MAX_FACTORS);

    *_num_factors = num_factors;
    return LIQUID_OK;
}

// compute number's unique prime factors
//  _n          :   number to factor
//  _factors    :   pre-allocated array of factors [size: LIQUID_MAX_FACTORS x 1]
//  _num_factors:   number of unique factors found, sorted ascending
int liquid_unique_factor(unsigned int   _n,
                         unsigned int * _factors,
                         unsigned int * _num_factors)
{
    unsigned int k;
    unsigned int n = _n;
    unsigned int num_factors = 0;
    do {
        for (k=2; k<=n; k++) {
            if ( (n % k) == 0) {
                // k factors _n; append to list
                _factors[num_factors] = k;
                n /= k;
                
                if (num_factors == 0)
                    num_factors++;
                else if (_factors[num_factors-1] != k)
                    num_factors++;

                break;
            }
        }
    } while (n > 1 && num_factors < LIQUID_MAX_FACTORS);

    if (n > 1 && num_factors == LIQUID_MAX_FACTORS)
        return liquid_error(LIQUID_EICONFIG,"liquid_unqiue_factor(), could not factor %u in %u numbers", _n, LIQUID_MAX_FACTORS);

    *_num_factors = num_factors;
    return LIQUID_OK;
}

// compute greatest common divisor between to integers \(p\) and \(q\)
unsigned int liquid_gcd(unsigned int _p,
                        unsigned int _q)
{
    // check base cases
    if (_p == 0 || _q == 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_gcd(%u,%u), input cannot be zero", _p, _q);
        return 0;
    } else if (_p == 1 || _q == 1) {
        return 1;
    } else if (_p == _q) {
        return _p;
    } else if (_p < _q) {
        return liquid_gcd(_q, _p);
    }

    // dumb, slow method
    unsigned int gcd = 1;
    unsigned int r   = 2; // root
    while ( r <= _q ) {
        while ((_p % r)==0 && (_q % r) == 0) {
            _p /= r;
            _q /= r;
            gcd *= r;
        }
        r += (r == 2) ? 1 : 2;
    }
    
    return gcd;
}

// compute c = base^exp (mod n)
unsigned int liquid_modpow(unsigned int _base,
                           unsigned int _exp,
                           unsigned int _n)
{
    unsigned int c = 1;
    unsigned int i;
    for (i=0; i<_exp; i++)
        c = (c * _base) % _n;

    return c;
}

// find smallest primitive root of _n
unsigned int liquid_primitive_root(unsigned int _n)
{
    // TODO : flesh out this function
    return 1;
}

// find smallest primitive root of _n (assuming _n is prime)
unsigned int liquid_primitive_root_prime(unsigned int _n)
{
    // find unique factors of _n-1
    unsigned int unique_factors[LIQUID_MAX_FACTORS];
    unsigned int num_unique_factors = 0;
    unsigned int n = _n-1;
    unsigned int k;
    do {
        for (k=2; k<=n; k++) {
            if ( (n%k)==0 ) {
                // k is a factor of (_n-1)
                n /= k;

                // add element to end of table
                unique_factors[num_unique_factors] = k;

                // increment counter only if element is unique
                if (num_unique_factors == 0)
                    num_unique_factors++;
                else if (unique_factors[num_unique_factors-1] != k)
                    num_unique_factors++;
                break;
            }
        }
    } while (n > 1 && num_unique_factors < LIQUID_MAX_FACTORS);

#if 0
    // print unique factors
    printf("found %u unique factors of n-1 = %u\n", num_unique_factors, _n-1);
    for (k=0; k<num_unique_factors; k++)
        printf("  %3u\n", unique_factors[k]);
#endif

    // search for minimum integer for which
    //   g^( (_n-1)/m ) != 1 (mod _n)
    // for all unique roots 'm'
    unsigned int g;
    for (g=2; g<_n; g++) {
        int is_root = 1;
        for (k=0; k<num_unique_factors; k++) {
            unsigned int e = (_n-1) / unique_factors[k];
            if ( liquid_modpow(g,e,_n) == 1 ) {
                // not a root
                is_root = 0;
                break;
            }
        }

        if (is_root) {
            //printf("  %u is a primitive root!\n", g);
            break;
        }
    }

    return g;
}

// Euler's totient function
unsigned int liquid_totient(unsigned int _n)
{
    // find unique prime factors
    unsigned int t = _n;    // totient
    unsigned int n = _n;
    unsigned int p = 0;     // last unique prime factor
    unsigned int k;
    do {
        for (k=2; k<=n; k++) {
            if ( (n%k)==0 ) {
                n /= k;

                if (p != k) {
                    // factor is unique
                    t *= k-1;
                    t /= k;
                }

                p = k;
                break;
            }
        }
    } while (n > 1);

    return t;
}

