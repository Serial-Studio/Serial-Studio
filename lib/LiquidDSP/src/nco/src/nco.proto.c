/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
// Numerically-controlled oscillator (nco) with internal phase-locked
// loop (pll) implementation
//

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define NCO_STATIC_LUT_WORDBITS     32
#define NCO_STATIC_LUT_NBITS        10
#define NCO_STATIC_LUT_SIZE         (1LLU << NCO_STATIC_LUT_NBITS)
#define NCO_STATIC_LUT_HSIZE        (NCO_STATIC_LUT_SIZE >> 1)
#define NCO_STATIC_LUT_QSIZE        (NCO_STATIC_LUT_SIZE >> 2)
#define NCO_STATIC_LUT_INDEX_SHIFTED_PI2(index) \
    (((index) + NCO_STATIC_LUT_QSIZE) & (NCO_STATIC_LUT_SIZE-1))
#define NCO_STATIC_LUT_THETA_SHIFTED_PI2(theta) \
    ((theta) + (1LLU << (NCO_STATIC_LUT_WORDBITS - 2)))
#define NCO_STATIC_LUT_THETA_ACCUM(theta) \
    ((uint32_t)(theta & ((1LLU << (NCO_STATIC_LUT_WORDBITS-NCO_STATIC_LUT_NBITS))-1)))

#define NCO_PLL_BANDWIDTH_DEFAULT   (0.1)
#define NCO_PLL_GAIN_DEFAULT        (1000)

#define LIQUID_DEBUG_NCO            (0)

struct vco_tab_e_s {
    T value, skew;
};
typedef struct vco_tab_e_s vco_tab_e;

struct NCO(_s) {
    liquid_ncotype  type;           // NCO type (e.g. LIQUID_VCO)

    // type == LIQUID_NCO, LIQUID_VCO_INTERP
    uint32_t        theta;          // 32-bit phase     [radians]
    uint32_t        d_theta;        // 32-bit frequency [radians/sample]

    // type == LIQUID_NCO
    T*              nco_sintab;     // sine look-up table

    // type == LIQUID_VCO_INTERP
    vco_tab_e*      vcoi_sintab;    // sine interpolated look-up table

    // type == LIQUID_VCO_DIRECT
    int             vcod_n;         // normalized multiplier coefficient
    unsigned int    vcod_m;         // normalized divider coefficient
    T*              vcod_sintab;    // sine direct look-up table
    T*              vcod_costab;    // cosine direct look-up table
    unsigned int    vcod_index;     // direct look-up index [0, m)

    // phase-locked loop
    T               alpha;          // frequency proportion
    T               beta;           // phase proportion
};

// reset internal phase-locked loop filter
int NCO(_pll_reset)(NCO() _q);

/* constrain phase (or frequency) and convert to fixed-point */
uint32_t NCO(_constrain)(T _theta);

/* utilities used with type LIQUID_VCO_DIRECT */
void NCO(_constrain_vcod)(int *_n, unsigned int *_m);

/* utilities used with types LIQUID_VCO_* */
T NCO(_fp_sin)(T x);
T NCO(_fp_cos)(T x);

// compute index for sine/cosine look-up table
unsigned int NCO(_static_index)(NCO() _q);

