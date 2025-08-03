//
// scramble_example.c
//
// Data-scrambling example.  Physical layer synchronization of
// received waveforms relies on independent and identically
// distributed underlying data symbols.  If the message sequence,
// however, is '00000....' and the modulation scheme is BPSK,
// the synchronizer probably won't be able to recover the symbol
// timing.  It is imperative to increase the entropy of the data
// for this to happen.  The data scrambler routine attempts to
// 'whiten' the data sequence with a bit mask in order to achieve
// maximum entropy.  This example demonstrates the interface.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.h"

float compute_entropy(unsigned char * _x,
                      unsigned int _n);

int main() {
    unsigned int n=32;  // number of data bytes

    unsigned char x[n]; // input data
    unsigned char y[n]; // scrambled data
    unsigned char z[n]; // unscrambled data

    unsigned int i;

    // generate data
    for (i=0; i<n; i++)
        x[i] = rand() % 2 ? 0x0f : 0xc8;

    // scramble input
    memmove(y,x,n);
    scramble_data(y,n);

    // unscramble result
    memmove(z,y,n);
    unscramble_data(z,n);

    float Hx = compute_entropy(x,n);
    float Hy = compute_entropy(y,n);

    // print results
    printf("i\tx\ty\tz\n");
    printf("--\t--\t--\t--\n");
    for (i=0; i<n; i++)
        printf("%u\t%.2x\t%.2x\t%.2x\n", i, x[i], y[i], z[i]);

    printf("H(x) = %12.8f\n", Hx);
    printf("H(y) = %12.8f\n", Hy);

    printf("done.\n");
    return 0;
}

// raw entropy calculation, orders 1, 2, 4, 8
float compute_entropy(unsigned char * _x,
                      unsigned int _n)
{
    unsigned int i;
    unsigned int j;

    // initialize counter arrays
    unsigned int c1[2];     // 1-bit symbols
    unsigned int c2[4];     // 2-bit symbols
    unsigned int c4[16];    // 4-bit symbols
    unsigned int c8[256];   // 8-bit symbols

    for (i=0; i<2;   i++) c1[i] = 0;
    for (i=0; i<4;   i++) c2[i] = 0;
    for (i=0; i<16;  i++) c4[i] = 0;
    for (i=0; i<256; i++) c8[i] = 0;

    for (i=0; i<_n; i++) {
        // b:1
        for (j=0; j<8; j++)  c1[ (_x[i] >> j) & 0x01 ]++;

        // b:2
        for (j=0; j<8; j+=2) c2[ (_x[i] >> j) & 0x03 ]++;

        // b:4
        for (j=0; j<8; j+=4) c4[ (_x[i] >> j) & 0x0f ]++;

        // b:8
        c8[ _x[i] ]++;
    }

    // compute entropy
    float H1 = 0.0f;
    float H2 = 0.0f;
    float H4 = 0.0f;
    float H8 = 0.0f;

    float p;
    for (i=0; i<2; i++) {
        p = (float) c1[i] / (float)(8*_n);
        H1 -= p * log2f(p+1e-12f);
    }
    H1 /= 1.0f;

    for (i=0; i<4; i++) {
        p = (float) c2[i] / (float)(4*_n);
        H2 -= p * log2f(p+1e-12f);
    }
    H2 /= 2.0f;

    for (i=0; i<16; i++) {
        p = (float) c4[i] / (float)(2*_n);
        H4 -= p * log2f(p+1e-12f);
    }
    H4 /= 4.0f;

    for (i=0; i<256; i++) {
        p = (float) c8[i] / (float)(_n);
        H8 -= p * log2f(p+1e-12f);
    }
    H8 /= 8.0f;

#if 0
    printf("H1 : %12.8f\n", H1);
    printf("H2 : %12.8f\n", H2);
    printf("H4 : %12.8f\n", H4);
    printf("H8 : %12.8f\n", H8);
#endif

    // not sure if product is truly the entropy, but
    // it is a fair assessment
    return H1 * H2 * H4 * H8;
}

