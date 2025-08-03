//
// mdct_example.c
//
// Modified discrete cosine transform (MDCT) example. Demonstrates
// the functionality and interface for the mdct and imdct routines.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "mdct_example.m"

int main() {
    // MDCT options
    unsigned int num_channels=64; // MDCT size
    unsigned int num_symbols=16;

    // filter options
    unsigned int h_len=21;  // filter length
    float fc=0.01f;         // filter cutoff
    float As=60.0f;         // stop-band attenuation [dB]
    float mu=0.0f;          // timing offset

    // derived values
    unsigned int i,j;
    unsigned int num_samples = num_channels*num_symbols;
    float x[num_samples];
    float X[num_samples];
    float y[num_samples];
    for (i=0; i<num_samples; i++) {
        x[i] = 0.0f;
        X[i] = 0.0f;
        y[i] = 0.0f;
    }

    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"num_channels = %u;\n", num_channels);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = num_channels*num_symbols;\n");

    // generate window
    float w[2*num_channels];
    for (i=0; i<2*num_channels; i++) {
#if 0
        // raised half sine
        w[i] = sinf(M_PI/(2.0f*num_channels)*(i+0.5f));
#elif 0
        // shaped pulse
        float t0 = sinf(M_PI/(2*num_channels)*(i+0.5));
        w[i] = sinf(M_PI*0.5f*t0*t0);
#else
        w[i] = liquid_kbd(i,2*num_channels,10.0f);
#endif
    }

    // generate basic filter
    float h[h_len];
    liquid_firdes_kaiser(h_len,fc,As,mu,h);
    firfilt_rrrf f = firfilt_rrrf_create(h,h_len);

    // generate random signal (filtered noise)
    for (i=0; i<num_samples; i++) {
        // generate filtered noise
        firfilt_rrrf_push(f, randnf());
        firfilt_rrrf_execute(f, &x[i]); 

        fprintf(fid,"x(%4u) = %12.4e;", i+1, x[i]);
    }
    firfilt_rrrf_destroy(f);

    // run analyzer, accounting for input overlap
    float buffer[2*num_channels];
    memset(buffer, 0x00, 2*num_channels*sizeof(float));
    for (i=0; i<num_symbols; i++) {
        // copy last half of buffer to first half
        memmove(buffer, &buffer[num_channels], num_channels*sizeof(float));

        // copy input block to last half of buffer
        memmove(&buffer[num_channels], &x[i*num_channels], num_channels*sizeof(float));

        // run transform
        mdct(buffer, &X[i*num_channels], w, num_channels);

        //mdct(&x[i*num_channels],&X[i*num_channels],w,num_channels);
    }

    // run synthesizer, accounting for output overlap
    for (i=0; i<num_symbols; i++) {
        // run inverse MDCT
        imdct(&X[i*num_channels],buffer,w,num_channels);

        // accumulate first half of buffer to output
        for (j=0; j<num_channels; j++)
            y[i*num_channels+j] += buffer[j];

        // copy last half of buffer to output (only if we aren't at the last symbol)
        if (i==num_symbols-1) break;
        memmove(&y[(i+1)*num_channels], &buffer[num_channels], num_channels*sizeof(float));
    }

    // print results to file
    fprintf(fid,"w = zeros(1,2*num_channels);\n");
    for (i=0; i<2*num_channels; i++)
        fprintf(fid,"w(%3u) = %12.4e;\n",i+1,w[i]);

    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%3u) = %12.4e;\n", i+1, x[i]);
        fprintf(fid,"y(%3u) = %12.4e;\n", i+1, y[i]);
    }
    fprintf(fid,"X = zeros(num_symbols,num_channels);\n");
    for (i=0; i<num_symbols; i++) {
        for (j=0; j<num_channels; j++)
            fprintf(fid,"X(%3u,%3u) = %12.4e;\n", i+1, j+1, X[i*num_channels+j]);
    }

    // plot spectral response
    /*
    fprintf(fid,"figure;\n");
    fprintf(fid,"f = [0:(num_channels-1)]/(2*num_channels);\n");
    fprintf(fid,"plot(f,20*log10(abs(X')),'Color',[1 1 1]*0.5);\n");
    */

    // plot time-domain reconstruction
    fprintf(fid,"t = 0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,x,t-num_channels,y);\n");
    fprintf(fid,"xlabel('sample index');\n");
    fprintf(fid,"ylabel('signal output');\n");
    fprintf(fid,"legend('original','reconstructed',0);\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

