//
// firfilt_cccf_example.c
//
// This example demonstrates the finite impulse response (FIR) filter
// with complex coefficients as a cross-correlator between transmitted
// and received sequences.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firfilt_cccf_example.m"

// print usage/help message
void usage()
{
    printf("firfilt_cccf_example:\n");
    printf("  h     : print usage/help\n");
    printf("  n     : sequence length, default: 32\n");
    printf("  s     : SNR, signal-to-noise ratio [dB], default: 20\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int sequence_len = 256;    // sequence length
    float        SNRdB        = 10.0f;  // signal-to-noise ratio (dB)

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:s:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'n': sequence_len = atof(optarg);  break;
        case 's': SNRdB = atoi(optarg);         break;
        default:
            usage();
            exit(-1);
        }
    }
    
    unsigned int i;

    // derived values
    unsigned int num_samples = 3*sequence_len;

    // data arrays
    float complex sequence[sequence_len];   // sequence
    float complex x[num_samples];           // input sequence
    float complex rxy[num_samples];         // correlator output

    // generate random sequence
    for (i=0; i<sequence_len; i++) {
        sequence[i] = (rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2) +
                      (rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2) * _Complex_I;
    }

    // generate correlator object
    firfilt_cccf q = firfilt_cccf_create(sequence, sequence_len);

    // normalize by number of points
    firfilt_cccf_set_scale(q, 1.0f / (float)sequence_len);

    // generate the input: fill buffer with zeros then insert
    // the sequence in the middle flipped and conjugated
    for (i=0; i<num_samples; i++)
        x[i] = 0.0f;
    for (i=0; i<sequence_len; i++)
        x[sequence_len + (sequence_len-i-1)] = conjf(sequence[i]);

    // add noise
    float nstd = powf(10.0f, -SNRdB/20.0f);
    for (i=0; i<num_samples; i++)
        cawgn(&x[i],nstd);
        
    // compute cross-correlation
    for (i=0; i<num_samples; i++) {
        firfilt_cccf_push(q,x[i]);
        firfilt_cccf_execute(q,&rxy[i]);
    }

    // find peak
    float complex rxy_peak = 0;
    for (i=0; i<num_samples; i++) {
        if (i==0 || cabsf(rxy[i]) > cabsf(rxy_peak))
            rxy_peak = rxy[i];
    }
    printf("peak cross-correlation : %12.8f, angle %12.8f\n", cabsf(rxy_peak),
                                                              cargf(rxy_peak));

    // destroy allocated objects
    firfilt_cccf_destroy(q);

    // 
    // write results to file
    //
    FILE* fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"num_samples = %u;\n", num_samples);

    // write signal to output file
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n",i+1,crealf(x[i]),cimagf(x[i]));

        fprintf(fid,"rxy(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rxy[i]), cimagf(rxy[i]));
    }

    fprintf(fid,"\n\n");
    fprintf(fid,"t=0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1),\n");
    fprintf(fid,"  hold on;\n");
    fprintf(fid,"  plot(t,real(x),'Color',[0 0.2 0.5]);\n");
    fprintf(fid,"  plot(t,imag(x),'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  hold off;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('received signal');\n");
    fprintf(fid,"  legend('real','imag','location','northeast');\n");
    fprintf(fid,"  axis([0 num_samples -1.5 1.5]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2),\n");
    fprintf(fid,"  plot(t,abs(rxy),'Color',[0.5 0 0],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('cross-correlation magnitude');\n");
    fprintf(fid,"  axis([0 num_samples -0.2 1.2]);\n");
    fprintf(fid,"  grid on;\n");

    fprintf(fid,"\n\n");
    fclose(fid);
    printf("data written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
