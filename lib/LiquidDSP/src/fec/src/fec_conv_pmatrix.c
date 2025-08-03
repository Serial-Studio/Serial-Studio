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

#include "liquid.internal.h"

// 2/3-rate K=7 punctured convolutional code
int fec_conv27p23_matrix[4] = {
    1, 1,
    1, 0
};

// 3/4-rate K=7 punctured convolutional code
int fec_conv27p34_matrix[6] = {
    1, 1, 0,
    1, 0, 1
};

// 4/5-rate K=7 punctured convolutional code
int fec_conv27p45_matrix[8] = {
    1, 1, 1, 1,
    1, 0, 0, 0
};

// 5/6-rate K=7 punctured convolutional code
int fec_conv27p56_matrix[10] = {
    1, 1, 0, 1, 0,
    1, 0, 1, 0, 1
};

// 6/7-rate K=7 punctured convolutional code
int fec_conv27p67_matrix[12] = {
    1, 1, 1, 0, 1, 0,
    1, 0, 0, 1, 0, 1
};

// 7/8-rate K=7 punctured convolutional code
int fec_conv27p78_matrix[14] = {
    1, 1, 1, 1, 0, 1, 0,
    1, 0, 0, 0, 1, 0, 1
};




// 2/3-rate K=9 punctured convolutional code
int fec_conv29p23_matrix[4] = {
    1, 1,
    1, 0
};

// 3/4-rate K=9 punctured convolutional code
int fec_conv29p34_matrix[6] = {
    1, 1, 1,
    1, 0, 0
};

// 4/5-rate K=9 punctured convolutional code
int fec_conv29p45_matrix[8] = {
    1, 1, 0, 1,
    1, 0, 1, 0
};

// 5/6-rate K=9 punctured convolutional code
int fec_conv29p56_matrix[10] = {
    1, 0, 1, 1, 0,
    1, 1, 0, 0, 1
};

// 6/7-rate K=9 punctured convolutional code
int fec_conv29p67_matrix[12] = {
    1, 1, 0, 1, 1, 0,
    1, 0, 1, 0, 0, 1
};

// 7/8-rate K=9 punctured convolutional code
int fec_conv29p78_matrix[14] = {
    1, 1, 0, 1, 0, 1, 1,
    1, 0, 1, 0, 1, 0, 0
};

