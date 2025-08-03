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

//
// fir (finite impulse response) filter design using Parks-McClellan
// algorithm
//
// Much of this program has been borrowed heavily from [McClellan:1973]
// and [Janovetz:1998] with the exception of the Lagrange polynomial
// interpolation formulas and the structured 'firdespm' object. Several
// improvements have been made in the search algorithm to help maintain
// stability and convergence.
//
// References:
//  [Parks:1972] T. W. Parks and J. H. McClellan, "Chebyshev
//      Approximation for Nonrecursive Digital Filters with Linear
//      Phase," IEEE Transactions on Circuit Theory, vol. CT-19,
//      no. 2, March 1972.
//  [McClellan:1973] J. H. McClellan, T. W. Parks, L. R. Rabiner, "A
//      Computer Program for Designing Optimum FIR Linear Phase
//      Digital Filters," IEEE Transactions on Audio and
//      Electroacoustics, vol. AU-21, No. 6, December 1973.
//  [Rabiner:1975] L. R. Rabiner, J. H. McClellan, T. W. Parks, "FIR
//      Digital filter Design Techniques Using Weighted Chebyshev
//      Approximations," Proceedings of the IEEE, March 1975.
//  [Parks:1987] T. W. Parks and C. S. Burrus, "Digital Filter
//      Design," Upper Saddle River, NJ, John Wiley & Sons, Inc., 1987
//      (Section 3.3.3)
//  [Janovetz:1998] J. Janovetz, online: http://www.janovetz.com/jake/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid.internal.h"

#define LIQUID_FIRDESPM_DEBUG       0
#define LIQUID_FIRDESPM_DEBUG_PRINT 0

#define LIQUID_FIRDESPM_DEBUG_FILENAME "firdespm_internal_debug.m"
#if LIQUID_FIRDESPM_DEBUG
int firdespm_output_debug_file(firdespm _q);
#endif

#if 0
// initialize internal memory and arrays
int firdespm_init_memory(firdespm     _q,
                         unsigned int _h_len,
                         unsigned int _num_bands);
#endif

// initialize the frequency grid on the disjoint bounded set
int firdespm_init_grid(firdespm _q);

// compute interpolating polynomial
int firdespm_compute_interp(firdespm _q);

// compute error signal from actual response (interpolator
// output), desired response, and weights
int firdespm_compute_error(firdespm _q);

// search error curve for _r+1 extremal indices
int firdespm_iext_search(firdespm _q);

// evaluates result to determine if Remez exchange algorithm
// has converged
int firdespm_is_search_complete(firdespm _q);

// compute filter taps (coefficients) from result
int firdespm_compute_taps(firdespm _q, float * _h);

// structured data type
struct firdespm_s {
    // constants
    unsigned int h_len;         // filter length
    unsigned int s;             // odd/even filter length
    unsigned int n;             // filter semi-length
    unsigned int r;             // number of approximating functions
    unsigned int num_bands;     // number of discrete bands
    unsigned int grid_size;     // number of points on the grid
    unsigned int grid_density;  // density of the grid

    // band type (e.g. LIQUID_FIRDESPM_BANDPASS)
    liquid_firdespm_btype btype;

    // filter description parameters
    double * bands;             // bands array [size: 2*num_bands]
    double * des;               // desired response [size: num_bands]
    double * weights;           // weights [size: num_bands]
    liquid_firdespm_wtype * wtype;
    
    // dense grid elements
    double * F;                 // frequencies, [0, 0.5]
    double * D;                 // desired response
    double * W;                 // weight
    double * E;                 // error

    double * x;                 // Chebyshev points : cos(2*pi*f)
    double * alpha;             // Lagrange interpolating polynomial
    double * c;                 // interpolants
    double rho;                 // extremal weighted error

    unsigned int * iext;        // indices of extrema
    unsigned int num_exchanges; // number of changes in extrema

    firdespm_callback callback; // user-defined callback function
    void *            userdata; // user-defined structure for callback function
#if LIQUID_FIRDESPM_DEBUG
    FILE * fid;
#endif

};

// run filter design (full life cycle of object)
//  _h_len      :   length of filter (number of taps)
//  _num_bands  :   number of frequency bands
//  _bands      :   band edges, f in [0,0.5], [size: _num_bands x 2]
//  _des        :   desired response [size: _num_bands x 1]
//  _weights    :   response weighting [size: _num_bands x 1]
//  _wtype      :   weight types (e.g. LIQUID_FIRDESPM_FLATWEIGHT) [size: _num_bands x 1]
//  _btype      :   band type (e.g. LIQUID_FIRDESPM_BANDPASS)
//  _h          :   output coefficients array [size: _h_len x 1]
int firdespm_run(unsigned int            _h_len,
                 unsigned int            _num_bands,
                 float *                 _bands,
                 float *                 _des,
                 float *                 _weights,
                 liquid_firdespm_wtype * _wtype,
                 liquid_firdespm_btype   _btype,
                 float *                 _h)
{
    // create object
    firdespm q = firdespm_create(_h_len,_num_bands,_bands,_des,_weights,_wtype,_btype);

    // execute
    firdespm_execute(q,_h);

    // destroy
    firdespm_destroy(q);
    return LIQUID_OK;
}

