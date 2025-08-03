/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// firdes_fexp_test.c : test synthesis of fexp receive filter
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firdes_fexp_test.m"

// print usage/help message
void usage()
{
    printf("Usage: sandbox/firdes_fexp_test [OPTION]\n");
    printf("Run example fexp receive filter design\n");
    printf("\n");
    printf("  u/h   : print usage/help\n");
    printf("  k     : filter samples/symbol, k >= 2, default: 4\n");
    printf("  m     : filter delay (symbols), m >= 1, default: 3\n");
    printf("  b     : filter excess bandwidth factor, 0 < b < 1, default: 0.3\n");
}

float firdes_freqresponse_fexp(float _f, unsigned int _k, float _beta);

int main(int argc, char*argv[]) {
    // options
    unsigned int k=4;       // filter samples/symbol
    unsigned int m=3;       // filter delay [symbols]
    float beta = 0.3f;      // filter excess bandwidth factor

    // read properties from command line
    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:b:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();              return 0;
        case 'k': k     = atoi(optarg); break;
        case 'm': m     = atoi(optarg); break;
        case 'b': beta  = atof(optarg); break;
        default:
            exit(1);
        }
    }


    // validate input
    if (k < 2) {
        fprintf(stderr,"error: %s, k must be at least 2\n", argv[0]);
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: %s, m must be at least 1\n", argv[0]);
        exit(1);
    } else if (beta <= 0.0f || beta >= 1.0f) {
        fprintf(stderr,"error: %s, beta must be in (0,1)\n", argv[0]);
        exit(1);
    }


    unsigned int i;

    // derived values
    unsigned int h_len = 2*k*m+1;   // filter length

    // arrays
    float ht[h_len];         // transmit filter coefficients
    float hr[h_len];         // receive filter coefficients

    //
    // start of filter design procedure
    //

    float H_prime[h_len];           // frequency response of Nyquist filter
    float complex h_tx[h_len];      // impulse response of square-root Nyquist filter
    float complex H_tx[h_len];      // frequency response of square-root Nyquist filter

    // compute frequency response of Nyquist filter
    for (i=0; i<h_len; i++) {
        float f = (float)i / (float)h_len;
        if (f > 0.5f) f = f - 1.0f;

        H_prime[i] = firdes_freqresponse_fexp(f,k,beta);
    }

    // compute square-root response, copy to fft input
    for (i=0; i<h_len; i++)
        H_tx[i] = sqrtf(H_prime[i]);

    // compute ifft and copy response
    fft_run(h_len, H_tx, h_tx, LIQUID_FFT_BACKWARD, 0);
    for (i=0; i<h_len; i++)
        ht[i] = crealf( h_tx[(i+k*m+1)%h_len] ) / (float)(h_len);

    // copy receive...
    for (i=0; i<h_len; i++)
        hr[i] = ht[i];

#if 0
    // print results
    for (i=0; i<h_len; i++)
        printf("H_prime(%3u) = %12.8f;\n", i+1, H_prime[i]);
#endif

    //
    // end of filter design procedure
    //

    // print results to screen
    printf("fexp receive filter:\n");
    for (i=0; i<h_len; i++)
        printf("  hr(%3u) = %12.8f;\n", i+1, hr[i]);

    // compute isi
    float rxy0 = liquid_filter_crosscorr(ht,h_len, hr,h_len, 0);
    float isi_rms = 0.0f;
    for (i=1; i<2*m; i++) {
        float e = liquid_filter_crosscorr(ht,h_len, hr,h_len, i*k) / rxy0;
        isi_rms += e*e;
    }
    isi_rms = sqrtf(isi_rms / (float)(2*m-1));
    printf("\n");
    printf("ISI (RMS) = %12.8f dB\n", 20*log10f(isi_rms));
    

    // 
    // export output file
    //
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"beta = %f;\n", beta);
    fprintf(fid,"h_len = 2*k*m+1;\n");
    fprintf(fid,"nfft = 1024;\n");
    fprintf(fid,"ht = zeros(1,h_len);\n");
    fprintf(fid,"hp = zeros(1,h_len);\n");
    fprintf(fid,"hr = zeros(1,h_len);\n");

    // print results
    for (i=0; i<h_len; i++)   fprintf(fid,"ht(%3u) = %12.4e;\n", i+1, ht[i]);
    for (i=0; i<h_len; i++)   fprintf(fid,"hr(%3u) = %12.4e;\n", i+1, hr[i]);
    
    fprintf(fid,"hc = k*conv(ht,hr);\n");

    // plot results
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"Ht = 20*log10(abs(fftshift(fft(ht,  nfft))));\n");
    fprintf(fid,"Hr = 20*log10(abs(fftshift(fft(hr,  nfft))));\n");
    fprintf(fid,"Hc = 20*log10(abs(fftshift(fft(hc/k,nfft))));\n");
    fprintf(fid,"\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,Ht,'LineWidth',1,'Color',[0.00 0.25 0.50],...\n");
    fprintf(fid,"     f,Hr,'LineWidth',1,'Color',[0.00 0.50 0.25],...\n");
    fprintf(fid,"     f,Hc,'LineWidth',2,'Color',[0.50 0.00 0.00],...\n");
    fprintf(fid,"     [-0.5/k 0.5/k], [1 1]*20*log10(0.5),'or');\n");
    fprintf(fid,"legend('transmit','receive','composite','alias points',1);\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('PSD');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-0.5 0.5 -100 20]);\n");

    fprintf(fid,"\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"tr = [  -k*m:k*m]/k;\n");
    fprintf(fid,"tc = [-2*k*m:2*k*m]/k;\n");
    fprintf(fid,"ic = [0:k:(4*k*m)]+1;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(tr,ht,'-x', tr,hr,'-x');\n");
    fprintf(fid,"  legend('transmit','receive',1);\n");
    fprintf(fid,"  xlabel('Time');\n");
    fprintf(fid,"  ylabel('fexp Tx/Rx Filters');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-2*m 2*m floor(5*min([hr ht]))/5 ceil(5*max([hr ht]))/5]);\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(tc,hc,'-x', tc(ic),hc(ic),'or');\n");
    fprintf(fid,"  xlabel('Time');\n");
    fprintf(fid,"  ylabel('fexp Composite Response');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-2*m 2*m -0.2 1.2]);\n");
    fprintf(fid,"  axis([-2*m 2*m floor(5*min(hc))/5 ceil(5*max(hc))/5]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    return 0;
}

float firdes_freqresponse_fexp(float _f,
                               unsigned int _k,
                               float _beta)
{
    // TODO : validate input

    // enforce even symmetry
    float f = fabsf(_f);

    float f0 = 0.5f*(1.0f - _beta) / (float)_k;
    float f1 = 0.5f*(1.0f        ) / (float)_k;
    float f2 = 0.5f*(1.0f + _beta) / (float)_k;

    if ( f < f0 ) {
        return 1.0f;
    } else if (f > f0 && f < f2) {
        // transition band
        float B     = 0.5f/(float)_k;
        float gamma = logf(2.0f)/(_beta*B);
        if ( f < f1) {
            return expf(gamma*(B*(1-_beta) - f));
        } else {
            return 1.0f - expf(gamma*(f - (1+_beta)*B));
        }
    } else {
        return 0.0f;
    }

    return 0.0f;
}

