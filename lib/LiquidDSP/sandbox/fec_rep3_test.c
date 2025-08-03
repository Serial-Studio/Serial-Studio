//
//
//

#include <stdio.h>
#include "liquid.h"

int main() {
    unsigned char s0, s1, s2, r, r_hat;
    unsigned int errors = 0;

    for (s0=0; s0<2; s0++) {
        for (s1=0; s1<2; s1++) {
            for (s2=0; s2<2; s2++) {
                r = (s0 + s1 + s2) < 2 ? 0 : 1;

                //r_hat = (!s0 & s1 & s2) | (s0 & !(!s1 & !s2));
                r_hat = (s1 & s2) | (s0 & s1) | (s0 & s2);

                int error_found = (s0 ^ s1) | (s0 ^ s2) | (s1 ^ s2);

                errors += r != r_hat;

                printf("%2u %2u %2u    %2u (%2u) %2u\n", s0, s1, s2, r, r_hat, error_found);
            }
        }
    }
    printf("errors : %u\n", errors);

    return 0;
}