// run filter design for basic low-pass filter
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _as     : stop-band attenuation [dB], _as > 0
//  _mu     : fractional sample offset, -0.5 < _mu < 0.5 [ignored]
//  _h      : output coefficient buffer, [size: _n x 1]
int firdespm_lowpass(unsigned int _n,
                     float        _fc,
                     float        _as,
                     float        _mu,
                     float *      _h)
{
    // validate inputs
    if (_mu < -0.5f || _mu > 0.5f)
        return liquid_error(LIQUID_EICONFIG,"firdespm_lowpass(), _mu (%12.4e) out of range [-0.5,0.5]", _mu);
    if (_fc < 0.0f || _fc > 0.5f)
        return liquid_error(LIQUID_EICONFIG,"firdespm_lowpass(), cutoff frequency (%12.4e) out of range (0, 0.5)", _fc);
    if (_n == 0)
        return liquid_error(LIQUID_EICONFIG,"firdespm_lowpass(), filter length must be greater than zero");

    // estimate transition band
    float ft = estimate_req_filter_df(_as, _n);

    // derived values
    float fp = _fc - 0.5*ft;     // pass-band cutoff frequency
    float fs = _fc + 0.5*ft;     // stop-band cutoff frequency
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;

    // derived values
    unsigned int num_bands = 2;
    float bands[4]   = {0.0f, fp, fs, 0.5f};
    float des[2]     = {1.0f, 0.0f};
    float weights[2] = {1.0f, 1.0f};
    liquid_firdespm_wtype wtype[2] = {LIQUID_FIRDESPM_FLATWEIGHT,
                                      LIQUID_FIRDESPM_EXPWEIGHT};

    // design filter
    return firdespm_run(_n,num_bands,bands,des,weights,wtype,btype,_h);
}

// create firdespm object
//  _h_len      :   length of filter (number of taps)
//  _num_bands  :   number of frequency bands
//  _bands      :   band edges, f in [0,0.5], [size: _num_bands x 2]
//  _des        :   desired response [size: _num_bands x 1]
//  _weights    :   response weighting [size: _num_bands x 1]
//  _wtype      :   weight types (e.g. LIQUID_FIRDESPM_FLATWEIGHT) [size: _num_bands x 1]
//  _btype      :   band type (e.g. LIQUID_FIRDESPM_BANDPASS)
firdespm firdespm_create(unsigned int            _h_len,
                         unsigned int            _num_bands,
                         float *                 _bands,
                         float *                 _des,
                         float *                 _weights,
                         liquid_firdespm_wtype * _wtype,
                         liquid_firdespm_btype   _btype)
{
    // basic validation
    if (_h_len==0)
        return liquid_error_config("firdespm_create(), filter length cannot be 0");
    if (_num_bands==0)
        return liquid_error_config("firdespm_create(), number of bands cannot be 0");

    // validate filter specification
    unsigned int i;
    int bands_valid = 1;
    int weights_valid = 1;
    // ensure bands are within [0,0.5]
    for (i=0; i<2*_num_bands; i++)
        bands_valid &= _bands[i] >= 0.0 && _bands[i] <= 0.5;
    // ensure bands are non-decreasing
    for (i=1; i<2*_num_bands; i++)
        bands_valid &= _bands[i] >= _bands[i-1];
    // ensure weights are greater than 0
    // TODO: ignore weights if pointer is NULL
    for (i=0; i<_num_bands; i++)
        weights_valid &= _weights[i] > 0;

    if (!bands_valid)
        return liquid_error_config("firdespm_create(), invalid bands");
    if (!weights_valid)
        return liquid_error_config("firdespm_create(), invalid weights (must be positive)");

    // create object
    firdespm q = (firdespm) malloc(sizeof(struct firdespm_s));

    // compute number of extremal frequencies
    q->h_len = _h_len;              // filter length
    q->s     = q->h_len % 2;        // odd/even length
    q->n     = (q->h_len - q->s)/2; // filter semi-length
    q->r     = q->n + q->s;         // number of approximating functions
    q->btype = _btype;              // set band type

    // allocate memory for extremal frequency set, interpolating polynomial
    q->iext  = (unsigned int*) malloc((q->r+1)*sizeof(unsigned int));
    q->x     = (double*) malloc((q->r+1)*sizeof(double));
    q->alpha = (double*) malloc((q->r+1)*sizeof(double));
    q->c     = (double*) malloc((q->r+1)*sizeof(double));

    // allocate memory for arrays
    q->num_bands = _num_bands;
    q->bands    = (double*) malloc(2*q->num_bands*sizeof(double));
    q->des      = (double*) malloc(  q->num_bands*sizeof(double));
    q->weights  = (double*) malloc(  q->num_bands*sizeof(double));

    // allocate memory for weighting types
    q->wtype = (liquid_firdespm_wtype*) malloc(q->num_bands*sizeof(liquid_firdespm_wtype));
    if (_wtype == NULL) {
        // set to default (LIQUID_FIRDESPM_FLATWEIGHT)
        for (i=0; i<q->num_bands; i++)
            q->wtype[i] = LIQUID_FIRDESPM_FLATWEIGHT;
    } else {
        // copy from input
        for (i=0; i<q->num_bands; i++)
            q->wtype[i] = _wtype[i];
    }

    // copy input arrays
    for (i=0; i<q->num_bands; i++) {
        q->bands[2*i+0] = _bands[2*i+0];
        q->bands[2*i+1] = _bands[2*i+1];
        q->des[i]       = _des[i];
        q->weights[i]   = _weights == NULL ? 1.0f : _weights[i];
    }

    // estimate grid size
    // TODO : adjust grid density based on expected value for rho
    q->grid_density = 20;
    q->grid_size = 0;
    double df = 0.5/(q->grid_density*q->r); // frequency step
    for (i=0; i<q->num_bands; i++) {
        double f0 = q->bands[2*i+0];         // lower band edge
        double f1 = q->bands[2*i+1];         // upper band edge
        q->grid_size += (unsigned int)( (f1-f0)/df + 1.0 );
    }

    // create the grid
    q->F = (double*) malloc(q->grid_size*sizeof(double));
    q->D = (double*) malloc(q->grid_size*sizeof(double));
    q->W = (double*) malloc(q->grid_size*sizeof(double));
    q->E = (double*) malloc(q->grid_size*sizeof(double));
    q->callback = NULL;
    q->userdata = NULL;
    firdespm_init_grid(q);
    // TODO : fix grid, weights according to filter type

    // return object
    return q;
}

