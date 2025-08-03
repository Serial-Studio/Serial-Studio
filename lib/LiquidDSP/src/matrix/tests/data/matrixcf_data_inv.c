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
// data for testing matrix inversion
//

#include <complex.h>

// matrixcf_data_inv_x [size: 5 x 5]
float complex matrixcf_data_inv_x[] = {
   -0.911099433899 +  -0.436777323484*_Complex_I /* ( 0, 0) */,
    0.598295390606 +  -0.283340752125*_Complex_I /* ( 0, 1) */,
   -0.264758616686 +  -0.421906232834*_Complex_I /* ( 0, 2) */,
   -0.066837862134 +  -0.934806823730*_Complex_I /* ( 0, 3) */,
    0.393610686064 +  -1.011345505714*_Complex_I /* ( 0, 4) */,
   -0.543692529202 +  -1.426580429077*_Complex_I /* ( 1, 0) */,
   -1.006833553314 +   0.448534607887*_Complex_I /* ( 1, 1) */,
    0.048818156123 +  -0.540948212147*_Complex_I /* ( 1, 2) */,
    0.180871278048 +   0.331172674894*_Complex_I /* ( 1, 3) */,
   -1.100448012352 +   1.841731786728*_Complex_I /* ( 1, 4) */,
    2.341797351837 +  -1.200128436089*_Complex_I /* ( 2, 0) */,
    0.239693909883 +   0.206349417567*_Complex_I /* ( 2, 1) */,
   -0.815828502178 +  -0.349400132895*_Complex_I /* ( 2, 2) */,
    1.213637232780 +   0.298941820860*_Complex_I /* ( 2, 3) */,
   -1.522765398026 +   1.651986479759*_Complex_I /* ( 2, 4) */,
    1.481738448143 +   0.055169839412*_Complex_I /* ( 3, 0) */,
   -1.241538286209 +  -0.077680915594*_Complex_I /* ( 3, 1) */,
    1.046607017517 +  -0.843883395195*_Complex_I /* ( 3, 2) */,
   -1.564810752869 +   1.346152186394*_Complex_I /* ( 3, 3) */,
    0.786287426949 +  -1.010108113289*_Complex_I /* ( 3, 4) */,
    1.234361886978 +  -1.305809140205*_Complex_I /* ( 4, 0) */,
    0.053748749197 +   0.403882414103*_Complex_I /* ( 4, 1) */,
   -0.081336200237 +  -0.462558329105*_Complex_I /* ( 4, 2) */,
   -1.370563983917 +  -0.284755766392*_Complex_I /* ( 4, 3) */,
    0.200873896480 +  -0.036809749901*_Complex_I /* ( 4, 4) */};

// matrixcf_data_inv_y [size: 5 x 5]
float complex matrixcf_data_inv_y[] = {
   -0.127852678827 +  -0.009178191835*_Complex_I /* ( 0, 0) */,
   -0.199905444866 +   0.033789259175*_Complex_I /* ( 0, 1) */,
    0.168465876479 +  -0.059607902071*_Complex_I /* ( 0, 2) */,
    0.087700609092 +  -0.030597427908*_Complex_I /* ( 0, 3) */,
    0.084793376582 +   0.131223765916*_Complex_I /* ( 0, 4) */,
    0.209779356201 +   0.642123753363*_Complex_I /* ( 1, 0) */,
   -0.045651767577 +  -0.019599459364*_Complex_I /* ( 1, 1) */,
    0.137284052424 +   0.504637287094*_Complex_I /* ( 1, 2) */,
   -0.333643348460 +   0.455368743084*_Complex_I /* ( 1, 3) */,
    0.244939020151 +  -0.609710193351*_Complex_I /* ( 1, 4) */,
   -0.114524820581 +   0.963012925652*_Complex_I /* ( 2, 0) */,
    0.303499486096 +   0.348121666797*_Complex_I /* ( 2, 1) */,
   -0.327372880299 +   0.397314645420*_Complex_I /* ( 2, 2) */,
   -0.231096370464 +   0.372958732742*_Complex_I /* ( 2, 3) */,
    0.089363987094 +  -0.240520272187*_Complex_I /* ( 2, 4) */,
    0.072169240922 +   0.159456098576*_Complex_I /* ( 3, 0) */,
   -0.064066539188 +   0.069570707500*_Complex_I /* ( 3, 1) */,
    0.090335627717 +  -0.121329478735*_Complex_I /* ( 3, 2) */,
    0.053196220990 +  -0.158230982223*_Complex_I /* ( 3, 3) */,
   -0.413653285108 +   0.167815066469*_Complex_I /* ( 3, 4) */,
    0.089194647874 +  -0.035492413461*_Complex_I /* ( 4, 0) */,
   -0.192303472410 +  -0.221655891788*_Complex_I /* ( 4, 1) */,
    0.111730542618 +  -0.221903756183*_Complex_I /* ( 4, 2) */,
    0.303835472120 +  -0.022543572811*_Complex_I /* ( 4, 3) */,
   -0.167008031325 +   0.051911194273*_Complex_I /* ( 4, 4) */};

