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

// 
// AUTOTEST: log Modified Bessel function of the first kind
//
void autotest_lnbesselif()
{
    float tol = 1e-5f;

    // test lnbesselif(nu,z) with various values for nu and z
    CONTEND_DELTA( liquid_lnbesselif( 0.0f, 0.0f),    0.0,                tol );
    CONTEND_DELTA( liquid_lnbesselif( 0.0f, 0.1f),    0.00249843923387607,tol );
    CONTEND_DELTA( liquid_lnbesselif( 0.1f, 7.1f),    5.21933724549090,   tol );
    CONTEND_DELTA( liquid_lnbesselif( 0.3f, 2.1f),    0.853008130814754,  tol );
    CONTEND_DELTA( liquid_lnbesselif( 0.9f, 9.3f),    7.23414120004177,   tol );
    CONTEND_DELTA( liquid_lnbesselif( 1.0f, 0.1f),   -2.99448253386220,   tol );
    CONTEND_DELTA( liquid_lnbesselif( 1.7f, 0.01f),  -9.44195081753909,   tol );
    CONTEND_DELTA( liquid_lnbesselif( 1.8f, 1e-3f), -14.1983271298778,    tol );
    CONTEND_DELTA( liquid_lnbesselif( 1.9f, 8.7f),    6.49469148684252,   tol );
    CONTEND_DELTA( liquid_lnbesselif( 4.9f, 0.01f), -30.5795429642925,    tol );
    CONTEND_DELTA( liquid_lnbesselif( 7.4f, 9.3f),    4.33486237261960,   tol );

    // test large values of nu
    CONTEND_DELTA( liquid_lnbesselif( 20.0f,  3.0f),  -34.1194307343208, tol);
    CONTEND_DELTA( liquid_lnbesselif( 30.0f,  3.0f),  -62.4217845317278, tol);
#if 0
    CONTEND_DELTA( liquid_lnbesselif( 35.0f,  3.0f),  -77.8824494916507, tol);
    CONTEND_DELTA( liquid_lnbesselif( 38.0f,  3.0f),  -87.5028737258841, tol);
    CONTEND_DELTA( liquid_lnbesselif( 39.0f,  3.0f),  -90.7624095618186, tol);
    CONTEND_DELTA( liquid_lnbesselif( 40.0f,  3.0f),  -94.0471931331690, tol);
    CONTEND_DELTA( liquid_lnbesselif( 80.0f,  3.0f), -241.208142562073,  tol);
    CONTEND_DELTA( liquid_lnbesselif( 140.0f, 3.0f), -498.439222461430,  tol);
#endif
}

// 
// AUTOTEST: Modified Bessel function of the first kind
//
void autotest_besselif()
{
    float tol = 1e-3f;

    // check case when nu=0
    CONTEND_DELTA(liquid_besselif(0.0f,0.0f), 1.0f, tol);
    CONTEND_DELTA(liquid_besselif(0.0f,0.1f), 1.00250156293410f, tol);
    CONTEND_DELTA(liquid_besselif(0.0f,0.2f), 1.01002502779515f, tol);
    CONTEND_DELTA(liquid_besselif(0.0f,0.5f), 1.06348337074132f, tol);
    CONTEND_DELTA(liquid_besselif(0.0f,1.0f), 1.26606587775201f, tol);
    CONTEND_DELTA(liquid_besselif(0.0f,2.0f), 2.27958530233607f, tol);
    CONTEND_DELTA(liquid_besselif(0.0f,3.0f), 4.88079258586503f, tol);

    // check case when nu=0.5
    CONTEND_DELTA(liquid_besselif(0.5f,0.0f), 0.000000000000000, tol);
    CONTEND_DELTA(liquid_besselif(0.5f,0.1f), 0.252733984600132, tol);
    CONTEND_DELTA(liquid_besselif(0.5f,0.2f), 0.359208417583362, tol);
    CONTEND_DELTA(liquid_besselif(0.5f,0.5f), 0.587993086790417, tol);
    CONTEND_DELTA(liquid_besselif(0.5f,1.0f), 0.937674888245489, tol);
    CONTEND_DELTA(liquid_besselif(0.5f,2.0f), 2.046236863089057, tol);
    CONTEND_DELTA(liquid_besselif(0.5f,3.0f), 4.614822903407577, tol);

    // check case when nu=1.3
    CONTEND_DELTA(liquid_besselif(1.3f,0.0f), 0.000000000000000, tol);
    CONTEND_DELTA(liquid_besselif(1.3f,0.1f), 0.017465030873157, tol);
    CONTEND_DELTA(liquid_besselif(1.3f,0.2f), 0.043144293848607, tol);
    CONTEND_DELTA(liquid_besselif(1.3f,0.5f), 0.145248507279042, tol);
    CONTEND_DELTA(liquid_besselif(1.3f,1.0f), 0.387392350983796, tol);
    CONTEND_DELTA(liquid_besselif(1.3f,2.0f), 1.290819215135879, tol);
    CONTEND_DELTA(liquid_besselif(1.3f,3.0f), 3.450680420553085, tol);
}

