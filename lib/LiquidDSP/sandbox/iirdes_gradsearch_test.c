// design recursive filter using gradient descent search
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <complex.h>
#include "liquid.h"

#define OUTPUT_FILENAME "iirdes_gradsearch_test.m"
#define DEBUG 0

// highly compressed linear activation function (like tanh but
// hugs lower bound between y=x and y=1 much more closely)
float activation(float _x)
    { float p=4.0f; return copysign(powf(tanhf(powf(fabsf(_x),p)),1./p),_x); }

typedef struct gs_s * gs;
struct gs_s {
    unsigned int n, L, r, nsos; // filter order, etc.
    unsigned int vlen;          // search vector length
    float complex * zd, * pd;   // digital zeros/poles [size: L x 1]
    float           kd, z0, p0; // gain, additional zero/pole for odd-length order
    float * A, * B;             // second-order sections, [size: L+r x 3]
    // fft object, buffers
    // design parameters
    unsigned int nfft;          //
    float * H;                  // frequency response
    float fp, fs;               // pass and stop bands
    float utility;              // utility value
};
gs    gs_create  (unsigned int _order);
void  gs_destroy (gs _q);
void  gs_unpack  (gs _q, float * _v, int _debug);
void  gs_expand  (gs _q, int _debug);
float gs_evaluate(gs _q, int _debug);
void  gs_print   (gs _q, int _debug);
float gs_callback(void * _q, float * _v, unsigned int _n);

int main()
{
    srand(time(NULL));
    gs q = gs_create(2);

    unsigned int n = q->vlen;
    float v[n]; // search vector
    unsigned int i;
    for (i=0; i<n; i++)
        v[i] = 0.0f;

#if 0
    v[0] = -0.99805f; v[1] = 0.06242f;  // zd[0] = -0.99805 +/- 0.06242i
    v[2] = -0.03220f; v[3] = 0.55972f;  // pd[0] = -0.03220 +/- 0.55972i
    v[4] =  0.307f;                     // kd    =  0.30749798
#endif
    gs_unpack(q,v,1);
    gs_expand(q,1);
    gs_evaluate(q,1);
    //gs_destroy(q); return -1;

    unsigned int num_iterations = 250;

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    // create gradsearch object
    printf("creating gradsearch object...\n");
    gradsearch gs = gradsearch_create(q, v, n, gs_callback, LIQUID_OPTIM_MINIMIZE);

    // execute search one iteration at a time
    printf("executing search...\n");
    fprintf(fid,"u = zeros(1,%u);\n", num_iterations);
    fprintf(fid,"v = zeros(%u,%u);\n", n, num_iterations);
    for (i=0; i<num_iterations; i++) {
        float rmse = gs_callback(q,v,n);
        fprintf(fid,"u(%3u) = %12.4e;\n", i+1, rmse);
        fprintf(fid,"v(:,%3u) = [", i+1);
        unsigned int j;
        for (j=0; j<n; j++)
            fprintf(fid,"%12.8f;", v[j]);
        fprintf(fid,"];\n");

        gradsearch_step(gs);

        gradsearch_print(gs);
    }

    // print results
    printf("\n");
    printf("%5u: ", num_iterations);
    gradsearch_print(gs);

    // save frequency response
    fprintf(fid,"nfft = %u;\n", q->nfft);
    fprintf(fid,"f = 0.5*[0:(nfft-1)]/nfft;\n");
    for (i=0; i<q->nfft; i++)
        fprintf(fid,"H(%4u) = %16.8e;\n", i+1, q->H[i]);

    // plot utility over time
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(u,'-x');\n");
    fprintf(fid,"xlabel('iteration');\n");
    fprintf(fid,"ylabel('utility');\n");
    fprintf(fid,"title('gradient search results');\n");
    fprintf(fid,"grid on;\n");

    // plot frequency response
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,20*log10(H));\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('PSD [dB[');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    gs_print(q,1);

    gradsearch_destroy(gs);
    gs_destroy(q);

    return 0;
}

