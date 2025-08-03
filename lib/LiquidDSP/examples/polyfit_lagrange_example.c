// 
// polyfit_lagrange_example.c
//
// Test exact polynomial fit to sample data using Lagrange
// interpolating polynomials.
// SEE ALSO: polyfit_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "polyfit_lagrange_example.m"

int main() {
    unsigned int n=15;      // number of samples

    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");

    // initialize data vectors
    float x[n];
    float y[n];
    unsigned int i;
    for (i=0; i<n; i++) {
        // compute Chebyshev points of the second kind
        x[i] = cosf(M_PI*(float)(i)/(float)(n-1));

        // random samples
        y[i] = 0.2f*randnf();

        printf("x : %12.8f, y : %12.8f\n", x[i], y[i]);
        fprintf(fid,"x(%3u) = %12.4e; y(%3u) = %12.4e;\n", i+1, x[i], i+1, y[i]);
    }

    // compute Lagrange interpolation weights
    //float p[n];
    //polyf_fit_lagrange(x,y,n,p);
    float w[n];
    polyf_fit_lagrange_barycentric(x,n,w);

    // print coefficients
    // NOTE : for Chebyshev points of the second kind, w[i] = (-1)^i * (i==0 || i==n-1 ? 1 : 2)
    for (i=0; i<n; i++)
        printf("  w[%3u] = %12.4e;\n", i, w[i]);

    // evaluate polynomial
    float xmin = -1.1f;
    float xmax =  1.1f;
    unsigned int num_steps = 16*n;
    float dx = (xmax-xmin)/(num_steps-1);
    float xtest = xmin;
    float ytest;
    for (i=0; i<num_steps; i++) {
        ytest = polyf_val_lagrange_barycentric(x,y,w,xtest,n);
        fprintf(fid,"xtest(%3u) = %12.4e; ytest(%3u) = %12.4e;\n", i+1, xtest, i+1, ytest);
        xtest += dx;
    }

    // plot results
    fprintf(fid,"plot(x,y,'s',xtest,ytest,'-');\n");
    fprintf(fid,"xlabel('x');\n");
    fprintf(fid,"ylabel('y, p^{(%u)}(x)');\n", n);
    fprintf(fid,"legend('data','poly-fit (barycentric)',0);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1.1 1.1 1.5*min(y) 1.5*max(y)]);\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

