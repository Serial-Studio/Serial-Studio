// test library versioning
#include <stdio.h>
#include <stdlib.h>
#include "liquid.h"

int main() {
    // 
    LIQUID_VALIDATE_LIBVERSION

    printf("liquid version           : %s\n", liquid_version);
    printf("liquid libversion        : %s\n", liquid_libversion());
    printf("liquid libversion number : 0x%.6x\n", liquid_libversion_number());

    return 0;
}
