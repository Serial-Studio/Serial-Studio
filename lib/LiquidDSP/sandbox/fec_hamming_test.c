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
//  Hamming code parity check matrix
//
//  bit position    1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38
//  encoded bits    P1  P2  1   P4  2   3   4   P8  5   6   7   8   9   10  11  P16 12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  P32 27  28  29  30  31  32
//
//  parity bit  P1  x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .
//  coverage    P2  .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .
//              P4  .   .   .   x   x   x   x   .   .   .   .   x   x   x   x   .   .   .   .   x   x   x   x   .   .   .   .   x   x   x   x   .
//              P8  .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   .   .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   .
//              P16 .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   .
//              P32 .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   x

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FEC_HAMMING 1

#if 0

// parity bit coverage mask for encoder (collapsed version of figure
// above, stripping out parity bits P1, P2, P4, P8 and only including
// data bits 1:8)
//
// bit position     3   5   6   7   9   10  11  12
//
//  parity bit  P1  x   x   .   x   x   .   x   .   =   1101 1010
//  coverage    P2  x   .   x   x   .   x   x   .   =   1011 0110
//              P4  .   x   x   x   .   .   .   x   =   0111 0001
//              P8  .   .   .   .   x   x   x   x   =   0000 1111
#define HAMMING_M1   0xda    // 1101 1010
#define HAMMING_M2   0xb6    // 1011 0110
#define HAMMING_M4   0x71    // 0111 0001
#define HAMMING_M8   0x0f    // 0000 1111

// parity bit coverage mask for decoder; used to compute syndromes
// for decoding a received message (see first figure, above).
#define HAMMING_S1   0x0aaa  // .... 1010 1010 1010
#define HAMMING_S2   0x0666  // .... 0110 0110 0110
#define HAMMING_S4   0x01e1  // .... 0001 1110 0001
#define HAMMING_S8   0x001f  // .... 0000 0001 1111

#endif


//
// Hamming(15,11)
//
// parity bit coverage mask for encoder (collapsed version of figure
// above, stripping out parity bits P1, P2, P4, P8 and only including
// data bits 1:11)
//
//  parity bit  P1  x   x   .   x   x   .   x   .   x   .   x   = .110 1101 0101
//  coverage    P2  x   .   x   x   .   x   x   .   .   x   x   = .101 1011 0011 
//              P4  .   x   x   x   .   .   .   x   x   x   x   = .011 1000 1111
//              P8  .   .   .   .   x   x   x   x   x   x   x   = .000 0111 1111

#define HAMMING_M1   0x06d5     // .110 1101 0101
#define HAMMING_M2   0x05b3     // .101 1011 0011
#define HAMMING_M4   0x038f     // .011 1000 1111
#define HAMMING_M8   0x007f     // .000 0111 1111

// parity bit coverage mask for decoder; used to compute syndromes
// for decoding a received message (see first figure, above).
#define HAMMING_S1   0x5555  // .101 0101 0101 0101
#define HAMMING_S2   0x3333  // .011 0011 0011 0011
#define HAMMING_S4   0x0f0f  // .000 1111 0000 1111
#define HAMMING_S8   0x00ff  // .000 0000 1111 1111

unsigned int fec_hamming_encode_symbol(unsigned int _sym_dec)
{
    // validate input
    if (_sym_dec >= (1<<11)) {
        fprintf(stderr,"error, fec_hamming_encode(), input symbol too large\n");
        exit(1);
    }

    // compute parity bits
    unsigned int p1 = liquid_bdotprod_uint16(_sym_dec, HAMMING_M1);
    unsigned int p2 = liquid_bdotprod_uint16(_sym_dec, HAMMING_M2);
    unsigned int p4 = liquid_bdotprod_uint16(_sym_dec, HAMMING_M4);
    unsigned int p8 = liquid_bdotprod_uint16(_sym_dec, HAMMING_M8);

#if DEBUG_FEC_HAMMING
    printf("parity bits (p1,p2,p4,p8) : (%1u,%1u,%1u,%1u)\n", p1, p2, p4, p8);
#endif

    // encode symbol by inserting parity bits with data bits to
    // make a 15-bit symbol
    unsigned int sym_enc = ((_sym_dec & 0x007f) << 0) |
                           ((_sym_dec & 0x0380) << 1) |
                           ((_sym_dec & 0x0400) << 2) |
                           ( p1 << 14 ) |
                           ( p2 << 13 ) |
                           ( p4 << 11 ) |
                           ( p8 << 7  );

    return sym_enc;
}

