/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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

#include "autotest/autotest.h"
#include "liquid.internal.h"

// compute emperical distributions and compare to theoretical

// add value to histogram
unsigned int _support_histogram_add(float        _value,
                                    float *      _bins,
                                    unsigned int _num_bins,
                                    float        _vmin,
                                    float        _vmax)
{
    float indexf = _num_bins * (_value - _vmin) / (_vmax - _vmin);
    unsigned int index = 0;
    if (indexf >= 0)
        index = (unsigned int)indexf;
    if (index >= _num_bins)
        index = _num_bins - 1;
    _bins[index]++;
    return index;
}

// noramlize histogram (area under curve)
float _support_histogram_normalize(float *      _bins,
                                   unsigned int _num_bins,
                                   float        _vmin,
                                   float        _vmax)
{
    float area = 0.0f;
    unsigned int i;
    for (i=0; i<_num_bins; i++)
        area += _bins[i];
    area *= (_vmax - _vmin) / (float)_num_bins;
    for (i=0; i<_num_bins; i++)
        _bins[i] /= area;
    return area;
}

// log histogram
void _support_histogram_log(float *      _bins,
                            unsigned int _num_bins,
                            float        _vmin,
                            float        _vmax)
{
    // find max(hist)
    unsigned int i;
    float hist_max = 0;
    for (i=0; i<_num_bins; i++)
        hist_max = _bins[i] > hist_max ? _bins[i] : hist_max;

    unsigned int num_chars = 72;
    float _vstep = (_vmax - _vmin) / (float)(_num_bins-1);
    printf("%8s : [%12s]\n", "v", "bin value");
    for (i=0; i<_num_bins; i++) {
        printf("%8.2f : [%12g]", _vmin + i*_vstep, _bins[i]);

        unsigned int k;
        unsigned int n = round(num_chars * _bins[i] / hist_max);
        for (k=0; k<n; k++)
            printf("-");
        printf("+");
        printf("\n");
    }
}

// validate pdf and cdf
void _support_histogram_validate(float *      _bins,
                                 float *      _pdf,
                                 float *      _cdf,
                                 unsigned int _num_bins,
                                 float        _vmin,
                                 float        _vmax,
                                 float        _tol)
{
    // normalize histogram
    _support_histogram_normalize(_bins, _num_bins, _vmin, _vmax);

    // compare pdf and cdf
    unsigned int i;
    for (i=0; i<_num_bins; i++) {
        CONTEND_DELTA(_bins[i], _pdf[i], _tol);
    }

    // accumulate and compare cdf
    float _vstep = (_vmax - _vmin) / (float)(_num_bins);
    float accum = 0.0f;
    for (i=1; i<_num_bins; i++) {
        // trapezoidal integration
        accum += 0.5f * (_bins[i-1] + _bins[i]) * _vstep;
        CONTEND_DELTA(accum, _cdf[i], _tol);
        //printf("[%2u] : %12.8f (expected %12.8f), e:%12.8f\n", i, accum, _cdf[i], accum-_cdf[i]);
    }
}

// normal distribution
void autotest_distribution_randnf()
{
    unsigned long int num_trials = 248000;
    float eta = 0.0f, sig = 1.0f;
    float tol = 0.05f;

    unsigned int num_bins = 31;
    float bins[num_bins];
    unsigned long int i;
    for (i=0; i<num_bins; i++)
        bins[i] = 0.0f;
    float vmin = -5.0f, vmax = +5.0f;

    // compute histogram
    for (i=0; i<num_trials; i++) {
        float v = randnf()*sig + eta;
        _support_histogram_add(v, bins, num_bins, vmin, vmax);
    }

    // compute ideal pdf and cdf
    float vstep = (vmax - vmin) / (float)(num_bins-1);
    float pdf[num_bins];
    float cdf[num_bins];
    for (i=0; i<num_bins; i++) {
        float v = vmin + i*vstep;
        pdf[i] = randnf_pdf(v, eta, sig);
        cdf[i] = randnf_cdf(v, eta, sig);
    }

    // log histogram
    //_support_histogram_log(bins, num_bins, vmin, vmax);
    //_support_histogram_log(pdf,  num_bins, vmin, vmax);
    //_support_histogram_log(cdf,  num_bins, vmin, vmax);

    // validate distributions
    _support_histogram_validate(bins, pdf, cdf, num_bins, vmin, vmax, tol);
}

// exponential distribution
void xautotest_distribution_randexpf()
{
    unsigned long int num_trials = 248000;
    float lambda = 1.3f;
    float tol = 0.05f;

    unsigned int num_bins = 21;
    float bins[num_bins];
    unsigned long int i;
    for (i=0; i<num_bins; i++)
        bins[i] = 0.0f;
    float vmin = -1.0f, vmax = +9.0f;

    // compute histogram
    for (i=0; i<num_trials; i++) {
        float v = randexpf(lambda);
        _support_histogram_add(v, bins, num_bins, vmin, vmax);
    }

    // compute ideal pdf and cdf
    float vstep = (vmax - vmin) / (float)(num_bins-1);
    float pdf[num_bins];
    float cdf[num_bins];
    for (i=0; i<num_bins; i++) {
        float v = vmin + i*vstep;
        pdf[i] = randexpf_pdf(v, lambda);
        cdf[i] = randexpf_cdf(v, lambda);
    }

    // log histogram
    _support_histogram_log(bins, num_bins, vmin, vmax);
    //_support_histogram_log(pdf,  num_bins, vmin, vmax);
    //_support_histogram_log(cdf,  num_bins, vmin, vmax);

    // validate distributions
    _support_histogram_validate(bins, pdf, cdf, num_bins, vmin, vmax, tol);
}

