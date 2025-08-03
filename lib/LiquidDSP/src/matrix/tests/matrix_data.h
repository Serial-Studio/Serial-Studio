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
// autotest matrix data definitions
//

#ifndef __LIQUID_MATRIX_DATA_H__
#define __LIQUID_MATRIX_DATA_H__

// 
// single-precision real floating-point data
//

// add
extern float matrixf_data_add_x[];
extern float matrixf_data_add_y[];
extern float matrixf_data_add_z[];

// aug
extern float matrixf_data_aug_x[];
extern float matrixf_data_aug_y[];
extern float matrixf_data_aug_z[];

// cgsolve
extern float matrixf_data_cgsolve_A[];
extern float matrixf_data_cgsolve_x[];
extern float matrixf_data_cgsolve_b[];

// chol
extern float matrixf_data_chol_A[];
extern float matrixf_data_chol_L[];

// gramschmidt
extern float matrixf_data_gramschmidt_A[];
extern float matrixf_data_gramschmidt_V[];

// inv
extern float matrixf_data_inv_x[];
extern float matrixf_data_inv_y[];

// linsolve
extern float matrixf_data_linsolve_A[];
extern float matrixf_data_linsolve_x[];
extern float matrixf_data_linsolve_b[];

// ludecomp
extern float matrixf_data_ludecomp_A[];

// mul
extern float matrixf_data_mul_x[];
extern float matrixf_data_mul_y[];
extern float matrixf_data_mul_z[];

// qrdecomp
extern float matrixf_data_qrdecomp_A[];
extern float matrixf_data_qrdecomp_Q[];
extern float matrixf_data_qrdecomp_R[];


// transmul
extern float matrixf_data_transmul_x[];
extern float matrixf_data_transmul_xxT[];
extern float matrixf_data_transmul_xxH[];
extern float matrixf_data_transmul_xTx[];
extern float matrixf_data_transmul_xHx[];


// 
// single-precision complex floating-point data
//

// add
extern float complex matrixcf_data_add_x[];
extern float complex matrixcf_data_add_y[];
extern float complex matrixcf_data_add_z[];

// aug
extern float complex matrixcf_data_aug_x[];
extern float complex matrixcf_data_aug_y[];
extern float complex matrixcf_data_aug_z[];

// chol
extern float complex matrixcf_data_chol_A[];
extern float complex matrixcf_data_chol_L[];

// inv
extern float complex matrixcf_data_inv_x[];
extern float complex matrixcf_data_inv_y[];

// linsolve
extern float complex matrixcf_data_linsolve_A[];
extern float complex matrixcf_data_linsolve_x[];
extern float complex matrixcf_data_linsolve_b[];

// ludecomp
extern float complex matrixcf_data_ludecomp_A[];

// mul
extern float complex matrixcf_data_mul_x[];
extern float complex matrixcf_data_mul_y[];
extern float complex matrixcf_data_mul_z[];

// qrdecomp
extern float complex matrixcf_data_qrdecomp_A[];
extern float complex matrixcf_data_qrdecomp_Q[];
extern float complex matrixcf_data_qrdecomp_R[];

// transmul
extern float complex matrixcf_data_transmul_x[];
extern float complex matrixcf_data_transmul_xxT[];
extern float complex matrixcf_data_transmul_xxH[];
extern float complex matrixcf_data_transmul_xTx[];
extern float complex matrixcf_data_transmul_xHx[];

#endif // __LIQUID_MATRIX_DATA_H__

