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
// data for testing multiply/transpose
//

#include <complex.h>

// matrixf_data_transmul_x [size: 5 x 4]
float matrixf_data_transmul_x[] = {
    1.366575479507 /* ( 0, 0) */,
    1.982354640961 /* ( 0, 1) */,
    0.913504719734 /* ( 0, 2) */,
    0.671039104462 /* ( 0, 3) */,
    0.264611721039 /* ( 1, 0) */,
    0.995791137218 /* ( 1, 1) */,
   -1.983934879303 /* ( 1, 2) */,
   -0.375840932131 /* ( 1, 3) */,
    0.245937779546 /* ( 2, 0) */,
   -0.343152791262 /* ( 2, 1) */,
   -0.143777698278 /* ( 2, 2) */,
   -1.387172579765 /* ( 2, 3) */,
    0.781192481518 /* ( 3, 0) */,
    1.444583415985 /* ( 3, 1) */,
    0.043851174414 /* ( 3, 2) */,
    0.399744838476 /* ( 3, 3) */,
   -0.488068610430 /* ( 4, 0) */,
    0.573721885681 /* ( 4, 1) */,
   -1.381630182266 /* ( 4, 2) */,
   -0.176615595818 /* ( 4, 3) */};

// matrixf_data_transmul_xxT [size: 5 x 5]
float matrixf_data_transmul_xxT[] = {
    7.082042816423 /* ( 0, 0) */,
    0.271085233449 /* ( 0, 1) */,
   -1.406346640934 /* ( 0, 2) */,
    4.239537802168 /* ( 0, 3) */,
   -0.910304016309 /* ( 0, 4) */,
    0.271085233449 /* ( 1, 0) */,
    5.138873363454 /* ( 1, 1) */,
    0.529971336750 /* ( 1, 2) */,
    1.407977702482 /* ( 1, 3) */,
    3.249602173055 /* ( 1, 4) */,
   -1.406346640934 /* ( 2, 0) */,
    0.529971336750 /* ( 2, 1) */,
    2.123159022134 /* ( 2, 2) */,
   -0.864407986864 /* ( 2, 3) */,
    0.126735142361 /* ( 2, 4) */,
    4.239537802168 /* ( 3, 0) */,
    1.407977702482 /* ( 3, 1) */,
   -0.864407986864 /* ( 3, 2) */,
    2.858801800305 /* ( 3, 3) */,
    0.316326313589 /* ( 3, 4) */,
   -0.910304016309 /* ( 4, 0) */,
    3.249602173055 /* ( 4, 1) */,
    0.126735142361 /* ( 4, 2) */,
    0.316326313589 /* ( 4, 3) */,
    2.507462799831 /* ( 4, 4) */};

// matrixf_data_transmul_xxH [size: 5 x 5]
float matrixf_data_transmul_xxH[] = {
    7.082042816423 /* ( 0, 0) */,
    0.271085233449 /* ( 0, 1) */,
   -1.406346640934 /* ( 0, 2) */,
    4.239537802168 /* ( 0, 3) */,
   -0.910304016309 /* ( 0, 4) */,
    0.271085233449 /* ( 1, 0) */,
    5.138873363454 /* ( 1, 1) */,
    0.529971336750 /* ( 1, 2) */,
    1.407977702482 /* ( 1, 3) */,
    3.249602173055 /* ( 1, 4) */,
   -1.406346640934 /* ( 2, 0) */,
    0.529971336750 /* ( 2, 1) */,
    2.123159022134 /* ( 2, 2) */,
   -0.864407986864 /* ( 2, 3) */,
    0.126735142361 /* ( 2, 4) */,
    4.239537802168 /* ( 3, 0) */,
    1.407977702482 /* ( 3, 1) */,
   -0.864407986864 /* ( 3, 2) */,
    2.858801800305 /* ( 3, 3) */,
    0.316326313589 /* ( 3, 4) */,
   -0.910304016309 /* ( 4, 0) */,
    3.249602173055 /* ( 4, 1) */,
    0.126735142361 /* ( 4, 2) */,
    0.316326313589 /* ( 4, 3) */,
    2.507462799831 /* ( 4, 4) */};

// matrixf_data_transmul_xTx [size: 4 x 4]
float matrixf_data_transmul_xTx[] = {
    2.846505957177 /* ( 0, 0) */,
    3.736623075087 /* ( 0, 1) */,
    1.396626890644 /* ( 0, 2) */,
    0.874893716720 /* ( 0, 3) */,
    3.736623075087 /* ( 1, 0) */,
    7.455061797501 /* ( 1, 1) */,
   -0.844681524592 /* ( 1, 2) */,
    1.908127088099 /* ( 1, 3) */,
    1.396626890644 /* ( 2, 0) */,
   -0.844681524592 /* ( 2, 1) */,
    6.701985390860 /* ( 2, 2) */,
    1.819632522483 /* ( 2, 3) */,
    0.874893716720 /* ( 3, 0) */,
    1.908127088099 /* ( 3, 1) */,
    1.819632522483 /* ( 3, 2) */,
    2.706786656609 /* ( 3, 3) */};

// matrixf_data_transmul_xHx [size: 4 x 4]
float matrixf_data_transmul_xHx[] = {
    2.846505957177 /* ( 0, 0) */,
    3.736623075087 /* ( 0, 1) */,
    1.396626890644 /* ( 0, 2) */,
    0.874893716720 /* ( 0, 3) */,
    3.736623075087 /* ( 1, 0) */,
    7.455061797501 /* ( 1, 1) */,
   -0.844681524592 /* ( 1, 2) */,
    1.908127088099 /* ( 1, 3) */,
    1.396626890644 /* ( 2, 0) */,
   -0.844681524592 /* ( 2, 1) */,
    6.701985390860 /* ( 2, 2) */,
    1.819632522483 /* ( 2, 3) */,
    0.874893716720 /* ( 3, 0) */,
    1.908127088099 /* ( 3, 1) */,
    1.819632522483 /* ( 3, 2) */,
    2.706786656609 /* ( 3, 3) */};

