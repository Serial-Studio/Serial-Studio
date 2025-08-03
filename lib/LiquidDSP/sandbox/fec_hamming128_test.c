//
// Hamming(12,8) code test
//
// Tests syndrome decoding
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

void print_bitstring(unsigned int _x,
                     unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        printf("%1u", (_x >> (_n-i-1)) & 1);
}

unsigned char hamming128_P[4] = {
        0xda,   // 1101 1010
        0xb6,   // 1011 0110
        0x71,   // 0111 0001
        0x0f};  // 0000 1111

int main(int argc, char*argv[])
{
    // ...
    unsigned int sym_org = 0x21;    // original symbol
    unsigned int sym_enc;           // encoded symbol
    unsigned int e = 0x001;         // error vector
    unsigned int sym_rec;           // received symbol
    unsigned int sym_dec;           // decoded symbol
    
    unsigned int i;

    // encode symbol
    unsigned int p0 = liquid_c_ones[sym_org & hamming128_P[0]] & 0x01;
    unsigned int p1 = liquid_c_ones[sym_org & hamming128_P[1]] & 0x01;
    unsigned int p2 = liquid_c_ones[sym_org & hamming128_P[2]] & 0x01;
    unsigned int p3 = liquid_c_ones[sym_org & hamming128_P[3]] & 0x01;
    unsigned int parity = (p0 << 3) | (p1 << 2) | (p2 << 1) | (p3 << 0);
    sym_enc = (parity << 8) | sym_org;

    // received symbol (add error)
    sym_rec = sym_enc ^ e;

    // 
    // decode symbol
    //
    sym_dec = sym_rec & 0xff;
    unsigned int e_hat = 0x000;
    // compute syndrome vector; s = r*H^T = ( H*r^T )^T
    unsigned int s = 0x0;   // 4 bits resolution
    for (i=0; i<4; i++) {
        s <<= 1;

        unsigned int p =
            ( (sym_rec & (1<<(12-i-1))) ? 1 : 0 ) +
            liquid_c_ones[sym_rec & hamming128_P[i]];

        s |= p & 0x01;
    }
    // compute weight of s
    unsigned int ws = liquid_count_ones(s);
    printf("    syndrome    :           "); print_bitstring(s,  4); printf(" (%u)\n", ws);

    if (ws == 0) {
        printf("no errors detected\n");
    } else {
        // estimate error location
        unsigned int e_test = 0x001;
        int syndrome_match = 0;

        // TODO : these can be pre-computed
        unsigned int n;
        for (n=0; n<12; n++) {
            // compute syndrome
            unsigned int s_hat = 0;

            for (i=0; i<4; i++) {
                s_hat <<= 1;

                unsigned int p =
                    ( (e_test & (1<<(12-i-1))) ? 1 : 0 ) +
                    liquid_c_ones[e_test & hamming128_P[i]];

                s_hat |= p & 0x01;
            }

            // print results
            //printf("e_test:"); print_bitstring(e_test, 72);
            printf("    %2u e = ", n);
            print_bitstring(e_test,12);
            printf(" s = ");
            print_bitstring(s_hat,4);
            if (s == s_hat) printf(" *");
            printf("\n");

            if (s == s_hat) {
                e_hat = e_test;
                syndrome_match = 1;
            }

            // shift e_test
            e_test <<= 1;
        }

        // apply error correction and return
        if (syndrome_match)
            sym_dec = (sym_rec ^ e_hat) & 0xff;
    }


    // print results
    printf("    sym org     :       "); print_bitstring(sym_org,  8); printf("\n");
    printf("    sym enc     :   ");     print_bitstring(sym_enc, 12); printf("\n");
    printf("    sym rec     :   ");     print_bitstring(sym_rec, 12); printf("\n");
    printf("    sym dec     :       "); print_bitstring(sym_dec,  8); printf("\n");

    // print number of bit errors
    printf("    bit errors  :   %u\n", count_bit_errors(sym_org, sym_dec));

    return 0;
}


