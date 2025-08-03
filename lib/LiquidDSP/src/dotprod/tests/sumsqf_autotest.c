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
float sumsqf_test_x3[3];    float sumsqf_test_y3;
float sumsqf_test_x4[4];    float sumsqf_test_y4;
float sumsqf_test_x7[7];    float sumsqf_test_y7;
float sumsqf_test_x8[8];    float sumsqf_test_y8;
float sumsqf_test_x15[15];  float sumsqf_test_y15;
float sumsqf_test_x16[16];  float sumsqf_test_y16;

// helper function
void sumsqf_runtest(float *      _x,
                    unsigned int _n,
                    float        _y)
{
    float tol = 1e-6;   // error tolerance

    // run test
    float y = liquid_sumsqf(_x, _n);

    CONTEND_DELTA( y, _y, tol );
}

// 
// AUTOTESTS : run test with pre-determined data sets
//
void autotest_sumsqf_3()    {   sumsqf_runtest( sumsqf_test_x3,  3,  sumsqf_test_y3  ); }
void autotest_sumsqf_4()    {   sumsqf_runtest( sumsqf_test_x4,  4,  sumsqf_test_y4  ); }
void autotest_sumsqf_7()    {   sumsqf_runtest( sumsqf_test_x7,  7,  sumsqf_test_y7  ); }
void autotest_sumsqf_8()    {   sumsqf_runtest( sumsqf_test_x8,  8,  sumsqf_test_y8  ); }
void autotest_sumsqf_15()   {   sumsqf_runtest( sumsqf_test_x15, 15, sumsqf_test_y15 ); }
void autotest_sumsqf_16()   {   sumsqf_runtest( sumsqf_test_x16, 16, sumsqf_test_y16 ); }

float sumsqf_test_x3[3] = {
  -0.4546496371984978f,
   0.4451201395218938f,
   0.0138788690209525f};
float sumsqf_test_y3 = 0.405030854218017;

float sumsqf_test_x4[4] = {
   0.1322698385026883f,
  -0.0569081631536912f,
  -0.3244384492417431f,
  -0.2872733941910143f};
float sumsqf_test_y4 = 0.208520159567467;

float sumsqf_test_x7[7] = {
  -0.221079351597278f,
  -0.227902662215897f,
   0.382941891419158f,
   0.246800053933030f,
  -0.190152017725480f,
   0.395758452636014f,
   0.211220685416265f};
float sumsqf_test_y7 = 0.545767182598435;

float sumsqf_test_x8[8] = {
  -0.3405090291337944f,
   0.5568858414046379f,
  -0.0870643704340343f,
   0.1724369367547939f,
  -0.7379946538182081f,
  -0.3514326419380984f,
   0.2782541955998314f,
   0.4354875172406391f};
float sumsqf_test_y8 = 1.39859872696022;

float sumsqf_test_x15[15] = {
  -0.4630291295549499f,
  -0.2776019612369674f,
  -0.4933486186123937f,
  -0.0850997992116534f,
   0.0117036410972943f,
   0.0215560948199280f,
   0.1203298759952301f,
   0.5866344749815807f,
   0.3791165816771581f,
  -0.4070288299889871f,
  -0.4971431800502791f,
  -0.2142770391709351f,
   0.3330589842198580f,
  -0.0150675851612766f,
  -0.3947266044391958f};
float sumsqf_test_y15 = 1.77074683901981f;

float sumsqf_test_x16[16] = {
  -0.2975264216819841f,
   0.5642827287388987f,
  -0.7956166087428503f,
  -0.1931368701566655f,
  -0.0287212417958668f,
   0.3697266870899014f,
   0.0791822603183984f,
   0.1668276194302965f,
   0.2048176237333448f,
  -0.0617609549162579f,
   0.5317006403634014f,
  -0.3964290790294236f,
   0.5404940967800361f,
   0.1755457122664283f,
   0.1585602895144933f,
   0.0791731424937176f};
float sumsqf_test_y16 = 2.08885480396333f;

