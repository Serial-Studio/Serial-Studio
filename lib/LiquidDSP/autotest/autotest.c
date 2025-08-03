/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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
// autotestlib.c
//

// default include headers
#include <stdio.h>
#include <stdlib.h>
#include "autotest/autotest.h"

// total number of checks invoked
unsigned long int liquid_autotest_num_checks=0;

// total number of checks which passed
unsigned long int liquid_autotest_num_passed=0;

// total number of checks which failed
unsigned long int liquid_autotest_num_failed=0;

// total number of warnings
unsigned long int liquid_autotest_num_warnings=0;

// verbosity flag
int liquid_autotest_verbose = 1;

// fail test
// increment liquid_autotest_num_checks
// increment liquid_autotest_num_failed
void liquid_autotest_failed()
{
    liquid_autotest_num_checks++;
    liquid_autotest_num_failed++;
}

// pass test
// increment liquid_autotest_num_checks
// increment liquid_autotest_num_passed
void liquid_autotest_passed()
{
    liquid_autotest_num_checks++;
    liquid_autotest_num_passed++;
}

// fail test, given expression
//  _file       :   filename (string)
//  _line       :   line number of test
//  _exprL      :   left side of expression (string)
//  _valueL     :   left side of expression (value)
//  _qualifier  :   expression qualifier
//  _exprR      :   right side of expression (string)
//  _valueR     :   right side of expression (value)
void liquid_autotest_failed_expr(const char * _file,
                                 unsigned int _line,
                                 const char * _exprL,
                                 double _valueL,
                                 const char * _qualifier,
                                 const char * _exprR,
                                 double _valueR)
{
    if (liquid_autotest_verbose) {
        printf("  TEST FAILED: %s line %u : expected %s (%0.2E) %s %s (%0.2E)\n",
                _file, _line, _exprL, _valueL, _qualifier, _exprR, _valueR);
    }
    liquid_autotest_failed();
}

// fail test, given true/false value
//  _file       :   filename (string)
//  _line       :   line number of test
//  _exprL      :   left side of expression (string)
//  _valueL     :   left side of expression (value)
//  _qualifier  :   expression qualifier
void liquid_autotest_failed_bool(const char * _file,
                                 unsigned int _line,
                                 const char * _exprL,
                                 double       _valueL,
                                 int          _qualifier)
{
    if (liquid_autotest_verbose) {
        printf("  TEST FAILED: %s line %u : expected \"%s\" (%g) is %s\n",
                _file, _line, _exprL, _valueL, _qualifier ? "true" : "false");
    }
    liquid_autotest_failed();
}

//  _file       :   filename (string)
//  _line       :   line number of test
//  _message    :   message string
void liquid_autotest_failed_msg(const char * _file,
                                unsigned int _line,
                                const char * _message)
{
    if (liquid_autotest_verbose)
        printf("  TEST FAILED: %s line %u : %s\n", _file, _line, _message);
    liquid_autotest_failed();
}

// print basic autotest results to stdout
void autotest_print_results(void)
{
    if (liquid_autotest_num_warnings > 0) {
        printf("==================================\n");
        printf(" WARNINGS : %-lu\n", liquid_autotest_num_warnings);
    }

    printf("==================================\n");
    if (liquid_autotest_num_checks==0) {
        printf(" NO CHECKS RUN\n");
    } else if (liquid_autotest_num_failed==0) {
        printf(" PASSED ALL %lu CHECKS\n", liquid_autotest_num_passed);
    } else {
        // compute and print percentage of failed tests
        double percent_failed = (double) liquid_autotest_num_failed /
                                (double) liquid_autotest_num_checks;
        printf(" FAILED %lu / %lu CHECKS (%7.2f%%)\n",
                liquid_autotest_num_failed,
                liquid_autotest_num_checks,
                100.0*percent_failed);
    }
    printf("==================================\n");
}

// print warning to stderr
// increment liquid_autotest_num_warnings
//  _file       :   filename (string)
//  _line       :   line number of test
//  _message    :   message string
void liquid_autotest_warn(const char * _file,
                          unsigned int _line,
                         const char * _message)
{
    if (liquid_autotest_verbose)
        fprintf(stderr,"  WARNING: %s line %u : %s\n", _file, _line, _message);

    liquid_autotest_num_warnings++;
}

// contend that data in two arrays are identical
//  _x      :   input array [size: _n x 1]
//  _y      :   input array [size: _n x 1]
//  _n      :   input array size
int liquid_autotest_same_data(unsigned char * _x,
                              unsigned char * _y,
                              unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        if (_x[i] != _y[i])
            return 0;
    }
    return 1;
}

