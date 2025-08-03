//
// gradsearch_datafit_example.c
//
// Fit 3-parameter curve to sampled data set in the minimum
// mean-squared error sense.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "gradsearch_datafit_example.m"

// gradient search data set
struct gsdataset {
    float * x;
    float * y;
    unsigned int n;
};

// gradient search curve-fit error
float gserror(void * _dataset,
              float * _v,
              unsigned int _n);

// parameterized function
float gsfunc(float _x, float * _v)
{
    float c0 = _v[0];
    float c1 = _v[1];
    float c2 = _v[2];

    return c0 + sincf(c1*(_x-c2));
}


int main() {
    // options
    unsigned int num_samples = 400;     // number of samples
    float sig = 0.1f;                   // noise variance
    unsigned int num_iterations = 1000; // number of iterations to run

    float v[3] = {1, 1, 1};
    unsigned int i;

    // range
    float xmin = 0.0f;
    float xmax = 6.0f;
    float dx = (xmax - xmin) / (num_samples-1);

    // generate data set
    float x[num_samples];
    float y[num_samples];
    for (i=0; i<num_samples; i++) {
        x[i] = xmin + i*dx;
        y[i] = sincf(x[i]) + randnf()*sig;
    }
    struct gsdataset q = {x, y, num_samples};

    // create gradsearch object
    gradsearch gs = gradsearch_create((void*)&q, v, 3, gserror, LIQUID_OPTIM_MINIMIZE);

    float rmse;

    // execute search
    //rmse = gradsearch_run(gs, num_iterations, -1e-6f);

     // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    // execute search one iteration at a time
    fprintf(fid,"u = zeros(1,%u);\n", num_iterations);
    for (i=0; i<num_iterations; i++) {
        rmse = gserror((void*)&q,v,3);
        fprintf(fid,"u(%3u) = %12.4e;\n", i+1, rmse);

        gradsearch_step(gs);

        if (((i+1)%100)==0)
            gradsearch_print(gs);
    }

    // print results
    printf("\n");
    gradsearch_print(gs);
    printf("  c0 = %12.8f, opt = 1\n", v[0]);
    printf("  c1 = %12.8f, opt = 0\n", v[1]);
    printf("  c2 = %12.8f, opt = 1\n", v[2]);
    printf("  rmse = %12.4e\n", rmse);

    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(u);\n");
    fprintf(fid,"xlabel('iteration');\n");
    fprintf(fid,"ylabel('error');\n");
    fprintf(fid,"title('gradient search results');\n");
    fprintf(fid,"grid on;\n");

    // save sampled data set
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"  x(%4u) = %12.8f;\n", i+1, x[i]);
        fprintf(fid,"  y(%4u) = %12.8f;\n", i+1, y[i]);
        fprintf(fid,"  y_hat(%4u) = %12.8f;\n", i+1, gsfunc(x[i],v));
    }
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(x,y,'x', x,y_hat,'-');\n");
    fprintf(fid,"xlabel('x');\n");
    fprintf(fid,"ylabel('f(x)');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"legend('data','fit',1);\n");


    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    gradsearch_destroy(gs);

    return 0;
}

// gradient search fit
float gserror(void * _dataset,
              float * _v,
              unsigned int _n)
{
    struct gsdataset * p = (struct gsdataset *) _dataset;

    float rmse = 0.0f; // mean-squared error
    unsigned int i;
    for (i=0; i<p->n; i++) {
        // compute function estimate
        float y_hat = gsfunc(p->x[i], _v);

        // compute error
        float e = p->y[i] - y_hat;

        // accumulate RMS error
        rmse += e*e;
    }

    // normalize error and return
    rmse = sqrtf(rmse / (float)(p->n));
    return rmse;
}