// create nco/vco object
NCO() NCO(_create)(liquid_ncotype _type)
{
    NCO() q = (NCO()) malloc(sizeof(struct NCO(_s)));
    q->type = _type;

    // initialize sine/cosine tables
    unsigned int i;
    switch (q->type) {
    case LIQUID_NCO: {
        q->vcoi_sintab = NULL;
        q->vcod_sintab = NULL;
        q->vcod_costab = NULL;
        q->nco_sintab  = (T*)malloc(NCO_STATIC_LUT_SIZE*sizeof(T));
        for (i=0; i<NCO_STATIC_LUT_SIZE; i++)
            q->nco_sintab[i] = SIN(TIL(2)*TFL(M_PI)*(T)(i)/(T)(NCO_STATIC_LUT_SIZE));
        break;
    }
    case LIQUID_VCO_INTERP: {
        q->nco_sintab  = NULL;
        q->vcod_sintab = NULL;
        q->vcod_costab = NULL;
        q->vcoi_sintab = (vco_tab_e*)malloc(NCO_STATIC_LUT_SIZE*sizeof(vco_tab_e));
        /* Calculate only [0, PI/2] range and mirror it to [PI, 3*PI/2] range
         * (in single forward iteration, i.e. "sine" direction)
         */
        uint32_t theta = 0;
        const int32_t d_theta = (uint32_t)(UINT32_MAX)/NCO_STATIC_LUT_SIZE;
        unsigned int i;
        for (i=0; i<NCO_STATIC_LUT_QSIZE; i++) {
            T value = NCO(_fp_sin)((uint32_t)(theta));
            T next_value = NCO(_fp_sin)((uint32_t)(theta + d_theta));
            T skew = (next_value - value) / (T)(d_theta);
            q->vcoi_sintab[i].value = value;
            q->vcoi_sintab[i].skew = skew;
            q->vcoi_sintab[i+NCO_STATIC_LUT_HSIZE].value = -value;
            q->vcoi_sintab[i+NCO_STATIC_LUT_HSIZE].skew = -skew;
            theta += d_theta;
        }
        q->vcoi_sintab[NCO_STATIC_LUT_QSIZE].value = TIL(1);
        q->vcoi_sintab[NCO_STATIC_LUT_QSIZE].skew =
            - q->vcoi_sintab[NCO_STATIC_LUT_QSIZE-1].skew;
        q->vcoi_sintab[NCO_STATIC_LUT_QSIZE+NCO_STATIC_LUT_HSIZE].value =
            - q->vcoi_sintab[NCO_STATIC_LUT_QSIZE].value;
        q->vcoi_sintab[NCO_STATIC_LUT_QSIZE+NCO_STATIC_LUT_HSIZE].skew =
              q->vcoi_sintab[NCO_STATIC_LUT_QSIZE-1].skew;
        /* Mirror [0, PI/2], [PI, 3*PI/2] ranges to [PI/2, PI], [3*PI/2, 2*PI]
         * (in single backward iteration, i.e. "cosine" direction)
         */
        for (i=1; i<NCO_STATIC_LUT_QSIZE; i++) {
            unsigned int i_pi2 = i + NCO_STATIC_LUT_QSIZE;
            T value = q->vcoi_sintab[NCO_STATIC_LUT_QSIZE - i].value;
            T skew = q->vcoi_sintab[NCO_STATIC_LUT_QSIZE - i - 1].skew;
            q->vcoi_sintab[i_pi2].value = value;
            q->vcoi_sintab[i_pi2].skew = -skew;
            q->vcoi_sintab[i_pi2 + NCO_STATIC_LUT_HSIZE].value = -value;
            q->vcoi_sintab[i_pi2 + NCO_STATIC_LUT_HSIZE].skew = skew;
        }
        break;
    }
    case LIQUID_VCO_DIRECT: {
        q->nco_sintab  = NULL;
        q->vcoi_sintab = NULL;
        q->vcod_sintab = NULL;
        q->vcod_costab = NULL;
        break;
    }
    default:
        return liquid_error_config("nco_%s_create(), unknown type : %u\n", EXTENSION, q->type);
    }

    // set default pll bandwidth
    NCO(_pll_set_bandwidth)(q, NCO_PLL_BANDWIDTH_DEFAULT);

    // reset object and return
    NCO(_reset)(q);
    return q;
}

// copy object
NCO() NCO(_copy)(NCO() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("nco_%s_copy(), object cannot be NULL", EXTENSION);

    // allocate new object, copy main component memory
    NCO() q_copy = (NCO()) malloc(sizeof(struct NCO(_s)));
    memmove(q_copy, q_orig, sizeof(struct NCO(_s)));

    // re-allocate arrays as needed
    switch (q_copy->type) {
    case LIQUID_NCO:
        q_copy->nco_sintab = (T *) liquid_malloc_copy(q_orig->nco_sintab, NCO_STATIC_LUT_SIZE, sizeof(T));
        break;
    case LIQUID_VCO_INTERP:
        q_copy->vcoi_sintab = (vco_tab_e *) liquid_malloc_copy(q_orig->vcoi_sintab, NCO_STATIC_LUT_SIZE, sizeof(vco_tab_e));
        break;
    case LIQUID_VCO_DIRECT:
        break;
    default:
        return liquid_error_config("nco_%s_copy(), unknown type: %u", q_copy->type, EXTENSION);
    }

    return q_copy;
}