// print array to standard out
//  _x      :   input array [size: _n x 1]
//  _n      :   input array size
void liquid_autotest_print_array(unsigned char * _x,
                                 unsigned int _n)
{
    unsigned int i;
    printf("   {");
    for (i=0; i<_n; i++) {
        printf("%.2x, ", (unsigned int)(_x[i]));
        if ( ((i+1)%16 == 0) && (i != (_n-1)) )
            printf("\n    ");
    }
    printf("}\n");
}

// validate spectral content
int liquid_autotest_validate_spectrum(float * _psd, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int _num_regions, const char * _debug_filename)
{
    unsigned int i, j;
    int fail[_nfft];
    for (j=0; j<_nfft; j++)
        fail[j] = 0;
    for (i=0; i<_num_regions; i++) {
        autotest_psd_s r = _regions[i];
        if (liquid_autotest_verbose) {
            printf(" region[%2u]: f=(%6.3f,%6.3f), (", i, r.fmin, r.fmax);
            if (r.test_lo) { printf("%7.2f,", r.pmin); } else { printf("   *   ,"); }
            if (r.test_hi) { printf("%7.2f)", r.pmax); } else { printf("   *   )"); }
            printf("\n");
        }
        if (r.fmin < -0.5 || r.fmax > 0.5 || r.fmin > r.fmax) {
            AUTOTEST_FAIL("invalid frequency range");
            return -1;
        }
        for (j=0; j<_nfft; j++) {
            // compute frequency value and check region
            float f = (float)j / (float)_nfft - 0.5f;
            if (f < r.fmin || f > r.fmax)
                continue;

            // test lower bound
            if (r.test_lo && _psd[j] < r.pmin) {
                //AUTOTEST_FAIL("region[%3u], %8.2f exceed minimum (%8.2f)", i, _psd[j], r.pmin);
                AUTOTEST_FAIL("minimum value exceeded");
                fail[j] = 1;
            } else {
                AUTOTEST_PASS();
            }

            // test upper bound
            if (r.test_hi && _psd[j] > r.pmax) {
                AUTOTEST_FAIL("maximum value exceeded");
                fail[j] = 1;
            } else {
                AUTOTEST_PASS();
            }
        }
    }

    // export debug file if requested
    if (_debug_filename != NULL) {
        FILE * fid = fopen(_debug_filename,"w");
        if (fid == NULL) {
            fprintf(stderr,"could not open '%s' for writing\n", _debug_filename);
            return -1;
        }
        fprintf(fid,"clear all; close all; nfft=%u; f=[0:(nfft-1)]/nfft-0.5; psd=zeros(1,nfft);\n", _nfft);
        fprintf(fid,"idx = [");
        for (i=0; i<_nfft; i++) { if (fail[i]) fprintf(fid,"%d,",i+1); }
        fprintf(fid,"];\n");
        for (i=0; i<_nfft; i++) { fprintf(fid,"psd(%6u) = %12.6f;\n", i+1, _psd[i]); }
        fprintf(fid,"figure; xlabel('f/F_s'); ylabel('PSD [dB]'); hold on;\n");
        // add target regions
        for (i=0; i<_num_regions; i++) {
            if (_regions[i].test_lo)
                fprintf(fid,"  plot([%f,%f],[%f,%f],'Color',[0.5 0 0]);\n",_regions[i].fmin,_regions[i].fmax,_regions[i].pmin,_regions[i].pmin);
            if (_regions[i].test_hi)
                fprintf(fid,"  plot([%f,%f],[%f,%f],'Color',[0 0.5 0]);\n",_regions[i].fmin,_regions[i].fmax,_regions[i].pmax,_regions[i].pmax);
        }
        // plot spectrum
        fprintf(fid,"  plot(f,psd,'LineWidth',2,'Color',[0 0.3 0.5]);\n");
        fprintf(fid,"  plot(f(idx),psd(idx),'xr');\n"); // identifying errors
        fprintf(fid,"hold off; grid on; xlim([-0.5 0.5]);\n");
        fclose(fid);
        printf("debug file written to %s\n", _debug_filename);
    }
    return 0;
}

