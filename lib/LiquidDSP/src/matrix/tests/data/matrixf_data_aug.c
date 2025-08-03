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
// data for testing matrix augmentation
//

#include <complex.h>

// matrixf_data_aug_x [size: 5 x 4]
float matrixf_data_aug_x[] = {
   -0.747572302818 /* ( 0, 0) */,
    1.023007512093 /* ( 0, 1) */,
   -0.806419134140 /* ( 0, 2) */,
    1.476346969604 /* ( 0, 3) */,
   -0.456311076880 /* ( 1, 0) */,
    1.049571633339 /* ( 1, 1) */,
    0.041211493313 /* ( 1, 2) */,
    0.870350718498 /* ( 1, 3) */,
   -0.585918903351 /* ( 2, 0) */,
   -2.498867988586 /* ( 2, 1) */,
    1.247432827950 /* ( 2, 2) */,
   -1.840264678001 /* ( 2, 3) */,
    0.618996977806 /* ( 3, 0) */,
   -1.083691835403 /* ( 3, 1) */,
   -1.827050209045 /* ( 3, 2) */,
   -0.579039454460 /* ( 3, 3) */,
    1.507880568504 /* ( 4, 0) */,
    1.633087396622 /* ( 4, 1) */,
   -0.439950227737 /* ( 4, 2) */,
   -0.058893665671 /* ( 4, 3) */};

// matrixf_data_aug_y [size: 5 x 3]
float matrixf_data_aug_y[] = {
    0.376702636480 /* ( 0, 0) */,
    0.790158689022 /* ( 0, 1) */,
    2.111151933670 /* ( 0, 2) */,
   -0.690664231777 /* ( 1, 0) */,
   -0.598035037518 /* ( 1, 1) */,
   -0.137144193053 /* ( 1, 2) */,
    1.078616261482 /* ( 2, 0) */,
    0.907722294331 /* ( 2, 1) */,
   -0.432205766439 /* ( 2, 2) */,
   -1.615019798279 /* ( 3, 0) */,
    0.122782632709 /* ( 3, 1) */,
    1.174023866653 /* ( 3, 2) */,
    0.233828529716 /* ( 4, 0) */,
    0.032883912325 /* ( 4, 1) */,
   -0.896481394768 /* ( 4, 2) */};

// matrixf_data_aug_z [size: 5 x 7]
float matrixf_data_aug_z[] = {
   -0.747572302818 /* ( 0, 0) */,
    1.023007512093 /* ( 0, 1) */,
   -0.806419134140 /* ( 0, 2) */,
    1.476346969604 /* ( 0, 3) */,
    0.376702636480 /* ( 0, 4) */,
    0.790158689022 /* ( 0, 5) */,
    2.111151933670 /* ( 0, 6) */,
   -0.456311076880 /* ( 1, 0) */,
    1.049571633339 /* ( 1, 1) */,
    0.041211493313 /* ( 1, 2) */,
    0.870350718498 /* ( 1, 3) */,
   -0.690664231777 /* ( 1, 4) */,
   -0.598035037518 /* ( 1, 5) */,
   -0.137144193053 /* ( 1, 6) */,
   -0.585918903351 /* ( 2, 0) */,
   -2.498867988586 /* ( 2, 1) */,
    1.247432827950 /* ( 2, 2) */,
   -1.840264678001 /* ( 2, 3) */,
    1.078616261482 /* ( 2, 4) */,
    0.907722294331 /* ( 2, 5) */,
   -0.432205766439 /* ( 2, 6) */,
    0.618996977806 /* ( 3, 0) */,
   -1.083691835403 /* ( 3, 1) */,
   -1.827050209045 /* ( 3, 2) */,
   -0.579039454460 /* ( 3, 3) */,
   -1.615019798279 /* ( 3, 4) */,
    0.122782632709 /* ( 3, 5) */,
    1.174023866653 /* ( 3, 6) */,
    1.507880568504 /* ( 4, 0) */,
    1.633087396622 /* ( 4, 1) */,
   -0.439950227737 /* ( 4, 2) */,
   -0.058893665671 /* ( 4, 3) */,
    0.233828529716 /* ( 4, 4) */,
    0.032883912325 /* ( 4, 5) */,
   -0.896481394768 /* ( 4, 6) */};

