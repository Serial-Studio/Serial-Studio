//
// ordfilt_rrrf_example.c
//

#include <stdio.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "ordfilt_rrrf_example.m"

int main() {
    // options
    unsigned int num_samples = 2400; // number of random input samples
    unsigned int n           =  101; // filter length
    unsigned int k           =    5; // order statistic index

    // arrays
    float x[num_samples];   // filter input
    float y[num_samples];   // filter output

    // generate input tone with noise
    unsigned int i;
    for (i=0; i<num_samples; i++)
        x[i] = -cosf(2*M_PI*(float)i/(float)num_samples) + fabsf(randnf());

    // create object
    ordfilt_rrrf q = ordfilt_rrrf_create(n,k);
    ordfilt_rrrf_print(q);

    // apply filter
    ordfilt_rrrf_execute_block(q,x,num_samples,y);

    // destroy filter object
    ordfilt_rrrf_destroy(q);

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% ordfilt_rrrf_example.m: auto-generated file\n\n");
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"num_samples=%u;\n", num_samples);
    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    
    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%4u) = %12.4e; y(%4u) = %12.4e;\n", i+1, x[i], i+1, y[i]);

    fprintf(fid,"t = 0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,x,'Color',[0.3 0.3 0.3],...\n");
    fprintf(fid,"     t,y,'LineWidth',2);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('signals');\n");
    fprintf(fid,"axis([0 num_samples -4 4]);\n");
    fprintf(fid,"legend('noise','filtered noise',1);");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

