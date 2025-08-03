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
#include <math.h>
#include "autotest/autotest.h"
#include "liquid.h"

// AUTOTEST : test simple recovery of frame in noise
void autotest_framesync64()
{
    unsigned int i;

    // create objects
    unsigned int context = 0;
    framegen64 fg = framegen64_create();
    framesync64 fs = framesync64_create(framing_autotest_callback, (void*)&context);

    // generate the frame
    float complex frame[LIQUID_FRAME64_LEN];
    framegen64_execute(fg, NULL, NULL, frame);

    // add some noise
    for (i=0; i<LIQUID_FRAME64_LEN; i++)
        frame[i] += 0.01f*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;

    // try to receive the frame
    framesync64_execute(fs, frame, LIQUID_FRAME64_LEN);

    // ensure callback was actually invoked
    CONTEND_EQUALITY(context, FRAMING_AUTOTEST_SECRET);

    // parse statistics
    framedatastats_s stats = framesync64_get_framedatastats(fs);
    CONTEND_EQUALITY(stats.num_frames_detected, 1);
    CONTEND_EQUALITY(stats.num_headers_valid,   1);
    CONTEND_EQUALITY(stats.num_payloads_valid,  1);
    CONTEND_EQUALITY(stats.num_bytes_received, 64);

    // destroy objects
    framegen64_destroy(fg);
    framesync64_destroy(fs);
}

// test copying from one object to another
void autotest_framegen64_copy()
{
    framegen64 q_orig = framegen64_create();
    framegen64 q_copy = framegen64_copy(q_orig);

    if (q_copy == NULL) {
        AUTOTEST_FAIL("could not copy framegen64 object");
    } else {
        AUTOTEST_PASS();
        framegen64_destroy(q_copy);
    }
    framegen64_destroy(q_orig);
}

// test copying from one object to another
void autotest_framesync64_copy()
{
    unsigned int i;
    int context_0 = 0;
    int context_1 = 0;

    // create objects
    framegen64  fg  = framegen64_create();
    framesync64 fs0 = framesync64_create(framing_autotest_callback, (void*)&context_0);

    // feed random samples into synchronizer
    float complex buf[LIQUID_FRAME64_LEN];
    for (i=0; i<LIQUID_FRAME64_LEN; i++)
        buf[i] = 0.01f*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;
    framesync64_execute(fs0, buf, LIQUID_FRAME64_LEN);

    // generate the frame
    framegen64_execute(fg, NULL, NULL, buf);

    // add some noise
    for (i=0; i<LIQUID_FRAME64_LEN; i++)
        buf[i] += 0.01f*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;

    // copy object, but set different context
    framesync64 fs1 = framesync64_copy(fs0);
    framesync64_set_userdata(fs1, (void*)&context_1);
    framesync64_print(fs0);
    framesync64_print(fs1);

    // try to receive the frame with each receiver
    for (i=0; i<LIQUID_FRAME64_LEN; i++) {
        // step one sample through at a time
        framesync64_execute(fs0, buf+i, 1);
        framesync64_execute(fs1, buf+i, 1);

        // ensure that the frames are recovered at exactly the same time
        CONTEND_EQUALITY( context_0, context_1 );
    }

    // check that frame was actually recovered by each object
    CONTEND_EQUALITY( context_0, 0x01234567 );
    CONTEND_EQUALITY( context_1, 0x01234567 );

    // parse statistics
    framedatastats_s stats_0 = framesync64_get_framedatastats(fs0);
    framedatastats_s stats_1 = framesync64_get_framedatastats(fs1);
    framedatastats_print(&stats_0);
    framedatastats_print(&stats_1);

    CONTEND_EQUALITY(stats_0.num_frames_detected, stats_1.num_frames_detected);
    CONTEND_EQUALITY(stats_0.num_headers_valid  , stats_1.num_headers_valid  );
    CONTEND_EQUALITY(stats_0.num_payloads_valid , stats_1.num_payloads_valid );
    CONTEND_EQUALITY(stats_0.num_bytes_received , stats_1.num_bytes_received );
    framesync64_print(fs0);
    framesync64_print(fs1);

    // destroy objects
    framegen64_destroy(fg);
    framesync64_destroy(fs0);
    framesync64_destroy(fs1);
}

void autotest_framesync64_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping framesync64 config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check invalid function calls
    CONTEND_ISNULL(framesync64_copy(NULL));
    CONTEND_ISNULL(framegen64_copy (NULL));

    // create proper object and test configurations
    framesync64 q = framesync64_create(NULL, NULL);

    CONTEND_EQUALITY(LIQUID_OK, framesync64_print(q))
    CONTEND_EQUALITY(LIQUID_OK, framesync64_set_callback(q,NULL))
    CONTEND_EQUALITY(LIQUID_OK, framesync64_set_userdata(q,NULL))

    CONTEND_EQUALITY(LIQUID_OK, framesync64_set_threshold(q,0.654321f))
    CONTEND_EQUALITY(0.654321f, framesync64_get_threshold(q))

    framesync64_destroy(q);
}

static int callback_framesync64_autotest_debug(
    unsigned char *  _header,
    int              _header_valid,
    unsigned char *  _payload,
    unsigned int     _payload_len,
    int              _payload_valid,
    framesyncstats_s _stats,
    void *           _userdata)
{
    // return custom code based on user context
    return *((int*)_userdata);
}

