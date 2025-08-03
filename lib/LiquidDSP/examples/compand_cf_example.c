//
// compand_cf_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "compand_cf_example.m"

int main() {
    // options
    float mu=255.0f;
    int n=31;

    // open debug file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");

    float complex x, y, z;
    int i, j;

    for (i=0; i<n+1; i++) {
        for (j=0; j<n+1; j++) {
            x = (float)(2*i - n)/(float)(n) + _Complex_I*(float)(2*j-n)/(float)(n);
            compress_cf_mulaw(x,mu,&y);
            expand_cf_mulaw(y,mu,&z);

            if (i==j) {
                printf("%8.4f + j*%8.4f > ", crealf(x), cimagf(x));
                printf("%8.4f + j*%8.4f > ", crealf(y), cimagf(y));
                printf("%8.4f + j*%8.4f\n",  crealf(z), cimagf(z));
            }

            fprintf(fid,"x(%3d,%3d) = %12.4e + j*%12.4e;\n", i+1, j+1, crealf(x), cimagf(x));
            fprintf(fid,"y(%3d,%3d) = %12.4e + j*%12.4e;\n", i+1, j+1, crealf(y), cimagf(y));
        }
    }

    for (i=0; i<n+1; i++)
        fprintf(fid,"t(%3d) = %12.4e;\n", i+1, (float)(2*i-n)/(float)(n));

    // plot results
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"mesh(t,t,real(y));\n");
    fprintf(fid,"xlabel('x: real');\n");
    fprintf(fid,"ylabel('x: imag');\n");
    fprintf(fid,"box off;\n");
    fprintf(fid,"view(3);\n");
    fprintf(fid,"title('real[y]');\n");
    //fprintf(fid,"axis([-1 1 -1 1 -1 1]);\n");

    // close debug file
    fclose(fid);
    printf("results wrtten to %s\n", OUTPUT_FILENAME);
    printf("done.\n");
    return 0;
}

