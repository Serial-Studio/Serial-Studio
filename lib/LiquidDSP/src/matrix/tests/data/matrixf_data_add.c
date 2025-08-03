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
// data for testing matrix addition
//

#include <complex.h>

// matrixf_data_add_x [size: 5 x 4]
float matrixf_data_add_x[] = {
   -2.153858900070 /* ( 0, 0) */,
    0.374406367540 /* ( 0, 1) */,
   -0.356648415327 /* ( 0, 2) */,
    0.673922061920 /* ( 0, 3) */,
   -1.686753273010 /* ( 1, 0) */,
   -0.382442235947 /* ( 1, 1) */,
    0.287308961153 /* ( 1, 2) */,
   -0.479356884956 /* ( 1, 3) */,
   -0.336519986391 /* ( 2, 0) */,
    0.173820436001 /* ( 2, 1) */,
   -1.243059277534 /* ( 2, 2) */,
   -1.508571028709 /* ( 2, 3) */,
    0.903724849224 /* ( 3, 0) */,
    0.490690946579 /* ( 3, 1) */,
    0.242906242609 /* ( 3, 2) */,
    0.192125678062 /* ( 3, 3) */,
    0.053418140858 /* ( 4, 0) */,
    0.389735013247 /* ( 4, 1) */,
    0.781731247902 /* ( 4, 2) */,
   -1.207890510559 /* ( 4, 3) */};

// matrixf_data_add_y [size: 5 x 4]
float matrixf_data_add_y[] = {
   -1.569433808327 /* ( 0, 0) */,
    0.182892739773 /* ( 0, 1) */,
    2.420134067535 /* ( 0, 2) */,
   -0.114732131362 /* ( 0, 3) */,
   -1.274159908295 /* ( 1, 0) */,
   -1.230959534645 /* ( 1, 1) */,
    0.574799835682 /* ( 1, 2) */,
   -0.756966531277 /* ( 1, 3) */,
    1.426752924919 /* ( 2, 0) */,
    1.018160581589 /* ( 2, 1) */,
   -0.099268406630 /* ( 2, 2) */,
    0.683501064777 /* ( 2, 3) */,
    0.145665585995 /* ( 3, 0) */,
    0.337123543024 /* ( 3, 1) */,
    0.754367768764 /* ( 3, 2) */,
    0.908503055573 /* ( 3, 3) */,
   -1.320610523224 /* ( 4, 0) */,
   -1.090982913971 /* ( 4, 1) */,
    0.494600951672 /* ( 4, 2) */,
    0.713486075401 /* ( 4, 3) */};

// matrixf_data_add_z [size: 5 x 4]
float matrixf_data_add_z[] = {
   -3.723292708397 /* ( 0, 0) */,
    0.557299107313 /* ( 0, 1) */,
    2.063485652208 /* ( 0, 2) */,
    0.559189930558 /* ( 0, 3) */,
   -2.960913181305 /* ( 1, 0) */,
   -1.613401770592 /* ( 1, 1) */,
    0.862108796835 /* ( 1, 2) */,
   -1.236323416233 /* ( 1, 3) */,
    1.090232938528 /* ( 2, 0) */,
    1.191981017590 /* ( 2, 1) */,
   -1.342327684164 /* ( 2, 2) */,
   -0.825069963932 /* ( 2, 3) */,
    1.049390435219 /* ( 3, 0) */,
    0.827814489603 /* ( 3, 1) */,
    0.997274011374 /* ( 3, 2) */,
    1.100628733635 /* ( 3, 3) */,
   -1.267192382365 /* ( 4, 0) */,
   -0.701247900724 /* ( 4, 1) */,
    1.276332199574 /* ( 4, 2) */,
   -0.494404435158 /* ( 4, 3) */};

