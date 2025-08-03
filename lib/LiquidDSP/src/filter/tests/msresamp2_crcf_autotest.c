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
void testbench_msresamp2_crcf_interp(unsigned int _num_stages,
                                     float        _fc,
                                     float        _as)
{
    // create and configure objects
    msresamp2_crcf resamp = msresamp2_crcf_create(
        LIQUID_RESAMP_INTERP, _num_stages, _fc, 0.0f, _as);
    float delay = msresamp2_crcf_get_delay(resamp);

    // generate samples and push through spgram object
    unsigned int  M = 1 << _num_stages; // interpolation rate
    unsigned int  buf_len = 0;
    unsigned int  num_blocks = 0;
    while ((float)buf_len < 2*M*delay) {
        buf_len += M;
        num_blocks++;
    }
    float complex buf[buf_len]; // input buffer
    unsigned int i;
    for (i=0; i<num_blocks; i++) {
        float complex x = (i==0) ? 1.0f : 0.0f;

        // generate block of samples
        msresamp2_crcf_execute(resamp, &x, buf+ i*M);
    }

    // scale by samples/symbol
    liquid_vectorcf_mulscalar(buf, buf_len, 1.0f/(float)M, buf);
#if 0
    for (i=0; i<buf_len; i++)
        printf("%3u : %12.8f %12.8f\n", i, crealf(buf[i]), cimagf(buf[i]));
#endif

    // verify result
    float f0 = _fc / (float)M;
    float f1 = 1.0f / (float)M - f0;
    autotest_psd_s regions[] = {
        {.fmin=-0.5f, .fmax=-f1,   .pmin=   0, .pmax=-_as, .test_lo=0, .test_hi=1},
        {.fmin=  -f0, .fmax= f0,   .pmin=-0.1, .pmax= 0.1, .test_lo=1, .test_hi=1},
        {.fmin=   f1, .fmax= 0.5f, .pmin=   0, .pmax=-_as, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/msresamp2_crcf_interp_M%u_f%.3u_a%u.m",
        M, (int)(_fc*1000), (int)_as);
    liquid_autotest_validate_psd_signal(buf, buf_len, regions, 3,
        liquid_autotest_verbose ? filename : NULL);

    // destroy objects
    msresamp2_crcf_destroy(resamp);
}

void autotest_msresamp2_crcf_interp_01() { testbench_msresamp2_crcf_interp(1, 0.25f, 60.0f); }
void autotest_msresamp2_crcf_interp_02() { testbench_msresamp2_crcf_interp(2, 0.25f, 60.0f); }
void autotest_msresamp2_crcf_interp_03() { testbench_msresamp2_crcf_interp(3, 0.25f, 60.0f); }
void autotest_msresamp2_crcf_interp_04() { testbench_msresamp2_crcf_interp(4, 0.25f, 60.0f); }

void autotest_msresamp2_crcf_interp_05() { testbench_msresamp2_crcf_interp(1, 0.45f, 60.0f); }
void autotest_msresamp2_crcf_interp_06() { testbench_msresamp2_crcf_interp(2, 0.45f, 60.0f); }
void autotest_msresamp2_crcf_interp_07() { testbench_msresamp2_crcf_interp(3, 0.45f, 60.0f); }
void autotest_msresamp2_crcf_interp_08() { testbench_msresamp2_crcf_interp(4, 0.45f, 60.0f); }

void autotest_msresamp2_crcf_interp_09() { testbench_msresamp2_crcf_interp(3, 0.45f, 80.0f); }
void autotest_msresamp2_crcf_interp_10() { testbench_msresamp2_crcf_interp(3, 0.45f, 90.0f); }
void autotest_msresamp2_crcf_interp_11() { testbench_msresamp2_crcf_interp(3, 0.45f,100.0f); }
//void xautotest_msresamp2_crcf_interp_12() { testbench_msresamp2_crcf_interp(3, 0.45f,120.0f); }

// test copy method
void autotest_msresamp2_copy()
{
    // create original resampler
    unsigned int num_stages = 4;
    msresamp2_crcf q0 = msresamp2_crcf_create(
        LIQUID_RESAMP_INTERP, num_stages, 0.4f, 0.0f, 60.0f);

    // allocate buffers for output
    unsigned int M = 1 << num_stages; // interpolation factor
    float complex v, y0[M], y1[M];

    // push samples through original object
    unsigned int i, num_samples = 35;
    for (i=0; i<num_samples; i++) {
        v = randnf() + _Complex_I*randnf();
        msresamp2_crcf_execute(q0, &v, y0);
    }

    // copy object
    msresamp2_crcf q1 = msresamp2_crcf_copy(q0);

    // run random samples through both filters and compare
    for (i=0; i<num_samples; i++) {
        v = randnf() + _Complex_I*randnf();
        msresamp2_crcf_execute(q0, &v, y0);
        msresamp2_crcf_execute(q1, &v, y1);
        CONTEND_SAME_DATA(y0, y1, M*sizeof(float complex));
    }

    // clean up allocated objects
    msresamp2_crcf_destroy(q0);
    msresamp2_crcf_destroy(q1);
}

