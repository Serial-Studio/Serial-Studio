//
// firinterp_firdecim_crcf_example.c
//
// This example demonstrates interpolation and decimation of a QPSK
// signal with a square-root Nyquist filter.
// Data symbols are generated and then interpolated according to a
// finite impulse response square-root Nyquist filter.  The resulting
// sequence is then decimated with the same filter, matched to the
// interpolator.
//
// SEE ALSO: firinterp_crcf_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firinterp_firdecim_crcf_example.m"

// print usage/help message
void usage()
{
    printf("firinterp_firdecim_crcf_example:\n");
    printf("  -h         : print usage/help\n");
    printf("  -k <s/sym> : samples/symbol (interp factor), k > 1, default: 2\n");
    printf("  -m <delay> : filter delay (symbols), m > 0,         default: 2\n");
    printf("  -b <bw>    : excess bandwidth factor, 0 < beta < 1, default: 0.5\n");
    printf("  -n <num>   : number of data symbols,                default: 8\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int k        = 2;      // samples/symbol
    unsigned int m        = 3;      // filter delay
    float        dt       = 0.5f;   // filter fractional symbol delay
    float        beta     = 0.5f;   // filter excess bandwidth
    unsigned int num_syms = 8;      // number of data symbols

    int dopt;
    while ((dopt = getopt(argc,argv,"hk:m:b:n:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                          return 0;
        case 'k': k = atoi(optarg);                 break;
        case 'm': m = atoi(optarg);                 break;
        case 'b': beta = atof(optarg);              break;
        case 'n': num_syms = atoi(optarg);  break;
        default:
            usage();
            return 1;
        }
    }

    // validate options
    if (k < 2) {
        fprintf(stderr,"error: %s, interp factor must be greater than 1\n", argv[0]);
        return 1;
    } else if (m < 1) {
        fprintf(stderr,"error: %s, filter delay must be greater than 0\n", argv[0]);
        return 1;
    } else if (beta <= 0.0 || beta > 1.0f) {
        fprintf(stderr,"error: %s, beta (excess bandwidth factor) must be in (0,1]\n", argv[0]);
        return 1;
    } else if (num_syms < 1) {
        fprintf(stderr,"error: %s, must have at least one data symbol\n", argv[0]);
        return 1;
    }

    // derived values
    unsigned int h_len          = 2*k*m + 1;        // prototype filter length
    unsigned int num_syms_total = num_syms + 2*m;   // number of total symbols (w/ delay)
    unsigned int num_samples    = k*num_syms_total; // number of samples

    // design filter and create interpolator and decimator objects
    float h[h_len];     // transmit filter
    float g[h_len];     // receive filter (reverse of h)
    liquid_firdes_rrcos(k,m,beta,dt,h);
    unsigned int i;
    for (i=0; i<h_len; i++)
        g[i] = h[h_len-i-1];
    firinterp_crcf interp = firinterp_crcf_create(k,h,h_len);
    firdecim_crcf  decim  = firdecim_crcf_create(k,g,h_len);
    firdecim_crcf_set_scale(decim, 1.0f/(float)k);

    // allocate memory for buffers
    float complex x[num_syms_total];   // input symbols
    float complex y[num_samples];   // interpolated sequence
    float complex z[num_syms_total];   // decimated (received) symbols

    // generate input symbols, padded with zeros at the end
    for (i=0; i<num_syms_total; i++) {
        float complex s = (rand() % 2 ? 1.0f : -1.0f) +
                          (rand() % 2 ? 1.0f : -1.0f) * _Complex_I;
        x[i] = i < num_syms ? s : 0;
    }

    // run interpolator
    for (i=0; i<num_syms_total; i++)
        firinterp_crcf_execute(interp, x[i], &y[k*i]);

    // run decimator
    for (i=0; i<num_syms_total; i++)
        firdecim_crcf_execute(decim, &y[k*i], &z[i]);

    // destroy objects
    firinterp_crcf_destroy(interp);
    firdecim_crcf_destroy(decim);

    // print results to screen
    printf("filter impulse response :\n");
    for (i=0; i<h_len; i++)
        printf("  [%4u] : %8.4f\n", i, h[i]);

    printf("input symbols\n");
    for (i=0; i<num_syms_total; i++) {
        printf("  [%4u] : %8.4f + j*%8.4f", i, crealf(x[i]), cimagf(x[i]));

        // highlight actual data symbols
        if (i < num_syms) printf(" *\n");
        else                      printf("\n");
    }

    printf("interpolator output samples:\n");
    for (i=0; i<num_samples; i++) {
        printf("  [%4u] : %8.4f + j*%8.4f", i, crealf(y[i]), cimagf(y[i]));

        if ( (i >= k*m) && ((i%k)==0))  printf(" **\n");
        else                            printf("\n");
    }

    printf("output symbols:\n");
    for (i=0; i<num_syms_total; i++) {
        printf("  [%4u] : %8.4f + j*%8.4f", i, crealf(z[i]), cimagf(z[i]));

        // highlight symbols (compensate for filter delay)
        if ( i < 2*m ) printf("\n");
        else           printf(" *\n");
    }

    //
    // export results to file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"dt = %8.6f;\n", dt);
    fprintf(fid,"h_len=%u;\n",h_len);
    fprintf(fid,"num_syms_total = %u;\n", num_syms_total);
    fprintf(fid,"num_samples = k*num_syms_total;\n");
    fprintf(fid,"h = zeros(1,h_len);\n");
    fprintf(fid,"x = zeros(1,num_syms_total);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");

    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%4u) = %12.4e;\n", i+1, h[i]);

    for (i=0; i<num_syms_total; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));

    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    for (i=0; i<num_syms_total; i++)
        fprintf(fid,"z(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(z[i]), cimagf(z[i]));

    fprintf(fid,"\n\n");
    fprintf(fid,"tx = [0:(num_syms_total-1)];\n");
    fprintf(fid,"ty = [0:(num_samples-1)]/k - m + dt/k;\n");
    fprintf(fid,"tz = [0:(num_syms_total-1)] - 2*m;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"    plot(tx,real(x),'s',ty,real(y),'-',tz,real(z),'x');\n");
    fprintf(fid,"    xlabel('time');\n");
    fprintf(fid,"    ylabel('real');\n");
    fprintf(fid,"    grid on;\n");
    fprintf(fid,"    legend('symbols in','interp','symbols out',0);\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"    plot(tx,imag(x),'s',ty,imag(y),'-',tz,imag(z),'x');\n");
    fprintf(fid,"    xlabel('time');\n");
    fprintf(fid,"    ylabel('imag');\n");
    fprintf(fid,"    grid on;\n");

    fclose(fid);
    printf("results written to %s.\n",OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
