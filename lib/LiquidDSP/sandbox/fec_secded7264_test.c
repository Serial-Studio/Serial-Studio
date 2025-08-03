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
// (72,64) code test (SEC-DED)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FEC_SECDED 1

// P matrix [8 x 64]
//  11111111 00001111 00001111 00001100 01101000 10001000 10001000 10000000 : 
//  11110000 11111111 00000000 11110011 01100100 01000100 01000100 01000000 : 
//  00110000 11110000 11111111 00001111 00000010 00100010 00100010 00100110 : 
//  11001111 00000000 11110000 11111111 00000001 00010001 00010001 00010110 : 
//  01101000 10001000 10001000 10000000 11111111 00001111 00000000 11110011 : 
//  01100100 01000100 01000100 01000000 11110000 11111111 00001111 00001100 : 
//  00000010 00100010 00100010 00100110 11001111 00000000 11111111 00001111 : 
//  00000001 00010001 00010001 00010110 00110000 11110000 11110000 11111111 : 
unsigned char P[64] = {
    0xFF, 0x0F, 0x0F, 0x0C, 0x68, 0x88, 0x88, 0x80,
    0xF0, 0xFF, 0x00, 0xF3, 0x64, 0x44, 0x44, 0x40,
    0x30, 0xF0, 0xFF, 0x0F, 0x02, 0x22, 0x22, 0x26,
    0xCF, 0x00, 0xF0, 0xFF, 0x01, 0x11, 0x11, 0x16,
    0x68, 0x88, 0x88, 0x80, 0xFF, 0x0F, 0x00, 0xF3,
    0x64, 0x44, 0x44, 0x40, 0xF0, 0xFF, 0x0F, 0x0C,
    0x02, 0x22, 0x22, 0x26, 0xCF, 0x00, 0xFF, 0x0F,
    0x01, 0x11, 0x11, 0x16, 0x30, 0xF0, 0xF0, 0xFF};

void print_bitstring_short(unsigned char _x,
                           unsigned char _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        printf("%1u", (_x >> (_n-i-1)) & 1);
}

void print_bitstring(unsigned char * _x,
                     unsigned char   _n)
{
    unsigned int i;
    // compute number of elements in _x
    div_t d = div(_n, 8);
    unsigned int N = d.quot + (d.rem ? 1 : 0);

    // print leader
    printf("    ");
    if (d.rem == 0) printf(" ");
    for (i=0; i<8-d.rem-1; i++)
        printf(" ");

    // print bitstring
    for (i=0; i<N; i++) {
        if (i==0 && d.rem)
            print_bitstring_short(_x[i], d.rem);
        else
            print_bitstring_short(_x[i], 8);

        printf(" ");

    }
    printf("\n");
}


