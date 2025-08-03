// 
// quasinewton_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.h"

#define DEBUG_BFGS 0

// utility function pointer
typedef float (*utility)(float * _x, unsigned int _n);

// utility function
float myutility(float * _x, unsigned int _n);

// useful function to print vector to screen
void print_vector(const char * _str, float * _v, unsigned int _n);

// estimate gradient at point _x
//  _x          :   input point, [size: _n x 1]
//  _n          :   dimension
//  _u          :   utility function pointer
//  _gradient   :   resulting gradient (normalized?)
void estimate_gradient(float * _x,
                       unsigned int _n,
                       utility _u,
                       float * _gradient);

// update BFGS estimate of Hessian matrix inverse
//  _n      :   dimension
//  _dx     :   step size (previous), [size: _n x 1]
//  _y      :   gradient difference
//  _H0     :   Hessian matrix inverse estimate (previous), [size: _n x _n]
//  _H1     :   Hessian matrix inverse estimate [size: _n x _n]
void bfgs_update(unsigned int _n,
                 float * _dx,
                 float * _y,
                 float * _H0,
                 float * _H1);

// check the strong Wolfe conditions (c1=0.0001, c2=0.9)
//  _alpha  :   step length
//  _u0     :   f(x)
//  _u1     :   f(x + alpha*p)
//  _gamma0 :   p'*grad(x) (NOTE: should be less than zero)
//  _gamma1 :   p'*grad(x + alpha*p)
int check_wolfe_conditions(float _alpha,
                           float _u0,
                           float _u1,
                           float _gamma0,
                           float _gamma1);

// check Wolfe conditions
//  _x      :   input vector [size: _n x 1]
//  _p      :   step direction vector [size: _n x 1]
//  _n      :   dimension
//  _alpha  :   test step length
//  _u      :   utility function pointer
int check_wolfe_conditions2(float * _x,
                            float * _p,
                            unsigned int _n,
                            float _alpha,
                            utility _u);

int main() {
    // options
    unsigned int n = 4;                 // search dimension
    unsigned int num_iterations = 10;   // number of steps

    // allocate memory for arrays
    float x[n];     // search vector
    float p[n];     // direction
    float dx[n];    // search vector step
    float gradient0[n];
    float gradient1[n];
    float y[n];
    float H0[n*n];
    float H1[n*n];
    float alpha = 0.2f;

    unsigned int i;
    for (i=0; i<n; i++) x[i] = 0.5f;

    // initialize gradient
    for (i=0; i<n; i++) gradient1[i] = 0.0f;

    // initialize Hessian... as identity matrix
    matrixf_eye(H0, n);
    matrixf_eye(H1, n);

    // compute initial gradient estimate
    estimate_gradient(x, n, myutility, gradient1);

    // run search
    printf("\n");

    // print status
    float u = myutility(x, n);
    printf(" -   : x =       {");
    for (i=0; i<n; i++) printf("%8.4f ", x[i]);
    printf("} : %12.8f\n", u);


    unsigned int t;
    for (t=0; t<num_iterations; t++) {
        // copy old gradient estimate
        memmove(gradient0, gradient1, n*sizeof(float));
        memmove(H0, H1, n*n*sizeof(float));

        // compute direction
        matrixf_mul(H0,        n, n,
                    gradient0, n, 1,
                    p,         n, 1);
        for (i=0; i<n; i++) p[i] = -p[i];

        // coarse search for alpha...
        // TODO : improve this search!!!
        float alpha_min = 0.0f;
        float du_min = 0.0f;
        float u0 = myutility(x, n);
        for (i=0; i<100; i++) {
            alpha = 0.001f + 0.01f*i;
            float x_prime[n];
            unsigned int j;
            for (j=0; j<n; j++)
                x_prime[j] = x[j] + alpha*p[j];
            float u1 = myutility(x_prime, n);
            float du = u1 - u0;
            if (i==0 || du < du_min) {
                alpha_min = alpha;
                du_min = du;
            }
        }
        alpha = alpha_min;
        //printf("  alpha = %12.8f\n", alpha);
        // test Wolfe conditions
        if (check_wolfe_conditions2(x,p,n,alpha,myutility)==0)
            printf("warning: Wolfe conditions failed\n");


        // compute step size
        for (i=0; i<n; i++) dx[i] = alpha*p[i];

        // step x
        for (i=0; i<n; i++) x[i] += dx[i];

        // compute gradient estimate
        estimate_gradient(x, n, myutility, gradient1);

        // update y
        for (i=0; i<n; i++) y[i] = gradient1[i] - gradient0[i];

        // update H1
        bfgs_update(n, dx, y, H0, H1);

        // print status
        u = myutility(x, n);
        printf(" %-3u : x =       {", t);
        for (i=0; i<n; i++) printf("%8.4f ", x[i]);
        printf("} : %12.8f\n", u);

#if 0
        print_vector("       gradient0", gradient0, n);
        print_vector("       gradient1", gradient1, n);
        print_vector("       p        ", p, n);
        print_vector("       dx       ", dx, n);
        print_vector("       y        ", y, n);
        printf("H0 =\n"); matrixf_print(H0, n, n);
        printf("H1 =\n"); matrixf_print(H1, n, n);
#endif
    }


    printf("done.\n");
    return 0;
}

