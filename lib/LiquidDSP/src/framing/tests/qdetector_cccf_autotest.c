/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
#include "autotest/autotest.h"
#include "liquid.h"

// autotest helper functions
//  _sequence_len   :   sequence length
void qdetector_cccf_runtest_linear(unsigned int _sequence_len);
void qdetector_cccf_runtest_gmsk  (unsigned int _sequence_len);

// run test given initialized object
void qdetector_cccf_runtest(qdetector_cccf _q);

// linear tests
void autotest_qdetector_cccf_linear_n64()   { qdetector_cccf_runtest_linear(  64); }
void autotest_qdetector_cccf_linear_n83()   { qdetector_cccf_runtest_linear(  83); }
void autotest_qdetector_cccf_linear_n128()  { qdetector_cccf_runtest_linear( 128); }
void autotest_qdetector_cccf_linear_n167()  { qdetector_cccf_runtest_linear( 167); }
void autotest_qdetector_cccf_linear_n256()  { qdetector_cccf_runtest_linear( 256); }
void autotest_qdetector_cccf_linear_n335()  { qdetector_cccf_runtest_linear( 335); }
void autotest_qdetector_cccf_linear_n512()  { qdetector_cccf_runtest_linear( 512); }
void autotest_qdetector_cccf_linear_n671()  { qdetector_cccf_runtest_linear( 671); }
void autotest_qdetector_cccf_linear_n1024() { qdetector_cccf_runtest_linear(1024); }
void autotest_qdetector_cccf_linear_n1341() { qdetector_cccf_runtest_linear(1341); }

// gmsk tests
void autotest_qdetector_cccf_gmsk_n64()     { qdetector_cccf_runtest_gmsk  (  64); }
void autotest_qdetector_cccf_gmsk_n83()     { qdetector_cccf_runtest_gmsk  (  83); }
void autotest_qdetector_cccf_gmsk_n128()    { qdetector_cccf_runtest_gmsk  ( 128); }
void autotest_qdetector_cccf_gmsk_n167()    { qdetector_cccf_runtest_gmsk  ( 167); }
void autotest_qdetector_cccf_gmsk_n256()    { qdetector_cccf_runtest_gmsk  ( 256); }
void autotest_qdetector_cccf_gmsk_n335()    { qdetector_cccf_runtest_gmsk  ( 335); }
void autotest_qdetector_cccf_gmsk_n512()    { qdetector_cccf_runtest_gmsk  ( 512); }
void autotest_qdetector_cccf_gmsk_n671()    { qdetector_cccf_runtest_gmsk  ( 671); }
void autotest_qdetector_cccf_gmsk_n1024()   { qdetector_cccf_runtest_gmsk  (1024); }
void autotest_qdetector_cccf_gmsk_n1341()   { qdetector_cccf_runtest_gmsk  (1341); }

