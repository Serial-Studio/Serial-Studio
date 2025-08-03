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
// 1/2-rate (8,4) Hamming code table generator
//

#include <stdio.h>
#include <stdlib.h>

// generator matrices

// encoding matrix
#define HAMMING84_G0    0x0d    // 1101
#define HAMMING84_G1    0x0b    // 1011
#define HAMMING84_G2    0x08    // 1000
#define HAMMING84_G3    0x07    // 0111
#define HAMMING84_G4    0x04    // 0100
#define HAMMING84_G5    0x02    // 0010
#define HAMMING84_G6    0x01    // 0001
#define HAMMING84_G7    0x0e    // 1110

// syndrome matrix
#define HAMMING84_H0    0xaa    // 1010 1010
#define HAMMING84_H1    0x66    // 0110 0110
#define HAMMING84_H2    0x1e    // 0001 1110
#define HAMMING84_H3    0xff    // 1111 1111

// decoding matrix
#define HAMMING84_R0    0x20    // 0010 0000
#define HAMMING84_R1    0x08    // 0000 1000
#define HAMMING84_R2    0x04    // 0000 0100
#define HAMMING84_R3    0x02    // 0000 0010

unsigned int count_ones(unsigned int _x);
unsigned int hamming84_enc_symbol(unsigned int _p);
unsigned int hamming84_dec_symbol(unsigned int _x);

int main() {
    unsigned int n = 16;
    unsigned int k = 256;

    // generate encoding table
    unsigned int enc_gentab[n];
    unsigned int p;
    for (p=0; p<n; p++)
        enc_gentab[p] = hamming84_enc_symbol(p);

    // generate decoding table
    unsigned int dec_gentab[k];
    unsigned int x;
    for (x=0; x<k; x++)
        dec_gentab[x] = hamming84_dec_symbol(x);

    printf("// Hamming(8,4) table generator\n");
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
        for (i=0; i<8; i++) {
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

unsigned int hamming84_enc_symbol(unsigned int _p)
{
    unsigned int x0 = count_ones(_p & HAMMING84_G0) % 2;
    unsigned int x1 = count_ones(_p & HAMMING84_G1) % 2;
    unsigned int x2 = count_ones(_p & HAMMING84_G2) % 2;
    unsigned int x3 = count_ones(_p & HAMMING84_G3) % 2;
    unsigned int x4 = count_ones(_p & HAMMING84_G4) % 2;
    unsigned int x5 = count_ones(_p & HAMMING84_G5) % 2;
    unsigned int x6 = count_ones(_p & HAMMING84_G6) % 2;
    unsigned int x7 = count_ones(_p & HAMMING84_G7) % 2;

    unsigned int x = (x0 << 7) | (x1 << 6) | (x2 << 5) |
                     (x3 << 4) | (x4 << 3) | (x5 << 2) |
                     (x6 << 1) | (x7     );

    return x;
}

unsigned int hamming84_dec_symbol(unsigned int _r)
{
    // compute syndromes
    unsigned int z0 = count_ones(_r & HAMMING84_H0) % 2;
    unsigned int z1 = count_ones(_r & HAMMING84_H1) % 2;
    unsigned int z2 = count_ones(_r & HAMMING84_H2) % 2;
    unsigned int z = (z0 << 0) | (z1 << 1) | (z2 << 2);

    unsigned int r_hat = _r;
    if (z) {
        // error detected: flip appropriate bit of
        // received symbol
        r_hat ^= 1 << (8-z);
    }

#if 0
    // check extra parity bit
    unsigned int z3 = count_ones(r_hat & HAMMING84_H3) % 2;
    if (z3) printf("decoding error!\n");
#endif

    // decode
    unsigned int p0 = count_ones(r_hat & HAMMING84_R0) % 2;
    unsigned int p1 = count_ones(r_hat & HAMMING84_R1) % 2;
    unsigned int p2 = count_ones(r_hat & HAMMING84_R2) % 2;
    unsigned int p3 = count_ones(r_hat & HAMMING84_R3) % 2;

    unsigned int p = (p0 << 3) | (p1 << 2) | (p2 << 1) | (p3 << 0);

    return p;
}