// my utility function
float myutility(float * _x, unsigned int _n)
{
#if 0
    float u = 1.0f;
    unsigned int i;
    for (i=0; i<_n; i++) {
        float di = _x[i] - 1.0f;
        float ui = expf(-di*di);
        u *= ui;
    }

    return 1.0f - u;
#else
    return liquid_rosenbrock(NULL, _x, _n);
#endif
}

// useful function to print vector to screen
void print_vector(const char * _str, float * _v, unsigned int _n)
{
    printf("%s {", _str);
    unsigned int i;
    for (i=0; i<_n; i++)
        printf("%8.4f ", _v[i]);
    printf("}\n");
}

// estimate gradient at point _x
//  _x          :   input point, [size: _n x 1]
//  _n          :   dimension
//  _u          :   utility function pointer
//  _gradient   :   resulting gradient (normalized?)
void estimate_gradient(float * _x,
                       unsigned int _n,
                       utility _u,
                       float * _gradient)
{
    // temporary array
    float x_prime[_n];
    float dx = 1e-6f;

    unsigned int i;
    for (i=0; i<_n; i++)
        x_prime[i] = _x[i];
    
    // evaluate at nominal point
    float f0 = _u(x_prime, _n);

    // evaluate 
    float f1 = 0.0f;
    for (i=0; i<_n; i++) {
        // step along dimension i
        x_prime[i] = _x[i] + dx;

        // evaluate
        f1 = _u(x_prime, _n);

        // reset
        x_prime[i] = _x[i];

        // compute derivative
        _gradient[i] = (f1 - f0) / dx;
    }

    // normalize derivative?
}

