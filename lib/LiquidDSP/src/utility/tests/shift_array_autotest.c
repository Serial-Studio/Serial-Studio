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
// AUTOTEST : lshift
//
void autotest_lshift() {
    // input        : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 1110 1111 0101 1111 1010 1010 0000 0000
    // output [2]   : 0101 1111 1010 1010 0000 0000 0000 0000
    // output [3]   : 1010 1010 0000 0000 0000 0000 0000 0000
    // output [4]   : 0000 0000 0000 0000 0000 0000 0000 0000
    unsigned char input[4] = {0x81, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x81, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0xEF, 0x5F, 0xAA, 0x00};
    unsigned char output_test_2[4] = {0x5F, 0xAA, 0x00, 0x00};
    unsigned char output_test_3[4] = {0xAA, 0x00, 0x00, 0x00};
    unsigned char output_test_4[4] = {0x00, 0x00, 0x00, 0x00};

    unsigned char output[4];

    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<5; i++) {
        memmove(output, input, 4);
        liquid_lshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        default:;
        }
    }
}


//
// AUTOTEST : rshift
//
void autotest_rshift() {
    // input        : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 0000 0000 1000 0001 1110 1111 0101 1111
    // output [2]   : 0000 0000 0000 0000 1000 0001 1110 1111
    // output [3]   : 0000 0000 0000 0000 0000 0000 1000 0001
    // output [4]   : 0000 0000 0000 0000 0000 0000 0000 0000
    unsigned char input[4] = {0x81, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x81, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0x00, 0x81, 0xEF, 0x5F};
    unsigned char output_test_2[4] = {0x00, 0x00, 0x81, 0xEF};
    unsigned char output_test_3[4] = {0x00, 0x00, 0x00, 0x81};
    unsigned char output_test_4[4] = {0x00, 0x00, 0x00, 0x00};

    unsigned char output[4];

    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<5; i++) {
        memmove(output, input, 4);
        liquid_rshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        default:;
        }
    }
}


//
// AUTOTEST : lcircshift
//
void autotest_lcircshift() {
    // input        : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 1110 1111 0101 1111 1010 1010 1000 0001
    // output [2]   : 0101 1111 1010 1010 1000 0001 1110 1111
    // output [3]   : 1010 1010 1000 0001 1110 1111 0101 1111
    // output [4]   : 1000 0001 1110 1111 0101 1111 1010 1010
    unsigned char input[4] = {0x81, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x81, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0xEF, 0x5F, 0xAA, 0x81};
    unsigned char output_test_2[4] = {0x5F, 0xAA, 0x81, 0xEF};
    unsigned char output_test_3[4] = {0xAA, 0x81, 0xEF, 0x5F};
    unsigned char output_test_4[4] = {0x81, 0xEF, 0x5F, 0xAA};

    unsigned char output[4];

    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<5; i++) {
        memmove(output, input, 4);
        liquid_lcircshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        default:;
        }
    }
}


//
// AUTOTEST : rcircshift
//
void autotest_rcircshift() {
    // input        : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [0]   : 1000 0001 1110 1111 0101 1111 1010 1010
    // output [1]   : 1010 1010 1000 0001 1110 1111 0101 1111
    // output [2]   : 0101 1111 1010 1010 1000 0001 1110 1111
    // output [3]   : 1110 1111 0101 1111 1010 1010 1000 0001
    // output [4]   : 1000 0001 1110 1111 0101 1111 1010 1010
    unsigned char input[4] = {0x81, 0xEF, 0x5F, 0xAA};
    
    unsigned char output_test_0[4] = {0x81, 0xEF, 0x5F, 0xAA};
    unsigned char output_test_1[4] = {0xAA, 0x81, 0xEF, 0x5F};
    unsigned char output_test_2[4] = {0x5F, 0xAA, 0x81, 0xEF};
    unsigned char output_test_3[4] = {0xEF, 0x5F, 0xAA, 0x81};
    unsigned char output_test_4[4] = {0x81, 0xEF, 0x5F, 0xAA};

    unsigned char output[4];

    // 
    // run tests
    //
    unsigned int i;
    for (i=0; i<5; i++) {
        memmove(output, input, 4);
        liquid_rcircshift( output, 4, i);
        switch (i) {
        case 0: CONTEND_SAME_DATA( output, output_test_0, 4 ); break;
        case 1: CONTEND_SAME_DATA( output, output_test_1, 4 ); break;
        case 2: CONTEND_SAME_DATA( output, output_test_2, 4 ); break;
        case 3: CONTEND_SAME_DATA( output, output_test_3, 4 ); break;
        case 4: CONTEND_SAME_DATA( output, output_test_4, 4 ); break;
        default:;
        }
    }
}

