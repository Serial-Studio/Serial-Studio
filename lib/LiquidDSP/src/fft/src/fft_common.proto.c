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
// fft_common.c : common utilities specific to precision
//

#include <stdio.h>
#include <stdlib.h>
#include "liquid.internal.h"

struct FFT(plan_s)
{
    // common data
    unsigned int nfft;  // fft size
    TC * x;             // input array pointer (not allocated)
    TC * y;             // output array pointer (not allocated)
    int direction;      // forward/reverse
    int flags;
    liquid_fft_type   type;     // type of transform
    liquid_fft_method method;   // transform method

    // 'execute' function pointer
    FFT(_execute_t) * execute;

    // real even/odd DFT parameters (DCT/DST)
    T * xr; // input array (real)
    T * yr; // output array (real)

    // common data structure shared between specific FFT algorithms
    union {
        // DFT
        struct {
            TC * twiddle;               // twiddle factors
            DOTPROD() * dotprod;        // inner dot products
        } dft;

        // radix-2 transform data
        struct {
            unsigned int m;             // log2(nfft)
            unsigned int * index_rev;   // reversed indices
            TC * twiddle;               // twiddle factors
        } radix2;

        // recursive mixed-radix transform data:
        //  - compute 'Q' FFTs of size 'P'
        //  - apply twiddle factors
        //  - compute 'P' FFTs of size 'Q'
        //  - transpose result
        struct {
            unsigned int P;     // first FFT size
            unsigned int Q;     // second FFT size
            TC * x;             // input buffer (copied)
            TC * t0;            // temporary buffer (small FFT input)
            TC * t1;            // temporary buffer (small FFT output)
            TC * twiddle;       // twiddle factors
            FFT(plan) fft_P;    // sub-transform of size P
            FFT(plan) fft_Q;    // sub-transform of size Q
        } mixedradix;

        // Rader's algorithm for computing FFTs of prime length
        struct {
            unsigned int * seq; // transformation sequence, size: nfft-1
            TC * R;             // DFT of sequence { exp(-j*2*pi*g^i/nfft }, size: nfft-1
            TC * x_prime;       // sub-transform time-domain buffer
            TC * X_prime;       // sub-transform freq-domain buffer
            FFT(plan) fft;      // sub-FFT of size nfft-1
            FFT(plan) ifft;     // sub-IFFT of size nfft-1
        } rader;

        // Rader's alternate algorithm for computing FFTs of prime length
        struct {
            unsigned int nfft_prime;
            unsigned int * seq; // transformation sequence, size: nfft_prime
            TC * R;             // DFT of sequence { exp(-j*2*pi*g^i/nfft }, size: nfft_prime
            TC * x_prime;       // sub-transform time-domain buffer
            TC * X_prime;       // sub-transform freq-domain buffer
            FFT(plan) fft;      // sub-FFT of size nfft_prime
            FFT(plan) ifft;     // sub-IFFT of size nfft_prime
        } rader2;
    } data;
};

// allocate one-dimensional array 
//  _n      :   array size
void * FFT(_malloc)(unsigned int _n)
{
    return FFT_MALLOC(_n);
}

// allocate one-dimensional array allocated by fft_malloc
//  _x      :   pointer to array
void FFT(_free)(void * _x)
{
    FFT_FREE(_x);
}

// create FFT plan, regular complex one-dimensional transform
//  _nfft   :   FFT size
//  _x      :   input array [size: _nfft x 1]
//  _y      :   output array [size: _nfft x 1]
//  _dir    :   fft direction: {LIQUID_FFT_FORWARD, LIQUID_FFT_BACKWARD}
//  _flags  :   fft method
FFT(plan) FFT(_create_plan)(unsigned int _nfft,
                            TC *         _x,
                            TC *         _y,
                            int          _dir,
                            int          _flags)
{
    // determine best method for execution
    // TODO : check flags and allow user override
    liquid_fft_method method = liquid_fft_estimate_method(_nfft);

    // initialize fft based on method
    switch (method) {
    case LIQUID_FFT_METHOD_RADIX2:
        // use radix-2 decimation-in-time method
        return FFT(_create_plan_radix2)(_nfft, _x, _y, _dir, _flags);

    case LIQUID_FFT_METHOD_MIXED_RADIX:
        // use Cooley-Tukey mixed-radix algorithm
        return FFT(_create_plan_mixed_radix)(_nfft, _x, _y, _dir, _flags);

    case LIQUID_FFT_METHOD_RADER:
        // use Rader's algorithm for FFTs of prime length
        return FFT(_create_plan_rader)(_nfft, _x, _y, _dir, _flags);

    case LIQUID_FFT_METHOD_RADER2:
        // use Rader's algorithm for FFTs of prime length
        return FFT(_create_plan_rader2)(_nfft, _x, _y, _dir, _flags);

    case LIQUID_FFT_METHOD_DFT:
        // use slow DFT
        return FFT(_create_plan_dft)(_nfft, _x, _y, _dir, _flags);

    case LIQUID_FFT_METHOD_UNKNOWN:
    default:;
    }
    return liquid_error_config("fft_create_plan(), unknown/invalid fft method (%u)", method);
}

