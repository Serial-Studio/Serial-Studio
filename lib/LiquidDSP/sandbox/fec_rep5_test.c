//
//
//

#include <stdio.h>
#include "liquid.h"

int main() {
    unsigned char s0, s1, s2, s3, s4, r, r_hat;
    unsigned int errors = 0;

    for (s0=0; s0<2; s0++) {
    for (s1=0; s1<2; s1++) {
    for (s2=0; s2<2; s2++) {
    for (s3=0; s3<2; s3++) {
    for (s4=0; s4<2; s4++) {
        r = (s0 + s1 + s2 + s3 + s4) < 3 ? 0 : 1;

        r_hat = (s0 & s1) | (s0 & s2) | (s0 & s3) | (s0 & s4) |
                            (s1 & s2) | (s1 & s3) | (s1 & s4) |
                                        (s2 & s3) | (s2 & s4) |
                                                    (s3 & s4);

        r_hat = (s0 & s1 & s2) |
                (s0 & s1 & s3) |
                (s0 & s1 & s4) |
                (s0 & s2 & s3) |
                (s0 & s2 & s4) |
                (s0 & s3 & s4) |
                (s1 & s2 & s3) |
                (s1 & s2 & s4) |
                (s1 & s3 & s4) |
                (s2 & s3 & s4);

        int error_found = 0;

        errors += r != r_hat;

        printf("%2u %2u %2u %2u %2u    %2u (%2u) %c %2u\n",
                s0, s1, s2, s3, s4,    r,  r_hat, r==r_hat ? ' ' : '*', error_found);
    }
    }
    }
    }
    }
    printf("errors : %u\n", errors);

    return 0;
}
