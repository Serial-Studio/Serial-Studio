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
// Lightweight autotest header
//
// Similar to CxxTest, but written for liquid DSP code in C
//

#ifndef __LIQUID_AUTOTEST_H__
#define __LIQUID_AUTOTEST_H__

#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include "liquid.h"

// total number of checks invoked
extern unsigned long int liquid_autotest_num_checks;

// total number of checks which passed
extern unsigned long int liquid_autotest_num_passed;

// total number of checks which failed
extern unsigned long int liquid_autotest_num_failed;

// total number of warnings
extern unsigned long int liquid_autotest_num_warnings;

// verbosity flag
extern int liquid_autotest_verbose;

// fail test
// increment liquid_autotest_num_checks
// increment liquid_autotest_num_failed
void liquid_autotest_failed();

// pass test
// increment liquid_autotest_num_checks
// increment liquid_autotest_num_passed
void liquid_autotest_passed();

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
                                 double _valueR);

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
                                 int          _qualifier);

// fail test with message
//  _file       :   filename (string)
//  _line       :   line number of test
//  _message    :   message string
void liquid_autotest_failed_msg(const char * _file,
                                unsigned int _line,
                                const char * _message);

// print basic autotest results to stdout
void autotest_print_results(void);

// export results to .json file
int autotest_export_results(char * _filename);


// print warning to stderr
// increment liquid_autotest_num_warnings
//  _file       :   filename (string)
//  _line       :   line number of test
//  _message    :   message string
void liquid_autotest_warn(const char * _file,
                          unsigned int _line,
                          const char * _message);


// contend that data in two arrays are identical
//  _x      :   input array [size: _n x 1]
//  _y      :   input array [size: _n x 1]
//  _n      :   input array size
int liquid_autotest_same_data(unsigned char * _x,
                              unsigned char * _y,
                              unsigned int _n);

// print array to standard out
//  _x      :   input array [size: _n x 1]
//  _n      :   input array size
void liquid_autotest_print_array(unsigned char * _x,
                                 unsigned int _n);

// Compute magnitude of (possibly) complex number
#define LIQUID_AUTOTEST_VMAG(V) (sqrt(creal(V)*creal(V)+cimag(V)*cimag(V)))

// Compute isnan on (possibly) complex number
#define LIQUID_AUTOTEST_ISNAN(V) (isnan(crealf(V)) || isnan(cimagf(V)))

// CONTEND_TRUE
#define TEST_TRUE(F,L,EX,X)                                         \
{                                                                   \
    if (!(X))                                                       \
    {                                                               \
        liquid_autotest_failed_bool(F,L,EX,X,1);                    \
    } else {                                                        \
         liquid_autotest_passed();                                  \
    }                                                               \
}
#define CONTEND_TRUE_FL(F,L,X) TEST_TRUE(F,L,#X,(X))
#define CONTEND_TRUE(X)        CONTEND_TRUE_FL(__FILE__,__LINE__,X)

// CONTEND_FALSE
#define TEST_FALSE(F,L,EX,X)                                        \
{                                                                   \
    if ((X))                                                        \
    {                                                               \
        liquid_autotest_failed_bool(F,L,EX,X,0);                    \
    } else {                                                        \
         liquid_autotest_passed();                                  \
    }                                                               \
}
#define CONTEND_FALSE_FL(F,L,X) TEST_FALSE(F,L,#X,(X))
#define CONTEND_FALSE(X)        CONTEND_FALSE_FL(__FILE__,__LINE__,X)

// CONTEND_EQUALITY
#define TEST_EQUALITY(F,L,EX,X,EY,Y)                                \
{                                                                   \
    if ((X)!=(Y))                                                   \
    {                                                               \
        liquid_autotest_failed_expr(F,L,EX,X,"==",EY,Y);            \
    } else {                                                        \
         liquid_autotest_passed();                                  \
    }                                                               \
}
#define CONTEND_EQUALITY_FL(F,L,X,Y)      TEST_EQUALITY(F,L,#X,(X),#Y,(Y))
#define CONTEND_EQUALITY(X,Y)             CONTEND_EQUALITY_FL(__FILE__,__LINE__,X,Y)

