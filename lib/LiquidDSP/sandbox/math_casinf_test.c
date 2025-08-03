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
// math_casinf_test.c
//
// complex arc-sine test
//

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

#define sandbox_randf() ((float) rand() / (float) RAND_MAX)

float complex sandbox_cacosf(float complex _z)
{
    // return based on quadrant
    int sign_i = crealf(_z) > 0;
    int sign_q = cimagf(_z) > 0;

    if (sign_i == sign_q) {
        return - _Complex_I*clogf(_z + csqrtf(_z*_z - 1.0f));
    } else {
        return - _Complex_I*clogf(_z - csqrtf(_z*_z - 1.0f));
    }

    // should never get to this state
    return 0.0f;
}

float complex sandbox_casinf(float complex _z)
{
    return 0.5f*M_PI - sandbox_cacosf(_z);
}

int main() {
    unsigned int n=32;  // number of tests
    unsigned int d=2;   // number items per line

    // data arrays
    float complex z[n];
    float complex test[n];

    float complex err_max = 0.0f;
    unsigned int i;
    for (i=0; i<n; i++) {
        // generate random complex number
        z[i] = 2.0f*(2.0f*sandbox_randf() - 1.0f) +
               2.0f*(2.0f*sandbox_randf() - 1.0f) * _Complex_I;

        test[i] = casinf(z[i]);
        float complex asinz_hat = sandbox_casinf(z[i]);

        float complex err = test[i] - asinz_hat;

        printf("%3u: z=%6.2f+j%6.2f, asin(z)=%6.2f+j%6.2f (%6.2f+j%6.2f) e=%12.4e\n",
                i,
                crealf(z[i]),       cimagf(z[i]),
                crealf(test[i]),    cimagf(test[i]),
                crealf(asinz_hat),  cimagf(asinz_hat),
                cabsf(err));

        if ( cabsf(err) > cabsf(err_max) )
            err_max = err;
    }

    printf("maximum error: %12.4e;\n", cabsf(err_max));

    // 
    // print autotest lines
    //

    printf("\n");
    printf("    float complex z[%u] = {\n      ", n);
    for (i=0; i<n; i++) {
        printf("%12.4e+_Complex_I*%12.4e", crealf(z[i]), cimagf(z[i]));

        if ( i == n-1)
            printf(" ");
        else if ( ((i+1)%d)==0 )
            printf(",\n      ");
        else
            printf(", ");
    }
    printf("};\n");

    printf("\n");
    printf("    float complex test[%u] = {\n      ", n);
    for (i=0; i<n; i++) {
        printf("%12.4e+_Complex_I*%12.4e", crealf(test[i]), cimagf(test[i]));

        if ( i == n-1)
            printf(" ");
        else if ( ((i+1)%d)==0 )
            printf(",\n      ");
        else
            printf(", ");
    }
    printf("};\n");

    printf("done.\n");
    return 0;
}

