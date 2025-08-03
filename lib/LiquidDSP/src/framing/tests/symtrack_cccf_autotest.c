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

#include <stdio.h>
#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

// autotest helper functions
void testbench_symtrack_cccf(unsigned int _k, unsigned int _m, float _beta, int _ms)
{
    int          ftype       = LIQUID_FIRFILT_ARKAISER;
    unsigned int num_symbols = 6000;    // number of data symbols
    float        noise_floor = -30.0f;  // noise floor [dB]
    float        SNRdB       = 30.0f;   // signal-to-noise ratio [dB]
    float        bandwidth   =  0.15f;  // loop filter bandwidth
    float        dphi        =  0.02f;  // carrier frequency offset [radians/sample]
    float        phi         =  2.1f;   // carrier phase offset [radians]

    // buffers
    unsigned int    buf_len = 800;  // buffer size
    float complex   buf_0[buf_len]; // sample buffer
    float complex   buf_1[buf_len]; // recovered symbols buffer

    // create stream generator
    symstreamcf gen = symstreamcf_create_linear(ftype,_k,2*_m,_beta,_ms);

    // create channel emulator and add impairments
    channel_cccf channel = channel_cccf_create();
    channel_cccf_add_awgn          (channel, noise_floor, SNRdB);
    channel_cccf_add_carrier_offset(channel, dphi, phi);
    //float complex h[4] = {1.0f, 0, 0, 0.2f*cexpf(_Complex_I*1.4f)};
    //channel_cccf_add_multipath     (channel, h, 4);

    // create symbol tracking synchronizer
    symtrack_cccf symtrack = symtrack_cccf_create(ftype,_k,_m,_beta,_ms);
    symtrack_cccf_set_bandwidth(symtrack,bandwidth);
    //symtrack_cccf_set_eq_off(symtrack); // disable equalization
    if (liquid_autotest_verbose)
        symtrack_cccf_print(symtrack);

    unsigned int total_samples = 0;
    //unsigned int total_symbols = 0;
    unsigned int num_symbols_evm = 0;
    float        evm = 0.0f;
    modemcf demod = modemcf_create(_ms); // for checking output EVM
    //FILE * fid = fopen("symtrack_test.dat","w");
    //fprintf(fid,"#v=load('symtrack_test.dat'); v=v(:,1)+j*v(:,2); plot(v,'x');\n");
    //fprintf(fid,"#axis([-1 1 -1 1]*1.5); axis square; grid on;\n");
    while (total_samples < num_symbols * _k)
    {
        // write samples to buffer
        symstreamcf_write_samples(gen, buf_0, buf_len);

        // apply channel
        channel_cccf_execute_block(channel, buf_0, buf_len, buf_0);

        // run resulting stream through synchronizer
        unsigned int num_symbols_sync;
        symtrack_cccf_execute_block(symtrack, buf_0, buf_len, buf_1, &num_symbols_sync);
        //total_symbols += num_symbols_sync;
        total_samples += buf_len;

        // check output EVM
        if (total_samples > 0.80 * num_symbols * _k) {
            unsigned int i, sym;
            for (i=0; i<num_symbols_sync; i++) {
                modemcf_demodulate(demod, buf_1[i], &sym);
                float err = modemcf_get_demodulator_evm(demod);
                evm += err*err;
                //fprintf(fid,"%12.8f %12.8f\n", crealf(buf_1[i]), cimagf(buf_1[i]));
            }
            num_symbols_evm += num_symbols_sync;
        }
    }
    //fclose(fid);

    // destroy objects
    symstreamcf_destroy  (gen);
    channel_cccf_destroy (channel);
    symtrack_cccf_destroy(symtrack);
    modemcf_destroy(demod);

    // verify output constellation EVM is reasonably high
    evm = 10*log10f(evm / (float)num_symbols_evm);
    printf("EVM: %12.8f, %u\n", evm, num_symbols_evm);
    CONTEND_LESS_THAN(evm, -15.0f);
}

