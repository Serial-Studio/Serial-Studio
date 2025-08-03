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
#include "liquid.internal.h"

// test data
float complex sumsqcf_test_x3[3];   float sumsqcf_test_y3;
float complex sumsqcf_test_x4[4];   float sumsqcf_test_y4;
float complex sumsqcf_test_x7[7];   float sumsqcf_test_y7;
float complex sumsqcf_test_x8[8];   float sumsqcf_test_y8;
float complex sumsqcf_test_x15[15]; float sumsqcf_test_y15;
float complex sumsqcf_test_x16[16]; float sumsqcf_test_y16;

// helper function
void sumsqcf_runtest(float complex * _x,
                     unsigned int    _n,
                     float           _y)
{
    float tol = 1e-6;   // error tolerance

    // run test
    float y = liquid_sumsqcf(_x, _n);

    CONTEND_DELTA( y, _y, tol );
}

// 
// AUTOTESTS : run test with pre-determined data sets
//
void autotest_sumsqcf_3()   {   sumsqcf_runtest( sumsqcf_test_x3,  3,  sumsqcf_test_y3  );  }
void autotest_sumsqcf_4()   {   sumsqcf_runtest( sumsqcf_test_x4,  4,  sumsqcf_test_y4  );  }
void autotest_sumsqcf_7()   {   sumsqcf_runtest( sumsqcf_test_x7,  7,  sumsqcf_test_y7  );  }
void autotest_sumsqcf_8()   {   sumsqcf_runtest( sumsqcf_test_x8,  8,  sumsqcf_test_y8  );  }
void autotest_sumsqcf_15()  {   sumsqcf_runtest( sumsqcf_test_x15, 15, sumsqcf_test_y15 );  }
void autotest_sumsqcf_16()  {   sumsqcf_runtest( sumsqcf_test_x16, 16, sumsqcf_test_y16 );  }

float complex sumsqcf_test_x3[3] = {
  -0.143606511525 +  -0.137405158308*_Complex_I,
  -0.155077565599 +  -0.128712786230*_Complex_I,
   0.259257309730 +  -0.354313982924*_Complex_I};
float sumsqcf_test_y3 = 0.272871791516851;

float complex sumsqcf_test_x4[4] = {
  -0.027688113439 +   0.014257850202*_Complex_I,
   0.135913101830 +  -0.193497844930*_Complex_I,
  -0.184688262513 +  -0.018367564232*_Complex_I,
   0.033677897260 +  -0.365996497668*_Complex_I};
float sumsqcf_test_y4 = 0.226418463954813;

float complex sumsqcf_test_x7[7] = {
  -0.052790293375 +   0.173778162166*_Complex_I,
   0.026113336498 +  -0.228399854303*_Complex_I,
   0.060259677552 +  -0.064704230326*_Complex_I,
  -0.085637350173 +  -0.140391580928*_Complex_I,
   0.137662823620 +  -0.049602389650*_Complex_I,
   0.081078554377 +   0.103320097893*_Complex_I,
  -0.140068020211 +  -0.028552894932*_Complex_I};
float sumsqcf_test_y7 = 0.179790025178960;

float complex sumsqcf_test_x8[8] = {
  -0.114842287937 +  -0.044108491804*_Complex_I,
  -0.027032488500 +  -0.098073597323*_Complex_I,
  -0.248865158871 +  -0.058431293594*_Complex_I,
   0.152349654138 +   0.011146740847*_Complex_I,
   0.100890388238 +   0.037191727983*_Complex_I,
  -0.173317554621 +  -0.287191794305*_Complex_I,
   0.159045702603 +  -0.097006888823*_Complex_I,
  -0.048463564653 +  -0.123659611524*_Complex_I};
float sumsqcf_test_y8 = 0.290592731534459;

float complex sumsqcf_test_x15[15] = {
  -0.233166865552 +  -0.325575589001*_Complex_I,
  -0.062157314569 +  -0.052675113778*_Complex_I,
  -0.184924733094 +  -0.037448582846*_Complex_I,
  -0.019336799407 +  -0.146627815330*_Complex_I,
   0.014671587594 +  -0.040490423681*_Complex_I,
  -0.070920638099 +   0.353056761369*_Complex_I,
   0.342121380549 +   0.016365636789*_Complex_I,
   0.407809024847 +  -0.067677610212*_Complex_I,
   0.166345037028 +  -0.070618449000*_Complex_I,
  -0.151572833379 +  -0.241061531174*_Complex_I,
  -0.295395183108 +   0.107933512849*_Complex_I,
   0.214887288420 +   0.158211288996*_Complex_I,
   0.089528110626 +   0.534731503540*_Complex_I,
  -0.387245894254 +   0.127860010582*_Complex_I,
  -0.123711595377 +   0.212526707755*_Complex_I};
float sumsqcf_test_y15 = 1.44880523546855;

float complex sumsqcf_test_x16[16] = {
  -0.065168142317 +   0.069453199546*_Complex_I,
   0.175268433034 +  -0.227486860237*_Complex_I,
  -0.190532229460 +   0.079975095234*_Complex_I,
   0.119309235855 +  -0.238114343006*_Complex_I,
   0.125737810036 +   0.045214179459*_Complex_I,
  -0.197170380197 +  -0.159688600627*_Complex_I,
   0.075166226059 +   0.148949236785*_Complex_I,
  -0.290229918639 +   0.019293769432*_Complex_I,
  -0.145299853755 +  -0.083512058709*_Complex_I,
  -0.256618190275 +  -0.450932031739*_Complex_I,
  -0.169487127499 +   0.187004249967*_Complex_I,
   0.203885942759 +   0.121347578873*_Complex_I,
  -0.176280563451 +  -0.304717971490*_Complex_I,
   0.240587060249 +  -0.055540407201*_Complex_I,
   0.022889112723 +   0.027170265053*_Complex_I,
   0.265769617236 +  -0.023686695049*_Complex_I};
float sumsqcf_test_y16 = 1.07446555417927;
