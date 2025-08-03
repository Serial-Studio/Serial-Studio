//
// Hamming(12,8) example
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

int main(int argc, char*argv[])
{
    // original symbol
    unsigned int sym_org = 139;

    // encoded symbol
    unsigned int sym_enc = fec_hamming128_encode_symbol(sym_org);

    // received symbol
    unsigned int n=7;  // index of bit to corrupt
    unsigned int sym_rec = sym_enc ^ (1<<(12-n));

    // decoded symbol
    unsigned int sym_dec = fec_hamming128_decode_symbol(sym_rec);

    // print results
    printf("    sym org     :       "); print_bitstring(sym_org,  8); printf("\n");
    printf("    sym enc     :   ");     print_bitstring(sym_enc, 12); printf("\n");
    printf("    sym rec     :   ");     print_bitstring(sym_rec, 12); printf("\n");
    printf("    sym dec     :       "); print_bitstring(sym_dec,  8); printf("\n");

    // print number of bit errors
    printf("    bit errors  :   %u\n", count_bit_errors(sym_org, sym_dec));

    return 0;
}