// create firdespm object with user-defined callback
//  _h_len      :   length of filter (number of taps)
//  _num_bands  :   number of frequency bands
//  _bands      :   band edges, f in [0,0.5], [size: _num_bands x 2]
//  _btype      :   band type (e.g. LIQUID_FIRDESPM_BANDPASS)
//  _callback   :   user-defined callback for specifying desired response & weights
//  _userdata   :   user-defined data structure for callback function
firdespm firdespm_create_callback(unsigned int          _h_len,
                                  unsigned int          _num_bands,
                                  float *               _bands,
                                  liquid_firdespm_btype _btype,
                                  firdespm_callback     _callback,
                                  void *                _userdata)
{
    // basic validation
    if (_h_len==0)
        return liquid_error_config("firdespm_create_callback(), filter length cannot be 0");
    if (_num_bands==0)
        return liquid_error_config("firdespm_create_callback(), number of bands cannot be 0");


    // validate input
    unsigned int i;
    int bands_valid = 1;
    // ensure bands are within [0,0.5]
    for (i=0; i<2*_num_bands; i++)
        bands_valid &= _bands[i] >= 0.0 && _bands[i] <= 0.5;
    // ensure bands are non-decreasing
    for (i=1; i<2*_num_bands; i++)
        bands_valid &= _bands[i] >= _bands[i-1];

    if (!bands_valid)
        return liquid_error_config("firdespm_create(), invalid bands");

    // create object
    firdespm q = (firdespm) malloc(sizeof(struct firdespm_s));

    // compute number of extremal frequencies
    q->h_len = _h_len;              // filter length
    q->s     = q->h_len % 2;        // odd/even length
    q->n     = (q->h_len - q->s)/2; // filter semi-length
    q->r     = q->n + q->s;         // number of approximating functions
    q->btype = _btype;              // set band type
    q->callback = _callback;
    q->userdata = _userdata;

    // allocate memory for extremal frequency set, interpolating polynomial
    q->iext  = (unsigned int*) malloc((q->r+1)*sizeof(unsigned int));
    q->x     = (double*) malloc((q->r+1)*sizeof(double));
    q->alpha = (double*) malloc((q->r+1)*sizeof(double));
    q->c     = (double*) malloc((q->r+1)*sizeof(double));

    // allocate memory for arrays
    q->num_bands = _num_bands;
    q->bands    = (double*) malloc(2*q->num_bands*sizeof(double));
    q->des      = (double*) malloc(  q->num_bands*sizeof(double));
    q->weights  = (double*) malloc(  q->num_bands*sizeof(double));
    q->wtype = (liquid_firdespm_wtype*) malloc(q->num_bands*sizeof(liquid_firdespm_wtype));

    // copy input arrays
    for (i=0; i<q->num_bands; i++) {
        q->bands[2*i+0] = _bands[2*i+0];
        q->bands[2*i+1] = _bands[2*i+1];
        q->des[i]       = 0.0f;
        q->weights[i]   = 0.0f;
    }

    // estimate grid size
    // TODO : adjust grid density based on expected value for rho
    q->grid_density = 20;
    q->grid_size = 0;
    double df = 0.5/(q->grid_density*q->r); // frequency step
    for (i=0; i<q->num_bands; i++) {
        double f0 = q->bands[2*i+0];         // lower band edge
        double f1 = q->bands[2*i+1];         // upper band edge
        q->grid_size += (unsigned int)( (f1-f0)/df + 1.0 );
    }

    // create the grid
    q->F = (double*) malloc(q->grid_size*sizeof(double));
    q->D = (double*) malloc(q->grid_size*sizeof(double));
    q->W = (double*) malloc(q->grid_size*sizeof(double));
    q->E = (double*) malloc(q->grid_size*sizeof(double));
    firdespm_init_grid(q);
    // TODO : fix grid, weights according to filter type

    // return object
    return q;
}

