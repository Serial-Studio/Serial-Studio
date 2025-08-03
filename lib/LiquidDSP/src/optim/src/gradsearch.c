/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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

#define DEBUG_GRADSEARCH 0

// gradient search algorithm (steepest descent) object
struct gradsearch_s {
    float * v;                  // vector to optimize (externally allocated)
    unsigned int num_parameters;// ...
    float u;                    // utility at current position

    // properties
    float delta;                // gradient approximation step size
    float alpha;                // line search step size

    float * p;                  // gradient estimate
    float pnorm;                // L2-norm of gradient estimate

    utility_function utility;   // utility function pointer
    void * userdata;            // object to optimize (user data)
    int direction;              // search direction (minimize/maximimze utility)
};

// create a gradient search object
//   _userdata          :   user data object pointer
//   _v                 :   array of parameters to optimize
//   _num_parameters    :   array length (number of parameters to optimize)
//   _u                 :   utility function pointer
//   _direction         :   search direction (e.g. LIQUID_OPTIM_MAXIMIZE)
gradsearch gradsearch_create(void *           _userdata,
                             float *          _v,
                             unsigned int     _num_parameters,
                             utility_function _utility,
                             int              _direction)
{
    gradsearch q = (gradsearch) malloc( sizeof(struct gradsearch_s) );

    // set user-defined properties
    q->userdata       = _userdata;
    q->v              = _v;
    q->num_parameters = _num_parameters;
    q->utility        = _utility;
    q->direction      = _direction;

    // set internal properties
    // TODO : make these user-configurable properties
    q->delta = 1e-6f;       // gradient approximation step size
    q->alpha = q->delta;    // line search step size

    // allocate array for gradient estimate
    q->p = (float*) malloc(q->num_parameters*sizeof(float));
    q->pnorm = 0.0f;
    q->u = 0.0f;

    return q;
}

void gradsearch_destroy(gradsearch _q)
{
    // free gradient estimate array
    free(_q->p);

    // free main object memory
    free(_q);
}

// print status
void gradsearch_print(gradsearch _q)
{
#if 0
    //printf("gradient search:\n");
    printf("u=%12.4e ",   _q->u);       // utility
    // enable more verbose output
    printf("|p|=%7.1e ",  _q->pnorm);   // norm(p)
    printf("del=%7.1e ",  _q->delta);   // delta
    printf("step=%7.1e ", _q->alpha);   // alpha (step size)

    unsigned int i;
    printf("{");
    for (i=0; i<_q->num_parameters; i++)
        printf("%8.4f", _q->v[i]);
    printf("}\n");
#else
    printf("<liquid.gradsearch");
    printf(", n=%u", _q->num_parameters);
    printf(", dir=\"%s\"", _q->direction == LIQUID_OPTIM_MAXIMIZE ? "max" : "min");
    printf(", pnorm=%g", _q->pnorm);   // norm(p)
    printf(", delta=%g", _q->delta);   // delta
    printf(", u=%g", _q->u);
    printf(">\n");
#endif
    // return LIQUID_OK;
}

float gradsearch_step(gradsearch _q)
{
    unsigned int i;

    // ensure norm(p) > 0, otherwise increase delta
    unsigned int n=20;
    for (i=0; i<n; i++) {
        // compute gradient
        gradsearch_gradient(_q->utility, _q->userdata, _q->v, _q->num_parameters, _q->delta, _q->p);

        // normalize gradient vector
        _q->pnorm = gradsearch_norm(_q->p, _q->num_parameters);

        if (_q->pnorm > 0.0f) {
            // try to keep delta about 1e-4 * pnorm
            if (1e-4f*_q->pnorm < _q->delta)
                _q->delta *= 0.90f;
            else if ( 1e-5f*_q->pnorm > _q->delta)
                _q->delta *= 1.10f;

            break;
        } else {
            // step size is too small to approximate gradient
            _q->delta *= 10.0f;
        }
    }
    
    if (i == n) {
        liquid_error(LIQUID_ENOCONV,"gradsearch_step(), function ill-conditioned");
        return _q->utility(_q->userdata, _q->v, _q->num_parameters);
    }

    // run line search
    _q->alpha = gradsearch_linesearch(_q->utility,
                                      _q->userdata,
                                      _q->direction,
                                      _q->num_parameters,
                                      _q->v,
                                      _q->p,
                                      _q->delta);

    // step in the negative direction of the gradient
    float dir = _q->direction == LIQUID_OPTIM_MINIMIZE ? 1.0f : -1.0f;
    for (i=0; i<_q->num_parameters; i++)
        _q->v[i] = _q->v[i] - dir*_q->alpha*_q->p[i];

    // evaluate utility at current position
    _q->u = _q->utility(_q->userdata, _q->v, _q->num_parameters);

    // return utility
    return _q->u;
}

