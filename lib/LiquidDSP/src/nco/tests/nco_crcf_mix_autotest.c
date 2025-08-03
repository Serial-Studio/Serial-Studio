/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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

#include <stdlib.h>
#include <complex.h>
#include "autotest/autotest.h"
#include "liquid.h"

void testbench_nco_crcf_mix(int   _type,
                            float _phase,
                            float _frequency)
{
    // options
    unsigned int buf_len = 1200;
    float        tol     = 1e-2f;

    // create and initialize object
    nco_crcf nco = nco_crcf_create(_type);
    nco_crcf_set_phase    (nco, _phase);
    nco_crcf_set_frequency(nco, _frequency);

    // generate signal (pseudo-random)
    float complex buf_0[buf_len];
    float complex buf_1[buf_len];
    unsigned int i;
    for (i=0; i<buf_len; i++)
        buf_0[i] = cexpf(_Complex_I*2*M_PI*randf());

    // mix signal
    nco_crcf_mix_block_up(nco, buf_0, buf_1, buf_len);

    // compare result to expected
    float theta = _phase;
    for (i=0; i<buf_len; i++) {
        float complex v = buf_0[i] * cexpf(_Complex_I*theta);
        CONTEND_DELTA( crealf(buf_1[i]), crealf(v), tol);
        CONTEND_DELTA( cimagf(buf_1[i]), cimagf(v), tol);

        // update and constrain phase
        theta += _frequency;
        while (theta >  M_PI) { theta -= 2*M_PI; }
        while (theta < -M_PI) { theta += 2*M_PI; }
    }

    // destroy object
    nco_crcf_destroy(nco);
}

// test NCO mixing
void autotest_nco_crcf_mix_nco_0() { testbench_nco_crcf_mix(LIQUID_NCO,  0.000f,  0.000f); }
void autotest_nco_crcf_mix_nco_1() { testbench_nco_crcf_mix(LIQUID_NCO,  1.234f,  0.000f); }
void autotest_nco_crcf_mix_nco_2() { testbench_nco_crcf_mix(LIQUID_NCO, -1.234f,  0.000f); }
void autotest_nco_crcf_mix_nco_3() { testbench_nco_crcf_mix(LIQUID_NCO, 99.000f,  0.000f); }
void autotest_nco_crcf_mix_nco_4() { testbench_nco_crcf_mix(LIQUID_NCO,    M_PI,  0.000f); }
void autotest_nco_crcf_mix_nco_5() { testbench_nco_crcf_mix(LIQUID_NCO,  0.000f,    M_PI); }
void autotest_nco_crcf_mix_nco_6() { testbench_nco_crcf_mix(LIQUID_NCO,  0.000f,   -M_PI); }
void autotest_nco_crcf_mix_nco_7() { testbench_nco_crcf_mix(LIQUID_NCO,  0.000f,  0.123f); }
void autotest_nco_crcf_mix_nco_8() { testbench_nco_crcf_mix(LIQUID_NCO,  0.000f, -0.123f); }
void autotest_nco_crcf_mix_nco_9() { testbench_nco_crcf_mix(LIQUID_NCO,  0.000f,  1e-5f ); }

// test VCO mixing
void autotest_nco_crcf_mix_vco_0() { testbench_nco_crcf_mix(LIQUID_VCO,  0.000f,  0.000f); }
void autotest_nco_crcf_mix_vco_1() { testbench_nco_crcf_mix(LIQUID_VCO,  1.234f,  0.000f); }
void autotest_nco_crcf_mix_vco_2() { testbench_nco_crcf_mix(LIQUID_VCO, -1.234f,  0.000f); }
void autotest_nco_crcf_mix_vco_3() { testbench_nco_crcf_mix(LIQUID_VCO, 99.000f,  0.000f); }
void autotest_nco_crcf_mix_vco_4() { testbench_nco_crcf_mix(LIQUID_VCO,    M_PI,  0.000f); }
void autotest_nco_crcf_mix_vco_5() { testbench_nco_crcf_mix(LIQUID_VCO,  0.000f,    M_PI); }
void autotest_nco_crcf_mix_vco_6() { testbench_nco_crcf_mix(LIQUID_VCO,  0.000f,   -M_PI); }
void autotest_nco_crcf_mix_vco_7() { testbench_nco_crcf_mix(LIQUID_VCO,  0.000f,  0.123f); }
void autotest_nco_crcf_mix_vco_8() { testbench_nco_crcf_mix(LIQUID_VCO,  0.000f, -0.123f); }
void autotest_nco_crcf_mix_vco_9() { testbench_nco_crcf_mix(LIQUID_VCO,  0.000f,  1e-5f ); }

