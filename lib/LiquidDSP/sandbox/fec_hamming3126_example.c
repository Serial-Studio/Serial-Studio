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
// Hamming code parity check matrix
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

//
// (12,8) Hamming code
//
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


#if 0
//
// (15,11) Hamming code
//
// parity bit coverage mask for encoder (collapsed version of figure
// above, stripping out parity bits P1, P2, P4, P8 and only including
// data bits 1:11)
//
//  parity bit  P1  x   x   .   x   x   .   x   .   x   .   x   = .110 1101 0101
//  coverage    P2  x   .   x   x   .   x   x   .   .   x   x   = .101 1011 0011 
//              P4  .   x   x   x   .   .   .   x   x   x   x   = .011 1000 1111
//              P8  .   .   .   .   x   x   x   x   x   x   x   = .000 0111 1111

#define HAMMING_M1   0x0d65     // .110 1101 0101
#define HAMMING_M2   0x05b3     // .101 1011 0011
#define HAMMING_M4   0x038f     // .011 1000 1111
#define HAMMING_M8   0x007f     // .000 0111 1111

// parity bit coverage mask for decoder; used to compute syndromes
// for decoding a received message (see first figure, above).
#define HAMMING_S1   0x5555  // .101 0101 0101 0101
#define HAMMING_S2   0x3333  // .011 0011 0011 0011
#define HAMMING_S4   0x0f0f  // .000 1111 0000 1111
#define HAMMING_S8   0x00ff  // .000 0000 1111 1111
#endif

//
// (31,26) Hamming code
//
// parity bit coverage mask for encoder (collapsed version of figure
// above, stripping out parity bits P1, P2, P4, P8, P16 and only including
// data bits 1:26)
//
//  bit position    3   5   6   7   8   9   10  11  12  13  14  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30
//                          *               *               *               *               *               *
//  parity bit  P1  x   x   .   x   x   .   x   .   x   .   x   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   = ..11 0110 1010 1101 0101 0101 0101
//  coverage    P2  x   .   x   x   .   x   x   .   .   x   x   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   = ..10 1101 1001 1011 0011 0011 0011
//              P4  .   x   x   x   .   .   .   x   x   x   x   .   .   .   x   x   x   x   .   .   .   .   x   x   x   x   = ..01 1100 0111 1000 1111 0000 1111
//              P8  .   .   .   .   x   x   x   x   x   x   x   .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   = ..00 0011 1111 1000 0000 1111 1111
//              P16 .   .   .   .   .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   = ..00 0000 0000 0111 1111 1111 1111

#define HAMMING_M1  0x036AD555  //  ..11 0110 1010 1101 0101 0101 0101
#define HAMMING_M2  0x02D9B333  //  ..10 1101 1001 1011 0011 0011 0011
#define HAMMING_M4  0x01C78F0F  //  ..01 1100 0111 1000 1111 0000 1111
#define HAMMING_M8  0x003F80FF  //  ..00 0011 1111 1000 0000 1111 1111
#define HAMMING_M16 0x00007FFF  //  ..00 0000 0000 0111 1111 1111 1111

//  bit position    1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31
//                              *               *               *               *               *               *               *
//  parity bit  P1  x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   .   x   = .101 0101 0101 0101 0101 0101 0101 0101
//  coverage    P2  .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   .   .   x   x   = .011 0011 0011 0011 0011 0011 0011 0011
//              P4  .   .   .   x   x   x   x   .   .   .   .   x   x   x   x   .   .   .   .   x   x   x   x   .   .   .   .   x   x   x   x   = .000 1111 0000 1111 0000 1111 0000 1111
//              P8  .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   .   .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   = .000 0000 1111 1111 0000 0000 1111 1111
//              P16 .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   = .000 0000 0000 0000 1111 1111 1111 1111
//
// parity bit coverage mask for decoder; used to compute syndromes
// for decoding a received message (see figure, above).
#define HAMMING_S1  0x55555555  //  .101 0101 0101 0101 0101 0101 0101 0101
#define HAMMING_S2  0x33333333  //  .011 0011 0011 0011 0011 0011 0011 0011
#define HAMMING_S4  0x0f0f0f0f  //  .000 1111 0000 1111 0000 1111 0000 1111
#define HAMMING_S8  0x00ff00ff  //  .000 0000 1111 1111 0000 0000 1111 1111
#define HAMMING_S16 0x0000ffff  //  .000 0000 0000 0000 1111 1111 1111 1111



