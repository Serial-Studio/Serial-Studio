// compare polyfit and polyfit_lagrange
//
// SEE ALSO: polyfit_example.c polyfit_lagrange_example.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME "polyfit_comparison_example.m"

int main()
{
    float x[3] = {-1.0, 0.0, 1.0};
    float y[3] = { 2.0, 7.0, 4.0};
    float p0[3], p1[3];

    // conventional
    polyf_fit         (x,y,3,p0,3);
    polyf_fit_lagrange(x,y,3,p1);

    // evaluate
    unsigned int n = 51;
    float x_eval[n], y0[n], y1[n];
    unsigned int i;
    for (i=0; i<n; i++) {
        x_eval[i] = 2.2f * ((float)i/(float)(n-1) - 0.5f);
        y0[i] = polyf_val(p0, 3, x_eval[i]);
        y1[i] = polyf_val(p1, 3, x_eval[i]);
    }

    // write results to output file for plotting
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"x = [%g,%g,%g];\n", x[0], x[1], x[2]);
    fprintf(fid,"y = [%g,%g,%g];\n", y[0], y[1], y[2]);
    fprintf(fid,"p0= [%g,%g,%g];\n", p0[0], p0[1], p0[2]);
    fprintf(fid,"p1= [%g,%g,%g];\n", p1[0], p1[1], p1[2]);
    fprintf(fid,"n = %u;\n", n);
    fprintf(fid,"y0= zeros(1,n);\n");
    fprintf(fid,"y1= zeros(1,n);\n");
    for (i=0; i<n; i++)
        fprintf(fid,"xeval(%u)=%g;y0(%u)=%g;y1(%u)=%g;\n", i+1,x_eval[i],i+1,y0[i],i+1,y1[i]);
    fprintf(fid,"plot(x,y,'s',xeval,y0,'-x',xeval,y1,'-o');\n");
    fprintf(fid,"legend('data','conventional','lagrange');\n");
    fprintf(fid,"grid on;\n");
    //fprintf(fid,"axis([-1.1 1.1 1.5*min(y) 1.5*max(y)]);\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);
    return 0;
}

