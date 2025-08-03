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
// fft_radix2_benchmark.c : benchmark FFTs of length 2^m
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include "liquid.h"

#include "src/fft/bench/fft_runbench.h"

// power-of-two transforms
void benchmark_fft_2      LIQUID_FFT_BENCHMARK_API(2,       LIQUID_FFT_FORWARD)
void benchmark_fft_4      LIQUID_FFT_BENCHMARK_API(4,       LIQUID_FFT_FORWARD)
void benchmark_fft_8      LIQUID_FFT_BENCHMARK_API(8,       LIQUID_FFT_FORWARD)
void benchmark_fft_16     LIQUID_FFT_BENCHMARK_API(16,      LIQUID_FFT_FORWARD)
void benchmark_fft_32     LIQUID_FFT_BENCHMARK_API(32,      LIQUID_FFT_FORWARD)
void benchmark_fft_64     LIQUID_FFT_BENCHMARK_API(64,      LIQUID_FFT_FORWARD)
void benchmark_fft_128    LIQUID_FFT_BENCHMARK_API(128,     LIQUID_FFT_FORWARD)
void benchmark_fft_256    LIQUID_FFT_BENCHMARK_API(256,     LIQUID_FFT_FORWARD)
void benchmark_fft_512    LIQUID_FFT_BENCHMARK_API(512,     LIQUID_FFT_FORWARD)
void benchmark_fft_1024   LIQUID_FFT_BENCHMARK_API(1024,    LIQUID_FFT_FORWARD)
void benchmark_fft_2048   LIQUID_FFT_BENCHMARK_API(2048,    LIQUID_FFT_FORWARD)
void benchmark_fft_4096   LIQUID_FFT_BENCHMARK_API(4096,    LIQUID_FFT_FORWARD)
void benchmark_fft_8192   LIQUID_FFT_BENCHMARK_API(8192,    LIQUID_FFT_FORWARD)
void benchmark_fft_16384  LIQUID_FFT_BENCHMARK_API(16384,   LIQUID_FFT_FORWARD)
void benchmark_fft_32768  LIQUID_FFT_BENCHMARK_API(32768,   LIQUID_FFT_FORWARD)

