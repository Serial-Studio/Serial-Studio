// test sparse matrix operations
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

int main(int argc, char*argv[])
{
#if 0
    unsigned int M = 12;
    unsigned int N = 16;

    // create empty list
    smatrixb q = smatrixb_create(M,N);

    // set value(s)
    smatrixb_set(q,1,2,  1);
    smatrixb_set(q,2,2,  1);
    smatrixb_set(q,2,3,  1);
    smatrixb_set(q,2,7,  1);
    smatrixb_set(q,2,8,  1);
    smatrixb_set(q,2,11, 1);
    smatrixb_delete(q,2,2);

    printf("\ncompact form:\n");
    smatrixb_print(q);

    printf("\nexpanded form:\n");
    smatrixb_print_expanded(q);

    printf("\ncertain values:\n");
    printf("  A[%2u,%2u] = %1u\n", 1, 1, smatrixb_get(q,1,1));
    printf("  A[%2u,%2u] = %1u\n", 1, 2, smatrixb_get(q,1,2));

    // generate vectors
    unsigned char x[N];
    unsigned char y[M];
    unsigned int i;
    unsigned int j;
    for (j=0; j<N; j++)
        x[j] = rand() % 2 ? 1 : 0;

    smatrixb_vmul(q,x,y);

    // print results
    printf("x = [");
    for (j=0; j<N; j++) printf("%2u", x[j]);
    printf(" ];\n");

    printf("y = [");
    for (i=0; i<M; i++) printf("%2u", y[i]);
    printf(" ];\n");

    smatrixb_destroy(q);
//#else
    unsigned int M = 12;
    unsigned int N = 12;

    // create empty list
    smatrixf q = smatrixf_create(M,N);

    // set value(s)
    smatrixf_set(q,1,2,  1.0f);
    smatrixf_set(q,2,2,  2.0f);
    smatrixf_set(q,2,3,  3.0f);
    smatrixf_set(q,2,7,  4.0f);
    smatrixf_set(q,2,8,  5.0f);
    smatrixf_set(q,2,11, 6.0f);
    smatrixf_delete(q,2,2);

    printf("\ncompact form:\n");
    smatrixf_print(q);

    printf("\nexpanded form:\n");
    smatrixf_print_expanded(q);

    printf("\ncertain values:\n");
    printf("  A[%2u,%2u] = %6.2f\n", 1, 1, smatrixf_get(q,1,1));
    printf("  A[%2u,%2u] = %6.2f\n", 1, 2, smatrixf_get(q,1,2));

    smatrixf_destroy(q);
#endif

#if 1
    // 
    // test matrix multiplication
    //
    smatrixb a = smatrixb_create( 8,12);
    smatrixb b = smatrixb_create(12, 5);
    smatrixb c = smatrixb_create( 8, 5);

    // initialize 'a'
    // 0 0 0 0 1 0 0 0 0 0 0 0
    // 0 0 0 0 0 0 0 0 0 0 0 0
    // 0 0 0 1 0 0 0 1 0 0 0 0
    // 0 0 0 0 1 0 0 0 0 0 0 0
    // 0 0 0 1 0 0 1 1 0 0 0 0
    // 0 0 0 0 0 0 0 0 0 0 0 0
    // 0 1 0 0 0 0 0 0 0 0 0 0
    // 0 0 0 0 0 0 0 1 1 0 0 0
    smatrixb_set(a,0,4, 1);
    smatrixb_set(a,2,3, 1);
    smatrixb_set(a,2,7, 1);
    smatrixb_set(a,3,4, 1);
    smatrixb_set(a,4,3, 1);
    smatrixb_set(a,4,6, 1);
    smatrixb_set(a,4,7, 1);
    smatrixb_set(a,6,1, 1);
    smatrixb_set(a,7,7, 1);
    smatrixb_set(a,7,8, 1);

    // initialize 'b'
    // 1 1 0 0 0
    // 0 0 0 0 1
    // 0 0 0 0 0
    // 0 0 0 0 0
    // 0 0 0 0 0
    // 0 0 0 0 1
    // 0 0 0 1 0
    // 0 0 0 1 0
    // 0 0 0 0 0
    // 0 1 0 0 1
    // 1 0 0 1 0
    // 0 1 0 0 0
    smatrixb_set(b,0,0,  1);
    smatrixb_set(b,0,1,  1);
    smatrixb_set(b,1,4,  1);
    smatrixb_set(b,5,4,  1);
    smatrixb_set(b,6,3,  1);
    smatrixb_set(b,7,3,  1);
    smatrixb_set(b,9,1,  1);
    smatrixb_set(b,9,4,  1);
    smatrixb_set(b,10,0, 1);
    smatrixb_set(b,11,1, 1);

    // compute 'c'
    // 0 0 0 0 0
    // 0 0 0 0 0
    // 0 0 0 1 0
    // 0 0 0 0 0
    // 0 0 0(0)0
    // 0 0 0 0 0
    // 0 0 0 0 1
    // 0 0 0 1 0
    smatrixb_mul(a,b,c);

    // print results
    printf("a:\n"); smatrixb_print_expanded(a);
    printf("b:\n"); smatrixb_print_expanded(b);
    printf("c:\n"); smatrixb_print_expanded(c);

    smatrixb_destroy(a);
    smatrixb_destroy(b);
    smatrixb_destroy(c);
#else
    // 
    // test matrix multiplication
    //
    smatrixf a = smatrixf_create(4, 5);
    smatrixf b = smatrixf_create(5, 3);
    smatrixf c = smatrixf_create(4, 3);

    // initialize 'a'
    // 0 0 0 0 4
    // 0 0 0 0 0
    // 0 0 0 3 0
    // 2 0 0 0 1
    smatrixf_set(a, 0,4, 4);
    smatrixf_set(a, 2,3, 3);
    smatrixf_set(a, 3,0, 2);
    smatrixf_set(a, 3,4, 0);
    smatrixf_set(a, 3,4, 1);

    // initialize 'b'
    // 7 6 0
    // 0 0 0
    // 0 0 0
    // 0 5 0
    // 2 0 0
    smatrixf_set(b, 0,0, 7);
    smatrixf_set(b, 0,1, 6);
    smatrixf_set(b, 3,1, 5);
    smatrixf_set(b, 4,0, 2);

    printf("a:\n"); smatrixf_print(a); //smatrixf_print_expanded(a);
    printf("b:\n"); smatrixf_print(b); //smatrixf_print_expanded(b);
    // compute 'c'
    //  8   0   0
    //  0   0   0
    //  0  15   0
    // 16  12   0
    smatrixf_mul(a,b,c);

    // print results
    printf("c:\n"); smatrixf_print_expanded(c);
    printf("c:\n"); smatrixf_print(c);

    smatrixf_destroy(a);
    smatrixf_destroy(b);
    smatrixf_destroy(c);
#endif
    printf("done.\n");
    return 0;
}

