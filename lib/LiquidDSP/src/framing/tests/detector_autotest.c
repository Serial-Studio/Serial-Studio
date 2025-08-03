/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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

// autotest helper function
//  _n      :   sequence length
//  _dt     :   fractional sample offset
//  _dphi   :   carrier frequency offset
//  _gamma  :   gain
void detector_cccf_runtest(unsigned int _n,
                           float        _dt,
                           float        _dphi);

//
// AUTOTESTS
//

void autotest_detector_cccf_n64()   { detector_cccf_runtest(  64, 0.2f, 0.01f); }
void autotest_detector_cccf_n83()   { detector_cccf_runtest(  83, 0.2f, 0.01f); }

void autotest_detector_cccf_n128()  { detector_cccf_runtest( 128, 0.2f, 0.01f); }
void autotest_detector_cccf_n167()  { detector_cccf_runtest( 167, 0.2f, 0.01f); }

void autotest_detector_cccf_n256()  { detector_cccf_runtest( 256, 0.2f, 0.01f); }
void autotest_detector_cccf_n335()  { detector_cccf_runtest( 335, 0.2f, 0.01f); }

void autotest_detector_cccf_n512()  { detector_cccf_runtest( 512, 0.2f, 0.01f); }
void autotest_detector_cccf_n671()  { detector_cccf_runtest( 671, 0.2f, 0.01f); }

void autotest_detector_cccf_n1024() { detector_cccf_runtest(1024, 0.2f, 0.01f); }
void autotest_detector_cccf_n1341() { detector_cccf_runtest(1341, 0.2f, 0.01f); }

// autotest helper function
//  _n      :   sequence length
//  _dt     :   fractional sample offset
//  _dphi   :   carrier frequency offset
void detector_cccf_runtest(unsigned int _n,
                           float        _dt,
                           float        _dphi)
{
    // TODO: validate input

    unsigned int i;

    // fixed values
    float noise_floor = -80.0f;     // noise floor [dB]
    float SNRdB       =  30.0f;     // signal-to-noise ratio [dB]
    unsigned int m    =  11;        // resampling filter semi-length
    float threshold   =  0.3f;      // detection threshold

    // derived values
    unsigned int num_samples = _n + 2*m + 1;
    float nstd = powf(10.0f, noise_floor/20.0f);
    float gamma = powf(10.0f, (SNRdB + noise_floor)/20.0f);
    float delay = (float)(_n + m) + _dt;    // expected delay

    // arrays
    float complex s[_n];            // synchronization pattern (samples)
    float complex x[num_samples];   // resampled signal with noise and offsets

    // generate synchronization pattern (two samples per symbol)
    unsigned int n2 = (_n - (_n%2)) / 2;    // n2 = floor(n/2)
    unsigned int mm = liquid_nextpow2(n2);  // mm = ceil( log2(n2) )
    msequence ms = msequence_create_default(mm);
    float complex v = 0.0f;
    for (i=0; i<_n; i++) {
        if ( (i%2)==0 )
            v = msequence_advance(ms) ? 1.0f : -1.0f;
        s[i] = v;
    }
    msequence_destroy(ms);

    // create fractional sample interpolator
    firfilt_crcf finterp = firfilt_crcf_create_kaiser(2*m+1, 0.45f, 40.0f, _dt);

    // generate sequence
    for (i=0; i<num_samples; i++) {
        // add fractional sample timing offset
        if (i < _n) firfilt_crcf_push(finterp, s[i]);
        else        firfilt_crcf_push(finterp, 0.0f);

        // compute output
        firfilt_crcf_execute(finterp, &x[i]);

        // add channel gain
        x[i] *= gamma;

        // add carrier offset
        x[i] *= cexpf(_Complex_I*_dphi*i);

        // add noise
        x[i] += nstd * ( randnf() + _Complex_I*randnf() ) * M_SQRT1_2;
    }
    
    // destroy fractional sample interpolator
    firfilt_crcf_destroy(finterp);

    // create detector
    detector_cccf sync = detector_cccf_create(s, _n, threshold, 2*_dphi);
    
    // push signal through detector
    float tau_hat   = 0.0f;     // fractional sample offset estimate
    float dphi_hat  = 0.0f;     // carrier offset estimate
    float gamma_hat = 1.0f;     // signal level estimate (linear)
    float delay_hat = 0.0f;     // total delay offset estimate
    int signal_detected = 0;    // signal detected flag
    for (i=0; i<num_samples; i++) {
        
        // correlate
        int detected = detector_cccf_correlate(sync, x[i], &tau_hat, &dphi_hat, &gamma_hat);

        if (detected) {
            signal_detected = 1;
            delay_hat = (float)i + (float)tau_hat;
            if (liquid_autotest_verbose) {
                printf("****** preamble found, tau_hat=%8.6f, dphi_hat=%8.6f, gamma_hat=%8.6f\n",
                        tau_hat, dphi_hat, gamma_hat);
            }
        }
    }
    
    // destroy objects
    detector_cccf_destroy(sync);

    // 
    // run tests
    //
    
    // convert to dB
    gamma     = 20*log10f(gamma);
    gamma_hat = 20*log10f(gamma_hat);

    if (liquid_autotest_verbose) {
        printf("detector autotest [%3u]: signal detected? %s\n", _n, signal_detected ? "yes" : "no");
        printf("    dphi    :   estimate = %12.6f (expected %12.6f)\n", dphi_hat,  _dphi);
        printf("    delay   :   estimate = %12.6f (expected %12.6f)\n", delay_hat, delay);
        printf("    gamma   :   estimate = %12.6f (expected %12.6f)\n", gamma_hat, gamma);
    }

    // ensure signal was detected
    CONTEND_EXPRESSION( signal_detected );

    // check carrier offset estimate
    CONTEND_DELTA( dphi_hat, _dphi, 0.01f );
    
    // check delay estimate
    CONTEND_DELTA( delay_hat, delay, 0.2f );
    
    // check signal level estimate
    CONTEND_DELTA( gamma_hat, gamma, 2.0f );
}