// test debug interface to write file
void testbench_framesync64_debug(int _code)
{
    // create objects
    framegen64  fg = framegen64_create();
    framesync64 fs = framesync64_create(callback_framesync64_autotest_debug,(void*)(&_code));

    // set prefix for filename
    const char prefix[] = "autotest/logs/framesync64";
    framesync64_set_prefix(fs,prefix);
    CONTEND_SAME_DATA(framesync64_get_prefix(fs), prefix, strlen(prefix));

    // generate the frame
    float complex frame[LIQUID_FRAME64_LEN];
    unsigned char header [ 8] = {80,81,82,83,84,85,86,87};
    unsigned char payload[64];
    unsigned int i;
    for (i=0; i<64; i++)
        payload[i] = rand() & 0xff;
    framegen64_execute(fg, header, payload, frame);

    // add some noise
    for (i=0; i<LIQUID_FRAME64_LEN; i++)
        frame[i] += 0.01f*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;

    // try to receive the frame
    framesync64_execute(fs, frame, LIQUID_FRAME64_LEN);

    // parse statistics
    framedatastats_s stats = framesync64_get_framedatastats(fs);
    CONTEND_EQUALITY(stats.num_frames_detected, 1);
    CONTEND_EQUALITY(stats.num_headers_valid,   1);
    CONTEND_EQUALITY(stats.num_payloads_valid,  1);
    CONTEND_EQUALITY(stats.num_bytes_received, 64);

    // get filename from the last file written and copy before destroying
    // objects
    const char * fn = framesync64_get_filename(fs);
    char filename[256] = "";
    snprintf(filename,255,"%s",fn==NULL ? "" : fn);
    printf("filename: %s\n", filename);

    // destroy objects
    framegen64_destroy(fg);
    framesync64_destroy(fs);

    // check if output file should exist
    if (strlen(filename) == 0) {
        if (_code==0) {
            AUTOTEST_PASS();
        } else {
            AUTOTEST_FAIL("no output file written when one was expected");
        }
        return;
    }

    // load file
    FILE * fid = fopen(filename,"rb");
    if (fid == NULL) {
        AUTOTEST_FAIL("could not open file for reading");
        return;
    }

    fclose(fid);
}

// test exporting debugging files with different return codes
void autotest_framesync64_debug_none() { testbench_framesync64_debug( 0); }
void autotest_framesync64_debug_user() { testbench_framesync64_debug( 1); }
void autotest_framesync64_debug_ndet() { testbench_framesync64_debug(-1); }
void autotest_framesync64_debug_head() { testbench_framesync64_debug(-2); }
void autotest_framesync64_debug_rand() { testbench_framesync64_debug(-3); }


static int callback_framesync64_autotest_estimation(
    unsigned char *  _header,
    int              _header_valid,
    unsigned char *  _payload,
    unsigned int     _payload_len,
    int              _payload_valid,
    framesyncstats_s _stats,
    void *           _userdata)
{
    //printf("callback invoked, payload valid: %s\n", _payload_valid ? "yes" : "no");
    memmove(_userdata, &_stats, sizeof(framesyncstats_s));
    return 0;
}

// add channel offsets to frame
void framesync64_channel(float complex * _frame,
                         float           _rssi,
                         float           _SNRdB,
                         float           _dphi)
{
    // derived values
    float gain = powf(10.0f, _rssi/20.0f); // for RSSI, not PSD (given 2 samples/symbol)
    float n0   = _rssi - _SNRdB + 10*log10f(2.0f);  // noise floor accounting for 2 samples/symbol
    float nstd = powf(10.0f, n0/20.0f);

    unsigned int i;
    for (i=0; i<LIQUID_FRAME64_LEN; i++)
        _frame[i] = _frame[i]*cexp(_Complex_I*_dphi*i)*gain + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
}

// AUTOTEST : test simple recovery of frame in noise
void autotest_framesync64_estimation()
{
    // create objects
    framegen64 fg = framegen64_create();
    framesyncstats_s stats;
    framesync64 fs = framesync64_create(callback_framesync64_autotest_estimation,
            (void*)&stats);

    // generate the frame
    float complex frame[LIQUID_FRAME64_LEN];
    framegen64_execute(fg, NULL, NULL, frame);

    // add offsets
    float rssi  = -43.0f;
    float SNRdB =  25.0f;
    float dphi  =   1e-2f;
    framesync64_channel(frame, rssi, SNRdB, dphi);

    // try to receive the frame
    framesync64_execute(fs, frame, LIQUID_FRAME64_LEN);

    if (liquid_autotest_verbose)
        framesyncstats_print(&stats);

    // check results (relatively high tolerance)
    CONTEND_DELTA( stats.rssi, rssi,  1.0f );
    CONTEND_DELTA( -stats.evm, SNRdB, 3.0f ); // error biased negative
    CONTEND_DELTA( stats.cfo,  dphi,  4e-3f);

    // destroy objects
    framegen64_destroy(fg);
    framesync64_destroy(fs);

#if 0
    FILE * fp = fopen("framesync64_errors.txt", "a");
    fprintf(fp,"%12.8f %12.8f %12.4e\n",(stats.rssi-rssi),(-stats.evm-SNRdB),(stats.cfo-dphi));
    fclose(fp);
#endif

#if 0
    FILE * fid = fopen("framesync64_estimation.m","w");
    fprintf(fid,"clear all; close all; n=%u; y=zeros(1,n);\n", LIQUID_FRAME64_LEN);
    unsigned int nfft=240;
    spgramcf q = spgramcf_create_default(nfft);
    spgramcf_write(q, frame, LIQUID_FRAME64_LEN);
    float psd[nfft];
    spgramcf_get_psd(q, psd);
    spgramcf_destroy(q);
    unsigned int i;
    for (i=0; i<LIQUID_FRAME64_LEN; i++)
        fprintf(fid,"y(%3u) = %12.4e + %12.4ej;\n", i+1, crealf(frame[i]), cimagf(frame[i]));
    for (i=0; i<nfft; i++)
        fprintf(fid,"Y(%3u) = %12.4e;\n", i+1, psd[i]);
    fclose(fid);
#endif
}

