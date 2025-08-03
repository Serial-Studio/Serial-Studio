/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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

// test spectral waterfall (spwaterfall) objects

#include <stdlib.h>
#include <string.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

void autotest_spwaterfall_invalid_config()
{
#ifdef LIQUID_STRICT_EXIT
    AUTOTEST_WARN("spwaterfall test not run with strict mode enabled");
    return;
#endif
    AUTOTEST_WARN("testing spwaterfall invalid configurations; ignore printed errors");
    // default configurations
    unsigned int nfft  = 1200;
    int          wtype = LIQUID_WINDOW_HAMMING;
    unsigned int wlen  =  800;
    unsigned int delay =  200;
    unsigned int time  =  960;

    // test invalid configurations, normal construction
    CONTEND_ISNULL(spwaterfallcf_create(   1, wtype,   wlen, delay, time))
    CONTEND_ISNULL(spwaterfallcf_create(nfft, wtype,      0, delay, time))
    CONTEND_ISNULL(spwaterfallcf_create(nfft, wtype, nfft+1, delay, time))
    CONTEND_ISNULL(spwaterfallcf_create(nfft, LIQUID_WINDOW_KBD, 801, delay, time))
    CONTEND_ISNULL(spwaterfallcf_create(nfft, wtype,   wlen,     0, time))
    CONTEND_ISNULL(spwaterfallcf_create(nfft, wtype,   wlen, delay,    0))

    // test invalid configurations, default construction
    CONTEND_ISNULL(spwaterfallcf_create_default(   0, time))
    CONTEND_ISNULL(spwaterfallcf_create_default(nfft,    0))

    // create proper object but test invalid internal configurations
    spwaterfallcf q = spwaterfallcf_create_default(540, 320);

    CONTEND_INEQUALITY(LIQUID_OK, spwaterfallcf_set_rate(q, -10e6))

    spwaterfallcf_destroy(q);
}

int testbench_spwaterfallcf_compare(const void * _v0, const void * _v1)
    { return *(float*)_v0 > *(float*)_v1 ? 1 : -1; }

void testbench_spwaterfallcf_noise(unsigned int _nfft,
                                   unsigned int _window_len,
                                   unsigned int _delay,
                                   unsigned int _time,
                                   float        _noise_floor)
{
    unsigned int num_samples = 4*_nfft*_time;  // number of samples to generate
    float        nstd        = powf(10.0f,_noise_floor/20.0f); // noise std. dev.
    float        tol         = 4.0f; // error tolerance [dB] TODO: drop tolerance to 0.5 dB
    int _wtype = LIQUID_WINDOW_HAMMING;

    // create spectral periodogram
    spwaterfallcf q = spwaterfallcf_create(_nfft, _wtype, _window_len, _delay, _time);

    unsigned int i;
    for (i=0; i<num_samples; i++)
        spwaterfallcf_push(q, nstd*( randnf() + _Complex_I*randnf() ) * M_SQRT1_2);

    // verify number of samples processed
    CONTEND_EQUALITY(spwaterfallcf_get_num_samples_total(q), num_samples);

    // compute power spectral density output
    const float * psd = spwaterfallcf_get_psd(q);
    unsigned int time = spwaterfallcf_get_num_time(q);

#if 0
    // export results for testing
    FILE * fid = fopen("spwaterfallcf_testbench.m", "w");
    fprintf(fid,"clear all; close all; psd=zeros(%u,%u);\n", _nfft, time);
    for (i=0; i<_nfft*time; i++)
        fprintf(fid,"psd(%u) = %g;\n", i+1, psd[i]);
    fclose(fid);
#endif
    // compute median value; should be close to noise floor
    float * v = (float*) malloc(_nfft*time*sizeof(float));
    memmove(v, psd, _nfft*time*sizeof(float));
    qsort(v, _nfft*time, sizeof(float), &testbench_spwaterfallcf_compare);
    float median = v[_nfft*time/2];
    if (liquid_autotest_verbose) {
        printf("  spwaterfallcf_test(noise): nfft:%4u, wtype:%s, n0:%6.1f, est:%6.1f, tol:%5.2f\n",
                _nfft, liquid_window_str[_wtype][1], _noise_floor, median, tol);
    }
    CONTEND_DELTA(median, _noise_floor, tol)

    // destroy objects and free memory
    free(v);
    spwaterfallcf_destroy(q);
}

// test different transform sizes
void autotest_spwaterfallcf_noise_440()  { testbench_spwaterfallcf_noise( 440, 320, 100, 240, -80.0); }
void autotest_spwaterfallcf_noise_1024() { testbench_spwaterfallcf_noise( 680, 480, 150, 640, -80.0); }
void autotest_spwaterfallcf_noise_1200() { testbench_spwaterfallcf_noise(1200, 800, 400, 800, -80.0); }

