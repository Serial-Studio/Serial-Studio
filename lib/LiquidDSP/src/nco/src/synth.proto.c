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
// Numerically-controlled synthesizer (direct digital synthesis)
// with internal phase-locked loop (pll) implementation
//

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYNTH_PLL_BANDWIDTH_DEFAULT (0.1)
#define SYNTH_PLL_GAIN_DEFAULT (1000)

#define LIQUID_DEBUG_SYNTH (0)

struct SYNTH(_s) {
    T            theta;   // phase
    T            d_theta; // frequency
    TC *         tab;     // synth table
    unsigned int index;   // table index
    unsigned int length;  // table length
    T            sumsq;

    TC prev_half;
    TC current;
    TC next_half;

    // phase-locked loop
    T alpha;
    T beta;
};

SYNTH() SYNTH(_create)(const TC * _table, unsigned int _length)
{
    SYNTH() q = (SYNTH())malloc(sizeof(struct SYNTH(_s)));

    q->length = _length;
    q->tab    = (TC *)malloc(q->length * sizeof(TC));
    memcpy(q->tab, _table, q->length * sizeof(TC));

    // set default pll bandwidth
    SYNTH(_pll_set_bandwidth)(q, SYNTH_PLL_BANDWIDTH_DEFAULT);

    // reset object and return
    SYNTH(_reset)(q);
    // default frequency is to visit each sample once
    SYNTH(_set_frequency)(q, (2.f * M_PI) / _length);
    SYNTH(_compute_synth)(q);
    return q;
}

void SYNTH(_destroy)(SYNTH() _q)
{
    if (!_q) {
        return;
    }

    free(_q->tab);
    free(_q);
}

void SYNTH(_reset)(SYNTH() _q)
{
    _q->theta   = 0;
    _q->d_theta = 0;

    // reset sine table index
    _q->index = 0;

    // reset pll filter state
    SYNTH(_pll_reset)(_q);
}

void SYNTH(_set_frequency)(SYNTH() _q, T _f)
{
    _q->d_theta = _f;
}

void SYNTH(_adjust_frequency)(SYNTH() _q, T _df)
{
    _q->d_theta += _df;
}

void SYNTH(_set_phase)(SYNTH() _q, T _phi)
{
    _q->theta = _phi;
    SYNTH(_constrain_phase)(_q);
    SYNTH(_compute_synth)(_q);
}

void SYNTH(_adjust_phase)(SYNTH() _q, T _dphi)
{
    _q->theta += _dphi;
    SYNTH(_constrain_phase)(_q);
}

void SYNTH(_step)(SYNTH() _q)
{
    _q->theta += _q->d_theta;
    SYNTH(_constrain_phase)(_q);
    SYNTH(_compute_synth)(_q);
}

// get phase
T SYNTH(_get_phase)(SYNTH() _q)
{
    return _q->theta;
}

// ge frequency
T SYNTH(_get_frequency)(SYNTH() _q)
{
    return _q->d_theta;
}

unsigned int SYNTH(_get_length)(SYNTH() _q)
{
    return _q->length;
}

TC SYNTH(_get_current)(SYNTH() _q)
{
    return _q->current;
}

TC SYNTH(_get_half_previous)(SYNTH() _q)
{
    return _q->prev_half;
}

TC SYNTH(_get_half_next)(SYNTH() _q)
{
    return _q->next_half;
}

// pll methods

// reset pll state, retaining base frequency
void SYNTH(_pll_reset)(SYNTH() _q)
{
}

// set pll bandwidth
void SYNTH(_pll_set_bandwidth)(SYNTH() _q, T _bandwidth)
{
    // validate input
    if (_bandwidth < 0.0f) {
        liquid_error(LIQUID_EIRANGE,"synth_pll_set_bandwidth(), bandwidth must be positive");
        return;
    }

    _q->alpha = _bandwidth;       // frequency proportion
    _q->beta  = sqrtf(_q->alpha); // phase proportion
}

void SYNTH(_pll_step)(SYNTH() _q, T _dphi)
{
    // increase frequency proportional to error
    SYNTH(_adjust_frequency)(_q, _dphi * _q->alpha);

    // increase phase proportional to error
    SYNTH(_adjust_phase)(_q, _dphi * _q->beta);

    SYNTH(_compute_synth)(_q);

    // constrain frequency
    // SYNTH(_constrain_frequency)(_q);
}