// batch execution of gradient search : run many steps and stop
// when criteria are met
float gradsearch_execute(gradsearch   _q,
                         unsigned int _max_iterations,
                         float        _target_utility)
{
    int continue_running = 1;
    unsigned int num_iterations = 0;

    float u = 0.0f;
    while (continue_running) {
        // increment number of iterations
        num_iterations++;

        // step gradient search algorithm
        u = gradsearch_step(_q);

        // check exit criteria (any one of the following)
        //  * maximum number of iterations met
        //  * maximize utility and target exceeded
        //  * minimize utility and target exceeded
        if ( (num_iterations >= _max_iterations                            ) ||
             (_q->direction == LIQUID_OPTIM_MINIMIZE && u < _target_utility) ||
             (_q->direction == LIQUID_OPTIM_MAXIMIZE && u > _target_utility) )
        {
            continue_running = 0;
        }

    }

    // return final utility
    return u;
}


// 
// internal (generic functions)
//

// compute the gradient of a function at a particular point
//  _utility    :   user-defined function
//  _userdata   :   user-defined data object
//  _x          :   operating point, [size: _n x 1]
//  _n          :   dimensionality of search
//  _delta      :   step value for which to compute gradient
//  _gradient   :   resulting gradient
void gradsearch_gradient(utility_function _utility,
                         void  *          _userdata,
                         float *          _x,
                         unsigned int     _n,
                         float            _delta,
                         float *          _gradient)
{
    // operating point for evaluation
    float x_prime[_n];
    float u_prime;

    // evaluate function at current operating point
    float u0 = _utility(_userdata, _x, _n);
        
    unsigned int i;
    for (i=0; i<_n; i++) {
        // copy operating point
        memmove(x_prime, _x, _n*sizeof(float));
        
        // increment test vector by delta along dimension 'i'
        x_prime[i] += _delta;

        // evaluate new utility
        u_prime = _utility(_userdata, x_prime, _n);

        // compute gradient estimate
        _gradient[i] = (u_prime - u0) / _delta;
    }
}

// execute line search; loosely solve:
//
//    min|max phi(alpha) := f(_x - alpha*_p)
//
// and return best guess at alpha that achieves this
//
//  _utility    :   user-defined function
//  _userdata   :   user-defined data object
//  _direction  :   search direction (e.g. LIQUID_OPTIM_MINIMIZE)
//  _n          :   dimensionality of search
//  _x          :   operating point, [size: _n x 1]
//  _p          :   normalized gradient, [size: _n x 1]
//  _alpha      :   initial step size
float gradsearch_linesearch(utility_function _utility,
                            void  *          _userdata,
                            int              _direction,
                            unsigned int     _n,
                            float *          _x,
                            float *          _p,
                            float            _alpha)
{
#if DEBUG_GRADSEARCH
    unsigned int i;
    // print vector operating point
    printf("  linesearch v: {");
    for (i=0; i<_n; i++)
        printf("%8.4f", _x[i]);
    printf("}\n");
    // print gradient
    printf("  linesearch g: {");
    for (i=0; i<_n; i++)
        printf("%8.4f", _p[i]);
    printf("}\n");
#endif
    // evaluate utility at base point
    float u0 = _utility(_userdata, _x, _n);

    // initialize step size estimate
    float alpha = _alpha;

    // step direction
    float dir = _direction == LIQUID_OPTIM_MINIMIZE ? 1.0f : -1.0f;

    // test vector, TODO : dynamic allocation?
    float x_prime[_n];

    // run line search
    int continue_running = 1;
    unsigned int num_iterations = 0;
    unsigned int max_iterations = 250;
    float gamma = 2.00;
    while (continue_running) {
        // increment iteration counter
        num_iterations++;

        // update evaluation point
        unsigned int i;
        for (i=0; i<_n; i++)
            x_prime[i] = _x[i] - dir*alpha*_p[i];
        
        // compute utility for line search step
        float uls = _utility(_userdata, x_prime, _n);
#if DEBUG_GRADSEARCH
        printf("  linesearch %6u : alpha=%12.6f, u0=%12.8f, uls=%12.8f\n", num_iterations, alpha, u0, uls);
#endif

        // check exit criteria
        if ( (_direction == LIQUID_OPTIM_MINIMIZE && uls > u0) ||
             (_direction == LIQUID_OPTIM_MAXIMIZE && uls < u0) )
        {
            // compared this utility to previous; went too far.
            // backtrack step size and stop line search
            alpha *= 1.0f/gamma;
            continue_running = 0;
        } else if ( num_iterations >= max_iterations ) {
            // maximum number of iterations met: stop line search
            continue_running = 0;
        } else {
            // save new best estimate, increase step size, and continue
            u0 = uls;
            alpha *= gamma;
        }
    }

    return alpha;
}

// normalize vector, returning its l2-norm
float gradsearch_norm(float *      _v,
                      unsigned int _n)
{
    // compute l2-norm
    float vnorm = liquid_vectorf_norm(_v, _n);

    // scale values (avoiding division by zero)
    float scale = vnorm == 0.0f ? 0.0f : 1.0f / vnorm;
    unsigned int i;
    for (i=0; i<_n; i++)
        _v[i] *= scale;

    // return normalization
    return vnorm;
}