gs gs_create(unsigned int _order)
{
    gs q    = (gs) malloc(sizeof(struct gs_s));
    q->n    = _order;
    q->r    = (q->n) % 2;
    q->L    = (q->n - q->r)/2;
    q->vlen = 4*q->L + 2*q->r + 1; // optimum vector length
    q->nsos = q->L + q->r;
    q->nfft = 1024;
    q->zd   = (float complex*)malloc(q->n*sizeof(float complex));
    q->pd   = (float complex*)malloc(q->n*sizeof(float complex));
    q->B    = (float        *)malloc(3*q->nsos*sizeof(float));
    q->A    = (float        *)malloc(3*q->nsos*sizeof(float));
    q->H    = (float        *)malloc(q->nfft*sizeof(float));
    q->fp   = 0.2f;
    q->fs   = 0.3f;
    return q;
}

void gs_destroy(gs _q)
{
    // free allocated memory
    free(_q->zd);
    free(_q->pd);
    free(_q->B);
    free(_q->A);
    free(_q->H);
    free(_q);
}

// unpack poles, zeros, and gain
// format: [{z-mag, z-phase, p-mag, p-phase}*L, k, {z-real, p-real}*r]
void gs_unpack(gs _q, float * _v, int _debug)
{
    unsigned int i;
    float x, y, r, t;
    for (i=0; i<_q->L; i++) {
        x = _v[4*i+0]; y = _v[4*i+1];
        r = activation(sqrtf(x*x+y*y));
        t = atan2f(y,x);
        _q->zd[2*i+0] = r * cexpf(-_Complex_I*t);
        _q->zd[2*i+1] = r * cexpf(+_Complex_I*t);

        x = _v[4*i+2]; y = _v[4*i+3];
        r = activation(sqrtf(x*x+y*y));
        t = atan2f(y,x);
        _q->pd[2*i+0] = r * cexpf(-_Complex_I*t);
        _q->pd[2*i+1] = r * cexpf(+_Complex_I*t);
    }
    // unpack real-valued zero/pole
    if (_q->r) {
        _q->zd[2*_q->L] = tanhf(_v[4*_q->L  ]);
        _q->pd[2*_q->L] = tanhf(_v[4*_q->L+1]);
    }
    _q->kd = _v[4*_q->L + 2*_q->r]; // unpack gain

    // debug: print values
    if (_debug) {
        printf("gs_unpack:\n");
        printf("v : [");
        for (i=0; i<_q->vlen; i++)
            printf("%8.5f,", _v[i]);
        printf("]\n");
        gs_print(_q, _debug);
    }
}

// expand second-order sections
void gs_expand(gs _q, int _debug)
{
    // convert complex digital poles/zeros/gain into second-
    // order sections form
    iirdes_dzpk2sosf(_q->zd,_q->pd,_q->n,_q->kd,_q->B,_q->A);

    // print coefficients
    if (_debug) {
        unsigned int i;
        printf("B [%u x 3] :\n", _q->L+_q->r);
        for (i=0; i<_q->L+_q->r; i++)
            printf("  %12.8f %12.8f %12.8f\n", _q->B[3*i+0], _q->B[3*i+1], _q->B[3*i+2]);
        printf("A [%u x 3] :\n", _q->L+_q->r);
        for (i=0; i<_q->L+_q->r; i++)
            printf("  %12.8f %12.8f %12.8f\n", _q->A[3*i+0], _q->A[3*i+1], _q->A[3*i+2]);
    }
}

