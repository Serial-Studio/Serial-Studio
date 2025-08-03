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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.internal.h"

// quasi-Newton search object
struct qs1dsearch_s {
    float vn, va, v0, vb, vp;   // values
    float un, ua, u0, ub, up;   // utilities
    int init;                   // values initialized?

    // External utility function.
    liquid_utility_1d utility;   //
    void *            context;   // user-defined data pointer passed to utility callback
    int               direction; // search direction (minimize/maximimze utility)
    unsigned int      num_steps; // number of steps evaluated
};

// create quasi-newton method search object
qs1dsearch qs1dsearch_create(liquid_utility_1d _utility,
                             void *            _context,
                             int               _direction)
{
    // validate input
    if (_utility == NULL)
        return liquid_error_config("qs1dsearch_create(), utility callback cannot be NULL");

    // allocate main object memory and initialize
    qs1dsearch q = (qs1dsearch) malloc( sizeof(struct qs1dsearch_s) );
    q->utility   = _utility;
    q->context   = _context;
    q->direction = _direction;

    // reset object and return
    qs1dsearch_reset(q);
    return q;
}

qs1dsearch qs1dsearch_copy(qs1dsearch _q)
{
    liquid_error(LIQUID_ENOIMP,"qs1dsearch_copy(), method not yet implemented");
    return NULL;
}

int qs1dsearch_destroy(qs1dsearch _q)
{
    if (_q == NULL)
        return liquid_error(LIQUID_EIOBJ,"qs1dsearch_destroy(), invalid null pointer passed");

    free(_q);
    return LIQUID_OK;
}

int qs1dsearch_print(qs1dsearch _q)
{
    printf("<liquid.qs1dsearch, v=[%12g,%12g,%12g], u=[%12g,%12g,%12g]>\n",
        _q->vn, _q->v0, _q->vp, _q->un, _q->u0, _q->up);
    return LIQUID_OK;
}

int qs1dsearch_reset(qs1dsearch _q)
{
    _q->init = 0;
    _q->num_steps = 0;
    _q->vn = 0.0f; _q->un = 0.0f;
    _q->va = 0.0f; _q->ua = 0.0f;
    _q->v0 = 0.0f; _q->u0 = 0.0f;
    _q->vb = 0.0f; _q->ub = 0.0f;
    _q->vp = 0.0f; _q->up = 0.0f;
    return LIQUID_OK;
}

// perform initial search
int qs1dsearch_init(qs1dsearch _q,
                    float      _v)
{
    // specify initial step size
    float step = 1e-16f;

    // try positive direction
    if ( qs1dsearch_init_direction(_q, _v, step)==LIQUID_OK )
        return LIQUID_OK;

    // try negative direction
    if ( qs1dsearch_init_direction(_q, _v, -step)==LIQUID_OK )
        return LIQUID_OK;

    // check edge case where _v is exactly the optimum
    _q->vn = _v - step; _q->un =_q->utility(_q->vn, _q->context);
    _q->v0 = _v;        _q->u0 =_q->utility(_q->v0, _q->context);
    _q->vp = _v + step; _q->up =_q->utility(_q->vp, _q->context);
    if ( (_q->direction == LIQUID_OPTIM_MINIMIZE && (_q->u0 < _q->un && _q->u0 < _q->up)) ||
         (_q->direction == LIQUID_OPTIM_MAXIMIZE && (_q->u0 > _q->un && _q->u0 > _q->up)) )
    {
        _q->init = 1;
        return LIQUID_OK;
    }

    // TODO: be more persistent?
    return LIQUID_EIVAL;// FIXME: return proper error here, e.g. LIQUID_ENOCONV
}

