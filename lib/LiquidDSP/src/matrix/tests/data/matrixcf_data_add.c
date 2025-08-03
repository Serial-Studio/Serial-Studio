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

// matrixcf_data_add_x [size: 5 x 4]
float complex matrixcf_data_add_x[] = {
    1.366575479507 +  -1.463535666466*_Complex_I /* ( 0, 0) */,
    1.982354640961 +   0.090445250273*_Complex_I /* ( 0, 1) */,
    0.913504719734 +  -0.689249753952*_Complex_I /* ( 0, 2) */,
    0.671039104462 +  -0.184951126575*_Complex_I /* ( 0, 3) */,
    0.264611721039 +   0.204716682434*_Complex_I /* ( 1, 0) */,
    0.995791137218 +  -0.192152589560*_Complex_I /* ( 1, 1) */,
   -1.983934879303 +  -0.306251227856*_Complex_I /* ( 1, 2) */,
   -0.375840932131 +  -0.751154422760*_Complex_I /* ( 1, 3) */,
    0.245937779546 +  -0.470900952816*_Complex_I /* ( 2, 0) */,
   -0.343152791262 +   1.205224752426*_Complex_I /* ( 2, 1) */,
   -0.143777698278 +  -0.457689523697*_Complex_I /* ( 2, 2) */,
   -1.387172579765 +   0.225333094597*_Complex_I /* ( 2, 3) */,
    0.781192481518 +  -1.102205991745*_Complex_I /* ( 3, 0) */,
    1.444583415985 +   0.660028517246*_Complex_I /* ( 3, 1) */,
    0.043851174414 +   0.049496211112*_Complex_I /* ( 3, 2) */,
    0.399744838476 +  -1.435891628265*_Complex_I /* ( 3, 3) */,
   -0.488068610430 +  -0.211131110787*_Complex_I /* ( 4, 0) */,
    0.573721885681 +   0.016210200265*_Complex_I /* ( 4, 1) */,
   -1.381630182266 +  -0.026558181271*_Complex_I /* ( 4, 2) */,
   -0.176615595818 +  -0.414863616228*_Complex_I /* ( 4, 3) */};

// matrixcf_data_add_y [size: 5 x 4]
float complex matrixcf_data_add_y[] = {
   -0.345586329699 +   0.240964725614*_Complex_I /* ( 0, 0) */,
   -0.025732314214 +   1.212726473808*_Complex_I /* ( 0, 1) */,
    0.907316803932 +   1.614625453949*_Complex_I /* ( 0, 2) */,
   -1.277831077576 +  -0.193561494350*_Complex_I /* ( 0, 3) */,
   -0.566267848015 +  -0.477513909340*_Complex_I /* ( 1, 0) */,
   -0.220387980342 +   0.152425482869*_Complex_I /* ( 1, 1) */,
    1.062286615372 +  -1.132043361664*_Complex_I /* ( 1, 2) */,
    0.560977220535 +  -0.981467425823*_Complex_I /* ( 1, 3) */,
    0.025816366076 +  -0.697246491909*_Complex_I /* ( 2, 0) */,
    0.658244788647 +  -1.403432965279*_Complex_I /* ( 2, 1) */,
   -1.326129436493 +  -0.529069602489*_Complex_I /* ( 2, 2) */,
   -1.535833358765 +  -1.406206607819*_Complex_I /* ( 2, 3) */,
    0.101660177112 +  -1.552777647972*_Complex_I /* ( 3, 0) */,
    1.834807038307 +   0.647780478001*_Complex_I /* ( 3, 1) */,
   -0.620168924332 +  -0.103246472776*_Complex_I /* ( 3, 2) */,
   -0.063054643571 +  -0.756766498089*_Complex_I /* ( 3, 3) */,
    0.280569136143 +   0.560459613800*_Complex_I /* ( 4, 0) */,
   -0.069713011384 +  -0.971132814884*_Complex_I /* ( 4, 1) */,
    0.225165605545 +  -1.117488980293*_Complex_I /* ( 4, 2) */,
   -0.290932357311 +   0.302335798740*_Complex_I /* ( 4, 3) */};

// matrixcf_data_add_z [size: 5 x 4]
float complex matrixcf_data_add_z[] = {
    1.020989149809 +  -1.222570940852*_Complex_I /* ( 0, 0) */,
    1.956622326747 +   1.303171724081*_Complex_I /* ( 0, 1) */,
    1.820821523666 +   0.925375699997*_Complex_I /* ( 0, 2) */,
   -0.606791973114 +  -0.378512620926*_Complex_I /* ( 0, 3) */,
   -0.301656126976 +  -0.272797226906*_Complex_I /* ( 1, 0) */,
    0.775403156877 +  -0.039727106690*_Complex_I /* ( 1, 1) */,
   -0.921648263931 +  -1.438294589520*_Complex_I /* ( 1, 2) */,
    0.185136288404 +  -1.732621848583*_Complex_I /* ( 1, 3) */,
    0.271754145622 +  -1.168147444725*_Complex_I /* ( 2, 0) */,
    0.315091997385 +  -0.198208212852*_Complex_I /* ( 2, 1) */,
   -1.469907134771 +  -0.986759126186*_Complex_I /* ( 2, 2) */,
   -2.923005938530 +  -1.180873513222*_Complex_I /* ( 2, 3) */,
    0.882852658629 +  -2.654983639717*_Complex_I /* ( 3, 0) */,
    3.279390454292 +   1.307808995247*_Complex_I /* ( 3, 1) */,
   -0.576317749918 +  -0.053750261664*_Complex_I /* ( 3, 2) */,
    0.336690194905 +  -2.192658126354*_Complex_I /* ( 3, 3) */,
   -0.207499474287 +   0.349328503013*_Complex_I /* ( 4, 0) */,
    0.504008874297 +  -0.954922614619*_Complex_I /* ( 4, 1) */,
   -1.156464576721 +  -1.144047161564*_Complex_I /* ( 4, 2) */,
   -0.467547953129 +  -0.112527817488*_Complex_I /* ( 4, 3) */};

