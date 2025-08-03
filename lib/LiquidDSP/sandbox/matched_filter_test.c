//
// matched_filter_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "matched_filter_test.m"

// print usage/help message
void usage()
{
    printf("matched_filter_test options:\n");
    printf("  u/h   : print usage/help\n");
    printf("  k     : filter samples/symbol, k >= 2, default: 4\n");
    printf("  m     : filter delay (symbols), m >= 1, default: 2\n");
    printf("  b     : filter excess bandwidth factor, 0 < b < 1, default: 0.9\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int k=4;   // samples/symbol
    unsigned int m=2;   // symbol delay
    float beta=0.9f;    // excess bandwidth factor

    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:b:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();            return 0;
        case 'k':   k = atoi(optarg);           break;
        case 'm':   m = atoi(optarg);           break;
        case 'b':   beta = atof(optarg);        break;
        default:
            exit(1);
        }
    }

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

    // design filter
    unsigned int h_len = 2*k*m+1;
    float h[h_len];
    liquid_firdes_rrcos(k,m,beta,0,h);

    // truncate filter to be of length 2*k*m
    unsigned int g_len = 2*k*m;

    // generate receive filter coefficients (reverse of transmit)
    float g[g_len];
    unsigned int i;
    for (i=0; i<g_len; i++)
        g[i] = h[g_len-i-1];

    // create interpolator and decimator
    firfilt_rrrf q = firfilt_rrrf_create(g,g_len);

    // compute auto-correlation of g
    float x;    // input sample
    float y;    // output sample
    for (i=0; i<2*g_len-1; i++) {

        // generate, push input sample
        x = (i < g_len) ? h[i] : 0.0f;
        firfilt_rrrf_push(q, x);
        
        // compute (normalized) output sample
        firfilt_rrrf_execute(q, &y);
        y /= (float)k;

        // print result, highlighting optimal decimation point
        printf("%3u : %c %12.8f\n", i, ((i+1)%k)==0 ? '*' : ' ', y);
    }

    // clean it up
    firfilt_rrrf_destroy(q);

    printf("done.\n");
    return 0;
}

