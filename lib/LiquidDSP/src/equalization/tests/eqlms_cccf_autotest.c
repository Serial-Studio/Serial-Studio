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

// static channel filter, run equalization with various update strategies on modulated data
void testbench_eqlms(unsigned int k, unsigned int m, float beta, int init,
                     unsigned int p, float mu, unsigned int num_symbols,
                     int update, int ms)
{
    //float          tol    = 0.025f; // error tolerance
    unsigned int   i;
    modemcf        mod    = modemcf_create(ms);
    firinterp_crcf interp = firinterp_crcf_create_prototype(LIQUID_FIRFILT_ARKAISER,k,m,beta,0);

    // create fixed channel filter
    float complex h[5] = {
         1.00f +  0.00f*_Complex_I,
         0.00f + -0.01f*_Complex_I,
        -0.11f +  0.02f*_Complex_I,
         0.02f +  0.01f*_Complex_I,
        -0.09f + -0.04f*_Complex_I };
    firfilt_cccf fchannel = firfilt_cccf_create(h,5);

    // prototype low-pass filter
    float complex hp[2*k*p+1];
    for (i=0; i<2*k*p+1; i++)
        hp[i] = sincf( (float)i/(float)k - p) * liquid_hamming(i,2*k*p+1) / k;

    // create and initialize equalizer
    eqlms_cccf eq;
    switch (init) {
    case 0: eq = eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_ARKAISER,k,p,beta,0); break;
    case 1: eq = eqlms_cccf_create_lowpass (2*k*p+1, 0.5f/(float)k); break;
    case 2: eq = eqlms_cccf_create         (hp, 2*k*p+1); break; // external coefficients
    default:eq = eqlms_cccf_create         (NULL, 2*k*p+1); break; // NULL puts 1 at center
    }
    eqlms_cccf_set_bw(eq, mu);

    // run equalization
    float complex buf[k];                   // sample buffer
    float complex sym_in, sym_out;          // modulated/recovered symbols
    wdelaycf buf_sym = wdelaycf_create(m+p);// symbol buffer to account for filter delays
    float rmse = 0.0f; // root mean-squared error
    for (i=0; i<2*num_symbols; i++) {
        // generate modulated input symbol
        unsigned int sym = modemcf_gen_rand_sym(mod);
        modemcf_modulate(mod, sym, &sym_in);
        wdelaycf_push(buf_sym, sym_in);

        // interpolate
        firinterp_crcf_execute(interp, sym_in, buf);

        // apply channel filter (in place)
        firfilt_cccf_execute_block(fchannel, buf, k, buf);

        // run through equalizing filter as decimator
        eqlms_cccf_decim_execute(eq, buf, &sym_out, k);

        // skip first m+p symbols
        if (i < m + p) continue;

        // read input symbol, delayed by m+p symbols
        wdelaycf_read(buf_sym, &sym_in);

        // update equalizer weights
        if (i < num_symbols) {
            float complex d_hat;
            unsigned int  index;
            switch (update) {
            case 0: eqlms_cccf_step(eq, sym_in, sym_out); break; // perfect knowledge
            case 1: eqlms_cccf_step_blind(eq, sym_out);   break; // CM
            case 2:
                // decision-directed
                modemcf_demodulate(mod, sym_out, &index);
                modemcf_get_demodulator_sample(mod, &d_hat);
                eqlms_cccf_step(eq, d_hat, sym_out);
                break;
            default:;
            }
            continue;
        }

        // observe error
        float error = cabsf(sym_in-sym_out);
        rmse += error * error;
#if 0
        if (liquid_autotest_verbose) {
            printf("%3u : x = {%12.8f,%12.8f}, y = {%12.8f,%12.8f}, error=%12.8f %s\n",
                    i, crealf(sym_in ), cimagf(sym_in ), crealf(sym_out), cimagf(sym_out),
                    error, error > tol ? "*" : "");
        }
        //CONTEND_DELTA(error, 0.0f, tol);
#endif
    }
    rmse = 10*log10f( rmse/num_symbols );
    printf("rmse : %.3f dB\n", rmse);
    CONTEND_LESS_THAN(rmse, -20.0f);

    // clean up objects
    wdelaycf_destroy(buf_sym);
    firfilt_cccf_destroy(fchannel);
    firinterp_crcf_destroy(interp);
    eqlms_cccf_destroy(eq);
    modemcf_destroy(mod);
}

// test different update strategies:       k,m,beta,init,p, mu,  n,update,mod scheme
void autotest_eqlms_00() { testbench_eqlms(2,7, 0.3,   0,7,0.3,800,     0,LIQUID_MODEM_QPSK); }
void autotest_eqlms_01() { testbench_eqlms(2,7, 0.3,   0,7,0.3,800,     1,LIQUID_MODEM_QPSK); }
void autotest_eqlms_02() { testbench_eqlms(2,7, 0.3,   0,7,0.3,800,     2,LIQUID_MODEM_QPSK); }

