//
// ellip_func_test.c
//
// gcc -I. -I./include -lm libliquid.a ellip_func_test.c -o ellip_func_test
//

#include <stdio.h>
#include <stdlib.h>

#include "include/liquid.h"
#include "src/filter/src/ellip.c"

int main() {

    float complex u = 0.7f;
    float k = 0.8f;
    unsigned int n=7;

    float complex cd = ellip_cdf(u,k,n);
    float complex sn = ellip_snf(u,k,n);
    printf("u   : %12.8f + j*%12.8f\n", crealf(u), cimagf(u));
    printf("k   : %12.8f\n", k);
    printf("cd  : %12.8f + j*%12.8f\n", crealf(cd), cimagf(cd));
    printf("sn  : %12.8f + j*%12.8f\n", crealf(sn), cimagf(sn));

    printf("\n");
    float complex acd = ellip_acdf(cd,k,n);
    float complex asn = ellip_asnf(sn,k,n);
    printf("acd : %12.8f + j*%12.8f\n", crealf(acd), cimagf(acd));
    printf("asn : %12.8f + j*%12.8f\n", crealf(asn), cimagf(asn));

    printf("done.\n");
    return 0;
}

