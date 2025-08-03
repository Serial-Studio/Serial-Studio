// This example demonstrates the autocorr (auto-correlation) object
// functionality.  A random time-domain sequence is generated which
// exhibits time-domain repetitions (auto-correlation properties),
// for example:  abcdabcdabcd....abcd.  The sequence is pushed through
// the autocorr object, and the results are written to an output file.
// The command-line arguments allow the user to experiment with the
// sequence length, number of sequence repetitions, and properties of
// the auto-correlator, as well as signal-to-noise ratio.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "autocorr_cccf_example.m"

// print usage/help message
void usage()
{
    printf("autocorr_cccf_example:\n");
    printf("  u/h   : print usage/help\n");
    printf("  m     : sequence length, default: 32\n");
    printf("  n     : number of sequences (repetitions), default: 8\n");
    printf("  w     : autocorr window length, default: 64\n");
    printf("  d     : autocorr delay length (multiple of 's'), default: 32\n");
    printf("  e     : normalize by energy? default: off\n");
    printf("  s     : SNR, signal-to-noise ratio [dB], default: 20\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int sequence_len = 32;     // short sequence length
    unsigned int n = 8;                 // number short sequences (repetition length)
    unsigned int window_size = 64;      // autocorr window size
    unsigned int delay = sequence_len;  // autocorr delay (multiple of 's')
    int normalize_by_energy = 0;        // normalize output by E{x^2}?
    float SNRdB=20.0f;                  // signal-to-noise ratio (dB)

    int dopt;
    while ((dopt = getopt(argc,argv,"uhm:n:w:d:es:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();                      return 0;
        case 'm': sequence_len = atof(optarg);  break;
        case 'n': n = atof(optarg);             break;
        case 'w': window_size = atof(optarg);   break;
        case 'd': delay = atof(optarg);         break;
        case 'e': normalize_by_energy = 1;      break;
        case 's': SNRdB = atoi(optarg);         break;
            usage();
            return 1;
        }
    }


    // derived values
    unsigned int num_samples = sequence_len*(n+2); // pad end w/ zeros

    // data arrays
    float complex sequence[sequence_len];   // short sequence
    float complex x[num_samples];           // autocorr input sequence
    float complex rxx[num_samples];         // autocorr output

    // generate objects
    autocorr_cccf q = autocorr_cccf_create(window_size,delay);

    unsigned int i;

    // generate random training sequence using QPSK symbols
    modemcf mod = modemcf_create(LIQUID_MODEM_QPSK);
    for (i=0; i<sequence_len; i++)
        modemcf_modulate(mod, rand()%4, &sequence[i]);
    modemcf_destroy(mod);

    // write training sequence 'n' times, followed by zeros
    unsigned int t=0;
    for (i=0; i<n; i++) {
        // copy sequence
        memmove(&x[t], sequence, sequence_len*sizeof(float complex));

        t += sequence_len;
    }

    // pad end with zeros
    for (i=t; i<num_samples; i++)
        x[i] = 0;

    // add noise
    float nstd = powf(10.0f, -SNRdB/20.0f);
    for (i=0; i<num_samples; i++)
        cawgn(&x[i],nstd);
        
    // compute auto-correlation
    for (i=0; i<num_samples; i++) {
        autocorr_cccf_push(q,x[i]);
        autocorr_cccf_execute(q,&rxx[i]);

        // normalize by energy
        if (normalize_by_energy)
            rxx[i] /= autocorr_cccf_get_energy(q);
    }

    // find peak
    float complex rxx_peak = 0;
    for (i=0; i<num_samples; i++) {
        if (i==0 || cabsf(rxx[i]) > cabsf(rxx_peak))
            rxx_peak = rxx[i];
    }
    printf("peak auto-correlation : %12.8f, angle %12.8f\n", cabsf(rxx_peak),
                                                             cargf(rxx_peak));


    // destroy allocated objects
    autocorr_cccf_destroy(q);

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
        fprintf(fid,"x  (%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]  ), cimagf(x[i]  ));
        fprintf(fid,"rxx(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rxx[i]), cimagf(rxx[i]));
    }
    fprintf(fid,"t=1:num_samples;\n");
    fprintf(fid,"figure('position',[100 100 800 600]);\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(x),t,imag(x));\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('received signal');\n");
    fprintf(fid,"  legend('real','imag');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,abs(rxx));\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('auto-correlation magnitude');\n");
    fclose(fid);
    printf("data written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