// test different initialization methods:  k,m,beta,init,p, mu,  n,update,mod scheme
void autotest_eqlms_03() { testbench_eqlms(2,7, 0.3,   0,7,0.3,800,     0,LIQUID_MODEM_QAM16); }
void autotest_eqlms_04() { testbench_eqlms(2,7, 0.3,   1,7,0.3,800,     0,LIQUID_MODEM_QAM16); }
void autotest_eqlms_05() { testbench_eqlms(2,7, 0.3,   2,7,0.3,800,     0,LIQUID_MODEM_QAM16); }
void autotest_eqlms_06() { testbench_eqlms(2,7, 0.3,   3,6,0.3,800,     0,LIQUID_MODEM_QAM16); }

// test different configurations:          k,m,beta,init,p, mu,  n,update,mod scheme
void autotest_eqlms_07() { testbench_eqlms(2,9, 0.3,   0,7,0.3,800,     0,LIQUID_MODEM_QPSK); }
void autotest_eqlms_08() { testbench_eqlms(2,7, 0.2,   0,9,0.3,800,     0,LIQUID_MODEM_QPSK); }
void autotest_eqlms_09() { testbench_eqlms(2,7, 0.3,   0,3,0.3,800,     0,LIQUID_MODEM_QPSK); }
void autotest_eqlms_10() { testbench_eqlms(2,7, 0.3,   0,7,0.5,800,     0,LIQUID_MODEM_ARB64VT); }
void autotest_eqlms_11() { testbench_eqlms(2,7, 0.3,   0,7,0.1,800,     0,LIQUID_MODEM_QPSK); }
//void xautotest_eqlms_12() { testbench_eqlms(4,7, 0.3,   0,7,0.7,800,     0,LIQUID_MODEM_QPSK); }

void autotest_eqlms_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping eqlms config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check that object returns NULL for invalid configurations
    CONTEND_ISNULL(eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, 0, 12, 0.3f, 0.0f));
    CONTEND_ISNULL(eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, 2,  0, 0.3f, 0.0f));
    CONTEND_ISNULL(eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, 2, 12, 2.0f, 0.0f));
    CONTEND_ISNULL(eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, 2, 12, 0.3f,-2.0f));

    CONTEND_ISNULL(eqlms_cccf_create_lowpass( 0, 0.1f));
    CONTEND_ISNULL(eqlms_cccf_create_lowpass(13,-0.1f));

    // create proper object and test other interfaces
    unsigned int i, k=2, m=3, h_len = 2*k*m+1;
    eqlms_cccf q = eqlms_cccf_create(NULL, h_len);
    CONTEND_EQUALITY(LIQUID_OK, eqlms_cccf_print(q));

    // test getting/setting properties
    CONTEND_EQUALITY(eqlms_cccf_get_length(q), h_len);
    float mu = 0.1f;
    eqlms_cccf_set_bw(q, mu);
    CONTEND_EQUALITY(eqlms_cccf_get_bw(q), mu);
    CONTEND_INEQUALITY(LIQUID_OK, eqlms_cccf_set_bw(q, -1));

    // other configurations
    CONTEND_INEQUALITY(LIQUID_OK, eqlms_cccf_decim_execute(q, NULL, NULL, 0));

    // test getting weights
    float complex h[h_len];
    eqlms_cccf_copy_coefficients(q, h);
    for (i=0; i<h_len; i++)
        CONTEND_EQUALITY(h[i], i==k*m ? 1 : 0);
    const float complex * w = eqlms_cccf_get_coefficients(q);
    for (i=0; i<h_len; i++)
        CONTEND_EQUALITY(w[i], i==k*m ? 1 : 0);

    // clean it up
    eqlms_cccf_destroy(q);
}

void autotest_eqlms_cccf_copy()
{
    // create initial object
    eqlms_cccf q0 = eqlms_cccf_create_lowpass(21, 0.12345f);
    eqlms_cccf_set_bw(q0, 0.1f);
    eqlms_cccf_print(q0);

    // run random samples through object
    unsigned int i;
    float complex x, v, y0, y1;
    for (i=0; i<120; i++) {
        x = randnf() + _Complex_I*randnf();
        eqlms_cccf_push(q0, x);
    }

    // copy object
    eqlms_cccf q1 = eqlms_cccf_copy(q0);

    // run random samples through both objects
    for (i=0; i<120; i++) {
        // push random sample in
        x = randnf() + _Complex_I*randnf();
        eqlms_cccf_push(q0, x);
        eqlms_cccf_push(q1, x);

        // compute output
        eqlms_cccf_execute(q0, &y0);
        eqlms_cccf_execute(q1, &y1);
        CONTEND_EQUALITY(y0, y1);

        // step equalization algorithm
        v = randnf() + _Complex_I*randnf();
        eqlms_cccf_step(q0, v, y0);
        eqlms_cccf_step(q1, v, y1);
    }

    // get and compare coefficients
    CONTEND_SAME_DATA(eqlms_cccf_get_coefficients(q0),
                      eqlms_cccf_get_coefficients(q1),
                      21 * sizeof(float complex));

    // destroy filter objects
    eqlms_cccf_destroy(q0);
    eqlms_cccf_destroy(q1);
}