// validate spectral content of a signal (complex)
int liquid_autotest_validate_psd_signal(float complex * _buf, unsigned int _buf_len,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename)
{
    // compute signal's power spectral density
    unsigned int nfft = 4 << liquid_nextpow2(_buf_len < 64 ? 64 : _buf_len);
    float complex * buf_time = (float complex*) malloc(nfft*sizeof(float complex));
    float complex * buf_freq = (float complex*) malloc(nfft*sizeof(float complex));
    float         * buf_psd  = (float *       ) malloc(nfft*sizeof(float        ));
    if (buf_time == NULL || buf_freq == NULL || buf_psd == NULL) {
        AUTOTEST_FAIL("liquid_autotest_validate_psd_signal(), could not allocate appropriate memory for validating psd");
        return -1;
    }
    unsigned int i;
    for (i=0; i<nfft; i++)
        buf_time[i] = i < _buf_len ? _buf[i] : 0;
    fft_run(nfft, buf_time, buf_freq, LIQUID_FFT_FORWARD, 0);
    for (i=0; i<nfft; i++)
        buf_psd[i] = 20*log10( cabsf( buf_freq[(i+nfft/2)%nfft] ) );

    // run test
    int rc = liquid_autotest_validate_spectrum(buf_psd, nfft,
            _regions, num_regions, debug_filename);

    // free memory and return
    free(buf_time);
    free(buf_freq);
    free(buf_psd);
    return rc;
}

// validate spectral content of a signal (real)
int liquid_autotest_validate_psd_signalf(float * _buf, unsigned int _buf_len,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename)
{
    // copy to temporary complex array
    float complex * buf_cplx = (float complex*) malloc(_buf_len*sizeof(float complex));
    if (buf_cplx == NULL) {
        AUTOTEST_FAIL("liquid_autotest_validate_psd_signalf(), could not allocate appropriate memory for validating psd");
        return -1;
    }
    unsigned int i;
    for (i=0; i<_buf_len; i++)
        buf_cplx[i] = _buf[i];

    // run test
    int rc = liquid_autotest_validate_psd_signal(buf_cplx, _buf_len,
            _regions, num_regions, debug_filename);

    // free memory and return
    free(buf_cplx);
    return rc;
}

// validate spectral content of a filter (real coefficients)
int liquid_autotest_validate_psd_firfilt_crcf(firfilt_crcf _q, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename)
{
    float psd[_nfft];
    unsigned int i;
    for (i=0; i<_nfft; i++) {
        float f = (float)(i)/(float)(_nfft) - 0.5f;
        float complex H;
        firfilt_crcf_freqresponse(_q, f, &H);
        psd[i] = 20*log10f(cabsf(H));
    }
    return liquid_autotest_validate_spectrum(psd,_nfft,_regions,num_regions,debug_filename);
}

// validate spectral content of a filter (complex coefficients)
int liquid_autotest_validate_psd_firfilt_cccf(firfilt_cccf _q, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename)
{
    float psd[_nfft];
    unsigned int i;
    for (i=0; i<_nfft; i++) {
        float f = (float)(i)/(float)(_nfft) - 0.5f;
        float complex H;
        firfilt_cccf_freqresponse(_q, f, &H);
        psd[i] = 20*log10f(cabsf(H));
    }
    return liquid_autotest_validate_spectrum(psd,_nfft,_regions,num_regions,debug_filename);
}

// validate spectral content of an iir filter (real coefficients, input)
int liquid_autotest_validate_psd_iirfilt_rrrf(iirfilt_rrrf _q, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename)
{
    float psd[_nfft];
    unsigned int i;
    for (i=0; i<_nfft; i++) {
        float f = (float)(i)/(float)(_nfft) - 0.5f;
        float complex H;
        iirfilt_rrrf_freqresponse(_q, f, &H);
        psd[i] = 20*log10f(cabsf(H));
    }
    return liquid_autotest_validate_spectrum(psd,_nfft,_regions,num_regions,debug_filename);
}

// validate spectral content of a spectral periodogram object
int liquid_autotest_validate_psd_spgramcf(spgramcf _q,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename)
{
    unsigned int nfft = spgramcf_get_nfft(_q);
    float psd[nfft];
    spgramcf_get_psd(_q, psd);
    return liquid_autotest_validate_spectrum(psd,nfft,_regions,num_regions,debug_filename);
}

// callback function to simplify testing for framing objects
int framing_autotest_callback(
    unsigned char *  _header,
    int              _header_valid,
    unsigned char *  _payload,
    unsigned int     _payload_len,
    int              _payload_valid,
    framesyncstats_s _stats,
    void *           _context)
{
    printf("*** callback invoked (%s) ***\n", _payload_valid ? "pass" : "FAIL");
    unsigned int * secret = (unsigned int*) _context;
    *secret = FRAMING_AUTOTEST_SECRET;
    return 0;
}