// copy object
firdespm firdespm_copy(firdespm q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firdespm_copy(), object cannot be NULL");

    // create filter object and copy base parameters
    firdespm q_copy = (firdespm) malloc(sizeof(struct firdespm_s));
    memmove(q_copy, q_orig, sizeof(struct firdespm_s));

    // copy memory for filter description parameters
    q_copy->bands   = (double*)liquid_malloc_copy(q_orig->bands, 2*q_orig->num_bands, sizeof(double));
    q_copy->des     = (double*)liquid_malloc_copy(q_orig->des,     q_orig->num_bands, sizeof(double));
    q_copy->weights = (double*)liquid_malloc_copy(q_orig->weights, q_orig->num_bands, sizeof(double));
    q_copy->wtype   = (liquid_firdespm_wtype*) liquid_malloc_copy(q_orig->wtype, q_orig->num_bands, sizeof(liquid_firdespm_wtype));

    // copy the grid
    q_copy->F = (double*) liquid_malloc_copy(q_copy->F, q_orig->grid_size, sizeof(double));
    q_copy->D = (double*) liquid_malloc_copy(q_copy->D, q_orig->grid_size, sizeof(double));
    q_copy->W = (double*) liquid_malloc_copy(q_copy->W, q_orig->grid_size, sizeof(double));
    q_copy->E = (double*) liquid_malloc_copy(q_copy->E, q_orig->grid_size, sizeof(double));

    // copy memory for extremal frequency set, interpolating polynomial
    q_copy->iext  = (unsigned int*) liquid_malloc_copy(q_copy->iext, q_orig->r+1,sizeof(unsigned int));
    q_copy->x     = (double*)       liquid_malloc_copy(q_copy->x,    q_orig->r+1,sizeof(double));
    q_copy->alpha = (double*)       liquid_malloc_copy(q_copy->alpha,q_orig->r+1,sizeof(double));
    q_copy->c     = (double*)       liquid_malloc_copy(q_copy->c,    q_orig->r+1,sizeof(double));

    return q_copy;
}

// destroy firdespm object
int firdespm_destroy(firdespm _q)
{
#if LIQUID_FIRDESPM_DEBUG
    firdespm_output_debug_file(_q);
#endif

    // free memory for extremal frequency set, interpolating polynomial
    free(_q->iext);
    free(_q->x);
    free(_q->alpha);
    free(_q->c);

    // free dense grid elements
    free(_q->F);
    free(_q->D);
    free(_q->W);
    free(_q->E);

    // free band description elements
    free(_q->bands);
    free(_q->des);
    free(_q->weights);
    free(_q->wtype);

    // free object
    free(_q);
    return LIQUID_OK;
}

// print firdespm object internals
int firdespm_print(firdespm _q)
{
    unsigned int i;
    printf("<liquid.firdespm");

    printf(", lo=[");
    for (i=0; i<_q->num_bands; i++) printf("%g,", _q->bands[2*i+0]);
    printf("]");

    printf(", hi=[");
    for (i=0; i<_q->num_bands; i++) printf("%g,", _q->bands[2*i+1]);
    printf("]");

    printf(", des=[");
    for (i=0; i<_q->num_bands; i++) printf("%g,", _q->des[i]);
    printf("]");

    printf(", w=[");
    for (i=0; i<_q->num_bands; i++) printf("%g,", _q->weights[i]);
    printf("]");

    printf(">\n");
    return LIQUID_OK;
}

// execute filter design, storing result in _h
int firdespm_execute(firdespm _q, float * _h)
{
    unsigned int i;

    // initial guess of extremal frequencies evenly spaced on F
    // TODO : guarantee at least one extremal frequency lies in each band
    for (i=0; i<_q->r+1; i++) {
        _q->iext[i] = (i * (_q->grid_size-1)) / _q->r;
#if LIQUID_FIRDESPM_DEBUG_PRINT
        printf("iext_guess[%3u] = %u\n", i, _q->iext[i]);
#endif
    }

    // iterate over the Remez exchange algorithm
    unsigned int p;
    unsigned int max_iterations = 40;
    for (p=0; p<max_iterations; p++) {
        // compute interpolator
        firdespm_compute_interp(_q);

        // compute error
        firdespm_compute_error(_q);

        // search for new extremal frequencies
        firdespm_iext_search(_q);

        // check stopping criteria
        if (firdespm_is_search_complete(_q))
            break;
    }
#if LIQUID_FIRDESPM_DEBUG_PRINT
    printf("search complete in %u iterations\n", p);
#endif

    // compute filter taps
    return firdespm_compute_taps(_q, _h);
}


