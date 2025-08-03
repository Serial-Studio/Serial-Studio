// test linking to liquid after library has been installed
#include <stdio.h>
#include <stdlib.h>
#include <liquid/liquid.h>

int main()
{
    // test liquid version number
    printf("checking installed liquid version numbers...\n");
    printf("  header  : 0x%.6x\n", LIQUID_VERSION_NUMBER);
    printf("  library : 0x%.6x\n", liquid_libversion_number());
    LIQUID_VALIDATE_LIBVERSION;

    // create object, print and return
    printf("creating test object...\n");
    resamp_crcf q = resamp_crcf_create(0.12345f, 12, 0.25f, 60.0f, 256);
    resamp_crcf_print(q);
    resamp_crcf_destroy(q);
    return 0;
}

