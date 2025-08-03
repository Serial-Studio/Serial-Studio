// 
// svd_test.c : test singular value decomposition
// 
// References:
//  [Golub:1970] G. H. Golub and C. Reinsch, "Singular Value
//      Decomposition and Least Squares Solutions," Handbook Series
//      Linear Algebra, Numerische Mathematik, vol. 14, pp 403-420,
//      1970

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <complex.h>

#include "liquid.h"

#define DEBUG 1

int main() {
    // problem definition
    //  sig1    =   sqrt(1248)
    //  sig2    =   20
    //  sig3    =   sqrt(384)
    //  sig4    =   0
    //  sig5    =   0
    float a[40] = {
        22,  10,   2,   3,  7,
        14,   7,  10,   0,  8,
        -1,  13,  -1, -11,  3,
        -3,  -2,  13,  -2,  4,
         9,   8,   1,  -2,  4,
         9,   1,  -7,   5, -1,
         2,  -6,   6,   5,  1,
         4,   5,   0,  -2,  2};

    // NOTE: m >= n
    unsigned int m = 8;
    unsigned int n = 5;

#if 0
    // initialize as random matrix
    unsigned int ii;
    for (ii=0; ii<m*n; ii++)
        a[ii] = randnf();
#endif

    matrixf_print(a,m,n);

    // 
    float q[n];     // singular values of 'a'
    float u[m*n];
    float v[n*n];
    float eps = 1e-8;
    float tol = 1e-8;

    // internal variables
    int i,j,k,l,l1;
    float c,f,g,h,s,x,y,z;
    float e[n];

    // initialization
    memmove(u,a,m*n*sizeof(float));

    // Householder's reduction to bidiagonal form
    g=0.0;
    x=0.0;
    for (i=0; i<n; i++) {
        e[i] = g;
        s    = 0;
        l    = i+1;

        for (j=i; j<m; j++) {
            float u_ji = matrix_access(u,m,n,j,i);
            s += creal( u_ji*conj(u_ji) );
        }
        if (s < tol) {
            g = 0.0;
        } else {
            f = matrix_access(u,m,n,i,i);
            g = (f<0) ? sqrt(s) : -sqrt(s);
            h = f*g - s;
            matrix_access(u,m,n,i,i) = f-g;

            for (j=l; j<n; j++) {
                s = 0;
                for (k=i; k<m; k++)
                    s += matrix_access(u,m,n,k,i)*matrix_access(u,m,n,k,j);
                f = s/h;
                for (k=i; k<m; k++)
                    matrix_access(u,m,n,k,j) += f*matrix_access(u,m,n,k,i);
            } // for j
        } // if (s < tol)

        q[i] = g;
        s = 0;
        for (j=l; j<n; j++) {
            float u_ij = matrix_access(u,m,n,i,j);
            s += creal( u_ij*conj(u_ij) );
        }
        if (s < tol) {
            g = 0;
        } else {
            f = matrix_access(u,m,n,i,i+1);
            g = (f < 0) ? sqrt(s) : -sqrt(s);
            h = f*g - s;
            matrix_access(u,m,n,i,i+1) = f-g;
            for (j=l; j<n; j++)
                e[j] = matrix_access(u,m,n,i,j)/h;
            for (j=l; j<m; j++) {
                s = 0;
                for (k=l; k<n; k++)
                    s += matrix_access(u,m,n,j,k)*matrix_access(u,m,n,i,k);
                for (k=l; k<n; k++)
                    matrix_access(u,m,n,j,k) += s*e[k];
            } // for j
        } // if (s < tol)

        y = fabsf(q[i]) + fabsf(e[i]);
        if (y > x)
            x = y;
    }

    // accumulation of right-hand transformations
    for (i=n-1; i>=0; i--) {
        //if (g != 0) {
        if ( fabs(g) > 1e-3 ) {
            h = matrix_access(u,m,n,i,i+1)*g;
            for (j=l; j<n; j++)
                matrix_access(v,n,n,j,i) = matrix_access(u,m,n,i,j)/h;
            for (j=l; j<n; j++) {
                s = 0.0;
                for (k=l; k<n; k++)
                    s += matrix_access(u,m,n,i,k)*matrix_access(v,n,n,k,j);
                for (k=l; k<n; k++)
                    matrix_access(v,n,n,k,j) += s*matrix_access(v,n,n,k,i);
            } // for j
        } // g != 0

        for (j=l; j<n; j++) {
            matrix_access(v,n,n,i,j) = 0.0;
            matrix_access(v,n,n,j,i) = 0.0;
        }
        matrix_access(v,n,n,i,i) = 1;
        g = e[i];
        l = i;
    }
#if DEBUG
    printf("U:\n"); matrixf_print(u,m,n);
    printf("V:\n"); matrixf_print(v,n,n);
#endif

    // accumulation of left-hand transformations
    printf("q:\n"); matrixf_print(q,n,1);
    for (i=n-1; i>=0; i--) {
        l = i+1;
        g = q[i];
        for (j=l; j<n; j++)
            matrix_access(u,m,n,i,j) = 0.0;
        //if (g != 0) {
        if ( fabs(g) > 1e-3 ) {
            h = matrix_access(u,m,n,i,i) * g;
            for (j=l; j<n; j++) {
                s = 0.0;
                for (k=l; k<m; k++)
                    s += matrix_access(u,m,n,k,i)*matrix_access(u,m,n,k,j);
                f = s/h;
                for (k=i; k<m; k++)
                    matrix_access(u,m,n,k,j) += f*matrix_access(u,m,n,k,i);
            } // for j
            for (j=i; j<m; j++)
                matrix_access(u,m,n,j,i) /= g;
        } else { // g != 0
            //printf("  treating g as zero\n");
            for (j=i; j<m; j++)
                matrix_access(u,m,n,j,i) = 0.0;
        } // g != 0
        matrix_access(u,m,n,i,i) += 1.0;
    }
#if DEBUG
    printf("U:\n"); matrixf_print(u,m,n);
    printf("V:\n"); matrixf_print(v,n,n);
    printf("q:\n"); matrixf_print(q,n,1);
    printf("e:\n"); matrixf_print(e,n,1);
#endif

    exit(1);

    // diagonalization of the bidiagonal form
    unsigned int t=10;
    eps *= x;
    for (k=n-1; k>=0; k--) {
        if (k==(n-2)) {
            printf("...\n");
            break;
        }
        test_f_splitting:
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
        printf("test f splitting\n");
        printf("  k = %u\n", k);
        t--;
        if (t==0) {
            fprintf(stderr,"***********************************************\n");
            fprintf(stderr,"    bailing\n");
            fprintf(stderr,"***********************************************\n");
            t=10;
            continue;
        }
        for (l=k; l>0; l--) {
            printf("  e[%3u] = %12.4e, q[%3u] = %12.4e (eps: %12.4e)\n", l, e[l], l-1, q[l-1], eps);
            if ( fabs(e[l])   <= eps ) goto test_f_convergence;
            if ( fabs(q[l-1]) <= eps ) goto cancellation;
        } // for l
        // FIXME: figure out logic here (need to set l > 0)
        l = 1;

        // cancellation of e[l] if l>0
        cancellation:
        printf("cancellation\n");
        c = 0;
        s = 1;
        l1 = l-1;
        for (i=l; i<k; i++) {
            f = s*e[i];
            e[i] *= c;
            printf("  i=%u, f=%f (eps: %e)\n", i, f, eps);
            if ( fabs(f) <= eps ) goto test_f_convergence;
            g = q[i];
            h = sqrt(f*f + g*g);
            q[i] = h;
            c = g/h;
            s = -f/h;

            // withu
            for (j=0; j<m; j++) {
                y = matrix_access(u,m,n,j,l1);
                z = matrix_access(u,m,n,j,i);
                matrix_access(u,m,n,j,l1) =  y*c + z*s;
                matrix_access(u,m,n,j,i)  = -y*s + z*c;
            } // for j
        } // for i
        printf("  y=%f, z=%f\n", y, z);

        test_f_convergence:
        printf("test f convergence\n");
        z = q[k];
        printf("  z = %f\n", z);
        if (l==k)
            goto convergence;

        // shift from bottom 2x2 minor
        x = q[l];
        y = q[k-1];
        g = e[k-1];
        h = e[k];
        f = ((y-z)*(y+z) + (g-h)*(g+h)) / (2*h*y);
        printf("  l=%u, k=%u, z=%f, x=%f, y=%f, g=%e, h=%f, f=%e\n",
                  l,    k,    z,    x,    y,    g,    h,    f);
        g = sqrt(f*f+1);
        f = ((x-z)*(x+z) + h*(y/( f<0 ? f-g : f+g )-h))/x;
        printf("  g -> %f, f -> %f\n", g, f);

        // next QR transformation
        c = 1;
        s = 1;
        for (i=l+1; i<k; i++) {
            g = e[i];
            y = q[i];
            h = s*g;
            g = c*g;
            z = sqrt(f*f+h*h);
            e[i-1] = z;
            c = f/z;
            s = h/z;
            f =  x*c + g*s;
            g = -x*s + g*c;
            h = y*s;
            y = y*c;

            // withv
            for (j=0; j<n; j++) {
                x = matrix_access(v,n,n,j,i-1);
                z = matrix_access(v,n,n,j,i);
                matrix_access(v,n,n,j,i-1) =  x*c + z*s;
                matrix_access(v,n,n,j,i)   = -x*s + z*c;
            } // for j

            z = sqrt(f*f + h*h);
            q[i-1] = z;
            c = f/z;
            s = h/z;
            f =  c*g + s*y;
            x = -s*g + c*y;

            // withu
            for (j=0; j<m; j++) {
                y = matrix_access(u,m,n,j,i-1);
                z = matrix_access(u,m,n,j,i);
                matrix_access(u,m,n,j,i-1) =  y*c + z*s;
                matrix_access(u,m,n,j,i)   = -y*s + z*c;
            } // for j
        } // for i

        e[l] = 0;
        e[k] = f;
        q[k] = x;
        goto test_f_splitting;

        convergence:
        printf("convergence\n");
        printf("  z = %f\n", z);
        if (z < 0) {
            // q[k] is made non-negative
            printf("  q[%3u] = %f\n", k, q[k]);
            q[k] = -z;
            for (j=0; j<n; j++)
                matrix_access(v,n,n,j,k) = -matrix_access(v,n,n,j,k);
        } // if (z < 0)
    } // for k

    // print results
    printf("\n\n");
    printf("A:\n"); matrixf_print(a,m,n);
    printf("U:\n"); matrixf_print(u,m,n);
    printf("V:\n"); matrixf_print(v,n,n);
    printf("q:\n"); matrixf_print(q,n,1);
    printf("e:\n"); matrixf_print(e,n,1);


    printf("done.\n");
    return 0;
}

