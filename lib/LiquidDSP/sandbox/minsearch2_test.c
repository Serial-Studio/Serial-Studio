// 
// minsearch2.c
//
// search to find minimum of a function using parabolic
// interpolation using mid-point between x0 and x2
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

double function(double _x) {
#if 0
    // -sin(x) minimum at x = pi/2
    double y = 1.0f-sin(_x);
#else
    // exp(-(x-pi/2)^2)
    double t = _x - M_PI/2;
    double y = 1-exp(-t*t);
#endif
    return y;
}

int main() {
    // options
    unsigned int num_iterations = 10;
    double x0;          // lower value
    double x1 = 1.2f;   // middle value (initial guess)
    double x2;          // upper value
    double dx = 2.0f;   // step size
    double gamma = 0.7f;// step size reduction
    double tol = 1e-6f; // error tolerance

    // choose x0, x2
    x0 = x1 - dx;
    x2 = x1 + dx;

    //
    double y0, y1, y2;

    double x_hat;
    double del = 0.0f;
    unsigned int i;
    for (i=0; i<num_iterations; i++) {
        // choose center point between [x0,x2]
        x1 = 0.5f*(x0 + x2);

        // compute functions
        y0 = function(x0);
        y1 = function(x1);
        y2 = function(x2);

#if 0
        printf("%4u : (%8.3f,%8.3f,%8.3f) (%8.3f,%8.3f,%8.3f)\n",
                i,
                x0, x1, x2,
                y0, y1, y2);
#endif

        // ensure values are reasonable
        assert(x2 > x1);
        assert(x1 > x0);

        // compute minimum
        // TODO : exploit uniform spacing...
        double t0 = y0*(x1*x1 - x2*x2) +
                    y1*(x2*x2 - x0*x0) +
                    y2*(x0*x0 - x1*x1);

        double t1 = y0*(x1 - x2) +
                    y1*(x2 - x0) +
                    y2*(x0 - x1);

        x_hat = 0.5f * t0 / t1;
        del = x_hat - x1;

        // print
        printf("%4u : %12.8f, e=%12.4e, del: %12.4e\n", i, x_hat, x_hat-M_PI/2, del);

        // reduce step size
        dx *= gamma;

        // 
        x0 = x_hat - dx;
        x2 = x_hat + dx;

        if (fabs(del) < tol)
            break;
    }

    printf("estimated minimum : %f (%f)\n", x_hat, M_PI/2);

    return 0;
}
