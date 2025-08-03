// Test polynomial fit to sample data.
#include <stdio.h>
#include "liquid.h"
#define OUTPUT_FILENAME "polyfit_example.m"

int main() {
    unsigned int n= 25;      // number of samples
    unsigned int order=2;   // polynomial order

    // initialize data vectors
    float x[n];
    float y[n];
    unsigned int i;
    for (i=0; i<n; i++) {
        x[i] = 14*randf()-7;
        y[i] = 0.1*x[i]*x[i] + 0.1*randnf();
    }

    // compute coefficients
    float p[order+1];
    polyf_fit(x,y,n,p,order+1);

    // evaluate polynomial and print results
    for (i=0; i<n; i++)
        printf("x:%12.6f, y:%12.6f -> %12.6f\n", x[i],y[i],polyf_val(p,order+1,x[i]));
    for (i=0; i<=order; i++)
        printf("p[%3u] = %12.8f\n", i, p[i]);

    // plot results
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"n = %u; order = %u;\n", n, order);
    fprintf(fid,"x = zeros(1,n); y = zeros(1,n); p = zeros(1,order+1);\n");
    for (i=0; i<n; i++)
        fprintf(fid,"x(%3u) = %12.4e; y(%3u) = %12.4e;\n", i+1, x[i], i+1, y[i]);
    for (i=0; i<=order; i++)
        fprintf(fid,"p(%3u) = %12.4e;\n", i+1, p[order-i]); // reverse order
    fprintf(fid,"xtest = linspace(-8,8,101);\n");
    fprintf(fid,"ytest = polyval(p,xtest);\n");
    fprintf(fid,"plot(x,y,'x',xtest,ytest,'-');\n");
    fprintf(fid,"xlabel('x');\n");
    fprintf(fid,"ylabel('y, p^{(%u)}(x)');\n", order);
    fprintf(fid,"legend('data','poly-fit');\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);
    printf("done.\n");
    return 0;
}

