// quad-section search in one dimension
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

float utility(float _v, void * _context)
{
    float v_opt = *(float*)(_context);
    float v = _v - v_opt;
    return tanhf(v)*tanhf(v);
}

int main()
{
    // create qs1dsearch object
    float v_opt  = 0.0f;
    qs1dsearch q = qs1dsearch_create(utility, &v_opt, LIQUID_OPTIM_MINIMIZE);

    //qs1dsearch_init_bounds(q, -20, 10);
    qs1dsearch_init(q, -50);

    // run search
    unsigned int i;
    for (i=0; i<32; i++) {
        qs1dsearch_step(q);
        qs1dsearch_print(q);
    }

    // print results
    printf("%3u : u(%12.8f) = %12.4e, v_opt=%12.4e (error=%12.4e)\n",
        i,
        qs1dsearch_get_opt_v(q),
        qs1dsearch_get_opt_u(q),
        v_opt,
        v_opt - qs1dsearch_get_opt_v(q));

    qs1dsearch_destroy(q);
    return 0;
}