// destroy nco object
int NCO(_destroy)(NCO() _q)
{
    if (_q==NULL)
        return liquid_error(LIQUID_EIOBJ,"nco_%s_destroy(), object is null", EXTENSION);

    switch (_q->type) {
    case LIQUID_NCO:
        free(_q->nco_sintab);
        break;
    case LIQUID_VCO_INTERP:
        free(_q->vcoi_sintab);
        break;
    case LIQUID_VCO_DIRECT:
        free(_q->vcod_sintab);
        free(_q->vcod_costab);
        break;
    default:
        break;
    }

    free(_q);
    return LIQUID_OK;
}

// Print nco object internals to stdout
int NCO(_print)(NCO() _q)
{
    printf("<liquid.nco_%s", EXTENSION);
    switch (_q->type) {
    case LIQUID_NCO: printf(", type=\"nco\""); break;
    case LIQUID_VCO_INTERP:
    case LIQUID_VCO_DIRECT:
        printf(", type=\"vco\""); break;
    default:;
    }
    printf(", phase=0x%.8x", _q->theta);
    printf(", freq=0x%.8x", _q->d_theta);
    printf(">\n");
#if 0
    // print entire table(s)
    unsigned int i;
    switch (_q->type) {
    case LIQUID_NCO:
        for (i=0; i<NCO_STATIC_LUT_SIZE; i++)
            printf("  sintab[%4u] = %16.12f\n", i, _q->nco_sintab[i]);
        break;
    case LIQUID_VCO_INTERP:
        for (i=0; i<NCO_STATIC_LUT_SIZE; i++)
            printf("  sintab[%4u]  .value = %16.12f  .skew = %22.15e\n",
                   i, _q->vcoi_sintab[i].value, _q->vcoi_sintab[i].skew);
        break;
    case LIQUID_VCO_DIRECT:
        for (i=0; i<_q->vcod_m; i++)
            printf("  [%10u]  sintab[] = %16.12f  costab[] = %16.12f\n",
                   i, _q->vcod_sintab[i], _q->vcod_costab[i]);
        break;
    default:
        break;
    }
#endif
    return LIQUID_OK;
}

// reset internal state of nco object
int NCO(_reset)(NCO() _q)
{
    switch (_q->type) {
    case LIQUID_NCO:
    case LIQUID_VCO_INTERP:
        // reset phase and frequency states
        _q->theta   = 0;
        _q->d_theta = 0;
        break;
    case LIQUID_VCO_DIRECT:
        NCO(_set_vcodirect_frequency)(_q, 0, 0);
        break;
    default:
        break;
    }

    // reset pll filter state
    return NCO(_pll_reset)(_q);
}

// set frequency of nco object
int NCO(_set_frequency)(NCO() _q,
                        T     _dtheta)
{
    if (_q->type == LIQUID_VCO_DIRECT) {
        return liquid_error(LIQUID_EICONFIG,"nco_set_frequency(), "
                       "cannot be used with object type == LIQUID_VCO_DIRECT");
    }
    _q->d_theta = NCO(_constrain)(_dtheta);
    return LIQUID_OK;
}

// adjust frequency of nco object
int NCO(_adjust_frequency)(NCO() _q,
                           T     _df)
{
    if (_q->type == LIQUID_VCO_DIRECT) {
        return liquid_error(LIQUID_EICONFIG,"nco_adjust_frequency(), "
                       "cannot be used with object type == LIQUID_VCO_DIRECT");
    }
    _q->d_theta += NCO(_constrain)(_df);
    return LIQUID_OK;
}

// set phase of nco object, constraining phase
int NCO(_set_phase)(NCO() _q,
                    T     _phi)
{
    if (_q->type == LIQUID_VCO_DIRECT) {
        return liquid_error(LIQUID_EICONFIG,"error: nco_set_phase(), "
                       "cannot be used with object type == LIQUID_VCO_DIRECT\n");
    }
    _q->theta = NCO(_constrain)(_phi);
    return LIQUID_OK;
}

