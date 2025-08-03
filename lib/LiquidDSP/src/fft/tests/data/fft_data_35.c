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
// autotest fft data for 35-point transform
//

#include <complex.h>

float complex fft_test_x35[] = {
    0.621203985274 +  -1.170486588571*_Complex_I,
    0.001346936460 +   0.911992162865*_Complex_I,
    0.852266167932 +  -0.485602417107*_Complex_I,
   -1.440614617555 +  -0.143907395580*_Complex_I,
   -0.886824740774 +  -0.735649970983*_Complex_I,
    0.822316647558 +  -2.190416870881*_Complex_I,
   -0.848542045588 +   0.265512380995*_Complex_I,
    0.224076450439 +  -2.302360677412*_Complex_I,
   -0.510056035858 +  -1.331941821231*_Complex_I,
    1.682894720322 +   1.515017086970*_Complex_I,
   -0.293255187957 +   0.559255597549*_Complex_I,
    1.692889886331 +  -1.640844999342*_Complex_I,
   -1.032973350809 +   0.405524091118*_Complex_I,
    0.177413388869 +  -1.166118195604*_Complex_I,
   -0.077664879368 +  -1.471655833896*_Complex_I,
   -0.713927870525 +   1.118966977478*_Complex_I,
    3.112942596938 +   0.183067680138*_Complex_I,
   -2.132342418783 +   0.888305824658*_Complex_I,
    0.570015793758 +  -1.075297345387*_Complex_I,
    0.029232277101 +  -0.052772048068*_Complex_I,
    0.021810229730 +   1.026136624357*_Complex_I,
    0.183820954153 +   0.359011138378*_Complex_I,
   -0.646415125847 +   0.878859460005*_Complex_I,
    0.379503797426 +   0.690555809274*_Complex_I,
   -1.205501520591 +  -1.281704908973*_Complex_I,
   -0.671296057445 +  -1.202380495393*_Complex_I,
    1.000730854392 +   1.337218974137*_Complex_I,
   -0.507823699969 +   0.667041823900*_Complex_I,
    0.786493220706 +   0.711624105805*_Complex_I,
   -0.147343975279 +  -0.421666316903*_Complex_I,
    1.000992786069 +   0.072055493934*_Complex_I,
   -0.638324259500 +   0.503386775311*_Complex_I,
    1.518863022405 +  -0.447779056699*_Complex_I,
   -0.690987494413 +   0.744128091375*_Complex_I,
    0.074647158829 +  -1.048901976585*_Complex_I};

float complex fft_test_y35[] = {
    2.309567594431 +  -5.331826820366*_Complex_I,
   -7.442846930878 +  -4.725553620071*_Complex_I,
   -2.183297818919 +   6.404493143269*_Complex_I,
   -2.751014847029 +   2.658941664250*_Complex_I,
    8.330534466967 +   0.919074147759*_Complex_I,
    6.588325473188 +  -2.237962814214*_Complex_I,
   -7.983324045653 +  -4.751388462518*_Complex_I,
    9.842334426677 +  -4.886656356639*_Complex_I,
    2.723677138226 +  -1.864201873505*_Complex_I,
    3.314550864796 +  -2.486087207928*_Complex_I,
   -9.968089532702 + -11.225031015883*_Complex_I,
    3.348282695100 +   3.685146610266*_Complex_I,
    3.650591794608 +   7.864477419325*_Complex_I,
   12.324040428213 +  -2.521426915911*_Complex_I,
   -1.118920460046 +  -5.025187742730*_Complex_I,
   14.513400880941 +  11.251977643133*_Complex_I,
   -2.482556277234 +  -3.095357211645*_Complex_I,
   -7.549326283090 +   3.844289159351*_Complex_I,
   -6.554611159279 +  -1.426517057076*_Complex_I,
    6.564157092619 +   4.071022052298*_Complex_I,
    5.022588868136 + -15.817689097768*_Complex_I,
   -5.251727726400 +   2.789037545358*_Complex_I,
   -2.128113345135 + -15.921584799030*_Complex_I,
   -6.717549051974 +   1.209151038219*_Complex_I,
    3.222807461576 +   5.205190352829*_Complex_I,
   -2.420694516532 +  -6.628232249491*_Complex_I,
   -0.210938110900 +   1.317502940068*_Complex_I,
   -4.284810675111 +   4.379050426632*_Complex_I,
   -1.842031171135 +   3.520287066742*_Complex_I,
    5.259391106033 +  -9.692983760094*_Complex_I,
   -3.879590649031 +   2.871689364722*_Complex_I,
   -1.641216184114 +   4.267406466810*_Complex_I,
    1.294145378923 +  -2.702480168973*_Complex_I,
    1.886124883788 +  -3.741139781213*_Complex_I,
    7.958277715545 +  -3.144460685955*_Complex_I};

