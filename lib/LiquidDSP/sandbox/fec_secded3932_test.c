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
// (39,32) code test (SEC-DED)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FEC_SECDED 1

// P matrix [7 x 32 bits], [7 x 4 bytes]
//  1000 1010 1000 0010 0000 1111 0001 1011
//  0001 0000 0001 1111 0111 0001 0110 0001
//  0001 0110 1111 0000 1001 0010 1010 0110
//  1111 1111 0000 0001 1010 0100 0100 0100
//  0110 1100 1111 1111 0000 1000 0000 1000
//  0010 0001 0010 0100 1111 1111 1001 0000
//  1100 0001 0100 1000 0100 0000 1111 1111
unsigned char P[28] = {
    0x8a, 0x82, 0x0f, 0x1b,
    0x10, 0x1f, 0x71, 0x61,
    0x16, 0xf0, 0x92, 0xa6,
    0xff, 0x01, 0xa4, 0x44,
    0x6c, 0xff, 0x08, 0x08,
    0x21, 0x24, 0xff, 0x90,
    0xc1, 0x48, 0x40, 0xff};


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
    
    // error vector [22 x 1]
    unsigned char err[5] = {0,0,0,0,2};

    // original message [16 x 1]
    unsigned char m[4] = {0,0,0,1};
    //m[0] = rand() & 0xffff;
    //m[1] = rand() & 0xffff;

    // derived values
    unsigned char v[5];     // encoded/transmitted message
    unsigned char e[5];     // error vector
    unsigned char r[5];     // received vector
    unsigned char s;        // syndrome vector
    unsigned char e_hat[5] = {0,0,0,0,0};  // estimated error vector
    unsigned char v_hat[5]; // estimated transmitted message
    unsigned char m_hat[4]; // estimated original message

#if 0
    // print P matrix
    printf("P : \n");
    print_bitstring(&P[ 0],32);
    print_bitstring(&P[ 4],32);
    print_bitstring(&P[ 8],32);
    print_bitstring(&P[12],32);
    print_bitstring(&P[16],32);
    print_bitstring(&P[20],32);
    print_bitstring(&P[24],32);
#endif

    // original message
    printf("m (original message):   ");
    print_bitstring(m,32);

    // compute encoded/transmitted message: v = m*G
    v[0] = 0;
    for (i=0; i<7; i++) {
        v[0] <<= 1;

        unsigned int p = liquid_c_ones[P[4*i+0] & m[0]] +
                         liquid_c_ones[P[4*i+1] & m[1]] +
                         liquid_c_ones[P[4*i+2] & m[2]] +
                         liquid_c_ones[P[4*i+3] & m[3]];
        //printf("p = %u\n", p);
        v[0] |= p & 0x01;
    }
    v[1] = m[0];
    v[2] = m[1];
    v[3] = m[2];
    v[4] = m[3];
    printf("v (encoded message):    ");
    print_bitstring(v,39);

    // use pre-determined error vector
    e[0] = err[0];
    e[1] = err[1];
    e[2] = err[2];
    e[3] = err[3];
    e[4] = err[4];
    printf("e (error vector):       ");
    print_bitstring(e,39);

    // compute received vector: r = v + e
    r[0] = v[0] ^ e[0];
    r[1] = v[1] ^ e[1];
    r[2] = v[2] ^ e[2];
    r[3] = v[3] ^ e[3];
    r[4] = v[4] ^ e[4];
    printf("r (received vector):    ");
    print_bitstring(r,39);

    // compute syndrome vector, s = r*H^T = ( H*r^T )^T
    s = 0;
    for (i=0; i<7; i++) {
        s <<= 1;

        unsigned int p =
            ( (r[0] & (1<<(7-i-1))) ? 1 : 0 )+
            liquid_count_ones(P[4*i+0] & r[1]) +
            liquid_count_ones(P[4*i+1] & r[2]) +
            liquid_count_ones(P[4*i+2] & r[3]) +
            liquid_count_ones(P[4*i+3] & r[4]);
        //printf("p = %u\n", p);

        s |= p & 0x01;
    }
    printf("s (syndrome vector):    ");
    print_bitstring(&s,7);

    // compute weight of s
    unsigned int ws = liquid_count_ones(s);
    printf("weight(s) = %u\n", ws);

    if (ws == 0) {
        printf("no errors detected\n");
    } else {
        // estimate error location
        int syndrome_match = 0;

        // TODO : these can be pre-computed
        unsigned int n;
        for (n=0; n<39; n++) {
            // compute syndrome
            unsigned int s_test = 0;
            unsigned char e_test[5] = {0,0,0,0,0};

            div_t d = div(n,8);
            e_test[5-d.quot-1] = 1 << d.rem;

            for (i=0; i<7; i++) {
                s_test <<= 1;
                unsigned int p =
                    ( (e_test[0] & (1<<(7-i-1))) ? 1 : 0 )+
                    liquid_count_ones(P[4*i+0] & e_test[1]) +
                    liquid_count_ones(P[4*i+1] & e_test[2]) +
                    liquid_count_ones(P[4*i+2] & e_test[3]) +
                    liquid_count_ones(P[4*i+3] & e_test[4]);

                s_test |= p & 0x01;
            }

#if 1
            // print results
            //printf("e_test:"); print_bitstring(e_test, 72);
            printf("%3u : e = ", n);
            print_bitstring_short(e_test[0],7); printf(" ");
            print_bitstring_short(e_test[1],8); printf(" ");
            print_bitstring_short(e_test[2],8); printf(" ");
            print_bitstring_short(e_test[3],8); printf(" ");
            print_bitstring_short(e_test[4],8); printf(" ");
            printf(", s = ");
            print_bitstring_short(s_test,7);
            if (s == s_test) printf(" *");
            printf("\n");
#else
            // print output array (secded2216_syndrome_w1[])
            printf("0x%.2x\n", s_test);
#endif

            if (s == s_test) {
                memmove(e_hat, e_test, sizeof(e_test));
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
    printf("e-hat (estimated error):");
    print_bitstring(e_hat,39);

    printf("v-hat (estimated tx):   ");
    v_hat[0] = r[0] ^ e_hat[0];
    v_hat[1] = r[1] ^ e_hat[1];
    v_hat[2] = r[2] ^ e_hat[2];
    v_hat[3] = r[3] ^ e_hat[3];
    v_hat[4] = r[4] ^ e_hat[4];
    print_bitstring(v_hat,39);

    // compute estimated original message: (last 16 bits of encoded message)
    m_hat[0] = v_hat[1];
    m_hat[1] = v_hat[2];
    m_hat[2] = v_hat[3];
    m_hat[3] = v_hat[4];
    printf("m-hat (estimated orig.):");
    print_bitstring(m_hat,32);

    // compute errors between v, v_hat
    unsigned int num_errors_encoded = count_bit_errors_array(v, v_hat, 5);
    printf("decoding errors (encoded)  : %2u / 39\n", num_errors_encoded);

    // compute errors between m, m_hat
    unsigned int num_errors_decoded = count_bit_errors_array(m, m_hat, 4);
    printf("decoding errors (original) : %2u / 32\n", num_errors_decoded);

    return 0;
}