// adjust phase of nco object, constraining phase
int NCO(_adjust_phase)(NCO() _q,
                       T     _dphi)
{
    if (_q->type == LIQUID_VCO_DIRECT) {
        return liquid_error(LIQUID_EICONFIG,"error: nco_adjust_phase(), "
                       "cannot be used with object type == LIQUID_VCO_DIRECT");
    }
    _q->theta += NCO(_constrain)(_dphi);
    return LIQUID_OK;
}

// increment internal phase of nco object
int NCO(_step)(NCO() _q)
{
    if ((_q->type == LIQUID_NCO) || (_q->type == LIQUID_VCO_INTERP)) {
        _q->theta += _q->d_theta;
    } else if (_q->type == LIQUID_VCO_DIRECT) {
        (_q->vcod_index)++;
        if (_q->vcod_index == _q->vcod_m)
            _q->vcod_index = 0;
    }
    return LIQUID_OK;
}

// get phase [radians]
T NCO(_get_phase)(NCO() _q)
{
    if (_q->type == LIQUID_VCO_DIRECT) {
        return liquid_error(LIQUID_EICONFIG,"error: nco_get_phase(), "
                       "cannot be used with object type == LIQUID_VCO_DIRECT");
    }
    return TIL(2)*TFL(M_PI)*(T)_q->theta / (T)(1LLU<<32);
}

// get frequency [radians/sample]
T NCO(_get_frequency)(NCO() _q)
{
    if (_q->type == LIQUID_VCO_DIRECT) {
        return liquid_error(LIQUID_EICONFIG,"nco_%s_get_frequency(), cannot be used with object type == LIQUID_VCO_DIRECT", EXTENSION);
    }
    T d_theta = TIL(2)*TFL(M_PI)*(T)_q->d_theta / (T)(1LLU<<32);
    return d_theta > TFL(M_PI) ? d_theta - TIL(2)*TFL(M_PI) : d_theta;
}

// get frequency of LIQUID_VCO_DIRECT object
// [fraction defined by normalized multiplier and divider coefficients]
void NCO(_get_vcodirect_frequency)(NCO()         _q,
                                   int*          _n,
                                   unsigned int* _m)
{
    if (_q->type != LIQUID_VCO_DIRECT) {
        liquid_error(LIQUID_EICONFIG,
            "nco_%s_get_vcodirect_frequency(), cannot be used with object type == LIQUID_VCO_DIRECT", EXTENSION);
        return;
    }
    *_n = _q->vcod_n;
    *_m = _q->vcod_m;
}

// set frequency of LIQUID_VCO_DIRECT object
// [fraction defined by multiplier and divider coefficients]
void NCO(_set_vcodirect_frequency)(NCO()        _q,
                                   int          _n,
                                   unsigned int _m)
{
    if (_q->type != LIQUID_VCO_DIRECT) {
        liquid_error(LIQUID_EICONFIG,
            "nco_%s_set_vcodirect_frequency(), cannot be used with object type == LIQUID_VCO_DIRECT", EXTENSION);
        return;
    }

    free(_q->vcod_sintab);
    free(_q->vcod_costab);

    _q->vcod_index = 0;

    NCO(_constrain_vcod)(&_n, &_m);

    if ((_n != 0) && (_m > 0)) {
        _q->vcod_n = _n;
        _q->vcod_m = _m;
        _q->vcod_sintab = (T*)malloc(_q->vcod_m*sizeof(T));
        _q->vcod_costab = (T*)malloc(_q->vcod_m*sizeof(T));
        const int32_t d_theta = (int32_t)((double)(UINT32_MAX)
                                          * (double)(_q->vcod_n)/(double)(_q->vcod_m));
        /* Calculate both sine and cosine values
         * over 'n' integral periods within 'm' count
         * constraining theta argument to [0, PI/2) range
         */
        static const uint32_t VALUE_PI_2 = (1ULL << 30);
        static const uint32_t VALUE_PI   = (1ULL << 31);
        uint32_t theta = 0;
        unsigned int i;
        for (i=0; i<_q->vcod_m; i++) {
            uint32_t theta_pi2 = theta % VALUE_PI_2;
            T sin_theta_pi2 = NCO(_fp_sin)(theta_pi2);
            T cos_theta_pi2 = NCO(_fp_cos)(theta_pi2);
            if ((theta % VALUE_PI) < VALUE_PI_2)
                _q->vcod_sintab[i] = sin_theta_pi2;
            else
                _q->vcod_sintab[i] = cos_theta_pi2;
            if (theta > VALUE_PI)
                _q->vcod_sintab[i] *= TIL(-1);
            if (((theta - VALUE_PI_2) % VALUE_PI) >= VALUE_PI_2)
                _q->vcod_costab[i] = cos_theta_pi2;
            else
                _q->vcod_costab[i] = sin_theta_pi2;
            if ((theta - VALUE_PI_2) <= VALUE_PI)
                _q->vcod_costab[i] *= TIL(-1);
            theta += d_theta;
        }
    } else {
        /* Use constant single-element sine/cosine table
         * in order to optimize indexed access
         */
        _q->vcod_n = 0;
        _q->vcod_m = 1;
        _q->vcod_sintab = (T*)malloc(sizeof(T));
        _q->vcod_costab = (T*)malloc(sizeof(T));
        _q->vcod_sintab[0] = TIL(0);
        _q->vcod_costab[0] = TIL(1);
    }
}

