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
// generate tables for counting ones in a byte
//

#include <stdio.h>
#include <stdlib.h>

// slow implementation counting ones in a byte
unsigned int count_ones(unsigned char _x)
{
    unsigned int n=0;
    unsigned int i;
    for (i=0; i<8; i++)
        n += (_x >> i) & 0x01;
    
    return n;
}

int main()
{
    unsigned int i;

    printf("// auto-generated file (do not edit)\n");
    printf("\n");
    printf("// number of ones in a byte\n");
    printf("unsigned const char liquid_c_ones[256] = {\n    ");
    for (i=0; i<256; i++) {
        // 
        unsigned int n = count_ones((unsigned char)i);

        // print results
        printf("%1u", n);

        if ( i != 255 ) {
            printf(",");
            if ( ((i+1)%16)==0 )
                printf("\n    ");
            else
                printf(" ");
        }

    }
    printf("};\n\n");

    // do the same modulo 2
    printf("// number of ones in a byte modulo 2\n");
    printf("unsigned const char liquid_c_ones_mod2[256] = {\n    ");
    for (i=0; i<256; i++) {
        // 
        unsigned int n = count_ones((unsigned char)i);

        // print results
        printf("%1u", n % 2);

        if ( i != 255 ) {
            printf(",");
            if ( ((i+1)%16)==0 )
                printf("\n    ");
            else
                printf(" ");
        }

    }
    printf("};\n\n");

    
    return 0;
}
