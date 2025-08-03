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
#include "autotest/autotest.h"
#include "liquid.h"

// common structure for relaying information to/from callback
typedef struct {
    int             id;
    float complex * buf;
    unsigned int    buf_len;
    unsigned int    count;
} autotest_qdsync_s;

// synchronization callback, return 0:continue, 1:reset
int autotest_qdsync_callback(float complex * _buf,
                             unsigned int    _buf_len,
                             void *          _context)
{
    // save samples to buffer as appropriate
    autotest_qdsync_s * q = (autotest_qdsync_s *) _context;
    if (liquid_autotest_verbose)
        printf("[%d] qdsync callback got %u samples\n", q->id, _buf_len);
    unsigned int i;
    for (i=0; i<_buf_len; i++) {
        if (q->count == q->buf_len)
            return 1; // buffer full; reset synchronizer

        // save payload
        q->buf[q->count++] = _buf[i];
    }
    return 0;
}

void testbench_qdsync_linear(unsigned int _k,
                             unsigned int _m,
                             float        _beta)
{
    // options
    unsigned int seq_len      = 1200;   // total number of sync symbols
    unsigned int k            =   _k;   // samples/symbol
    unsigned int m            =   _m;   // filter delay [symbols]
    float        beta         = _beta;  // excess bandwidth factor
    int          ftype        = LIQUID_FIRFILT_ARKAISER;
    float        nstd         = 0.001f;
    float        tau          = 0.400f; // fractional sample timing offset

    // generate synchronization sequence (QPSK symbols)
    float complex seq_tx[seq_len];  // transmitted
    float complex seq_rx[seq_len];  // received with initial correction
    unsigned int i;
    for (i=0; i<seq_len ; i++) {
        seq_tx[i] = (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 +
                    (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 * _Complex_I;
    }

    // create sync object, only using first few symbols
    autotest_qdsync_s obj = {.id=0, .buf=seq_rx, .buf_len=seq_len, .count=0};
    qdsync_cccf q = qdsync_cccf_create_linear(seq_tx, 240, ftype, k, m, beta,
            autotest_qdsync_callback, (void*)&obj);
    qdsync_cccf_set_range(q, 0.001f);

    // create interpolator
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype,k,m,beta,0);

    // create delay object
    fdelay_crcf delay = fdelay_crcf_create_default(100);
    fdelay_crcf_set_delay(delay, 10*k + tau);

    // run signal through sync object
    float complex buf[k];
    for (i=0; i<4*seq_len + 2*m + 50; i++) {
        // produce symbol (preamble sequence or zero)
        float complex s = (i < seq_len) ? seq_tx[i] : 0;

        // interpolate symbol
        firinterp_crcf_execute(interp, s, buf);

        // apply delay in place
        fdelay_crcf_execute_block(delay, buf, k, buf);

        // add noise
        unsigned int j;
        for (j=0; j<k; j++)
            buf[j] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // run through synchronizer
        qdsync_cccf_execute(q, buf, k);
    }
    float rxy_hat  = qdsync_cccf_get_rxy  (q);
    float tau_hat  = qdsync_cccf_get_tau  (q);
    float gamma_hat= qdsync_cccf_get_gamma(q);
    float dphi_hat = qdsync_cccf_get_dphi (q);
    float phi_hat  = qdsync_cccf_get_phi  (q);

    // compute error in terms of offset from unity; might be residual carrier phase/gain
    // TODO: perform residual carrier/phase error correction?
    float rmse = 0.0f;
    for (i=0; i<seq_len; i++) {
        float e = cabsf(seq_rx[i]) - 1.0f;
        rmse += e*e;
    }
    rmse = 10*log10f( rmse / (float)seq_len );
    if (liquid_autotest_verbose) {
        printf("qdsync: rxy:%5.3f, tau:%5.2f, gamma:%5.3f, dphi:%12.4e, phi:%8.5f, rmse:%5.2f\n",
                rxy_hat, tau_hat, gamma_hat, dphi_hat, phi_hat, rmse);
    }
    CONTEND_LESS_THAN   ( rmse,              -30.0f )
    CONTEND_GREATER_THAN( rxy_hat,            0.75f )
    CONTEND_LESS_THAN   ( fabsf(tau_hat-tau), 0.10f )
    CONTEND_GREATER_THAN( gamma_hat,          0.75f )
    CONTEND_LESS_THAN   ( fabsf(dphi_hat),    1e-3f )
    CONTEND_LESS_THAN   ( fabsf( phi_hat),    0.4f  )

    // clean up objects
    qdsync_cccf_destroy(q);
    firinterp_crcf_destroy(interp);
    fdelay_crcf_destroy(delay);
#if 0
    FILE * fid = fopen("autotest/logs/qdsync_cccf_autotest.m","w");
    fprintf(fid,"clear all; close all;\n");
    for (i=0; i<seq_len; i++) {
        fprintf(fid,"s(%4u)=%12.4e+%12.4ej; r(%4u)=%12.4e+%12.4ej;\n",
            i+1, crealf(seq_tx[i]), cimagf(seq_tx[i]),
            i+1, crealf(seq_rx[i]), cimagf(seq_rx[i]));
    }
    fprintf(fid,"figure('color','white','position',[100 100 800 400]);\n");
    fprintf(fid,"subplot(1,2,1), plot(s,'.','MarkerSize',6); grid on; axis square;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5); xlabel('I'); ylabel('Q'); title('tx');\n");
    fprintf(fid,"subplot(1,2,2), plot(r,'.','MarkerSize',6); grid on; axis square;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5); xlabel('I'); ylabel('Q'); title('rx');\n");
    fclose(fid);
#endif
}

// test specific configurations
void autotest_qdsync_cccf_k2() { testbench_qdsync_linear(2, 7, 0.3f); }
void autotest_qdsync_cccf_k3() { testbench_qdsync_linear(3, 7, 0.3f); }
void autotest_qdsync_cccf_k4() { testbench_qdsync_linear(4, 7, 0.3f); }

// test setting buffer length ot different sizes throughout run
void autotest_qdsync_set_buf_len()
{
    // options
    unsigned int seq_len      = 2400;   // total number of sync symbols
    unsigned int k            =    2;   // samples/symbol
    unsigned int m            =   12;   // filter delay [symbols]
    float        beta         = 0.3f;   // excess bandwidth factor
    int          ftype        = LIQUID_FIRFILT_ARKAISER;

    // generate synchronization sequence (QPSK symbols)
    float complex seq_tx[seq_len];  // transmitted
    float complex seq_rx[seq_len];  // received with initial correction
    unsigned int i;
    for (i=0; i<seq_len ; i++)
        seq_tx[i] = cexpf(_Complex_I*2*M_PI*randf());

    // create sync object, only using first few symbols
    autotest_qdsync_s obj = {.id=0, .buf=seq_rx, .buf_len=seq_len, .count=0};
    qdsync_cccf q = qdsync_cccf_create_linear(seq_tx, 120, ftype, k, m, beta,
            autotest_qdsync_callback, (void*)&obj);
    qdsync_cccf_set_range(q, 0.001f);

    // create interpolator
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype,k,m,beta,0);

    // run signal through sync object
    float complex buf[k];
    for (i=0; i<seq_len + 20*m + 200; i++) {
        // interpolate symbol
        firinterp_crcf_execute(interp, (i < seq_len) ? seq_tx[i] : 0, buf);

        // run through synchronizer
        qdsync_cccf_execute(q, buf, k);

        // periodically increase buffer length
        if ( (i % 7)==0 )
            qdsync_cccf_set_buf_len(q, 13 + 17*(i % 11));
    }

    // compute error in terms of offset from unity; might be residual carrier phase/gain
    // TODO: perform residual carrier/phase error correction?
    float rmse = 0.0f;
    for (i=0; i<seq_len; i++) {
        float e = cabsf(seq_rx[i]) - 1.0f;
        rmse += e*e;
    }
    rmse = 10*log10f( rmse / (float)seq_len );
    if (liquid_autotest_verbose)
        printf("qdsync: rmse: %12.3f\n", rmse);
    CONTEND_LESS_THAN( rmse, -30.0f )

    // clean up objects
    qdsync_cccf_destroy(q);
    firinterp_crcf_destroy(interp);
}