// compute sine, cosine internally
T NCO(_sin)(NCO() _q)
{
    T value = TIL(0);

    if ((_q->type == LIQUID_NCO) || (_q->type == LIQUID_VCO_INTERP)) {
        unsigned int index = NCO(_static_index)(_q);
        if (_q->type == LIQUID_NCO)
            value = _q->nco_sintab[index];
        else
            value = _q->vcoi_sintab[index].value +
                    NCO_STATIC_LUT_THETA_ACCUM(_q->theta) * _q->vcoi_sintab[index].skew;
    } else if (_q->type == LIQUID_VCO_DIRECT) {
        value = _q->vcod_sintab[_q->vcod_index];
    }

    return value;
}

T NCO(_cos)(NCO() _q)
{
    T value = TIL(1);

    if ((_q->type == LIQUID_NCO) || (_q->type == LIQUID_VCO_INTERP)) {
        unsigned int index = NCO(_static_index)(_q);
        /* add pi/2 phase shift */
        index = NCO_STATIC_LUT_INDEX_SHIFTED_PI2(index);
        if (_q->type == LIQUID_NCO) {
            value = _q->nco_sintab[index];
        } else {
            uint32_t theta = NCO_STATIC_LUT_THETA_SHIFTED_PI2(_q->theta);
            value = _q->vcoi_sintab[index].value +
                    NCO_STATIC_LUT_THETA_ACCUM(theta) * _q->vcoi_sintab[index].skew;
        }
    } else if (_q->type == LIQUID_VCO_DIRECT) {
        value = _q->vcod_costab[_q->vcod_index];
    }

    return value;
}

// compute sin, cos of internal phase
int NCO(_sincos)(NCO() _q,
                 T *   _s,
                 T *   _c)
{
    if ((_q->type == LIQUID_NCO) || (_q->type == LIQUID_VCO_INTERP)) {
        unsigned int index = NCO(_static_index)(_q);
        unsigned int index_pi2 = NCO_STATIC_LUT_INDEX_SHIFTED_PI2(index);
        if (_q->type == LIQUID_NCO) {
            *_s = _q->nco_sintab[index];
            *_c = _q->nco_sintab[index_pi2];
        } else {
            uint32_t theta_pi2 = NCO_STATIC_LUT_THETA_SHIFTED_PI2(_q->theta);
            *_s = _q->vcoi_sintab[index].value +
                  NCO_STATIC_LUT_THETA_ACCUM(_q->theta) * _q->vcoi_sintab[index].skew;
            *_c = _q->vcoi_sintab[index_pi2].value +
                  NCO_STATIC_LUT_THETA_ACCUM(theta_pi2) * _q->vcoi_sintab[index_pi2].skew;
        }
    } else if (_q->type == LIQUID_VCO_DIRECT) {
        *_s = _q->vcod_sintab[_q->vcod_index];
        *_c = _q->vcod_costab[_q->vcod_index];
    } else {
        *_s = TIL(0);
        *_c = TIL(1);
    }
    return LIQUID_OK;
}

