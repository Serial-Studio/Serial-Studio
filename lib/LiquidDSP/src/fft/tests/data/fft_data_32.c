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

//
// autotest fft data for 32-point transform
//

#include <complex.h>

float complex fft_test_x32[] = {
    0.010014623512 +   0.557635892111*_Complex_I,
   -0.206536736342 +  -0.166543150147*_Complex_I,
    0.538145349965 +  -1.725439447290*_Complex_I,
    0.302742823470 +  -1.371600938217*_Complex_I,
    1.002711341702 +   0.326114543577*_Complex_I,
    0.141075699526 +   0.155158151624*_Complex_I,
   -0.607068327122 +   1.319516119806*_Complex_I,
    0.368752062157 +  -0.923715410356*_Complex_I,
    2.103742061281 +  -0.015112313666*_Complex_I,
    0.161141569742 +   0.602136525051*_Complex_I,
   -0.450622712484 +   1.781653187693*_Complex_I,
   -0.135385448029 +   0.118136375039*_Complex_I,
   -0.881476360789 +   0.743248990846*_Complex_I,
   -0.470865375669 +   0.297577338949*_Complex_I,
    1.578293870903 +   0.150586285410*_Complex_I,
   -0.582548890310 +  -0.090131349592*_Complex_I,
   -0.728216825909 +  -0.280660553441*_Complex_I,
   -0.405960077281 +  -0.823236356122*_Complex_I,
   -0.689782453002 +  -0.111721138015*_Complex_I,
   -0.483036608523 +  -2.255409787531*_Complex_I,
   -1.773638052539 +   0.276871847058*_Complex_I,
    1.768143481866 +   0.929411559777*_Complex_I,
    1.598162436054 +   0.336924712326*_Complex_I,
    0.025313208364 +   0.101634388542*_Complex_I,
    1.295932058649 +  -0.792267243573*_Complex_I,
    0.197302929571 +  -1.198246830820*_Complex_I,
   -0.674567746661 +   0.782836390216*_Complex_I,
   -1.155843266715 +  -2.060346979162*_Complex_I,
   -1.174296668808 +  -1.018450093532*_Complex_I,
   -2.374237969002 +   0.374087797223*_Complex_I,
    0.773269391236 +  -0.854101525457*_Complex_I,
   -0.320046719055 +  -0.112754382318*_Complex_I};

float complex fft_test_y32[] = {
   -1.249387330241 +  -4.946207393990*_Complex_I,
    6.246388763884 +  -6.773507691880*_Complex_I,
   -8.084921469264 + -10.845914953315*_Complex_I,
    4.357263742580 +  -0.359518611967*_Complex_I,
    0.869129033150 +   4.810140005791*_Complex_I,
   -4.320755060606 +  -2.990777115902*_Complex_I,
   -6.040916675986 +  11.391912365593*_Complex_I,
    1.334419178501 +   8.684519829396*_Complex_I,
    4.553475487339 +  -2.672989876360*_Complex_I,
    5.980003650433 +   0.810448601904*_Complex_I,
   -6.393705093395 +   4.226375095002*_Complex_I,
   -2.975901497257 +   6.587514214591*_Complex_I,
   -1.005810190439 +  -6.128898292417*_Complex_I,
    0.016624592858 +  -1.973370861210*_Complex_I,
    0.024605667140 +  -7.634608092684*_Complex_I,
    2.682037461215 +  -1.379684965569*_Complex_I,
    5.090591302217 +   7.901478702130*_Complex_I,
    4.974928932755 +  -2.673377557945*_Complex_I,
    2.018958625049 +   1.429758433848*_Complex_I,
   -3.215930811783 +  -5.513827987988*_Complex_I,
    9.696021083826 +   2.712450847683*_Complex_I,
   -6.175824724853 +   6.515234684364*_Complex_I,
  -10.139408716624 +   3.456026873018*_Complex_I,
    3.811736373367 +   3.481961808174*_Complex_I,
   -8.975590750922 +  -1.092757154258*_Complex_I,
    1.006822057237 +  -4.766098514087*_Complex_I,
   -0.499087378413 +   4.387815733060*_Complex_I,
   -0.223489533627 +   2.032893439147*_Complex_I,
   12.473346705334 +  -4.826450587127*_Complex_I,
    4.394902822920 +  12.095339998118*_Complex_I,
   -3.828535537115 +   2.263473712749*_Complex_I,
   -6.081522756885 +  -0.365006140312*_Complex_I};