// destroy FFT plan
int FFT(_destroy_plan)(FFT(plan) _q)
{
    switch (_q->type) {
    // complex one-dimensional transforms
    case LIQUID_FFT_FORWARD:
    case LIQUID_FFT_BACKWARD:
        switch (_q->method) {
        case LIQUID_FFT_METHOD_DFT:         return FFT(_destroy_plan_dft)(_q);
        case LIQUID_FFT_METHOD_RADIX2:      return FFT(_destroy_plan_radix2)(_q);
        case LIQUID_FFT_METHOD_MIXED_RADIX: return FFT(_destroy_plan_mixed_radix)(_q);
        case LIQUID_FFT_METHOD_RADER:       return FFT(_destroy_plan_rader)(_q);
        case LIQUID_FFT_METHOD_RADER2:      return FFT(_destroy_plan_rader2)(_q);
        case LIQUID_FFT_METHOD_UNKNOWN:
        default:;
        }
        return liquid_error(LIQUID_EIMODE,"fft_destroy_plan(), unknown/invalid fft method (%u)", _q->method);
    // discrete cosine transforms
    case LIQUID_FFT_REDFT00:
    case LIQUID_FFT_REDFT10:
    case LIQUID_FFT_REDFT01:
    case LIQUID_FFT_REDFT11:
    // discrete sine transforms
    case LIQUID_FFT_RODFT00:
    case LIQUID_FFT_RODFT10:
    case LIQUID_FFT_RODFT01:
    case LIQUID_FFT_RODFT11:
        return FFT(_destroy_plan_r2r_1d)(_q);

    // modified discrete cosine transform
    case LIQUID_FFT_MDCT:
        return LIQUID_OK;
    case LIQUID_FFT_IMDCT:
        return LIQUID_OK;

    case LIQUID_FFT_UNKNOWN:
    default:;
    }
    return liquid_error(LIQUID_EIMODE,"fft_destroy_plan(), unknown/invalid fft type (%u)", _q->type);
}

// print FFT plan
int FFT(_print_plan)(FFT(plan) _q)
{
    switch (_q->type) {
    // complex one-dimensional transforms
    case LIQUID_FFT_FORWARD:
    case LIQUID_FFT_BACKWARD:
        printf("fft plan [%s], n=%u, ",
                _q->direction == LIQUID_FFT_FORWARD ? "forward" : "reverse",
                _q->nfft);
        switch (_q->method) {
        case LIQUID_FFT_METHOD_DFT:         printf("DFT\n");                break;
        case LIQUID_FFT_METHOD_RADIX2:      printf("Radix-2\n");            break;
        case LIQUID_FFT_METHOD_MIXED_RADIX: printf("Cooley-Tukey\n");       break;
        case LIQUID_FFT_METHOD_RADER:       printf("Rader (Type I)\n");     break;
        case LIQUID_FFT_METHOD_RADER2:      printf("Rader (Type II)\n");    break;
        case LIQUID_FFT_METHOD_UNKNOWN:
        default:
            return liquid_error(LIQUID_EIMODE,"fft_print_plan(), unknown/invalid fft method (%u)", _q->method);
        }
        // print recursive plan
        return FFT(_print_plan_recursive)(_q, 0);
    // discrete cosine transforms
    case LIQUID_FFT_REDFT00:
    case LIQUID_FFT_REDFT10:
    case LIQUID_FFT_REDFT01:
    case LIQUID_FFT_REDFT11:
    // discrete sine transforms
    case LIQUID_FFT_RODFT00:
    case LIQUID_FFT_RODFT10:
    case LIQUID_FFT_RODFT01:
    case LIQUID_FFT_RODFT11:
        return FFT(_print_plan)(_q);

    // modified discrete cosine transform
    case LIQUID_FFT_MDCT:   return LIQUID_OK;
    case LIQUID_FFT_IMDCT:  return LIQUID_OK;

    case LIQUID_FFT_UNKNOWN:
    default:;
    }
    return liquid_error(LIQUID_EIMODE,"fft_print_plan(), unknown/invalid fft type (%u)", _q->type);
}

