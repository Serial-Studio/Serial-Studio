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

// test multi-stage arbitrary resampler
//   r  : resampling rate (output/input)
//   As : resampling filter stop-band attenuation [dB]
void testbench_msresamp_crcf(float r, float As)
{
    // options
    unsigned int n=800000;      // number of output samples to analyze
    float bw = 0.2f; // target output bandwidth
    unsigned int nfft = 800;
    float tol = 0.5f;

    // create and configure objects
    spgramcf     q   = spgramcf_create(nfft, LIQUID_WINDOW_HANN, nfft/2, nfft/4);
    symstreamrcf gen = symstreamrcf_create_linear(LIQUID_FIRFILT_KAISER,r*bw,25,0.2f,LIQUID_MODEM_QPSK);
    symstreamrcf_set_gain(gen, sqrtf(bw));
    msresamp_crcf resamp = msresamp_crcf_create(r,As);

    // generate samples and push through spgram object
    unsigned int  buf_len = 256;
    float complex buf_0[buf_len]; // input buffer
    float complex buf_1[buf_len]; // output buffer
    while (spgramcf_get_num_samples_total(q) < n) {
        // generate block of samples
        symstreamrcf_write_samples(gen, buf_0, buf_len);

        // resample
        unsigned int nw = 0;
        msresamp_crcf_execute(resamp, buf_0, buf_len, buf_1, &nw);

        // run samples through the spgram object
        spgramcf_write(q, buf_1, nw);
    }

    // verify result
    float psd[nfft];
    spgramcf_get_psd(q, psd);
    autotest_psd_s regions[] = {
        {.fmin=-0.5f,    .fmax=-0.6f*bw, .pmin=0,     .pmax=-As+tol, .test_lo=0, .test_hi=1},
        {.fmin=-0.4f*bw, .fmax=+0.4f*bw, .pmin=0-tol, .pmax= 0 +tol, .test_lo=1, .test_hi=1},
        {.fmin=+0.6f*bw, .fmax=+0.5f,    .pmin=0,     .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/msresamp_crcf_r%.3u_a%.2u_autotest.m", (int)(r*1000), (int)(As));
    liquid_autotest_validate_spectrum(psd, nfft, regions, 3,
        liquid_autotest_verbose ? filename : NULL);

    // destroy objects
    spgramcf_destroy(q);
    symstreamrcf_destroy(gen);
    msresamp_crcf_destroy(resamp);
}

void autotest_msresamp_crcf_01() { testbench_msresamp_crcf(0.127115323f, 60.0f); }
void autotest_msresamp_crcf_02() { testbench_msresamp_crcf(0.373737373f, 60.0f); }
void autotest_msresamp_crcf_03() { testbench_msresamp_crcf(0.676543210f, 60.0f); }
//void xautotest_msresamp_crcf_04() { testbench_msresamp_crcf(0.127115323f,80.0f); }

// test arbitrary resampler output length calculation
void testbench_msresamp_crcf_num_output(float _rate)
{
    if (liquid_autotest_verbose)
        printf("testing msresamp_crcf_get_num_output() with r=%g\n", _rate);

    // create object
    float As = 60.0f;
    msresamp_crcf q = msresamp_crcf_create(_rate, As);

    // sizes to test in sequence
    unsigned int s = _rate < 0.1f ? 131 : 1; // scale: increase for large decimation rates
    unsigned int sizes[10] = {1*s, 2*s, 3*s, 20*s, 7*s, 64*s, 4*s, 4*s, 4*s, 27*s};

    // allocate buffers (over-provision output to help avoid segmentation faults on error)
    unsigned int max_input = 64*s;
    unsigned int max_output = 16 + (unsigned int)(4.0f * max_input * _rate);
    printf("max_input : %u, max_output : %u\n", max_input, max_output);
    float complex * buf_0 = (float complex*) malloc(max_input  * sizeof(float complex));
    float complex * buf_1 = (float complex*) malloc(max_output * sizeof(float complex));
    unsigned int i;
    for (i=0; i<max_input; i++)
        buf_0[i] = 0.0f;

    // run numerous blocks
    unsigned int b;
    for (b=0; b<8; b++) {
        for (i=0; i<10; i++) {
            unsigned int num_input  = sizes[i];
            unsigned int num_output = msresamp_crcf_get_num_output(q, num_input);
            unsigned int num_written;
            msresamp_crcf_execute(q, buf_0, num_input, buf_1, &num_written);
            if (liquid_autotest_verbose) {
                printf(" b[%2u][%2u], num_input:%5u, num_output:%5u, num_written:%5u\n",
                        b, i, num_input, num_output, num_written);
            }
            CONTEND_EQUALITY(num_output, num_written)
        }
    }

    // destroy object
    free(buf_0);
    free(buf_1);
    msresamp_crcf_destroy(q);
}

void autotest_msresamp_crcf_num_output_0(){ testbench_msresamp_crcf_num_output(1.00f);      }
void autotest_msresamp_crcf_num_output_1(){ testbench_msresamp_crcf_num_output(1e3f);       }
void autotest_msresamp_crcf_num_output_2(){ testbench_msresamp_crcf_num_output(1e-3f);      }
void autotest_msresamp_crcf_num_output_3(){ testbench_msresamp_crcf_num_output(sqrtf( 2));  }
void autotest_msresamp_crcf_num_output_4(){ testbench_msresamp_crcf_num_output(sqrtf(17));  }
void autotest_msresamp_crcf_num_output_5(){ testbench_msresamp_crcf_num_output(1.0f/M_PI);  }
void autotest_msresamp_crcf_num_output_6(){ testbench_msresamp_crcf_num_output(expf(8.0f)); }
void autotest_msresamp_crcf_num_output_7(){ testbench_msresamp_crcf_num_output(expf(-8.f)); }

// test copy method
void autotest_msresamp_crcf_copy()
{
    // create initial object
    float rate = 0.071239213987520f;
    msresamp_crcf q0 = msresamp_crcf_create(rate, 60.0f);

    // run samples through filter
    unsigned int i, nw_0, nw_1, buf_len = 640;
    float complex buf  [buf_len]; // input buffer
    float complex buf_0[buf_len];
    float complex buf_1[buf_len];
    for (i=0; i<buf_len; i++)
        buf[i] = randnf() + _Complex_I*randnf();
    msresamp_crcf_execute(q0, buf, buf_len, buf_0, &nw_0);

    // copy object
    msresamp_crcf q1 = msresamp_crcf_copy(q0);

    // run samples through both filters and check equality
    for (i=0; i<buf_len; i++)
        buf[i] = randnf() + _Complex_I*randnf();

    msresamp_crcf_execute(q0, buf, buf_len, buf_0, &nw_0);
    msresamp_crcf_execute(q1, buf, buf_len, buf_1, &nw_1);

    // check that the same number of samples were written
    CONTEND_EQUALITY(nw_0, nw_1);

    // check output sample values
    CONTEND_SAME_DATA(buf_0, buf_1, nw_0*sizeof(float complex));

    // destroy objects
    msresamp_crcf_destroy(q0);
    msresamp_crcf_destroy(q1);
}