// compute complex exponential of internal phase
int NCO(_cexpf)(NCO() _q,
                TC *  _y)
{
    T vsin;
    T vcos;
    NCO(_sincos)(_q, &vsin, &vcos);
    *_y = vcos + _Complex_I*vsin;
    return LIQUID_OK;
}

// pll methods

// reset pll state, retaining base frequency
int NCO(_pll_reset)(NCO() _q)
{
    return LIQUID_OK;
}

// set pll bandwidth
int NCO(_pll_set_bandwidth)(NCO() _q,
                            T     _bw)
{
    // validate input
    if (_bw < TIL(0))
        return liquid_error(LIQUID_EIRANGE,"nco_%s_pll_set_bandwidth(), bandwidth must be positive", EXTENSION);

    _q->alpha = _bw;                // frequency proportion
    _q->beta  = SQRT(_q->alpha);    // phase proportion
    return LIQUID_OK;
}

// advance pll phase
//  _q      :   nco object
//  _dphi   :   phase error
int NCO(_pll_step)(NCO() _q,
                   T     _dphi)
{
    if (_q->type == LIQUID_VCO_DIRECT)
        return liquid_error(LIQUID_EIRANGE,"nco_%s_pll_step(), cannot be used with object type == LIQUID_VCO_DIRECT", EXTENSION);

    // increase frequency proportional to error
    NCO(_adjust_frequency)(_q, _dphi*_q->alpha);

    // TODO: ensure frequency doesn't run away

    // increase phase proportional to error
    NCO(_adjust_phase)(_q, _dphi*_q->beta);

    // constrain frequency
    //NCO(_constrain_frequency)(_q);
    return LIQUID_OK;
}

// mixing functions

// Rotate input vector up by NCO angle, y = x exp{+j theta}
//  _q      :   nco object
//  _x      :   input sample
//  _y      :   output sample
int NCO(_mix_up)(NCO() _q,
                 TC    _x,
                 TC *  _y)
{
    // compute complex phasor
    TC v;
    NCO(_cexpf)(_q, &v);

    // rotate input
    *_y = _x * v;
    return LIQUID_OK;
}

// Rotate input vector down by NCO angle, y = x exp{-j theta}
//  _q      :   nco object
//  _x      :   input sample
//  _y      :   output sample
int NCO(_mix_down)(NCO() _q,
                   TC    _x,
                   TC *  _y)
{
    // compute complex phasor
    TC v;
    NCO(_cexpf)(_q, &v);

    // rotate input (negative direction)
    *_y = _x * CONJ(v);
    return LIQUID_OK;
}


// Rotate input vector array up by NCO angle:
//      y(t) = x(t) exp{+j (f*t + theta)}
// TODO : implement NCO/VCO-specific versions
//  _q      :   nco object
//  _x      :   input array [size: _n x 1]
//  _y      :   output sample [size: _n x 1]
//  _n      :   number of input, output samples
int NCO(_mix_block_up)(NCO()        _q,
                       TC *         _x,
                       TC *         _y,
                       unsigned int _n)
{
    unsigned int i;
    // FIXME: this method should be more efficient but is causing occasional
    //        errors so instead favor slower but more reliable algorithm
    //        (anyway it must be rewritten to work with LIQUID_VCO_DIRECT type)
#if 0
    T theta =   _q->theta;
    T d_theta = _q->d_theta;
    for (i=0; i<_n; i++) {
        // multiply _x[i] by [cos(theta) + _Complex_I*sin(theta)]
        _y[i] = _x[i] * liquid_cexpjf(theta);

        theta += d_theta;
    }

    NCO(_set_phase)(_q, theta);
#else
    for (i=0; i<_n; i++) {
        // mix single sample up
        NCO(_mix_up)(_q, _x[i], &_y[i]);

        // step NCO phase
        NCO(_step)(_q);
    }
#endif
    return LIQUID_OK;
}