// print FFT plan (recursively)
int FFT(_print_plan_recursive)(FFT(plan)    _q,
                               unsigned int _level)
{
    // print indentation based on recursion level
    unsigned int i;
    for (i=0; i<_level; i++)
        printf("  ");
    printf("%u, ", _q->nfft);

    switch (_q->method) {
    case LIQUID_FFT_METHOD_DFT:
        printf("DFT\n");
        break;

    case LIQUID_FFT_METHOD_RADIX2:
        printf("Radix-2\n");
        break;

    case LIQUID_FFT_METHOD_MIXED_RADIX:
        // two internal transforms
        printf("Cooley-Tukey mixed radix, Q=%u, P=%u\n",
                _q->data.mixedradix.Q,
                _q->data.mixedradix.P);
        FFT(_print_plan_recursive)(_q->data.mixedradix.fft_Q, _level+1);
        FFT(_print_plan_recursive)(_q->data.mixedradix.fft_P, _level+1);
        break;

    case LIQUID_FFT_METHOD_RADER:
        printf("Rader (Type-II), nfft-prime=%u\n", _q->nfft-1);
        FFT(_print_plan_recursive)(_q->data.rader.fft, _level+1);
        break;

    case LIQUID_FFT_METHOD_RADER2:
        printf("Rader (Type-II), nfft-prime=%u\n", _q->data.rader2.nfft_prime);
        FFT(_print_plan_recursive)(_q->data.rader2.fft, _level+1);
        break;

    case LIQUID_FFT_METHOD_UNKNOWN:     printf("(unknown)\n");      break;
    default:                            printf("(unknown)\n");      break;
    }
    return LIQUID_OK;
}

// execute fft
int FFT(_execute)(FFT(plan) _q)
{
    // invoke internal function pointer
    return _q->execute(_q);
}

// perform n-point FFT allocating plan internally
//  _nfft   :   fft size
//  _x      :   input array [size: _nfft x 1]
//  _y      :   output array [size: _nfft x 1]
//  _dir    :   fft direction: LIQUID_FFT_{FORWARD,BACKWARD}
//  _flags  :   fft flags
int FFT(_run)(unsigned int _nfft,
              TC *         _x,
              TC *         _y,
              int          _dir,
              int          _flags)
{
    // create plan
    FFT(plan) plan = FFT(_create_plan)(_nfft, _x, _y, _dir, _flags);

    // execute fft
    FFT(_execute)(plan);

    // destroy plan
    FFT(_destroy_plan)(plan);
    return LIQUID_OK;
}

// perform real n-point FFT allocating plan internally
//  _nfft   : fft size
//  _x      : input array [size: _nfft x 1]
//  _y      : output array [size: _nfft x 1]
//  _type   : fft type, e.g. LIQUID_FFT_REDFT10
//  _flags  : fft flags
int FFT(_r2r_1d_run)(unsigned int _nfft,
                     T *          _x,
                     T *          _y,
                     int          _type,
                     int          _flags)
{
    // create plan
    FFT(plan) plan = FFT(_create_plan_r2r_1d)(_nfft, _x, _y, _type, _flags);

    // execute fft
    FFT(_execute)(plan);

    // destroy plan
    FFT(_destroy_plan)(plan);
    return LIQUID_OK;
}

// perform _n-point FFT shift
int FFT(_shift)(TC *_x, unsigned int _n)
{
    unsigned int i, n2;
    if (_n%2)
        n2 = (_n-1)/2;
    else
        n2 = _n/2;

    TC tmp;
    for (i=0; i<n2; i++) {
        tmp = _x[i];
        _x[i] = _x[i+n2];
        _x[i+n2] = tmp;
    }
    return LIQUID_OK;
}

