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
// autotest fft data for 22-point transform
//

#include <complex.h>

float complex fft_test_x22[] = {
    1.005393906711 +   1.012308570546*_Complex_I,
    1.688071880070 +   1.150240841419*_Complex_I,
    0.683345289428 +  -0.062623351865*_Complex_I,
    0.155361503274 +  -0.795786088580*_Complex_I,
   -1.820284248422 +   1.218801819999*_Complex_I,
    0.628547514320 +  -0.973049954879*_Complex_I,
    1.065280228237 +   0.036573157841*_Complex_I,
   -2.060385212897 +   0.046192542979*_Complex_I,
   -1.013657698873 +   0.781474907362*_Complex_I,
    0.375646814842 +   1.554324770487*_Complex_I,
    0.891369658302 +  -0.161096019710*_Complex_I,
   -1.068796361214 +   0.437936260633*_Complex_I,
    2.551818984217 +  -0.855823653619*_Complex_I,
    1.902986665335 +   0.173617409071*_Complex_I,
    0.782297326663 +   1.157416587734*_Complex_I,
    1.408853663828 +  -0.360396487397*_Complex_I,
   -1.133068299545 +   0.494376224047*_Complex_I,
   -0.644483632405 +  -0.024946520734*_Complex_I,
   -1.906331676937 +   0.114276645380*_Complex_I,
   -0.607040125764 +   0.926527519920*_Complex_I,
    0.405414441208 +   0.272172965895*_Complex_I,
    0.522244149054 +   0.024648904750*_Complex_I};

float complex fft_test_y22[] = {
    3.812584769431 +   6.167167051279*_Complex_I,
   -2.048551145080 +   1.920025824226*_Complex_I,
    6.865239600362 +  -6.254294174522*_Complex_I,
    6.680188302485 +   7.175301698042*_Complex_I,
    0.336985685391 +  -2.472221525673*_Complex_I,
    2.704578846644 +  -1.072297553521*_Complex_I,
   -5.869735761695 +   0.406925054472*_Complex_I,
    7.260505946731 +   4.415111182242*_Complex_I,
    1.284910248555 +  -1.935024971881*_Complex_I,
    1.066939615916 +  -8.084240769565*_Complex_I,
   -5.103441669518 +   4.459170611366*_Complex_I,
   -0.789428947455 +   1.848548655941*_Complex_I,
   -2.925771543223 +  -0.333175332942*_Complex_I,
    8.231473045218 +  -1.322435498403*_Complex_I,
   -2.591866662060 +   5.216079561352*_Complex_I,
   -0.164183264077 +  -4.464934113704*_Complex_I,
  -10.152302601435 +   2.776060476179*_Complex_I,
   -1.546651996340 +   8.030006620400*_Complex_I,
    2.238990051464 +  -2.081025590453*_Complex_I,
    2.420796442681 +  -1.572310054104*_Complex_I,
   11.406980883199 +  10.003031983787*_Complex_I,
   -0.999573899550 +  -0.554680582506*_Complex_I};