// mixing functions

void SYNTH(_mix_up)(SYNTH() _q, TC _x, TC * _y)
{
    *_y = _x * _q->current;
}

void SYNTH(_mix_down)(SYNTH() _q, TC _x, TC * _y)
{
    *_y = _x * conjf(_q->current);
}

void SYNTH(_mix_block_up)(SYNTH() _q, TC * _x, TC * _y, unsigned int _n)
{
    unsigned int i;
    for (i = 0; i < _n; i++) {
        // mix single sample up
        SYNTH(_mix_up)(_q, _x[i], &_y[i]);

        // step SYNTH phase
        SYNTH(_step)(_q);
    }
}

void SYNTH(_mix_block_down)(SYNTH() _q, TC * _x, TC * _y, unsigned int _n)
{
    unsigned int i;
    for (i = 0; i < _n; i++) {
        // mix single sample down
        SYNTH(_mix_down)(_q, _x[i], &_y[i]);

        // step SYNTH phase
        SYNTH(_step)(_q);
    }
}

void SYNTH(_spread)(SYNTH() _q, TC _x, TC * _y)
{
    unsigned int i;
    for (i = 0; i < _q->length; i++) {
        SYNTH(_mix_up)(_q, _x, &_y[i]);

        SYNTH(_step)(_q);
    }
}

void SYNTH(_despread)(SYNTH() _q, TC * _x, TC * _y)
{
    TC despread = 0;
    T  sum      = 0;
    unsigned int i;
    for (i = 0; i < _q->length; i++) {
        TC temp;
        SYNTH(_mix_down)(_q, _x[i], &temp);

        despread += temp;
        sum += cabsf(_x[i]) * cabsf(_q->current);

        SYNTH(_step)(_q);
    }
    *_y = despread / sum;
}

void SYNTH(_despread_triple)(SYNTH() _q, TC * _x, TC * _early, TC * _punctual, TC * _late)
{
    TC despread_early    = 0;
    TC despread_punctual = 0;
    TC despread_late     = 0;

    T sum_early    = 0;
    T sum_punctual = 0;
    T sum_late     = 0;

    unsigned int i;
    for (i = 0; i < _q->length; i++) {
        despread_early += _x[i] * conjf(_q->prev_half);
        despread_punctual += _x[i] * conjf(_q->current);
        despread_late += _x[i] * conjf(_q->next_half);

        sum_early += cabsf(_x[i]) * cabsf(_q->prev_half);
        sum_punctual += cabsf(_x[i]) * cabsf(_q->current);
        sum_late += cabsf(_x[i]) * cabsf(_q->next_half);

        SYNTH(_step)(_q);
    }

    *_early    = despread_early / sum_early;
    *_punctual = despread_punctual / sum_punctual;
    *_late     = despread_late / sum_late;
}

//
// internal methods
//

// constrain frequency of SYNTH object to be in (-pi,pi)
void SYNTH(_constrain_frequency)(SYNTH() _q)
{
    if (_q->d_theta > M_PI)
        _q->d_theta -= 2 * M_PI;
    else if (_q->d_theta < -M_PI)
        _q->d_theta += 2 * M_PI;
}

// constrain phase of SYNTH object to be in (-pi,pi)
void SYNTH(_constrain_phase)(SYNTH() _q)
{
    if (_q->theta > M_PI)
        _q->theta -= 2 * M_PI;
    else if (_q->theta < -M_PI)
        _q->theta += 2 * M_PI;
}

void SYNTH(_compute_synth)(SYNTH() _q)
{
    // assume phase is constrained to be in (-pi,pi)

    // compute index
    float index = _q->theta * (float)_q->length / (2 * M_PI) + 2.f * (float)_q->length;
    _q->index   = ((unsigned int)(index + 0.5f)) % _q->length;
    assert(_q->index < _q->length);

    unsigned int prev_index = (_q->index + _q->length - 1) % _q->length;
    unsigned int next_index = (_q->index + 1) % _q->length;

    _q->current = _q->tab[_q->index];
    TC prev     = _q->tab[prev_index];
    TC next     = _q->tab[next_index];

    _q->prev_half = (_q->current + prev) / 2;
    _q->next_half = (_q->current + next) / 2;
}
