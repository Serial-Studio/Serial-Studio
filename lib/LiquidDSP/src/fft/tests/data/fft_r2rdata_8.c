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
// autotest fft r2r (real-to-real) data
//

// 8-point real even/odd dft data
float fftdata_r2r_x8[] = {
      0.0000000000,
      1.0000000000,
      2.0000000000,
      3.0000000000,
      4.0000000000,
      5.0000000000,
      6.0000000000,
      7.0000000000};

// REDFT00
float fftdata_r2r_REDFT00_y8[] = {
     49.0000000000,
    -20.1956693581,
     -0.0000000000,
     -2.5724165284,
     -0.0000000000,
     -1.2319141135,
     -0.0000000000,
     -1.0000000000};

// REDFT10
float fftdata_r2r_REDFT10_y8[] = {
     56.0000000000,
    -25.7692920908,
      0.0000000000,
     -2.6938192036,
      0.0000000000,
     -0.8036116149,
      0.0000000000,
     -0.2028092910};

// REDFT01
float fftdata_r2r_REDFT01_y8[] = {
     29.1819286410,
    -32.3061136840,
     12.7168729872,
    -10.9904036256,
      5.7286734878,
     -4.9189401648,
      1.8807638636,
     -1.2927815051};

// REDFT11
float fftdata_r2r_REDFT11_y8[] = {
     24.7243981823,
    -31.5148535947,
     13.9257769120,
    -12.7826885392,
      9.1714938314,
     -8.8071984223,
      7.6789810021,
     -7.5857732734};

// RODFT00
float fftdata_r2r_RODFT00_y8[] = {
     39.6989727373,
    -24.7272967751,
     12.1243556530,
    -10.7257823333,
      5.8736974182,
     -5.1961524227,
      2.5477916399,
     -1.5869428264};

// RODFT10
float fftdata_r2r_RODFT10_y8[] = {
     35.8808162684,
    -20.9050074380,
     12.5996671239,
    -11.3137084990,
      8.4188284171,
     -8.6591376023,
      7.1371381075,
     -8.0000000000};

// RODFT01
float fftdata_r2r_RODFT01_y8[] = {
     41.8902640723,
     -9.2302062214,
      0.3792058953,
     -2.4608789465,
      0.0160780480,
     -1.1773622132,
      0.2426629216,
     -0.6033416816};

// RODFT11
float fftdata_r2r_RODFT11_y8[] = {
     46.6916824794,
     -7.4005942194,
      0.9237106918,
     -1.7485238108,
     -0.1159888643,
     -0.8699819349,
     -0.3640003929,
     -0.5519032668};

