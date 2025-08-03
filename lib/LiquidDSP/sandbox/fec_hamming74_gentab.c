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
// 1/2-rate (7,4) Hamming code table generator
//

#include <stdio.h>
#include <stdlib.h>

// generator matrices

// encoding matrix
#define HAMMING74_G0    0x0d    // 1101
#define HAMMING74_G1    0x0b    // 1011
#define HAMMING74_G2    0x08    // 1000
#define HAMMING74_G3    0x07    // 0111
#define HAMMING74_G4    0x04    // 0100
#define HAMMING74_G5    0x02    // 0010
#define HAMMING74_G6    0x01    // 0001

// syndrome matrix
#define HAMMING74_H0    0x55    // .101 0101
#define HAMMING74_H1    0x33    // .011 0011
#define HAMMING74_H2    0x0f    // .000 1111

// decoding matrix
#define HAMMING74_R0    0x10    // .001 0000
#define HAMMING74_R1    0x04    // .000 0100
#define HAMMING74_R2    0x02    // .000 0010
#define HAMMING74_R3    0x01    // .000 0001

unsigned int count_ones(unsigned int _x);
unsigned int hamming74_enc_symbol(unsigned int _p);
unsigned int hamming74_dec_symbol(unsigned int _x);

int main() {
    unsigned int n = 16;
    unsigned int k = 128;

    // generate encoding table
    unsigned int enc_gentab[n];
    unsigned int p;
    for (p=0; p<n; p++)
        enc_gentab[p] = hamming74_enc_symbol(p);

    // generate decoding table
    unsigned int dec_gentab[k];
    unsigned int x;
    for (x=0; x<k; x++)
        dec_gentab[x] = hamming74_dec_symbol(x);

    printf("// Hamming(7,4) table generator\n");
    printf("\n");
    printf("unsigned char enc_gentab[%u] = {\n    ", n);
    for (p=0; p<n; p++) {
        printf("0x%.2x", enc_gentab[p]);
        if (p==n-1)
            printf("};\n");
        else if (((p+1)%8)==0)
            printf(",\n    ");
        else
            printf(", ");
    }

    printf("\n");
    printf("unsigned char dec_gentab[%u] = {\n    ", k);
    for (x=0; x<k; x++) {
        printf("0x%.2x", dec_gentab[x]);
        if (x==k-1)
            printf("};\n");
        else if (((x+1)%8)==0)
            printf(",\n    ");
        else
            printf(", ");
    }

    // test codec
    unsigned int r;     // received symbol
    unsigned int p_hat; // decoded symbol
    unsigned int i;
    unsigned int num_errors=0;
    for (p=0; p<n; p++) {
        // encode symbol
        x = enc_gentab[p];

        // test decoding with no errors
        p_hat = dec_gentab[x];
        num_errors += (p_hat != p);

        // add single bit error and decode
        for (i=0; i<7; i++) {
            // flip bit at index i
            r = x ^ (1 << i);

            // decode symbol
            p_hat = dec_gentab[r];

            // check for errors
            num_errors += (p_hat != p);
        }
    }
    printf("\n");
    printf("// number of decoding errors: %u\n", num_errors);

    return 0;
}

// count number of ones in an integer
unsigned int count_ones(unsigned int _x)
{
    unsigned int num_ones=0;
    unsigned int i;
    for (i=0; i<64; i++) {
        num_ones += _x & 0x01;
        _x >>= 1;
    }
    return num_ones;
}

unsigned int hamming74_enc_symbol(unsigned int _p)
{
    unsigned int x0 = count_ones(_p & HAMMING74_G0) % 2;
    unsigned int x1 = count_ones(_p & HAMMING74_G1) % 2;
    unsigned int x2 = count_ones(_p & HAMMING74_G2) % 2;
    unsigned int x3 = count_ones(_p & HAMMING74_G3) % 2;
    unsigned int x4 = count_ones(_p & HAMMING74_G4) % 2;
    unsigned int x5 = count_ones(_p & HAMMING74_G5) % 2;
    unsigned int x6 = count_ones(_p & HAMMING74_G6) % 2;

    unsigned int x = (x0 << 6) | (x1 << 5) | (x2 << 4) |
                     (x3 << 3) | (x4 << 2) | (x5 << 1) |
                     (x6     );

    return x;
}

unsigned int hamming74_dec_symbol(unsigned int _r)
{
    // compute syndromes
    unsigned int z0 = count_ones(_r & HAMMING74_H0) % 2;
    unsigned int z1 = count_ones(_r & HAMMING74_H1) % 2;
    unsigned int z2 = count_ones(_r & HAMMING74_H2) % 2;
    unsigned int z = (z0 << 0) | (z1 << 1) | (z2 << 2);

    unsigned int r_hat = _r;
    if (z) {
        // error detected: flip appropriate bit of
        // received symbol
        r_hat ^= 1 << (7-z);
    }

    // decode
    unsigned int p0 = count_ones(r_hat & HAMMING74_R0) % 2;
    unsigned int p1 = count_ones(r_hat & HAMMING74_R1) % 2;
    unsigned int p2 = count_ones(r_hat & HAMMING74_R2) % 2;
    unsigned int p3 = count_ones(r_hat & HAMMING74_R3) % 2;

    unsigned int p = (p0 << 3) | (p1 << 2) | (p2 << 1) | (p3 << 0);

    return p;
}

