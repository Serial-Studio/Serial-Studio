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
// autotest fft data for 30-point transform
//

#include <complex.h>

float complex fft_test_x30[] = {
    1.383958311928 +   1.009063372834*_Complex_I,
    0.656943839727 +  -0.201492142788*_Complex_I,
   -1.263185461543 +  -0.762235271683*_Complex_I,
   -0.279286295800 +   0.737833309276*_Complex_I,
    0.061984731638 +   0.232640465778*_Complex_I,
    0.078974781535 +   0.151163978562*_Complex_I,
   -0.067264854607 +  -1.701011326832*_Complex_I,
   -2.367250035232 +  -0.675654408115*_Complex_I,
    0.901831068347 +  -2.291740807507*_Complex_I,
   -0.685180193104 +   0.640309251420*_Complex_I,
   -0.865870302577 +  -0.234993063029*_Complex_I,
    0.394411867502 +  -1.629554965527*_Complex_I,
   -0.163205564173 +   0.199656642882*_Complex_I,
    0.561129126184 +   2.875025531619*_Complex_I,
   -1.191762025326 +   0.544473782291*_Complex_I,
    0.808768695645 +   1.055445062240*_Complex_I,
   -0.145391531392 +  -0.095884441741*_Complex_I,
    1.006538762836 +  -0.724775764669*_Complex_I,
    2.116364610835 +  -2.412759658708*_Complex_I,
    0.210526780948 +   1.338479312378*_Complex_I,
    0.722177277886 +   1.447742766425*_Complex_I,
    0.148533332865 +   0.262381546261*_Complex_I,
   -1.337190861421 +   1.251733062365*_Complex_I,
   -0.346555962201 +  -0.012481387193*_Complex_I,
    1.400932749094 +  -0.360556660035*_Complex_I,
    0.479524006659 +  -0.777208683834*_Complex_I,
   -0.525976158853 +  -0.155146817266*_Complex_I,
   -1.237887495307 +   0.159614688084*_Complex_I,
    0.526429110861 +  -1.368405705104*_Complex_I,
   -0.688995272873 +   0.833548127008*_Complex_I};

float complex fft_test_y30[] = {
    0.294027040082 +  -0.664790204608*_Complex_I,
   -7.251086654153 +   1.930700457878*_Complex_I,
    4.192769230897 +   1.789725983362*_Complex_I,
   11.197367573831 +   3.254783235859*_Complex_I,
  -10.053118996304 +  -1.739286933243*_Complex_I,
   10.012405959748 +  -4.383419562391*_Complex_I,
   -1.293134786584 +   8.917417869363*_Complex_I,
    3.828158834339 +   7.240756714477*_Complex_I,
    2.213781017086 +  -3.491740260928*_Complex_I,
  -11.803330439118 +   1.771364969478*_Complex_I,
    9.272563287802 +   0.848929030951*_Complex_I,
   -0.497098653141 +  -0.996109950789*_Complex_I,
   -3.419412966566 +  -7.252273855897*_Complex_I,
   -3.013334143526 +  -1.690385014079*_Complex_I,
    8.340586193744 +   1.433397080247*_Complex_I,
    2.813635161314 +  -8.730057114052*_Complex_I,
    0.460908379865 +   1.518681481090*_Complex_I,
   -5.717851959043 +  -2.820174393000*_Complex_I,
    8.168927986272 +  11.157813415293*_Complex_I,
    2.879730739941 +   1.551007915618*_Complex_I,
    0.710609564242 +  -1.414210144075*_Complex_I,
    5.664928404839 +  12.624792285877*_Complex_I,
   -4.559435692794 +   5.128817112052*_Complex_I,
    3.020061485600 +   2.317084746039*_Complex_I,
    9.287256582183 +   1.097899941841*_Complex_I,
    4.921470505274 +  -5.250097784974*_Complex_I,
    5.775306053035 +   5.396588308077*_Complex_I,
   -8.507611683876 +   0.041180219148*_Complex_I,
    3.499272220645 +   8.240657702592*_Complex_I,
    1.080399112219 +  -7.557152066173*_Complex_I};

