// 
// simpelx.c : simplex method for solving linear programming problems
//

#include <stdio.h>
#include <stdlib.h>
#include "liquid.h"

void matrixf_pivot2(float * _x,
                    unsigned int _m,
                    unsigned int _n,
                    unsigned int _i,
                    unsigned int _j)
{
    float v = 1.0f / matrix_access(_x,_m,_n,_i,_j);

    float g;    // multiplier
    unsigned int r,c;
    for (r=0; r<_m; r++) {
        // skip over pivot row
        if (r == _i) continue;

        // compute multiplier
        g = matrix_access(_x,_m,_n,r,_j) * v;

        // back-substitution
        for (c=0; c<_n; c++) {
            matrix_access(_x,_m,_n,r,c) -= matrix_access(_x,_m,_n,_i,c)*g;
        }
    }

    // normalize row by multiplier
    for (c=0; c<_n; c++)
        matrix_access(_x,_m,_n,_i,c) *= v;
}

int main() {
    // problem definition
    float x[18] = {
        1.0f,   -1.0f,  -1.0f,  0.0f,   0.0f,   0.0f,
        0.0f,    2.0f,   1.0f,  1.0f,   0.0f,   4.0f,
        0.0f,    1.0f,   2.0f,  0.0f,   1.0f,   3.0f
    };
    unsigned int r = 3;
    unsigned int c = 6;

    matrixf_print(x,r,c);

    // pivot around x(1,1) = 2
    matrixf_pivot2(x,r,c, 1,1);
    matrixf_print(x,r,c);

    // pivot around x(2,2) = 1.5
    matrixf_pivot2(x,r,c, 2,2);
    matrixf_print(x,r,c);

    // print solution
    printf("solution:\n");
    printf("z     : %12.8f\n", matrixf_access(x,r,c,0,c-1));
    unsigned int i;
    for (i=1; i<r; i++)
    printf("x[%2u] : %12.8f\n", i-1, matrixf_access(x,r,c,i,c-1));

    printf("done.\n");
    return 0;
}

