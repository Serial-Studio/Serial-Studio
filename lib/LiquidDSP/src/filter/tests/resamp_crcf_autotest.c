/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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

#include "autotest/autotest.h"
#include "liquid.h"

// test single-stage arbitrary resampler
//   r  : resampling rate (output/input)
//   As : resampling filter stop-band attenuation [dB]
void testbench_resamp_crcf(float r, float As, int _id)
{
    // options
    float        bw   = 0.25f;  // target output signal bandwidth
    float        tol  = 0.5f;   // output PSD error tolerance [dB]
    unsigned int m    = 20;     // resampler semi-length
    unsigned int npfb = 2048;   // number of filters in bank
    float        fc   = 0.45f;  // resampler cut-off frequency

    // create resampler
    resamp_crcf resamp = resamp_crcf_create(r,m,fc,As,npfb);

    // generate pulse with sharp transition and very narrow side-lobes
    unsigned int p = (unsigned int) (40.0f / r);
    unsigned int pulse_len = 4*p + 1;
    //printf("pulse len: 4*%u+1 = %u, r=%f, bw=%f\n", p, pulse_len, r, bw);
    float        pulse[pulse_len];
    liquid_firdes_kaiser(pulse_len, 0.5*r*bw, 120, 0, pulse);

    // allocate buffers and copy input
    unsigned int  num_input  = pulse_len + 2*m + 1;
    unsigned int  num_output = resamp_crcf_get_num_output(resamp, num_input);
    float complex buf_0[num_input];  // input buffer
    float complex buf_1[num_output]; // output buffer
    unsigned int i;
    for (i=0; i<num_input; i++)
        buf_0[i] = i < pulse_len ? pulse[i]*bw : 0;

    // resample
    unsigned int nw = 0;
    resamp_crcf_execute_block(resamp, buf_0, num_input, buf_1, &nw);

    // verify result
    autotest_psd_s regions[] = {
        {.fmin=-0.5f,    .fmax=-0.6f*bw, .pmin=0,     .pmax=-As+tol, .test_lo=0, .test_hi=1},
        {.fmin=-0.4f*bw, .fmax=+0.4f*bw, .pmin=0-tol, .pmax= 0 +tol, .test_lo=1, .test_hi=1},
        {.fmin=+0.6f*bw, .fmax=+0.5f,    .pmin=0,     .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/resamp_crcf_%.2d.m", _id);
    liquid_autotest_validate_psd_signal(buf_1, nw, regions, 3,
        liquid_autotest_verbose ? filename : NULL);

    // destroy objects
    resamp_crcf_destroy(resamp);
}

void autotest_resamp_crcf_00() { testbench_resamp_crcf(0.127115323f, 60.0f,  0); }
void autotest_resamp_crcf_01() { testbench_resamp_crcf(0.373737373f, 60.0f,  1); }
void autotest_resamp_crcf_02() { testbench_resamp_crcf(0.676543210f, 60.0f,  2); }
void autotest_resamp_crcf_03() { testbench_resamp_crcf(0.973621947f, 60.0f,  3); }
//void xautotest_resamp_crcf_04() { testbench_resamp_crcf(1.023832447f, 60.0f,  4); }
//void xautotest_resamp_crcf_05() { testbench_resamp_crcf(2.182634827f, 60.0f,  5); }
//void xautotest_resamp_crcf_06() { testbench_resamp_crcf(8.123980823f, 60.0f,  6); }

void autotest_resamp_crcf_10() { testbench_resamp_crcf(0.127115323f, 80.0f, 10); }
void autotest_resamp_crcf_11() { testbench_resamp_crcf(0.373737373f, 80.0f, 11); }
void autotest_resamp_crcf_12() { testbench_resamp_crcf(0.676543210f, 80.0f, 12); }
void autotest_resamp_crcf_13() { testbench_resamp_crcf(0.973621947f, 80.0f, 13); }

// test arbitrary resampler output length calculation
void testbench_resamp_crcf_num_output(float _rate, unsigned int _npfb)
{
    if (liquid_autotest_verbose)
        printf("testing resamp_crcf_get_num_output() with r=%g, npfb=%u\n", _rate, _npfb);

    // create object
    float fc = 0.4f;
    float As = 60.0f;
    unsigned int m = 20;
    resamp_crcf q = resamp_crcf_create(_rate, m, fc, As, _npfb);

    // sizes to test in sequence
    unsigned int sizes[10] = {1, 2, 3, 20, 7, 64, 4, 4, 4, 27};

    // allocate buffers (over-provision output to help avoid segmentation faults on error)
    unsigned int max_input = 64;
    unsigned int max_output = 16 + (unsigned int)(4.0f * max_input * _rate);
    printf("max_input : %u, max_output : %u\n", max_input, max_output);
    float complex buf_0[max_input];
    float complex buf_1[max_output];
    unsigned int i;
    for (i=0; i<max_input; i++)
        buf_0[i] = 0.0f;

    // run numerous blocks
    unsigned int b;
    for (b=0; b<8; b++) {
        for (i=0; i<10; i++) {
            unsigned int num_input  = sizes[i];
            unsigned int num_output = resamp_crcf_get_num_output(q, num_input);
            unsigned int num_written;
            resamp_crcf_execute_block(q, buf_0, num_input, buf_1, &num_written);
            if (liquid_autotest_verbose) {
                printf(" b[%2u][%2u], num_input:%5u, num_output:%5u, num_written:%5u\n",
                        b, i, num_input, num_output, num_written);
            }
            CONTEND_EQUALITY(num_output, num_written)
        }
    }

    // destroy object
    resamp_crcf_destroy(q);
}

void autotest_resamp_crcf_num_output_0(){ testbench_resamp_crcf_num_output(1.00f,     64); }
void autotest_resamp_crcf_num_output_1(){ testbench_resamp_crcf_num_output(1.00f,    256); }
void autotest_resamp_crcf_num_output_2(){ testbench_resamp_crcf_num_output(0.50f,    256); }
void autotest_resamp_crcf_num_output_3(){ testbench_resamp_crcf_num_output(sqrtf( 2),256); }
void autotest_resamp_crcf_num_output_4(){ testbench_resamp_crcf_num_output(sqrtf(17), 16); }
void autotest_resamp_crcf_num_output_5(){ testbench_resamp_crcf_num_output(1.0f/M_PI, 64); }
void autotest_resamp_crcf_num_output_6(){ testbench_resamp_crcf_num_output(expf(5.0f),64); }
void autotest_resamp_crcf_num_output_7(){ testbench_resamp_crcf_num_output(expf(-5.f),64); }

// test copy method
void autotest_resamp_crcf_copy()
{
    // create object with irregular parameters
    float rate = 0.71239213987520f;
    resamp_crcf q0 = resamp_crcf_create(rate, 17, 0.37f, 60.0f, 64);

    // run samples through filter
    unsigned int i, nw0, nw1, num_samples = 80;
    float complex y0, y1;
    for (i=0; i<num_samples; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        resamp_crcf_execute(q0, v, &y0, &nw0);
    }

    // copy object
    resamp_crcf q1 = resamp_crcf_copy(q0);

    // run samples through both filters and check equality
    for (i=0; i<num_samples; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        resamp_crcf_execute(q0, v, &y0, &nw0);
        resamp_crcf_execute(q1, v, &y1, &nw1);

        // check that either 0 or 1 samples were written
        CONTEND_LESS_THAN(nw0, 2);
        CONTEND_LESS_THAN(nw1, 2);

        // check that the same number of samples were written
        CONTEND_EQUALITY(nw0, nw1);

        // check output sample values
        if (nw0==1 && nw1==1)
            CONTEND_EQUALITY(y0, y1);
    }

    // destroy objects
    resamp_crcf_destroy(q0);
    resamp_crcf_destroy(q1);
}

