//
// chromosome_example.c
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

int main() {
    unsigned int bits_per_trait[] = {4, 8, 8, 4};
    chromosome p1 = chromosome_create(bits_per_trait, 4);
    chromosome p2 = chromosome_create(bits_per_trait, 4);
    chromosome c  = chromosome_create(bits_per_trait, 4);

    // 0000 11111111 00000000 1111
    p1->traits[0] = 0x0;
    p1->traits[1] = 0xFF;
    p1->traits[2] = 0x00;
    p1->traits[3] = 0xF;

    // 0101 01010101 01010101 0101
    p2->traits[0] = 0x5;
    p2->traits[1] = 0x55;
    p2->traits[2] = 0x55;
    p2->traits[3] = 0x5;

    printf("parent [1]:\n");
    chromosome_print(p1);

    printf("parent [2]:\n");
    chromosome_print(p2);

    printf("\n\n");

    // 
    // test crossover
    //

    printf("testing crossover...\n");

    chromosome_crossover(p1, p2, c, 0);
    // .... ........ ........ ....
    // 0101 01010101 01010101 0101
    chromosome_print(c);

    chromosome_crossover(p1, p2, c, 4);
    // 0000 ........ ........ ....
    // .... 01010101 01010101 0101
    chromosome_print(c);

    chromosome_crossover(p1, p2, c, 6);
    // 0000 11...... ........ ....
    // .... ..010101 01010101 0101
    chromosome_print(c);

    chromosome_crossover(p1, p2, c, 14);
    // 0000 11111111 00...... ....
    // .... ........ ..010101 0101
    chromosome_print(c);

    chromosome_crossover(p1, p2, c, 24);
    // 0000 11111111 00000000 1111
    // .... ........ ........ ....
    chromosome_print(c);

    // 
    // test mutation
    //

    printf("testing mutation...\n");

    unsigned int i;
    for (i=0; i<24; i++) {
        chromosome_reset(c);
        chromosome_mutate(c,i);
        // 0000 01000000 00000000 0000
        chromosome_print(c);
    }

    chromosome_destroy(p1);
    chromosome_destroy(p2);
    chromosome_destroy(c);

    return 0;
}