int main(int argc, char*argv[])
{
    unsigned int i;
    
    // error vector [72 x 1]
    unsigned char e[9] = {0,0,0,0,0,0,0,0,2};

    // original message [64 x 1]
    unsigned char m[8] = {0,0,0,0,0,0,0,1};

    // derived values
    unsigned char v[9];     // encoded/transmitted message
    unsigned char r[9];     // received vector
    unsigned char s;        // syndrome vector
    unsigned char v_hat[9]; // estimated transmitted message
    unsigned char m_hat[8]; // estimated original message

    // original message
    printf("m (original message):\n         ");
    print_bitstring(m,64);

    // compute encoded/transmitted message: v = m*G
    v[0] = 0;
    for (i=0; i<8; i++) {
        v[0] <<= 1;

        unsigned int p = liquid_c_ones[ P[8*i+0] & m[0] ] +
                         liquid_c_ones[ P[8*i+1] & m[1] ] +
                         liquid_c_ones[ P[8*i+2] & m[2] ] +
                         liquid_c_ones[ P[8*i+3] & m[3] ] +
                         liquid_c_ones[ P[8*i+4] & m[4] ] +
                         liquid_c_ones[ P[8*i+5] & m[5] ] +
                         liquid_c_ones[ P[8*i+6] & m[6] ] +
                         liquid_c_ones[ P[8*i+7] & m[7] ];
        //printf("p = %u\n", p);
        v[0] |= p & 0x01;
    }
    for (i=0; i<8; i++)
        v[i+1] = m[i];
    printf("v (encoded/transmitted message):\n");
    print_bitstring(v,72);

    // use pre-determined error vector
    printf("e (error vector):\n");
    print_bitstring(e,72);

    // compute received vector: r = v + e
    for (i=0; i<9; i++)
        r[i] = v[i] ^ e[i];
    printf("r (received vector):\n");
    print_bitstring(r,72);

    // compute syndrome vector, s = r*H^T = ( H*r^T )^T
    s = 0;
    for (i=0; i<8; i++) {
        s <<= 1;
        unsigned int p =
            ( (r[0] & (1<<(8-i-1))) ? 1 : 0 )+
            liquid_c_ones[ P[8*i+0] & r[1] ] +
            liquid_c_ones[ P[8*i+1] & r[2] ] +
            liquid_c_ones[ P[8*i+2] & r[3] ] +
            liquid_c_ones[ P[8*i+3] & r[4] ] +
            liquid_c_ones[ P[8*i+4] & r[5] ] +
            liquid_c_ones[ P[8*i+5] & r[6] ] +
            liquid_c_ones[ P[8*i+6] & r[7] ] +
            liquid_c_ones[ P[8*i+7] & r[8] ];

        printf("p = %u\n", p);

        s |= p & 0x01;
    }
    printf("s (syndrome vector):\n");
    print_bitstring(&s,8);

    // compute weight of s
    unsigned int ws = liquid_count_ones(s);
    printf("w(s) = %u\n", ws);

    // estimated error vector
    unsigned char e_hat[9] = {0,0,0,0,0,0,0,0,0}; 

    if (ws == 0) {
        printf("no errors detected\n");
    } else {
        // estimate error location
        int syndrome_match = 0;

        // TODO : these can be pre-computed
        unsigned int n;
        for (n=0; n<72; n++) {
            // compute syndrome
            unsigned char e_test[9]  = {0,0,0,0,0,0,0,0,0};
            unsigned char s_hat = 0;

            div_t d = div(n,8);
            e_test[9-d.quot-1] = 1 << d.rem;

            for (i=0; i<8; i++) {
                s_hat <<= 1;
                unsigned int p =
                    ( (e_test[0] & (1<<(8-i-1))) ? 1 : 0 )+
                    liquid_c_ones[ P[8*i+0] & e_test[1] ] +
                    liquid_c_ones[ P[8*i+1] & e_test[2] ] +
                    liquid_c_ones[ P[8*i+2] & e_test[3] ] +
                    liquid_c_ones[ P[8*i+3] & e_test[4] ] +
                    liquid_c_ones[ P[8*i+4] & e_test[5] ] +
                    liquid_c_ones[ P[8*i+5] & e_test[6] ] +
                    liquid_c_ones[ P[8*i+6] & e_test[7] ] +
                    liquid_c_ones[ P[8*i+7] & e_test[8] ];

                s_hat |= p & 0x01;
            }

            // print results
            //printf("e_test:"); print_bitstring(e_test, 72);
            printf("%2u e=", n);
            for (i=0; i<9; i++) {
                print_bitstring_short(e_test[i],8);
                printf(" ");
            }
            printf("s=");
            print_bitstring_short(s_hat,8);
            if (s == s_hat) printf("*");
            printf("\n");

            if (s == s_hat) {
                memmove(e_hat, e_test, 9*sizeof(unsigned char));
                syndrome_match = 1;
            }
        }

        if (syndrome_match) {
            printf("syndrome match!\n");
        } else {
            printf("no syndrome match; expected multiple errors\n");
        }
    }

    // compute estimated transmitted message: v_hat = r + e_hat
    printf("e-hat (estimated error vector):\n");
    print_bitstring(e_hat,72);

    printf("v-hat (estimated transmitted vector):\n");
    for (i=0; i<9; i++)
        v_hat[i] = r[i] ^ e_hat[i];
    print_bitstring(v_hat,72);
    //print_bitstring(v,    72);

    // compute errors between v, v_hat
    unsigned int num_errors_encoded = count_bit_errors_array(v, v_hat, 9);
    printf("decoding errors (encoded)  : %2u / 72\n", num_errors_encoded);

    // compute estimated original message: (last 64 bits of encoded message)
    for (i=0; i<8; i++)
        m_hat[i] = v_hat[i+1];
    printf("m-hat (estimated original vector):\n         ");
    print_bitstring(m_hat,64);
    //print_bitstring(m,    64);

    // compute errors between m, m_hat
    unsigned int num_errors_decoded = count_bit_errors_array(m, m_hat, 8);
    printf("decoding errors (original) : %2u / 64\n", num_errors_decoded);

    return 0;
}