// 
// internal methods
//

#if 0
// initialize internal memory and arrays
int firdespm_init_memory(firdespm     _q,
                         unsigned int _h_len,
                         unsigned int _num_bands)
{
    return LIQUID_OK;
}
#endif

// initialize the frequency grid on the disjoint bounded set
int firdespm_init_grid(firdespm _q)
{
    unsigned int i,j;

    // frequency step size
    double df = 0.5/(_q->grid_density*_q->r);
#if LIQUID_FIRDESPM_DEBUG_PRINT
    printf("df : %12.8f\n", df);
#endif

#if 0
    firdespm_print(_q);

    // compute total bandwidth
    double b=0.0;
    for (i=0; i<_q->num_bands; i++)
        b += _q->bands[2*i+1] - _q->bands[2*i+0];

    printf("b : %12.8f\n", b);

    // adjust frequency step size if it is too large
    double g = b / (df * _q->r); // approximate oversampling rate (points/extrema)
    double gmin = 4.0;
    if (g < gmin)
        df *= g / gmin;
    printf("df : %12.8f\n", df);
#endif

    // number of grid points counter
    unsigned int n = 0;

    double f0, f1;
    double fw = 1.0f;   // weighting function
    for (i=0; i<_q->num_bands; i++) {
        // extract band edges
        f0 = _q->bands[2*i+0];
        f1 = _q->bands[2*i+1];

        // ensure first point is not zero for differentiator
        // and Hilbert transforms due to transformation (below)
        if (i==0 && _q->btype != LIQUID_FIRDESPM_BANDPASS)
            f0 = f0 < df ? df : f0;

        // compute the number of gridpoints in this band
        unsigned int num_points = (unsigned int)( (f1-f0)/df + 0.5 );

        // ensure at least one point per band
        if (num_points < 1) num_points = 1;

        //printf("band : [%12.8f %12.8f] %3u points\n",f0,f1,num_points);

        // add points to grid
        for (j=0; j<num_points; j++) {
            // add frequency points
            _q->F[n] = f0 + j*df;

            // compute desired response using function pointer if provided
            if (_q->callback != NULL) {
                _q->callback(_q->F[n], _q->userdata, &_q->D[n], &_q->W[n]);
            } else {
                _q->D[n] = _q->des[i];

                // compute weight, applying weighting function
                switch (_q->wtype[i]) {
                case LIQUID_FIRDESPM_FLATWEIGHT: fw = 1.0f;             break;
                case LIQUID_FIRDESPM_EXPWEIGHT:  fw = expf(2.0f*j*df);  break;
                case LIQUID_FIRDESPM_LINWEIGHT:  fw = 1.0f + 2.7f*j*df; break;
                default:
                    return liquid_error(LIQUID_EICONFIG,"firdespm_init_grid(), invalid weighting specifier: %d", _q->wtype[i]);
                }
                _q->W[n] = _q->weights[i] * fw;
            }

            n++;
        }
        // force endpoint to be upper edge of frequency band
        _q->F[n-1] = f1;   // according to Janovetz
    }
    _q->grid_size = n;

    // take care of special symmetry conditions here
    if (_q->btype == LIQUID_FIRDESPM_BANDPASS) {
        if (_q->s == 0) {
            // even length filter
            for (i=0; i<_q->grid_size; i++) {
                _q->D[i] /= cos(M_PI*_q->F[i]);
                _q->W[i] *= cos(M_PI*_q->F[i]);
            }
            // force weight at endpoint to be (nearly) zero
            //_q->W[_q->grid_size-1] = 6.12303177e-17f;
        }
    } else {
        // differentiator, Hilbert transform
        if (_q->s == 0) {
            // even length filter
            for (i=0; i<_q->grid_size; i++) {
                _q->D[i] /= sin(M_PI*_q->F[i]);
                _q->W[i] *= sin(M_PI*_q->F[i]);
            }
        } else {
            // odd length filter
            for (i=0; i<_q->grid_size; i++) {
                _q->D[i] /= sin(2*M_PI*_q->F[i]);
                _q->W[i] *= sin(2*M_PI*_q->F[i]);
            }
        }
    }
    return LIQUID_OK;
}

