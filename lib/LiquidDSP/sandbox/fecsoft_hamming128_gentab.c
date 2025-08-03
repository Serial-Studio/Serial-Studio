//
// fecsoft_hamming128_gentab.c
//
// This script generates the nearest neighbors to each symbol
// for faster decoding.
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

int main()
{
    // find the 17 symbols with a hamming distance of exactly 3

    unsigned int sym_org;   // original symbol
    unsigned int sym_enc;   // encoded symbol

    unsigned int n3;        // counter for symbols with a Hamming distance 3
    unsigned char sym3[17]; // ...
    unsigned int i;

    printf("unsigned char fecsoft_hamming128_neighbors[256][17] = {\n");
    for (sym_org=0; sym_org<256; sym_org++) {

        // 
        printf("    {");

        // reset counter
        n3=0;

        // encode symbol
        sym_enc = fec_hamming128_encode_symbol(sym_org);

        // look at all possible symbols...
        for (i=0; i<256; i++) {
            unsigned int sym_test = fec_hamming128_encode_symbol(i);

            // compute number of bit differences...
            unsigned int d = count_bit_errors(sym_enc, sym_test);

            //
            if (d==3) {
                if (n3 == 17) {
                    fprintf(stderr,"error: expected no more than 17 symbols of distance 3\n");
                    exit(1);
                }
                sym3[n3++] = i;
            }

            // print results...
            //printf("  %3u : %3u\n", i, d);
        }

        //printf("  %3u : n3 =%3u\n", sym_org, n3);
        if (n3 != 17) {
            fprintf(stderr,"error: expected exactly 17 symbols of distance 3\n");
            exit(1);
        }

        // print line
        for (i=0; i<17; i++) {
            printf("0x%.2x", sym3[i]);
            if (i != 16)
                printf(", ");
        }
        printf("}");
        if (sym_org != 255)
            printf(",\n");
        else
            printf("};\n");
    }

    return 0;
}

