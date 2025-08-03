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

#include <string.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"


//
// AUTOTEST : lbshift
//
void autotest_lbshift() {
    // input        : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 0000 0011 1101 1110 1011 1111 0101 0100
    // output [2]   : 0000 0111 1011 1101 0111 1110 1010 1000
    // output [3]   : 0000 1111 0111 1010 1111 1101 0101 0000
    // output [4]   : 0001 1110 1111 0101 1111 1010 1010 0000
    // output [5]   : 0011 1101 1110 1011 1111 0101 0100 0000
    // output [6]   : 0111 1011 1101 0111 1110 1010 1000 0000
    // output [7]   : 1111 0111 1010 1111 1101 0101 0000 0000
    unsigned char input[4] = {0x81, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x81, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0x03, 0xDE, 0xBF, 0x54};
    unsigned char output_test_2[4] = {0x07, 0xBD, 0x7E, 0xA8};
    unsigned char output_test_3[4] = {0x0F, 0x7A, 0xFD, 0x50};
    unsigned char output_test_4[4] = {0x1E, 0xF5, 0xFA, 0xA0};
    unsigned char output_test_5[4] = {0x3D, 0xEB, 0xF5, 0x40};
    unsigned char output_test_6[4] = {0x7B, 0xD7, 0xEA, 0x80};
    unsigned char output_test_7[4] = {0xF7, 0xAF, 0xD5, 0x00};

    unsigned char output[4];
    
    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<8; i++) {
        memmove(output, input, 4);
        liquid_lbshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        case 5: CONTEND_SAME_DATA( output, output_test_5, 4 ); break;
        case 6: CONTEND_SAME_DATA( output, output_test_6, 4 ); break;
        case 7: CONTEND_SAME_DATA( output, output_test_7, 4 ); break;
        default:;
        }
    }
}


//
// AUTOTEST : rbshift
//
void autotest_rbshift() {
    // input        : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 0100 0000 1111 0111 1010 1111 1101 0101
    // output [2]   : 0010 0000 0111 1011 1101 0111 1110 1010
    // output [3]   : 0001 0000 0011 1101 1110 1011 1111 0101
    // output [4]   : 0000 1000 0001 1110 1111 0101 1111 1010
    // output [5]   : 0000 0100 0000 1111 0111 1010 1111 1101
    // output [6]   : 0000 0010 0000 0111 1011 1101 0111 1110
    // output [7]   : 0000 0001 0000 0011 1101 1110 1011 1111
    unsigned char input[4] = {0x81, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x81, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0x40, 0xF7, 0xAF, 0xD5};
    unsigned char output_test_2[4] = {0x20, 0x7B, 0xD7, 0xEA};
    unsigned char output_test_3[4] = {0x10, 0x3D, 0xEB, 0xF5};
    unsigned char output_test_4[4] = {0x08, 0x1E, 0xF5, 0xFA};
    unsigned char output_test_5[4] = {0x04, 0x0F, 0x7A, 0xFD};
    unsigned char output_test_6[4] = {0x02, 0x07, 0xBD, 0x7E};
    unsigned char output_test_7[4] = {0x01, 0x03, 0xDE, 0xBF};

    unsigned char output[4];
    
    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<8; i++) {
        memmove(output, input, 4);
        liquid_rbshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        case 5: CONTEND_SAME_DATA( output, output_test_5, 4 ); break;
        case 6: CONTEND_SAME_DATA( output, output_test_6, 4 ); break;
        case 7: CONTEND_SAME_DATA( output, output_test_7, 4 ); break;
        default:;
        }
    }
}


//
// AUTOTEST : lbcircshift
//
void autotest_lbcircshift() {
    // input        : 1001 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1001 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 0010 0011 1101 1110 1011 1111 0101 0101
    // output [2]   : 0100 0111 1011 1101 0111 1110 1010 1010
    // output [3]   : 1000 1111 0111 1010 1111 1101 0101 0100
    // output [4]   : 0001 1110 1111 0101 1111 1010 1010 1001
    // output [5]   : 0011 1101 1110 1011 1111 0101 0101 0010
    // output [6]   : 0111 1011 1101 0111 1110 1010 1010 0100
    // output [7]   : 1111 0111 1010 1111 1101 0101 0100 1000
    unsigned char input[4] = {0x91, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x91, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0x23, 0xDE, 0xBF, 0x55};
    unsigned char output_test_2[4] = {0x47, 0xBD, 0x7E, 0xAA};
    unsigned char output_test_3[4] = {0x8F, 0x7A, 0xFD, 0x54};
    unsigned char output_test_4[4] = {0x1E, 0xF5, 0xFA, 0xA9};
    unsigned char output_test_5[4] = {0x3D, 0xEB, 0xF5, 0x52};
    unsigned char output_test_6[4] = {0x7B, 0xD7, 0xEA, 0xA4};
    unsigned char output_test_7[4] = {0xF7, 0xAF, 0xD5, 0x48};

    unsigned char output[4];
    
    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<8; i++) {
        memmove(output, input, 4);
        liquid_lbcircshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        case 5: CONTEND_SAME_DATA( output, output_test_5, 4 ); break;
        case 6: CONTEND_SAME_DATA( output, output_test_6, 4 ); break;
        case 7: CONTEND_SAME_DATA( output, output_test_7, 4 ); break;
        default:;
        }
    }
}


//
// AUTOTEST : rbcircshift
//
void autotest_rbcircshift() {
    // input        : 1001 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1001 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 0100 1000 1111 0111 1010 1111 1101 0101
    // output [2]   : 1010 0100 0111 1011 1101 0111 1110 1010
    // output [3]   : 0101 0010 0011 1101 1110 1011 1111 0101
    // output [4]   : 1010 1001 0001 1110 1111 0101 1111 1010
    // output [5]   : 0101 0100 1000 1111 0111 1010 1111 1101
    // output [6]   : 1010 1010 0100 0111 1011 1101 0111 1110
    // output [7]   : 0101 0101 0010 0011 1101 1110 1011 1111
    unsigned char input[4] = {0x91, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x91, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0x48, 0xF7, 0xAF, 0xD5};
    unsigned char output_test_2[4] = {0xA4, 0x7B, 0xD7, 0xEA};
    unsigned char output_test_3[4] = {0x52, 0x3D, 0xEB, 0xF5};
    unsigned char output_test_4[4] = {0xA9, 0x1E, 0xF5, 0xFA};
    unsigned char output_test_5[4] = {0x54, 0x8F, 0x7A, 0xFD};
    unsigned char output_test_6[4] = {0xAA, 0x47, 0xBD, 0x7E};
    unsigned char output_test_7[4] = {0x55, 0x23, 0xDE, 0xBF};

    unsigned char output[4];
    
    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<8; i++) {
        memmove(output, input, 4);
        liquid_rbcircshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        case 5: CONTEND_SAME_DATA( output, output_test_5, 4 ); break;
        case 6: CONTEND_SAME_DATA( output, output_test_6, 4 ); break;
        case 7: CONTEND_SAME_DATA( output, output_test_7, 4 ); break;
        default:;
        }
    }
}