// compute interpolating polynomial
int firdespm_compute_interp(firdespm _q)
{
    unsigned int i;

    // compute Chebyshev points on F[iext[]] : cos(2*pi*f)
    for (i=0; i<_q->r+1; i++) {
        _q->x[i] = cos(2*M_PI*_q->F[_q->iext[i]]);
#if LIQUID_FIRDESPM_DEBUG_PRINT
        printf("x[%3u] = %12.8f\n", i, _q->x[i]);
#endif
    }
    //printf("\n");

    // compute Lagrange interpolating polynomial
    poly_fit_lagrange_barycentric(_q->x,_q->r+1,_q->alpha);
#if LIQUID_FIRDESPM_DEBUG_PRINT
    for (i=0; i<_q->r+1; i++)
        printf("a[%3u] = %12.8f\n", i, _q->alpha[i]);
#endif

    // compute rho
    double t0 = 0.0;    // numerator
    double t1 = 0.0;    // denominator
    for (i=0; i<_q->r+1; i++) {
        //printf("D[%3u] = %16.8e, W[%3u] = %16.8e\n", i, _q->D[_q->iext[i]], i, _q->W[_q->iext[i]]);
        t0 += _q->alpha[i] * _q->D[_q->iext[i]];
        t1 += _q->alpha[i] / _q->W[_q->iext[i]] * (i % 2 ? -1.0 : 1.0);
    }
    _q->rho = t0/t1;
#if LIQUID_FIRDESPM_DEBUG_PRINT
    printf("  rho   :   %12.4e\n", _q->rho);
    printf("\n");
#endif

    // compute polynomial values (interpolants)
    for (i=0; i<_q->r+1; i++) {
        _q->c[i] = _q->D[_q->iext[i]] - (i % 2 ? -1 : 1) * _q->rho / _q->W[_q->iext[i]];
#if LIQUID_FIRDESPM_DEBUG_PRINT
        printf("c[%3u] = %16.8e\n", i, _q->c[i]);
#endif
    }
    return LIQUID_OK;
}

int firdespm_compute_error(firdespm _q)
{
    unsigned int i;

    double xf;
    double H;
    for (i=0; i<_q->grid_size; i++) {
        // compute actual response
        xf = cos(2*M_PI*_q->F[i]);
        H = poly_val_lagrange_barycentric(_q->x,_q->c,_q->alpha,xf,_q->r+1);

        // compute error
        _q->E[i] = _q->W[i] * (_q->D[i] - H);
    }
    return LIQUID_OK;
}

