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
// autotest fft data for 36-point transform
//

#include <complex.h>

float complex fft_test_x36[] = {
   -0.515260084522 +   1.287529717076*_Complex_I,
    0.587117158481 +  -0.148572876243*_Complex_I,
   -0.782947562463 +   1.641852621518*_Complex_I,
   -0.526927266439 +   0.584647073200*_Complex_I,
   -1.531349139822 +  -1.069623628897*_Complex_I,
    1.018192400002 +  -0.793147230271*_Complex_I,
   -1.074504882712 +  -0.091149144200*_Complex_I,
   -0.401657438434 +   0.004317962559*_Complex_I,
    1.259620132258 +  -0.104377362194*_Complex_I,
   -0.102194404981 +  -0.973411762169*_Complex_I,
   -0.895434561911 +  -1.050093649832*_Complex_I,
    1.026577410282 +   1.080478300358*_Complex_I,
    0.691470386469 +  -0.436083438367*_Complex_I,
   -0.230801075525 +   1.635643257519*_Complex_I,
   -0.121589188789 +   0.070584124184*_Complex_I,
    0.332245123083 +   0.603463018191*_Complex_I,
    0.098342650657 +   0.713292188052*_Complex_I,
   -0.653525164870 +   0.226711651388*_Complex_I,
   -0.486468735347 +   0.247902880597*_Complex_I,
   -0.092383856045 +   0.443156295025*_Complex_I,
   -1.207432550343 +  -0.503252568552*_Complex_I,
    0.680629730441 +   0.210743200774*_Complex_I,
   -1.655855919610 +   0.985314552623*_Complex_I,
   -0.635578415199 +   1.126805769525*_Complex_I,
    0.429626094959 +  -0.591166406548*_Complex_I,
    0.843172830682 +  -0.010918445865*_Complex_I,
   -0.130407009132 +   0.920489918178*_Complex_I,
    0.879595565427 +   0.076478701355*_Complex_I,
    0.407356105015 +   0.244802856694*_Complex_I,
   -0.247029167369 +   1.416537545584*_Complex_I,
    0.001447109646 +  -0.855321598060*_Complex_I,
   -1.005160244372 +   0.880697474487*_Complex_I,
   -1.506889600501 +  -0.376866773437*_Complex_I,
    0.595335431863 +   0.874899791600*_Complex_I,
   -1.720946712489 +  -0.251050064927*_Complex_I,
   -0.304668621370 +  -0.437688673914*_Complex_I};

float complex fft_test_y36[] = {
   -6.978283472983 +   7.583625277009*_Complex_I,
   -6.674217696450 +  -3.304059977181*_Complex_I,
   -9.133615960691 +   4.553754806959*_Complex_I,
    7.138280833022 +  -0.461421293427*_Complex_I,
    6.376807409683 +  -0.306551799085*_Complex_I,
    2.594166782732 +   4.623935739453*_Complex_I,
    2.991096252215 +  -1.709441742997*_Complex_I,
    7.042547687256 +  -5.210100129931*_Complex_I,
   -3.291195373643 +  -2.837550873581*_Complex_I,
    3.068320668506 +  -5.673130538045*_Complex_I,
    1.959771299385 +  -2.235257672510*_Complex_I,
  -10.609066858652 +   0.928096828820*_Complex_I,
    3.208839022246 +   0.484194748895*_Complex_I,
    2.548602732505 +   9.743720342785*_Complex_I,
    6.240917395617 +   6.571681544646*_Complex_I,
   -4.101181674885 +   4.039956075121*_Complex_I,
   -9.685452992487 +   0.380605286598*_Complex_I,
   -3.555255829985 +   3.488301753752*_Complex_I,
  -10.504163464296 +  -6.018056829194*_Complex_I,
   -2.111740241301 +   1.705549685664*_Complex_I,
    2.140088966360 +   7.541207884663*_Complex_I,
    0.985036473018 +  -0.558056384811*_Complex_I,
   -1.305685512500 +   8.210384283174*_Complex_I,
    4.783233947768 +  -2.666144508286*_Complex_I,
    6.484426654393 +  -5.252223925558*_Complex_I,
   -2.313461997972 +   5.743487698749*_Complex_I,
   -1.086778704978 +   8.029162089587*_Complex_I,
    6.916062245443 +   0.764580425536*_Complex_I,
   -2.415195588075 +  -0.073706064002*_Complex_I,
   -5.346986333451 +  -3.512605491284*_Complex_I,
   -0.924055660625 +   2.282174534826*_Complex_I,
   -0.470350054245 +   2.031728408857*_Complex_I,
    2.141016439694 +  -1.773104703203*_Complex_I,
   -1.014341113192 +   7.641158118573*_Complex_I,
   -4.249655466965 +   2.206889911886*_Complex_I,
    0.602106144731 +  -0.611713697716*_Complex_I};