// test normal operation
void autotest_spwaterfall_operation()
{
    // create default object
    spwaterfallcf q = spwaterfallcf_create(1200, LIQUID_WINDOW_HAMMING, 800, 10, 960);
    spwaterfallcf_print(q);
    CONTEND_EQUALITY(spwaterfallcf_get_num_freq(q), 1200);
    CONTEND_EQUALITY(spwaterfallcf_get_num_time(q),    0);
    CONTEND_EQUALITY(spwaterfallcf_get_window_len(q),800);
    CONTEND_EQUALITY(spwaterfallcf_get_delay(q),      10);
    CONTEND_EQUALITY(spwaterfallcf_get_wtype(q), LIQUID_WINDOW_HAMMING);

    // push individual samples
    spwaterfallcf_push(q, randnf() + _Complex_I*randnf());
    spwaterfallcf_push(q, randnf() + _Complex_I*randnf());

    CONTEND_EQUALITY(spwaterfallcf_get_num_samples_total(q), 2);
    spwaterfallcf_clear(q);
    CONTEND_EQUALITY(spwaterfallcf_get_num_samples_total(q), 2);
    spwaterfallcf_reset(q);
    CONTEND_EQUALITY(spwaterfallcf_get_num_samples_total(q), 0);

    // write a block of samples
    float complex buf[12];
    unsigned int i;
    for (i=0; i<12; i++)
        buf[i] = randnf() + _Complex_I*randnf();
    spwaterfallcf_write(q, buf, 12);
    CONTEND_EQUALITY(spwaterfallcf_get_num_samples_total(q), 12);

    spwaterfallcf_destroy(q);
}

void autotest_spwaterfall_copy()
{
    unsigned int nfft =  240;   // transform size
    unsigned int time =  192;   // time size
    float        nstd =  0.1f;  // noise standard deviation

    // create object with irregular values
    spwaterfallcf q0 = spwaterfallcf_create(nfft, LIQUID_WINDOW_KAISER, 217, 137, time);

    unsigned int i;
    unsigned int num_samples = 17 * nfft * time;
    for (i=0; i<num_samples; i++) {
        float complex v = 0.1f + nstd * (randnf() + _Complex_I*randnf());
        spwaterfallcf_push(q0, v);
    }

    // copy object and push same samples through both
    spwaterfallcf q1 = spwaterfallcf_copy(q0);
    for (i=0; i<num_samples; i++) {
        float complex v = 0.1f + nstd * (randnf() + _Complex_I*randnf());
        spwaterfallcf_push(q0, v);
        spwaterfallcf_push(q1, v);
    }

    // check parameters
    CONTEND_EQUALITY(spwaterfallcf_get_num_freq         (q0),spwaterfallcf_get_num_freq         (q1));
    CONTEND_EQUALITY(spwaterfallcf_get_num_time         (q0),spwaterfallcf_get_num_time         (q1));
    CONTEND_EQUALITY(spwaterfallcf_get_window_len       (q0),spwaterfallcf_get_window_len       (q1));
    CONTEND_EQUALITY(spwaterfallcf_get_delay            (q0),spwaterfallcf_get_delay            (q1));
    CONTEND_EQUALITY(spwaterfallcf_get_wtype            (q0),spwaterfallcf_get_wtype            (q1));
    CONTEND_EQUALITY(spwaterfallcf_get_num_samples_total(q0),spwaterfallcf_get_num_samples_total(q1));

    // compute power spectral density output
    const float * psd_0 = spwaterfallcf_get_psd(q0);
    const float * psd_1 = spwaterfallcf_get_psd(q1);
    unsigned int nt = spwaterfallcf_get_num_time(q0);
    CONTEND_SAME_DATA(psd_0, psd_1, nfft*nt*sizeof(float));

    // destroy objects and free memory
    spwaterfallcf_destroy(q0);
    spwaterfallcf_destroy(q1);
}

// test file export
void autotest_spwaterfall_gnuplot()
{
    // create default object
    spwaterfallcf q = spwaterfallcf_create_default(540, 320);
    unsigned int i;
    for (i=0; i<800000; i++)
        spwaterfallcf_push(q, randnf() + _Complex_I*randnf());

    // export once before setting values
    CONTEND_EQUALITY(LIQUID_OK,spwaterfallcf_export(q,"autotest/logs/spwaterfall"))

    // set values and export again
    CONTEND_EQUALITY(LIQUID_OK,spwaterfallcf_set_freq(q, 100e6))
    CONTEND_EQUALITY(LIQUID_OK,spwaterfallcf_set_rate(q,  20e6))
    CONTEND_EQUALITY(LIQUID_OK,spwaterfallcf_set_dims(q, 640, 480))
    CONTEND_EQUALITY(LIQUID_OK, spwaterfallcf_set_commands(q,NULL))
    CONTEND_EQUALITY(LIQUID_OK,spwaterfallcf_set_commands(q,"set title 'waterfall'"))
    CONTEND_EQUALITY(LIQUID_OK,spwaterfallcf_export(q,"autotest/logs/spwaterfall"))

    spwaterfallcf_destroy(q);
}
