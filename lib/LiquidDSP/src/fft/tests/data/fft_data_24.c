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
// autotest fft data for 24-point transform
//

#include <complex.h>

float complex fft_test_x24[] = {
   -1.420307293620 +   0.936847943063*_Complex_I,
   -0.496969771953 +   0.590714565032*_Complex_I,
   -1.354677361467 +  -2.010032405560*_Complex_I,
   -0.345483299359 +   0.404926987051*_Complex_I,
    0.689486222452 +  -0.768259750193*_Complex_I,
   -1.549641684121 +  -0.858118594125*_Complex_I,
    0.420175234777 +  -0.280408837681*_Complex_I,
   -2.217262854791 +  -0.375004631542*_Complex_I,
   -1.198653374297 +   0.900261122326*_Complex_I,
    0.329725819477 +   0.014746627154*_Complex_I,
    0.045545673584 +  -0.712071131515*_Complex_I,
   -0.495265874931 +  -0.922857927267*_Complex_I,
    0.628733326629 +   0.643883091672*_Complex_I,
    0.331696680042 +  -1.105791987558*_Complex_I,
    0.515399107713 +  -0.438177945625*_Complex_I,
    0.183533661425 +  -0.031372217136*_Complex_I,
   -0.276753781687 +   0.187288341292*_Complex_I,
    0.177067067896 +  -0.017072068523*_Complex_I,
   -0.269294774259 +   0.299567125253*_Complex_I,
    0.691280106543 +  -0.285429769347*_Complex_I,
   -0.478959763653 +   0.592471601549*_Complex_I,
    0.731469600041 +  -1.116061252131*_Complex_I,
    0.506952261982 +  -1.310440154883*_Complex_I,
   -0.131823537621 +   1.440176198054*_Complex_I};

float complex fft_test_y24[] = {
   -4.984028609200 +  -4.220215070640*_Complex_I,
   -4.250507506099 +   6.724208699822*_Complex_I,
    0.057360770043 +   0.512716211712*_Complex_I,
   -3.908279516708 +   6.693724296623*_Complex_I,
   -1.890450979255 +   6.088953815148*_Complex_I,
   -1.373512942118 +   4.739496879303*_Complex_I,
   -4.642576156472 +   5.105686189601*_Complex_I,
   -2.584328440882 +   1.728098263528*_Complex_I,
    0.744531943959 +   0.135606499920*_Complex_I,
   -0.971959807211 +  -7.134164325953*_Complex_I,
    0.738417588646 +  -0.940988007706*_Complex_I,
   -0.528146358843 +  -0.272559882858*_Complex_I,
    0.599319565507 +   0.302073070033*_Complex_I,
   -3.856916042162 + -10.635041786616*_Complex_I,
   -5.293262572847 +  -3.982011344991*_Complex_I,
   -4.348687900619 +   2.460786763911*_Complex_I,
    5.015153490569 +   6.700996972453*_Complex_I,
    0.389766514080 +  -0.278393532462*_Complex_I,
    0.801466543459 +   8.782425209839*_Complex_I,
    3.623555226700 +  -0.026142755609*_Complex_I,
   -3.328686450422 +   0.591920646925*_Complex_I,
   -5.710969715584 +   4.204863120029*_Complex_I,
    2.683867262111 +  -0.108391775481*_Complex_I,
   -1.068500953539 +  -4.689297523031*_Complex_I};