unsigned int fec_hamming_encode_symbol(unsigned int _sym_dec)
{
    // validate input
    if (_sym_dec >= (1<<26)) {
        fprintf(stderr,"error, fec_hamming_encode(), input symbol too large\n");
        exit(1);
    }

    // compute parity bits
    unsigned int p1  = liquid_bdotprod_uint32(_sym_dec, HAMMING_M1);
    unsigned int p2  = liquid_bdotprod_uint32(_sym_dec, HAMMING_M2);
    unsigned int p4  = liquid_bdotprod_uint32(_sym_dec, HAMMING_M4);
    unsigned int p8  = liquid_bdotprod_uint32(_sym_dec, HAMMING_M8);
    unsigned int p16 = liquid_bdotprod_uint32(_sym_dec, HAMMING_M16);

#if DEBUG_FEC_HAMMING
    printf("parity bits (p1,p2,p4,p8,p16) : (%1u,%1u,%1u,%1u,%1u)\n", p1, p2, p4, p8, p16);
#endif

    // encode symbol by inserting parity bits with data bits to
    // make a 31-bit symbol
    unsigned int sym_enc = ((_sym_dec & 0x00007fff) << 0) | //  ..00 0000 0000 0111 1111 1111 1111
                           ((_sym_dec & 0x003F8000) << 1) | //  ..00 0011 1111 1000 0000 0000 0000
                           ((_sym_dec & 0x01C00000) << 2) | //  ..01 1100 0000 0000 0000 0000 0000
                           ((_sym_dec & 0x02000000) << 3) | //  ..10 0000 0000 0000 0000 0000 0000
                           ( p1  << 30 ) |  // 30 = 31 - 1  (position of P1)
                           ( p2  << 29 ) |  // 29 = 31 - 2  (position of P2)
                           ( p4  << 27 ) |  // 27 = 31 - 4  (position of P4)
                           ( p8  << 23 ) |  // 23 = 31 - 8  (position of P8)
                           ( p16 << 15 );   // 15 = 31 - 16 (position of P16)

    return sym_enc;
}

unsigned int fec_hamming_decode_symbol(unsigned int _sym_enc)
{
    // validate input
    if (_sym_enc >= (1<<31)) {
        fprintf(stderr,"error, fec_hamming_decode(), input symbol too large\n");
        exit(1);
    }

    // compute syndrome bits
    unsigned int s1  = liquid_bdotprod_uint32(_sym_enc, HAMMING_S1);
    unsigned int s2  = liquid_bdotprod_uint32(_sym_enc, HAMMING_S2);
    unsigned int s4  = liquid_bdotprod_uint32(_sym_enc, HAMMING_S4);
    unsigned int s8  = liquid_bdotprod_uint32(_sym_enc, HAMMING_S8);
    unsigned int s16 = liquid_bdotprod_uint32(_sym_enc, HAMMING_S16);

    // index
    unsigned int z = (s16<<4) | (s8<<3) | (s4<<2) | (s2<<1) | s1;

#if DEBUG_FEC_HAMMING
    printf("syndrome bits (s1,s2,s4,s8,16) : (%1u,%1u,%1u,%1u,%1u)\n", s1, s2, s4, s8, s16);
    printf("syndrome z : %u\n", z);
#endif

    // flip bit at this position
    if (z) {
        if (z > 31) {
            // if this happens there are likely too many errors
            // to correct; just pass without trying to do anything
            fprintf(stderr,"warning, fec_hamming_decode_symbol(), syndrome index exceeds maximum\n");
        } else {
            //printf("error detected!\n");
            _sym_enc ^= 1 << (31-z);
        }
    }

    // strip data bits from encoded symbol with parity bits
    unsigned int sym_dec = ((_sym_enc & 0x00007fff)     )   |   //  .000 0000 0000 0000 0111 1111 1111 1111
                           ((_sym_enc & 0x007f0000) >> 1)   |   //  .000 0000 0111 1111 0000 0000 0000 0000
                           ((_sym_enc & 0x07000000) >> 2)   |   //  .000 0111 0000 0000 0000 0000 0000 0000
                           ((_sym_enc & 0x10000000) >> 3);      //  .001 0000 0000 0000 0000 0000 0000 0000
    return sym_dec;
}

//
// Hamming(31,26) example
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
    unsigned int sym_org = 28236851;    // ..01 1010 1110 1101 1100 0011 0011

    // encoded symbol
    unsigned int sym_enc = fec_hamming_encode_symbol(sym_org);

    // received symbol
    unsigned int n=7;  // index of bit to corrupt
    unsigned int sym_rec = sym_enc ^ (1<<(31-n));

    // decoded symbol
    unsigned int sym_dec = fec_hamming_decode_symbol(sym_rec);

    // print results
    printf("    sym org     :        "); print_bitstring(sym_org, 26); printf("\n");
    printf("    sym enc     :   ");      print_bitstring(sym_enc, 31); printf("\n");
    printf("    sym rec     :   ");      print_bitstring(sym_rec, 31); printf("\n");
    printf("    sym dec     :        "); print_bitstring(sym_dec, 26); printf("\n");

    // print number of bit errors
    printf("    bit errors  :   %u\n", count_bit_errors(sym_org, sym_dec));

    return 0;
}

