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

void autotest_firfilt_crcf_kaiser()
{
    // design filter
    firfilt_crcf q = firfilt_crcf_create_kaiser(51, 0.2f, 60.0f, 0.0f);
    firfilt_crcf_set_scale(q, 0.4f);

    // verify resulting spectrum
    autotest_psd_s regions[] = {
      {.fmin=-0.5,   .fmax=-0.25,  .pmin= 0,   .pmax=-60,  .test_lo=0, .test_hi=1},
      {.fmin=-0.15,  .fmax=+0.15,  .pmin=-0.1, .pmax=+0.1, .test_lo=1, .test_hi=1},
      {.fmin= 0.25,  .fmax=+0.5,   .pmin= 0,   .pmax=-60,  .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_firfilt_crcf(q, 1200, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/firfilt_crcf_kaiser.m" : NULL);
    firfilt_crcf_destroy(q);
}

void autotest_firfilt_crcf_firdespm()
{
    // design filter
    firfilt_crcf q = firfilt_crcf_create_firdespm(51, 0.2f, 60.0f);
    firfilt_crcf_set_scale(q, 0.4f);

    // verify resulting spectrum
    autotest_psd_s regions[] = {
      {.fmin=-0.5,   .fmax=-0.25,  .pmin= 0,   .pmax=-60,  .test_lo=0, .test_hi=1},
      {.fmin=-0.15,  .fmax=+0.15,  .pmin=-0.1, .pmax=+0.1, .test_lo=1, .test_hi=1},
      {.fmin= 0.25,  .fmax=+0.5,   .pmin= 0,   .pmax=-60,  .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_firfilt_crcf(q, 1200, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/firfilt_crcf_firdespm.m" : NULL);
    firfilt_crcf_destroy(q);
}

void autotest_firfilt_crcf_rect()
{
    // design filter
    firfilt_crcf q = firfilt_crcf_create_rect(4);
    firfilt_crcf_set_scale(q, 0.25f);

    // verify resulting spectrum
    autotest_psd_s regions[] = {
      {.fmin=-0.5,   .fmax=-0.20,  .pmin= 0, .pmax=-10, .test_lo=0, .test_hi=1},
      {.fmin=-0.12,  .fmax=+0.12,  .pmin=-5, .pmax= +1, .test_lo=1, .test_hi=1},
      {.fmin= 0.20,  .fmax=+0.5,   .pmin= 0, .pmax=-10, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_firfilt_crcf(q, 301, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/firfilt_crcf_rect.m" : NULL);
    firfilt_crcf_destroy(q);
}

void autotest_firfilt_crcf_notch()
{
    // design filter and verify resulting spectrum
    firfilt_crcf q = firfilt_crcf_create_notch(20, 60.0f, 0.125f);
    autotest_psd_s regions[] = {
      {.fmin=-0.5,   .fmax=-0.20,  .pmin=-0.1, .pmax=+0.1, .test_lo=1, .test_hi=1},
      {.fmin=-0.126, .fmax=-0.124, .pmin=   0, .pmax= -50, .test_lo=0, .test_hi=1},
      {.fmin=-0.06,  .fmax=+0.06,  .pmin=-0.1, .pmax=+0.1, .test_lo=1, .test_hi=1},
      {.fmin=+0.124, .fmax=+0.126, .pmin=   0, .pmax= -50, .test_lo=0, .test_hi=1},
      {.fmin= 0.20,  .fmax=+0.5,   .pmin=-0.1, .pmax=+0.1, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_psd_firfilt_crcf(q, 1200, regions, 5,
        liquid_autotest_verbose ? "autotest/logs/firfilt_crcf_notch.m" : NULL);
    firfilt_crcf_destroy(q);
}

void autotest_firfilt_cccf_notch()
{
    // design filter and verify resulting spectrum
    firfilt_cccf q = firfilt_cccf_create_notch(20, 60.0f, 0.125f);
    autotest_psd_s regions[] = {
      {.fmin=-0.5,   .fmax=+0.06,  .pmin=-0.1, .pmax=+0.1, .test_lo=1, .test_hi=1},
      {.fmin=+0.124, .fmax=+0.126, .pmin=   0, .pmax= -50, .test_lo=0, .test_hi=1},
      {.fmin= 0.20,  .fmax=+0.5,   .pmin=-0.1, .pmax=+0.1, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_psd_firfilt_cccf(q, 1200, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/firfilt_cccf_notch.m" : NULL);
    firfilt_cccf_destroy(q);
}

void autotest_firfilt_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firfilt config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // no need to check every combination
    CONTEND_ISNULL(firfilt_crcf_create(NULL, 0));
    CONTEND_ISNULL(firfilt_crcf_create_kaiser(0, 0, 0, 0));
    CONTEND_ISNULL(firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_UNKNOWN, 0, 0, 0, 4));
    CONTEND_ISNULL(firfilt_crcf_create_firdespm(0, 0, 0));
    CONTEND_ISNULL(firfilt_crcf_create_rect(0));
    CONTEND_ISNULL(firfilt_crcf_create_dc_blocker(0, 0));
    CONTEND_ISNULL(firfilt_crcf_create_notch(0, 0, 0));
    CONTEND_ISNULL(firfilt_cccf_create_notch(0, 0, 0));
    CONTEND_ISNULL(firfilt_crcf_copy(NULL));

    // create proper object and test configurations
    firfilt_crcf q = firfilt_crcf_create_kaiser(11, 0.2f, 60.0f, 0.0f);

    CONTEND_EQUALITY(LIQUID_OK, firfilt_crcf_print(q))

    float scale = 7.0f;
    CONTEND_EQUALITY(LIQUID_OK, firfilt_crcf_set_scale(q, 3.0f))
    CONTEND_EQUALITY(LIQUID_OK, firfilt_crcf_get_scale(q, &scale))
    CONTEND_EQUALITY(scale, 3.0f)
    CONTEND_EQUALITY(firfilt_crcf_get_length(q), 11)

    firfilt_crcf_destroy(q);
}

// TODO: test recreate where new filter length does not match original
void autotest_firfilt_recreate()
{
    // create random-ish coefficients
    unsigned int i, n = 21;
    float h0[n], h1[n];
    for (i=0; i<n; i++)
        h0[i] = cosf(0.3*i) + sinf(sqrtf(2.0f)*i);

    firfilt_crcf q = firfilt_crcf_create(h0, n);

    CONTEND_EQUALITY(LIQUID_OK, firfilt_crcf_print(q))
    CONTEND_EQUALITY(LIQUID_OK, firfilt_crcf_set_scale(q, 3.0f))

    // copy coefficients to separate array
    CONTEND_EQUALITY(LIQUID_OK, firfilt_crcf_copy_coefficients(q, h1))

    // scale coefficients by a constant
    for (i=0; i<n; i++)
        h1[i] *= 7.1f;

    // recreate with new coefficients array
    q = firfilt_crcf_recreate(q, h1, n);

    // assert the scale has not changed
    float scale;
    firfilt_crcf_get_scale(q, &scale);
    CONTEND_EQUALITY(scale, 3.0f)

    // assert the coefficients are original scaled by 7.1
    // NOTE: need to account for time-reversal here
    const float * h = firfilt_crcf_get_coefficients(q);
    for (i=0; i<n; i++)
        CONTEND_EQUALITY(h[n-i-1], h0[i]*7.1f);

    // re-create with longer coefficients array and test impulse response
    float h2[2*n+1]; // new random-ish coefficients
    for (i=0; i<2*n+1; i++)
        h2[i] = cosf(0.2*i+1) + sinf(logf(2.0f)*i);
    q = firfilt_crcf_recreate(q, h2, 2*n+1);
    for (i=0; i<2*n+1; i++) {
        firfilt_crcf_push(q, i==0 ? 1 : 0);
        float complex v;
        firfilt_crcf_execute(q, &v);
        // output is same as input, subject to scaling factor
        CONTEND_EQUALITY(v, h2[i]*scale);
    }

    firfilt_crcf_destroy(q);
}

// compare push vs write methods
void autotest_firfilt_push_write()
{
    // create two identical objects
    firfilt_rrrf q0 = firfilt_rrrf_create_kaiser(51, 0.2f, 60.0f, 0.0f);
    firfilt_rrrf q1 = firfilt_rrrf_create_kaiser(51, 0.2f, 60.0f, 0.0f);

    // generate pseudo-random inputs, and compare outputs
    float buf[8] = {-1,3,5,-3,5,1,-3,-4};
    unsigned int trial, i;
    for (trial=0; trial<20; trial++) {
        unsigned int n = trial % 8;

        // push/write samples
        for (i=0; i<n; i++)
            firfilt_rrrf_push(q0, buf[i]);
        firfilt_rrrf_write(q1, buf, n);

        // compute outputs and compare
        float v0, v1;
        firfilt_rrrf_execute(q0, &v0);
        firfilt_rrrf_execute(q1, &v1);
        CONTEND_EQUALITY(v0, v1);
    }

    // destroy objects
    firfilt_rrrf_destroy(q0);
    firfilt_rrrf_destroy(q1);
}