// 
// AUTOTEST: Modified Bessel function of the first kind
//
void autotest_besseli0f()
{
    float tol = 1e-3f;
    CONTEND_DELTA(liquid_besseli0f(0.0f), 1.0f, tol);
    CONTEND_DELTA(liquid_besseli0f(0.1f), 1.00250156293410f, tol);
    CONTEND_DELTA(liquid_besseli0f(0.2f), 1.01002502779515f, tol);
    CONTEND_DELTA(liquid_besseli0f(0.5f), 1.06348337074132f, tol);
    CONTEND_DELTA(liquid_besseli0f(1.0f), 1.26606587775201f, tol);
    CONTEND_DELTA(liquid_besseli0f(2.0f), 2.27958530233607f, tol);
    CONTEND_DELTA(liquid_besseli0f(3.0f), 4.88079258586503f, tol);
}

// 
// AUTOTEST: Bessel function of the first kind
//
void autotest_besseljf()
{
    float tol = 1e-3f;

    // check case when nu=0
    CONTEND_DELTA(liquid_besseljf(0.0f,0.0f),  1.000000000000000, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,0.1f),  0.997501562066040, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,0.2f),  0.990024972239576, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,0.5f),  0.938469807240813, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,1.0f),  0.765197686557967, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,2.0f),  0.223890779141236, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,3.0f), -0.260051954901934, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,4.0f), -0.397149809863847, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,6.0f),  0.150645257250997, tol);
    CONTEND_DELTA(liquid_besseljf(0.0f,8.0f),  0.171650807137554, tol);

    // check case when nu=0.5
    CONTEND_DELTA(liquid_besseljf(0.5f,0.0f),  0.000000000000000, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,0.1f),  0.251892940326001, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,0.2f),  0.354450744211402, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,0.5f),  0.540973789934529, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,1.0f),  0.671396707141804, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,2.0f),  0.513016136561828, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,3.0f),  0.065008182877376, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,4.0f), -0.301920513291637, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,6.0f), -0.091015409523068, tol);
    CONTEND_DELTA(liquid_besseljf(0.5f,8.0f),  0.279092808570990, tol);

    // check case when nu=1.7
    CONTEND_DELTA(liquid_besseljf(1.7f,0.0f),  0.000000000000000, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,0.1f),  0.003971976455203, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,0.2f),  0.012869169735073, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,0.5f),  0.059920175825578, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,1.0f),  0.181417665056645, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,2.0f),  0.437811462130677, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,3.0f),  0.494432522734784, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,4.0f),  0.268439400467270, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,6.0f), -0.308175744215833, tol);
    CONTEND_DELTA(liquid_besseljf(1.7f,8.0f), -0.001102600927987, tol);
}

// 
// AUTOTEST: Bessel function of the first kind
//
void autotest_besselj0f()
{
    float tol = 1e-3f;
    CONTEND_DELTA(liquid_besselj0f(0.0f),  1.0f, tol);
    CONTEND_DELTA(liquid_besselj0f(0.1f),  0.997501562066040f, tol);
    CONTEND_DELTA(liquid_besselj0f(0.2f),  0.990024972239576f, tol);
    CONTEND_DELTA(liquid_besselj0f(0.5f),  0.938469807240813f, tol);
    CONTEND_DELTA(liquid_besselj0f(1.0f),  0.765197686557967f, tol);
    CONTEND_DELTA(liquid_besselj0f(2.0f),  0.223890779141236f, tol);
    CONTEND_DELTA(liquid_besselj0f(2.5f), -0.048383776468199f, tol);
    CONTEND_DELTA(liquid_besselj0f(3.0f), -0.260051954901934f, tol);
    CONTEND_DELTA(liquid_besselj0f(3.5f), -0.380127739987263f, tol);
    CONTEND_DELTA(liquid_besselj0f(4.0f), -0.397149809863848f, tol);
    CONTEND_DELTA(liquid_besselj0f(4.5f), -0.320542508985121f, tol);
}

