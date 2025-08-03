//
// throttle_test.c
//
// Test throttling down processor to reach target rate by inserting sleep
// statements in between processing blocks
//

#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "liquid.h"

int main() {
    // options
    float target_rate = 200e3f;    // target processing rate [samples/second]

    // design filter from prototype
    firfilt_crcf filter = firfilt_crcf_create_kaiser(25, 0.25f, 60.0f, 0.0f);

    // generate dummy buffers for processing
    unsigned int  buf_len = 1024;
    float complex buf_0[buf_len];
    float complex buf_1[buf_len];
    memset(buf_0, 0x00, buf_len*sizeof(complex float));

    // run with updates
    struct timespec t0, t1;
    float sleep_time = 200.0f; // initial guess
    unsigned int i;
    for (i=0; i<100; i++) {
        // run block of samples for 200 ms
        unsigned int num_samples_processed = 0;
        clock_gettime(CLOCK_REALTIME, &t0);
        float block_time = 0;
        while (block_time < 0.2) {
            // process a block of samples
            firfilt_crcf_execute_block(filter, buf_0, buf_len, buf_1);
            num_samples_processed += buf_len;

            // throttle by calling sleep
            usleep((unsigned int)sleep_time);

            // update time reference
            clock_gettime(CLOCK_REALTIME, &t1);
            block_time = t1.tv_sec - t0.tv_sec + 1e-9*(t1.tv_nsec - t0.tv_nsec);
        }

        // get processing rate
        float rate = (float)num_samples_processed / block_time;
        printf("  %6u, rate = %12.3f k (target = %12.3f k), sleep time = %12.1f us\r",
                i, rate*1e-3f, target_rate*1e-3f, sleep_time);
        fflush(stdout);

        // adjust sleep time proportional to deviation from target rate
        sleep_time *= expf(0.2f*logf(rate/target_rate));
    }
    printf("\n");

    // destroy filter object
    firfilt_crcf_destroy(filter);
    return 0;
}

