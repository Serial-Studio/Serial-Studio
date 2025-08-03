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
// generate table for byte reversal
//

#include <stdio.h>
#include <stdlib.h>

// slow implementation of byte reversal
unsigned char reverse_byte(unsigned char _x)
{
    unsigned char y = 0x00;
    unsigned int i;
    for (i=0; i<8; i++) {
        y <<= 1;
        y |= _x & 1;
        _x >>= 1;
    }
    return y;
}


int main()
{
    printf("// auto-generated file (do not edit)\n");
    printf("\n");
    printf("// reverse byte table\n");
    printf("unsigned const char liquid_reverse_byte[256] = {\n    ");
    unsigned int i;
    for (i=0; i<256; i++) {
        // reverse byte
        unsigned char byte_rev = reverse_byte((unsigned char)i);

        // print results
        printf("0x%.2x", byte_rev);

        if ( i != 255 ) {
            printf(",");
            if ( ((i+1)%8)==0 )
                printf("\n    ");
            else
                printf(" ");
        }

    }
    printf("};\n\n");
    
    return 0;
}