// CONTEND_INEQUALITY
#define TEST_INEQUALITY(F,L,EX,X,EY,Y)                              \
{                                                                   \
    if ((X)==(Y))                                                   \
    {                                                               \
        liquid_autotest_failed_expr(F,L,EX,X,"!=",EY,Y);            \
    } else {                                                        \
        liquid_autotest_passed();                                   \
    }                                                               \
}
#define CONTEND_INEQUALITY_FL(F,L,X,Y)    TEST_INEQUALITY(F,L,#X,(X),#Y,(Y))
#define CONTEND_INEQUALITY(X,Y)           CONTEND_INEQUALITY_FL(__FILE__,__LINE__,X,Y)

// CONTEND_GREATER_THAN
#define TEST_GREATER_THAN(F,L,EX,X,EY,Y)                            \
{                                                                   \
    if ((X)<=(Y))                                                   \
    {                                                               \
        liquid_autotest_failed_expr(F,L,EX,X,">",EY,Y);             \
    } else {                                                        \
        liquid_autotest_passed();                                   \
    }                                                               \
}
#define CONTEND_GREATER_THAN_FL(F,L,X,Y)  TEST_GREATER_THAN(F,L,#X,(X),#Y,(Y))
#define CONTEND_GREATER_THAN(X,Y)         CONTEND_GREATER_THAN_FL(__FILE__,__LINE__,X,Y)

// CONTEND_LESS_THAN
#define TEST_LESS_THAN(F,L,EX,X,EY,Y)                               \
{                                                                   \
    if ((X)>=(Y))                                                   \
    {                                                               \
        liquid_autotest_failed_expr(F,L,EX,X,"<",EY,Y);             \
    } else {                                                        \
        liquid_autotest_passed();                                   \
    }                                                               \
}
#define CONTEND_LESS_THAN_FL(F,L,X,Y)     TEST_LESS_THAN(F,L,#X,(X),#Y,(Y))
#define CONTEND_LESS_THAN(X,Y)            CONTEND_LESS_THAN_FL(__FILE__,__LINE__,X,Y)

// CONTEND_DELTA
// Test delta between two (possibly complex numbers) is within tolerance. Use the
// expanded macro for computing magnitude of difference to account for both real
// as well as complex numbers
#define TEST_DELTA(F,L,EX,X,EY,Y,ED,D)                                          \
{                                                                               \
    if (LIQUID_AUTOTEST_VMAG((X)-(Y)) > (D) ||                                  \
        LIQUID_AUTOTEST_ISNAN((X)) || LIQUID_AUTOTEST_ISNAN((Y)) )              \
    {                                                                           \
        liquid_autotest_failed_expr(F,L,"abs(" #X "-" #Y ")",                   \
            LIQUID_AUTOTEST_VMAG((X)-(Y)),"<",ED,D);                            \
    } else {                                                                    \
        liquid_autotest_passed();                                               \
    }                                                                           \
}
#define CONTEND_DELTA_FL(F,L,X,Y,D)       TEST_DELTA(F,L,#X,(X),#Y,(Y),#D,(D))
#define CONTEND_DELTA(X,Y,D)              CONTEND_DELTA_FL(__FILE__,__LINE__,X,Y,D)

