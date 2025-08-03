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
//  Golay(24,12) code test
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FEC_GOLAY 1

// P matrix [12 x 12]
unsigned int P[12] = {
    0x08ed, 0x01db, 0x03b5, 0x0769,
    0x0ed1, 0x0da3, 0x0b47, 0x068f,
    0x0d1d, 0x0a3b, 0x0477, 0x0ffe};

#if 0
// generator matrix [12 x 24]
unsigned int G[12] = {
    0x008ed800, 0x001db400, 0x003b5200, 0x00769100,
    0x00ed1080, 0x00da3040, 0x00b47020, 0x0068f010,
    0x00d1d008, 0x00a3b004, 0x00477002, 0x00ffe001};
#endif

// generator matrix transposed [24 x 12]
unsigned int Gt[24] = {
    0x08ed, 0x01db, 0x03b5, 0x0769, 0x0ed1, 0x0da3, 0x0b47, 0x068f, 
    0x0d1d, 0x0a3b, 0x0477, 0x0ffe, 0x0800, 0x0400, 0x0200, 0x0100, 
    0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001};

// parity check matrix [12 x 24]
unsigned int H[12] = {
    0x008008ed, 0x004001db, 0x002003b5, 0x00100769,
    0x00080ed1, 0x00040da3, 0x00020b47, 0x0001068f,
    0x00008d1d, 0x00004a3b, 0x00002477, 0x00001ffe};

void print_bitstring(unsigned int _x,
                     unsigned int _n)
{
    unsigned int i;
    printf("    ");
    for (i=0; i<_n; i++)
        printf("%1u ", (_x >> (_n-i-1)) & 1);
    printf("\n");
}

int main(int argc, char*argv[])
{
    unsigned int i;
    
    // error vector
    //  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //  1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0
    unsigned int err = 0x00000d00;

    // original message
    // 1 0 1 1 1 1 0 0 1 1 0 1
    unsigned int m  = 0x0bcd;

    // derived values
    unsigned int v;     // encoded/transmitted message
    unsigned int e;     // error vector
    unsigned int r;     // received vector
    unsigned int s;     // syndrome vector
    unsigned int e_hat; // estimated error vector
    unsigned int v_hat; // estimated transmitted message
    unsigned int m_hat; // estimated original message

    // original message
    printf("m (original message):\n");
    print_bitstring(m,12);

    // compute encoded/transmitted message: v = m*G
    v = 0;
    for (i=0; i<24; i++) {
        v <<= 1;
        v |= liquid_count_ones_mod2(Gt[i] & m);
    }
    printf("v (encoded/transmitted message):\n");
    print_bitstring(v,24);

    // use pre-determined error vector
    e = err;
    printf("e (error vector):\n");
    print_bitstring(e,24);

    // compute received vector: r = v + e
    r = v ^ e;
    printf("r (received vector):\n");
    print_bitstring(r,24);

    // compute syndrome vector, s = r*H^T = ( H*r^T )^T
    s = 0;
    for (i=0; i<12; i++) {
        s <<= 1;
        s |= liquid_count_ones_mod2(H[i] & r);
    }
    printf("s (syndrome vector):\n");
    print_bitstring(s,12);

    // compute weight of s
    unsigned int ws = liquid_count_ones(s);
    printf("w(s) = %u\n", ws);

    // step 2:
    e_hat = 0;
    if (ws <= 3) {
        printf("    w(s) <= 3: estimating error vector as [s, 0(12)]\n");
        // set e_hat = [s 0(12)]
        e_hat = s << 12;
    } else {
        // step 3: search for p[i] s.t. w(s+p[i]) <= 2
        printf("    searching for w(s + p_i) <= 2...\n");
        unsigned int j;
        unsigned int spj;
        int flag3 = 0;
        unsigned int p3_index = 0;
        for (j=0; j<12; j++) {
            spj = s ^ P[j];
            unsigned int wj = liquid_count_ones(spj);
            printf("    w(s + p[%2u]) = %2u%s\n", j, wj, wj <= 2 ? " *" : "");
            if (wj <= 2) {
                flag3 = 1;
                p3_index = j;
                break;
            }
        }

        if (flag3) {
            // vector found!
            printf("    w(s + p[%2u]) <= 2: estimating error vector as [s+p[%2u],u[%2u]]\n", p3_index, p3_index, p3_index);
            // NOTE : uj = 1 << (12-j-1)
            e_hat = ((s ^ P[p3_index]) << 12) | (1 << (11-p3_index));
        } else {
            // step 4: compute s*P
            unsigned int sP = 0;
            for (i=0; i<12; i++) {
                sP <<= 1;
                sP |= liquid_count_ones_mod2(s & P[i]);
            }
            printf("s*P:\n");
            print_bitstring(sP,12);

            unsigned int wsP = liquid_count_ones(sP);
            printf("w(s*P) = %u\n", wsP);

            if (wsP == 2 || wsP == 3) {
                // step 5: set e = [0, s*P]
                printf("    w(s*P) in [2,3]: estimating error vector as [0(12), s*P]\n");
                e_hat = sP;
            } else {
                // step 6: search for p[i] s.t. w(s*P + p[i]) == 2...

                printf("    searching for w(s*P + p_i) == 2...\n");
                unsigned int j;
                unsigned int sPpj;
                int flag6 = 0;
                unsigned int p6_index = 0;
                for (j=0; j<12; j++) {
                    sPpj = sP ^ P[j];
                    unsigned int wj = liquid_count_ones(sPpj);
                    printf("    w(s*P + p[%2u]) = %2u%s\n", j, wj, wj == 2 ? " *" : "");
                    if (wj == 2) {
                        flag6 = 1;
                        p6_index = j;
                        break;
                    }
                }

                if (flag6) {
                    // vector found!
                    printf("    w(s*P + p[%2u]) == 2: estimating error vector as [u[%2u],s*P+p[%2u]]\n", p6_index, p6_index, p6_index);
                    // NOTE : uj = 1 << (12-j-1)
                    //      [      uj << 1 2    ] [    sP + p[j]    ]
                    e_hat = (1 << (23-p6_index)) | (sP ^ P[p6_index]);
                } else {
                    // step 7: decoding error
                    printf("  **** decoding error\n");
                }
            }
        }
    }

    // step 8: compute estimated transmitted message: v_hat = r + e_hat
    printf("e-hat (estimated error vector):\n");
    print_bitstring(e_hat,24);
    v_hat = r ^ e_hat;

    printf("v-hat (estimated transmitted vector):\n");
    print_bitstring(v_hat,24);
    print_bitstring(v,    24);

    // compute errors between v, v_hat
    printf("decoding errors (encoded)  : %2u / 24\n", count_bit_errors(v, v_hat));

    // compute estimated original message: (last 12 bits of encoded message)
    m_hat = v_hat & ((1<<12)-1);

    // compute errors between m, m_hat
    printf("decoding errors (original) : %2u / 12\n", count_bit_errors(m, m_hat));

    return 0;
}