void autotest_symtrack_cccf_bpsk() { testbench_symtrack_cccf( 2,12,0.25f,LIQUID_MODEM_BPSK); }
void autotest_symtrack_cccf_qpsk() { testbench_symtrack_cccf( 2,12,0.25f,LIQUID_MODEM_QPSK); }

// invalid configuration tests
void autotest_symtrack_cccf_config_invalid()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping symtrack_cccf config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    //CONTEND_ISNULL(symtrack_cccf_create(LIQUID_FIRFILT_UNKNOWN, 2, 12, 0.25f, LIQUID_MODEM_QPSK));
    CONTEND_ISNULL(symtrack_cccf_create(LIQUID_FIRFILT_RRC,   1, 12, 0.25f, LIQUID_MODEM_QPSK));
    CONTEND_ISNULL(symtrack_cccf_create(LIQUID_FIRFILT_RRC,   2,  0, 0.25f, LIQUID_MODEM_QPSK));
    CONTEND_ISNULL(symtrack_cccf_create(LIQUID_FIRFILT_RRC,   2, 12, 2.00f, LIQUID_MODEM_QPSK));
    CONTEND_ISNULL(symtrack_cccf_create(LIQUID_FIRFILT_RRC,   2, 12, 0.25f, LIQUID_MODEM_UNKNOWN));
    CONTEND_ISNULL(symtrack_cccf_create(LIQUID_FIRFILT_RRC,   2, 12, 0.25f, LIQUID_MODEM_NUM_SCHEMES));

    // create proper object but test invalid internal configurations
    symtrack_cccf q = symtrack_cccf_create_default();

    //CONTEND_INEQUALITY(LIQUID_OK, symtrack_cccf_set_modscheme(q, LIQUID_MODEM_UNKNOWN))
    CONTEND_INEQUALITY(LIQUID_OK, symtrack_cccf_set_modscheme(q, LIQUID_MODEM_NUM_SCHEMES))
    CONTEND_INEQUALITY(LIQUID_OK, symtrack_cccf_set_bandwidth(q, -1.0f))

    // destroy object
    symtrack_cccf_destroy(q);
}

// configuration tests
void autotest_symtrack_cccf_config_valid()
{
    // create proper object and test configuration methods
    symtrack_cccf q =
        symtrack_cccf_create(LIQUID_FIRFILT_ARKAISER, 4, 12, 0.25f, LIQUID_MODEM_QAM64);

    // test valid configurations
    CONTEND_EQUALITY(LIQUID_OK, symtrack_cccf_adjust_phase(q, 0.1f) );
    CONTEND_EQUALITY(LIQUID_OK, symtrack_cccf_set_eq_cm   (q      ) );
    CONTEND_EQUALITY(LIQUID_OK, symtrack_cccf_set_eq_dd   (q      ) );
    CONTEND_EQUALITY(LIQUID_OK, symtrack_cccf_set_eq_off  (q      ) );

    // test access methods
    CONTEND_EQUALITY(symtrack_cccf_get_k(q),    4);
    CONTEND_EQUALITY(symtrack_cccf_get_m(q),    12);
    CONTEND_EQUALITY(symtrack_cccf_get_beta(q), 0.25f);
    CONTEND_EQUALITY(symtrack_cccf_get_ftype(q), LIQUID_FIRFILT_ARKAISER);
    CONTEND_EQUALITY(symtrack_cccf_get_modscheme(q), LIQUID_MODEM_QAM64);

    // test setting bandwidth
    CONTEND_EQUALITY(symtrack_cccf_set_bandwidth(q,  0.1f), LIQUID_OK);
    CONTEND_EQUALITY(symtrack_cccf_get_bandwidth(q), 0.1f);

    // test setting modulation scheme
    CONTEND_EQUALITY(symtrack_cccf_set_modscheme(q, LIQUID_MODEM_APSK16), LIQUID_OK);
    CONTEND_EQUALITY(symtrack_cccf_get_modscheme(q), LIQUID_MODEM_APSK16);

    // destroy object
    symtrack_cccf_destroy(q);
}