// CONTEND_EXPRESSION
#define TEST_EXPRESSION(F,L,EX,X)                                   \
{                                                                   \
    if (!X)                                                         \
    {                                                               \
        liquid_autotest_failed_expr(F,L,#X,(X),"is","1",1);         \
    } else {                                                        \
        liquid_autotest_passed();                                   \
    }                                                               \
}
#define CONTEND_EXPRESSION_FL(F,L,X)      TEST_EXPRESSION(F,L,#X,(X))
#define CONTEND_EXPRESSION(X)             CONTEND_EXPRESSION_FL(__FILE__,__LINE__,X)

// CONTEND_ISNULL
#define TEST_ISNULL(F,L,EX,X)                                       \
{                                                                   \
    if ((void*)(X)!=(NULL))                                         \
    {                                                               \
        liquid_autotest_failed_expr(F,L,EX,(long)X,"==","NULL",1);  \
    } else {                                                        \
         liquid_autotest_passed();                                  \
    }                                                               \
}
#define CONTEND_ISNULL_FL(F,L,X)          TEST_ISNULL(F,L,#X,(X))
#define CONTEND_ISNULL(X)                 CONTEND_ISNULL_FL(__FILE__,__LINE__,X)

// CONTEND_SAME_DATA
#define TEST_SAME_DATA(F,L,EX,X,EY,Y,EN,N)                                      \
{                                                                               \
    if (!liquid_autotest_same_data((uint8_t*)(X),(uint8_t*)(Y),(N)))            \
    {                                                                           \
        liquid_autotest_failed_msg(F,L,EX "[] != " EY "[] for " EN " bytes");   \
        if (liquid_autotest_verbose)                                            \
        {                                                                       \
            liquid_autotest_print_array((uint8_t*)(X),N);                       \
            liquid_autotest_print_array((uint8_t*)(Y),N);                       \
        }                                                                       \
    } else {                                                                    \
        liquid_autotest_passed();                                               \
    }                                                                           \
}
#define CONTEND_SAME_DATA_FL(F,L,X,Y,N)  TEST_SAME_DATA(F,L,#X,(X),#Y,(Y),#N,(N))
#define CONTEND_SAME_DATA(X,Y,N)         CONTEND_SAME_DATA_FL(__FILE__,__LINE__,X,Y,N)


// AUTOTEST WARN
#define AUTOTEST_WARN_FL(F,L,MSG)      liquid_autotest_warn(F,L,#MSG)
#define AUTOTEST_WARN(MSG)             AUTOTEST_WARN_FL(__FILE__,__LINE__,MSG)

// AUTOTEST PASS
#define AUTOTEST_PASS_FL(F,L)          liquid_autotest_passed()
#define AUTOTEST_PASS()                AUTOTEST_PASS_FL(__FILE__,__LINE__)

// AUTOTEST FAIL
#define AUTOTEST_FAIL_FL(F,L,MSG)      liquid_autotest_failed_msg(F,L,MSG)
#define AUTOTEST_FAIL(MSG)             AUTOTEST_FAIL_FL(__FILE__,__LINE__,MSG)

// supporting methods
typedef struct {
    float fmin, fmax;
    float pmin, pmax;
    int   test_lo, test_hi;
} autotest_psd_s;

// validate spectral content
int liquid_autotest_validate_spectrum(float * _psd, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename);

// validate spectral content of a signal (complex)
int liquid_autotest_validate_psd_signal(float complex * _buf, unsigned int _buf_len,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename);

// validate spectral content of a signal (real)
int liquid_autotest_validate_psd_signalf(float * _buf, unsigned int _buf_len,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename);

// validate spectral content of a filter (real coefficients)
int liquid_autotest_validate_psd_firfilt_crcf(firfilt_crcf _q, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename);

// validate spectral content of a filter (complex coefficients)
int liquid_autotest_validate_psd_firfilt_cccf(firfilt_cccf _q, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename);

// validate spectral content of an iir filter (real coefficients, input)
int liquid_autotest_validate_psd_iirfilt_rrrf(iirfilt_rrrf _q, unsigned int _nfft,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename);

// validate spectral content of a spectral periodogram object
int liquid_autotest_validate_psd_spgramcf(spgramcf _q,
        autotest_psd_s * _regions, unsigned int num_regions, const char * debug_filename);

// callback function to simplify testing for framing objects
#define FRAMING_AUTOTEST_SECRET 0x01234567
int framing_autotest_callback(
    unsigned char *  _header,
    int              _header_valid,
    unsigned char *  _payload,
    unsigned int     _payload_len,
    int              _payload_valid,
    framesyncstats_s _stats,
    void *           _context);

#endif // __LIQUID_AUTOTEST_H__

