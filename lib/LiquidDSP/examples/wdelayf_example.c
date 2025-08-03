//
// wdelayf_example.c
//
// SEE ALSO: bufferf_example.c
//           windowf_example.c

#include <stdio.h>

#include "liquid.h"

#define OUTPUT_FILENAME "wdelayf_example.m"

int main() {
    unsigned int delay = 10;
    unsigned int num_samples = 64;

    // create wdelay, all elements initialized to 0
    wdelayf w = wdelayf_create(delay);
    float y; // output

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"delay = %u;\n", delay);

    // push several elements
    unsigned int i;
    float x;
    for (i=0; i<num_samples; i++) {
        if (i==0)   x = 1.0f;
        else        x = 0.0f;

        wdelayf_push(w, x);
        wdelayf_read(w, &y);

        printf("%4u : %12.8f\n", i, y);
        fprintf(fid,"x(%4u) = %12.8f; y(%4u) = %12.8f;\n", i+1, x, i+1, y);
    }
    wdelayf_print(w);

    fprintf(fid,"\n\n");
    fprintf(fid,"t = 0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,x,t,y);\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    wdelayf_destroy(w);

    printf("done.\n");
    return 0;
}


