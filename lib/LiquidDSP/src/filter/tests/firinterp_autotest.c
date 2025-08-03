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

#include "autotest/autotest.h"
#include "liquid.h"

void autotest_firinterp_rrrf_common()
{
    firinterp_rrrf interp = firinterp_rrrf_create_kaiser(17, 4, 60.0f);
    CONTEND_EQUALITY(firinterp_rrrf_get_interp_rate(interp), 17);
    firinterp_rrrf_destroy(interp);
}

void autotest_firinterp_crcf_common()
{
    firinterp_crcf interp = firinterp_crcf_create_kaiser(7, 4, 60.0f);
    CONTEND_EQUALITY(firinterp_crcf_get_interp_rate(interp), 7);
    firinterp_crcf_destroy(interp);
}

//
// AUTOTEST: 
//
void autotest_firinterp_rrrf_generic()
{
    float h[9] = {
      -0.2762293319046737,
       1.4757679031218007,
       0.1432569489572376,
      -0.2142368750177835,
       1.3471241294836864,
       0.1166010284926269,
       0.0536534505390281,
       0.1412672462812405,
      -0.0991854372394269};

    unsigned int M = 4;     // firinterp factor
    firinterp_rrrf q = firinterp_rrrf_create(M,h,9);

    float x[] = {1.0, -1.0, 1.0, 1.0};
    float y[16];
    float test[16] = {
      -0.2762293319046737,
       1.4757679031218007,
       0.1432569489572376,
      -0.2142368750177835,
       1.6233534613883602,
      -1.3591668746291738,
      -0.0896034984182095,
       0.3555041212990241,
      -1.7225388986277870,
       1.3591668746291738,
       0.0896034984182095,
      -0.3555041212990241,
       1.1700802348184398,
       1.5923689316144276,
       0.1969103994962658,
      -0.0729696287365430};

    float tol = 1e-6;

    unsigned int i;
    for (i=0; i<4; i++)
        firinterp_rrrf_execute(q, x[i], &y[i*M]);

    for (i=0; i<16; i++) {
        CONTEND_DELTA(y[i], test[i], tol);

        if (liquid_autotest_verbose)
            printf("  y(%u) = %8.4f;\n", i+1, y[i]);
    }

    if (liquid_autotest_verbose)
        firinterp_rrrf_print(q);

    // destroy interpolator object
    firinterp_rrrf_destroy(q);
}