// search error curve for r+1 extremal indices
// TODO : return number of values which have changed (stopping criteria)
int firdespm_iext_search(firdespm _q)
{
    unsigned int i;

    // found extremal frequency indices
    unsigned int nmax = 2*_q->r + 2*_q->num_bands; // max number of extremals
    unsigned int found_iext[nmax];
    unsigned int num_found=0;

#if 0
    // check for extremum at f=0
    if ( fabs(_q->E[0]) > fabs(_q->E[1]) )
        found_iext[num_found++] = 0;
#else
    // force f=0 into candidate set
    found_iext[num_found++] = 0;
#if LIQUID_FIRDESPM_DEBUG_PRINT
    printf("num_found : %4u [%4u / %4u]\n", num_found, 0, _q->grid_size);
#endif
#endif

    // search inside grid
    for (i=1; i<_q->grid_size-1; i++) {
        if ( ((_q->E[i]>=0.0) && (_q->E[i-1]<=_q->E[i]) && (_q->E[i+1]<=_q->E[i]) ) ||
             ((_q->E[i]< 0.0) && (_q->E[i-1]>=_q->E[i]) && (_q->E[i+1]>=_q->E[i]) ) )
        {
            //assert(num_found < nmax);
            if (num_found < nmax)
                found_iext[num_found++] = i;
#if LIQUID_FIRDESPM_DEBUG_PRINT
            printf("num_found : %4u [%4u / %4u]\n", num_found, i, _q->grid_size);
#endif
        }
    }

#if 0
    // check for extremum at f=0.5
    if ( fabs(_q->E[_q->grid_size-1]) > fabs(_q->E[_q->grid_size-2]) )
        found_iext[num_found++] = _q->grid_size-1;
#else
    // force f=0.5 into candidate set
    //assert(num_found < nmax);
    if (num_found < nmax)
        found_iext[num_found++] = _q->grid_size-1;
    //printf("num_found : %4u [%4u / %4u]\n", num_found, _q->grid_size-1, _q->grid_size);
#endif
    //printf("r+1 = %4u, num_found = %4u\n", _q->r+1, num_found);
    if (num_found < _q->r+1) {
        // too few extremal frequencies found.  Theoretically, this
        // should never happen as the Chebyshev alternation theorem
        // guarantees at least r+1 extrema, however due to finite
        // machine precision, interpolation can be imprecise

        _q->num_exchanges = 0;
        return 0;
        //return liquid_error(LIQUID_EINT,"firdespm_iext_search(), too few extrema found (expected %u, found %u); returning prematurely",
        //_q->r+1, num_found);
    }

    assert(num_found <= nmax);

    // search extrema and eliminate smallest
    unsigned int imin=0;    // index of found_iext where _E is a minimum extreme
    unsigned int sign=0;    // sign of error
    unsigned int num_extra = num_found - _q->r - 1; // number of extra extremal frequencies
    unsigned int alternating_sign;

#if LIQUID_FIRDESPM_DEBUG_PRINT
    for (i=0; i<_q->r+1; i++)
        printf("iext[%4u] = %4u : %16.8e\n", i, found_iext[i], _q->E[found_iext[i]]);
#endif

    while (num_extra) {
        // evaluate sign of first extrema
        sign = _q->E[found_iext[0]] > 0.0;

        //
        imin = 0;
        alternating_sign = 1;
        for (i=1; i<num_found; i++) {
            // update new minimum error extreme
            if ( fabs(_q->E[found_iext[i]]) < fabs(_q->E[found_iext[imin]]) )
                imin = i;
    
            if ( sign && _q->E[found_iext[i]] < 0.0 ) {
                sign = 0;
            } else if ( !sign && _q->E[found_iext[i]] >= 0.0 ) {
                sign = 1;
            } else {
                // found two extrema with non-alternating sign; delete
                // the smaller of the two
                if ( fabs(_q->E[found_iext[i]]) < fabs(_q->E[found_iext[i-1]]) )
                    imin = i;
                else
                    imin = i-1;
                alternating_sign = 0;
                break;
            }
        }
        //printf("  imin : %3u : %12.4e;\n", imin, _q->E[found_iext[imin]]);

        // 
        if ( alternating_sign && num_extra==1) {
            //imin = (fabs(_q->E[found_iext[0]]) > fabs(_q->E[found_iext[num_extra-1]])) ? 0 : num_extra-1;
            if (fabs(_q->E[found_iext[0]]) < fabs(_q->E[found_iext[num_found-1]]))
                imin = 0;
            else
                imin = num_found-1;
        }

        // Delete value in 'found_iext' at 'index imin'.  This
        // is equivalent to shifing all values left one position
        // starting at index imin+1
        //printf("deleting found_iext[%3u] = %3u\n", imin, found_iext[imin]);
#if 0
        memmove( &found_iext[imin],
                 &found_iext[imin+1],
                 (num_found-imin)*sizeof(unsigned int));
#else
        // equivalent code:
        for (i=imin; i<num_found; i++)
            found_iext[i] = found_iext[i+1];
#endif

        num_extra--;
        num_found--;

        //printf("num extra: %3u, num found: %3u\n", num_extra, num_found);
    }

    // count number of changes
    _q->num_exchanges=0;
    for (i=0; i<_q->r+1; i++)
        _q->num_exchanges += _q->iext[i] == found_iext[i] ? 0 : 1;

    // copy new values
    memmove(_q->iext, found_iext, (_q->r+1)*sizeof(unsigned int));

#if LIQUID_FIRDESPM_DEBUG_PRINT
    for (i=0; i<_q->r+1; i++)
        printf("iext_new[%4u] = %4u : %16.8e\n", i, found_iext[i], _q->E[found_iext[i]]);
#endif
    return LIQUID_OK;
}

// evaluates result to determine if Remez exchange algorithm
// has converged
int firdespm_is_search_complete(firdespm _q)
{
    // if no extremal frequencies have been exchanged, Remez
    // algorithm has converged
    if (_q->num_exchanges == 0)
        return 1;

    unsigned int i;
    double tol = 1e-3f;

    double e=0.0;
    double emin=0.0;
    double emax=0.0;
    for (i=0; i<_q->r+1; i++) {
        e = fabs(_q->E[_q->iext[i]]);
        if (i==0 || e < emin) emin = e;
        if (i==0 || e > emax) emax = e;
    }

#if LIQUID_FIRDESPM_DEBUG_PRINT
    printf("emin : %16.8e, emax : %16.8e, metric : %16.8e\n", emin, emax, (emax-emin)/emax);
#endif
    return (emax-emin) / emax < tol ? 1 : 0;
}

