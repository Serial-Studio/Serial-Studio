/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "autotest/autotest.h"
#include "liquid.h"

void autotest_dsssframe64sync()
{
    // create objects
    unsigned int context = 0;
    dsssframe64gen  fg = dsssframe64gen_create();
    dsssframe64sync fs = dsssframe64sync_create(framing_autotest_callback, (void*)&context);

    // generate the frame
    unsigned int frame_len = dsssframe64gen_get_frame_len(fg);
    float complex * frame = (float complex *)malloc(frame_len*sizeof(float complex));
    dsssframe64gen_execute(fg, NULL, NULL, frame);

    // add some noise
    unsigned int i;
    for (i=0; i<frame_len; i++)
        frame[i] += 1.0f*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;

    // try to receive the frame
    dsssframe64sync_execute(fs, frame, frame_len);

    // ensure callback was invoked
    CONTEND_EQUALITY(context, FRAMING_AUTOTEST_SECRET);

    // parse statistics
    framedatastats_s stats = dsssframe64sync_get_framedatastats(fs);
    CONTEND_EQUALITY(stats.num_frames_detected, 1);
    CONTEND_EQUALITY(stats.num_headers_valid,   1);
    CONTEND_EQUALITY(stats.num_payloads_valid,  1);
    CONTEND_EQUALITY(stats.num_bytes_received, 64);

    // destroy objects and free memory
    dsssframe64gen_destroy(fg);
    dsssframe64sync_destroy(fs);
    free(frame);
}

// test errors and invalid configuration
void autotest_dsssframe64_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping dsssframe64 config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // test copying/creating invalid objects
    CONTEND_ISNULL( dsssframe64gen_copy(NULL) );
    CONTEND_ISNULL( dsssframe64sync_copy(NULL) );

    // create valid objects
    dsssframe64gen  fg = dsssframe64gen_create();
    dsssframe64sync fs = dsssframe64sync_create(NULL, NULL);
    CONTEND_EQUALITY( LIQUID_OK, dsssframe64gen_print(fg) );
    CONTEND_EQUALITY( LIQUID_OK, dsssframe64sync_print(fs) );

    // synchronizer parameters
    CONTEND_EQUALITY( 0,         dsssframe64sync_is_frame_open(fs) );
    CONTEND_EQUALITY( LIQUID_OK, dsssframe64sync_set_callback(fs, NULL) );
    CONTEND_EQUALITY( LIQUID_OK, dsssframe64sync_set_context (fs, NULL) );
    float threshold = 0.123f;
    CONTEND_EQUALITY( LIQUID_OK, dsssframe64sync_set_threshold(fs, threshold) );
    CONTEND_EQUALITY( threshold, dsssframe64sync_get_threshold(fs) );
    float range = 0.00722f;
    CONTEND_EQUALITY( LIQUID_OK, dsssframe64sync_set_range(fs, range) );
    CONTEND_EQUALITY( range,     dsssframe64sync_get_range(fs) );

    dsssframe64gen_destroy(fg);
    dsssframe64sync_destroy(fs);
}

// test that the complete internal state of one generator can be copied to a new
// object
void autotest_dsssframe64gen_copy()
{
    // create object and copy
    dsssframe64gen q0 = dsssframe64gen_create();
    dsssframe64gen q1 = dsssframe64gen_copy(q0);

    // allocate buffers for frames
    unsigned int frame_len = dsssframe64gen_get_frame_len(q0);
    float complex * buf_0 = (float complex *)malloc(frame_len*sizeof(float complex));
    float complex * buf_1 = (float complex *)malloc(frame_len*sizeof(float complex));
    unsigned char   header [ 8] = {0,0,0,0,0,0,0,0,};
    unsigned char   payload[64] = {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

    // encode
    dsssframe64gen_execute(q0, header, payload, buf_0);
    dsssframe64gen_execute(q1, header, payload, buf_1);

    // ensure identical outputs
    CONTEND_SAME_DATA(buf_0, buf_1, frame_len*sizeof(float complex));

    // destroy objects and free memory
    dsssframe64gen_destroy(q0);
    dsssframe64gen_destroy(q1);
    free(buf_0);
    free(buf_1);
}

// test that the complete internal state of one synchronizer can be copied to a new
// object and it can maintain state
void autotest_dsssframe64sync_copy()
{
    // create object and generte frame
    dsssframe64gen fg = dsssframe64gen_create();
    unsigned int    frame_len = dsssframe64gen_get_frame_len(fg);
    float complex * frame= (float complex *)malloc(frame_len*sizeof(float complex));
    dsssframe64gen_execute(fg, NULL, NULL, frame);

    // creamte original frame synchronizer
    dsssframe64sync q0 = dsssframe64sync_create(NULL, NULL);

    // run half of frame through synchronizer
    unsigned int n = 12000;
    dsssframe64sync_execute(q0, frame, n);

    // ensure frame was not yet decoded
    framedatastats_s s0, s1;
    s0 = dsssframe64sync_get_framedatastats(q0);
    CONTEND_EQUALITY(s0.num_frames_detected, 0);

    // copy object
    dsssframe64sync q1 = dsssframe64sync_copy(q0);

    // run remaining half of frame through synchronizers
    dsssframe64sync_execute(q0, frame+n, frame_len-n);
    dsssframe64sync_execute(q1, frame+n, frame_len-n);

    // ensure frame was decoded by both synchronizers
    s0 = dsssframe64sync_get_framedatastats(q0);
    CONTEND_EQUALITY(s0.num_frames_detected, 1);
    CONTEND_EQUALITY(s0.num_headers_valid,   1);
    CONTEND_EQUALITY(s0.num_payloads_valid,  1);
    CONTEND_EQUALITY(s0.num_bytes_received, 64);

    s1 = dsssframe64sync_get_framedatastats(q1);
    CONTEND_EQUALITY(s1.num_frames_detected, 1);
    CONTEND_EQUALITY(s1.num_headers_valid,   1);
    CONTEND_EQUALITY(s1.num_payloads_valid,  1);
    CONTEND_EQUALITY(s1.num_bytes_received, 64);

    // destroy objects and free memory
    dsssframe64gen_destroy(fg);
    dsssframe64sync_destroy(q0);
    dsssframe64sync_destroy(q1);
    free(frame);
}

