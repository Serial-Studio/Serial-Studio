//
// repack_bytes_example.c
//
// This example demonstrates the repack_bytes() interface by packing a
// sequence of three 3-bit symbols into five 2-bit symbols.  The results
// are printed to the screen.  Because the total number of bits in the
// input is 9 and not evenly divisible by 2, the last of the 5 output
// symbols has a zero explicitly padded to the end.
//

#include <stdio.h>

#include "liquid.h"

// print symbol to screen, one bit at a time
void print_symbol(unsigned char _sym,
                  unsigned int _bits_per_symbol)
{
    unsigned int i;
    unsigned int n; // shift amount
    for (i=0; i<_bits_per_symbol; i++) {
        n = _bits_per_symbol - i - 1;
        printf("%c", (_sym >> n) & 0x01 ? '1' : '0');
    }
}

// print symbol array to screen
void print_symbol_array(unsigned char * _sym,
                        unsigned int _bits_per_symbol,
                        unsigned int _num_symbols)
{
    unsigned int i;
    for (i=0; i<_num_symbols; i++) {
        print_symbol(_sym[i], _bits_per_symbol);
        printf("%c", i < _num_symbols-1 ? ',' : '\n');
    }
}

int main() {
    // input symbols:   111 000 111
    // expected output: 11 10 00 11 1(0)
    unsigned char input[3] = {
        0x07,   // 111
        0x00,   // 000
        0x07    // 111(0)
    };
    
    // allocate memory for output array
    unsigned char output[5];
    unsigned int N;

    // print input symbol array
    printf("input symbols:  ");
    print_symbol_array(input,3,3);

    // repack bytes into output array
    liquid_repack_bytes( input, 3, 3, output, 2, 5, &N );

    // print output array
    printf("output symbols: ");
    print_symbol_array(output,2,5);

    return 0;
}

