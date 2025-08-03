//
// math_lngamma_example.c
//
// Demonstrates accuracy of lngamma function
//

#include "liquid.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define OUTPUT_FILENAME "math_lngamma_example.m"

int main() {
    unsigned int n = 256;   // number of steps
    float zmin = 1e-3f;     // minimum value
    float zmax = 6.00f;     // maximum value

    unsigned int d = n/32;  // print every d values to screen

    // log scale values
    float xmin = logf(zmin);
    float xmax = logf(zmax);
    float dx = (xmax-xmin)/(n-1);

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    unsigned int i;
    float z;
    float g;
    float x = xmin; // log(z)
    for (i=0; i<n; i++) {
        z = expf(x);
        g = liquid_lngammaf(z);

        fprintf(fid,"z(%4u) = %16.8e; g(%4u) = %16.8e;\n", i+1, z, i+1, g);
        if ( (i%d)==0 )
            printf("lngamma(%12.8f) = %12.8f\n",z,g);
        x += dx;
    }
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  semilogx(z,g,z,log(gamma(z)));\n");
    fprintf(fid,"  xlabel('z');\n");
    fprintf(fid,"  ylabel('lngamma(z)');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  loglog(z,abs(log(gamma(z))-g));\n");
    fprintf(fid,"  xlabel('z');\n");
    fprintf(fid,"  ylabel('error');\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    return 0;
}