// Rotate input vector array down by NCO angle:
//      y(t) = x(t) exp{-j (f*t + theta)}
// TODO : implement NCO/VCO-specific versions
//  _q      :   nco object
//  _x      :   input array [size: _n x 1]
//  _y      :   output sample [size: _n x 1]
//  _n      :   number of input, output samples
int NCO(_mix_block_down)(NCO()        _q,
                         TC *         _x,
                         TC *         _y,
                         unsigned int _n)
{
    unsigned int i;
    // FIXME: this method should be more efficient but is causing occasional
    //        errors so instead favor slower but more reliable algorithm
    //        (anyway it must be rewritten to work with LIQUID_VCO_DIRECT type)
#if 0
    T theta =   _q->theta;
    T d_theta = _q->d_theta;
    for (i=0; i<_n; i++) {
        // multiply _x[i] by [cos(-theta) + _Complex_I*sin(-theta)]
        _y[i] = _x[i] * liquid_cexpjf(-theta);

        theta += d_theta;
    }

    NCO(_set_phase)(_q, theta);
#else
    for (i=0; i<_n; i++) {
        // mix single sample down
        NCO(_mix_down)(_q, _x[i], &_y[i]);

        // step NCO phase
        NCO(_step)(_q);
    }
#endif
    return LIQUID_OK;
}

//
// internal methods
//

uint32_t NCO(_constrain)(T _theta)
{
#if 0
    // NOTE: there seems to be an occasional issue with this causing tests to fail,
    //       namely randomly setting the frequency to be pi radians/sample. This
    //       results in nco_crcf_pll not locking
    // divide value by 2*pi and compute modulo
    T p = _theta * 0.159154943091895;   // 1/(2 pi) ~ 0.159154943091895

    // extract fractional part of p
    T fpart = p - ((long)p);    // fpart is in (-1,1)

    // ensure fpart is in [0,1)
    if (fpart < 0.) fpart += 1.;

    // map to range of precision needed
    uint32_t retVal = (uint32_t)(fpart * 0xffffffff);
    return retVal >= UINT_MAX ? UINT_MAX : retVal;
#else
    // NOTE: this is not efficient, but resolves the issue above
    while (_theta >= 2*M_PI) _theta -= 2*M_PI;
    while (_theta <       0) _theta += 2*M_PI;

    return (uint32_t) ((_theta/(2*M_PI)) * 0xffffffff);
#endif
}

void NCO(_constrain_vcod)(int *_n, unsigned int *_m)
{
    if ((*_m) == 0)
        return;

    /* fold 'n' into [-'m'/2, 'm'/2) range */
    *_n %= (int)(*_m);
    if ((T)(abs(*_n)) >= (T)(*_m)/TIL(2)) {
        int sign = ((*_n) > 0) ? -1 : 1;
        *_n = sign*((*_m) - (unsigned int)abs(*_n));
    }

    /* try optimize values via reducing them by common denominators */
    // with base 10
    while ((((*_n) % 10) == 0) && (((*_m) % 10) == 0)) {
        *_n /= 10;
        *_m /= 10;
    }
    // with base 2
    while ((((*_n) & 1) == 0) && (((*_m) & 1) == 0)) {
        *_n >>= 1;
        *_m >>= 1;
    }
}

T NCO(_fp_sin)(T x)
{
    return SIN(x * TFL(M_PI) / ((uint32_t)(INT32_MAX)+1));
}

T NCO(_fp_cos)(T x)
{
    return COS(x * TFL(M_PI) / ((uint32_t)(INT32_MAX)+1));
}

unsigned int NCO(_static_index)(NCO() _q)
{
    /* TODO: LIQUID_NCO and LIQUID_VCO_INTERP are expected to share same code.
     *       But "appropriate" rounding for LIQUID_VCO_INTERP type causes
     *       phase breaks at some wrap points.
     *       Not sure, so just keep it for type it originated from...
     */
    if (_q->type == LIQUID_NCO) {
        //round down
        //return (_q->theta >> (NCO_STATIC_LUT_WORDBITS-NCO_STATIC_LUT_NBITS))
        //        & (NCO_STATIC_LUT_SIZE - 1);
        // round appropriately
        return ((_q->theta + (1<<(NCO_STATIC_LUT_WORDBITS-NCO_STATIC_LUT_NBITS-1)))
                 >> (NCO_STATIC_LUT_WORDBITS-NCO_STATIC_LUT_NBITS))
                & (NCO_STATIC_LUT_SIZE-1);
    } else {
        return (_q->theta >> (NCO_STATIC_LUT_WORDBITS-NCO_STATIC_LUT_NBITS))
                & (NCO_STATIC_LUT_SIZE-1);
    }
}