// update BFGS estimate of Hessian matrix inverse
//  _n      :   dimension
//  _dx     :   step size (previous), [size: _n x 1]
//  _y      :   gradient difference
//  _H0     :   Hessian matrix inverse estimate (previous), [size: _n x _n]
//  _H1     :   Hessian matrix inverse estimate [size: _n x _n]
void bfgs_update(unsigned int _n,
                 float * _dx,
                 float * _y,
                 float * _H0,
                 float * _H1)
{
#if DEBUG_BFGS
    // print inputs
    printf("********************\n");
    print_vector("  dx      ", _dx, _n);
    print_vector("  y       ", _y, _n);
    printf("H0 = \n");
    matrixf_print(_H0,_n,_n);
#endif
    // 
    //float In[_n*_n];        // I(_n)                        [_n x _n]
    float ydxT[_n*_n];      // _y * _dx'                    [_n x _n]
    float yTdx;             // _y' * _dx                    [1  x  1]
    float q[_n*_n];         // I(_n) - (_y*_dx' / _y'_dx)   [_n x _n]
    float dxdxT[_n*_n];     // _dx * _dx'                   [_n x _n]
    float tmp[_n*_n];       // temporary array              [_n x _n]

    // compute _y * _dx'
    matrixf_mul(_y,     _n,     1,
                _dx,    1,      _n,
                ydxT,   _n,     _n);
#if DEBUG_BFGS
    printf("y*dx' = \n");
    matrixf_print(ydxT,_n,_n);
#endif


    // compute _y' * _dx'
    matrixf_mul(_y,     1,      _n,
                _dx,    _n,     1,
                &yTdx,  1,      1);

    // compute _n x _n identity matrix

    // compute q
    unsigned int i;
    unsigned int j;
    for (i=0; i<_n; i++) {
        for (j=0; j<_n; j++) {
            matrix_access(q,_n,_n,i,j) = (i==j ? 1.0f : 0.0f) -
                                         matrix_access(ydxT,_n,_n,i,j)/yTdx;
        }
    }
#if DEBUG_BFGS
    printf("q = \n");
    matrixf_print(q,_n,_n);
#endif

    // compute _dx*_dx'
    matrixf_mul(_dx,   _n,  1,
                _dx,   1,  _n,
                dxdxT, _n, _n);
#if DEBUG_BFGS
    printf("dx*dx' = \n");
    matrixf_print(dxdxT,_n,_n);
#endif

    // compute _H0 * q, store in tmp
    matrixf_mul(_H0, _n, _n,
                q,   _n, _n,
                tmp, _n, _n);

    // compute q' (transpose)
    matrixf_trans(q, _n, _n);

    // compute q' * _H0 * _q, store in _H1
    matrixf_mul(q,   _n, _n,
                tmp, _n, _n,
                _H1, _n, _n);

    // add dxdxT / yTdx to _H1
    for (i=0; i<_n; i++) {
        for (j=0; j<_n; j++) {
            matrix_access(_H1,_n,_n,i,j) += matrix_access(dxdxT,_n,_n,i,j) / yTdx;
        }
    }

#if DEBUG_BFGS
    printf("H1 = \n");
    matrixf_print(_H1,_n,_n);
#endif

}

// check the strong Wolfe conditions (c1=0.0001, c2=0.9)
//  _alpha  :   step length
//  _u0     :   f(x)
//  _u1     :   f(x + alpha*p)
//  _gamma0 :   p'*grad(x) (NOTE: should be less than zero)
//  _gamma1 :   p'*grad(x + alpha*p)
int check_wolfe_conditions(float _alpha,
                           float _u0,
                           float _u1,
                           float _gamma0,
                           float _gamma1)
{
    // TODO : validate input
    // _alpha > 0
    // _gamma < 0

    //
    float c1 = 1e-4f;
    float c2 = 0.9f;

    // check first condition
    int cond1 = (_u1 <= _u0 + c1*_alpha*_gamma0) ? 1 : 0;

    // check second condition
    int cond2 = ( fabsf(_gamma1) <= c2*fabsf(_gamma0) ) ? 1 : 0;

#if 0
    printf("(g0: %12.8f g1: %12.8f) [%c %c]",
            _gamma0,
            _gamma1,
            cond1 ? '1' : '0',
            cond2 ? '1' : '0');
#endif

    //
    return cond1 && cond2;
}

// check Wolfe conditions
//  _x      :   input vector [size: _n x 1]
//  _p      :   step direction vector [size: _n x 1]
//  _n      :   dimension
//  _alpha  :   test step length
//  _u      :   utility function pointer
int check_wolfe_conditions2(float * _x,
                            float * _p,
                            unsigned int _n,
                            float _alpha,
                            utility _u)
{
    // allocate temporary arrays
    float grad0[_n];
    float grad1[_n];
    float x_prime[_n];

    // compute f(x)
    float u0 = _u(_x,_n);

    // compute gradient(f(x))
    estimate_gradient(_x,_n,_u,grad0);

    // compute f(x + alpha*p)
    unsigned int i;
    for (i=0; i<_n; i++)
        x_prime[i] = _x[i] + _alpha*_p[i];
    float u1 = _u(x_prime, _n);

    // compute gradient(f(x + alpha*p))
    estimate_gradient(x_prime, _n, _u, grad1);

    // compute p'*grad0
    float gamma0 = 0.0f;
    for (i=0; i<_n; i++)
        gamma0 += _p[i] * grad0[i];

    // compute p'*grad1
    float gamma1 = 0.0f;
    for (i=0; i<_n; i++)
        gamma1 += _p[i] * grad1[i];

    // test Wolfe conditions
    return check_wolfe_conditions(_alpha, u0, u1, gamma0, gamma1);
}