// test copying from one object to another
void autotest_qdsync_cccf_copy()
{
    // options
    unsigned int seq_len= 2400; // total number of symbols in sequence
    unsigned int split  = 1033; // cut-off point where object gets copied
    unsigned int k      =    2; // samples/symbol
    unsigned int m      =   12; // filter delay [symbols]
    float        beta   = 0.25; // excess bandwidth factor
    int          ftype  = LIQUID_FIRFILT_ARKAISER;
    float        nstd   = 0.001f;
    unsigned int i;

    // generate random frame sequence
    float complex seq_tx     [seq_len];  // transmitted
    float complex seq_rx_orig[seq_len];  // received with initial correction (original)
    float complex seq_rx_copy[seq_len];  // received with initial correction (original)
    for (i=0; i<seq_len ; i++) {
        seq_tx[i] = (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 +
                    (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 * _Complex_I;
    }

    // create objects
    autotest_qdsync_s c_orig = {.id=0, .buf=seq_rx_orig, .buf_len=seq_len, .count=0};
    qdsync_cccf q_orig = qdsync_cccf_create_linear(seq_tx, 240, ftype, k, m, beta,
            autotest_qdsync_callback, (void*)&c_orig);

    // create interpolator
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype,k,m,beta,0);

    // feed some symbols through synchronizer (about half)
    float complex buf[k];
    for (i=0; i<split; i++) {
        // interpolate symbol
        firinterp_crcf_execute(interp, (i < seq_len) ? seq_tx[i] : 0, buf);

        // add noise
        unsigned int j;
        for (j=0; j<k; j++)
            buf[j] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // run through synchronizer
        qdsync_cccf_execute(q_orig, buf, k);
    }

    // copy object, but set different context
    autotest_qdsync_s c_copy = {.id=1, .buf=seq_rx_copy, .buf_len=seq_len, .count=c_orig.count};
    memmove(c_copy.buf, c_orig.buf, c_orig.count*sizeof(float complex)); // copy what's in buffer so far
    qdsync_cccf q_copy = qdsync_cccf_copy(q_orig);
    qdsync_cccf_set_context(q_copy, (void*)&c_copy);

    // run remaining sequence through detectors
    for (i=split; i<seq_len + 20*m * 40; i++) {
        // interpolate symbol
        firinterp_crcf_execute(interp, (i < seq_len) ? seq_tx[i] : 0, buf);

        // add noise
        unsigned int j;
        for (j=0; j<k; j++)
            buf[j] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // run through each synchronizer
        qdsync_cccf_execute(q_orig, buf, k);
        qdsync_cccf_execute(q_copy, buf, k);
    }

    // compare output buffers
    CONTEND_EQUALITY(c_orig.count, seq_len);
    CONTEND_EQUALITY(c_copy.count, seq_len);
    // print values for visual comparison
    //for (i=0; i<seq_len; i++) {
    //    printf(" [%4u] orig={%12.8f,%12.8f}, copy={%12.8f,%12.8f}\n", i,
    //            crealf(c_orig.buf[i]), cimagf(c_orig.buf[i]),
    //            crealf(c_copy.buf[i]), cimagf(c_copy.buf[i]));
    //}
    // data should be identical at this point
    CONTEND_SAME_DATA(c_orig.buf, c_copy.buf, seq_len*sizeof(float complex));

    // destroy objects
    firinterp_crcf_destroy(interp);
    qdsync_cccf_destroy(q_orig);
    qdsync_cccf_destroy(q_copy);
}

void autotest_qdsync_cccf_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping qdsync_cccf config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check invalid function calls
    CONTEND_ISNULL(qdsync_cccf_copy(NULL));
    CONTEND_ISNULL(qdsync_cccf_create_linear(NULL,0,LIQUID_FIRFILT_ARKAISER,4,12,0.25f,NULL,NULL));

    // create proper object and test configurations
    float complex seq[] = {+1,-1,+1,-1,-1,+1,-1,+1,-1,+1,-1,+1,+1,+1,-1,+1,-1,-1,-1,-1,};
    qdsync_cccf q = qdsync_cccf_create_linear(seq,20,LIQUID_FIRFILT_ARKAISER,4,12,0.25f,NULL,NULL);

    CONTEND_EQUALITY(LIQUID_OK, qdsync_cccf_print(q))
    CONTEND_EQUALITY(LIQUID_OK, qdsync_cccf_set_callback(q,autotest_qdsync_callback))
    CONTEND_EQUALITY(LIQUID_OK, qdsync_cccf_set_context(q,NULL))

    // set/get threshold
    CONTEND_EQUALITY(LIQUID_OK, qdsync_cccf_set_threshold(q,0.654321f))
    CONTEND_EQUALITY(0.654321f, qdsync_cccf_get_threshold(q))

    // set/get range
    CONTEND_EQUALITY(LIQUID_OK, qdsync_cccf_set_range(q,0.007220f))
    CONTEND_EQUALITY(0.007220f, qdsync_cccf_get_range(q))

    // set invalid buffer length
    CONTEND_INEQUALITY(LIQUID_OK, qdsync_cccf_set_buf_len(q,0))

    // query properties
    CONTEND_EQUALITY(0, qdsync_cccf_is_open(q))

    // destroy object
    qdsync_cccf_destroy(q);
}

