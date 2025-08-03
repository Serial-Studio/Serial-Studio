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
// data for testing L/U decomposition
//

#include <complex.h>

// matrixcf_data_ludecomp_A [size: 8 x 8]
float complex matrixcf_data_ludecomp_A[] = {
    0.455808967352 +   0.239869371057*_Complex_I /* ( 0, 0) */,
    1.076113820076 +   0.303303003311*_Complex_I /* ( 0, 1) */,
   -1.174549579620 +  -1.593330740929*_Complex_I /* ( 0, 2) */,
    1.428434848785 +  -2.108702898026*_Complex_I /* ( 0, 3) */,
    1.944794058800 +   1.039716124535*_Complex_I /* ( 0, 4) */,
   -0.003220892511 +  -1.070197224617*_Complex_I /* ( 0, 5) */,
    2.282850980759 +   0.567153334618*_Complex_I /* ( 0, 6) */,
    0.677789986134 +  -0.110934779048*_Complex_I /* ( 0, 7) */,
   -0.541479706764 +   0.508462309837*_Complex_I /* ( 1, 0) */,
    0.659551382065 +   0.341979026794*_Complex_I /* ( 1, 1) */,
    0.422295093536 +  -0.748002707958*_Complex_I /* ( 1, 2) */,
    0.991572380066 +  -1.739566326141*_Complex_I /* ( 1, 3) */,
   -0.973251938820 +  -0.314995020628*_Complex_I /* ( 1, 4) */,
   -0.348222613335 +   1.216362476349*_Complex_I /* ( 1, 5) */,
   -0.444941103458 +  -0.435953140259*_Complex_I /* ( 1, 6) */,
    0.664277911186 +   0.398205667734*_Complex_I /* ( 1, 7) */,
    1.578197240829 +  -0.245545297861*_Complex_I /* ( 2, 0) */,
   -0.734657287598 +  -0.314642846584*_Complex_I /* ( 2, 1) */,
   -0.185400843620 +   0.411517560482*_Complex_I /* ( 2, 2) */,
   -0.141458645463 +   2.540255069733*_Complex_I /* ( 2, 3) */,
   -1.887707233429 +   1.261052608490*_Complex_I /* ( 2, 4) */,
    1.356706976891 +  -0.073478087783*_Complex_I /* ( 2, 5) */,
    0.382849365473 +   1.176013708115*_Complex_I /* ( 2, 6) */,
    0.088731415570 +  -0.000313452416*_Complex_I /* ( 2, 7) */,
    0.694614350796 +   0.107012517750*_Complex_I /* ( 3, 0) */,
   -0.541421890259 +  -1.525843501091*_Complex_I /* ( 3, 1) */,
    1.210077285767 +  -0.249905958772*_Complex_I /* ( 3, 2) */,
    0.051122765988 +   0.576834678650*_Complex_I /* ( 3, 3) */,
    2.360952138901 +  -0.439353585243*_Complex_I /* ( 3, 4) */,
    0.927220702171 +   0.293185442686*_Complex_I /* ( 3, 5) */,
   -0.235832184553 +  -0.484229415655*_Complex_I /* ( 3, 6) */,
   -1.589996099472 +   0.180045768619*_Complex_I /* ( 3, 7) */,
    1.345695137978 +  -0.080361045897*_Complex_I /* ( 4, 0) */,
   -0.245824366808 +  -1.841626405716*_Complex_I /* ( 4, 1) */,
    0.978698849678 +   1.369340777397*_Complex_I /* ( 4, 2) */,
   -1.106017351151 +  -1.615537166595*_Complex_I /* ( 4, 3) */,
    0.627505123615 +   1.024900913239*_Complex_I /* ( 4, 4) */,
    1.808397769928 +  -0.614134788513*_Complex_I /* ( 4, 5) */,
   -0.322292149067 +  -0.765307128429*_Complex_I /* ( 4, 6) */,
   -0.674273192883 +   0.044275555760*_Complex_I /* ( 4, 7) */,
   -2.861634492874 +   2.582857608795*_Complex_I /* ( 5, 0) */,
   -1.920535564423 +  -0.081001155078*_Complex_I /* ( 5, 1) */,
   -1.339942932129 +  -0.246527969837*_Complex_I /* ( 5, 2) */,
    0.540911912918 +   0.283990591764*_Complex_I /* ( 5, 3) */,
   -0.800716042519 +   0.764756917953*_Complex_I /* ( 5, 4) */,
    1.206449866295 +   0.518103539944*_Complex_I /* ( 5, 5) */,
   -0.377558648586 +   0.065486297011*_Complex_I /* ( 5, 6) */,
   -1.090067625046 +   0.741791069508*_Complex_I /* ( 5, 7) */,
   -1.424072742462 +   0.091005645692*_Complex_I /* ( 6, 0) */,
    0.340615779161 +   1.995890378952*_Complex_I /* ( 6, 1) */,
   -0.395366579294 +   0.685165762901*_Complex_I /* ( 6, 2) */,
    0.367168039083 +  -1.265154719353*_Complex_I /* ( 6, 3) */,
    0.716018438339 +   1.003421306610*_Complex_I /* ( 6, 4) */,
   -0.648339152336 +   2.441966056824*_Complex_I /* ( 6, 5) */,
    0.788251757622 +   1.254729628563*_Complex_I /* ( 6, 6) */,
   -0.776828289032 +  -0.615517139435*_Complex_I /* ( 6, 7) */,
    1.112848401070 +  -0.297139286995*_Complex_I /* ( 7, 0) */,
    0.366721868515 +   0.650049626827*_Complex_I /* ( 7, 1) */,
    0.072020366788 +  -0.518339037895*_Complex_I /* ( 7, 2) */,
    1.033115744591 +  -0.196805760264*_Complex_I /* ( 7, 3) */,
   -1.083071947098 +  -1.565491795540*_Complex_I /* ( 7, 4) */,
    1.409144878387 +   0.992799341679*_Complex_I /* ( 7, 5) */,
    0.387732833624 +  -1.445696353912*_Complex_I /* ( 7, 6) */,
   -0.528750956059 +  -1.205648779869*_Complex_I /* ( 7, 7) */};