float gs_evaluate(gs _q, int _debug)
{
    float u = 0.0f;
    unsigned int n;
    for (n=0; n<_q->nfft; n++) {
        float f = 0.5f * (float)n / (float)_q->nfft;
        float D = 0.0f;
        float W = 1.0f;
        if (f < _q->fp) { // pass band
            D = 1.0f; W = 1.0f;
        } else if (f > _q->fs) { // stop band
            D = 0.0f; W = 1.0f;
        } else { // transition band (don't care)
            D = 0.0f; W = 0.0f;
        }

        // compute 3-point DFT for each second-order section
        float complex H = 1.0f;
        unsigned int i;
        for (i=0; i<_q->nsos; i++) {
            float complex Hb =  _q->B[3*i+0] * cexpf(_Complex_I*2*M_PI*f*0) +
                                _q->B[3*i+1] * cexpf(_Complex_I*2*M_PI*f*1) +
                                _q->B[3*i+2] * cexpf(_Complex_I*2*M_PI*f*2);

            float complex Ha =  _q->A[3*i+0] * cexpf(_Complex_I*2*M_PI*f*0) +
                                _q->A[3*i+1] * cexpf(_Complex_I*2*M_PI*f*1) +
                                _q->A[3*i+2] * cexpf(_Complex_I*2*M_PI*f*2);

            // consolidate
            H *= Hb / Ha;
        }
        // compare to ideal
        float H_abs = crealf(H)*crealf(H) + cimagf(H)*cimagf(H);
        float e = W * (D-H_abs)*(D-H_abs);
        if (_debug)
            printf(" %5u %8.6f %5.3f %16.13f %16.13f\n", n, f, D, H_abs, e);
        u += e*e;
        _q->H[n] = H_abs; // save response
    }
    //u /= (float)(_q->nfft);
    if (_debug) printf("u = %12.8f\n", u);
    _q->utility = u;
    return _q->utility;
}

void gs_print(gs _q, int _debug)
{
    // print digital z/p/k
    unsigned int i;
    printf("zeros (digital):\n");
    for (i=0; i<_q->n; i++)
        printf("  zd[%3u] = %12.4e + j*%12.4e;\n", i, crealf(_q->zd[i]), cimagf(_q->zd[i]));
    printf("poles (digital):\n");
    for (i=0; i<_q->n; i++)
        printf("  pd[%3u] = %12.4e + j*%12.4e;\n", i, crealf(_q->pd[i]), cimagf(_q->pd[i]));
    printf("gain (digital):\n");
    printf("  kd : %12.8f + j*%12.8f\n", crealf(_q->kd), cimagf(_q->kd));
    // print response...
    printf("  u  : %12.8f\n", _q->utility);
    gs_evaluate(_q, 1);
}

// gradient search error
float gs_callback(void * _context,
                  float * _v,
                  unsigned int _n)
{
    gs _q = (gs)_context;
    if (_n != _q->vlen) {
        fprintf(stderr,"search vector length mismatch\n");
        exit(1);
    }
    gs_unpack(_q, _v, DEBUG);
    gs_expand(_q, DEBUG);
    return gs_evaluate(_q, DEBUG);
}

#if 0
make examples/iirdes_example && ./examples/iirdes_example -t ellip -n 2 -o sos -f 0.25
B [1 x 3] :
    0.30749798   0.61379653   0.30749798
A [1 x 3] :
    1.00000000   0.06440119   0.31432679
results written to iirdes_example.m.
done.


make examples/iirdes_example && ./examples/iirdes_example -t ellip -n 2 -o tf -f 0.25
a[  0] =   1.00000000;
a[  1] =   0.06440119;
a[  2] =   0.31432679;
b[  0] =   0.30749798;
b[  1] =   0.61379653;
b[  2] =   0.30749798;
results written to iirdes_example.m.
done.

a = [1.00000000   0.06440119   0.31432679];
b = [0.30749798   0.61379653   0.30749798];

roots(a) - poles
  -0.03220 + 0.55972i
  -0.03220 - 0.55972i

roots(b) - zeros
  -0.99805 + 0.06242i
  -0.99805 - 0.06242i


#endif
