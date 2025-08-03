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
#include "liquid.internal.h"

// Test DC gain control
void autotest_agc_crcf_dc_gain_control()
{
    // set parameters
    float gamma = 0.1f;     // nominal signal level
    float bt    = 0.1f;     // bandwidth-time product
    float tol   = 0.001f;   // error tolerance

    // create AGC object and initialize
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, bt);

    unsigned int i;
    float complex x = gamma;    // input sample
    float complex y;            // output sample
    for (i=0; i<256; i++)
        agc_crcf_execute(q, x, &y);
    
    // Check results
    CONTEND_DELTA( crealf(y), 1.0f, tol );
    CONTEND_DELTA( cimagf(y), 0.0f, tol );
    CONTEND_DELTA( agc_crcf_get_gain(q), 1.0f/gamma, tol );

    // explicitly set gain and check result
    agc_crcf_set_gain(q, 1.0f);
    CONTEND_EQUALITY( agc_crcf_get_gain(q), 1.0f );

    // destroy AGC object
    agc_crcf_destroy(q);
}

// test gain control on DC input with separate scale
void autotest_agc_crcf_scale()
{
    // set parameters
    float scale = 4.0f;     // output scale (independent of AGC loop)
    float tol   = 0.001f;   // error tolerance

    // create AGC object and initialize
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, 0.1f);
    agc_crcf_set_scale    (q, scale);
    CONTEND_EQUALITY(agc_crcf_get_scale(q), scale);

    unsigned int i;
    float complex x = 0.1f; // input sample
    float complex y;        // output sample
    for (i=0; i<256; i++)
        agc_crcf_execute(q, x, &y);
    
    // Check results
    CONTEND_DELTA( crealf(y), scale, tol );
    CONTEND_DELTA( cimagf(y),  0.0f, tol );

    // destroy AGC object
    agc_crcf_destroy(q);
}

// Test AC gain control
void autotest_agc_crcf_ac_gain_control()
{
    // set parameters
    float gamma = 0.1f;             // nominal signal level
    float bt    = 0.1f;             // bandwidth-time product
    float tol   = 0.001f;           // error tolerance
    float dphi  = 0.1f;             // NCO frequency

    // create AGC object and initialize
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, bt);

    unsigned int i;
    float complex x;
    float complex y;
    for (i=0; i<256; i++) {
        x = gamma * cexpf(_Complex_I*i*dphi);
        agc_crcf_execute(q, x, &y);
    }

    if (liquid_autotest_verbose)
        printf("gamma : %12.8f, rssi : %12.8f\n", gamma, agc_crcf_get_signal_level(q));

    // Check results
    CONTEND_DELTA( agc_crcf_get_gain(q), 1.0f/gamma, tol);

    // destroy AGC object
    agc_crcf_destroy(q);
}

// Test RSSI on sinusoidal input
void autotest_agc_crcf_rssi_sinusoid()
{
    // set parameters
    float gamma = 0.3f;         // nominal signal level
    float bt    = 0.05f;        // agc bandwidth
    float tol   = 0.001f;       // error tolerance

    // signal properties
    float dphi = 0.1f;          // signal frequency

    // create AGC object and initialize
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, bt);

    unsigned int i;
    float complex x, y;
    for (i=0; i<512; i++) {
        // generate sample (complex sinusoid)
        x = gamma * cexpf(_Complex_I*dphi*i);

        // execute agc
        agc_crcf_execute(q, x, &y);
    }

    // get received signal strength indication
    float rssi = agc_crcf_get_signal_level(q);

    if (liquid_autotest_verbose)
        printf("gamma : %12.8f, rssi : %12.8f\n", gamma, rssi);

    // Check results
    CONTEND_DELTA( rssi, gamma, tol );

    // destroy agc object
    agc_crcf_destroy(q);
}

// Test RSSI on noise input
void autotest_agc_crcf_rssi_noise()
{
    // set parameters
    float gamma = -30.0f;   // nominal signal level [dB]
    float bt    =  2e-3f;   // agc bandwidth
    float tol   =  1.0f;    // error tolerance [dB]

    // signal properties
    float nstd = powf(10.0f, gamma/20);

    // create AGC object and initialize
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, bt);

    unsigned int i;
    float complex x, y;
    for (i=0; i<8000; i++) {
        // generate sample (circular complex noise)
        x = nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // execute agc
        agc_crcf_execute(q, x, &y);
    }

    // get received signal strength indication
    float rssi = agc_crcf_get_rssi(q);

    if (liquid_autotest_verbose)
        printf("gamma : %12.8f, rssi : %12.8f\n", gamma, rssi);

    // Check results
    CONTEND_DELTA( rssi, gamma, tol );

    // destroy agc object
    agc_crcf_destroy(q);
}

