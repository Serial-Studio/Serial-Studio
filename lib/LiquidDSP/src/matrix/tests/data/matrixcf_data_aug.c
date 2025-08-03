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

// matrixcf_data_aug_x [size: 5 x 4]
float complex matrixcf_data_aug_x[] = {
   -1.383545994759 +   0.803655147552*_Complex_I /* ( 0, 0) */,
   -0.918114125729 +  -1.194809913635*_Complex_I /* ( 0, 1) */,
    0.090901032090 +   0.484884619713*_Complex_I /* ( 0, 2) */,
    0.109402157366 +   1.450437188148*_Complex_I /* ( 0, 3) */,
   -2.269510746002 +  -0.606436431408*_Complex_I /* ( 1, 0) */,
   -0.195189133286 +   0.416639328003*_Complex_I /* ( 1, 1) */,
    1.940145850182 +   0.895506143570*_Complex_I /* ( 1, 2) */,
   -0.784153759480 +  -0.345893263817*_Complex_I /* ( 1, 3) */,
    0.652509629726 +   0.994532823563*_Complex_I /* ( 2, 0) */,
   -2.253150939941 +   0.327611356974*_Complex_I /* ( 2, 1) */,
    1.012208938599 +  -0.677044689655*_Complex_I /* ( 2, 2) */,
   -0.700399398804 +  -0.330108255148*_Complex_I /* ( 2, 3) */,
   -1.175772666931 +   0.248428389430*_Complex_I /* ( 3, 0) */,
    0.412228941917 +  -2.519471645355*_Complex_I /* ( 3, 1) */,
   -1.667356371880 +  -1.187105178833*_Complex_I /* ( 3, 2) */,
    1.243350982666 +  -0.736937880516*_Complex_I /* ( 3, 3) */,
    0.033468723297 +   0.131351217628*_Complex_I /* ( 4, 0) */,
   -0.617851972580 +   1.434038400650*_Complex_I /* ( 4, 1) */,
   -1.009798288345 +   0.758803665638*_Complex_I /* ( 4, 2) */,
    1.450994849205 +  -0.595933079720*_Complex_I /* ( 4, 3) */};

// matrixcf_data_aug_y [size: 5 x 3]
float complex matrixcf_data_aug_y[] = {
    0.301848381758 +   0.353115469217*_Complex_I /* ( 0, 0) */,
    0.703616917133 +   0.044240720570*_Complex_I /* ( 0, 1) */,
    0.268176555634 +   1.071476221085*_Complex_I /* ( 0, 2) */,
   -0.717849135399 +  -0.764326214790*_Complex_I /* ( 1, 0) */,
   -0.108926303685 +  -0.315297245979*_Complex_I /* ( 1, 1) */,
    0.357895255089 +  -1.419853448868*_Complex_I /* ( 1, 2) */,
   -0.831380963326 +   1.003911018372*_Complex_I /* ( 2, 0) */,
   -0.361211270094 +  -0.926369905472*_Complex_I /* ( 2, 1) */,
    2.307183980942 +  -0.432167291641*_Complex_I /* ( 2, 2) */,
   -0.694230437279 +  -1.021739125252*_Complex_I /* ( 3, 0) */,
    0.412434548140 +  -1.840429663658*_Complex_I /* ( 3, 1) */,
    0.342358648777 +  -1.084336757660*_Complex_I /* ( 3, 2) */,
   -0.314995974302 +  -0.811702668667*_Complex_I /* ( 4, 0) */,
    0.912520587444 +  -2.686280250549*_Complex_I /* ( 4, 1) */,
    0.204153224826 +  -0.616621196270*_Complex_I /* ( 4, 2) */};

// matrixcf_data_aug_z [size: 5 x 7]
float complex matrixcf_data_aug_z[] = {
   -1.383545994759 +   0.803655147552*_Complex_I /* ( 0, 0) */,
   -0.918114125729 +  -1.194809913635*_Complex_I /* ( 0, 1) */,
    0.090901032090 +   0.484884619713*_Complex_I /* ( 0, 2) */,
    0.109402157366 +   1.450437188148*_Complex_I /* ( 0, 3) */,
    0.301848381758 +   0.353115469217*_Complex_I /* ( 0, 4) */,
    0.703616917133 +   0.044240720570*_Complex_I /* ( 0, 5) */,
    0.268176555634 +   1.071476221085*_Complex_I /* ( 0, 6) */,
   -2.269510746002 +  -0.606436431408*_Complex_I /* ( 1, 0) */,
   -0.195189133286 +   0.416639328003*_Complex_I /* ( 1, 1) */,
    1.940145850182 +   0.895506143570*_Complex_I /* ( 1, 2) */,
   -0.784153759480 +  -0.345893263817*_Complex_I /* ( 1, 3) */,
   -0.717849135399 +  -0.764326214790*_Complex_I /* ( 1, 4) */,
   -0.108926303685 +  -0.315297245979*_Complex_I /* ( 1, 5) */,
    0.357895255089 +  -1.419853448868*_Complex_I /* ( 1, 6) */,
    0.652509629726 +   0.994532823563*_Complex_I /* ( 2, 0) */,
   -2.253150939941 +   0.327611356974*_Complex_I /* ( 2, 1) */,
    1.012208938599 +  -0.677044689655*_Complex_I /* ( 2, 2) */,
   -0.700399398804 +  -0.330108255148*_Complex_I /* ( 2, 3) */,
   -0.831380963326 +   1.003911018372*_Complex_I /* ( 2, 4) */,
   -0.361211270094 +  -0.926369905472*_Complex_I /* ( 2, 5) */,
    2.307183980942 +  -0.432167291641*_Complex_I /* ( 2, 6) */,
   -1.175772666931 +   0.248428389430*_Complex_I /* ( 3, 0) */,
    0.412228941917 +  -2.519471645355*_Complex_I /* ( 3, 1) */,
   -1.667356371880 +  -1.187105178833*_Complex_I /* ( 3, 2) */,
    1.243350982666 +  -0.736937880516*_Complex_I /* ( 3, 3) */,
   -0.694230437279 +  -1.021739125252*_Complex_I /* ( 3, 4) */,
    0.412434548140 +  -1.840429663658*_Complex_I /* ( 3, 5) */,
    0.342358648777 +  -1.084336757660*_Complex_I /* ( 3, 6) */,
    0.033468723297 +   0.131351217628*_Complex_I /* ( 4, 0) */,
   -0.617851972580 +   1.434038400650*_Complex_I /* ( 4, 1) */,
   -1.009798288345 +   0.758803665638*_Complex_I /* ( 4, 2) */,
    1.450994849205 +  -0.595933079720*_Complex_I /* ( 4, 3) */,
   -0.314995974302 +  -0.811702668667*_Complex_I /* ( 4, 4) */,
    0.912520587444 +  -2.686280250549*_Complex_I /* ( 4, 5) */,
    0.204153224826 +  -0.616621196270*_Complex_I /* ( 4, 6) */};

