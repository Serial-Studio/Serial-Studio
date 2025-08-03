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
// autotest fft data for 26-point transform
//

#include <complex.h>

float complex fft_test_x26[] = {
   -1.513790990128 +   0.450104660529*_Complex_I,
   -0.609587704775 +   0.200963343771*_Complex_I,
    1.150854971928 +  -0.979670346844*_Complex_I,
    0.676761753784 +  -0.390760850862*_Complex_I,
    0.025326431025 +   0.226613394038*_Complex_I,
   -0.877894422758 +   0.377687762743*_Complex_I,
    0.016945667503 +  -0.414424826825*_Complex_I,
    0.671396901344 +   1.014597796222*_Complex_I,
    1.620562100771 +  -0.445584464270*_Complex_I,
   -0.621912682345 +   0.523347355420*_Complex_I,
   -1.722706628967 +  -1.473722873869*_Complex_I,
    1.604125850850 +  -0.595523792175*_Complex_I,
    2.695969244871 +  -0.740444785313*_Complex_I,
   -1.837539349404 +  -0.402987576873*_Complex_I,
    0.643703593669 +  -0.530984627964*_Complex_I,
    0.745985203740 +  -1.158124796569*_Complex_I,
    0.492860315079 +   0.183477887101*_Complex_I,
   -0.715219690752 +  -0.979086251385*_Complex_I,
    0.179883358483 +   0.236135674483*_Complex_I,
    0.196402574786 +  -1.059380996958*_Complex_I,
    0.069933652344 +   0.829344522775*_Complex_I,
    0.325146685501 +  -1.266467132602*_Complex_I,
   -1.839777223485 +  -0.327473446299*_Complex_I,
    1.778506841624 +  -1.571433253340*_Complex_I,
    1.002599293378 +   0.297630901673*_Complex_I,
   -1.821267785996 +   1.027493831629*_Complex_I};

float complex fft_test_y26[] = {
    2.337267962069 +  -6.968672891765*_Complex_I,
   -1.374144569280 +   2.440460452235*_Complex_I,
    0.293963382503 +  -1.014603415840*_Complex_I,
   -5.808647515741 +   2.932080281412*_Complex_I,
   -3.954391977726 +   4.050421232706*_Complex_I,
  -10.434029005073 +  -2.430379478773*_Complex_I,
  -14.396971442054 +   7.561455425624*_Complex_I,
    4.801073402095 +  -7.297832464880*_Complex_I,
   -6.451827528493 +  -1.778154888801*_Complex_I,
    7.694740702469 +  -3.586296872724*_Complex_I,
    0.830361317715 +  -4.000025563805*_Complex_I,
    2.193823341792 +   2.627715132519*_Complex_I,
    5.557050661872 +   3.023112572307*_Complex_I,
    3.307459610871 +   1.590676230192*_Complex_I,
  -12.118932653799 +  -2.054113664628*_Complex_I,
    5.893155083845 +  -6.130479717222*_Complex_I,
    1.236890091413 +  -1.250300895885*_Complex_I,
    6.703265906214 +   0.834044438577*_Complex_I,
   -7.565202099594 +   2.208332250508*_Complex_I,
    3.230245599909 +   6.555949348851*_Complex_I,
   -7.293883944894 +  -3.228615872305*_Complex_I,
   -4.814526866453 +   4.783901741180*_Complex_I,
   -0.579907911716 +   5.262229129419*_Complex_I,
   -0.768605942699 +   3.726121287503*_Complex_I,
   -1.461710271209 +  -1.198541330006*_Complex_I,
   -6.415081077369 +   5.044238707360*_Complex_I};

