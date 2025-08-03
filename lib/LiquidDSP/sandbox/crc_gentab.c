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
// CRC table generator
//

#include <stdio.h>
#include <stdlib.h>

// generator polynomial
#define CRC32_POLY 0xEDB88320 // 0x04C11DB7

int main() {
    unsigned int crc;
    unsigned int mask;
    unsigned int crcpoly = CRC32_POLY;

    unsigned int crc_gentab[256];

    // generate table
    unsigned int i, j;
    for (i=0; i<256; i++) {
        crc = i;
        for (j=0; j<8; j++) {
            mask = -(crc & 0x01);
            crc = (crc >> 1) ^ (crcpoly & mask);
        }
        crc_gentab[i] = crc;
    }

    printf("\n");
    printf("unsigned char crc_gentab[%u] = {\n    ", 256);
    for (i=0; i<256; i++) {
        printf("0x%.2x", crc_gentab[i]);
        if (i==255)
            printf("};\n");
        else if (((i+1)%8)==0)
            printf(",\n    ");
        else
            printf(", ");
    }

    // test it...

    return 0;
}