unsigned int fec_hamming_decode_symbol(unsigned int _sym_enc)
{
    // validate input
    if (_sym_enc >= (1<<15)) {
        fprintf(stderr,"error, fec_hamming_decode(), input symbol too large\n");
        exit(1);
    }

    // compute syndrome bits
    unsigned int s1 = liquid_bdotprod_uint16(_sym_enc, HAMMING_S1);
    unsigned int s2 = liquid_bdotprod_uint16(_sym_enc, HAMMING_S2);
    unsigned int s4 = liquid_bdotprod_uint16(_sym_enc, HAMMING_S4);
    unsigned int s8 = liquid_bdotprod_uint16(_sym_enc, HAMMING_S8);

    // index
    unsigned int z = (s8<<3) | (s4<<2) | (s2<<1) | s1;

#if DEBUG_FEC_HAMMING
    printf("syndrome bits (s1,s2,s4,s8) : (%1u,%1u,%1u,%1u)\n", s1, s2, s4, s8);
    printf("syndrome z : %u\n", z);
#endif

    // flip bit at this position
    if (z) {
        if (z > 15) {
            // if this happens there are likely too many errors
            // to correct; just pass without trying to do anything
            fprintf(stderr,"warning, fec_hamming_decode_symbol(), syndrome index exceeds maximum\n");
        } else {
            //printf("error detected!\n");
            _sym_enc ^= 1 << (15-z);
        }
    }

    // strip data bits (x) from encoded symbol with parity bits (.)
    //      symbol:  [..x. xxx. xxxx]
    //                0000 0000 1111     >  0x000f
    //                0000 1110 0000     >  0x00e0
    //                0010 0000 0000     >  0x0200
    //
    //      symbol: [ -..x .xxx .xxx xxxx]
    //                -000 0000 0xxx xxxx   > 0x007f
    //                -000 0111 0000 0000   > 0x0700
    //                -001 0000 0000 0000   > 0x1000
    unsigned int sym_dec = ((_sym_enc & 0x007f)     )   |
                           ((_sym_enc & 0x0700) >> 1)   |
                           ((_sym_enc & 0x1000) >> 2);


    return sym_dec;
}

//
// Hamming(12,8) example
//

void print_bitstring(unsigned int _x,
                     unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        printf("%1u", (_x >> (_n-i-1)) & 1);
}

int main(int argc, char*argv[])
{
    // original symbol
    unsigned int sym_org = 1377;    // 10101100001

    // encoded symbol
    unsigned int sym_enc = fec_hamming_encode_symbol(sym_org);

    // received symbol
    unsigned int n=7;  // index of bit to corrupt
    unsigned int sym_rec = sym_enc ^ (1<<(15-n));

    // decoded symbol
    unsigned int sym_dec = fec_hamming_decode_symbol(sym_rec);

    // print results
    printf("    sym org     :       "); print_bitstring(sym_org, 11); printf("\n");
    printf("    sym enc     :   ");     print_bitstring(sym_enc, 15); printf("\n");
    printf("    sym rec     :   ");     print_bitstring(sym_rec, 15); printf("\n");
    printf("    sym dec     :       "); print_bitstring(sym_dec, 11); printf("\n");

    // print number of bit errors
    printf("    bit errors  :   %u\n", count_bit_errors(sym_org, sym_dec));

    return 0;
}

