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
// autotest fft data for 20-point transform
//

#include <complex.h>

float complex fft_test_x20[] = {
   -0.138174469322 +  -0.274748504122*_Complex_I,
   -1.227476709426 +   0.905279873773*_Complex_I,
   -0.940584818722 +  -0.463678927507*_Complex_I,
   -0.327372793730 +   0.441537277752*_Complex_I,
    0.086812488375 +   1.122322318862*_Complex_I,
   -0.100645730164 +  -0.142102548176*_Complex_I,
   -0.339607323410 +  -0.632982556339*_Complex_I,
   -1.122342071583 +  -0.361529677196*_Complex_I,
   -0.394764118785 +   1.317868322379*_Complex_I,
    0.885025645922 +   0.277662260330*_Complex_I,
   -0.937809434341 +  -0.464711897028*_Complex_I,
   -0.316998490158 +   0.688777215432*_Complex_I,
    0.649526607014 +   0.181558179235*_Complex_I,
    0.780299241568 +  -0.070192359053*_Complex_I,
    1.932600474030 +  -0.136400805574*_Complex_I,
   -0.216121187682 +  -0.461249755715*_Complex_I,
    0.926223942818 +  -0.028529590209*_Complex_I,
    1.602135289481 +   1.434596371731*_Complex_I,
    1.623836928491 +  -1.265873017618*_Complex_I,
    1.568843067308 +  -0.364404259258*_Complex_I};

float complex fft_test_y20[] = {
    3.993406537683 +   1.703197921701*_Complex_I,
    3.008874212687 +   7.663142996950*_Complex_I,
   -1.159042365700 +   3.282695683450*_Complex_I,
    0.903124379379 +   1.428002755716*_Complex_I,
   -1.572075523393 +   4.283883169775*_Complex_I,
    2.253301421641 +   2.928788716985*_Complex_I,
    3.534976344610 +   2.383761115026*_Complex_I,
    1.326813143543 +   3.111834244207*_Complex_I,
   -3.988701280111 +  -0.677308408606*_Complex_I,
   -1.984004176890 +  -4.401380983625*_Complex_I,
    0.942714014611 +  -2.993550877540*_Complex_I,
    2.448617718798 +  -1.195359461640*_Complex_I,
   -3.328697057289 +  -4.221639306000*_Complex_I,
    4.287827550431 +   1.139111429335*_Complex_I,
   -4.013398939947 +  -0.677595378939*_Complex_I,
   -2.670924173537 +   7.635447143438*_Complex_I,
   -2.067686784436 +  -7.802196902076*_Complex_I,
   -1.972088145081 +  -6.957222482710*_Complex_I,
   -3.101333982661 +  -2.675851028288*_Complex_I,
    0.394807719219 +  -9.452730429599*_Complex_I};