// autotest helper function
//  _sequence_len   :   sequence length
void qdetector_cccf_runtest_linear(unsigned int _sequence_len)
{
    unsigned int k     =     2;     // samples per symbol
    unsigned int m     =     7;     // filter delay [symbols]
    float        beta  =  0.3f;     // excess bandwidth factor
    int          ftype = LIQUID_FIRFILT_ARKAISER; // filter type

    // generate synchronization sequence (QPSK symbols)
    unsigned int i;
    float complex sequence[_sequence_len];
    for (i=0; i<_sequence_len; i++) {
        sequence[i] = (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 +
                      (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 * _Complex_I;
    }

    // create detector
    qdetector_cccf q = qdetector_cccf_create_linear(sequence, _sequence_len, ftype, k, m, beta);

    // run test
    qdetector_cccf_runtest(q);

    // destroy objects
    qdetector_cccf_destroy(q);
}

// autotest helper function
//  _sequence_len   :   sequence length
void qdetector_cccf_runtest_gmsk(unsigned int _sequence_len)
{
    unsigned int k     =     2;     // samples per symbol
    unsigned int m     =     7;     // filter delay [symbols]
    float        beta  =  0.3f;     // excess bandwidth factor

    // generate synchronization sequence (QPSK symbols)
    unsigned char sequence[_sequence_len];
    unsigned int i;
    for (i=0; i<_sequence_len; i++)
        sequence[i] = rand() & 0x01;

    // create detector
    qdetector_cccf q = qdetector_cccf_create_gmsk(sequence, _sequence_len, k, m, beta);

    // run test
    qdetector_cccf_runtest(q);

    // destroy objects
    qdetector_cccf_destroy(q);
}

// autotest helper function
void qdetector_cccf_runtest(qdetector_cccf _q)
{
    if (liquid_autotest_verbose)
        qdetector_cccf_print(_q);

    float gamma =  1.0f;    // channel gain
    float tau   =  0.0f;    // fractional sample timing offset
    float dphi  = -0.000f;  // carrier frequency offset (zero for now)
    float phi   =  0.5f;    // carrier phase offset

    float complex * seq = (float complex*)qdetector_cccf_get_sequence(_q);
    unsigned int sequence_len = qdetector_cccf_get_seq_len(_q);
    unsigned int num_samples = 8*sequence_len;
    float complex buf_rx[num_samples];

    // add channel impairments
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        buf_rx[i] = i < sequence_len ? seq[i] : 0.0f;
        buf_rx[i] *= gamma;
        buf_rx[i] *= cexpf(_Complex_I*(dphi*i + phi));
    }

    // estimates
    float tau_hat   = 0.0f;
    float gamma_hat = 0.0f;
    float dphi_hat  = 0.0f;
    float phi_hat   = 0.0f;
    int   frame_detected = 0;
    int   false_positive = 0;

    // try to detect frame
    float complex * v = NULL;
    for (i=0; i<num_samples; i++) {
        if (frame_detected)
            break;

        v = qdetector_cccf_execute(_q,buf_rx[i]);

        if (v != NULL) {
            frame_detected = 1;

            // get statistics
            tau_hat   = qdetector_cccf_get_tau(_q);
            gamma_hat = qdetector_cccf_get_gamma(_q);
            dphi_hat  = qdetector_cccf_get_dphi(_q);
            phi_hat   = qdetector_cccf_get_phi(_q);
            break;
        }
    }
    unsigned int sample_index = i;

    unsigned int buf_len = qdetector_cccf_get_buf_len(_q);
    if (liquid_autotest_verbose) {
        // print results
        printf("\n");
        printf("frame detected  :   %s\n", frame_detected ? "yes" : "no");
        printf("  sample index  : %8u, actual=%8u (error=%8d)\n", sample_index, buf_len, (int)sample_index - (int)buf_len);
        printf("  gamma hat     : %8.3f, actual=%8.3f (error=%8.3f)\n",            gamma_hat, gamma, gamma_hat - gamma);
        printf("  tau hat       : %8.3f, actual=%8.3f (error=%8.3f) samples\n",    tau_hat,   tau,   tau_hat   - tau  );
        printf("  dphi hat      : %8.5f, actual=%8.5f (error=%8.5f) rad/sample\n", dphi_hat,  dphi,  dphi_hat  - dphi );
        printf("  phi hat       : %8.5f, actual=%8.5f (error=%8.5f) radians\n",    phi_hat,   phi,   phi_hat   - phi  );
        printf("\n");
    }

    if (false_positive)
        AUTOTEST_FAIL("false positive detected");
    else if (!frame_detected)
        AUTOTEST_FAIL("frame not detected");
    else {
        // check signal level estimate
        // TODO: check discrepancy with short sequences
        //CONTEND_DELTA( gamma_hat, gamma, 0.05f );

        // check timing offset estimate
        CONTEND_DELTA( tau_hat, tau, 0.05f );

        // check carrier frequency offset estimate
        CONTEND_DELTA( dphi_hat, dphi, 0.01f );

        // check carrier phase offset estimate
        CONTEND_DELTA( phi_hat, phi, 0.1f );
    }
}