// Test squelch functionality
void autotest_agc_crcf_squelch()
{
    // create agc object, set loop bandwidth, and initialize parameters
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, 0.25);
    agc_crcf_set_signal_level(q,1e-3f);     // initial guess at starting signal level

    // initialize squelch functionality
    CONTEND_FALSE(agc_crcf_squelch_is_enabled(q));
    agc_crcf_squelch_enable(q);             // enable squelch
    agc_crcf_squelch_set_threshold(q, -50); // threshold for detection [dB]
    agc_crcf_squelch_set_timeout  (q, 100); // timeout for hysteresis
    CONTEND_TRUE(agc_crcf_squelch_is_enabled(q));
    CONTEND_EQUALITY(agc_crcf_squelch_get_threshold(q), -50);
    CONTEND_EQUALITY(agc_crcf_squelch_get_timeout  (q), 100);

    // run agc
    unsigned int num_samples = 2000; // total number of samples to run
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        // generate signal, applying tapering window appropriately
        float gamma = 0.0f;
        if      (i <  500) gamma = 1e-3f;
        else if (i <  550) gamma = 1e-3f + (1e-2f - 1e-3f)*(0.5f - 0.5f*cosf(M_PI*(float)(i- 500)/50.0f));
        else if (i < 1450) gamma = 1e-2f;
        else if (i < 1500) gamma = 1e-3f + (1e-2f - 1e-3f)*(0.5f + 0.5f*cosf(M_PI*(float)(i-1450)/50.0f));
        else               gamma = 1e-3f;
        float complex x = gamma * cexpf(_Complex_I*2*M_PI*0.0193f*i);

        // apply gain
        float complex y;
        agc_crcf_execute(q, x, &y);

        // retrieve signal level [dB]
        //rssi = agc_crcf_get_rssi(q);

        // get squelch mode
        int mode = agc_crcf_squelch_get_status(q);

        // check certain conditions based on sample input (assuming 2000 samples)
        switch (i) {
            case    0: CONTEND_EQUALITY(mode, LIQUID_AGC_SQUELCH_ENABLED);  break;
            case  500: CONTEND_EQUALITY(mode, LIQUID_AGC_SQUELCH_ENABLED);  break;
            case  600: CONTEND_EQUALITY(mode, LIQUID_AGC_SQUELCH_SIGNALHI); break;
            case 1400: CONTEND_EQUALITY(mode, LIQUID_AGC_SQUELCH_SIGNALHI); break;
            case 1500: CONTEND_EQUALITY(mode, LIQUID_AGC_SQUELCH_SIGNALLO); break;
            case 1600: CONTEND_EQUALITY(mode, LIQUID_AGC_SQUELCH_ENABLED);  break;
            case 1900: CONTEND_EQUALITY(mode, LIQUID_AGC_SQUELCH_ENABLED);  break;
            default:;
        }
    }

    // destroy AGC object
    agc_crcf_destroy(q);
}

// test lock state control
void autotest_agc_crcf_lock()
{
    // set parameters
    float gamma = 0.1f;     // nominal signal level
    float tol   = 0.01f;    // error tolerance

    // create AGC object and initialize buffers for block processing
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, 0.1f);
    float complex buf_0[4] = {gamma, gamma, gamma, gamma,};
    float complex buf_1[4];
    unsigned int i;

    // basic tests
    CONTEND_EQUALITY(agc_crcf_get_bandwidth(q),0.1f);
    CONTEND_EQUALITY(agc_crcf_print(q),        LIQUID_OK);
    agc_crcf_set_rssi(q, 0.0f);

    // lock AGC and show it is not tracking
    CONTEND_DELTA( agc_crcf_get_rssi(q), 0, tol );  // base signal level is 0 dB
    CONTEND_FALSE( agc_crcf_is_locked(q) );         // not locked
    agc_crcf_lock(q);
    CONTEND_TRUE( agc_crcf_is_locked(q) );          // locked
    for (i=0; i<256; i++)
        agc_crcf_execute_block(q, buf_0, 4, buf_1);
    CONTEND_DELTA( agc_crcf_get_rssi(q), 0, tol );  // signal level has not changed

    // unlock AGC and show it is tracking
    agc_crcf_unlock(q);
    CONTEND_FALSE( agc_crcf_is_locked(q) );         // unlocked
    agc_crcf_init(q, buf_0, 4);
    // agc tracks to signal level
    CONTEND_DELTA( agc_crcf_get_rssi(q), 20*log10f(gamma), tol );

    // destroy AGC object
    agc_crcf_destroy(q);
}

// configuration
void autotest_agc_crcf_invalid_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping agc config test with strict exit enabled");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // create main object and check invalid configurations
    agc_crcf q = agc_crcf_create();

    // invalid bandwidths
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_bandwidth(q, -1))
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_bandwidth(q,  2))

    // invalid gains
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_gain(q,  0))
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_gain(q, -1))

    // invalid signal levels
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_signal_level(q,  0))
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_signal_level(q, -1))

    // invalid scale values
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_scale(q,  0))
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_set_scale(q, -1))

    // initialize gain on input array, but array has length 0
    CONTEND_INEQUALITY(LIQUID_OK, agc_crcf_init(q, NULL, 0))

    // destroy object
    agc_crcf_destroy(q);
}

// copy test
void autotest_agc_crcf_copy()
{
    // create base object and initialize
    agc_crcf q0 = agc_crcf_create();
    agc_crcf_set_bandwidth(q0, 0.01234f);

    // start running input through AGC
    unsigned int n = 32;
    unsigned int i;
    float complex x, y0, y1;
    for (i=0; i<n; i++) {
        x = randnf() + _Complex_I*randnf();
        agc_crcf_execute(q0, x, &y0);
    }

    // copy AGC
    agc_crcf q1 = agc_crcf_copy(q0);

    // continue running through both AGCs
    for (i=0; i<n; i++) {
        // run AGCs in parallel
        x = randnf() + _Complex_I*randnf();
        agc_crcf_execute(q0, x, &y0);
        agc_crcf_execute(q1, x, &y1);
        CONTEND_EQUALITY(y0, y1);
    }

    // destroy AGC objects
    agc_crcf_destroy(q0);
    agc_crcf_destroy(q1);
}