void autotest_firinterp_crcf_generic()
{
    // h = [0, 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25, 0];
    float h[9] = {
      -0.7393353832652201,
       0.1909821993029451,
      -1.7013834621383086,
      -0.6157406339062349,
       0.5806218191269317,
       0.0576963976148674,
      -1.0958217797368455,
      -0.6379821629743743,
       0.7019489165905530};

    unsigned int M = 4;     // firinterp factor
    firinterp_crcf q = firinterp_crcf_create(M,h,9);

    //  x = [1+j*0.2, -0.2+j*1.3, 0.5+j*0.3, 1.1-j*0.2]
    float complex x[4] = {
      1.0000e+00+  2.0000e-01*_Complex_I, 
     -2.0000e-01+  1.3000e+00*_Complex_I, 
      5.0000e-01+  3.0000e-01*_Complex_I, 
      1.1000e+00+ -2.0000e-01*_Complex_I
    };
        
    float complex y[16];

    // z = [x(1) 0 0 0 x(2) 0 0 0 x(3) 0 0 0 x(4) 0 0 0];
    // test = filter(h,1,z)
    float complex test[16] = {
      -0.7393353832652201 - 0.1478670766530440*_Complex_I,
       0.1909821993029451 + 0.0381964398605890*_Complex_I,
      -1.7013834621383086 - 0.3402766924276617*_Complex_I,
      -0.6157406339062349 - 0.1231481267812470*_Complex_I,
       0.7284888957799757 - 0.8450116344193997*_Complex_I,
       0.0194999577542784 + 0.2598161386168021*_Complex_I,
      -0.7555450873091838 - 2.4309628567271702*_Complex_I,
      -0.5148340361931273 - 0.9280592566729803*_Complex_I,
       0.2161568611325566 + 0.6733975332035558*_Complex_I,
       0.0839518201284991 + 0.1322999766902112*_Complex_I,
      -0.6315273751217851 - 1.9349833522993918*_Complex_I,
      -0.1802738843582426 - 1.0140990020385570*_Complex_I,
      -0.6633477953463869 + 1.2345872139588425*_Complex_I,
       0.2389286180406733 - 0.0208875205761288*_Complex_I,
      -2.4194326982205623 + 0.0115301585066081*_Complex_I,
      -0.9963057787840456 - 0.0682465221110653*_Complex_I };

    float tol = 1e-6;

    unsigned int i;
    for (i=0; i<4; i++)
        firinterp_crcf_execute(q, x[i], &y[i*M]);

    for (i=0; i<16; i++) {
        CONTEND_DELTA( crealf(y[i]), crealf(test[i]), tol);
        CONTEND_DELTA( cimagf(y[i]), cimagf(test[i]), tol);

        if (liquid_autotest_verbose)
            printf("  y(%u) = %8.4f + j%8.4f;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }

    if (liquid_autotest_verbose)
        firinterp_crcf_print(q);

    // destroy interpolator object
    firinterp_crcf_destroy(q);
}

// test Nyquist filter
void testbench_firinterp_crcf_nyquist(int          _ftype,
                                      unsigned int _M,
                                      unsigned int _m,
                                      float        _beta)
{
    float tol = 1e-6;
    // create interpolator object
    firinterp_crcf q = firinterp_crcf_create_prototype(_ftype, _M, _m, _beta, 0);

    // create input buffer of symbols to interpolate
    unsigned int num_symbols = _m+16;   //
    float complex x[num_symbols];       // input symbols
    float complex y[_M];                // output interp buffer
    unsigned int i;
    for (i=0; i<num_symbols; i++)
        x[i] = cexpf(_Complex_I*0.7f*(float)i);

    for (i=0; i<num_symbols; i++) {
        // interpolate and store into output buffer
        firinterp_crcf_execute(q, x[i], y);

        // for a Nyquist filter, output should match input at
        // proper sampling time (compensating for delay)
        if (i >= _m) {
            CONTEND_DELTA( crealf(x[i-_m]), crealf(y[0]), tol);
            CONTEND_DELTA( cimagf(x[i-_m]), cimagf(y[0]), tol);

            if (liquid_autotest_verbose) {
                printf("%3u: x=%8.4f + j%8.4f, y=%8.4f + j%8.4f;\n", i+1,
                        crealf(x[i-_m]), cimagf(x[i-_m]),
                        crealf(y[   0]), cimagf(y[   0]));
            }
        }
    }

    // destroy interpolator object
    firinterp_crcf_destroy(q);
}

// add specific tests
void autotest_firinterp_crcf_rnyquist_0() 
    { testbench_firinterp_crcf_nyquist(LIQUID_FIRFILT_KAISER, 2, 9,0.3f); }

void autotest_firinterp_crcf_rnyquist_1() 
    { testbench_firinterp_crcf_nyquist(LIQUID_FIRFILT_KAISER, 3, 9,0.3f); }

void autotest_firinterp_crcf_rnyquist_2() 
    { testbench_firinterp_crcf_nyquist(LIQUID_FIRFILT_KAISER, 7, 9,0.3f); }

void autotest_firinterp_crcf_rnyquist_3() 
    { testbench_firinterp_crcf_nyquist(LIQUID_FIRFILT_RCOS,   2, 9,0.3f); }

// test copy method
void autotest_firinterp_copy()
{
    // create base object
    firinterp_crcf q0 = firinterp_crcf_create_kaiser(3, 7, 60.0f);
    firinterp_crcf_set_scale(q0, 0.12345f);

    // run samples through filter
    unsigned int i;
    float complex buf_0[3], buf_1[3];
    for (i=0; i<20; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        firinterp_crcf_execute(q0, v, buf_0);
    }

    // copy object
    firinterp_crcf q1 = firinterp_crcf_copy(q0);

    // run samples through both filters in parallel
    for (i=0; i<60; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        firinterp_crcf_execute(q0, v, buf_0);
        firinterp_crcf_execute(q1, v, buf_1);

        CONTEND_SAME_DATA( buf_0, buf_1, 3*sizeof(float complex) );
    }

    // destroy objects
    firinterp_crcf_destroy(q0);
    firinterp_crcf_destroy(q1);
}

// test flush method(s)
void autotest_firinterp_flush()
{
    // create base object
    unsigned int m = 7;
    firinterp_crcf q = firinterp_crcf_create_kaiser(3, m, 60.0f);

    // run samples through filter
    unsigned int i;
    float complex buf[3];
    for (i=0; i<20; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        firinterp_crcf_execute(q, v, buf);
    }

    // ensure buffer does not contain zeros
    CONTEND_GREATER_THAN( liquid_sumsqcf(buf,3), 0.0f );

    // flush buffer
    for (i=0; i<2*m; i++)
        firinterp_crcf_flush(q, buf);

    // ensure buffer contains only zeros
    CONTEND_EQUALITY( liquid_sumsqcf(buf,3), 0.0f );

    // destroy objects
    firinterp_crcf_destroy(q);
}