// compute filter taps (coefficients) from result
int firdespm_compute_taps(firdespm _q, float * _h)
{
    unsigned int i;

    // re-generate interpolator and compute coefficients
    // for best cosine approximation
    firdespm_compute_interp(_q);

    // evaluate Lagrange polynomial on evenly spaced points
    unsigned int p = _q->r - _q->s + 1;
    double G[p];
    for (i=0; i<p; i++) {
        double f = (double)(i) / (double)(_q->h_len);
        double xf = cos(2*M_PI*f);
        double cf = poly_val_lagrange_barycentric(_q->x,_q->c,_q->alpha,xf,_q->r+1);
        double g=1.0;

        if (_q->btype == LIQUID_FIRDESPM_BANDPASS && _q->s==1) {
            // odd filter length, even symmetry
            g = 1.0;
        } else if (_q->btype == LIQUID_FIRDESPM_BANDPASS && _q->s==0) {
            // even filter length, even symmetry
            g = cos(M_PI * i / _q->h_len);
        } else if (_q->btype != LIQUID_FIRDESPM_BANDPASS && _q->s==1) {
            // odd filter length, odd symmetry
        } else if (_q->btype != LIQUID_FIRDESPM_BANDPASS && _q->s==0) {
            // even filter length, odd symmetry
        }

        G[i] = cf * g;
        //printf("G(%3u) = %12.4e (cf = %12.8f, f=%12.8f, c = %12.8f);\n", i+1, G[i], cf, f, g);
    }

    // compute inverse DFT (slow method), performing
    // transformation here for different filter types
    // TODO : flesh out computation for other filter types
    unsigned int j;
    if (_q->btype == LIQUID_FIRDESPM_BANDPASS) {
        // odd filter length, even symmetry
        for (i=0; i<_q->h_len; i++) {
            double v = G[0];
            double f = ((double)i - (double)(p-1) + 0.5*(1-_q->s)) / (double)(_q->h_len);
            for (j=1; j<_q->r; j++)
                v += 2.0 * G[j] * cos(2*M_PI*f*j);
            _h[i] = v / (double)(_q->h_len);
        }
    } else if (_q->btype != LIQUID_FIRDESPM_BANDPASS && _q->s==1) {
        // odd filter length, odd symmetry
        return liquid_error(LIQUID_EINT,"firdespm_compute_taps(), filter configuration not yet supported");
    } else if (_q->btype != LIQUID_FIRDESPM_BANDPASS && _q->s==0) {
        // even filter length, odd symmetry
        return liquid_error(LIQUID_EINT,"firdespm_compute_taps(), filter configuration not yet supported");
    }
#if LIQUID_FIRDESPM_DEBUG_PRINT
    printf("\n");
    for (i=0; i<_q->h_len; i++)
        printf("h(%3u) = %12.8f;\n", i+1, _h[i]);
#endif
    return LIQUID_OK;
}

#if LIQUID_FIRDESPM_DEBUG
int firdespm_output_debug_file(firdespm _q)
{
    FILE * fid = fopen(LIQUID_FIRDESPM_DEBUG_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", LIQUID_FIRDESPM_DEBUG_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");

    unsigned int i;
    for (i=0; i<_q->grid_size; i++) {
        fprintf(fid,"F(%4u) = %16.8e;\n", i+1, _q->F[i]);
        fprintf(fid,"D(%4u) = %16.8e;\n", i+1, _q->D[i]);
        fprintf(fid,"W(%4u) = %16.8e;\n", i+1, _q->W[i]);
        fprintf(fid,"E(%4u) = %16.8e;\n", i+1, _q->E[i]);
    }

    for (i=0; i<_q->r+1; i++) {
        fprintf(fid,"iext(%4u) = %u;\n", i+1, _q->iext[i]+1);
    }

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(F,E,'-', F(iext),E(iext),'x');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('frequency');\n");
    fprintf(fid,"ylabel('error');\n");

    // evaluate poly
    unsigned int n=1024;
    for (i=0; i<n; i++) {
        double f = (double) i / (double)(2*(n-1));
        double x = cos(2*M_PI*f);
        double c = poly_val_lagrange_barycentric(_q->x,_q->c,_q->alpha,x,_q->r+1);

        fprintf(fid,"f(%4u) = %20.12e; H(%4u) = %20.12e;\n", i+1, f, i+1, c);
    }

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,H,'-', F(iext),D(iext)-E(iext),'x');\n");
    //fprintf(fid,"plot(f,20*log10(abs(H)),'-', F(iext),20*log10(abs(D(iext)-E(iext))),'x');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('frequency');\n");
    fprintf(fid,"ylabel('filter response');\n");

    fprintf(fid,"rho = %20.12e;\n", _q->rho);

    fclose(fid);
    printf("internal debugging results written to %s.\n", LIQUID_FIRDESPM_DEBUG_FILENAME);
    return LIQUID_OK;
}
#endif

#if 0
// design halfband filter using Parks-McClellan algorithm given the
// filter length and desired transition band
int liquid_firdespm_halfband_ft(unsigned int _m,
                                float        _ft,
                                float *      _h)
{
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    unsigned int h_len = 4*_m + 1;
    unsigned int num_bands = 2;
    float f0 = 0.25f - 0.5f*_ft;
    float f1 = 0.25f + 0.5f*_ft;
    float bands[4]   = {0.0f, f0, f1, 0.5f};
    float des[2]     = {1.0f, 0.0f};
    float weights[2] = {1.0f, 1.0f}; // best with {1, 1}
    liquid_firdespm_wtype wtype[2] = { // best with {flat, flat}
        LIQUID_FIRDESPM_FLATWEIGHT, LIQUID_FIRDESPM_FLATWEIGHT,};

    // design filter
    firdespm_run(h_len,num_bands,bands,des,weights,wtype,btype,_h);
#if 0
    // ensure values at odd indices are 0 (excepting center value)
    unsigned int i;
    for (i=0; i<_m; i++) {
        _h[        2*i] = 0;
        _h[h_len-2*i-1] = 0;
    }
#endif
    return LIQUID_OK;
}
#endif

