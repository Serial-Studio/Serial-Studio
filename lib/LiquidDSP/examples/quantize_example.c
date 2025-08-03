//
// quantize_example.c
//
// Demonstrates the quantizer/compander combo.
//

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "quantize_example.m"

// print usage/help message
void usage()
{
    printf("quantize_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  b     : number of bits, 0 < b <= 16 [default: 4]\n");
    printf("  m     : mu, compression factor, mu > 0 [default: 255.0]\n");
}


int main(int argc, char*argv[]) {
    unsigned int num_bits=4;
    float mu = 255.0f;
    unsigned int num_samples = 64;

    int dopt;
    while ((dopt = getopt(argc,argv,"uhb:m:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();                  return 0;
        case 'b': num_bits = atoi(optarg);  break;
        case 'm': mu = atof(optarg);        break;
        default:
            exit(1);
        }
    }

    // validate input
    if (num_bits > 16 || num_bits < 1) {
        fprintf(stderr,"error: %s, quantizer bits must be in [1,16]\n", argv[0]);
        exit(1);
    } else if (mu < 0.0f) {
        fprintf(stderr,"error: %s, mu must be greater than 0\n", argv[0]);
        exit(1);
    }

    unsigned int i;
    float x[num_samples];
    float y[num_samples];

    float v;        // compressed sample
    unsigned int q; // quantized sample
    float u;        // uncompressed sample
    float e;        // error
    float rmse = 0.0f;
    printf("         input        ADC        output\n");
    printf("         -----        ---        ------\n");
    float phi = 0.0f;
    float dphi = 0.02f * 256.0f / (float) num_samples;
    for (i=0; i<num_samples; i++) {
        // generate windowed pulse
        x[i] = 0.5f*cosf(2.0f*M_PI*phi) + 0.5f*cosf(2.0f*M_PI*phi*0.57f);
        x[i] *= 0.5f * liquid_kaiser(i, num_samples, 10.0f);
        phi += dphi;

        // compress sample
        v = compress_mulaw(x[i],mu);

        // quantize: analog to digital converter
        q = quantize_adc(v,num_bits);

        // quantize: digital to analog converter
        u = quantize_dac(q,num_bits);

        // expand sample
        y[i] = expand_mulaw(u,mu);

        // compute error
        e = y[i] - x[i];
        rmse += e*e;

        printf("%4u : %12.8f > 0x%4.4x > %12.8f\n", i, x[i], q, y[i]);
    }
    rmse = sqrtf(rmse / (float)num_samples);
    printf("-----------------------\n");
    printf("rmse : %12.4e\n", rmse);

    // open debug file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"num_samples = %u;\n", num_samples);

    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %16.8e; y(%4u) = %16.8e;\n", i+1, x[i], i+1, y[i]);
    }
    fprintf(fid,"figure;\n");
    fprintf(fid,"t  = 0:(num_samples-1);\n");
    fprintf(fid,"%% generate stairs-like plot\n");
    fprintf(fid,"t0 = reshape([t(1:num_samples-1)(:) t(1:num_samples-1)(:)]',1,2*(num_samples-1)) + 0.5;\n");
    fprintf(fid,"y0 = reshape([y(1:num_samples-1)(:) y(2:num_samples)(:)]',  1,2*(num_samples-1));\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(t,x,t0,y0);\n");
    fprintf(fid,"xlabel('sample index');\n");
    fprintf(fid,"ylabel('signal');\n");
    fprintf(fid,"legend('original','reconstructed',1);\n");
    fprintf(fid,"grid on;\n");
 
    // close debug file
    fclose(fid);
    printf("results wrtten to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