// perform initial search along a particular direction
int qs1dsearch_init_direction(qs1dsearch _q,
                              float      _v_init,
                              float      _step)
{
    //printf("# qs1dsearch_init_direction(_q, v_init:%12.8f, step:%12.4e);\n", _v_init, _step);
    // start over and search in proper direction
    float vn = _v_init;              float un = 0.0f;
    float v0 = _v_init;              float u0 = _q->utility(v0, _q->context);
    float vp = _v_init + _step*0.5f; float up = _q->utility(vp, _q->context);
    unsigned int i;
    for (i=0; i<180; i++) {
        // move values down
        vn = v0; v0 = vp;
        un = u0; u0 = up;
        //
        vp = v0 + _step;
        up = _q->utility(vp, _q->context);
        //printf("%3u %12.4e:{%12.8f,%12.8f,%12.8f},{%12.8f,%12.8f,%12.8f}\n",i, _step, vn, v0, vp, un, u0, up);
        //printf("%3u %12.4e:{%12.4e,%12.4e,%12.4e},{%12.4e,%12.4e,%12.4e}\n",i, _step, vn, v0, vp, un, u0, up);
        if ( (_q->direction == LIQUID_OPTIM_MINIMIZE && (u0 < un && u0 < up)) ||
             (_q->direction == LIQUID_OPTIM_MAXIMIZE && (u0 > un && u0 > up)) )
        {
            // skipped over optimum; set internal bounds and return
            int swap = (_step < 0);
            _q->vn = swap ? vp : vn;
            _q->v0 = v0;
            _q->vp = swap ? vn : vp;

            _q->un = swap ? up : un;
            _q->u0 = u0;
            _q->up = swap ? un : up;
            _q->init = 1;
            return LIQUID_OK;
        } else
        if ( (_q->direction == LIQUID_OPTIM_MINIMIZE && (u0 >= un && up > u0)) ||
             (_q->direction == LIQUID_OPTIM_MAXIMIZE && (u0 <= un && up < u0)) )
        {
            // clearly moving in wrong direction: exit early
            break;
        }
        _step *= 1.5; // increase step size
    }

    // did not converge; return non-zero value
    return LIQUID_EIVAL;// FIXME: return proper error here, e.g. LIQUID_ENOCONV
}

int qs1dsearch_init_bounds(qs1dsearch _q,
                           float      _vn,
                           float      _vp)
{
    // set bounds appropriately
    _q->vn = _vn < _vp ? _vn : _vp;
    _q->vp = _vn < _vp ? _vp : _vn;
    _q->v0 = 0.5f*(_vn + _vp);

    // evaluate utility
    _q->un = _q->utility(_q->vn, _q->context);
    _q->u0 = _q->utility(_q->v0, _q->context);
    _q->up = _q->utility(_q->vp, _q->context);

    _q->init = 1;

    // TODO: ensure v0 is optimum here
    return LIQUID_OK;
}

int qs1dsearch_step(qs1dsearch _q)
{
    if (!_q->init)
        return liquid_error(LIQUID_ENOINIT,"qs1dsearch_step(), object has not be properly initialized");

    // TODO: allow option for geometric mean?
    // compute new candidate points
    _q->va = 0.5f*(_q->vn + _q->v0);
    _q->vb = 0.5f*(_q->v0 + _q->vp);

    // evaluate utility
    _q->ua = _q->utility(_q->va, _q->context);
    _q->ub = _q->utility(_q->vb, _q->context);

#if 0
    printf(" %3u [%7.3f,%7.3f,%7.3f,%7.3f,%7.3f] : {%7.3f,%7.3f,%7.3f,%7.3f,%7.3f}\n",
        _q->num_steps,
        _q->vn, _q->va, _q->v0, _q->vb, _q->vp,
        _q->un, _q->ua, _q->u0, _q->ub, _q->up);
#endif

    // [ (vn)  va  (v0)  vb  (vp) ]
    // optimum should be va, v0, or vb
    if ((_q->direction == LIQUID_OPTIM_MINIMIZE && _q->ua < _q->u0 && _q->ua < _q->ub) ||
        (_q->direction == LIQUID_OPTIM_MAXIMIZE && _q->ua > _q->u0 && _q->ua > _q->ub))
    {
        // va is optimum
        _q->vp = _q->v0; _q->up = _q->u0;
        _q->v0 = _q->va; _q->u0 = _q->ua;
    } else
    if ((_q->direction == LIQUID_OPTIM_MINIMIZE && _q->u0 < _q->ua && _q->u0 < _q->ub) ||
        (_q->direction == LIQUID_OPTIM_MAXIMIZE && _q->u0 > _q->ua && _q->u0 > _q->ub))
    {
        // v0 is optimum
        _q->vn = _q->va; _q->un = _q->ua;
        _q->vp = _q->vb; _q->up = _q->ub;
    } else {
        // vb is optimum
        _q->vn = _q->v0; _q->un = _q->u0;
        _q->v0 = _q->vb; _q->u0 = _q->ub;
    }

    _q->num_steps++;
    return LIQUID_OK;
}

int qs1dsearch_execute(qs1dsearch _q)
{
    return LIQUID_OK;
}

unsigned int qs1dsearch_get_num_steps(qs1dsearch _q)
{
    return _q->num_steps;
}

float qs1dsearch_get_opt_v(qs1dsearch _q)
{
    return _q->v0;
}

float qs1dsearch_get_opt_u(qs1dsearch _q)
{
    return _q->u0;
}

