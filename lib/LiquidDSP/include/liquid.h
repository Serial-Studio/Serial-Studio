/*
 * Copyright (c) 2007 - 2025 Joseph Gaeddert
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
#ifndef __LIQUID_H__
#define __LIQUID_H__

#ifdef __cplusplus
extern "C" {
#   define LIQUID_USE_COMPLEX_H 0
#else
#   define LIQUID_USE_COMPLEX_H 1
#endif // __cplusplus

// common headers
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

//
// Make sure the version and version number macros weren't defined by
// some prevoiusly included header file.
//
#ifdef LIQUID_VERSION
#  undef LIQUID_VERSION
#endif
#ifdef LIQUID_VERSION_NUMBER
#  undef LIQUID_VERSION_NUMBER
#endif

// Compile-time version numbers

// macros for expanding version numbers to string literals
#define LIQUID_VERSION_STR_EX(num) #num
#define LIQUID_VERSION_STR(num) LIQUID_VERSION_STR_EX(num)

#define LIQUID_VERSION_MAJOR    1
#define LIQUID_VERSION_MINOR    7
#define LIQUID_VERSION_PATCH    0
#define LIQUID_VERSION_DEV      0

// final version string is constructed by concatenating inidividual string versions
#define LIQUID_VERSION \
    LIQUID_VERSION_STR(LIQUID_VERSION_MAJOR) "." \
    LIQUID_VERSION_STR(LIQUID_VERSION_MINOR) "." \
    LIQUID_VERSION_STR(LIQUID_VERSION_PATCH)

// final version number is constructed from major, minor, patch
#define LIQUID_VERSION_NUMBER ( \
    (uint32_t) ( \
    (LIQUID_VERSION_MAJOR << 16) | \
    (LIQUID_VERSION_MINOR <<  8) | \
    (LIQUID_VERSION_PATCH      )))

//
// Run-time library version numbers
//
extern const char liquid_version[];
const char * liquid_libversion(void);
int liquid_libversion_number(void);

// run-time library validation
#define LIQUID_VALIDATE_LIBVERSION                                          \
  if (LIQUID_VERSION_NUMBER != liquid_libversion_number()) {                \
    fprintf(stderr,"%s:%u: ", __FILE__,__LINE__);                           \
    fprintf(stderr,"error: invalid liquid runtime library\n");              \
    fprintf(stderr,"  header version  : %d\n", LIQUID_VERSION_NUMBER);      \
    fprintf(stderr,"  library version : %d\n", liquid_libversion_number()); \
    exit(1);                                                                \
  }                                                                         \

// basic error types
#define LIQUID_NUM_ERRORS 14
typedef enum {
    // everything ok
    LIQUID_OK=0,

    // internal logic error; this is a bug with liquid and should be reported immediately
    LIQUID_EINT,

    // invalid object, examples:
    //  - destroy() method called on NULL pointer
    LIQUID_EIOBJ,

    // invalid parameter, or configuration; examples:
    //  - setting bandwidth of a filter to a negative number
    //  - setting FFT size to zero
    //  - create a spectral periodogram object with window size greater than nfft
    LIQUID_EICONFIG,

    // input out of range; examples:
    //  - try to take log of -1
    //  - try to create an FFT plan of size zero
    LIQUID_EIVAL,

    // invalid vector length or dimension; examples
    //  - trying to refer to the 17th element of a 2 x 2 matrix
    //  - trying to multiply two matrices of incompatible dimensions
    LIQUID_EIRANGE,

    // invalid mode; examples:
    //  - try to create a modem of type 'LIQUID_MODEM_XXX' which does not exit
    LIQUID_EIMODE,

    // unsupported mode (e.g. LIQUID_FEC_CONV_V27 with 'libfec' not installed)
    LIQUID_EUMODE,

    // object has not been created or properly initialized
    //  - try to run firfilt_crcf_execute(NULL, ...)
    //  - try to modulate using an arbitrary modem without initializing the constellation
    LIQUID_ENOINIT,

    // not enough memory allocated for operation; examples:
    //  - try to factor 100 = 2*2*5*5 but only give 3 spaces for factors
    LIQUID_EIMEM,

    // file input/output; examples:
    //  - could not open a file for writing because of insufficient permissions
    //  - could not open a file for reading because it does not exist
    //  - try to read more data than a file has space for
    //  - could not parse line in file (improper formatting)
    LIQUID_EIO,

    // algorithm could not converge or no solution could be found
    //  - try to find roots of polynomial can sometimes cause instability
    //  - filter design using Parks-McClellan with extremely tight constraints
    LIQUID_ENOCONV,

    // method or function declared but not implemented or disabled
    LIQUID_ENOIMP,

} liquid_error_code;

// error descriptions
extern const char * liquid_error_str[LIQUID_NUM_ERRORS];
const char *        liquid_error_info(liquid_error_code _code);

#define LIQUID_CONCAT(prefix, name) prefix ## name
#define LIQUID_VALIDATE_INPUT

/*
 * Compile-time complex data type definitions
 *
 * Default: use the C99 complex data type, otherwise
 * define complex type compatible with the C++ complex standard,
 * otherwise resort to defining binary compatible array.
 */
#if LIQUID_USE_COMPLEX_H==1
#   include <complex.h>
#   define LIQUID_DEFINE_COMPLEX(R,C) typedef R _Complex C
#elif defined _GLIBCXX_COMPLEX || defined _LIBCPP_COMPLEX
#   define LIQUID_DEFINE_COMPLEX(R,C) typedef std::complex<R> C
#else
#   define LIQUID_DEFINE_COMPLEX(R,C) typedef struct {R real; R imag;} C;
#endif
//#   define LIQUID_DEFINE_COMPLEX(R,C) typedef R C[2]

LIQUID_DEFINE_COMPLEX(float,  liquid_float_complex);
LIQUID_DEFINE_COMPLEX(double, liquid_double_complex);

// external compile-time deprecation warnings with messages
#ifdef __GNUC__
#   define DEPRECATED(MSG,X) X __attribute__((deprecated (MSG)))
#elif defined(_MSC_VER)
#   define DEPRECATED(MSG,X) __declspec(deprecated) X
#else
#   define DEPRECATED(MSG,X) X
#endif

//
// MODULE : agc (automatic gain control)
//

// available squelch modes
typedef enum {
    LIQUID_AGC_SQUELCH_UNKNOWN=0,   // unknown/unavailable squelch mode
    LIQUID_AGC_SQUELCH_ENABLED,     // squelch enabled but signal not detected
    LIQUID_AGC_SQUELCH_RISE,        // signal first hit/exceeded threshold
    LIQUID_AGC_SQUELCH_SIGNALHI,    // signal level high (above threshold)
    LIQUID_AGC_SQUELCH_FALL,        // signal first dropped below threshold
    LIQUID_AGC_SQUELCH_SIGNALLO,    // signal level low (below threshold)
    LIQUID_AGC_SQUELCH_TIMEOUT,     // signal level low (below threshold for a certain time)
    LIQUID_AGC_SQUELCH_DISABLED,    // squelch not enabled
} agc_squelch_mode;

#define LIQUID_AGC_MANGLE_CRCF(name) LIQUID_CONCAT(agc_crcf, name)
#define LIQUID_AGC_MANGLE_RRRF(name) LIQUID_CONCAT(agc_rrrf, name)

// large macro
//   AGC    : name-mangling macro
//   T      : primitive data type
//   TC     : input/output data type
#define LIQUID_AGC_DEFINE_API(AGC,T,TC)                                     \
                                                                            \
/* Automatic gain control (agc) for level correction and signal         */  \
/* detection                                                            */  \
typedef struct AGC(_s) * AGC();                                             \
                                                                            \
/* Create automatic gain control object.                                */  \
AGC() AGC(_create)(void);                                                   \
                                                                            \
/* Copy object including all internal objects and state                 */  \
AGC() AGC(_copy)(AGC() _q);                                                 \
                                                                            \
/* Destroy object, freeing all internally-allocated memory.             */  \
int AGC(_destroy)(AGC() _q);                                                \
                                                                            \
/* Print object properties to stdout, including received signal         */  \
/* strength indication (RSSI), loop bandwidth, lock status, and squelch */  \
/* status.                                                              */  \
int AGC(_print)(AGC() _q);                                                  \
                                                                            \
/* Reset internal state of agc object, including gain estimate, input   */  \
/* signal level estimate, lock status, and squelch mode                 */  \
/* If the squelch mode is disabled, it stays disabled, but all enabled  */  \
/* modes (e.g. LIQUID_AGC_SQUELCH_TIMEOUT) resets to just               */  \
/* LIQUID_AGC_SQUELCH_ENABLED.                                          */  \
int AGC(_reset)(AGC() _q);                                                  \
                                                                            \
/* Execute automatic gain control on an single input sample             */  \
/*  _q      : automatic gain control object                             */  \
/*  _x      : input sample                                              */  \
/*  _y      : output sample                                             */  \
int AGC(_execute)(AGC() _q,                                                 \
                  TC    _x,                                                 \
                  TC *  _y);                                                \
                                                                            \
/* Execute automatic gain control on block of samples pointed to by _x  */  \
/* and store the result in the array of the same length _y.             */  \
/*  _q      : automatic gain control object                             */  \
/*  _x      : input data array, [size: _n x 1]                          */  \
/*  _n      : number of input, output samples                           */  \
/*  _y      : output data array, [size: _n x 1]                         */  \
int AGC(_execute_block)(AGC()        _q,                                    \
                        TC *         _x,                                    \
                        unsigned int _n,                                    \
                        TC *         _y);                                   \
                                                                            \
/* Lock agc object. When locked, the agc object still makes an estimate */  \
/* of the signal level, but the gain setting is fixed and does not      */  \
/* change.                                                              */  \
/* This is useful for providing coarse input signal level correction    */  \
/* and quickly detecting a packet burst but not distorting signals with */  \
/* amplitude variation due to modulation.                               */  \
int AGC(_lock)(AGC() _q);                                                   \
                                                                            \
/* Unlock agc object, and allow amplitude correction to resume.         */  \
int AGC(_unlock)(AGC() _q);                                                 \
                                                                            \
/* Get lock state of agc object                                         */  \
int AGC(_is_locked)(AGC() _q);                                              \
                                                                            \
/* Set loop filter bandwidth: attack/release time.                      */  \
/*  _q      : automatic gain control object                             */  \
/*  _bt     : bandwidth-time constant, _bt > 0                          */  \
int AGC(_set_bandwidth)(AGC() _q, float _bt);                               \
                                                                            \
/* Get the agc object's loop filter bandwidth.                          */  \
float AGC(_get_bandwidth)(AGC() _q);                                        \
                                                                            \
/* Get the input signal's estimated energy level, relative to unity.    */  \
/* The result is a linear value.                                        */  \
float AGC(_get_signal_level)(AGC() _q);                                     \
                                                                            \
/* Set the agc object's estimate of the input signal by specifying an   */  \
/* explicit linear value. This is useful for initializing the agc       */  \
/* object with a preliminary estimate of the signal level to help gain  */  \
/* convergence.                                                         */  \
/*  _q      : automatic gain control object                             */  \
/*  _x2     : signal level of input, _x2 > 0                            */  \
int AGC(_set_signal_level)(AGC() _q,                                        \
                           float _x2);                                      \
                                                                            \
/* Get the agc object's estimated received signal strength indication   */  \
/* (RSSI) on the input signal.                                          */  \
/* This is similar to getting the signal level (above), but returns the */  \
/* result in dB rather than on a linear scale.                          */  \
float AGC(_get_rssi)(AGC() _q);                                             \
                                                                            \
/* Set the agc object's estimated received signal strength indication   */  \
/* (RSSI) on the input signal by specifying an explicit value in dB.    */  \
/*  _q      : automatic gain control object                             */  \
/*  _rssi   : signal level of input [dB]                                */  \
int AGC(_set_rssi)(AGC() _q, float _rssi);                                  \
                                                                            \
/* Get the gain value currently being applied to the input signal       */  \
/* (linear).                                                            */  \
float AGC(_get_gain)(AGC() _q);                                             \
                                                                            \
/* Set the agc object's internal gain by specifying an explicit linear  */  \
/* value.                                                               */  \
/*  _q      : automatic gain control object                             */  \
/*  _gain   : gain to apply to input signal, _gain > 0                  */  \
int AGC(_set_gain)(AGC() _q,                                                \
                   float _gain);                                            \
                                                                            \
/* Get the output scaling applied to each sample (linear).              */  \
float AGC(_get_scale)(AGC() _q);                                            \
                                                                            \
/* Set the agc object's output scaling (linear). Note that this does    */  \
/* affect the response of the AGC.                                      */  \
/*  _q      : automatic gain control object                             */  \
/*  _gain   : gain to apply to input signal, _gain > 0                  */  \
int AGC(_set_scale)(AGC() _q,                                               \
                    float _scale);                                          \
                                                                            \
/* Estimate signal level and initialize internal gain on an input       */  \
/* array.                                                               */  \
/*  _q      : automatic gain control object                             */  \
/*  _x      : input data array, [size: _n x 1]                          */  \
/*  _n      : number of input, output samples                           */  \
int AGC(_init)(AGC()        _q,                                             \
               TC *         _x,                                             \
               unsigned int _n);                                            \
                                                                            \
/* Enable squelch mode.                                                 */  \
int AGC(_squelch_enable)(AGC() _q);                                         \
                                                                            \
/* Disable squelch mode.                                                */  \
int AGC(_squelch_disable)(AGC() _q);                                        \
                                                                            \
/* Return flag indicating if squelch is enabled or not.                 */  \
int AGC(_squelch_is_enabled)(AGC() _q);                                     \
                                                                            \
/* Set threshold for enabling/disabling squelch.                        */  \
/*  _q      :   automatic gain control object                           */  \
/*  _thresh :   threshold for enabling squelch [dB]                     */  \
int AGC(_squelch_set_threshold)(AGC() _q,                                   \
                                 T     _thresh);                            \
                                                                            \
/* Get squelch threshold (value in dB)                                  */  \
T    AGC(_squelch_get_threshold)(AGC() _q);                                 \
                                                                            \
/* Set timeout before enabling squelch.                                 */  \
/*  _q       : automatic gain control object                            */  \
/*  _timeout : timeout before enabling squelch [samples]                */  \
int AGC(_squelch_set_timeout)(AGC()        _q,                              \
                              unsigned int _timeout);                       \
                                                                            \
/* Get squelch timeout (number of samples)                              */  \
unsigned int AGC(_squelch_get_timeout)(AGC() _q);                           \
                                                                            \
/* Get squelch status (e.g. LIQUID_AGC_SQUELCH_TIMEOUT)                 */  \
int AGC(_squelch_get_status)(AGC() _q);                                     \

// Define agc APIs
LIQUID_AGC_DEFINE_API(LIQUID_AGC_MANGLE_CRCF, float, liquid_float_complex)
LIQUID_AGC_DEFINE_API(LIQUID_AGC_MANGLE_RRRF, float, float)



//
// MODULE : audio
//

// CVSD: continuously variable slope delta
typedef struct cvsd_s * cvsd;

// create cvsd object
//  _num_bits   :   number of adjacent bits to observe (4 recommended)
//  _zeta       :   slope adjustment multiplier (1.5 recommended)
//  _alpha      :   pre-/post-emphasis filter coefficient (0.9 recommended)
// NOTE: _alpha must be in [0,1]
cvsd cvsd_create(unsigned int _num_bits,
                 float _zeta,
                 float _alpha);

// destroy cvsd object
int cvsd_destroy(cvsd _q);

// print cvsd object parameters
int cvsd_print(cvsd _q);

// encode/decode single sample
unsigned char   cvsd_encode(cvsd _q, float _audio_sample);
float           cvsd_decode(cvsd _q, unsigned char _bit);

// encode/decode 8 samples at a time
int cvsd_encode8(cvsd _q, float * _audio, unsigned char * _data);
int cvsd_decode8(cvsd _q, unsigned char _data, float * _audio);


//
// MODULE : buffer
//

// circular buffer
#define LIQUID_CBUFFER_MANGLE_FLOAT(name)  LIQUID_CONCAT(cbufferf,  name)
#define LIQUID_CBUFFER_MANGLE_CFLOAT(name) LIQUID_CONCAT(cbuffercf, name)

// large macro
//   CBUFFER : name-mangling macro
//   T       : data type
#define LIQUID_CBUFFER_DEFINE_API(CBUFFER,T)                                \
                                                                            \
/* Circular buffer object for storing and retrieving samples in a       */  \
/* first-in/first-out (FIFO) manner using a minimal amount of memory    */  \
typedef struct CBUFFER(_s) * CBUFFER();                                     \
                                                                            \
/* Create circular buffer object of a particular maximum storage length */  \
/*  _max_size  : maximum buffer size, _max_size > 0                     */  \
CBUFFER() CBUFFER(_create)(unsigned int _max_size);                         \
                                                                            \
/* Create circular buffer object of a particular maximum storage size   */  \
/* and specify the maximum number of elements that can be read at any   */  \
/* any given time                                                       */  \
/*  _max_size  : maximum buffer size, _max_size > 0                     */  \
/*  _max_read  : maximum size that will be read from buffer             */  \
CBUFFER() CBUFFER(_create_max)(unsigned int _max_size,                      \
                               unsigned int _max_read);                     \
                                                                            \
/* Copy object including all internal objects and state                 */  \
CBUFFER() CBUFFER(_copy)(CBUFFER() _q);                                     \
                                                                            \
/* Destroy cbuffer object, freeing all internal memory                  */  \
int CBUFFER(_destroy)(CBUFFER() _q);                                        \
                                                                            \
/* Print cbuffer object properties to stdout                            */  \
int CBUFFER(_print)(CBUFFER() _q);                                          \
                                                                            \
/* Clear internal buffer                                                */  \
int CBUFFER(_reset)(CBUFFER() _q);                                          \
                                                                            \
/* Get the number of elements currently in the buffer                   */  \
unsigned int CBUFFER(_size)(CBUFFER() _q);                                  \
                                                                            \
/* Get the maximum number of elements the buffer can hold               */  \
unsigned int CBUFFER(_max_size)(CBUFFER() _q);                              \
                                                                            \
/* Get the maximum number of elements you may read at once              */  \
unsigned int CBUFFER(_max_read)(CBUFFER() _q);                              \
                                                                            \
/* Get the number of available slots (max_size - size)                  */  \
unsigned int CBUFFER(_space_available)(CBUFFER() _q);                       \
                                                                            \
/* Return flag indicating if the buffer is empty or not                 */  \
int CBUFFER(_is_empty)(CBUFFER() _q);                                       \
                                                                            \
/* Return flag indicating if the buffer is full or not                  */  \
int CBUFFER(_is_full)(CBUFFER() _q);                                        \
                                                                            \
/* Write a single sample into the buffer                                */  \
/*  _q  : circular buffer object                                        */  \
/*  _v  : input sample                                                  */  \
int CBUFFER(_push)(CBUFFER() _q,                                            \
                   T         _v);                                           \
                                                                            \
/* Write a block of samples to the buffer                               */  \
/*  _q  : circular buffer object                                        */  \
/*  _v  : array of samples to write to buffer                           */  \
/*  _n  : number of samples to write                                    */  \
int CBUFFER(_write)(CBUFFER()    _q,                                        \
                    T *          _v,                                        \
                    unsigned int _n);                                       \
                                                                            \
/* Remove and return a single element from the buffer by setting the    */  \
/* value of the output sample pointed to by _v                          */  \
/*  _q  : circular buffer object                                        */  \
/*  _v  : pointer to sample output                                      */  \
int CBUFFER(_pop)(CBUFFER() _q,                                             \
                  T *       _v);                                            \
                                                                            \
/* Read buffer contents by returning a pointer to the linearized array; */  \
/* note that the returned pointer is only valid until another operation */  \
/* is performed on the circular buffer object                           */  \
/*  _q              : circular buffer object                            */  \
/*  _num_requested  : number of elements requested                      */  \
/*  _v              : output pointer                                    */  \
/*  _num_read       : number of elements referenced by _v               */  \
int CBUFFER(_read)(CBUFFER()      _q,                                       \
                   unsigned int   _num_requested,                           \
                   T **           _v,                                       \
                   unsigned int * _num_read);                               \
                                                                            \
/* Release _n samples from the buffer                                   */  \
/*  _q : circular buffer object                                         */  \
/*  _n : number of elements to release                                  */  \
int CBUFFER(_release)(CBUFFER()    _q,                                      \
                      unsigned int _n);                                     \

// Define buffer APIs
LIQUID_CBUFFER_DEFINE_API(LIQUID_CBUFFER_MANGLE_FLOAT,  float)
LIQUID_CBUFFER_DEFINE_API(LIQUID_CBUFFER_MANGLE_CFLOAT, liquid_float_complex)



// Windowing functions
#define LIQUID_WINDOW_MANGLE_FLOAT(name)  LIQUID_CONCAT(windowf,  name)
#define LIQUID_WINDOW_MANGLE_CFLOAT(name) LIQUID_CONCAT(windowcf, name)

// large macro
//   WINDOW : name-mangling macro
//   T      : data type
#define LIQUID_WINDOW_DEFINE_API(WINDOW,T)                                  \
                                                                            \
/* Sliding window first-in/first-out buffer with a fixed size           */  \
typedef struct WINDOW(_s) * WINDOW();                                       \
                                                                            \
/* Create window buffer object of a fixed length. Samples may be pushed */  \
/* into the buffer which retains the most recent \(n\) samples.         */  \
/*  _n      : length of the window buffer [samples]                     */  \
WINDOW() WINDOW(_create)(unsigned int _n);                                  \
                                                                            \
/* Recreate window buffer object with new length.                       */  \
/* This extends an existing window's size, similar to the standard C    */  \
/* library's realloc() to n samples.                                    */  \
/* If the size of the new window is larger than the old one, the newest */  \
/* values are retained at the beginning of the buffer and the oldest    */  \
/* values are truncated. If the size of the new window is smaller than  */  \
/* the old one, the oldest values are truncated.                        */  \
/*  _q      : old window object                                         */  \
/*  _n      : new window length [samples]                               */  \
WINDOW() WINDOW(_recreate)(WINDOW() _q, unsigned int _n);                   \
                                                                            \
/* Copy object including all internal objects and state                 */  \
WINDOW() WINDOW(_copy)(WINDOW() _q);                                        \
                                                                            \
/* Destroy window object, freeing all internally memory                 */  \
int WINDOW(_destroy)(WINDOW() _q);                                          \
                                                                            \
/* Print window object to stdout                                        */  \
int WINDOW(_print)(WINDOW() _q);                                            \
                                                                            \
/* Print window object to stdout (with extra information)               */  \
int WINDOW(_debug_print)(WINDOW() _q);                                      \
                                                                            \
/* Reset window object (initialize to zeros)                            */  \
int WINDOW(_reset)(WINDOW() _q);                                            \
                                                                            \
/* Read the contents of the window by returning a pointer to the        */  \
/* aligned internal memory array. This method guarantees that the       */  \
/* elements are linearized. This method should only be used for         */  \
/* reading; writing values to the buffer has unspecified results.       */  \
/* Note that the returned pointer is only valid until another operation */  \
/* is performed on the window buffer object                             */  \
/*  _q      : window object                                             */  \
/*  _v      : output pointer (set to internal array)                    */  \
int WINDOW(_read)(WINDOW() _q,                                              \
                  T **     _v);                                             \
                                                                            \
/* Index single element in buffer at a particular index                 */  \
/* This retrieves the \(i^{th}\) sample in the window, storing the      */  \
/* output value in _v.                                                  */  \
/* This is equivalent to first invoking read() and then indexing on the */  \
/* resulting pointer; however the result is obtained much faster.       */  \
/* Therefore setting the index to 0 returns the oldest value in the     */  \
/* window.                                                              */  \
/*  _q      : window object                                             */  \
/*  _i      : index of element to read                                  */  \
/*  _v      : output value pointer                                      */  \
int WINDOW(_index)(WINDOW()     _q,                                         \
                   unsigned int _i,                                         \
                   T *          _v);                                        \
                                                                            \
/* Shifts a single sample into the right side of the window, pushing    */  \
/* the oldest (left-most) sample out of the end. Unlike stacks, the     */  \
/* window object has no equivalent "pop" method, as values are retained */  \
/* in memory until they are overwritten.                                */  \
/*  _q      : window object                                             */  \
/*  _v      : single input element                                      */  \
int WINDOW(_push)(WINDOW() _q,                                              \
                  T        _v);                                             \
                                                                            \
/* Write array of elements onto window buffer                           */  \
/* Effectively, this is equivalent to pushing each sample one at a      */  \
/* time, but executes much faster.                                      */  \
/*  _q      : window object                                             */  \
/*  _v      : input array of values to write                            */  \
/*  _n      : number of input values to write                           */  \
int WINDOW(_write)(WINDOW()     _q,                                         \
                   T *          _v,                                         \
                   unsigned int _n);                                        \

// Define window APIs
LIQUID_WINDOW_DEFINE_API(LIQUID_WINDOW_MANGLE_FLOAT,  float)
LIQUID_WINDOW_DEFINE_API(LIQUID_WINDOW_MANGLE_CFLOAT, liquid_float_complex)
//LIQUID_WINDOW_DEFINE_API(LIQUID_WINDOW_MANGLE_UINT,   unsigned int)


// wdelay functions : windowed-delay
// Implements an efficient z^-k delay with minimal memory
#define LIQUID_WDELAY_MANGLE_FLOAT(name)  LIQUID_CONCAT(wdelayf,  name)
#define LIQUID_WDELAY_MANGLE_CFLOAT(name) LIQUID_CONCAT(wdelaycf, name)
//#define LIQUID_WDELAY_MANGLE_UINT(name)   LIQUID_CONCAT(wdelayui, name)

// large macro
//   WDELAY : name-mangling macro
//   T      : data type
#define LIQUID_WDELAY_DEFINE_API(WDELAY,T)                                  \
                                                                            \
/* Efficient digital delay line using a minimal amount of memory        */  \
typedef struct WDELAY(_s) * WDELAY();                                       \
                                                                            \
/* Create delay buffer object with a particular number of samples of    */  \
/* delay                                                                */  \
/*  _delay  :   number of samples of delay in the wdelay object         */  \
WDELAY() WDELAY(_create)(unsigned int _delay);                              \
                                                                            \
/* Copy object including all internal objects and state                 */  \
WDELAY() WDELAY(_copy)(WDELAY() _q);                                        \
                                                                            \
/* Re-create delay buffer object, adjusting the delay size, preserving  */  \
/* the internal state of the object                                     */  \
/*  _q      :   old delay buffer object                                 */  \
/*  _delay  :   delay for new object                                    */  \
WDELAY() WDELAY(_recreate)(WDELAY()     _q,                                 \
                           unsigned int _delay);                            \
                                                                            \
/* Destroy delay buffer object, freeing internal memory                 */  \
int WDELAY(_destroy)(WDELAY() _q);                                          \
                                                                            \
/* Print delay buffer object's state to stdout                          */  \
int WDELAY(_print)(WDELAY() _q);                                            \
                                                                            \
/* Clear/reset state of object                                          */  \
int WDELAY(_reset)(WDELAY() _q);                                            \
                                                                            \
/* Read delayed sample at the head of the buffer and store it to the    */  \
/* output pointer                                                       */  \
/*  _q  :   delay buffer object                                         */  \
/*  _v  :   value of delayed element                                    */  \
int WDELAY(_read)(WDELAY() _q,                                              \
                  T *      _v);                                             \
                                                                            \
/* Push new sample into delay buffer object                             */  \
/*  _q  :   delay buffer object                                         */  \
/*  _v  :   new value to be added to buffer                             */  \
int WDELAY(_push)(WDELAY() _q,                                              \
                  T        _v);                                             \

// Define wdelay APIs
LIQUID_WDELAY_DEFINE_API(LIQUID_WDELAY_MANGLE_FLOAT,  float)
LIQUID_WDELAY_DEFINE_API(LIQUID_WDELAY_MANGLE_CFLOAT, liquid_float_complex)
//LIQUID_WDELAY_DEFINE_API(LIQUID_WDELAY_MANGLE_UINT,   unsigned int)



//
// MODULE : channel
//

#define LIQUID_CHANNEL_MANGLE_CCCF(name) LIQUID_CONCAT(channel_cccf,name)

// large macro
//   CHANNEL    : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_CHANNEL_DEFINE_API(CHANNEL,TO,TC,TI)                         \
                                                                            \
/* Channel emulation                                                    */  \
typedef struct CHANNEL(_s) * CHANNEL();                                     \
                                                                            \
/* Create channel object with default parameters                        */  \
CHANNEL() CHANNEL(_create)(void);                                           \
                                                                            \
/* Copy object including all internal objects and state                 */  \
CHANNEL() CHANNEL(_copy)(CHANNEL() _q);                                     \
                                                                            \
/* Destroy channel object, freeing all internal memory                  */  \
int CHANNEL(_destroy)(CHANNEL() _q);                                        \
                                                                            \
/* Print channel object internals to standard output                    */  \
int CHANNEL(_print)(CHANNEL() _q);                                          \
                                                                            \
/* Include additive white Gausss noise impairment                       */  \
/*  _q          : channel object                                        */  \
/*  _noise_floor: noise floor power spectral density [dB]               */  \
/*  _snr        : signal-to-noise ratio [dB]                            */  \
int CHANNEL(_add_awgn)(CHANNEL() _q,                                        \
                       float     _noise_floor,                              \
                       float     _snr);                                     \
                                                                            \
/* Include carrier offset impairment                                    */  \
/*  _q          : channel object                                        */  \
/*  _frequency  : carrier frequency offset [radians/sample]             */  \
/*  _phase      : carrier phase offset [radians]                        */  \
int CHANNEL(_add_carrier_offset)(CHANNEL() _q,                              \
                                 float     _frequency,                      \
                                 float     _phase);                         \
                                                                            \
/* Include multi-path channel impairment                                */  \
/*  _q          : channel object                                        */  \
/*  _h          : channel coefficients (NULL for random)                */  \
/*  _h_len      : number of channel coefficients                        */  \
int CHANNEL(_add_multipath)(CHANNEL()    _q,                                \
                            TC *         _h,                                \
                            unsigned int _h_len);                           \
                                                                            \
/* Include slowly-varying shadowing impairment                          */  \
/*  _q          : channel object                                        */  \
/*  _sigma      : standard deviation for log-normal shadowing           */  \
/*  _fd         : Doppler frequency, 0 <= _fd < 0.5                     */  \
int CHANNEL(_add_shadowing)(CHANNEL()    _q,                                \
                            float        _sigma,                            \
                            float        _fd);                              \
                                                                            \
/* Apply channel impairments on single input sample                     */  \
/*  _q      : channel object                                            */  \
/*  _x      : input sample                                              */  \
/*  _y      : pointer to output sample                                  */  \
int CHANNEL(_execute)(CHANNEL()      _q,                                    \
                      TI             _x,                                    \
                      TO *           _y);                                   \
                                                                            \
/* Apply channel impairments on block of samples                        */  \
/*  _q      : channel object                                            */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : input array, length                                       */  \
/*  _y      : output array, [size: _n x 1]                              */  \
int CHANNEL(_execute_block)(CHANNEL()      _q,                              \
                            TI *           _x,                              \
                            unsigned int   _n,                              \
                            TO *           _y);                             \

LIQUID_CHANNEL_DEFINE_API(LIQUID_CHANNEL_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)


//
// time-varying multi-path channel
//
#define LIQUID_TVMPCH_MANGLE_CCCF(name) LIQUID_CONCAT(tvmpch_cccf,name)

// large macro
//   TVMPCH    : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_TVMPCH_DEFINE_API(TVMPCH,TO,TC,TI)                           \
                                                                            \
/* Time-varying multipath channel emulation                             */  \
typedef struct TVMPCH(_s) * TVMPCH();                                       \
                                                                            \
/* Create time-varying multi-path channel emulator object, specifying   */  \
/* the number of coefficients, the standard deviation of coefficients,  */  \
/* and the coherence time. The larger the standard deviation, the more  */  \
/* dramatic the frequency response of the channel. The shorter the      */  \
/* coeherent time, the faster the channel effects.                      */  \
/*  _n      :   number of coefficients, _n > 0                          */  \
/*  _std    :   standard deviation, _std >= 0                           */  \
/*  _tau    :   normalized coherence time, 0 < _tau < 1                 */  \
TVMPCH() TVMPCH(_create)(unsigned int _n,                                   \
                         float        _std,                                 \
                         float        _tau);                                \
                                                                            \
/* Copy object including all internal objects and state                 */  \
TVMPCH() TVMPCH(_copy)(TVMPCH() _q);                                        \
                                                                            \
/* Destroy channel object, freeing all internal memory                  */  \
int TVMPCH(_destroy)(TVMPCH() _q);                                          \
                                                                            \
/* Reset object                                                         */  \
int TVMPCH(_reset)(TVMPCH() _q);                                            \
                                                                            \
/* Print channel object internals to standard output                    */  \
int TVMPCH(_print)(TVMPCH() _q);                                            \
                                                                            \
/* Push sample into emulator                                            */  \
/*  _q      : channel object                                            */  \
/*  _x      : input sample                                              */  \
int TVMPCH(_push)(TVMPCH() _q,                                              \
                  TI       _x);                                             \
                                                                            \
/* Compute output sample                                                */  \
/*  _q      : channel object                                            */  \
/*  _y      : output sample                                             */  \
int TVMPCH(_execute)(TVMPCH() _q,                                           \
                     TO *     _y);                                          \
                                                                            \
/* Execute filter on one sample, equivalent to push() and execute()     */  \
/*  _q      : channel object                                            */  \
/*  _x      : single input sample                                       */  \
/*  _y      : pointer to single output sample                           */  \
int TVMPCH(_execute_one)(TVMPCH() _q,                                       \
                         TI       _x,                                       \
                         TO *     _y);                                      \
                                                                            \
/* Apply channel impairments on a block of samples                      */  \
/*  _q      : channel object                                            */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : input array length                                        */  \
/*  _y      : output array, [size: _n x 1]                              */  \
int TVMPCH(_execute_block)(TVMPCH()     _q,                                 \
                           TI *         _x,                                 \
                           unsigned int _n,                                 \
                           TO *         _y);                                \

LIQUID_TVMPCH_DEFINE_API(LIQUID_TVMPCH_MANGLE_CCCF,
                         liquid_float_complex,
                         liquid_float_complex,
                         liquid_float_complex)


//
// MODULE : dotprod (vector dot product)
//

#define LIQUID_DOTPROD_MANGLE_RRRF(name) LIQUID_CONCAT(dotprod_rrrf,name)
#define LIQUID_DOTPROD_MANGLE_CCCF(name) LIQUID_CONCAT(dotprod_cccf,name)
#define LIQUID_DOTPROD_MANGLE_CRCF(name) LIQUID_CONCAT(dotprod_crcf,name)

// large macro
//   DOTPROD    : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_DOTPROD_DEFINE_API(DOTPROD,TO,TC,TI)                         \
                                                                            \
/* Vector dot product operation                                         */  \
typedef struct DOTPROD(_s) * DOTPROD();                                     \
                                                                            \
/* Run dot product without creating object. This is less efficient than */  \
/* creating the object as it is an unoptimized portable implementation  */  \
/* that doesn't take advantage of processor extensions. It is meant to  */  \
/* provide a baseline for performance comparison and a convenient way   */  \
/* to invoke a dot product operation when fast operation is not         */  \
/* necessary.                                                           */  \
/*  _v      : coefficients array, [size: _n x 1]                        */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : dotprod length, _n > 0                                    */  \
/*  _y      : output sample pointer                                     */  \
int DOTPROD(_run)( TC *         _v,                                         \
                   TI *         _x,                                         \
                   unsigned int _n,                                         \
                   TO *         _y);                                        \
                                                                            \
/* This provides the same unoptimized operation as the 'run()' method   */  \
/* above, but with the loop unrolled by a factor of 4. It is marginally */  \
/* faster than 'run()' without unrolling the loop.                      */  \
/*  _v      : coefficients array, [size: _n x 1]                        */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : dotprod length, _n > 0                                    */  \
/*  _y      : output sample pointer                                     */  \
int DOTPROD(_run4)(TC *         _v,                                         \
                   TI *         _x,                                         \
                   unsigned int _n,                                         \
                   TO *         _y);                                        \
                                                                            \
/* Create vector dot product object                                     */  \
/*  _v      : coefficients array, [size: _n x 1]                        */  \
/*  _n      : dotprod length, _n > 0                                    */  \
DOTPROD() DOTPROD(_create)(TC *         _v,                                 \
                           unsigned int _n);                                \
                                                                            \
/* Create vector dot product object with time-reversed coefficients     */  \
/*  _v      : time-reversed coefficients array, [size: _n x 1]          */  \
/*  _n      : dotprod length, _n > 0                                    */  \
DOTPROD() DOTPROD(_create_rev)(TC *         _v,                             \
                               unsigned int _n);                            \
                                                                            \
/* Re-create dot product object of potentially a different length with  */  \
/* different coefficients. If the length of the dot product object does */  \
/* not change, no memory reallocation is invoked.                       */  \
/*  _q      : old dotprod object                                        */  \
/*  _v      : coefficients array, [size: _n x 1]                        */  \
/*  _n      : dotprod length, _n > 0                                    */  \
DOTPROD() DOTPROD(_recreate)(DOTPROD()    _q,                               \
                             TC *         _v,                               \
                             unsigned int _n);                              \
                                                                            \
/* Re-create dot product object of potentially a different length with  */  \
/* different coefficients. If the length of the dot product object does */  \
/* not change, no memory reallocation is invoked. Filter coefficients   */  \
/* are stored in reverse order.                                         */  \
/*  _q      : old dotprod object                                        */  \
/*  _v      : time-reversed coefficients array, [size: _n x 1]          */  \
/*  _n      : dotprod length, _n > 0                                    */  \
DOTPROD() DOTPROD(_recreate_rev)(DOTPROD()    _q,                           \
                                 TC *         _v,                           \
                                 unsigned int _n);                          \
                                                                            \
/* Copy object including all internal objects and state                 */  \
DOTPROD() DOTPROD(_copy)(DOTPROD() _q);                                     \
                                                                            \
/* Destroy dotprod object, freeing all internal memory                  */  \
int DOTPROD(_destroy)(DOTPROD() _q);                                        \
                                                                            \
/* Print dotprod object internals to standard output                    */  \
int DOTPROD(_print)(DOTPROD() _q);                                          \
                                                                            \
/* Execute dot product on an input array                                */  \
/*  _q      : dotprod object                                            */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _y      : output sample pointer                                     */  \
int DOTPROD(_execute)(DOTPROD() _q,                                         \
                      TI *      _x,                                         \
                      TO *      _y);                                        \

LIQUID_DOTPROD_DEFINE_API(LIQUID_DOTPROD_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_DOTPROD_DEFINE_API(LIQUID_DOTPROD_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)

LIQUID_DOTPROD_DEFINE_API(LIQUID_DOTPROD_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)

//
// sum squared methods
//

float liquid_sumsqf(float *      _v,
                    unsigned int _n);

float liquid_sumsqcf(liquid_float_complex * _v,
                     unsigned int           _n);


//
// MODULE : equalization
//

// least mean-squares (LMS)
#define LIQUID_EQLMS_MANGLE_RRRF(name) LIQUID_CONCAT(eqlms_rrrf,name)
#define LIQUID_EQLMS_MANGLE_CCCF(name) LIQUID_CONCAT(eqlms_cccf,name)

// large macro
//   EQLMS  : name-mangling macro
//   T      : data type
#define LIQUID_EQLMS_DEFINE_API(EQLMS,T)                                    \
                                                                            \
/* Least mean-squares equalization object                               */  \
typedef struct EQLMS(_s) * EQLMS();                                         \
                                                                            \
/* Create LMS EQ initialized with external coefficients                 */  \
/*  _h : filter coefficients; set to NULL for {1,0,0...},[size: _n x 1] */  \
/*  _n : filter length                                                  */  \
EQLMS() EQLMS(_create)(T *          _h,                                     \
                       unsigned int _n);                                    \
                                                                            \
/* Create LMS EQ initialized with square-root Nyquist prototype filter  */  \
/* as initial set of coefficients. This is useful for applications      */  \
/* where the baseline matched filter is a good starting point, but      */  \
/* where equalization is needed to properly remove inter-symbol         */  \
/* interference.                                                        */  \
/* The filter length is \(2 k m + 1\)                                   */  \
/*  _type   : filter type (e.g. LIQUID_FIRFILT_RRC)                     */  \
/*  _k      : samples/symbol                                            */  \
/*  _m      : filter delay (symbols)                                    */  \
/*  _beta   : rolloff factor (0 < beta <= 1)                            */  \
/*  _dt     : fractional sample delay                                   */  \
EQLMS() EQLMS(_create_rnyquist)(int          _type,                         \
                                unsigned int _k,                            \
                                unsigned int _m,                            \
                                float        _beta,                         \
                                float        _dt);                          \
                                                                            \
/* Create LMS EQ initialized with low-pass filter                       */  \
/*  _n      : filter length                                             */  \
/*  _fc     : filter cut-off normalized to sample rate, 0 < _fc <= 0.5  */  \
EQLMS() EQLMS(_create_lowpass)(unsigned int _n,                             \
                               float        _fc);                           \
                                                                            \
/* Recreate LMS EQ initialized with external coefficients               */  \
/*  _q : old equalization object                                        */  \
/*  _h : filter coefficients; set to NULL for {1,0,0...},[size: _n x 1] */  \
/*  _n : filter length                                                  */  \
EQLMS() EQLMS(_recreate)(EQLMS()      _q,                                   \
                         T *          _h,                                   \
                         unsigned int _n);                                  \
                                                                            \
/* Copy object including all internal objects and state                 */  \
EQLMS() EQLMS(_copy)(EQLMS() _q);                                           \
                                                                            \
/* Destroy equalizer object, freeing all internal memory                */  \
int EQLMS(_destroy)(EQLMS() _q);                                            \
                                                                            \
/* Reset equalizer object, clearing internal state                      */  \
int EQLMS(_reset)(EQLMS() _q);                                              \
                                                                            \
/* Print equalizer internal state                                       */  \
int EQLMS(_print)(EQLMS() _q);                                              \
                                                                            \
/* Get equalizer learning rate                                          */  \
float EQLMS(_get_bw)(EQLMS() _q);                                           \
                                                                            \
/* Set equalizer learning rate                                          */  \
/*  _q      :   equalizer object                                        */  \
/*  _lambda :   learning rate, _lambda > 0                              */  \
int EQLMS(_set_bw)(EQLMS() _q,                                              \
                   float   _lambda);                                        \
                                                                            \
/* Get length of equalizer object (number of internal coefficients)     */  \
unsigned int EQLMS(_get_length)(EQLMS() _q);                                \
                                                                            \
/* Get pointer to coefficients array                                    */  \
const T * EQLMS(_get_coefficients)(EQLMS() _q);                             \
                                                                            \
/* Copy internal coefficients to external buffer                        */  \
/*  _q      : filter object                                             */  \
/*  _w      : pointer to output coefficients array, [size: _n x 1]      */  \
int EQLMS(_copy_coefficients)(EQLMS() _q,                                   \
                              T *     _w);                                  \
                                                                            \
/* Get equalizer's internal coefficients                                */  \
/*  _q      : filter object                                             */  \
/*  _w      : pointer to output coefficients array, [size: _n x 1]      */  \
DEPRECATED("use eqlms_xxxt_copy_coefficients(...) instead",                 \
void EQLMS(_get_weights)(EQLMS() _q,                                        \
                         T *     _w);                                       \
)                                                                           \
                                                                            \
/* Push sample into equalizer internal buffer                           */  \
/*  _q      :   equalizer object                                        */  \
/*  _x      :   input sample                                            */  \
int EQLMS(_push)(EQLMS() _q,                                                \
                 T       _x);                                               \
                                                                            \
/* Push block of samples into internal buffer of equalizer object       */  \
/*  _q      :   equalizer object                                        */  \
/*  _x      :   input sample array, [size: _n x 1]                      */  \
/*  _n      :   input sample array length                               */  \
int EQLMS(_push_block)(EQLMS()      _q,                                     \
                       T *          _x,                                     \
                       unsigned int _n);                                    \
                                                                            \
/* Execute internal dot product and return result                       */  \
/*  _q      :   equalizer object                                        */  \
/*  _y      :   output sample                                           */  \
int EQLMS(_execute)(EQLMS() _q,                                             \
                    T *     _y);                                            \
                                                                            \
/* Execute equalizer as decimator                                       */  \
/*  _q      :   equalizer object                                        */  \
/*  _x      :   input sample array, [size: _k x 1]                      */  \
/*  _y      :   output sample                                           */  \
/*  _k      :   down-sampling rate                                      */  \
int EQLMS(_decim_execute)(EQLMS()      _q,                                  \
                          T *          _x,                                  \
                          T *          _y,                                  \
                          unsigned int _k);                                 \
                                                                            \
/* Execute equalizer with block of samples using constant               */  \
/* modulus algorithm, operating on a decimation rate of _k              */  \
/* samples.                                                             */  \
/*  _q      :   equalizer object                                        */  \
/*  _k      :   down-sampling rate                                      */  \
/*  _x      :   input sample array, [size: _n x 1]                      */  \
/*  _n      :   input sample array length                               */  \
/*  _y      :   output sample array, [size: _n x 1]                     */  \
int EQLMS(_execute_block)(EQLMS()      _q,                                  \
                          unsigned int _k,                                  \
                          T *          _x,                                  \
                          unsigned int _n,                                  \
                          T *          _y);                                 \
                                                                            \
/* Step through one cycle of equalizer training                         */  \
/*  _q      :   equalizer object                                        */  \
/*  _d      :   desired output                                          */  \
/*  _d_hat  :   actual output                                           */  \
int EQLMS(_step)(EQLMS() _q,                                                \
                 T       _d,                                                \
                 T       _d_hat);                                           \
                                                                            \
/* Step through one cycle of equalizer training (blind)                 */  \
/*  _q      :   equalizer object                                        */  \
/*  _d_hat  :   actual output                                           */  \
int EQLMS(_step_blind)(EQLMS() _q,                                          \
                       T       _d_hat);                                     \
                                                                            \
/* Train equalizer object on group of samples                           */  \
/*  _q      :   equalizer object                                        */  \
/*  _w      :   input/output weights,  [size: _p x 1]                   */  \
/*  _x      :   received sample vector,[size: _n x 1]                   */  \
/*  _d      :   desired output vector, [size: _n x 1]                   */  \
/*  _n      :   input, output vector length                             */  \
DEPRECATED("method provides complexity with little benefit",                \
int EQLMS(_train)(EQLMS()      _q,                                          \
                  T *          _w,                                          \
                  T *          _x,                                          \
                  T *          _d,                                          \
                  unsigned int _n);                                         \
)                                                                           \

LIQUID_EQLMS_DEFINE_API(LIQUID_EQLMS_MANGLE_RRRF, float)
LIQUID_EQLMS_DEFINE_API(LIQUID_EQLMS_MANGLE_CCCF, liquid_float_complex)


// recursive least-squares (RLS)
#define LIQUID_EQRLS_MANGLE_RRRF(name) LIQUID_CONCAT(eqrls_rrrf,name)
#define LIQUID_EQRLS_MANGLE_CCCF(name) LIQUID_CONCAT(eqrls_cccf,name)

// large macro
//   EQRLS  : name-mangling macro
//   T      : data type
#define LIQUID_EQRLS_DEFINE_API(EQRLS,T)                                    \
                                                                            \
/* Recursive least mean-squares equalization object                     */  \
typedef struct EQRLS(_s) * EQRLS();                                         \
                                                                            \
/* Create RLS EQ initialized with external coefficients                 */  \
/*  _h : filter coefficients; set to NULL for {1,0,0...},[size: _n x 1] */  \
/*  _n : filter length                                                  */  \
EQRLS() EQRLS(_create)(T *          _h,                                     \
                       unsigned int _n);                                    \
                                                                            \
/* Re-create EQ initialized with external coefficients                  */  \
/*  _q :   equalizer object                                             */  \
/*  _h :   filter coefficients (NULL for {1,0,0...}), [size: _n x 1]    */  \
/*  _n :   filter length                                                */  \
EQRLS() EQRLS(_recreate)(EQRLS()      _q,                                   \
                         T *          _h,                                   \
                         unsigned int _n);                                  \
                                                                            \
/* Copy object including all internal objects and state                 */  \
EQRLS() EQRLS(_copy)(EQRLS() _q);                                           \
                                                                            \
/* Destroy equalizer object, freeing all internal memory                */  \
int EQRLS(_destroy)(EQRLS() _q);                                            \
                                                                            \
/* Reset equalizer object, clearing internal state                      */  \
int EQRLS(_reset)(EQRLS() _q);                                              \
                                                                            \
/* Print equalizer internal state                                       */  \
int EQRLS(_print)(EQRLS() _q);                                              \
                                                                            \
/* Get equalizer learning rate                                          */  \
float EQRLS(_get_bw)(EQRLS() _q);                                           \
                                                                            \
/* Set equalizer learning rate                                          */  \
/*  _q  :   equalizer object                                            */  \
/*  _mu :   learning rate, _mu > 0                                      */  \
int EQRLS(_set_bw)(EQRLS() _q,                                              \
                   float   _mu);                                            \
                                                                            \
/* Push sample into equalizer internal buffer                           */  \
/*  _q      :   equalizer object                                        */  \
/*  _x      :   input sample                                            */  \
int EQRLS(_push)(EQRLS() _q, T _x);                                         \
                                                                            \
/* Execute internal dot product and return result                       */  \
/*  _q      :   equalizer object                                        */  \
/*  _y      :   output sample                                           */  \
int EQRLS(_execute)(EQRLS() _q, T * _y);                                    \
                                                                            \
/* Step through one cycle of equalizer training                         */  \
/*  _q      :   equalizer object                                        */  \
/*  _d      :   desired output                                          */  \
/*  _d_hat  :   actual output                                           */  \
int EQRLS(_step)(EQRLS() _q, T _d, T _d_hat);                               \
                                                                            \
/* Get equalizer's internal coefficients                                */  \
/*  _q      :   equalizer object                                        */  \
/*  _w      :   weights, [size: _p x 1]                                 */  \
int EQRLS(_get_weights)(EQRLS() _q,                                         \
                        T *     _w);                                        \
                                                                            \
/* Train equalizer object on group of samples                           */  \
/*  _q      :   equalizer object                                        */  \
/*  _w      :   input/output weights,  [size: _p x 1]                   */  \
/*  _x      :   received sample vector,[size: _n x 1]                   */  \
/*  _d      :   desired output vector, [size: _n x 1]                   */  \
/*  _n      :   input, output vector length                             */  \
int EQRLS(_train)(EQRLS()      _q,                                          \
                  T *          _w,                                          \
                  T *          _x,                                          \
                  T *          _d,                                          \
                  unsigned int _n);                                         \

LIQUID_EQRLS_DEFINE_API(LIQUID_EQRLS_MANGLE_RRRF, float)
LIQUID_EQRLS_DEFINE_API(LIQUID_EQRLS_MANGLE_CCCF, liquid_float_complex)




//
// MODULE : fec (forward error-correction)
//

// soft bit values
#define LIQUID_SOFTBIT_0        (0)
#define LIQUID_SOFTBIT_1        (255)
#define LIQUID_SOFTBIT_ERASURE  (127)

// available CRC schemes
#define LIQUID_CRC_NUM_SCHEMES  7
typedef enum {
    LIQUID_CRC_UNKNOWN=0,   // unknown/unavailable CRC scheme
    LIQUID_CRC_NONE,        // no error-detection
    LIQUID_CRC_CHECKSUM,    // 8-bit checksum
    LIQUID_CRC_8,           // 8-bit CRC
    LIQUID_CRC_16,          // 16-bit CRC
    LIQUID_CRC_24,          // 24-bit CRC
    LIQUID_CRC_32           // 32-bit CRC
} crc_scheme;

// pretty names for crc schemes
extern const char * crc_scheme_str[LIQUID_CRC_NUM_SCHEMES][2];

// Print compact list of existing and available CRC schemes
int liquid_print_crc_schemes();

// returns crc_scheme based on input string
crc_scheme liquid_getopt_str2crc(const char * _str);

// get length of CRC (bytes)
unsigned int crc_get_length(crc_scheme _scheme);

// generate error-detection key
//  _scheme     :   error-detection scheme
//  _msg        :   input data message, [size: _n x 1]
//  _n          :   input data message size
unsigned int crc_generate_key(crc_scheme      _scheme,
                              unsigned char * _msg,
                              unsigned int    _n);

// generate error-detection key and append to end of message
//  _scheme     :   error-detection scheme (resulting in 'p' bytes)
//  _msg        :   input data message, [size: _n+p x 1]
//  _n          :   input data message size (excluding key at end)
int crc_append_key(crc_scheme      _scheme,
                   unsigned char * _msg,
                   unsigned int    _n);

// validate message using error-detection key
//  _scheme     :   error-detection scheme
//  _msg        :   input data message, [size: _n x 1]
//  _n          :   input data message size
//  _key        :   error-detection key
int crc_validate_message(crc_scheme      _scheme,
                         unsigned char * _msg,
                         unsigned int    _n,
                         unsigned int    _key);

// check message with key appended to end of array
//  _scheme     :   error-detection scheme (resulting in 'p' bytes)
//  _msg        :   input data message, [size: _n+p x 1]
//  _n          :   input data message size (excluding key at end)
int crc_check_key(crc_scheme      _scheme,
                  unsigned char * _msg,
                  unsigned int    _n);

// get size of key (bytes)
unsigned int crc_sizeof_key(crc_scheme _scheme);


// available FEC schemes
#define LIQUID_FEC_NUM_SCHEMES  28
typedef enum {
    LIQUID_FEC_UNKNOWN=0,       // unknown/unsupported scheme
    LIQUID_FEC_NONE,            // no error-correction
    LIQUID_FEC_REP3,            // simple repeat code, r1/3
    LIQUID_FEC_REP5,            // simple repeat code, r1/5
    LIQUID_FEC_HAMMING74,       // Hamming (7,4) block code, r1/2 (really 4/7)
    LIQUID_FEC_HAMMING84,       // Hamming (7,4) with extra parity bit, r1/2
    LIQUID_FEC_HAMMING128,      // Hamming (12,8) block code, r2/3

    LIQUID_FEC_GOLAY2412,       // Golay (24,12) block code, r1/2
    LIQUID_FEC_SECDED2216,      // SEC-DED (22,16) block code, r8/11
    LIQUID_FEC_SECDED3932,      // SEC-DED (39,32) block code
    LIQUID_FEC_SECDED7264,      // SEC-DED (72,64) block code, r8/9

    // codecs not defined internally (see http://www.ka9q.net/code/fec/)
    LIQUID_FEC_CONV_V27,        // r1/2, K=7, dfree=10
    LIQUID_FEC_CONV_V29,        // r1/2, K=9, dfree=12
    LIQUID_FEC_CONV_V39,        // r1/3, K=9, dfree=18
    LIQUID_FEC_CONV_V615,       // r1/6, K=15, dfree<=57 (Heller 1968)

    // punctured (perforated) codes
    LIQUID_FEC_CONV_V27P23,     // r2/3, K=7, dfree=6
    LIQUID_FEC_CONV_V27P34,     // r3/4, K=7, dfree=5
    LIQUID_FEC_CONV_V27P45,     // r4/5, K=7, dfree=4
    LIQUID_FEC_CONV_V27P56,     // r5/6, K=7, dfree=4
    LIQUID_FEC_CONV_V27P67,     // r6/7, K=7, dfree=3
    LIQUID_FEC_CONV_V27P78,     // r7/8, K=7, dfree=3

    LIQUID_FEC_CONV_V29P23,     // r2/3, K=9, dfree=7
    LIQUID_FEC_CONV_V29P34,     // r3/4, K=9, dfree=6
    LIQUID_FEC_CONV_V29P45,     // r4/5, K=9, dfree=5
    LIQUID_FEC_CONV_V29P56,     // r5/6, K=9, dfree=5
    LIQUID_FEC_CONV_V29P67,     // r6/7, K=9, dfree=4
    LIQUID_FEC_CONV_V29P78,     // r7/8, K=9, dfree=4

    // Reed-Solomon codes
    LIQUID_FEC_RS_M8            // m=8, n=255, k=223
} fec_scheme;

// pretty names for fec schemes
extern const char * fec_scheme_str[LIQUID_FEC_NUM_SCHEMES][2];

// Print compact list of existing and available FEC schemes
int liquid_print_fec_schemes();

// returns fec_scheme based on input string
fec_scheme liquid_getopt_str2fec(const char * _str);

// fec object (pointer to fec structure)
typedef struct fec_s * fec;

// return the encoded message length using a particular error-
// correction scheme (object-independent method)
//  _scheme     :   forward error-correction scheme
//  _msg_len    :   raw, uncoded message length
unsigned int fec_get_enc_msg_length(fec_scheme _scheme,
                                    unsigned int _msg_len);

// get the theoretical rate of a particular forward error-
// correction scheme (object-independent method)
float fec_get_rate(fec_scheme _scheme);

// create a fec object of a particular scheme
//  _scheme     :   error-correction scheme
//  _opts       :   (ignored)
fec fec_create(fec_scheme _scheme,
               void *_opts);

// recreate fec object
//  _q          :   old fec object
//  _scheme     :   new error-correction scheme
//  _opts       :   (ignored)
fec fec_recreate(fec _q,
                 fec_scheme _scheme,
                 void *_opts);

// Copy object including all internal objects and state
fec fec_copy(fec _q);

// destroy fec object
int fec_destroy(fec _q);

// print fec object internals
int fec_print(fec _q);

// encode a block of data using a fec scheme
//  _q              :   fec object
//  _dec_msg_len    :   decoded message length
//  _msg_dec        :   decoded message
//  _msg_enc        :   encoded message
int fec_encode(fec _q,
               unsigned int _dec_msg_len,
               unsigned char * _msg_dec,
               unsigned char * _msg_enc);

// decode a block of data using a fec scheme
//  _q              :   fec object
//  _dec_msg_len    :   decoded message length
//  _msg_enc        :   encoded message
//  _msg_dec        :   decoded message
int fec_decode(fec _q,
               unsigned int _dec_msg_len,
               unsigned char * _msg_enc,
               unsigned char * _msg_dec);

// decode a block of data using a fec scheme (soft decision)
//  _q              :   fec object
//  _dec_msg_len    :   decoded message length
//  _msg_enc        :   encoded message (soft bits)
//  _msg_dec        :   decoded message
int fec_decode_soft(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_enc,
                    unsigned char * _msg_dec);

//
// Packetizer
//

// computes the number of encoded bytes after packetizing
//
//  _n      :   number of uncoded input bytes
//  _crc    :   error-detecting scheme
//  _fec0   :   inner forward error-correction code
//  _fec1   :   outer forward error-correction code
unsigned int packetizer_compute_enc_msg_len(unsigned int _n,
                                            int _crc,
                                            int _fec0,
                                            int _fec1);

// computes the number of decoded bytes before packetizing
//
//  _k      :   number of encoded bytes
//  _crc    :   error-detecting scheme
//  _fec0   :   inner forward error-correction code
//  _fec1   :   outer forward error-correction code
unsigned int packetizer_compute_dec_msg_len(unsigned int _k,
                                            int _crc,
                                            int _fec0,
                                            int _fec1);

typedef struct packetizer_s * packetizer;

// create packetizer object
//
//  _n      :   number of uncoded input bytes
//  _crc    :   error-detecting scheme
//  _fec0   :   inner forward error-correction code
//  _fec1   :   outer forward error-correction code
packetizer packetizer_create(unsigned int _dec_msg_len,
                             int _crc,
                             int _fec0,
                             int _fec1);

// re-create packetizer object
//
//  _p      :   initialz packetizer object
//  _n      :   number of uncoded input bytes
//  _crc    :   error-detecting scheme
//  _fec0   :   inner forward error-correction code
//  _fec1   :   outer forward error-correction code
packetizer packetizer_recreate(packetizer _p,
                               unsigned int _dec_msg_len,
                               int _crc,
                               int _fec0,
                               int _fec1);

// Copy object including all internal objects and state
packetizer packetizer_copy(packetizer _p);

// destroy packetizer object
int packetizer_destroy(packetizer _p);

// print packetizer object internals
int packetizer_print(packetizer _p);

// access methods
unsigned int packetizer_get_dec_msg_len(packetizer _p);
unsigned int packetizer_get_enc_msg_len(packetizer _p);
crc_scheme   packetizer_get_crc        (packetizer _p);
fec_scheme   packetizer_get_fec0       (packetizer _p);
fec_scheme   packetizer_get_fec1       (packetizer _p);


// Execute the packetizer on an input message
//
//  _p      :   packetizer object
//  _msg    :   input message (uncoded bytes)
//  _pkt    :   encoded output message
int packetizer_encode(packetizer            _p,
                       const unsigned char * _msg,
                       unsigned char *       _pkt);

// Execute the packetizer to decode an input message, return validity
// check of resulting data
//
//  _p      :   packetizer object
//  _pkt    :   input message (coded bytes)
//  _msg    :   decoded output message
int packetizer_decode(packetizer            _p,
                      const unsigned char * _pkt,
                      unsigned char *       _msg);

// Execute the packetizer to decode an input message, return validity
// check of resulting data
//
//  _p      :   packetizer object
//  _pkt    :   input message (coded soft bits)
//  _msg    :   decoded output message
int packetizer_decode_soft(packetizer            _p,
                           const unsigned char * _pkt,
                           unsigned char *       _msg);


//
// interleaver
//
typedef struct interleaver_s * interleaver;

// create interleaver
//   _n     : number of bytes
interleaver interleaver_create(unsigned int _n);

// Copy object including all internal objects and state
interleaver linterleaver_copy(interleaver _q);

// destroy interleaver object
int interleaver_destroy(interleaver _q);

// print interleaver object internals
int interleaver_print(interleaver _q);

// set depth (number of internal iterations)
//  _q      :   interleaver object
//  _depth  :   depth
int interleaver_set_depth(interleaver  _q,
                          unsigned int _depth);

// execute forward interleaver (encoder)
//  _q          :   interleaver object
//  _msg_dec    :   decoded (un-interleaved) message
//  _msg_enc    :   encoded (interleaved) message
int interleaver_encode(interleaver     _q,
                       unsigned char * _msg_dec,
                       unsigned char * _msg_enc);

// execute forward interleaver (encoder) on soft bits
//  _q          :   interleaver object
//  _msg_dec    :   decoded (un-interleaved) message
//  _msg_enc    :   encoded (interleaved) message
int interleaver_encode_soft(interleaver     _q,
                            unsigned char * _msg_dec,
                            unsigned char * _msg_enc);

// execute reverse interleaver (decoder)
//  _q          :   interleaver object
//  _msg_enc    :   encoded (interleaved) message
//  _msg_dec    :   decoded (un-interleaved) message
int interleaver_decode(interleaver     _q,
                       unsigned char * _msg_enc,
                       unsigned char * _msg_dec);

// execute reverse interleaver (decoder) on soft bits
//  _q          :   interleaver object
//  _msg_enc    :   encoded (interleaved) message
//  _msg_dec    :   decoded (un-interleaved) message
int interleaver_decode_soft(interleaver     _q,
                            unsigned char * _msg_enc,
                            unsigned char * _msg_dec);



//
// MODULE : fft (fast Fourier transform)
//

// type of transform
typedef enum {
    LIQUID_FFT_UNKNOWN  =   0,  // unknown transform type

    // regular complex one-dimensional transforms
    LIQUID_FFT_FORWARD  =  +1,  // complex one-dimensional FFT
    LIQUID_FFT_BACKWARD =  -1,  // complex one-dimensional inverse FFT

    // discrete cosine transforms
    LIQUID_FFT_REDFT00  =  10,  // real one-dimensional DCT-I
    LIQUID_FFT_REDFT10  =  11,  // real one-dimensional DCT-II
    LIQUID_FFT_REDFT01  =  12,  // real one-dimensional DCT-III
    LIQUID_FFT_REDFT11  =  13,  // real one-dimensional DCT-IV

    // discrete sine transforms
    LIQUID_FFT_RODFT00  =  20,  // real one-dimensional DST-I
    LIQUID_FFT_RODFT10  =  21,  // real one-dimensional DST-II
    LIQUID_FFT_RODFT01  =  22,  // real one-dimensional DST-III
    LIQUID_FFT_RODFT11  =  23,  // real one-dimensional DST-IV

    // modified discrete cosine transform
    LIQUID_FFT_MDCT     =  30,  // MDCT
    LIQUID_FFT_IMDCT    =  31,  // IMDCT
} liquid_fft_type;

#define LIQUID_FFT_MANGLE_FLOAT(name) LIQUID_CONCAT(fft,name)

// Macro    :   FFT
//  FFT     :   name-mangling macro
//  T       :   primitive data type
//  TC      :   primitive data type (complex)
#define LIQUID_FFT_DEFINE_API(FFT,T,TC)                                     \
                                                                            \
/* Fast Fourier Transform (FFT) and inverse (plan) object               */  \
typedef struct FFT(plan_s) * FFT(plan);                                     \
                                                                            \
/* Allocate a one-dimensional array similar to the ordinary malloc. The */  \
/* implementation may internally align the allocated memory to support  */  \
/* some optimizations. Use the result as the input or output array      */  \
/* argument to one of the fft_create* methods. As with the ordinary     */  \
/* malloc, the result must be typecast to the proper type. Memory       */  \
/* allocated by this function must be deallocated by fft_free and not   */  \
/* by the ordinary free.                                                */  \
/*  _n      :   array size                                              */  \
void * FFT(_malloc)(unsigned int _n);                                       \
                                                                            \
/* Free the one-dimensional array allocated by fft_malloc.              */  \
/*  _x      :   pointer to array                                        */  \
void FFT(_free)(void * _x);                                                 \
                                                                            \
/* Create regular complex one-dimensional transform                     */  \
/*  _n      :   transform size                                          */  \
/*  _x      :   pointer to input array,  [size: _n x 1]                 */  \
/*  _y      :   pointer to output array, [size: _n x 1]                 */  \
/*  _dir    :   direction (e.g. LIQUID_FFT_FORWARD)                     */  \
/*  _flags  :   options, optimization                                   */  \
FFT(plan) FFT(_create_plan)(unsigned int _n,                                \
                            TC *         _x,                                \
                            TC *         _y,                                \
                            int          _dir,                              \
                            int          _flags);                           \
                                                                            \
/* Create real-to-real one-dimensional transform                        */  \
/*  _n      :   transform size                                          */  \
/*  _x      :   pointer to input array,  [size: _n x 1]                 */  \
/*  _y      :   pointer to output array, [size: _n x 1]                 */  \
/*  _type   :   transform type (e.g. LIQUID_FFT_REDFT00)                */  \
/*  _flags  :   options, optimization                                   */  \
FFT(plan) FFT(_create_plan_r2r_1d)(unsigned int _n,                         \
                                   T *          _x,                         \
                                   T *          _y,                         \
                                   int          _type,                      \
                                   int          _flags);                    \
                                                                            \
/* Destroy transform and free all internally-allocated memory           */  \
int FFT(_destroy_plan)(FFT(plan) _p);                                       \
                                                                            \
/* Print transform plan and internal strategy to stdout. This includes  */  \
/* information on the strategy for computing large transforms with many */  \
/* prime factors or with large prime factors.                           */  \
int FFT(_print_plan)(FFT(plan) _p);                                         \
                                                                            \
/* Run the transform                                                    */  \
int FFT(_execute)(FFT(plan) _p);                                            \
                                                                            \
/* Perform n-point FFT allocating plan internally                       */  \
/*  _nfft   : fft size                                                  */  \
/*  _x      : input array, [size: _nfft x 1]                            */  \
/*  _y      : output array, [size: _nfft x 1]                           */  \
/*  _dir    : fft direction: LIQUID_FFT_{FORWARD,BACKWARD}              */  \
/*  _flags  : fft flags                                                 */  \
int FFT(_run)(unsigned int _n,                                              \
              TC *         _x,                                              \
              TC *         _y,                                              \
              int          _dir,                                            \
              int          _flags);                                         \
                                                                            \
/* Perform n-point real one-dimensional FFT allocating plan internally  */  \
/*  _nfft   : fft size                                                  */  \
/*  _x      : input array, [size: _nfft x 1]                            */  \
/*  _y      : output array, [size: _nfft x 1]                           */  \
/*  _type   : fft type, e.g. LIQUID_FFT_REDFT10                         */  \
/*  _flags  : fft flags                                                 */  \
int FFT(_r2r_1d_run)(unsigned int _n,                                       \
                     T *          _x,                                       \
                     T *          _y,                                       \
                     int          _type,                                    \
                     int          _flags);                                  \
                                                                            \
/* Perform _n-point fft shift                                           */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : input array size                                          */  \
int FFT(_shift)(TC *         _x,                                            \
                unsigned int _n);                                           \


LIQUID_FFT_DEFINE_API(LIQUID_FFT_MANGLE_FLOAT,float,liquid_float_complex)

// If using fftw library (and not overridden with configuration), clean up
// planner's persistent data. This must be done outside any methods as fftw
// uses *static memory* in its planner which cannot be cleaned up if any
// other function or method in the calling process creates or uses an fftw
// plan.
int liquid_fftwf_cleanup_wrapper(void);


// antiquated fft methods
// FFT(plan) FFT(_create_plan_mdct)(unsigned int _n,
//                                  T * _x,
//                                  T * _y,
//                                  int _kind,
//                                  int _flags);


//
// spectral periodogram
//

#define LIQUID_SPGRAM_MANGLE_CFLOAT(name) LIQUID_CONCAT(spgramcf,name)
#define LIQUID_SPGRAM_MANGLE_FLOAT(name)  LIQUID_CONCAT(spgramf, name)

#define LIQUID_SPGRAM_PSD_MIN (1e-12)

// Macro    :   SPGRAM
//  SPGRAM  :   name-mangling macro
//  T       :   primitive data type
//  TC      :   primitive data type (complex)
//  TI      :   primitive data type (input)
#define LIQUID_SPGRAM_DEFINE_API(SPGRAM,T,TC,TI)                            \
                                                                            \
/* Spectral periodogram object for computing power spectral density     */  \
/* estimates of various signals                                         */  \
typedef struct SPGRAM(_s) * SPGRAM();                                       \
                                                                            \
/* Create spgram object, fully defined                                  */  \
/*  _nfft       : transform (FFT) size, _nfft >= 2                      */  \
/*  _wtype      : window type, e.g. LIQUID_WINDOW_HAMMING               */  \
/*  _window_len : window length, 1 <= _window_len <= _nfft              */  \
/*  _delay      : delay between transforms, _delay > 0                  */  \
SPGRAM() SPGRAM(_create)(unsigned int _nfft,                                \
                         int          _wtype,                               \
                         unsigned int _window_len,                          \
                         unsigned int _delay);                              \
                                                                            \
/* Create default spgram object of a particular transform size using    */  \
/* the Kaiser-Bessel window (LIQUID_WINDOW_KAISER), a window length     */  \
/* equal to _nfft/2, and a delay of _nfft/4                             */  \
/*  _nfft       : FFT size, _nfft >= 2                                  */  \
SPGRAM() SPGRAM(_create_default)(unsigned int _nfft);                       \
                                                                            \
/* Copy object including all internal objects and state                 */  \
SPGRAM() SPGRAM(_copy)(SPGRAM() _q);                                        \
                                                                            \
/* Destroy spgram object, freeing all internally-allocated memory       */  \
int SPGRAM(_destroy)(SPGRAM() _q);                                          \
                                                                            \
/* Clears the internal state of the object, but not the internal buffer */  \
int SPGRAM(_clear)(SPGRAM() _q);                                            \
                                                                            \
/* Reset the object to its original state completely. This effectively  */  \
/* executes the clear() method and then resets the internal buffer      */  \
int SPGRAM(_reset)(SPGRAM() _q);                                            \
                                                                            \
/* Print internal state of the object to stdout                         */  \
int SPGRAM(_print)(SPGRAM() _q);                                            \
                                                                            \
/* Set the filter bandwidth for accumulating independent transform      */  \
/* squared magnitude outputs.                                           */  \
/* This is used to compute a running time-average power spectral        */  \
/* density output.                                                      */  \
/* The value of _alpha determines how the power spectral estimate is    */  \
/* accumulated across transforms and can range from 0 to 1 with a       */  \
/* special case of -1 to accumulate infinitely.                         */  \
/* Setting _alpha to 0 minimizes the bandwidth and the PSD estimate     */  \
/* will never update.                                                   */  \
/* Setting _alpha to 1 forces the object to always use the most recent  */  \
/* spectral estimate.                                                   */  \
/* Setting _alpha to -1 is a special case to enable infinite spectral   */  \
/* accumulation.                                                        */  \
/*  _q      : spectral periodogram object                               */  \
/*  _alpha  : forgetting factor, set to -1 for infinite, 0<=_alpha<=1   */  \
int SPGRAM(_set_alpha)(SPGRAM() _q,                                         \
                       float    _alpha);                                    \
                                                                            \
/* Get the filter bandwidth for accumulating independent transform      */  \
/* squared magnitude outputs.                                           */  \
float SPGRAM(_get_alpha)(SPGRAM() _q);                                      \
                                                                            \
/* Set the center frequency of the received signal.                     */  \
/* This is for display purposes only when generating the output image.  */  \
/*  _q      : spectral periodogram object                               */  \
/*  _freq   : center frequency [Hz]                                     */  \
int SPGRAM(_set_freq)(SPGRAM() _q,                                          \
                      float    _freq);                                      \
                                                                            \
/* Set the sample rate (frequency) of the received signal.              */  \
/* This is for display purposes only when generating the output image.  */  \
/*  _q      : spectral periodogram object                               */  \
/*  _rate   : sample rate [Hz]                                          */  \
int SPGRAM(_set_rate)(SPGRAM() _q,                                          \
                      float    _rate);                                      \
                                                                            \
/* Get transform (FFT) size                                             */  \
unsigned int SPGRAM(_get_nfft)(SPGRAM() _q);                                \
                                                                            \
/* Get window length                                                    */  \
unsigned int SPGRAM(_get_window_len)(SPGRAM() _q);                          \
                                                                            \
/* Get delay between transforms                                         */  \
unsigned int SPGRAM(_get_delay)(SPGRAM() _q);                               \
                                                                            \
/* Get window type used for spectral estimation                         */  \
int SPGRAM(_get_wtype)(SPGRAM() _q);                                        \
                                                                            \
/* Get number of samples processed since reset                          */  \
unsigned long long int SPGRAM(_get_num_samples)(SPGRAM() _q);               \
                                                                            \
/* Get number of samples processed since object was created             */  \
unsigned long long int SPGRAM(_get_num_samples_total)(SPGRAM() _q);         \
                                                                            \
/* Get number of transforms processed since reset                       */  \
unsigned long long int SPGRAM(_get_num_transforms)(SPGRAM() _q);            \
                                                                            \
/* Get number of transforms processed since object was created          */  \
unsigned long long int SPGRAM(_get_num_transforms_total)(SPGRAM() _q);      \
                                                                            \
/* Push a single sample into the object, executing internal transform   */  \
/* as necessary.                                                        */  \
/*  _q  : spgram object                                                 */  \
/*  _x  : input sample                                                  */  \
int SPGRAM(_push)(SPGRAM() _q,                                              \
                  TI       _x);                                             \
                                                                            \
/* Write a block of samples to the object, executing internal           */  \
/* transform as necessary.                                              */  \
/*  _q  : spgram object                                                 */  \
/*  _x  : input buffer, [size: _n x 1]                                  */  \
/*  _n  : input buffer length                                           */  \
int SPGRAM(_write)(SPGRAM()     _q,                                         \
                   TI *         _x,                                         \
                   unsigned int _n);                                        \
                                                                            \
/* Compute spectral periodogram output (fft-shifted values, linear)     */  \
/* from current buffer contents                                         */  \
/*  _q   : spgram object                                                */  \
/*  _psd : output spectrum (linear), [size: _nfft x 1]                  */  \
int SPGRAM(_get_psd_mag)(SPGRAM() _q,                                       \
                         T *      _psd);                                    \
                                                                            \
/* Compute spectral periodogram output (fft-shifted values in dB) from  */  \
/* current buffer contents                                              */  \
/*  _q   : spgram object                                                */  \
/*  _psd : output spectrum (dB), [size: _nfft x 1]                      */  \
int SPGRAM(_get_psd)(SPGRAM() _q,                                           \
                     T *      _psd);                                        \
                                                                            \
/* Export stand-alone gnuplot file for plotting output spectrum,        */  \
/* returning 0 on success, anything other than 0 for failure            */  \
/*  _q        : spgram object                                           */  \
/*  _filename : input buffer, [size: _n x 1]                            */  \
int SPGRAM(_export_gnuplot)(SPGRAM()     _q,                                \
                            const char * _filename);                        \
                                                                            \
/* Estimate spectrum on input signal (create temporary object for       */  \
/* convenience                                                          */  \
/*  _nfft   : FFT size                                                  */  \
/*  _x      : input signal, [size: _n x 1]                              */  \
/*  _n      : input signal length                                       */  \
/*  _psd    : output spectrum, [size: _nfft x 1]                        */  \
int SPGRAM(_estimate_psd)(unsigned int _nfft,                               \
                          TI *         _x,                                  \
                          unsigned int _n,                                  \
                          T *          _psd);                               \

LIQUID_SPGRAM_DEFINE_API(LIQUID_SPGRAM_MANGLE_CFLOAT,
                         float,
                         liquid_float_complex,
                         liquid_float_complex)

LIQUID_SPGRAM_DEFINE_API(LIQUID_SPGRAM_MANGLE_FLOAT,
                         float,
                         liquid_float_complex,
                         float)

//
// asgram : ascii spectral periodogram
//

#define LIQUID_ASGRAM_MANGLE_CFLOAT(name) LIQUID_CONCAT(asgramcf,name)
#define LIQUID_ASGRAM_MANGLE_FLOAT(name)  LIQUID_CONCAT(asgramf, name)

// Macro    : ASGRAM
//  ASGRAM  : name-mangling macro
//  T       : primitive data type
//  TC      : primitive data type (complex)
//  TI      : primitive data type (input)
#define LIQUID_ASGRAM_DEFINE_API(ASGRAM,T,TC,TI)                            \
                                                                            \
/* ASCII spectral periodogram for computing and displaying an estimate  */  \
/* of a signal's power spectrum with ASCII characters                   */  \
typedef struct ASGRAM(_s) * ASGRAM();                                       \
                                                                            \
/* Create asgram object with size _nfft                                 */  \
/*  _nfft   : size of FFT taken for each transform (character width)    */  \
ASGRAM() ASGRAM(_create)(unsigned int _nfft);                               \
                                                                            \
/* Copy object including all internal objects and state                 */  \
ASGRAM() ASGRAM(_copy)(ASGRAM() _q);                                        \
                                                                            \
/* Destroy asgram object, freeing all internally-allocated memory       */  \
int ASGRAM(_destroy)(ASGRAM() _q);                                          \
                                                                            \
/* Reset the internal state of the asgram object                        */  \
int ASGRAM(_reset)(ASGRAM() _q);                                            \
                                                                            \
/* Set the scale and offset for spectrogram in terms of dB for display  */  \
/* purposes                                                             */  \
/*  _q      : asgram object                                             */  \
/*  _ref    : signal reference level [dB]                               */  \
/*  _div    : signal division [dB]                                      */  \
int ASGRAM(_set_scale)(ASGRAM() _q,                                         \
                       float    _ref,                                       \
                       float    _div);                                      \
                                                                            \
/* Set the display's 10 characters for output string starting from the  */  \
/* weakest and ending with the strongest                                */  \
/*  _q      : asgram object                                             */  \
/*  _ascii  : 10-character display, default: " .,-+*&NM#"               */  \
int ASGRAM(_set_display)(ASGRAM()     _q,                                   \
                         const char * _ascii);                              \
                                                                            \
/* Push a single sample into the asgram object, executing internal      */  \
/* transform as necessary.                                              */  \
/*  _q  : asgram object                                                 */  \
/*  _x  : input sample                                                  */  \
int ASGRAM(_push)(ASGRAM() _q,                                              \
                  TI       _x);                                             \
                                                                            \
/* Write a block of samples to the asgram object, executing internal    */  \
/* transforms as necessary.                                             */  \
/*  _q  : asgram object                                                 */  \
/*  _x  : input buffer, [size: _n x 1]                                  */  \
/*  _n  : input buffer length                                           */  \
int ASGRAM(_write)(ASGRAM()     _q,                                         \
                   TI *         _x,                                         \
                   unsigned int _n);                                        \
                                                                            \
/* Compute spectral periodogram output from current buffer contents     */  \
/* and return the ascii character string to display along with the peak */  \
/* value and its frequency location                                     */  \
/*  _q          : asgram object                                         */  \
/*  _ascii      : output ASCII string, [size: _nfft x 1]                */  \
/*  _peakval    : peak power spectral density value [dB]                */  \
/*  _peakfreq   : peak power spectral density frequency                 */  \
int ASGRAM(_execute)(ASGRAM() _q,                                           \
                     char *   _ascii,                                       \
                     float *  _peakval,                                     \
                     float *  _peakfreq);                                   \
                                                                            \
/* Compute spectral periodogram output from current buffer contents and */  \
/* print standard format to stdout                                      */  \
int ASGRAM(_print)(ASGRAM() _q);                                            \

LIQUID_ASGRAM_DEFINE_API(LIQUID_ASGRAM_MANGLE_CFLOAT,
                         float,
                         liquid_float_complex,
                         liquid_float_complex)

LIQUID_ASGRAM_DEFINE_API(LIQUID_ASGRAM_MANGLE_FLOAT,
                         float,
                         liquid_float_complex,
                         float)

//
// spectral periodogram waterfall
//

#define LIQUID_SPWATERFALL_MANGLE_CFLOAT(name) LIQUID_CONCAT(spwaterfallcf,name)
#define LIQUID_SPWATERFALL_MANGLE_FLOAT(name)  LIQUID_CONCAT(spwaterfallf, name)

// Macro        :   SPWATERFALL
//  SPWATERFALL :   name-mangling macro
//  T           :   primitive data type
//  TC          :   primitive data type (complex)
//  TI          :   primitive data type (input)
#define LIQUID_SPWATERFALL_DEFINE_API(SPWATERFALL,T,TC,TI)                  \
                                                                            \
/* Spectral periodogram waterfall object for computing time-varying     */  \
/* power spectral density estimates                                     */  \
typedef struct SPWATERFALL(_s) * SPWATERFALL();                             \
                                                                            \
/* Create spwaterfall object, fully defined                             */  \
/*  _nfft       : transform (FFT) size, _nfft >= 2                      */  \
/*  _wtype      : window type, e.g. LIQUID_WINDOW_HAMMING               */  \
/*  _window_len : window length, 1 <= _window_len <= _nfft              */  \
/*  _delay      : delay between transforms, _delay > 0                  */  \
/*  _time       : number of aggregated transforms, _time > 0            */  \
SPWATERFALL() SPWATERFALL(_create)(unsigned int _nfft,                      \
                                   int          _wtype,                     \
                                   unsigned int _window_len,                \
                                   unsigned int _delay,                     \
                                   unsigned int _time);                     \
                                                                            \
/* Create default spwatefall object (Kaiser-Bessel window)              */  \
/*  _nfft   : transform size, _nfft >= 2                                */  \
/*  _time   : delay between transforms, _delay > 0                      */  \
SPWATERFALL() SPWATERFALL(_create_default)(unsigned int _nfft,              \
                                           unsigned int _time);             \
                                                                            \
/* Copy object including all internal objects and state                 */  \
SPWATERFALL() SPWATERFALL(_copy)(SPWATERFALL() _q);                         \
                                                                            \
/* Destroy spwaterfall object, freeing all internally-allocated memory  */  \
int SPWATERFALL(_destroy)(SPWATERFALL() _q);                                \
                                                                            \
/* Clears the internal state of the object, but not the internal buffer */  \
int SPWATERFALL(_clear)(SPWATERFALL() _q);                                  \
                                                                            \
/* Reset the object to its original state completely. This effectively  */  \
/* executes the clear() method and then resets the internal buffer      */  \
int SPWATERFALL(_reset)(SPWATERFALL() _q);                                  \
                                                                            \
/* Print internal state of the object to stdout                         */  \
int SPWATERFALL(_print)(SPWATERFALL() _q);                                  \
                                                                            \
/* Get number of samples processed since object was created             */  \
uint64_t SPWATERFALL(_get_num_samples_total)(SPWATERFALL() _q);             \
                                                                            \
/* Get FFT size (columns in PSD output)                                 */  \
unsigned int SPWATERFALL(_get_num_freq)(SPWATERFALL() _q);                  \
                                                                            \
/* Get number of accumulated FFTs (rows in PSD output)                  */  \
unsigned int SPWATERFALL(_get_num_time)(SPWATERFALL() _q);                  \
                                                                            \
/* Get window length used in spectral estimation                        */  \
unsigned int SPWATERFALL(_get_window_len)(SPWATERFALL() _q);                \
                                                                            \
/* Get delay between transforms used in spectral estimation             */  \
unsigned int SPWATERFALL(_get_delay)(SPWATERFALL() _q);                     \
                                                                            \
/* Get window type used in spectral estimation                          */  \
int SPWATERFALL(_get_wtype)(SPWATERFALL() _q);                              \
                                                                            \
/* Get power spectral density (PSD), size: nfft x time                  */  \
const T * SPWATERFALL(_get_psd)(SPWATERFALL() _q);                          \
                                                                            \
/* Set the center frequency of the received signal.                     */  \
/* This is for display purposes only when generating the output image.  */  \
/*  _q      : spectral periodogram waterfall object                     */  \
/*  _freq   : center frequency [Hz]                                     */  \
int SPWATERFALL(_set_freq)(SPWATERFALL() _q,                                \
                           float         _freq);                            \
                                                                            \
/* Set the sample rate (frequency) of the received signal.              */  \
/* This is for display purposes only when generating the output image.  */  \
/*  _q      : spectral periodogram waterfall object                     */  \
/*  _rate   : sample rate [Hz]                                          */  \
int SPWATERFALL(_set_rate)(SPWATERFALL() _q,                                \
                           float         _rate);                            \
                                                                            \
/* Set the canvas size.                                                 */  \
/* This is for display purposes only when generating the output image.  */  \
/*  _q      : spectral periodogram waterfall object                     */  \
/*  _width  : image width [pixels]                                      */  \
/*  _height : image height [pixels]                                     */  \
int SPWATERFALL(_set_dims)(SPWATERFALL() _q,                                \
                           unsigned int  _width,                            \
                           unsigned int  _height);                          \
                                                                            \
/* Set commands for executing directly before 'plot' statement.         */  \
/*  _q          : spectral periodogram waterfall object                 */  \
/*  _commands   : gnuplot commands separated by semicolons              */  \
int SPWATERFALL(_set_commands)(SPWATERFALL() _q,                            \
                               const char *  _commands);                    \
                                                                            \
/* Push a single sample into the object, executing internal transform   */  \
/* as necessary.                                                        */  \
/*  _q  : spwaterfall object                                            */  \
/*  _x  : input sample                                                  */  \
int SPWATERFALL(_push)(SPWATERFALL() _q,                                    \
                       TI            _x);                                   \
                                                                            \
/* Write a block of samples to the object, executing internal           */  \
/* transform as necessary.                                              */  \
/*  _q  : spwaterfall object                                            */  \
/*  _x  : input buffer, [size: _n x 1]                                  */  \
/*  _n  : input buffer length                                           */  \
int SPWATERFALL(_write)(SPWATERFALL() _q,                                   \
                        TI *          _x,                                   \
                        unsigned int  _n);                                  \
                                                                            \
/* Export set of files for plotting                                     */  \
/*  _q    : spwaterfall object                                          */  \
/*  _base : base filename (will export .gnu, .bin, and .png files)      */  \
int SPWATERFALL(_export)(SPWATERFALL() _q,                                  \
                         const char *  _base);                              \


LIQUID_SPWATERFALL_DEFINE_API(LIQUID_SPWATERFALL_MANGLE_CFLOAT,
                              float,
                              liquid_float_complex,
                              liquid_float_complex)

LIQUID_SPWATERFALL_DEFINE_API(LIQUID_SPWATERFALL_MANGLE_FLOAT,
                              float,
                              liquid_float_complex,
                              float)


//
// MODULE : filter
//

//
// firdes: finite impulse response filter design
//

// prototypes
#define LIQUID_FIRFILT_NUM_TYPES (16)
typedef enum {
    LIQUID_FIRFILT_UNKNOWN=0,   // unknown filter type

    // Nyquist filter prototypes
    LIQUID_FIRFILT_KAISER,      // Nyquist Kaiser filter
    LIQUID_FIRFILT_PM,          // Parks-McClellan filter
    LIQUID_FIRFILT_RCOS,        // raised-cosine filter
    LIQUID_FIRFILT_FEXP,        // flipped exponential
    LIQUID_FIRFILT_FSECH,       // flipped hyperbolic secant
    LIQUID_FIRFILT_FARCSECH,    // flipped arc-hyperbolic secant

    // root-Nyquist filter prototypes
    LIQUID_FIRFILT_ARKAISER,    // root-Nyquist Kaiser (approximate optimum)
    LIQUID_FIRFILT_RKAISER,     // root-Nyquist Kaiser (true optimum)
    LIQUID_FIRFILT_RRC,         // root raised-cosine
    LIQUID_FIRFILT_hM3,         // harris-Moerder-3 filter
    LIQUID_FIRFILT_GMSKTX,      // GMSK transmit filter
    LIQUID_FIRFILT_GMSKRX,      // GMSK receive filter
    LIQUID_FIRFILT_RFEXP,       // flipped exponential
    LIQUID_FIRFILT_RFSECH,      // flipped hyperbolic secant
    LIQUID_FIRFILT_RFARCSECH,   // flipped arc-hyperbolic secant
} liquid_firfilt_type;

// Design (root-)Nyquist filter from prototype
//  _type   : filter type (e.g. LIQUID_FIRFILT_RRC)
//  _k      : samples/symbol,          _k > 1
//  _m      : symbol delay,            _m > 0
//  _beta   : excess bandwidth factor, _beta in [0,1)
//  _dt     : fractional sample delay, _dt in [-1,1]
//  _h      : output coefficient buffer (length: 2*_k*_m+1)
int liquid_firdes_prototype(liquid_firfilt_type _type,
                            unsigned int        _k,
                            unsigned int        _m,
                            float               _beta,
                            float               _dt,
                            float *             _h);

// pretty names for filter design types
extern const char * liquid_firfilt_type_str[LIQUID_FIRFILT_NUM_TYPES][2];

// returns filter type based on input string
int liquid_getopt_str2firfilt(const char * _str);

// estimate required filter length given
//  _df     :   transition bandwidth (0 < _b < 0.5)
//  _as     :   stop-band attenuation [dB], _as > 0
unsigned int estimate_req_filter_len(float _df,
                                     float _as);

// estimate filter stop-band attenuation given
//  _df     :   transition bandwidth (0 < _b < 0.5)
//  _n      :   filter length
float estimate_req_filter_As(float        _df,
                             unsigned int _n);

// estimate filter transition bandwidth given
//  _as     :   stop-band attenuation [dB], _as > 0
//  _n      :   filter length
float estimate_req_filter_df(float        _as,
                             unsigned int _n);


// returns the Kaiser window beta factor give the filter's target
// stop-band attenuation (As) [Vaidyanathan:1993]
//  _as     :   target filter's stop-band attenuation [dB], _as > 0
float kaiser_beta_As(float _as);


// Design FIR filter using Parks-McClellan algorithm

// band type specifier
typedef enum {
    LIQUID_FIRDESPM_BANDPASS=0,     // regular band-pass filter
    LIQUID_FIRDESPM_DIFFERENTIATOR, // differentiating filter
    LIQUID_FIRDESPM_HILBERT         // Hilbert transform
} liquid_firdespm_btype;

// weighting type specifier
typedef enum {
    LIQUID_FIRDESPM_FLATWEIGHT=0,   // flat weighting
    LIQUID_FIRDESPM_EXPWEIGHT,      // exponential weighting
    LIQUID_FIRDESPM_LINWEIGHT,      // linear weighting
} liquid_firdespm_wtype;

// run filter design (full life cycle of object)
//  _h_len      :   length of filter (number of taps)
//  _num_bands  :   number of frequency bands
//  _bands      :   band edges, f in [0,0.5], [size: _num_bands x 2]
//  _des        :   desired response, [size: _num_bands x 1]
//  _weights    :   response weighting, [size: _num_bands x 1]
//  _wtype      :   weight types (e.g. LIQUID_FIRDESPM_FLATWEIGHT) [size: _num_bands x 1]
//  _btype      :   band type (e.g. LIQUID_FIRDESPM_BANDPASS)
//  _h          :   output coefficients array, [size: _h_len x 1]
int firdespm_run(unsigned int            _h_len,
                 unsigned int            _num_bands,
                 float *                 _bands,
                 float *                 _des,
                 float *                 _weights,
                 liquid_firdespm_wtype * _wtype,
                 liquid_firdespm_btype   _btype,
                 float *                 _h);

// run filter design for basic low-pass filter
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _as     : stop-band attenuation [dB], _as > 0
//  _mu     : fractional sample offset, -0.5 < _mu < 0.5 [ignored]
//  _h      : output coefficient buffer, [size: _n x 1]
int firdespm_lowpass(unsigned int _n,
                      float        _fc,
                      float        _as,
                      float        _mu,
                      float *      _h);

// firdespm response callback function
//  _frequency  : normalized frequency
//  _userdata   : pointer to userdata
//  _desired    : (return) desired response
//  _weight     : (return) weight
typedef int (*firdespm_callback)(double   _frequency,
                                 void   * _userdata,
                                 double * _desired,
                                 double * _weight);

// structured object
typedef struct firdespm_s * firdespm;

// create firdespm object
//  _h_len      :   length of filter (number of taps)
//  _num_bands  :   number of frequency bands
//  _bands      :   band edges, f in [0,0.5], [size: _num_bands x 2]
//  _des        :   desired response, [size: _num_bands x 1]
//  _weights    :   response weighting, [size: _num_bands x 1]
//  _wtype      :   weight types (e.g. LIQUID_FIRDESPM_FLATWEIGHT) [size: _num_bands x 1]
//  _btype      :   band type (e.g. LIQUID_FIRDESPM_BANDPASS)
firdespm firdespm_create(unsigned int            _h_len,
                         unsigned int            _num_bands,
                         float *                 _bands,
                         float *                 _des,
                         float *                 _weights,
                         liquid_firdespm_wtype * _wtype,
                         liquid_firdespm_btype   _btype);

// create firdespm object with user-defined callback
//  _h_len      :   length of filter (number of taps)
//  _num_bands  :   number of frequency bands
//  _bands      :   band edges, f in [0,0.5], [size: _num_bands x 2]
//  _btype      :   band type (e.g. LIQUID_FIRDESPM_BANDPASS)
//  _callback   :   user-defined callback for specifying desired response & weights
//  _userdata   :   user-defined data structure for callback function
firdespm firdespm_create_callback(unsigned int          _h_len,
                                  unsigned int          _num_bands,
                                  float *               _bands,
                                  liquid_firdespm_btype _btype,
                                  firdespm_callback     _callback,
                                  void *                _userdata);

// Copy object including all internal objects and state
firdespm firdespm_copy(firdespm _q);

// destroy firdespm object
int firdespm_destroy(firdespm _q);

// print firdespm object internals
int firdespm_print(firdespm _q);

// execute filter design, storing result in _h
int firdespm_execute(firdespm _q, float * _h);

// Design halfband filter using Parks-McClellan algorithm given the
// filter length and desired transition band
//  _m      : filter semi-length, _m > 0
//  _ft     : filter transition band (relative), 0 < _ft < 0.5
//  _h      : output coefficient buffer, [size: 4 _m + 1 x 1]
int liquid_firdespm_halfband_ft(unsigned int _m,
                                float        _ft,
                                float *      _h);

// Design halfband filter using Parks-McClellan algorithm given the
// filter length and desired stop-band suppression
//  _m      : filter semi-length, _m > 0
//  _as     : filter stop-band suppression [dB], _as > 0
//  _h      : output coefficient buffer, [size: 4 _m + 1 x 1]
int liquid_firdespm_halfband_as(unsigned int _m,
                                float        _as,
                                float *      _h);

// Design FIR filter using generic window/taper method
//  _wtype  : window type, e.g. LIQUID_WINDOW_HAMMING
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _arg    : window-specific argument, if required
//  _h      : output coefficient buffer, [size: _n x 1]
int liquid_firdes_windowf(int          _wtype,
                          unsigned int _n,
                          float        _fc,
                          float        _arg,
                          float *      _h);

// Design FIR using Kaiser window
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _as     : stop-band attenuation [dB], _as > 0
//  _mu     : fractional sample offset, -0.5 < _mu < 0.5
//  _h      : output coefficient buffer, [size: _n x 1]
int liquid_firdes_kaiser(unsigned int _n,
                         float _fc,
                         float _as,
                         float _mu,
                         float *_h);

// Design finite impulse response notch filter
//  _m      : filter semi-length, m in [1,1000]
//  _f0     : filter notch frequency (normalized), -0.5 <= _fc <= 0.5
//  _as     : stop-band attenuation [dB], _as > 0
//  _h      : output coefficient buffer, [size: 2*_m+1 x 1]
int liquid_firdes_notch(unsigned int _m,
                        float        _f0,
                        float        _as,
                        float *      _h);

// Design FIR doppler filter
//  _n      : filter length
//  _fd     : normalized doppler frequency (0 < _fd < 0.5)
//  _K      : Rice fading factor (K >= 0)
//  _theta  : LoS component angle of arrival
//  _h      : output coefficient buffer
int liquid_firdes_doppler(unsigned int _n,
                          float        _fd,
                          float        _K,
                          float        _theta,
                          float *      _h);


// Design Nyquist raised-cosine filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_rcos(unsigned int _k,
                       unsigned int _m,
                       float _beta,
                       float _dt,
                       float * _h);

// Design root-Nyquist raised-cosine filter
int liquid_firdes_rrcos(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Design root-Nyquist Kaiser filter
int liquid_firdes_rkaiser(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Design (approximate) root-Nyquist Kaiser filter
int liquid_firdes_arkaiser(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Design root-Nyquist harris-Moerder filter
int liquid_firdes_hM3(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Design GMSK transmit and receive filters
int liquid_firdes_gmsktx(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);
int liquid_firdes_gmskrx(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Design flipped exponential Nyquist/root-Nyquist filters
int liquid_firdes_fexp( unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);
int liquid_firdes_rfexp(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Design flipped hyperbolic secand Nyquist/root-Nyquist filters
int liquid_firdes_fsech( unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);
int liquid_firdes_rfsech(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Design flipped arc-hyperbolic secand Nyquist/root-Nyquist filters
int liquid_firdes_farcsech( unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);
int liquid_firdes_rfarcsech(unsigned int _k, unsigned int _m, float _beta, float _dt, float * _h);

// Compute group delay for an FIR filter
//  _h      : filter coefficients array
//  _n      : filter length
//  _fc     : frequency at which delay is evaluated (-0.5 < _fc < 0.5)
float fir_group_delay(float * _h,
                      unsigned int _n,
                      float _fc);

// Compute group delay for an IIR filter
//  _b      : filter numerator coefficients
//  _nb     : filter numerator length
//  _a      : filter denominator coefficients
//  _na     : filter denominator length
//  _fc     : frequency at which delay is evaluated (-0.5 < _fc < 0.5)
float iir_group_delay(float * _b,
                      unsigned int _nb,
                      float * _a,
                      unsigned int _na,
                      float _fc);


// liquid_filter_autocorr()
//
// Compute auto-correlation of filter at a specific lag.
//
//  _h      :   filter coefficients, [size: _h_len x 1]
//  _h_len  :   filter length
//  _lag    :   auto-correlation lag (samples)
float liquid_filter_autocorr(float *      _h,
                             unsigned int _h_len,
                             int          _lag);

// liquid_filter_crosscorr()
//
// Compute cross-correlation of two filters at a specific lag.
//
//  _h      :   filter coefficients, [size: _h_len]
//  _h_len  :   filter length
//  _g      :   filter coefficients, [size: _g_len]
//  _g_len  :   filter length
//  _lag    :   cross-correlation lag (samples)
float liquid_filter_crosscorr(float *      _h,
                              unsigned int _h_len,
                              float *      _g,
                              unsigned int _g_len,
                              int          _lag);

// liquid_filter_isi()
//
// Compute inter-symbol interference (ISI)--both RMS and
// maximum--for the filter _h.
//
//  _h      :   filter coefficients, [size: 2*_k*_m+1 x 1]
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _rms    :   output root mean-squared ISI
//  _max    :   maximum ISI
void liquid_filter_isi(float *      _h,
                       unsigned int _k,
                       unsigned int _m,
                       float *      _rms,
                       float *      _max);

// Compute relative out-of-band energy
//
//  _h      :   filter coefficients, [size: _h_len x 1]
//  _h_len  :   filter length
//  _fc     :   analysis cut-off frequency
//  _nfft   :   fft size
float liquid_filter_energy(float *      _h,
                           unsigned int _h_len,
                           float        _fc,
                           unsigned int _nfft);

// Get static frequency response from filter coefficients at particular
// frequency with real-valued coefficients
//  _h      : coefficients, [size: _h_len x 1]
//  _h_len  : length of coefficients array
//  _fc     : center frequency for analysis, -0.5 <= _fc <= 0.5
//  _H      : pointer to output value
int liquid_freqrespf(float *                _h,
                     unsigned int           _h_len,
                     float                  _fc,
                     liquid_float_complex * _H);

// Get static frequency response from filter coefficients at particular
// frequency with complex coefficients
//  _h      : coefficients, [size: _h_len x 1]
//  _h_len  : length of coefficients array
//  _fc     : center frequency for analysis, -0.5 <= _fc <= 0.5
//  _H      : pointer to output value
int liquid_freqrespcf(liquid_float_complex * _h,
                      unsigned int           _h_len,
                      float                  _fc,
                      liquid_float_complex * _H);


//
// IIR filter design
//

// IIR filter design filter type
typedef enum {
    LIQUID_IIRDES_BUTTER=0,
    LIQUID_IIRDES_CHEBY1,
    LIQUID_IIRDES_CHEBY2,
    LIQUID_IIRDES_ELLIP,
    LIQUID_IIRDES_BESSEL
} liquid_iirdes_filtertype;

// IIR filter design band type
typedef enum {
    LIQUID_IIRDES_LOWPASS=0,
    LIQUID_IIRDES_HIGHPASS,
    LIQUID_IIRDES_BANDPASS,
    LIQUID_IIRDES_BANDSTOP
} liquid_iirdes_bandtype;

// IIR filter design coefficients format
typedef enum {
    LIQUID_IIRDES_SOS=0,
    LIQUID_IIRDES_TF
} liquid_iirdes_format;

// IIR filter design template
//  _ftype      :   filter type (e.g. LIQUID_IIRDES_BUTTER)
//  _btype      :   band type (e.g. LIQUID_IIRDES_BANDPASS)
//  _format     :   coefficients format (e.g. LIQUID_IIRDES_SOS)
//  _n          :   filter order
//  _fc         :   low-pass prototype cut-off frequency
//  _f0         :   center frequency (band-pass, band-stop)
//  _ap         :   pass-band ripple in dB
//  _as         :   stop-band ripple in dB
//  _b          :   numerator
//  _a          :   denominator
int liquid_iirdes(liquid_iirdes_filtertype _ftype,
                  liquid_iirdes_bandtype   _btype,
                  liquid_iirdes_format     _format,
                  unsigned int             _n,
                  float                    _fc,
                  float                    _f0,
                  float                    _ap,
                  float                    _as,
                  float *                  _b,
                  float *                  _a);

// compute analog zeros, poles, gain for specific filter types
int butter_azpkf(unsigned int           _n,
                 liquid_float_complex * _za,
                 liquid_float_complex * _pa,
                 liquid_float_complex * _ka);
int cheby1_azpkf(unsigned int           _n,
                 float                  _ep,
                 liquid_float_complex * _z,
                 liquid_float_complex * _p,
                 liquid_float_complex * _k);
int cheby2_azpkf(unsigned int           _n,
                 float                  _es,
                 liquid_float_complex * _z,
                 liquid_float_complex * _p,
                 liquid_float_complex * _k);
int ellip_azpkf(unsigned int            _n,
                float                   _ep,
                float                   _es,
                liquid_float_complex *  _z,
                liquid_float_complex *  _p,
                liquid_float_complex *  _k);
int bessel_azpkf(unsigned int           _n,
                 liquid_float_complex * _z,
                 liquid_float_complex * _p,
                 liquid_float_complex * _k);

// compute frequency pre-warping factor
float iirdes_freqprewarp(liquid_iirdes_bandtype _btype,
                         float _fc,
                         float _f0);

// convert analog z/p/k form to discrete z/p/k form (bilinear z-transform)
//  _za     :   analog zeros [length: _nza]
//  _nza    :   number of analog zeros
//  _pa     :   analog poles [length: _npa]
//  _npa    :   number of analog poles
//  _m      :   frequency pre-warping factor
//  _zd     :   output digital zeros [length: _npa]
//  _pd     :   output digital poles [length: _npa]
//  _kd     :   output digital gain (should actually be real-valued)
int bilinear_zpkf(liquid_float_complex * _za,
                  unsigned int           _nza,
                  liquid_float_complex * _pa,
                  unsigned int           _npa,
                  liquid_float_complex   _ka,
                  float                  _m,
                  liquid_float_complex * _zd,
                  liquid_float_complex * _pd,
                  liquid_float_complex * _kd);

// compute bilinear z-transform using polynomial expansion in numerator and
// denominator
//
//          b[0] + b[1]*s + ... + b[nb]*s^(nb-1)
// H(s) =   ------------------------------------
//          a[0] + a[1]*s + ... + a[na]*s^(na-1)
//
// computes H(z) = H( s -> _m*(z-1)/(z+1) ) and expands as
//
//          bd[0] + bd[1]*z^-1 + ... + bd[nb]*z^-n
// H(z) =   --------------------------------------
//          ad[0] + ad[1]*z^-1 + ... + ad[nb]*z^-m
//
//  _b          : numerator array, [size: _b_order+1]
//  _b_order    : polynomial order of _b
//  _a          : denominator array, [size: _a_order+1]
//  _a_order    : polynomial order of _a
//  _m          : bilateral warping factor
//  _bd         : output digital filter numerator, [size: _b_order+1]
//  _ad         : output digital filter numerator, [size: _a_order+1]
int bilinear_nd(liquid_float_complex * _b,
                unsigned int           _b_order,
                liquid_float_complex * _a,
                unsigned int           _a_order,
                float                  _m,
                liquid_float_complex * _bd,
                liquid_float_complex * _ad);

// digital z/p/k low-pass to high-pass
//  _zd     :   digital zeros (low-pass prototype), [length: _n]
//  _pd     :   digital poles (low-pass prototype), [length: _n]
//  _n      :   low-pass filter order
//  _zdt    :   output digital zeros transformed [length: _n]
//  _pdt    :   output digital poles transformed [length: _n]
int iirdes_dzpk_lp2hp(liquid_float_complex * _zd,
                      liquid_float_complex * _pd,
                      unsigned int _n,
                      liquid_float_complex * _zdt,
                      liquid_float_complex * _pdt);

// digital z/p/k low-pass to band-pass
//  _zd     :   digital zeros (low-pass prototype), [length: _n]
//  _pd     :   digital poles (low-pass prototype), [length: _n]
//  _n      :   low-pass filter order
//  _f0     :   center frequency
//  _zdt    :   output digital zeros transformed [length: 2*_n]
//  _pdt    :   output digital poles transformed [length: 2*_n]
int iirdes_dzpk_lp2bp(liquid_float_complex * _zd,
                      liquid_float_complex * _pd,
                      unsigned int           _n,
                      float                  _f0,
                      liquid_float_complex * _zdt,
                      liquid_float_complex * _pdt);

// convert discrete z/p/k form to transfer function
//  _zd     :   digital zeros [length: _n]
//  _pd     :   digital poles [length: _n]
//  _n      :   filter order
//  _kd     :   digital gain
//  _b      :   output numerator [length: _n+1]
//  _a      :   output denominator [length: _n+1]
int iirdes_dzpk2tff(liquid_float_complex * _zd,
                    liquid_float_complex * _pd,
                    unsigned int           _n,
                    liquid_float_complex   _kd,
                    float *                _b,
                    float *                _a);

// convert discrete z/p/k form to second-order sections
//  _zd     :   digital zeros [length: _n]
//  _pd     :   digital poles [length: _n]
//  _n      :   filter order
//  _kd     :   digital gain
//  _b      :   output numerator, [size: 3 x L+r]
//  _a      :   output denominator, [size: 3 x L+r]
//  where r = _n%2, L = (_n-r)/2
int iirdes_dzpk2sosf(liquid_float_complex * _zd,
                     liquid_float_complex * _pd,
                     unsigned int           _n,
                     liquid_float_complex   _kd,
                     float *                _b,
                     float *                _a);

// additional IIR filter design templates

// design 2nd-order IIR filter (active lag)
//          1 + t2 * s
//  F(s) = ------------
//          1 + t1 * s
//
//  _w      :   filter bandwidth
//  _zeta   :   damping factor (1/sqrt(2) suggested)
//  _K      :   loop gain (1000 suggested)
//  _b      :   output feed-forward coefficients, [size: 3 x 1]
//  _a      :   output feed-back coefficients, [size: 3 x 1]
void iirdes_pll_active_lag(float _w,
                           float _zeta,
                           float _K,
                           float * _b,
                           float * _a);

// design 2nd-order IIR filter (active PI)
//          1 + t2 * s
//  F(s) = ------------
//           t1 * s
//
//  _w      :   filter bandwidth
//  _zeta   :   damping factor (1/sqrt(2) suggested)
//  _K      :   loop gain (1000 suggested)
//  _b      :   output feed-forward coefficients, [size: 3 x 1]
//  _a      :   output feed-back coefficients, [size: 3 x 1]
void iirdes_pll_active_PI(float _w,
                          float _zeta,
                          float _K,
                          float * _b,
                          float * _a);

// checks stability of iir filter
//  _b      :   feed-forward coefficients, [size: _n x 1]
//  _a      :   feed-back coefficients, [size: _n x 1]
//  _n      :   number of coefficients
int iirdes_isstable(float * _b,
                    float * _a,
                    unsigned int _n);

//
// linear prediction
//

// compute the linear prediction coefficients for an input signal _x
//  _x      :   input signal, [size: _n x 1]
//  _n      :   input signal length
//  _p      :   prediction filter order
//  _a      :   prediction filter, [size: _p+1 x 1]
//  _e      :   prediction error variance, [size: _p+1 x 1]
void liquid_lpc(float * _x,
                unsigned int _n,
                unsigned int _p,
                float * _a,
                float * _g);

// solve the Yule-Walker equations using Levinson-Durbin recursion
// for _symmetric_ autocorrelation
//  _r      :   autocorrelation array, [size: _p+1 x 1]
//  _p      :   filter order
//  _a      :   output coefficients, [size: _p+1 x 1]
//  _e      :   error variance, [size: _p+1 x 1]
//
// NOTES:
//  By definition _a[0] = 1.0
void liquid_levinson(float * _r,
                     unsigned int _p,
                     float * _a,
                     float * _e);

//
// auto-correlator (delay cross-correlation)
//

#define LIQUID_AUTOCORR_MANGLE_CCCF(name) LIQUID_CONCAT(autocorr_cccf,name)
#define LIQUID_AUTOCORR_MANGLE_RRRF(name) LIQUID_CONCAT(autocorr_rrrf,name)

// Macro:
//   AUTOCORR   : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_AUTOCORR_DEFINE_API(AUTOCORR,TO,TC,TI)                       \
                                                                            \
/* Computes auto-correlation with a fixed lag on input signals          */  \
typedef struct AUTOCORR(_s) * AUTOCORR();                                   \
                                                                            \
/* Create auto-correlator object with a particular window length and    */  \
/* delay                                                                */  \
/*  _window_size    : size of the correlator window                     */  \
/*  _delay          : correlator delay [samples]                        */  \
AUTOCORR() AUTOCORR(_create)(unsigned int _window_size,                     \
                             unsigned int _delay);                          \
                                                                            \
/* Destroy auto-correlator object, freeing internal memory              */  \
int AUTOCORR(_destroy)(AUTOCORR() _q);                                      \
                                                                            \
/* Reset auto-correlator object's internals                             */  \
int AUTOCORR(_reset)(AUTOCORR() _q);                                        \
                                                                            \
/* Print auto-correlator parameters to stdout                           */  \
int AUTOCORR(_print)(AUTOCORR() _q);                                        \
                                                                            \
/* Push sample into auto-correlator object                              */  \
/*  _q      : auto-correlator object                                    */  \
/*  _x      : single input sample                                       */  \
int AUTOCORR(_push)(AUTOCORR() _q,                                          \
                    TI         _x);                                         \
                                                                            \
/* Write block of samples to auto-correlator object                     */  \
/*  _q      :   auto-correlation object                                 */  \
/*  _x      :   input array, [size: _n x 1]                             */  \
/*  _n      :   number of input samples                                 */  \
int AUTOCORR(_write)(AUTOCORR()   _q,                                       \
                     TI *         _x,                                       \
                     unsigned int _n);                                      \
                                                                            \
/* Compute single auto-correlation output                               */  \
/*  _q      : auto-correlator object                                    */  \
/*  _rxx    : auto-correlated output                                    */  \
int AUTOCORR(_execute)(AUTOCORR() _q,                                       \
                       TO *       _rxx);                                    \
                                                                            \
/* Compute auto-correlation on block of samples; the input and output   */  \
/* arrays may have the same pointer                                     */  \
/*  _q      :   auto-correlation object                                 */  \
/*  _x      :   input array, [size: _n x 1]                             */  \
/*  _n      :   number of input, output samples                         */  \
/*  _rxx    :   input array, [size: _n x 1]                             */  \
int AUTOCORR(_execute_block)(AUTOCORR()   _q,                               \
                             TI *         _x,                               \
                             unsigned int _n,                               \
                             TO *         _rxx);                            \
                                                                            \
/* return sum of squares of buffered samples                            */  \
float AUTOCORR(_get_energy)(AUTOCORR() _q);                                 \

LIQUID_AUTOCORR_DEFINE_API(LIQUID_AUTOCORR_MANGLE_CCCF,
                           liquid_float_complex,
                           liquid_float_complex,
                           liquid_float_complex)

LIQUID_AUTOCORR_DEFINE_API(LIQUID_AUTOCORR_MANGLE_RRRF,
                           float,
                           float,
                           float)


//
// Finite impulse response filter
//

#define LIQUID_FIRFILT_MANGLE_RRRF(name) LIQUID_CONCAT(firfilt_rrrf,name)
#define LIQUID_FIRFILT_MANGLE_CRCF(name) LIQUID_CONCAT(firfilt_crcf,name)
#define LIQUID_FIRFILT_MANGLE_CCCF(name) LIQUID_CONCAT(firfilt_cccf,name)

// Macro:
//   FIRFILT    : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_FIRFILT_DEFINE_API(FIRFILT,TO,TC,TI)                         \
                                                                            \
/* Finite impulse response (FIR) filter                                 */  \
typedef struct FIRFILT(_s) * FIRFILT();                                     \
                                                                            \
/* Create a finite impulse response filter (firfilt) object by directly */  \
/* specifying the filter coefficients in an array                       */  \
/*  _h      : filter coefficients, [size: _n x 1]                       */  \
/*  _n      : number of filter coefficients, _n > 0                     */  \
FIRFILT() FIRFILT(_create)(TC *         _h,                                 \
                           unsigned int _n);                                \
                                                                            \
/* Create object using Kaiser-Bessel windowed sinc method               */  \
/*  _n      : filter length, _n > 0                                     */  \
/*  _fc     : filter normalized cut-off frequency, 0 < _fc < 0.5        */  \
/*  _as     : filter stop-band attenuation [dB], _as > 0                */  \
/*  _mu     : fractional sample offset, -0.5 < _mu < 0.5                */  \
FIRFILT() FIRFILT(_create_kaiser)(unsigned int _n,                          \
                                  float        _fc,                         \
                                  float        _as,                         \
                                  float        _mu);                        \
                                                                            \
/* Create object from square-root Nyquist prototype.                    */  \
/* The filter length will be \(2 k m + 1 \) samples long with a delay   */  \
/* of \( k m + 1 \) samples.                                            */  \
/*  _type   : filter type (e.g. LIQUID_FIRFILT_RRC)                     */  \
/*  _k      : nominal samples per symbol, _k > 1                        */  \
/*  _m      : filter delay [symbols], _m > 0                            */  \
/*  _beta   : rolloff factor, 0 < beta <= 1                             */  \
/*  _mu     : fractional sample offset [samples], -0.5 < _mu < 0.5      */  \
FIRFILT() FIRFILT(_create_rnyquist)(int          _type,                     \
                                    unsigned int _k,                        \
                                    unsigned int _m,                        \
                                    float        _beta,                     \
                                    float        _mu);                      \
                                                                            \
/* Create object from Parks-McClellan algorithm prototype               */  \
/*  _h_len  : filter length, _h_len > 0                                 */  \
/*  _fc     : cutoff frequency, 0 < _fc < 0.5                           */  \
/*  _as     : stop-band attenuation [dB], _as > 0                       */  \
FIRFILT() FIRFILT(_create_firdespm)(unsigned int _h_len,                    \
                                    float        _fc,                       \
                                    float        _as);                      \
                                                                            \
/* Create rectangular filter prototype; that is                         */  \
/* \( \vec{h} = \{ 1, 1, 1, \ldots 1 \} \)                              */  \
/*  _n  : length of filter [samples], 0 < _n <= 1024                    */  \
FIRFILT() FIRFILT(_create_rect)(unsigned int _n);                           \
                                                                            \
/* Create DC blocking filter from prototype                             */  \
/*  _m  : prototype filter semi-length such that filter length is 2*m+1 */  \
/*  _as : prototype filter stop-band attenuation [dB], _as > 0          */  \
FIRFILT() FIRFILT(_create_dc_blocker)(unsigned int _m,                      \
                                      float        _as);                    \
                                                                            \
/* Create notch filter from prototype                                   */  \
/*  _m  : prototype filter semi-length such that filter length is 2*m+1 */  \
/*  _as : prototype filter stop-band attenuation [dB], _as > 0          */  \
/*  _f0 : center frequency for notch, _fc in [-0.5, 0.5]                */  \
FIRFILT() FIRFILT(_create_notch)(unsigned int _m,                           \
                                 float        _as,                          \
                                 float        _f0);                         \
                                                                            \
/* Re-create filter object of potentially a different length with       */  \
/* different coefficients. If the length of the filter does not change, */  \
/* not memory reallocation is invoked.                                  */  \
/*  _q      : original filter object                                    */  \
/*  _h      : pointer to filter coefficients, [size: _n x 1]            */  \
/*  _n      : filter length, _n > 0                                     */  \
FIRFILT() FIRFILT(_recreate)(FIRFILT()    _q,                               \
                             TC *         _h,                               \
                             unsigned int _n);                              \
                                                                            \
/* Copy object including all internal objects and state                 */  \
FIRFILT() FIRFILT(_copy)(FIRFILT() _q);                                     \
                                                                            \
/* Destroy filter object and free all internal memory                   */  \
int FIRFILT(_destroy)(FIRFILT() _q);                                        \
                                                                            \
/* Reset filter object's internal buffer                                */  \
int FIRFILT(_reset)(FIRFILT() _q);                                          \
                                                                            \
/* Print filter object information to stdout                            */  \
int FIRFILT(_print)(FIRFILT() _q);                                          \
                                                                            \
/* Set output scaling for filter                                        */  \
/*  _q      : filter object                                             */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int FIRFILT(_set_scale)(FIRFILT() _q,                                       \
                        TC        _scale);                                  \
                                                                            \
/* Get output scaling for filter                                        */  \
/*  _q      : filter object                                             */  \
/*  _scale  : scaling factor applied to each output sample              */  \
int FIRFILT(_get_scale)(FIRFILT() _q,                                       \
                        TC *      _scale);                                  \
                                                                            \
/* Push sample into filter object's internal buffer                     */  \
/*  _q      : filter object                                             */  \
/*  _x      : single input sample                                       */  \
int FIRFILT(_push)(FIRFILT() _q,                                            \
                   TI        _x);                                           \
                                                                            \
/* Write block of samples into filter object's internal buffer          */  \
/*  _q      : filter object                                             */  \
/*  _x      : buffer of input samples, [size: _n x 1]                   */  \
/*  _n      : number of input samples                                   */  \
int FIRFILT(_write)(FIRFILT()    _q,                                        \
                    TI *         _x,                                        \
                    unsigned int _n);                                       \
                                                                            \
/* Execute vector dot product on the filter's internal buffer and       */  \
/* coefficients                                                         */  \
/*  _q      : filter object                                             */  \
/*  _y      : pointer to single output sample                           */  \
int FIRFILT(_execute)(FIRFILT() _q,                                         \
                      TO *      _y);                                        \
                                                                            \
/* Execute filter on one sample, equivalent to push() and execute()     */  \
/*  _q      : filter object                                             */  \
/*  _x      : single input sample                                       */  \
/*  _y      : pointer to single output sample                           */  \
int FIRFILT(_execute_one)(FIRFILT() _q,                                     \
                          TI        _x,                                     \
                          TO *      _y);                                    \
                                                                            \
/* Execute the filter on a block of input samples; in-place operation   */  \
/* is permitted (_x and _y may point to the same place in memory)       */  \
/*  _q      : filter object                                             */  \
/*  _x      : pointer to input array, [size: _n x 1]                    */  \
/*  _n      : number of input, output samples                           */  \
/*  _y      : pointer to output array, [size: _n x 1]                   */  \
int FIRFILT(_execute_block)(FIRFILT()    _q,                                \
                            TI *         _x,                                \
                            unsigned int _n,                                \
                            TO *         _y);                               \
                                                                            \
/* Get length of filter object (number of internal coefficients)        */  \
unsigned int FIRFILT(_get_length)(FIRFILT() _q);                            \
                                                                            \
/* Get pointer to coefficients array                                    */  \
const TC * FIRFILT(_get_coefficients)(FIRFILT() _q);                        \
                                                                            \
/* Copy internal coefficients to external buffer                        */  \
/*  _q      : filter object                                             */  \
/*  _h      : pointer to output coefficients array, [size: _n x 1]      */  \
int FIRFILT(_copy_coefficients)(FIRFILT() _q,                               \
                                TC *      _h);                              \
                                                                            \
/* Compute complex frequency response of filter object                  */  \
/*  _q      : filter object                                             */  \
/*  _fc     : normalized frequency for evaluation                       */  \
/*  _H      : pointer to output complex frequency response              */  \
int FIRFILT(_freqresponse)(FIRFILT()              _q,                       \
                           float                  _fc,                      \
                           liquid_float_complex * _H);                      \
                                                                            \
/* Compute and return group delay of filter object                      */  \
/*  _q      : filter object                                             */  \
/*  _fc     : frequency to evaluate                                     */  \
float FIRFILT(_groupdelay)(FIRFILT() _q,                                    \
                           float     _fc);                                  \

LIQUID_FIRFILT_DEFINE_API(LIQUID_FIRFILT_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_FIRFILT_DEFINE_API(LIQUID_FIRFILT_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)

LIQUID_FIRFILT_DEFINE_API(LIQUID_FIRFILT_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)

// fdelay : arbitrary delay
#define LIQUID_FDELAY_MANGLE_RRRF(name) LIQUID_CONCAT(fdelay_rrrf,name)
#define LIQUID_FDELAY_MANGLE_CRCF(name) LIQUID_CONCAT(fdelay_crcf,name)

// Macro:
//   FDELAY    : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_FDELAY_DEFINE_API(FDELAY,TO,TC,TI)                           \
                                                                            \
/* Finite impulse response (FIR) filter                                 */  \
typedef struct FDELAY(_s) * FDELAY();                                       \
                                                                            \
/* Create a delay object with a maximum offset and filter specification */  \
/*  _nmax   : maximum integer sample offset                             */  \
/*  _m      : polyphase filter-bank semi-length, _m > 0                 */  \
/*  _npfb   : number of filters in polyphase filter-bank, _npfb > 0     */  \
FDELAY() FDELAY(_create)(unsigned int _nmax,                                \
                         unsigned int _m,                                   \
                         unsigned int _npfb);                               \
                                                                            \
/* Create a delay object with a maximum offset and default filter       */  \
/* parameters (_m = 8, _npfb = 64)                                      */  \
/*  _nmax   : maximum integer sample offset                             */  \
FDELAY() FDELAY(_create_default)(unsigned int _nmax);                       \
                                                                            \
/* Destroy delay object and free all internal memory                    */  \
int FDELAY(_destroy)(FDELAY() _q);                                          \
                                                                            \
/* Reset delay object internals                                         */  \
int FDELAY(_reset)(FDELAY() _q);                                            \
                                                                            \
/* Print delay object internals                                         */  \
int FDELAY(_print)(FDELAY() _q);                                            \
                                                                            \
/* Get current delay (accounting for _m?)                               */  \
float FDELAY(_get_delay)(FDELAY() _q);                                      \
int   FDELAY(_set_delay)(FDELAY() _q, float _delay);                        \
int   FDELAY(_adjust_delay)(FDELAY() _q, float _delta);                     \
                                                                            \
unsigned int FDELAY(_get_nmax)(FDELAY() _q);                                \
unsigned int FDELAY(_get_m)   (FDELAY() _q);                                \
unsigned int FDELAY(_get_npfb)(FDELAY() _q);                                \
                                                                            \
/* Push sample into filter object's internal buffer                     */  \
/*  _q      : filter object                                             */  \
/*  _x      : single input sample                                       */  \
int FDELAY(_push)(FDELAY() _q,                                              \
                  TI       _x);                                             \
                                                                            \
/* Write a block of samplex into filter object's internal buffer        */  \
/*  _q      : filter object                                             */  \
/*  _x      : buffer of input samples, [size: _n x 1]                   */  \
/*  _n      : number of input samples                                   */  \
int FDELAY(_write)(FDELAY()     _q,                                         \
                   TI *         _x,                                         \
                   unsigned int _n);                                        \
                                                                            \
/* Execute vector dot product on the filter's internal buffer and       */  \
/* coefficients                                                         */  \
/*  _q      : filter object                                             */  \
/*  _y      : pointer to single output sample                           */  \
int FDELAY(_execute)(FDELAY() _q,                                           \
                     TO *     _y);                                          \
                                                                            \
/* Execute the filter on a block of input samples; in-place operation   */  \
/* is permitted (_x and _y may point to the same place in memory)       */  \
/*  _q      : filter object                                             */  \
/*  _x      : pointer to input array, [size: _n x 1]                    */  \
/*  _n      : number of input, output samples                           */  \
/*  _y      : pointer to output array, [size: _n x 1]                   */  \
int FDELAY(_execute_block)(FDELAY()     _q,                                 \
                           TI *         _x,                                 \
                           unsigned int _n,                                 \
                           TO *         _y);                                \

LIQUID_FDELAY_DEFINE_API(LIQUID_FDELAY_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_FDELAY_DEFINE_API(LIQUID_FDELAY_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)

//
// FIR Hilbert transform
//  2:1 real-to-complex decimator
//  1:2 complex-to-real interpolator
//

#define LIQUID_FIRHILB_MANGLE_FLOAT(name)  LIQUID_CONCAT(firhilbf, name)
//#define LIQUID_FIRHILB_MANGLE_DOUBLE(name) LIQUID_CONCAT(firhilb, name)

// NOTES:
//   Although firhilb is a placeholder for both decimation and
//   interpolation, separate objects should be used for each task.
#define LIQUID_FIRHILB_DEFINE_API(FIRHILB,T,TC)                             \
                                                                            \
/* Finite impulse response (FIR) Hilbert transform                      */  \
typedef struct FIRHILB(_s) * FIRHILB();                                     \
                                                                            \
/* Create a firhilb object with a particular filter semi-length and     */  \
/* desired stop-band attenuation.                                       */  \
/* Internally the object designs a half-band filter based on applying   */  \
/* a Kaiser-Bessel window to a sinc function to guarantee zeros at all  */  \
/* off-center odd indexed samples.                                      */  \
/*  _m      : filter semi-length, delay is \( 2 m + 1 \)                */  \
/*  _as     : filter stop-band attenuation [dB]                         */  \
FIRHILB() FIRHILB(_create)(unsigned int _m,                                 \
                           float        _as);                               \
                                                                            \
/* Copy object including all internal objects and state                 */  \
FIRHILB() FIRHILB(_copy)(FIRHILB() _q);                                     \
                                                                            \
/* Destroy finite impulse response Hilbert transform, freeing all       */  \
/* internally-allocted memory and objects.                              */  \
int FIRHILB(_destroy)(FIRHILB() _q);                                        \
                                                                            \
/* Print firhilb object internals to stdout                             */  \
int FIRHILB(_print)(FIRHILB() _q);                                          \
                                                                            \
/* Reset firhilb object internal state                                  */  \
int FIRHILB(_reset)(FIRHILB() _q);                                          \
                                                                            \
/* Execute Hilbert transform (real to complex)                          */  \
/*  _q      :   Hilbert transform object                                */  \
/*  _x      :   real-valued input sample                                */  \
/*  _y      :   complex-valued output sample                            */  \
int FIRHILB(_r2c_execute)(FIRHILB() _q,                                     \
                          T         _x,                                     \
                          TC *      _y);                                    \
                                                                            \
/* Execute Hilbert transform (complex to real)                          */  \
/*  _q      :   Hilbert transform object                                */  \
/*  _x      :   complex-valued input sample                             */  \
/*  _y0     :   real-valued output sample, lower side-band retained     */  \
/*  _y1     :   real-valued output sample, upper side-band retained     */  \
int FIRHILB(_c2r_execute)(FIRHILB() _q,                                     \
                          TC        _x,                                     \
                          T *       _y0,                                    \
                          T *       _y1);                                   \
                                                                            \
/* Execute Hilbert transform decimator (real to complex)                */  \
/*  _q      :   Hilbert transform object                                */  \
/*  _x      :   real-valued input array, [size: 2 x 1]                  */  \
/*  _y      :   complex-valued output sample                            */  \
int FIRHILB(_decim_execute)(FIRHILB() _q,                                   \
                            T *       _x,                                   \
                            TC *      _y);                                  \
                                                                            \
/* Execute Hilbert transform decimator (real to complex) on a block of  */  \
/* samples                                                              */  \
/*  _q      :   Hilbert transform object                                */  \
/*  _x      :   real-valued input array, [size: 2*_n x 1]               */  \
/*  _n      :   number of output samples                                */  \
/*  _y      :   complex-valued output array, [size: _n x 1]             */  \
int FIRHILB(_decim_execute_block)(FIRHILB()    _q,                          \
                                  T *          _x,                          \
                                  unsigned int _n,                          \
                                  TC *         _y);                         \
                                                                            \
/* Execute Hilbert transform interpolator (real to complex)             */  \
/*  _q      :   Hilbert transform object                                */  \
/*  _x      :   complex-valued input sample                             */  \
/*  _y      :   real-valued output array, [size: 2 x 1]                 */  \
int FIRHILB(_interp_execute)(FIRHILB() _q,                                  \
                             TC        _x,                                  \
                             T *       _y);                                 \
                                                                            \
/* Execute Hilbert transform interpolator (complex to real) on a block  */  \
/* of samples                                                           */  \
/*  _q      :   Hilbert transform object                                */  \
/*  _x      :   complex-valued input array, [size: _n x 1]              */  \
/*  _n      :   number of *input* samples                               */  \
/*  _y      :   real-valued output array, [size: 2*_n x 1]              */  \
int FIRHILB(_interp_execute_block)(FIRHILB()    _q,                         \
                                   TC *         _x,                         \
                                   unsigned int _n,                         \
                                   T *          _y);                        \

LIQUID_FIRHILB_DEFINE_API(LIQUID_FIRHILB_MANGLE_FLOAT, float, liquid_float_complex)
//LIQUID_FIRHILB_DEFINE_API(LIQUID_FIRHILB_MANGLE_DOUBLE, double, liquid_double_complex)


//
// Infinite impulse response (IIR) Hilbert transform
//  2:1 real-to-complex decimator
//  1:2 complex-to-real interpolator
//

#define LIQUID_IIRHILB_MANGLE_FLOAT(name)  LIQUID_CONCAT(iirhilbf, name)
//#define LIQUID_IIRHILB_MANGLE_DOUBLE(name) LIQUID_CONCAT(iirhilb, name)

// NOTES:
//   Although iirhilb is a placeholder for both decimation and
//   interpolation, separate objects should be used for each task.
#define LIQUID_IIRHILB_DEFINE_API(IIRHILB,T,TC)                             \
                                                                            \
/* Infinite impulse response (IIR) Hilbert transform                    */  \
typedef struct IIRHILB(_s) * IIRHILB();                                     \
                                                                            \
/* Create a iirhilb object with a particular filter type, order, and    */  \
/* desired pass- and stop-band attenuation.                             */  \
/*  _ftype  : filter type (e.g. LIQUID_IIRDES_BUTTER)                   */  \
/*  _n      : filter order, _n > 0                                      */  \
/*  _ap     : pass-band ripple [dB], _ap > 0                            */  \
/*  _as     : stop-band ripple [dB], _as > 0                            */  \
IIRHILB() IIRHILB(_create)(liquid_iirdes_filtertype _ftype,                 \
                           unsigned int             _n,                     \
                           float                    _ap,                    \
                           float                    _as);                   \
                                                                            \
/* Copy object including all internal objects and state                 */  \
IIRHILB() IIRHILB(_copy)(IIRHILB() _q);                                     \
                                                                            \
/* Create a default iirhilb object with a particular filter order.      */  \
/*  _n      : filter order, _n > 0                                      */  \
IIRHILB() IIRHILB(_create_default)(unsigned int _n);                        \
                                                                            \
/* Destroy finite impulse response Hilbert transform, freeing all       */  \
/* internally-allocted memory and objects.                              */  \
int IIRHILB(_destroy)(IIRHILB() _q);                                        \
                                                                            \
/* Print iirhilb object internals to stdout                             */  \
int IIRHILB(_print)(IIRHILB() _q);                                          \
                                                                            \
/* Reset iirhilb object internal state                                  */  \
int IIRHILB(_reset)(IIRHILB() _q);                                          \
                                                                            \
/* Execute Hilbert transform (real to complex)                          */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : real-valued input sample                                  */  \
/*  _y      : complex-valued output sample                              */  \
int IIRHILB(_r2c_execute)(IIRHILB() _q,                                     \
                          T         _x,                                     \
                          TC *      _y);                                    \
                                                                            \
/* Execute Hilbert transform (real to complex) on a block of samples    */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : real-valued input sample array, [size: _n x 1]            */  \
/*  _n      : number of input,output samples                            */  \
/*  _y      : complex-valued output sample array, [size: _n x 1]        */  \
int IIRHILB(_r2c_execute_block)(IIRHILB()    _q,                            \
                                T *          _x,                            \
                                unsigned int _n,                            \
                                TC *         _y);                           \
                                                                            \
/* Execute Hilbert transform (complex to real)                          */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : complex-valued input sample                               */  \
/*  _y      : real-valued output sample                                 */  \
int IIRHILB(_c2r_execute)(IIRHILB() _q,                                     \
                          TC        _x,                                     \
                          T *       _y);                                    \
                                                                            \
/* Execute Hilbert transform (complex to real) on a block of samples    */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : complex-valued input sample array, [size: _n x 1]         */  \
/*  _n      : number of input,output samples                            */  \
/*  _y      : real-valued output sample array, [size: _n x 1]           */  \
int IIRHILB(_c2r_execute_block)(IIRHILB()    _q,                            \
                                TC *         _x,                            \
                                unsigned int _n,                            \
                                T *          _y);                           \
                                                                            \
/* Execute Hilbert transform decimator (real to complex)                */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : real-valued input array, [size: 2 x 1]                    */  \
/*  _y      : complex-valued output sample                              */  \
int IIRHILB(_decim_execute)(IIRHILB() _q,                                   \
                            T *       _x,                                   \
                            TC *      _y);                                  \
                                                                            \
/* Execute Hilbert transform decimator (real to complex) on a block of  */  \
/* samples                                                              */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : real-valued input array, [size: 2*_n x 1]                 */  \
/*  _n      : number of output samples                                  */  \
/*  _y      : complex-valued output array, [size: _n x 1]               */  \
int IIRHILB(_decim_execute_block)(IIRHILB()    _q,                          \
                                  T *          _x,                          \
                                  unsigned int _n,                          \
                                  TC *         _y);                         \
                                                                            \
/* Execute Hilbert transform interpolator (real to complex)             */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : complex-valued input sample                               */  \
/*  _y      : real-valued output array, [size: 2 x 1]                   */  \
int IIRHILB(_interp_execute)(IIRHILB() _q,                                  \
                             TC        _x,                                  \
                             T *       _y);                                 \
                                                                            \
/* Execute Hilbert transform interpolator (complex to real) on a block  */  \
/* of samples                                                           */  \
/*  _q      : Hilbert transform object                                  */  \
/*  _x      : complex-valued input array, [size: _n x 1]                */  \
/*  _n      : number of *input* samples                                 */  \
/*  _y      : real-valued output array, [size: 2*_n x 1]                */  \
int IIRHILB(_interp_execute_block)(IIRHILB()    _q,                         \
                                   TC *         _x,                         \
                                   unsigned int _n,                         \
                                   T *          _y);                        \

LIQUID_IIRHILB_DEFINE_API(LIQUID_IIRHILB_MANGLE_FLOAT, float, liquid_float_complex)
//LIQUID_IIRHILB_DEFINE_API(LIQUID_IIRHILB_MANGLE_DOUBLE, double, liquid_double_complex)


//
// FFT-based finite impulse response filter
//

#define LIQUID_FFTFILT_MANGLE_RRRF(name) LIQUID_CONCAT(fftfilt_rrrf,name)
#define LIQUID_FFTFILT_MANGLE_CRCF(name) LIQUID_CONCAT(fftfilt_crcf,name)
#define LIQUID_FFTFILT_MANGLE_CCCF(name) LIQUID_CONCAT(fftfilt_cccf,name)

// Macro:
//   FFTFILT : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_FFTFILT_DEFINE_API(FFTFILT,TO,TC,TI)                         \
                                                                            \
/* Fast Fourier transform (FFT) finite impulse response filter          */  \
typedef struct FFTFILT(_s) * FFTFILT();                                     \
                                                                            \
/* Create FFT-based FIR filter using external coefficients              */  \
/*  _h      : filter coefficients, [size: _h_len x 1]                   */  \
/*  _h_len  : filter length, _h_len > 0                                 */  \
/*  _n      : block size = nfft/2, _n >= _h_len-1                       */  \
FFTFILT() FFTFILT(_create)(TC *         _h,                                 \
                           unsigned int _h_len,                             \
                           unsigned int _n);                                \
                                                                            \
/* Copy object including all internal objects and state                 */  \
FFTFILT() FFTFILT(_copy)(FFTFILT() _q);                                     \
                                                                            \
/* Destroy filter object and free all internal memory                   */  \
int FFTFILT(_destroy)(FFTFILT() _q);                                        \
                                                                            \
/* Reset filter object's internal buffer                                */  \
int FFTFILT(_reset)(FFTFILT() _q);                                          \
                                                                            \
/* Print filter object information to stdout                            */  \
int FFTFILT(_print)(FFTFILT() _q);                                          \
                                                                            \
/* Set output scaling for filter                                        */  \
int FFTFILT(_set_scale)(FFTFILT() _q,                                       \
                        TC        _scale);                                  \
                                                                            \
/* Get output scaling for filter                                        */  \
int FFTFILT(_get_scale)(FFTFILT() _q,                                       \
                        TC *      _scale);                                  \
                                                                            \
/* Execute the filter on internal buffer and coefficients given a block */  \
/* of input samples; in-place operation is permitted (_x and _y may     */  \
/* point to the same place in memory)                                   */  \
/*  _q      : filter object                                             */  \
/*  _x      : pointer to input data array,  [size: _n x 1]              */  \
/*  _y      : pointer to output data array, [size: _n x 1]              */  \
int FFTFILT(_execute)(FFTFILT() _q,                                         \
                      TI *      _x,                                         \
                      TO *      _y);                                        \
                                                                            \
/* Get length of filter object's internal coefficients                  */  \
unsigned int FFTFILT(_get_length)(FFTFILT() _q);                            \

LIQUID_FFTFILT_DEFINE_API(LIQUID_FFTFILT_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_FFTFILT_DEFINE_API(LIQUID_FFTFILT_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)

LIQUID_FFTFILT_DEFINE_API(LIQUID_FFTFILT_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)


//
// Infinite impulse response filter
//

#define LIQUID_IIRFILT_MANGLE_RRRF(name) LIQUID_CONCAT(iirfilt_rrrf,name)
#define LIQUID_IIRFILT_MANGLE_CRCF(name) LIQUID_CONCAT(iirfilt_crcf,name)
#define LIQUID_IIRFILT_MANGLE_CCCF(name) LIQUID_CONCAT(iirfilt_cccf,name)

// Macro:
//   IIRFILT : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_IIRFILT_DEFINE_API(IIRFILT,TO,TC,TI)                         \
                                                                            \
/* Infinite impulse response (IIR) filter                               */  \
typedef struct IIRFILT(_s) * IIRFILT();                                     \
                                                                            \
/* Create infinite impulse response filter from external coefficients.  */  \
/* Note that the number of feed-forward and feed-back coefficients do   */  \
/* not need to be equal, but they do need to be non-zero.               */  \
/* Furthermore, the first feed-back coefficient \(a_0\) cannot be       */  \
/* equal to zero, otherwise the filter will be invalid as this value is */  \
/* factored out from all coefficients.                                  */  \
/* For stability reasons the number of coefficients should reasonably   */  \
/* not exceed about 8 for single-precision floating-point.              */  \
/*  _b      : feed-forward coefficients (numerator), [size: _nb x 1]    */  \
/*  _nb     : number of feed-forward coefficients, _nb > 0              */  \
/*  _a      : feed-back coefficients (denominator), [size: _na x 1]     */  \
/*  _na     : number of feed-back coefficients, _na > 0                 */  \
IIRFILT() IIRFILT(_create)(TC *         _b,                                 \
                           unsigned int _nb,                                \
                           TC *         _a,                                 \
                           unsigned int _na);                               \
                                                                            \
/* Create IIR filter using 2nd-order sections from external             */  \
/* coefficients.                                                        */  \
/*  _b      : feed-forward coefficients, [size: _nsos x 3]              */  \
/*  _a      : feed-back coefficients,    [size: _nsos x 3]              */  \
/*  _nsos   : number of second-order sections (sos), _nsos > 0          */  \
IIRFILT() IIRFILT(_create_sos)(TC *         _b,                             \
                               TC *         _a,                             \
                               unsigned int _nsos);                         \
                                                                            \
/* Create IIR filter from design template                               */  \
/*  _ftype  : filter type (e.g. LIQUID_IIRDES_BUTTER)                   */  \
/*  _btype  : band type (e.g. LIQUID_IIRDES_BANDPASS)                   */  \
/*  _format : coefficients format (e.g. LIQUID_IIRDES_SOS)              */  \
/*  _order  : filter order, _order > 0                                  */  \
/*  _fc     : low-pass prototype cut-off frequency, 0 <= _fc <= 0.5     */  \
/*  _f0     : center frequency (band-pass, band-stop), 0 <= _f0 <= 0.5  */  \
/*  _ap     : pass-band ripple in dB, _ap > 0                           */  \
/*  _as     : stop-band ripple in dB, _as > 0                           */  \
IIRFILT() IIRFILT(_create_prototype)(                                       \
            liquid_iirdes_filtertype _ftype,                                \
            liquid_iirdes_bandtype   _btype,                                \
            liquid_iirdes_format     _format,                               \
            unsigned int             _order,                                \
            float                    _fc,                                   \
            float                    _f0,                                   \
            float                    _ap,                                   \
            float                    _as);                                  \
                                                                            \
/* Create simplified low-pass Butterworth IIR filter                    */  \
/*  _order  : filter order, _order > 0                                  */  \
/*  _fc     : low-pass prototype cut-off frequency                      */  \
IIRFILT() IIRFILT(_create_lowpass)(unsigned int _order,                     \
                                   float        _fc);                       \
                                                                            \
/* Create 8th-order integrator filter                                   */  \
IIRFILT() IIRFILT(_create_integrator)(void);                                \
                                                                            \
/* Create 8th-order differentiator filter                               */  \
IIRFILT() IIRFILT(_create_differentiator)(void);                            \
                                                                            \
/* Create simple first-order DC-blocking filter with transfer function  */  \
/* \( H(z) = \frac{1 - z^{-1}}{1 - (1-\alpha)z^{-1}} \)                 */  \
/*  _alpha  : normalized filter bandwidth, _alpha > 0                   */  \
IIRFILT() IIRFILT(_create_dc_blocker)(float _alpha);                        \
                                                                            \
/* Create filter to operate as second-order integrating phase-locked    */  \
/* loop (active lag design)                                             */  \
/*  _w      : filter bandwidth, 0 < _w < 1                              */  \
/*  _zeta   : damping factor, \( 1/\sqrt{2} \) suggested, 0 < _zeta < 1 */  \
/*  _K      : loop gain, 1000 suggested, _K > 0                         */  \
IIRFILT() IIRFILT(_create_pll)(float _w,                                    \
                               float _zeta,                                 \
                               float _K);                                   \
                                                                            \
/* Copy object including all internal objects and state                 */  \
IIRFILT() IIRFILT(_copy)(IIRFILT() _q);                                     \
                                                                            \
/* Destroy iirfilt object, freeing all internal memory                  */  \
int IIRFILT(_destroy)(IIRFILT() _q);                                        \
                                                                            \
/* Print iirfilt object properties to stdout                            */  \
int IIRFILT(_print)(IIRFILT() _q);                                          \
                                                                            \
/* Reset iirfilt object internals                                       */  \
int IIRFILT(_reset)(IIRFILT() _q);                                          \
                                                                            \
/* Set output scaling for filter                                        */  \
/*  _q      : filter object                                             */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int IIRFILT(_set_scale)(IIRFILT() _q, TC _scale);                           \
                                                                            \
/* Get output scaling for filter                                        */  \
/*  _q      : filter object                                             */  \
/*  _scale  : scaling factor applied to each output sample              */  \
int IIRFILT(_get_scale)(IIRFILT() _q, TC * _scale);                         \
                                                                            \
/* Compute filter output given a single input sample                    */  \
/*  _q      : iirfilt object                                            */  \
/*  _x      : input sample                                              */  \
/*  _y      : output sample pointer                                     */  \
int IIRFILT(_execute)(IIRFILT() _q,                                         \
                      TI        _x,                                         \
                      TO *      _y);                                        \
                                                                            \
/* Execute the filter on a block of input samples;                      */  \
/* in-place operation is permitted (the input and output buffers may be */  \
/* the same)                                                            */  \
/*  _q      : filter object                                             */  \
/*  _x      : pointer to input array, [size: _n x 1]                    */  \
/*  _n      : number of input, output samples, _n > 0                   */  \
/*  _y      : pointer to output array, [size: _n x 1]                   */  \
int IIRFILT(_execute_block)(IIRFILT()    _q,                                \
                            TI *         _x,                                \
                            unsigned int _n,                                \
                            TO *         _y);                               \
                                                                            \
/* Return number of coefficients for iirfilt object (maximum between    */  \
/* the feed-forward and feed-back coefficients). Note that the filter   */  \
/* length = filter order + 1                                            */  \
unsigned int IIRFILT(_get_length)(IIRFILT() _q);                            \
                                                                            \
/* Compute complex frequency response of filter object                  */  \
/*  _q      : filter object                                             */  \
/*  _fc     : normalized frequency for evaluation                       */  \
/*  _H      : pointer to output complex frequency response              */  \
int IIRFILT(_freqresponse)(IIRFILT()              _q,                       \
                           float                  _fc,                      \
                           liquid_float_complex * _H);                      \
                                                                            \
/* Compute power spectral density response of filter object in dB       */  \
/*  _q      : filter object                                             */  \
/*  _fc     : normalized frequency for evaluation                       */  \
float IIRFILT(_get_psd)(IIRFILT() _q,                                       \
                        float     _fc);                                     \
                                                                            \
/* Compute and return group delay of filter object                      */  \
/*  _q      : filter object                                             */  \
/*  _fc     : frequency to evaluate                                     */  \
float IIRFILT(_groupdelay)(IIRFILT() _q, float _fc);                        \

LIQUID_IIRFILT_DEFINE_API(LIQUID_IIRFILT_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_IIRFILT_DEFINE_API(LIQUID_IIRFILT_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)

LIQUID_IIRFILT_DEFINE_API(LIQUID_IIRFILT_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)

//
// iirfiltsos : infinite impulse response filter (second-order sections)
//
#define LIQUID_IIRFILTSOS_MANGLE_RRRF(name)  LIQUID_CONCAT(iirfiltsos_rrrf,name)
#define LIQUID_IIRFILTSOS_MANGLE_CRCF(name)  LIQUID_CONCAT(iirfiltsos_crcf,name)
#define LIQUID_IIRFILTSOS_MANGLE_CCCF(name)  LIQUID_CONCAT(iirfiltsos_cccf,name)

#define LIQUID_IIRFILTSOS_DEFINE_API(IIRFILTSOS,TO,TC,TI)                   \
                                                                            \
/* Infinite impulse response filter primitive using second-order        */  \
/* sections                                                             */  \
typedef struct IIRFILTSOS(_s) * IIRFILTSOS();                               \
                                                                            \
/* create 2nd-order infinite impulse response filter                    */  \
/*  _b      : feed-forward coefficients, [size: _3 x 1]                 */  \
/*  _a      : feed-back coefficients,    [size: _3 x 1]                 */  \
IIRFILTSOS() IIRFILTSOS(_create)(TC * _b,                                   \
                                 TC * _a);                                  \
                                                                            \
/* Copy object including all internal objects and state                 */  \
IIRFILTSOS() IIRFILTSOS(_copy)(IIRFILTSOS() _q);                            \
                                                                            \
/* explicitly set 2nd-order IIR filter coefficients                     */  \
/*  _q      : iirfiltsos object                                         */  \
/*  _b      : feed-forward coefficients, [size: 3 x 1]                  */  \
/*  _a      : feed-back coefficients, [size: 3 x 1]                     */  \
int IIRFILTSOS(_set_coefficients)(IIRFILTSOS() _q,                          \
                                  TC *         _b,                          \
                                  TC *         _a);                         \
                                                                            \
/* destroy iirfiltsos object, freeing all internal memory               */  \
int IIRFILTSOS(_destroy)(IIRFILTSOS() _q);                                  \
                                                                            \
/* print iirfiltsos object properties to stdout                         */  \
int IIRFILTSOS(_print)(IIRFILTSOS() _q);                                    \
                                                                            \
/* clear/reset iirfiltsos object internals                              */  \
int IIRFILTSOS(_reset)(IIRFILTSOS() _q);                                    \
                                                                            \
/* compute filter output                                                */  \
/*  _q      : iirfiltsos object                                         */  \
/*  _x      : input sample                                              */  \
/*  _y      : output sample pointer                                     */  \
int IIRFILTSOS(_execute)(IIRFILTSOS() _q,                                   \
                         TI           _x,                                   \
                         TO *         _y);                                  \
                                                                            \
/* compute filter output, direct-form I method                          */  \
/*  _q      : iirfiltsos object                                         */  \
/*  _x      : input sample                                              */  \
/*  _y      : output sample pointer                                     */  \
int IIRFILTSOS(_execute_df1)(IIRFILTSOS() _q,                               \
                             TI           _x,                               \
                             TO *         _y);                              \
                                                                            \
/* compute filter output, direct-form II method                         */  \
/*  _q      : iirfiltsos object                                         */  \
/*  _x      : input sample                                              */  \
/*  _y      : output sample pointer                                     */  \
int IIRFILTSOS(_execute_df2)(IIRFILTSOS() _q,                               \
                             TI           _x,                               \
                             TO *         _y);                              \
                                                                            \
/* compute and return group delay of filter object                      */  \
/*  _q      : filter object                                             */  \
/*  _fc     : frequency to evaluate                                     */  \
float IIRFILTSOS(_groupdelay)(IIRFILTSOS() _q,                              \
                              float        _fc);                            \

LIQUID_IIRFILTSOS_DEFINE_API(LIQUID_IIRFILTSOS_MANGLE_RRRF,
                                      float,
                                      float,
                                      float)

LIQUID_IIRFILTSOS_DEFINE_API(LIQUID_IIRFILTSOS_MANGLE_CRCF,
                                      liquid_float_complex,
                                      float,
                                      liquid_float_complex)

LIQUID_IIRFILTSOS_DEFINE_API(LIQUID_IIRFILTSOS_MANGLE_CCCF,
                                      liquid_float_complex,
                                      liquid_float_complex,
                                      liquid_float_complex)

//
// FIR Polyphase filter bank
//
#define LIQUID_FIRPFB_MANGLE_RRRF(name) LIQUID_CONCAT(firpfb_rrrf,name)
#define LIQUID_FIRPFB_MANGLE_CRCF(name) LIQUID_CONCAT(firpfb_crcf,name)
#define LIQUID_FIRPFB_MANGLE_CCCF(name) LIQUID_CONCAT(firpfb_cccf,name)

// Macro:
//   FIRPFB : name-mangling macro
//   TO     : output data type
//   TC     : coefficients data type
//   TI     : input data type
#define LIQUID_FIRPFB_DEFINE_API(FIRPFB,TO,TC,TI)                           \
                                                                            \
/* Finite impulse response (FIR) polyphase filter bank (PFB)            */  \
typedef struct FIRPFB(_s) * FIRPFB();                                       \
                                                                            \
/* Create firpfb object with _num_filters sub-filters each having       */  \
/* exactly _h_len/_num_filters coefficients                             */  \
/* from an external array of coefficients                               */  \
/*  _num_filters    : number of filters in the bank, _num_filters > 1   */  \
/*  _h              : coefficients, [size: _h_len x 1]                  */  \
/*  _h_len          : complete filter length (where _h_len is a         */  \
/*                    multiple of _num_filters), _h_len >= _num_filters */  \
FIRPFB() FIRPFB(_create)(unsigned int _num_filters,                         \
                         TC *         _h,                                   \
                         unsigned int _h_len);                              \
                                                                            \
/* Create firpfb object using Kaiser-Bessel windowed sinc filter design */  \
/* method, using default values for cut-off frequency and stop-band     */  \
/* attenuation. This is equivalent to:                                  */  \
/*   FIRPFB(_create_kaiser)(_M, _m, 0.5, 60.0)                          */  \
/* which creates a Nyquist filter at the appropriate cut-off frequency. */  \
/*  _num_filters    : number of filters in the bank, _num_filters > 1   */  \
/*  _m              : filter semi-length [samples], _m > 0              */  \
FIRPFB() FIRPFB(_create_default)(unsigned int _num_filters,                 \
                                 unsigned int _m);                          \
                                                                            \
/* Create firpfb object using Kaiser-Bessel windowed sinc filter design */  \
/* method                                                               */  \
/*  _num_filters : number of filters in the bank, _num_filters > 1      */  \
/*  _m           : filter semi-length [samples], _m > 0                 */  \
/*  _fc          : filter normalized cut-off frequency, 0 < _fc < 0.5   */  \
/*  _as          : filter stop-band suppression [dB], _as > 0           */  \
FIRPFB() FIRPFB(_create_kaiser)(unsigned int _num_filters,                  \
                                unsigned int _m,                            \
                                float        _fc,                           \
                                float        _as);                          \
                                                                            \
/* Create firpfb from square-root Nyquist prototype                     */  \
/*  _type        : filter type (e.g. LIQUID_FIRFILT_RRC)                */  \
/*  _num_filters : number of filters in the bank, _num_filters > 1      */  \
/*  _k           : nominal samples/symbol, _k > 1                       */  \
/*  _m           : filter delay [symbols], _m > 0                       */  \
/*  _beta        : rolloff factor, 0 < _beta <= 1                       */  \
FIRPFB() FIRPFB(_create_rnyquist)(int          _type,                       \
                                  unsigned int _num_filters,                \
                                  unsigned int _k,                          \
                                  unsigned int _m,                          \
                                  float        _beta);                      \
                                                                            \
/* Create from square-root derivative Nyquist prototype                 */  \
/*  _type        : filter type (e.g. LIQUID_FIRFILT_RRC)                */  \
/*  _num_filters : number of filters in the bank, _num_filters > 1      */  \
/*  _k           : nominal samples/symbol, _k > 1                       */  \
/*  _m           : filter delay [symbols], _m > 0                       */  \
/*  _beta        : rolloff factor, 0 < _beta <= 1                       */  \
FIRPFB() FIRPFB(_create_drnyquist)(int          _type,                      \
                                   unsigned int _num_filters,               \
                                   unsigned int _k,                         \
                                   unsigned int _m,                         \
                                   float        _beta);                     \
                                                                            \
/* Re-create firpfb object of potentially a different length with       */  \
/* different coefficients. If the length of the filter does not change, */  \
/* not memory reallocation is invoked.                                  */  \
/*  _q           : original firpfb object                               */  \
/*  _num_filters : number of filters in the bank, _num_filters > 1      */  \
/*  _h           : coefficients, [size: _h_len x 1]                     */  \
/*  _h_len          : complete filter length (where _h_len is a         */  \
/*                    multiple of _num_filters), _h_len >= _num_filters */  \
FIRPFB() FIRPFB(_recreate)(FIRPFB()     _q,                                 \
                           unsigned int _num_filters,                       \
                           TC *         _h,                                 \
                           unsigned int _h_len);                            \
                                                                            \
/* Copy object including all internal objects and state                 */  \
FIRPFB() FIRPFB(_copy)(FIRPFB() _q);                                        \
                                                                            \
/* Destroy firpfb object, freeing all internal memory and destroying    */  \
/* all internal objects                                                 */  \
int FIRPFB(_destroy)(FIRPFB() _q);                                          \
                                                                            \
/* Print firpfb object's parameters to stdout                           */  \
int FIRPFB(_print)(FIRPFB() _q);                                            \
                                                                            \
/* Set output scaling for filter                                        */  \
/*  _q      : filter object                                             */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int FIRPFB(_set_scale)(FIRPFB() _q,                                         \
                        TC       _scale);                                   \
                                                                            \
/* Get output scaling for filter                                        */  \
/*  _q      : filter object                                             */  \
/*  _scale  : scaling factor applied to each output sample              */  \
int FIRPFB(_get_scale)(FIRPFB() _q,                                         \
                       TC *     _scale);                                    \
                                                                            \
/* Reset firpfb object's internal buffer                                */  \
int FIRPFB(_reset)(FIRPFB() _q);                                            \
                                                                            \
/* Push sample into filter object's internal buffer                     */  \
/*  _q      : filter object                                             */  \
/*  _x      : single input sample                                       */  \
int FIRPFB(_push)(FIRPFB() _q,                                              \
                  TI       _x);                                             \
                                                                            \
/* Write a block of samples into object's internal buffer               */  \
/*  _q      : filter object                                             */  \
/*  _x      : single input sample                                       */  \
int FIRPFB(_write)(FIRPFB()     _q,                                         \
                   TI *         _x,                                         \
                   unsigned int _n);                                        \
                                                                            \
/* Execute vector dot product on the filter's internal buffer and       */  \
/* coefficients using the coefficients from sub-filter at index _i      */  \
/*  _q      : firpfb object                                             */  \
/*  _i      : index of filter to use                                    */  \
/*  _y      : pointer to output sample                                  */  \
int FIRPFB(_execute)(FIRPFB()     _q,                                       \
                     unsigned int _i,                                       \
                     TO *         _y);                                      \
                                                                            \
/* Execute the filter on a block of input samples, all using index _i.  */  \
/* In-place operation is permitted (_x and _y may point to the same     */  \
/* place in memory)                                                     */  \
/*  _q      : firpfb object                                             */  \
/*  _i      : index of filter to use                                    */  \
/*  _x      : pointer to input array, [size: _n x 1]                    */  \
/*  _n      : number of input, output samples                           */  \
/*  _y      : pointer to output array, [size: _n x 1]                   */  \
int FIRPFB(_execute_block)(FIRPFB()     _q,                                 \
                           unsigned int _i,                                 \
                           TI *         _x,                                 \
                           unsigned int _n,                                 \
                           TO *         _y);                                \

LIQUID_FIRPFB_DEFINE_API(LIQUID_FIRPFB_MANGLE_RRRF,
                         float,
                         float,
                         float)

LIQUID_FIRPFB_DEFINE_API(LIQUID_FIRPFB_MANGLE_CRCF,
                         liquid_float_complex,
                         float,
                         liquid_float_complex)

LIQUID_FIRPFB_DEFINE_API(LIQUID_FIRPFB_MANGLE_CCCF,
                         liquid_float_complex,
                         liquid_float_complex,
                         liquid_float_complex)

//
// Interpolators
//

// firinterp : finite impulse response interpolator
#define LIQUID_FIRINTERP_MANGLE_RRRF(name) LIQUID_CONCAT(firinterp_rrrf,name)
#define LIQUID_FIRINTERP_MANGLE_CRCF(name) LIQUID_CONCAT(firinterp_crcf,name)
#define LIQUID_FIRINTERP_MANGLE_CCCF(name) LIQUID_CONCAT(firinterp_cccf,name)

#define LIQUID_FIRINTERP_DEFINE_API(FIRINTERP,TO,TC,TI)                     \
                                                                            \
/* Finite impulse response (FIR) interpolator                           */  \
typedef struct FIRINTERP(_s) * FIRINTERP();                                 \
                                                                            \
/* Create interpolator from external coefficients. Internally the       */  \
/* interpolator creates a polyphase filter bank to efficiently realize  */  \
/* resampling of the input signal.                                      */  \
/* If the input filter length is not a multiple of the interpolation    */  \
/* factor \(M\), the object internally pads the coefficients with zeros */  \
/* to compensate.                                                       */  \
/*  _interp : interpolation factor \(M\), _interp >= 2                  */  \
/*  _h      : filter coefficients, [size: _h_len x 1]                   */  \
/*  _h_len  : filter length, _h_len >= _interp                          */  \
FIRINTERP() FIRINTERP(_create)(unsigned int _interp,                        \
                               TC *         _h,                             \
                               unsigned int _h_len);                        \
                                                                            \
/* Create interpolator from filter prototype prototype (Kaiser-Bessel   */  \
/* windowed-sinc function)                                              */  \
/*  _interp : interpolation factor \(M\), _interp >= 2                  */  \
/*  _m      : filter delay [symbols], _m >= 1                           */  \
/*  _as     : stop-band attenuation [dB], _as >= 0                      */  \
FIRINTERP() FIRINTERP(_create_kaiser)(unsigned int _interp,                 \
                                      unsigned int _m,                      \
                                      float        _as);                    \
                                                                            \
/* Create interpolator object from filter prototype                     */  \
/*  _type   : filter type (e.g. LIQUID_FIRFILT_RCOS)                    */  \
/*  _interp : interpolation factor \(M\), _interp >= 2                  */  \
/*  _m      : filter delay (symbols),  _m > 0                           */  \
/*  _beta   : excess bandwidth factor, 0 <= _beta <= 1                  */  \
/*  _dt     : fractional sample delay, -1 <= _dt <= 1                   */  \
FIRINTERP() FIRINTERP(_create_prototype)(int          _type,                \
                                         unsigned int _interp,              \
                                         unsigned int _m,                   \
                                         float        _beta,                \
                                         float        _dt);                 \
                                                                            \
/* Create linear interpolator object                                    */  \
/*  _interp : interpolation factor \(M\), _interp >= 2                  */  \
FIRINTERP() FIRINTERP(_create_linear)(unsigned int _interp);                \
                                                                            \
/* Create window interpolator object                                    */  \
/*  _interp : interpolation factor \(M\), _interp >= 2                  */  \
/*  _m      : filter semi-length, _m > 0                                */  \
FIRINTERP() FIRINTERP(_create_window)(unsigned int _interp,                 \
                                      unsigned int _m);                     \
                                                                            \
/* Copy object including all internal objects and state                 */  \
FIRINTERP() FIRINTERP(_copy)(FIRINTERP() _q);                               \
                                                                            \
/* Destroy firinterp object, freeing all internal memory                */  \
int FIRINTERP(_destroy)(FIRINTERP() _q);                                    \
                                                                            \
/* Print firinterp object's internal properties to stdout               */  \
int FIRINTERP(_print)(FIRINTERP() _q);                                      \
                                                                            \
/* Reset internal state                                                 */  \
int FIRINTERP(_reset)(FIRINTERP() _q);                                      \
                                                                            \
/* Get interpolation rate                                               */  \
unsigned int FIRINTERP(_get_interp_rate)(FIRINTERP() _q);                   \
                                                                            \
/* Get sub-filter length (length of each poly-phase filter)             */  \
unsigned int FIRINTERP(_get_sub_len)(FIRINTERP() _q);                       \
                                                                            \
/* Set output scaling for interpolator                                  */  \
/*  _q      : interpolator object                                       */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int FIRINTERP(_set_scale)(FIRINTERP() _q,                                   \
                          TC          _scale);                              \
                                                                            \
/* Get output scaling for interpolator                                  */  \
/*  _q      : interpolator object                                       */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int FIRINTERP(_get_scale)(FIRINTERP() _q,                                   \
                          TC *        _scale);                              \
                                                                            \
/* Execute interpolation on single input sample and write \(M\) output  */  \
/* samples (\(M\) is the interpolation factor)                          */  \
/*  _q      : firinterp object                                          */  \
/*  _x      : input sample                                              */  \
/*  _y      : output sample array, [size: M x 1]                        */  \
int FIRINTERP(_execute)(FIRINTERP() _q,                                     \
                        TI          _x,                                     \
                        TO *        _y);                                    \
                                                                            \
/* Execute interpolation on block of input samples, increasing the      */  \
/* sample rate of the input by the interpolation factor \(M\).          */  \
/*  _q      : firinterp object                                          */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : size of input array                                       */  \
/*  _y      : output sample array, [size: M*_n x 1]                     */  \
int FIRINTERP(_execute_block)(FIRINTERP()  _q,                              \
                              TI *         _x,                              \
                              unsigned int _n,                              \
                              TO *         _y);                             \
                                                                            \
/* Execute interpolation with zero-valued input (e.g. flush internal    */  \
/* state)                                                               */  \
/*  _q      : firinterp object                                          */  \
/*  _y      : output sample array, [size: M x 1]                        */  \
int FIRINTERP(_flush)(FIRINTERP() _q,                                       \
                      TO *        _y);                                      \

LIQUID_FIRINTERP_DEFINE_API(LIQUID_FIRINTERP_MANGLE_RRRF,
                            float,
                            float,
                            float)

LIQUID_FIRINTERP_DEFINE_API(LIQUID_FIRINTERP_MANGLE_CRCF,
                            liquid_float_complex,
                            float,
                            liquid_float_complex)

LIQUID_FIRINTERP_DEFINE_API(LIQUID_FIRINTERP_MANGLE_CCCF,
                            liquid_float_complex,
                            liquid_float_complex,
                            liquid_float_complex)

// iirinterp : infinite impulse response interpolator
#define LIQUID_IIRINTERP_MANGLE_RRRF(name) LIQUID_CONCAT(iirinterp_rrrf,name)
#define LIQUID_IIRINTERP_MANGLE_CRCF(name) LIQUID_CONCAT(iirinterp_crcf,name)
#define LIQUID_IIRINTERP_MANGLE_CCCF(name) LIQUID_CONCAT(iirinterp_cccf,name)

#define LIQUID_IIRINTERP_DEFINE_API(IIRINTERP,TO,TC,TI)                     \
                                                                            \
/* Infinite impulse response (IIR) interpolator                         */  \
typedef struct IIRINTERP(_s) * IIRINTERP();                                 \
                                                                            \
/* Create infinite impulse response interpolator from external          */  \
/* coefficients.                                                        */  \
/* Note that the number of feed-forward and feed-back coefficients do   */  \
/* not need to be equal, but they do need to be non-zero.               */  \
/* Furthermore, the first feed-back coefficient \(a_0\) cannot be       */  \
/* equal to zero, otherwise the filter will be invalid as this value is */  \
/* factored out from all coefficients.                                  */  \
/* For stability reasons the number of coefficients should reasonably   */  \
/* not exceed about 8 for single-precision floating-point.              */  \
/*  _M      : interpolation factor, _M >= 2                             */  \
/*  _b      : feed-forward coefficients (numerator), [size: _nb x 1]    */  \
/*  _nb     : number of feed-forward coefficients, _nb > 0              */  \
/*  _a      : feed-back coefficients (denominator), [size: _na x 1]     */  \
/*  _na     : number of feed-back coefficients, _na > 0                 */  \
IIRINTERP() IIRINTERP(_create)(unsigned int _M,                             \
                               TC *         _b,                             \
                               unsigned int _nb,                            \
                               TC *         _a,                             \
                               unsigned int _na);                           \
                                                                            \
/* Create interpolator object with default Butterworth prototype        */  \
/*  _M      : interpolation factor, _M >= 2                             */  \
/*  _order  : filter order, _order > 0                                  */  \
IIRINTERP() IIRINTERP(_create_default)(unsigned int _M,                     \
                                       unsigned int _order);                \
                                                                            \
/* Create IIR interpolator from prototype                               */  \
/*  _M      : interpolation factor, _M >= 2                             */  \
/*  _ftype  : filter type (e.g. LIQUID_IIRDES_BUTTER)                   */  \
/*  _btype  : band type (e.g. LIQUID_IIRDES_BANDPASS)                   */  \
/*  _format : coefficients format (e.g. LIQUID_IIRDES_SOS)              */  \
/*  _order  : filter order, _order > 0                                  */  \
/*  _fc     : low-pass prototype cut-off frequency, 0 <= _fc <= 0.5     */  \
/*  _f0     : center frequency (band-pass, band-stop), 0 <= _f0 <= 0.5  */  \
/*  _ap     : pass-band ripple in dB, _ap > 0                           */  \
/*  _as     : stop-band ripple in dB, _as > 0                           */  \
IIRINTERP() IIRINTERP(_create_prototype)(                                   \
            unsigned int             _M,                                    \
            liquid_iirdes_filtertype _ftype,                                \
            liquid_iirdes_bandtype   _btype,                                \
            liquid_iirdes_format     _format,                               \
            unsigned int             _order,                                \
            float                    _fc,                                   \
            float                    _f0,                                   \
            float                    _ap,                                   \
            float                    _as);                                  \
                                                                            \
/* Copy object including all internal objects and state                 */  \
IIRINTERP() IIRINTERP(_copy)(IIRINTERP() _q);                               \
                                                                            \
/* Destroy interpolator object and free internal memory                 */  \
int IIRINTERP(_destroy)(IIRINTERP() _q);                                    \
                                                                            \
/* Print interpolator object internals to stdout                        */  \
int IIRINTERP(_print)(IIRINTERP() _q);                                      \
                                                                            \
/* Reset interpolator object                                            */  \
int IIRINTERP(_reset)(IIRINTERP() _q);                                      \
                                                                            \
/* Execute interpolation on single input sample and write \(M\) output  */  \
/* samples (\(M\) is the interpolation factor)                          */  \
/*  _q      : iirinterp object                                          */  \
/*  _x      : input sample                                              */  \
/*  _y      : output sample array, [size: _M x 1]                       */  \
int IIRINTERP(_execute)(IIRINTERP() _q,                                     \
                        TI          _x,                                     \
                        TO *        _y);                                    \
                                                                            \
/* Execute interpolation on block of input samples                      */  \
/*  _q      : iirinterp object                                          */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : size of input array                                       */  \
/*  _y      : output sample array, [size: _M*_n x 1]                    */  \
int IIRINTERP(_execute_block)(IIRINTERP()  _q,                              \
                              TI *         _x,                              \
                              unsigned int _n,                              \
                              TO *         _y);                             \
                                                                            \
/* Compute and return group delay of object                             */  \
/*  _q      : filter object                                             */  \
/*  _fc     : frequency to evaluate                                     */  \
float IIRINTERP(_groupdelay)(IIRINTERP() _q,                                \
                             float       _fc);                              \

LIQUID_IIRINTERP_DEFINE_API(LIQUID_IIRINTERP_MANGLE_RRRF,
                            float,
                            float,
                            float)

LIQUID_IIRINTERP_DEFINE_API(LIQUID_IIRINTERP_MANGLE_CRCF,
                            liquid_float_complex,
                            float,
                            liquid_float_complex)

LIQUID_IIRINTERP_DEFINE_API(LIQUID_IIRINTERP_MANGLE_CCCF,
                            liquid_float_complex,
                            liquid_float_complex,
                            liquid_float_complex)

//
// Decimators
//

// firdecim : finite impulse response decimator
#define LIQUID_FIRDECIM_MANGLE_RRRF(name) LIQUID_CONCAT(firdecim_rrrf,name)
#define LIQUID_FIRDECIM_MANGLE_CRCF(name) LIQUID_CONCAT(firdecim_crcf,name)
#define LIQUID_FIRDECIM_MANGLE_CCCF(name) LIQUID_CONCAT(firdecim_cccf,name)

#define LIQUID_FIRDECIM_DEFINE_API(FIRDECIM,TO,TC,TI)                       \
                                                                            \
/* Finite impulse response (FIR) decimator                              */  \
typedef struct FIRDECIM(_s) * FIRDECIM();                                   \
                                                                            \
/* Create decimator from external coefficients                          */  \
/*  _M      : decimation factor, _M >= 2                                */  \
/*  _h      : filter coefficients, [size: _h_len x 1]                   */  \
/*  _h_len  : filter length, _h_len >= _M                               */  \
FIRDECIM() FIRDECIM(_create)(unsigned int _M,                               \
                             TC *         _h,                               \
                             unsigned int _h_len);                          \
                                                                            \
/* Create decimator from filter prototype prototype (Kaiser-Bessel      */  \
/* windowed-sinc function)                                              */  \
/*  _M      : decimation factor, _M >= 2                                */  \
/*  _m      : filter delay [symbols], _m >= 1                           */  \
/*  _as     : stop-band attenuation [dB], _as >= 0                      */  \
FIRDECIM() FIRDECIM(_create_kaiser)(unsigned int _M,                        \
                                    unsigned int _m,                        \
                                    float        _as);                      \
                                                                            \
/* Create decimator object from filter prototype                        */  \
/*  _type   : filter type (e.g. LIQUID_FIRFILT_RCOS)                    */  \
/*  _M      : interpolation factor,    _M > 1                           */  \
/*  _m      : filter delay (symbols),  _m > 0                           */  \
/*  _beta   : excess bandwidth factor, 0 <= _beta <= 1                  */  \
/*  _dt     : fractional sample delay, -1 <= _dt <= 1                   */  \
FIRDECIM() FIRDECIM(_create_prototype)(int          _type,                  \
                                       unsigned int _M,                     \
                                       unsigned int _m,                     \
                                       float        _beta,                  \
                                       float        _dt);                   \
                                                                            \
/* Copy object including all internal objects and state                 */  \
FIRDECIM() FIRDECIM(_copy)(FIRDECIM() _q);                                  \
                                                                            \
/* Destroy decimator object, freeing all internal memory                */  \
int FIRDECIM(_destroy)(FIRDECIM() _q);                                      \
                                                                            \
/* Print decimator object properties to stdout                          */  \
int FIRDECIM(_print)(FIRDECIM() _q);                                        \
                                                                            \
/* Reset decimator object internal state                                */  \
int FIRDECIM(_reset)(FIRDECIM() _q);                                        \
                                                                            \
/* Get decimation rate                                                  */  \
unsigned int FIRDECIM(_get_decim_rate)(FIRDECIM() _q);                      \
                                                                            \
/* Set output scaling for decimator                                     */  \
/*  _q      : decimator object                                          */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int FIRDECIM(_set_scale)(FIRDECIM() _q,                                     \
                         TC         _scale);                                \
                                                                            \
/* Get output scaling for decimator                                     */  \
/*  _q      : decimator object                                          */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int FIRDECIM(_get_scale)(FIRDECIM() _q,                                     \
                         TC *       _scale);                                \
                                                                            \
/* Compute complex frequency response \(H\) of decimator on prototype   */  \
/* filter coefficients at a specific frequency \(f_c\)                  */  \
/*  _q      : decimator object                                          */  \
/*  _fc     : normalized frequency for evaluation                       */  \
/*  _H      : pointer to output complex frequency response              */  \
int FIRDECIM(_freqresp)(FIRDECIM()             _q,                          \
                        float                  _fc,                         \
                        liquid_float_complex * _H);                         \
                                                                            \
/* Execute decimator on _M input samples                                */  \
/*  _q      : decimator object                                          */  \
/*  _x      : input samples, [size: _M x 1]                             */  \
/*  _y      : output sample pointer                                     */  \
int FIRDECIM(_execute)(FIRDECIM() _q,                                       \
                       TI *       _x,                                       \
                       TO *       _y);                                      \
                                                                            \
/* Execute decimator on block of _n*_M input samples                    */  \
/*  _q      : decimator object                                          */  \
/*  _x      : input array, [size: _n*_M x 1]                            */  \
/*  _n      : number of _output_ samples                                */  \
/*  _y      : output array, [_size: _n x 1]                             */  \
int FIRDECIM(_execute_block)(FIRDECIM()   _q,                               \
                             TI *         _x,                               \
                             unsigned int _n,                               \
                             TO *         _y);                              \

LIQUID_FIRDECIM_DEFINE_API(LIQUID_FIRDECIM_MANGLE_RRRF,
                           float,
                           float,
                           float)

LIQUID_FIRDECIM_DEFINE_API(LIQUID_FIRDECIM_MANGLE_CRCF,
                           liquid_float_complex,
                           float,
                           liquid_float_complex)

LIQUID_FIRDECIM_DEFINE_API(LIQUID_FIRDECIM_MANGLE_CCCF,
                           liquid_float_complex,
                           liquid_float_complex,
                           liquid_float_complex)


// iirdecim : infinite impulse response decimator
#define LIQUID_IIRDECIM_MANGLE_RRRF(name) LIQUID_CONCAT(iirdecim_rrrf,name)
#define LIQUID_IIRDECIM_MANGLE_CRCF(name) LIQUID_CONCAT(iirdecim_crcf,name)
#define LIQUID_IIRDECIM_MANGLE_CCCF(name) LIQUID_CONCAT(iirdecim_cccf,name)

#define LIQUID_IIRDECIM_DEFINE_API(IIRDECIM,TO,TC,TI)                       \
                                                                            \
/* Infinite impulse response (IIR) decimator                            */  \
typedef struct IIRDECIM(_s) * IIRDECIM();                                   \
                                                                            \
/* Create infinite impulse response decimator from external             */  \
/* coefficients.                                                        */  \
/* Note that the number of feed-forward and feed-back coefficients do   */  \
/* not need to be equal, but they do need to be non-zero.               */  \
/* Furthermore, the first feed-back coefficient \(a_0\) cannot be       */  \
/* equal to zero, otherwise the filter will be invalid as this value is */  \
/* factored out from all coefficients.                                  */  \
/* For stability reasons the number of coefficients should reasonably   */  \
/* not exceed about 8 for single-precision floating-point.              */  \
/*  _M      : decimation factor, _M >= 2                                */  \
/*  _b      : feed-forward coefficients (numerator), [size: _nb x 1]    */  \
/*  _nb     : number of feed-forward coefficients, _nb > 0              */  \
/*  _a      : feed-back coefficients (denominator), [size: _na x 1]     */  \
/*  _na     : number of feed-back coefficients, _na > 0                 */  \
IIRDECIM() IIRDECIM(_create)(unsigned int _M,                               \
                             TC *         _b,                               \
                             unsigned int _nb,                              \
                             TC *         _a,                               \
                             unsigned int _na);                             \
                                                                            \
/* Create decimator object with default Butterworth prototype           */  \
/*  _M      : decimation factor, _M >= 2                                */  \
/*  _order  : filter order, _order > 0                                  */  \
IIRDECIM() IIRDECIM(_create_default)(unsigned int _M,                       \
                                     unsigned int _order);                  \
                                                                            \
/* Create IIR decimator from prototype                                  */  \
/*  _M      : decimation factor, _M >= 2                                */  \
/*  _ftype  : filter type (e.g. LIQUID_IIRDES_BUTTER)                   */  \
/*  _btype  : band type (e.g. LIQUID_IIRDES_BANDPASS)                   */  \
/*  _format : coefficients format (e.g. LIQUID_IIRDES_SOS)              */  \
/*  _order  : filter order, _order > 0                                  */  \
/*  _fc     : low-pass prototype cut-off frequency, 0 <= _fc <= 0.5     */  \
/*  _f0     : center frequency (band-pass, band-stop), 0 <= _f0 <= 0.5  */  \
/*  _ap     : pass-band ripple in dB, _ap > 0                           */  \
/*  _as     : stop-band ripple in dB, _as > 0                           */  \
IIRDECIM() IIRDECIM(_create_prototype)(                                     \
                unsigned int             _M,                                \
                liquid_iirdes_filtertype _ftype,                            \
                liquid_iirdes_bandtype   _btype,                            \
                liquid_iirdes_format     _format,                           \
                unsigned int             _order,                            \
                float                    _fc,                               \
                float                    _f0,                               \
                float                    _ap,                               \
                float                    _as);                              \
                                                                            \
/* Copy object including all internal objects and state                 */  \
IIRDECIM() IIRDECIM(_copy)(IIRDECIM() _q);                                  \
                                                                            \
/* Destroy decimator object and free internal memory                    */  \
int IIRDECIM(_destroy)(IIRDECIM() _q);                                      \
                                                                            \
/* Print decimator object internals                                     */  \
int IIRDECIM(_print)(IIRDECIM() _q);                                        \
                                                                            \
/* Reset decimator object                                               */  \
int IIRDECIM(_reset)(IIRDECIM() _q);                                        \
                                                                            \
/* Execute decimator on _M input samples                                */  \
/*  _q      : decimator object                                          */  \
/*  _x      : input samples, [size: _M x 1]                             */  \
/*  _y      : output sample pointer                                     */  \
int IIRDECIM(_execute)(IIRDECIM() _q,                                       \
                       TI *       _x,                                       \
                       TO *       _y);                                      \
                                                                            \
/* Execute decimator on block of _n*_M input samples                    */  \
/*  _q      : decimator object                                          */  \
/*  _x      : input array, [size: _n*_M x 1]                            */  \
/*  _n      : number of _output_ samples                                */  \
/*  _y      : output array, [_sze: _n x 1]                              */  \
int IIRDECIM(_execute_block)(IIRDECIM()   _q,                               \
                             TI *         _x,                               \
                             unsigned int _n,                               \
                             TO *         _y);                              \
                                                                            \
/* Compute and return group delay of object                             */  \
/*  _q      : filter object                                             */  \
/*  _fc     : frequency to evaluate                                     */  \
float IIRDECIM(_groupdelay)(IIRDECIM() _q,                                  \
                            float      _fc);                                \

LIQUID_IIRDECIM_DEFINE_API(LIQUID_IIRDECIM_MANGLE_RRRF,
                           float,
                           float,
                           float)

LIQUID_IIRDECIM_DEFINE_API(LIQUID_IIRDECIM_MANGLE_CRCF,
                           liquid_float_complex,
                           float,
                           liquid_float_complex)

LIQUID_IIRDECIM_DEFINE_API(LIQUID_IIRDECIM_MANGLE_CCCF,
                           liquid_float_complex,
                           liquid_float_complex,
                           liquid_float_complex)



//
// Half-band resampler
//
#define LIQUID_RESAMP2_MANGLE_RRRF(name) LIQUID_CONCAT(resamp2_rrrf,name)
#define LIQUID_RESAMP2_MANGLE_CRCF(name) LIQUID_CONCAT(resamp2_crcf,name)
#define LIQUID_RESAMP2_MANGLE_CCCF(name) LIQUID_CONCAT(resamp2_cccf,name)

#define LIQUID_RESAMP2_DEFINE_API(RESAMP2,TO,TC,TI)                         \
                                                                            \
/* Half-band resampler, implemented as a dyadic (half-band) polyphase   */  \
/* filter bank for interpolation, decimation, synthesis, and analysis.  */  \
typedef struct RESAMP2(_s) * RESAMP2();                                     \
                                                                            \
/* Create half-band resampler from design prototype.                    */  \
/*  _m  : filter semi-length (h_len = 4*m+1), _m >= 2                   */  \
/*  _f0 : filter center frequency, -0.5 <= _f0 <= 0.5                   */  \
/*  _as : stop-band attenuation [dB], _as > 0                           */  \
RESAMP2() RESAMP2(_create)(unsigned int _m,                                 \
                           float        _f0,                                \
                           float        _as);                               \
                                                                            \
/* Re-create half-band resampler with new properties                    */  \
/*  _q  : original half-band resampler object                           */  \
/*  _m  : filter semi-length (h_len = 4*m+1), _m >= 2                   */  \
/*  _f0 : filter center frequency, -0.5 <= _f0 <= 0.5                   */  \
/*  _as : stop-band attenuation [dB], _as > 0                           */  \
RESAMP2() RESAMP2(_recreate)(RESAMP2()    _q,                               \
                             unsigned int _m,                               \
                             float        _f0,                              \
                             float        _as);                             \
                                                                            \
/* Copy object including all internal objects and state                 */  \
RESAMP2() RESAMP2(_copy)(RESAMP2() _q);                                     \
                                                                            \
/* Destroy resampler, freeing all internally-allocated memory           */  \
int RESAMP2(_destroy)(RESAMP2() _q);                                        \
                                                                            \
/* print resampler object's internals to stdout                         */  \
int RESAMP2(_print)(RESAMP2() _q);                                          \
                                                                            \
/* Reset internal buffer                                                */  \
int RESAMP2(_reset)(RESAMP2() _q);                                          \
                                                                            \
/* Get resampler filter delay (semi-length m)                           */  \
unsigned int RESAMP2(_get_delay)(RESAMP2() _q);                             \
                                                                            \
/* Get output scaling for resampler                                     */  \
/*  _q      : resampler object                                          */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int RESAMP2(_set_scale)(RESAMP2() _q,                                       \
                        TC        _scale);                                  \
                                                                            \
/* Get output scaling for resampler                                     */  \
/*  _q      : resampler object                                          */  \
/*  _scale  : scaling factor applied to each output sample              */  \
int RESAMP2(_get_scale)(RESAMP2() _q,                                       \
                        TC *      _scale);                                  \
                                                                            \
/* Execute resampler as half-band filter for a single input sample      */  \
/* \(x\) where \(y_0\) is the output of the effective low-pass filter,  */  \
/* and \(y_1\) is the output of the effective high-pass filter.         */  \
/*  _q  : resampler object                                              */  \
/*  _x  : input sample                                                  */  \
/*  _y0 : output sample pointer (low frequency)                         */  \
/*  _y1 : output sample pointer (high frequency)                        */  \
int RESAMP2(_filter_execute)(RESAMP2() _q,                                  \
                             TI        _x,                                  \
                             TO *      _y0,                                 \
                             TO *      _y1);                                \
                                                                            \
/* Execute resampler as half-band analysis filterbank on a pair of      */  \
/* sequential time-domain input samples.                                */  \
/* The decimated outputs of the low- and high-pass equivalent filters   */  \
/* are stored in \(y_0\) and \(y_1\), respectively.                     */  \
/*  _q  : resampler object                                              */  \
/*  _x  : input array,  [size: 2 x 1]                                   */  \
/*  _y  : output array, [size: 2 x 1]                                   */  \
int RESAMP2(_analyzer_execute)(RESAMP2() _q,                                \
                               TI *      _x,                                \
                               TO *      _y);                               \
                                                                            \
/* Execute resampler as half-band synthesis filterbank on a pair of     */  \
/* input samples. The low- and high-pass input samples are provided by  */  \
/* \(x_0\) and \(x_1\), respectively. The sequential time-domain output */  \
/* samples are stored in \(y_0\) and \(y_1\).                           */  \
/*  _q  : resampler object                                              */  \
/*  _x  : input array, [size: 2 x 1]                                    */  \
/*  _y  : output array, [size: 2 x 1]                                   */  \
int RESAMP2(_synthesizer_execute)(RESAMP2() _q,                             \
                                  TI *      _x,                             \
                                  TO *      _y);                            \
                                                                            \
/* Execute resampler as half-band decimator on a pair of sequential     */  \
/* time-domain input samples.                                           */  \
/*  _q  : resampler object                                              */  \
/*  _x  : input array, [size: 2 x 1]                                    */  \
/*  _y  : output sample pointer                                         */  \
int RESAMP2(_decim_execute)(RESAMP2() _q,                                   \
                            TI *      _x,                                   \
                            TO *      _y);                                  \
                                                                            \
/* Execute resampler as half-band interpolator on a single input sample */  \
/*  _q  : resampler object                                              */  \
/*  _x  : input sample                                                  */  \
/*  _y  : output array, [size: 2 x 1]                                   */  \
int RESAMP2(_interp_execute)(RESAMP2() _q,                                  \
                             TI        _x,                                  \
                             TO *      _y);                                 \

LIQUID_RESAMP2_DEFINE_API(LIQUID_RESAMP2_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_RESAMP2_DEFINE_API(LIQUID_RESAMP2_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)

LIQUID_RESAMP2_DEFINE_API(LIQUID_RESAMP2_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)


//
// Rational resampler
//
#define LIQUID_RRESAMP_MANGLE_RRRF(name) LIQUID_CONCAT(rresamp_rrrf,name)
#define LIQUID_RRESAMP_MANGLE_CRCF(name) LIQUID_CONCAT(rresamp_crcf,name)
#define LIQUID_RRESAMP_MANGLE_CCCF(name) LIQUID_CONCAT(rresamp_cccf,name)

#define LIQUID_RRESAMP_DEFINE_API(RRESAMP,TO,TC,TI)                         \
                                                                            \
/* Rational rate resampler, implemented as a polyphase filterbank       */  \
typedef struct RRESAMP(_s) * RRESAMP();                                     \
                                                                            \
/* Create rational-rate resampler object from external coefficients to  */  \
/* resample at an exact rate \(P/Q\) = interp/decim.                    */  \
/* Note that to preserve the input filter coefficients, the greatest    */  \
/* common divisor (gcd) is not removed internally from interp and decim */  \
/* when this method is called.                                          */  \
/*  _interp : interpolation factor,               _interp > 0           */  \
/*  _decim  : decimation factor,                   _decim > 0           */  \
/*  _m      : filter semi-length (delay),               0 < _m          */  \
/*  _h      : filter coefficients, [size: 2*_interp*_m x 1]             */  \
RRESAMP() RRESAMP(_create)(unsigned int _interp,                            \
                           unsigned int _decim,                             \
                           unsigned int _m,                                 \
                           TC *         _h);                                \
                                                                            \
/* Create rational-rate resampler object from filter prototype to       */  \
/* resample at an exact rate \(P/Q\) = interp/decim.                    */  \
/* Note that because the filter coefficients are computed internally    */  \
/* here, the greatest common divisor (gcd) from _interp and _decim is   */  \
/* internally removed to improve speed.                                 */  \
/*  _interp : interpolation factor,               _interp > 0           */  \
/*  _decim  : decimation factor,                   _decim > 0           */  \
/*  _m      : filter semi-length (delay),               0 < _m          */  \
/*  _bw     : filter bandwidth relative to sample rate. When the        */  \
/*            the resampler is configured as an interpolator a value of */  \
/*            0.5 (critically filtered) or less is recommended.         */  \
/*            When the resampler is configured as a decimator, the      */  \
/*            critical bandwidth is 0.5*_interp/_decim.                 */  \
/*            When _bw < 0, the object will use the appropriate         */  \
/*            critical bandwidth (interpolation or decimation),         */  \
/*            0 < _bw <= 0.5                                            */  \
/*  _as     : filter stop-band attenuation [dB],        0 < _as         */  \
RRESAMP() RRESAMP(_create_kaiser)(unsigned int _interp,                     \
                                  unsigned int _decim,                      \
                                  unsigned int _m,                          \
                                  float        _bw,                         \
                                  float        _as);                        \
                                                                            \
/* Create rational-rate resampler object from filter prototype to       */  \
/* resample at an exact rate \(P/Q\) = interp/decim.                    */  \
/* Note that because the filter coefficients are computed internally    */  \
/* here, the greatest common divisor (gcd) from _interp and _decim is   */  \
/* internally removed to improve speed.                                 */  \
/*  _type   : filter type (e.g. LIQUID_FIRFILT_RCOS)                    */  \
/*  _interp : interpolation factor,               _interp > 0           */  \
/*  _decim  : decimation factor,                   _decim > 0           */  \
/*  _m      : filter semi-length (delay),               0 < _m          */  \
/*  _beta   : excess bandwidth factor,         0 <= _beta <= 1          */  \
RRESAMP() RRESAMP(_create_prototype)(int          _type,                    \
                                     unsigned int _interp,                  \
                                     unsigned int _decim,                   \
                                     unsigned int _m,                       \
                                     float        _beta);                   \
                                                                            \
/* Create rational resampler object with a specified resampling rate of */  \
/* exactly interp/decim with default parameters. This is a simplified   */  \
/* method to provide a basic resampler with a baseline set of           */  \
/* parameters abstracting away some of the complexities with the        */  \
/* filterbank design.                                                   */  \
/* The default parameters are                                           */  \
/*  m    = 12    (filter semi-length),                                  */  \
/*  bw   = 0.5   (filter bandwidth), and                                */  \
/*  as   = 60 dB (filter stop-band attenuation)                         */  \
/*  _interp : interpolation factor, _interp > 0                         */  \
/*  _decim  : decimation factor,    _decim > 0                          */  \
RRESAMP() RRESAMP(_create_default)(unsigned int _interp,                    \
                                   unsigned int _decim);                    \
                                                                            \
/* Copy object including all internal objects and state                 */  \
RRESAMP() RRESAMP(_copy)(RRESAMP() _q);                                     \
                                                                            \
/* Destroy resampler object, freeing all internal memory                */  \
int RRESAMP(_destroy)(RRESAMP() _q);                                        \
                                                                            \
/* Print resampler object internals to stdout                           */  \
int RRESAMP(_print)(RRESAMP() _q);                                          \
                                                                            \
/* Reset resampler object internals                                     */  \
int RRESAMP(_reset)(RRESAMP() _q);                                          \
                                                                            \
/* Set output scaling for filter, default: \( 2 w \sqrt{P/Q} \)         */  \
/*  _q      : resampler object                                          */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int RRESAMP(_set_scale)(RRESAMP() _q,                                       \
                         TC        _scale);                                 \
                                                                            \
/* Get output scaling for filter                                        */  \
/*  _q      : resampler object                                          */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int RRESAMP(_get_scale)(RRESAMP() _q,                                       \
                        TC *      _scale);                                  \
                                                                            \
/* Get resampler delay (filter semi-length \(m\))                       */  \
unsigned int RRESAMP(_get_delay)(RRESAMP() _q);                             \
                                                                            \
/* Get original interpolation factor when object was created, before    */  \
/* removing greatest common divisor                                     */  \
unsigned int RRESAMP(_get_P)(RRESAMP() _q);                                 \
                                                                            \
/* Get internal interpolation factor of resampler, \(P\), after         */  \
/* removing greatest common divisor                                     */  \
unsigned int RRESAMP(_get_interp)(RRESAMP() _q);                            \
                                                                            \
/* Get original decimation factor \(Q\) when object was created         */  \
/* before removing greatest common divisor                              */  \
unsigned int RRESAMP(_get_Q)(RRESAMP() _q);                                 \
                                                                            \
/* Get internal decimation factor of resampler, \(Q\), after removing   */  \
/* greatest common divisor                                              */  \
unsigned int RRESAMP(_get_decim)(RRESAMP() _q);                             \
                                                                            \
/* Get block length (e.g. greatest common divisor) between original     */  \
/* interpolation rate \(P\) and decimation rate \(Q\) values            */  \
unsigned int RRESAMP(_get_block_len)(RRESAMP() _q);                         \
                                                                            \
/* Get rate of resampler, \(r = P/Q\) = interp/decim                    */  \
float RRESAMP(_get_rate)(RRESAMP() _q);                                     \
                                                                            \
/* Write \(Q\) input samples (after removing greatest common divisor)   */  \
/* into buffer, but do not compute output. This effectively updates the */  \
/* internal state of the resampler.                                     */  \
/*  _q      : resamp object                                             */  \
/*  _buf    : input sample array, [size: decim x 1]                     */  \
int RRESAMP(_write)(RRESAMP() _q,                                           \
                    TI *      _buf);                                        \
                                                                            \
/* Execute rational-rate resampler on a block of input samples and      */  \
/* store the resulting samples in the output array.                     */  \
/* Note that the size of the input and output buffers correspond to the */  \
/* values of the interpolation and decimation rates (\(P\) and \(Q\),   */  \
/* respectively) passed when the object was created, even if they       */  \
/* share a common divisor.                                              */  \
/* Internally the rational resampler reduces \(P\) and \(Q\)            */  \
/* by their greatest common denominator to reduce processing;           */  \
/* however sometimes it is convenient to create the object based on     */  \
/* expected output/input block sizes. This expectation is preserved.    */  \
/* So if an object is created with an interpolation rate \(P=80\)       */  \
/* and a decimation rate \(Q=72\), the object will internally set       */  \
/* \(P=10\) and \(Q=9\) (with a g.c.d of 8); however when               */  \
/* "execute" is called the resampler will still expect an input buffer  */  \
/* of 72 and an output buffer of 80.                                    */  \
/*  _q  : resamp object                                                 */  \
/*  _x  : input sample array, [size: decim x 1]                         */  \
/*  _y  : output sample array, [size: interp x 1]                       */  \
int RRESAMP(_execute)(RRESAMP()       _q,                                   \
                      TI *           _x,                                    \
                      TO *           _y);                                   \
                                                                            \
/* Execute on a block of samples                                        */  \
/*  _q  : resamp object                                                 */  \
/*  _x  : input sample array, [size: decim*n x 1]                       */  \
/*  _n  : block size                                                    */  \
/*  _y  : output sample array, [size: interp*n x 1]                     */  \
int RRESAMP(_execute_block)(RRESAMP()      _q,                              \
                            TI *           _x,                              \
                            unsigned int   _n,                              \
                            TO *           _y);                             \

LIQUID_RRESAMP_DEFINE_API(LIQUID_RRESAMP_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_RRESAMP_DEFINE_API(LIQUID_RRESAMP_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)

LIQUID_RRESAMP_DEFINE_API(LIQUID_RRESAMP_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)


//
// Arbitrary resampler
//
#define LIQUID_RESAMP_MANGLE_RRRF(name) LIQUID_CONCAT(resamp_rrrf,name)
#define LIQUID_RESAMP_MANGLE_CRCF(name) LIQUID_CONCAT(resamp_crcf,name)
#define LIQUID_RESAMP_MANGLE_CCCF(name) LIQUID_CONCAT(resamp_cccf,name)

#define LIQUID_RESAMP_DEFINE_API(RESAMP,TO,TC,TI)                           \
                                                                            \
/* Arbitrary rate resampler, implemented as a polyphase filterbank      */  \
typedef struct RESAMP(_s) * RESAMP();                                       \
                                                                            \
/* Create arbitrary resampler object from filter prototype              */  \
/*  _rate   : arbitrary resampling rate,         0 < _rate              */  \
/*  _m      : filter semi-length (delay),        0 < _m                 */  \
/*  _fc     : filter cutoff frequency,           0 < _fc < 0.5          */  \
/*  _as     : filter stop-band attenuation [dB], 0 < _as                */  \
/*  _npfb   : number of filters in the bank,     0 < _npfb              */  \
RESAMP() RESAMP(_create)(float        _rate,                                \
                         unsigned int _m,                                   \
                         float        _fc,                                  \
                         float        _as,                                  \
                         unsigned int _npfb);                               \
                                                                            \
/* Create arbitrary resampler object with a specified input resampling  */  \
/* rate and default parameters. This is a simplified method to provide  */  \
/* a basic resampler with a baseline set of parameters, abstracting     */  \
/* away some of the complexities with the filterbank design.            */  \
/* The default parameters are                                           */  \
/*  m    = 7                    (filter semi-length),                   */  \
/*  fc   = min(0.49,_rate/2)    (filter cutoff frequency),              */  \
/*  as   = 60 dB                (filter stop-band attenuation), and     */  \
/*  npfb = 64                   (number of filters in the bank).        */  \
/*  _rate   : arbitrary resampling rate,         0 < _rate              */  \
RESAMP() RESAMP(_create_default)(float _rate);                              \
                                                                            \
/* Copy object including all internal objects and state                 */  \
RESAMP() RESAMP(_copy)(RESAMP() _q);                                        \
                                                                            \
/* Destroy arbitrary resampler object, freeing all internal memory      */  \
int RESAMP(_destroy)(RESAMP() _q);                                          \
                                                                            \
/* Print resamp object internals to stdout                              */  \
int RESAMP(_print)(RESAMP() _q);                                            \
                                                                            \
/* Reset resamp object internals                                        */  \
int RESAMP(_reset)(RESAMP() _q);                                            \
                                                                            \
/* Get resampler delay (filter semi-length \(m\))                       */  \
unsigned int RESAMP(_get_delay)(RESAMP() _q);                               \
                                                                            \
/* Set output scaling for resampler                                     */  \
/*  _q      : resampler object                                          */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int RESAMP(_set_scale)(RESAMP() _q,                                         \
                       TC       _scale);                                    \
                                                                            \
/* Get output scaling for resampler                                     */  \
/*  _q      : resampler object                                          */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int RESAMP(_get_scale)(RESAMP() _q,                                         \
                       TC *    _scale);                                     \
                                                                            \
/* Set rate of arbitrary resampler                                      */  \
/*  _q      : resampling object                                         */  \
/*  _rate   : new sampling rate, _rate > 0                              */  \
int RESAMP(_set_rate)(RESAMP() _q,                                          \
                      float    _rate);                                      \
                                                                            \
/* Get rate of arbitrary resampler                                      */  \
float RESAMP(_get_rate)(RESAMP() _q);                                       \
                                                                            \
/* adjust rate of arbitrary resampler                                   */  \
/*  _q      : resampling object                                         */  \
/*  _gamma  : rate adjustment factor: rate <- rate * gamma, _gamma > 0  */  \
int RESAMP(_adjust_rate)(RESAMP() _q,                                       \
                         float    _gamma);                                  \
                                                                            \
/* Set resampling timing phase                                          */  \
/*  _q      : resampling object                                         */  \
/*  _tau    : sample timing phase, -1 <= _tau <= 1                      */  \
int RESAMP(_set_timing_phase)(RESAMP() _q,                                  \
                               float    _tau);                              \
                                                                            \
/* Adjust resampling timing phase                                       */  \
/*  _q      : resampling object                                         */  \
/*  _delta  : sample timing adjustment, -1 <= _delta <= 1               */  \
int RESAMP(_adjust_timing_phase)(RESAMP() _q,                               \
                                 float    _delta);                          \
                                                                            \
/* Get the number of output samples given current state and input       */  \
/* buffer size.                                                         */  \
/*  _q          : resampling object                                     */  \
/*  _num_input  : number of input samples                               */  \
unsigned int RESAMP(_get_num_output)(RESAMP()     _q,                       \
                                     unsigned int _num_input);              \
                                                                            \
/* Execute arbitrary resampler on a single input sample and store the   */  \
/* resulting samples in the output array. The number of output samples  */  \
/* depends upon the resampling rate but will be at most                 */  \
/* \( \lceil{ r \rceil} \) samples.                                     */  \
/*  _q              : resamp object                                     */  \
/*  _x              : single input sample                               */  \
/*  _y              : output sample array (pointer)                     */  \
/*  _num_written    : number of samples written to _y                   */  \
int RESAMP(_execute)(RESAMP()       _q,                                     \
                     TI             _x,                                     \
                     TO *           _y,                                     \
                     unsigned int * _num_written);                          \
                                                                            \
/* Execute arbitrary resampler on a block of input samples and store    */  \
/* the resulting samples in the output array. The number of output      */  \
/* samples depends upon the resampling rate and the number of input     */  \
/* samples but will be at most \( \lceil{ r n_x \rceil} \) samples.     */  \
/*  _q              : resamp object                                     */  \
/*  _x              : input buffer, [size: _nx x 1]                     */  \
/*  _nx             : input buffer                                      */  \
/*  _y              : output sample array (pointer)                     */  \
/*  _ny             : number of samples written to _y                   */  \
int RESAMP(_execute_block)(RESAMP()       _q,                               \
                           TI *           _x,                               \
                           unsigned int   _nx,                              \
                           TO *           _y,                               \
                           unsigned int * _ny);                             \

LIQUID_RESAMP_DEFINE_API(LIQUID_RESAMP_MANGLE_RRRF,
                         float,
                         float,
                         float)

LIQUID_RESAMP_DEFINE_API(LIQUID_RESAMP_MANGLE_CRCF,
                         liquid_float_complex,
                         float,
                         liquid_float_complex)

LIQUID_RESAMP_DEFINE_API(LIQUID_RESAMP_MANGLE_CCCF,
                         liquid_float_complex,
                         liquid_float_complex,
                         liquid_float_complex)


//
// Multi-stage half-band resampler
//

// resampling type (interpolator/decimator)
typedef enum {
    LIQUID_RESAMP_INTERP=0, // interpolator
    LIQUID_RESAMP_DECIM,    // decimator
} liquid_resamp_type;

#define LIQUID_MSRESAMP2_MANGLE_RRRF(name) LIQUID_CONCAT(msresamp2_rrrf,name)
#define LIQUID_MSRESAMP2_MANGLE_CRCF(name) LIQUID_CONCAT(msresamp2_crcf,name)
#define LIQUID_MSRESAMP2_MANGLE_CCCF(name) LIQUID_CONCAT(msresamp2_cccf,name)

#define LIQUID_MSRESAMP2_DEFINE_API(MSRESAMP2,TO,TC,TI)                     \
                                                                            \
/* Multi-stage half-band resampler, implemented as cascaded dyadic      */  \
/* (half-band) polyphase filter banks for interpolation and decimation. */  \
typedef struct MSRESAMP2(_s) * MSRESAMP2();                                 \
                                                                            \
/* Create multi-stage half-band resampler as either decimator or        */  \
/* interpolator.                                                        */  \
/*  _type       : resampler type (e.g. LIQUID_RESAMP_DECIM)             */  \
/*  _num_stages : number of resampling stages, _num_stages <= 16        */  \
/*  _fc         : filter cut-off frequency, 0 < _fc < 0.5               */  \
/*  _f0         : filter center frequency (set to zero)                 */  \
/*  _as         : stop-band attenuation [dB], _as > 0                   */  \
MSRESAMP2() MSRESAMP2(_create)(int          _type,                          \
                               unsigned int _num_stages,                    \
                               float        _fc,                            \
                               float        _f0,                            \
                               float        _as);                           \
                                                                            \
/* Copy object including all internal objects and state                 */  \
MSRESAMP2() MSRESAMP2(_copy)(MSRESAMP2() _q);                               \
                                                                            \
/* Destroy multi-stage half-band resampler, freeing all internal memory */  \
int MSRESAMP2(_destroy)(MSRESAMP2() _q);                                    \
                                                                            \
/* Print msresamp object internals to stdout                            */  \
int MSRESAMP2(_print)(MSRESAMP2() _q);                                      \
                                                                            \
/* Reset msresamp object internal state                                 */  \
int MSRESAMP2(_reset)(MSRESAMP2() _q);                                      \
                                                                            \
/* Get multi-stage half-band resampling rate                            */  \
float MSRESAMP2(_get_rate)(MSRESAMP2() _q);                                 \
                                                                            \
/* Get number of half-band resampling stages in object                  */  \
unsigned int MSRESAMP2(_get_num_stages)(MSRESAMP2() _q);                    \
                                                                            \
/* Get resampling type (LIQUID_RESAMP_DECIM, LIQUID_RESAMP_INTERP)      */  \
int MSRESAMP2(_get_type)(MSRESAMP2() _q);                                   \
                                                                            \
/* Get group delay (number of output samples)                           */  \
float MSRESAMP2(_get_delay)(MSRESAMP2() _q);                                \
                                                                            \
/* Execute multi-stage resampler, M = 2^num_stages                      */  \
/*  LIQUID_RESAMP_INTERP:   input: 1,   output: M                       */  \
/*  LIQUID_RESAMP_DECIM:    input: M,   output: 1                       */  \
/*  _q      : msresamp object                                           */  \
/*  _x      : input sample array                                        */  \
/*  _y      : output sample array                                       */  \
int MSRESAMP2(_execute)(MSRESAMP2() _q,                                     \
                        TI *        _x,                                     \
                        TO *        _y);                                    \

LIQUID_MSRESAMP2_DEFINE_API(LIQUID_MSRESAMP2_MANGLE_RRRF,
                            float,
                            float,
                            float)

LIQUID_MSRESAMP2_DEFINE_API(LIQUID_MSRESAMP2_MANGLE_CRCF,
                            liquid_float_complex,
                            float,
                            liquid_float_complex)

LIQUID_MSRESAMP2_DEFINE_API(LIQUID_MSRESAMP2_MANGLE_CCCF,
                            liquid_float_complex,
                            liquid_float_complex,
                            liquid_float_complex)


//
// Multi-stage arbitrary resampler
//
#define LIQUID_MSRESAMP_MANGLE_RRRF(name) LIQUID_CONCAT(msresamp_rrrf,name)
#define LIQUID_MSRESAMP_MANGLE_CRCF(name) LIQUID_CONCAT(msresamp_crcf,name)
#define LIQUID_MSRESAMP_MANGLE_CCCF(name) LIQUID_CONCAT(msresamp_cccf,name)

#define LIQUID_MSRESAMP_DEFINE_API(MSRESAMP,TO,TC,TI)                       \
                                                                            \
/* Multi-stage half-band resampler, implemented as cascaded dyadic      */  \
/* (half-band) polyphase filter banks followed by an arbitrary rate     */  \
/* resampler for interpolation and decimation.                          */  \
typedef struct MSRESAMP(_s) * MSRESAMP();                                   \
                                                                            \
/* Create multi-stage arbitrary resampler                               */  \
/*  _r      :   resampling rate (output/input), _r > 0                  */  \
/*  _as     :   stop-band attenuation [dB], _as > 0                     */  \
MSRESAMP() MSRESAMP(_create)(float _r,                                      \
                             float _as);                                    \
                                                                            \
/* Copy object including all internal objects and state                 */  \
MSRESAMP() MSRESAMP(_copy)(MSRESAMP() _q);                                  \
                                                                            \
/* Destroy multi-stage arbitrary resampler                              */  \
int MSRESAMP(_destroy)(MSRESAMP() _q);                                      \
                                                                            \
/* Print msresamp object internals to stdout                            */  \
int MSRESAMP(_print)(MSRESAMP() _q);                                        \
                                                                            \
/* Reset msresamp object internal state                                 */  \
int MSRESAMP(_reset)(MSRESAMP() _q);                                        \
                                                                            \
/* Get filter delay (output samples)                                    */  \
float MSRESAMP(_get_delay)(MSRESAMP() _q);                                  \
                                                                            \
/* get overall resampling rate                                          */  \
float MSRESAMP(_get_rate)(MSRESAMP() _q);                                   \
                                                                            \
/* Get the number of output samples given current state and input       */  \
/* buffer size.                                                         */  \
/*  _q          : resampling object                                     */  \
/*  _num_input  : number of input samples                               */  \
unsigned int MSRESAMP(_get_num_output)(MSRESAMP()   _q,                     \
                                       unsigned int _num_input);            \
                                                                            \
/* Execute multi-stage resampler on one or more input samples.          */  \
/* The number of output samples depends upon the resampling rate and    */  \
/* the number of input samples. In general it is good practice to       */  \
/* allocate at least \( \lceil{ 1 + 2 r n_x \rceil} \) samples in the   */  \
/* output array to avoid overflows.                                     */  \
/*  _q  : msresamp object                                               */  \
/*  _x  : input sample array, [size: _nx x 1]                           */  \
/*  _nx : input sample array size                                       */  \
/*  _y  : pointer to output array for storing result                    */  \
/*  _ny : number of samples written to _y                               */  \
int MSRESAMP(_execute)(MSRESAMP()     _q,                                   \
                       TI *           _x,                                   \
                       unsigned int   _nx,                                  \
                       TO *           _y,                                   \
                       unsigned int * _ny);                                 \

LIQUID_MSRESAMP_DEFINE_API(LIQUID_MSRESAMP_MANGLE_RRRF,
                           float,
                           float,
                           float)

LIQUID_MSRESAMP_DEFINE_API(LIQUID_MSRESAMP_MANGLE_CRCF,
                           liquid_float_complex,
                           float,
                           liquid_float_complex)

LIQUID_MSRESAMP_DEFINE_API(LIQUID_MSRESAMP_MANGLE_CCCF,
                           liquid_float_complex,
                           liquid_float_complex,
                           liquid_float_complex)

//
// Direct digital [up/down] synthesizer
//

#define LIQUID_DDS_MANGLE_CCCF(name)  LIQUID_CONCAT(dds_cccf,name)

#define LIQUID_DDS_DEFINE_API(DDS,TO,TC,TI)                                 \
                                                                            \
/* Direct digital (up/down) synthesizer object                          */  \
typedef struct DDS(_s) * DDS();                                             \
                                                                            \
/* Create digital synthesizer object                                    */  \
/*  _num_stages : number of half-band stages, _num_stages > 0           */  \
/*  _fc         : signal relative center frequency, _fc in [-0.5,0.5]   */  \
/*  _bw         : signal relative bandwidth, _bw in (0,1)               */  \
/*  _as         : filter stop-band attenuation (dB), _as > 0            */  \
DDS() DDS(_create)(unsigned int _num_stages,                                \
                   float        _fc,                                        \
                   float        _bw,                                        \
                   float        _as);                                       \
                                                                            \
/* Copy object including all internal objects and state                 */  \
DDS() DDS(_copy)(DDS() _q);                                                 \
                                                                            \
/* Destroy digital synthesizer object                                   */  \
int DDS(_destroy)(DDS() _q);                                                \
                                                                            \
/* Print synthesizer object internals                                   */  \
int DDS(_print)(DDS() _q);                                                  \
                                                                            \
/* Reset synthesizer object internals                                   */  \
int DDS(_reset)(DDS() _q);                                                  \
                                                                            \
/* Set output scaling for synthesizer                                   */  \
/*  _q      : synthesizer object                                        */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int DDS(_set_scale)(DDS() _q,                                               \
                    TC    _scale);                                          \
                                                                            \
/* Get output scaling for synthesizer                                   */  \
/*  _q      : synthesizer object                                        */  \
/*  _scale  : scaling factor to apply to each output sample             */  \
int DDS(_get_scale)(DDS() _q,                                               \
                    TC *  _scale);                                          \
                                                                            \
/* Get number of half-band states in DDS object                         */  \
unsigned int DDS(_get_num_stages)(DDS() _q);                                \
                                                                            \
/* Get delay (samples) when running as interpolator                     */  \
unsigned int DDS(_get_delay_interp)(DDS() _q);                              \
                                                                            \
/* Get delay (samples) when running as decimator                        */  \
float        DDS(_get_delay_decim)(DDS() _q);                               \
                                                                            \
/* Run DDS object as decimator                                          */  \
/*  _q      : synthesizer object                                        */  \
/*  _x      : input data array, [size: (1<<_num_stages) x 1]            */  \
/*  _y      : output sample                                             */  \
int DDS(_decim_execute)(DDS() _q,                                           \
                        TI *  _x,                                           \
                        TO *  _y);                                          \
                                                                            \
/* Run DDS object as interpolator                                       */  \
/*  _q      : synthesizer object                                        */  \
/*  _x      : input sample                                              */  \
/*  _y      : output data array, [size: (1<<_num_stages) x 1]           */  \
int DDS(_interp_execute)(DDS() _q,                                          \
                          TI _x,                                            \
                          TO * _y);                                         \

LIQUID_DDS_DEFINE_API(LIQUID_DDS_MANGLE_CCCF,
                      liquid_float_complex,
                      liquid_float_complex,
                      liquid_float_complex)


//
// Symbol timing recovery (symbol synchronizer)
//
#define LIQUID_SYMSYNC_MANGLE_RRRF(name) LIQUID_CONCAT(symsync_rrrf,name)
#define LIQUID_SYMSYNC_MANGLE_CRCF(name) LIQUID_CONCAT(symsync_crcf,name)

#define LIQUID_SYMSYNC_DEFINE_API(SYMSYNC,TO,TC,TI)                         \
                                                                            \
/* Multi-rate symbol synchronizer for symbol timing recovery.           */  \
typedef struct SYMSYNC(_s) * SYMSYNC();                                     \
                                                                            \
/* Create synchronizer object from external coefficients                */  \
/*  _k      : samples per symbol, _k >= 2                               */  \
/*  _M      : number of filters in the bank, _M > 0                     */  \
/*  _h      : matched filter coefficients, [size: _h_len x 1]           */  \
/*  _h_len  : length of matched filter; \( h_{len} = 2 k m + 1 \)       */  \
SYMSYNC() SYMSYNC(_create)(unsigned int _k,                                 \
                           unsigned int _M,                                 \
                           TC *         _h,                                 \
                           unsigned int _h_len);                            \
                                                                            \
/* Create square-root Nyquist symbol synchronizer from prototype        */  \
/*  _type   : filter type (e.g. LIQUID_FIRFILT_RRC)                     */  \
/*  _k      : samples/symbol, _k >= 2                                   */  \
/*  _m      : symbol delay, _m > 0                                      */  \
/*  _beta   : rolloff factor, 0 <= _beta <= 1                           */  \
/*  _M      : number of filters in the bank, _M > 0                     */  \
SYMSYNC() SYMSYNC(_create_rnyquist)(int          _type,                     \
                                    unsigned int _k,                        \
                                    unsigned int _m,                        \
                                    float        _beta,                     \
                                    unsigned int _M);                       \
                                                                            \
/* Create symsync using Kaiser filter interpolator. This is useful when */  \
/* the input signal has its matched filter applied already.             */  \
/*  _k      : input samples/symbol, _k >= 2                             */  \
/*  _m      : symbol delay, _m > 0                                      */  \
/*  _beta   : rolloff factor, 0<= _beta <= 1                            */  \
/*  _M      : number of filters in the bank, _M > 0                     */  \
SYMSYNC() SYMSYNC(_create_kaiser)(unsigned int _k,                          \
                                  unsigned int _m,                          \
                                  float        _beta,                       \
                                  unsigned int _M);                         \
                                                                            \
/* Copy object including all internal objects and state                 */  \
SYMSYNC() SYMSYNC(_copy)(SYMSYNC() _q);                                     \
                                                                            \
/* Destroy symsync object, freeing all internal memory                  */  \
int SYMSYNC(_destroy)(SYMSYNC() _q);                                        \
                                                                            \
/* Print symsync object's parameters to stdout                          */  \
int SYMSYNC(_print)(SYMSYNC() _q);                                          \
                                                                            \
/* Reset symsync internal state                                         */  \
int SYMSYNC(_reset)(SYMSYNC() _q);                                          \
                                                                            \
/* Lock the symbol synchronizer's loop control                          */  \
int SYMSYNC(_lock)(SYMSYNC() _q);                                           \
                                                                            \
/* Unlock the symbol synchronizer's loop control                        */  \
int SYMSYNC(_unlock)(SYMSYNC() _q);                                         \
                                                                            \
/* Check the lock state of the symbol synchronizer's loop control,      */  \
/* returning 1 if the object is locked, 0 if unlocked.                  */  \
int SYMSYNC(_is_locked)(SYMSYNC() _q);                                      \
                                                                            \
/* Set synchronizer output rate (samples/symbol)                        */  \
/*  _q      : synchronizer object                                       */  \
/*  _k_out  : output samples/symbol, _k_out > 0                         */  \
int SYMSYNC(_set_output_rate)(SYMSYNC()    _q,                              \
                              unsigned int _k_out);                         \
                                                                            \
/* Set loop-filter bandwidth                                            */  \
/*  _q      : synchronizer object                                       */  \
/*  _bt     : loop bandwidth, 0 <= _bt <= 1                             */  \
int SYMSYNC(_set_lf_bw)(SYMSYNC() _q,                                       \
                        float     _bt);                                     \
                                                                            \
/* Return instantaneous fractional timing offset estimate               */  \
float SYMSYNC(_get_tau)(SYMSYNC() _q);                                      \
                                                                            \
/* Execute synchronizer on input data array                             */  \
/*  _q      : synchronizer object                                       */  \
/*  _x      : input data array, [size: _nx x 1]                         */  \
/*  _nx     : number of input samples                                   */  \
/*  _y      : output data array                                         */  \
/*  _ny     : number of samples written to output buffer                */  \
int SYMSYNC(_execute)(SYMSYNC()      _q,                                    \
                      TI *           _x,                                    \
                      unsigned int   _nx,                                   \
                      TO *           _y,                                    \
                      unsigned int * _ny);                                  \

LIQUID_SYMSYNC_DEFINE_API(LIQUID_SYMSYNC_MANGLE_RRRF,
                          float,
                          float,
                          float)

LIQUID_SYMSYNC_DEFINE_API(LIQUID_SYMSYNC_MANGLE_CRCF,
                          liquid_float_complex,
                          float,
                          liquid_float_complex)


//
// Finite impulse response Farrow filter
//

#define LIQUID_FIRFARROW_MANGLE_RRRF(name) LIQUID_CONCAT(firfarrow_rrrf,name)
#define LIQUID_FIRFARROW_MANGLE_CRCF(name) LIQUID_CONCAT(firfarrow_crcf,name)
//#define LIQUID_FIRFARROW_MANGLE_CCCF(name) LIQUID_CONCAT(firfarrow_cccf,name)

// Macro:
//   FIRFARROW  : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_FIRFARROW_DEFINE_API(FIRFARROW,TO,TC,TI)                     \
                                                                            \
/* Finite impulse response (FIR) Farrow filter for timing delay         */  \
typedef struct FIRFARROW(_s) * FIRFARROW();                                 \
                                                                            \
/* Create firfarrow object                                              */  \
/*  _h_len      : filter length, _h_len >= 2                            */  \
/*  _p          : polynomial order, _p >= 1                             */  \
/*  _fc         : filter cutoff frequency, 0 <= _fc <= 0.5              */  \
/*  _as         : stopband attenuation [dB], _as > 0                    */  \
FIRFARROW() FIRFARROW(_create)(unsigned int _h_len,                         \
                               unsigned int _p,                             \
                               float        _fc,                            \
                               float        _as);                           \
                                                                            \
/* Destroy firfarrow object, freeing all internal memory                */  \
int FIRFARROW(_destroy)(FIRFARROW() _q);                                    \
                                                                            \
/* Print firfarrow object's internal properties                         */  \
int FIRFARROW(_print)(FIRFARROW() _q);                                      \
                                                                            \
/* Reset firfarrow object's internal state                              */  \
int FIRFARROW(_reset)(FIRFARROW() _q);                                      \
                                                                            \
/* Push sample into firfarrow object                                    */  \
/*  _q      : firfarrow object                                          */  \
/*  _x      : input sample                                              */  \
int FIRFARROW(_push)(FIRFARROW() _q,                                        \
                     TI          _x);                                       \
                                                                            \
/* Set fractional delay of firfarrow object                             */  \
/*  _q      : firfarrow object                                          */  \
/*  _mu     : fractional sample delay, -1 <= _mu <= 1                   */  \
int FIRFARROW(_set_delay)(FIRFARROW() _q,                                   \
                          float       _mu);                                 \
                                                                            \
/* Execute firfarrow internal dot product                               */  \
/*  _q      : firfarrow object                                          */  \
/*  _y      : output sample pointer                                     */  \
int FIRFARROW(_execute)(FIRFARROW() _q,                                     \
                        TO *        _y);                                    \
                                                                            \
/* Execute firfarrow filter on block of samples.                        */  \
/* In-place operation is permitted (the input and output arrays may     */  \
/* share the same pointer)                                              */  \
/*  _q      : firfarrow object                                          */  \
/*  _x      : input array, [size: _n x 1]                               */  \
/*  _n      : input, output array size                                  */  \
/*  _y      : output array, [size: _n x 1]                              */  \
int FIRFARROW(_execute_block)(FIRFARROW()  _q,                              \
                              TI *         _x,                              \
                              unsigned int _n,                              \
                              TO *         _y);                             \
                                                                            \
/* Get length of firfarrow object (number of filter taps)               */  \
unsigned int FIRFARROW(_get_length)(FIRFARROW() _q);                        \
                                                                            \
/* Get coefficients of firfarrow object                                 */  \
/*  _q      : firfarrow object                                          */  \
/*  _h      : output coefficients pointer, [size: _h_len x 1]           */  \
int FIRFARROW(_get_coefficients)(FIRFARROW() _q,                            \
                                 float *     _h);                           \
                                                                            \
/* Compute complex frequency response                                   */  \
/*  _q      : filter object                                             */  \
/*  _fc     : frequency                                                 */  \
/*  _H      : output frequency response                                 */  \
int FIRFARROW(_freqresponse)(FIRFARROW()            _q,                     \
                             float                  _fc,                    \
                             liquid_float_complex * _H);                    \
                                                                            \
/* Compute group delay [samples]                                        */  \
/*  _q      :   filter object                                           */  \
/*  _fc     :   frequency                                               */  \
float FIRFARROW(_groupdelay)(FIRFARROW() _q,                                \
                             float       _fc);                              \

LIQUID_FIRFARROW_DEFINE_API(LIQUID_FIRFARROW_MANGLE_RRRF,
                            float,
                            float,
                            float)

LIQUID_FIRFARROW_DEFINE_API(LIQUID_FIRFARROW_MANGLE_CRCF,
                            liquid_float_complex,
                            float,
                            liquid_float_complex)


//
// Order-statistic filter
//

#define LIQUID_ORDFILT_MANGLE_RRRF(name) LIQUID_CONCAT(ordfilt_rrrf,name)

// Macro:
//   ORDFILT    : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_ORDFILT_DEFINE_API(ORDFILT,TO,TC,TI)                         \
                                                                            \
/* Finite impulse response (FIR) filter                                 */  \
typedef struct ORDFILT(_s) * ORDFILT();                                     \
                                                                            \
/* Create a order-statistic filter (ordfilt) object by specifying       */  \
/* the buffer size and appropriate sample index of order statistic.     */  \
/*  _n      : buffer size, _n > 0                                       */  \
/*  _k      : sample index for order statistic, 0 <= _k < _n            */  \
ORDFILT() ORDFILT(_create)(unsigned int _n,                                 \
                           unsigned int _k);                                \
                                                                            \
/* Create a median filter by specifying buffer semi-length.             */  \
/*  _m      : buffer semi-length                                        */  \
ORDFILT() ORDFILT(_create_medfilt)(unsigned int _m);                        \
                                                                            \
/* Copy object including all internal objects and state                 */  \
ORDFILT() ORDFILT(_copy)(ORDFILT() _q);                                     \
                                                                            \
/* Destroy filter object and free all internal memory                   */  \
int ORDFILT(_destroy)(ORDFILT() _q);                                        \
                                                                            \
/* Reset filter object's internal buffer                                */  \
int ORDFILT(_reset)(ORDFILT() _q);                                          \
                                                                            \
/* Print filter object information to stdout                            */  \
int ORDFILT(_print)(ORDFILT() _q);                                          \
                                                                            \
/* Push sample into filter object's internal buffer                     */  \
/*  _q      : filter object                                             */  \
/*  _x      : single input sample                                       */  \
int ORDFILT(_push)(ORDFILT() _q,                                            \
                   TI        _x);                                           \
                                                                            \
/* Write block of samples into object's internal buffer                 */  \
/*  _q      : filter object                                             */  \
/*  _x      : array of input samples, [size: _n x 1]                    */  \
/*  _n      : number of input elements                                  */  \
int ORDFILT(_write)(ORDFILT()    _q,                                        \
                    TI *         _x,                                        \
                    unsigned int _n);                                       \
                                                                            \
/* Execute on the filter's internal buffer                              */  \
/*  _q      : filter object                                             */  \
/*  _y      : pointer to single output sample                           */  \
int ORDFILT(_execute)(ORDFILT() _q,                                         \
                      TO *      _y);                                        \
                                                                            \
/* Execute filter on one sample, equivalent to push() and execute()     */  \
/*  _q      : filter object                                             */  \
/*  _x      : single input sample                                       */  \
/*  _y      : pointer to single output sample                           */  \
int ORDFILT(_execute_one)(ORDFILT() _q,                                     \
                          TI        _x,                                     \
                          TO *      _y);                                    \
                                                                            \
/* Execute the filter on a block of input samples; in-place operation   */  \
/* is permitted (_x and _y may point to the same place in memory)       */  \
/*  _q      : filter object                                             */  \
/*  _x      : pointer to input array, [size: _n x 1]                    */  \
/*  _n      : number of input, output samples                           */  \
/*  _y      : pointer to output array, [size: _n x 1]                   */  \
int ORDFILT(_execute_block)(ORDFILT()    _q,                                \
                            TI *         _x,                                \
                            unsigned int _n,                                \
                            TO *         _y);                               \

LIQUID_ORDFILT_DEFINE_API(LIQUID_ORDFILT_MANGLE_RRRF,
                          float,
                          float,
                          float)


//
// MODULE : framing
//

// framesyncstats : generic frame synchronizer statistic structure

typedef struct {
    // signal quality
    float evm;      // error vector magnitude [dB]
    float rssi;     // received signal strength indicator [dB]
    float cfo;      // carrier frequency offset (f/Fs)

    // demodulated frame symbols
    liquid_float_complex * framesyms;   // pointer to array, [size: framesyms x 1]
    unsigned int num_framesyms;         // length of framesyms

    // modulation/coding scheme etc.
    unsigned int mod_scheme;    // modulation scheme
    unsigned int mod_bps;       // modulation depth (bits/symbol)
    unsigned int check;         // data validity check (crc, checksum)
    unsigned int fec0;          // forward error-correction (inner)
    unsigned int fec1;          // forward error-correction (outer)
} framesyncstats_s;

// external framesyncstats default object
extern framesyncstats_s framesyncstats_default;

// initialize framesyncstats object on default
int framesyncstats_init_default(framesyncstats_s * _stats);

// print framesyncstats object
int framesyncstats_print(framesyncstats_s * _stats);


// framedatastats : gather frame data
typedef struct {
    unsigned int      num_frames_detected;
    unsigned int      num_headers_valid;
    unsigned int      num_payloads_valid;
    unsigned long int num_bytes_received;
} framedatastats_s;

// reset framedatastats object
int framedatastats_reset(framedatastats_s * _stats);

// print framedatastats object
int framedatastats_print(framedatastats_s * _stats);


// Generic frame synchronizer callback function type
//  _header         :   header data, [size: 8 bytes]
//  _header_valid   :   is header valid? (0:no, 1:yes)
//  _payload        :   payload data, [size: _payload_len]
//  _payload_len    :   length of payload (bytes)
//  _payload_valid  :   is payload valid? (0:no, 1:yes)
//  _stats          :   frame statistics object
//  _userdata       :   pointer to userdata
typedef int (*framesync_callback)(unsigned char *  _header,
                                  int              _header_valid,
                                  unsigned char *  _payload,
                                  unsigned int     _payload_len,
                                  int              _payload_valid,
                                  framesyncstats_s _stats,
                                  void *           _userdata);

// framesync csma callback functions invoked when signal levels is high or low
//  _userdata       :   user-defined data pointer
typedef void (*framesync_csma_callback)(void * _userdata);


#define LIQUID_QPACKETMODEM_MANGLE_FLOAT(name) LIQUID_CONCAT(qpacketmodem,name)

// Macro:
//   QPACKETMODEM   : name-mangling macro
//   T              : primitive data type
#define LIQUID_QPACKETMODEM_DEFINE_API(QPACKETMODEM,T,TC)                   \
                                                                            \
/* Packet encoder/decoder                                               */  \
typedef struct QPACKETMODEM(_s) * QPACKETMODEM();                           \
                                                                            \
/* Create packet encoder                                                */  \
QPACKETMODEM() QPACKETMODEM(_create)();                                     \
                                                                            \
/* Copy object including all internal objects and state                 */  \
QPACKETMODEM() QPACKETMODEM(_copy)   (QPACKETMODEM() _q);                   \
                                                                            \
/* Destroy object, freeing all allocated memory                         */  \
int QPACKETMODEM(_destroy)(QPACKETMODEM() _q);                              \
                                                                            \
/* Print modem status to stdout                                         */  \
int QPACKETMODEM(_print)(QPACKETMODEM() _q);                                \
                                                                            \
/* Reset internal state of modem object                                 */  \
int QPACKETMODEM(_reset)(QPACKETMODEM() _q);                                \
                                                                            \
/* Configure object with particular parameters                          */  \
/*  _q           : qpacketmodem object                                  */  \
/*  _payload_len : length of payload message [bytes]                    */  \
/*  _check       : data integrity check, e.g LIQUID_CRC_32              */  \
/*  _fec0        : forward error-correction scheme (inner), e.g.        */  \
/*                 LIQUID_FEC_GOLAY2412                                 */  \
/*  _fec1        : forward error-correction scheme (outer)              */  \
/*  _ms          : modulation scheme, e.g. LIQUID_MODEM_QPSK            */  \
int QPACKETMODEM(_configure)(QPACKETMODEM() _q,                             \
                             unsigned int   _payload_len,                   \
                             crc_scheme     _check,                         \
                             fec_scheme     _fec0,                          \
                             fec_scheme     _fec1,                          \
                             int            _ms);                           \
                                                                            \
/* Get length of encoded frame in symbols                               */  \
unsigned int QPACKETMODEM(_get_frame_len)(QPACKETMODEM() _q);               \
                                                                            \
/* Get unencoded/decoded payload length (bytes)                         */  \
unsigned int QPACKETMODEM(_get_payload_len)(QPACKETMODEM() _q);             \
                                                                            \
/* Get data integrity check, e.g. LIQUID_CRC_32                         */  \
unsigned int QPACKETMODEM(_get_crc)(QPACKETMODEM() _q);                     \
                                                                            \
/* Get inner forward error-correction scheme, e.g. LIQUID_GOLAY_2412    */  \
unsigned int QPACKETMODEM(_get_fec0)(QPACKETMODEM() _q);                    \
                                                                            \
/* Get outer forward error-correction scheme, e.g. LIQUID_GOLAY_2412    */  \
unsigned int QPACKETMODEM(_get_fec1)(QPACKETMODEM() _q);                    \
                                                                            \
/* Get modulation scheme, e.g. LIQUID_MODEM_QPSK                        */  \
unsigned int QPACKETMODEM(_get_modscheme)(QPACKETMODEM() _q);               \
                                                                            \
/* Get demodulator phase error (instantaneous) [radians]                */  \
float QPACKETMODEM(_get_demodulator_phase_error)(QPACKETMODEM() _q);        \
                                                                            \
/* Get demodulator error-vector magnitude after frame was received      */  \
float QPACKETMODEM(_get_demodulator_evm)(QPACKETMODEM() _q);                \
                                                                            \
/* encode packet into un-modulated frame symbol indices                 */  \
/*  _q          : qpacketmodem object                                   */  \
/*  _payload    : unencoded payload bytes                               */  \
/*  _syms       : encoded but un-modulated payload symbol indices       */  \
int QPACKETMODEM(_encode_syms)(QPACKETMODEM()        _q,                    \
                               const unsigned char * _payload,              \
                               unsigned char *       _syms);                \
                                                                            \
/* decode packet from demodulated frame symbol indices (hard-decision   */  \
/* decoding)                                                            */  \
/*  _q       : qpacketmodem object                                      */  \
/*  _syms    : received hard-decision symbol indices,                   */  \
/*             [size: frame_len x 1]                                    */  \
/*  _payload : recovered decoded payload bytes                          */  \
/*  _return  : flag indicating if data integrity check passed           */  \
int QPACKETMODEM(_decode_syms)(QPACKETMODEM()  _q,                          \
                               unsigned char * _syms,                       \
                               unsigned char * _payload);                   \
                                                                            \
/* decode packet from demodulated frame bits (soft-decision decoding)   */  \
/*  _q       : qpacketmodem object                                      */  \
/*  _bits    : received soft-decision bits, [size: bps*frame_len x 1]   */  \
/*  _payload : recovered decoded payload bytes                          */  \
int QPACKETMODEM(_decode_bits)(QPACKETMODEM()  _q,                          \
                               unsigned char * _bits,                       \
                               unsigned char * _payload);                   \
                                                                            \
/* encode and modulate packet into modulated frame samples              */  \
/*  _q          :   qpacketmodem object                                 */  \
/*  _payload    :   unencoded payload bytes                             */  \
/*  _frame      :   encoded/modulated payload symbols                   */  \
int QPACKETMODEM(_encode)(QPACKETMODEM()        _q,                         \
                          const unsigned char * _payload,                   \
                          TC *                  _frame);                    \
                                                                            \
/* decode packet from modulated frame samples, returning flag if CRC    */  \
/* passed using hard-decision decoding                                  */  \
/*  _q          : qpacketmodem object                                   */  \
/*  _frame      : encoded/modulated payload symbols                     */  \
/*  _payload    : recovered decoded payload bytes                       */  \
/*  _return     : flag indicating if data integrity check passed        */  \
int QPACKETMODEM(_decode)(QPACKETMODEM()  _q,                               \
                          TC *            _frame,                           \
                          unsigned char * _payload);                        \
                                                                            \
/* decode packet from modulated frame samples, returning flag if CRC    */  \
/* passed using soft-decision decoding                                  */  \
/*  _q          : qpacketmodem object                                   */  \
/*  _frame      : encoded/modulated payload symbols                     */  \
/*  _payload    : recovered decoded payload bytes                       */  \
/*  _return     : flag indicating if data integrity check passed        */  \
int QPACKETMODEM(_decode_soft)(QPACKETMODEM()  _q,                          \
                               TC *            _frame,                      \
                               unsigned char * _payload);                   \
                                                                            \
/* decode symbol from modulated frame samples, returning flag if all    */  \
/* symbols received                                                     */  \
/*  _q          : qpacketmodem object                                   */  \
/*  _symbol     : input received symbol before demodulation             */  \
/*  _return     : flag indicating if all symbols were received          */  \
int QPACKETMODEM(_decode_soft_sym)(QPACKETMODEM() _q,                       \
                                   TC             _symbol);                 \
                                                                            \
/* Decode entire packet, assuming that entire frame has been received.  */  \
/*  _q          : qpacketmodem object                                   */  \
/*  _payload    : output payload [bytes]                                */  \
/*  _return     : flag indicating if data integrity check passed        */  \
int QPACKETMODEM(_decode_soft_payload)(QPACKETMODEM()  _q,                  \
                                       unsigned char * _payload);           \

LIQUID_QPACKETMODEM_DEFINE_API(LIQUID_QPACKETMODEM_MANGLE_FLOAT,float,liquid_float_complex)

//
// pilot generator/synchronizer for packet burst recovery
//

// get number of pilots in frame
unsigned int qpilot_num_pilots(unsigned int _payload_len,
                               unsigned int _pilot_spacing);

// get length of frame with a particular payload length and pilot spacing
unsigned int qpilot_frame_len(unsigned int _payload_len,
                              unsigned int _pilot_spacing);

//
// pilot generator for packet burst recovery
//

typedef struct qpilotgen_s * qpilotgen;

// create packet encoder
qpilotgen qpilotgen_create(unsigned int _payload_len,
                           unsigned int _pilot_spacing);

qpilotgen qpilotgen_recreate(qpilotgen    _q,
                             unsigned int _payload_len,
                             unsigned int _pilot_spacing);

// Copy object including all internal objects and state
qpilotgen qpilotgen_copy(qpilotgen _q);

int qpilotgen_destroy(qpilotgen _q);
int qpilotgen_reset(  qpilotgen _q);
int qpilotgen_print(  qpilotgen _q);

unsigned int qpilotgen_get_frame_len(qpilotgen _q);

// insert pilot symbols
int qpilotgen_execute(qpilotgen              _q,
                      liquid_float_complex * _payload,
                      liquid_float_complex * _frame);

//
// pilot synchronizer for packet burst recovery
//
typedef struct qpilotsync_s * qpilotsync;

// create packet encoder
qpilotsync qpilotsync_create(unsigned int _payload_len,
                             unsigned int _pilot_spacing);

qpilotsync qpilotsync_recreate(qpilotsync   _q,
                               unsigned int _payload_len,
                               unsigned int _pilot_spacing);

// Copy object including all internal objects and state
qpilotsync qpilotsync_copy(qpilotsync _q);

int qpilotsync_destroy(qpilotsync _q);
int qpilotsync_reset(  qpilotsync _q);
int qpilotsync_print(  qpilotsync _q);

unsigned int qpilotsync_get_frame_len(qpilotsync _q);

// recover frame symbols from received frame
int qpilotsync_execute(qpilotsync             _q,
                       liquid_float_complex * _frame,
                       liquid_float_complex * _payload);

// get estimates
float qpilotsync_get_dphi(qpilotsync _q);
float qpilotsync_get_phi (qpilotsync _q);
float qpilotsync_get_gain(qpilotsync _q);
float qpilotsync_get_evm (qpilotsync _q);


//
// Basic frame generator (64 bytes data payload)
//

// frame length in samples
#define LIQUID_FRAME64_LEN (1440)

typedef struct framegen64_s * framegen64;

// create frame generator
framegen64 framegen64_create();

// copy object
framegen64 framegen64_copy(framegen64 _q);

// destroy frame generator
int framegen64_destroy(framegen64 _q);

// print frame generator internal properties
int framegen64_print(framegen64 _q);

// generate frame
//  _q          :   frame generator object
//  _header     :   8-byte header data, NULL for random
//  _payload    :   64-byte payload data, NULL for random
//  _frame      :   output frame samples, [size: LIQUID_FRAME64_LEN x 1]
int framegen64_execute(framegen64             _q,
                       unsigned char *        _header,
                       unsigned char *        _payload,
                       liquid_float_complex * _frame);

typedef struct framesync64_s * framesync64;

// create framesync64 object
//  _callback   :   callback function
//  _userdata   :   user data pointer passed to callback function
framesync64 framesync64_create(framesync_callback _callback,
                               void *             _userdata);

// copy object
framesync64 framesync64_copy(framesync64 _q);

// destroy frame synchronizer
int framesync64_destroy(framesync64 _q);

// print frame synchronizer internal properties
int framesync64_print(framesync64 _q);

// reset frame synchronizer internal state
int framesync64_reset(framesync64 _q);

// set the callback and userdata fields
int framesync64_set_callback(framesync64 _q, framesync_callback _callback);
int framesync64_set_userdata(framesync64 _q, void *             _userdata);

// push samples through frame synchronizer
//  _q      :   frame synchronizer object
//  _x      :   input samples, [size: _n x 1]
//  _n      :   number of input samples
int framesync64_execute(framesync64            _q,
                        liquid_float_complex * _x,
                        unsigned int           _n);

DEPRECATED("debugging enabled by default; return non-zero value to export file",
int framesync64_debug_enable(framesync64 _q);
)

DEPRECATED("debugging enabled by default; return non-zero value to export file",
int framesync64_debug_disable(framesync64 _q);
)

DEPRECATED("binary debugging file exported on non-zero return value",
int framesync64_debug_print(framesync64 _q, const char * _filename);
)

// set prefix for exporting debugging files, default: "framesync64"
//  _q      : frame sync object
//  _prefix : string with valid file path
int framesync64_set_prefix(framesync64  _q,
                           const char * _prefix);

// get prefix for exporting debugging files
const char * framesync64_get_prefix(framesync64  _q);

// get number of files exported
unsigned int framesync64_get_num_files_exported(framesync64  _q);

// get name of last file written
const char * framesync64_get_filename(framesync64  _q);

// get/set detection threshold
float framesync64_get_threshold(framesync64 _q);
int   framesync64_set_threshold(framesync64 _q, float _threshold);

// frame data statistics
int              framesync64_reset_framedatastats(framesync64 _q);
framedatastats_s framesync64_get_framedatastats  (framesync64 _q);

#if 0
// advanced modes
int framesync64_set_csma_callbacks(framesync64             _q,
                                   framesync_csma_callback _csma_lock,
                                   framesync_csma_callback _csma_unlock,
                                   void *                  _csma_userdata);
#endif

//
// Flexible frame : adjustable payload, mod scheme, etc., but bring
//                  your own error correction, redundancy check
//

// frame generator
typedef struct {
    unsigned int check;         // data validity check
    unsigned int fec0;          // forward error-correction scheme (inner)
    unsigned int fec1;          // forward error-correction scheme (outer)
    unsigned int mod_scheme;    // modulation scheme
} flexframegenprops_s;

int flexframegenprops_init_default(flexframegenprops_s * _fgprops);

typedef struct flexframegen_s * flexframegen;

// create flexframegen object
//  _props  :   frame properties (modulation scheme, etc.)
flexframegen flexframegen_create(flexframegenprops_s * _props);

// destroy flexframegen object
int flexframegen_destroy(flexframegen _q);

// print flexframegen object internals
int flexframegen_print(flexframegen _q);

// reset flexframegen object internals
int flexframegen_reset(flexframegen _q);

// is frame assembled?
int flexframegen_is_assembled(flexframegen _q);

// get frame properties
int flexframegen_getprops(flexframegen _q, flexframegenprops_s * _props);

// set frame properties
int flexframegen_setprops(flexframegen _q, flexframegenprops_s * _props);

// set length of user-defined portion of header
int flexframegen_set_header_len(flexframegen _q, unsigned int _len);

// set properties for header section
int flexframegen_set_header_props(flexframegen          _q,
                                  flexframegenprops_s * _props);

// get length of assembled frame (samples)
unsigned int flexframegen_getframelen(flexframegen _q);

// assemble a frame from an array of data
//  _q              :   frame generator object
//  _header         :   frame header
//  _payload        :   payload data, [size: _payload_len x 1]
//  _payload_len    :   payload data length
int flexframegen_assemble(flexframegen          _q,
                          const unsigned char * _header,
                          const unsigned char * _payload,
                          unsigned int          _payload_len);

// write samples of assembled frame, two samples at a time, returning
// '1' when frame is complete, '0' otherwise. Zeros will be written
// to the buffer if the frame is not assembled
//  _q          :   frame generator object
//  _buffer     :   output buffer, [size: _buffer_len x 1]
//  _buffer_len :   output buffer length
int flexframegen_write_samples(flexframegen           _q,
                               liquid_float_complex * _buffer,
                               unsigned int           _buffer_len);

// frame synchronizer

typedef struct flexframesync_s * flexframesync;

// create flexframesync object
//  _callback   :   callback function
//  _userdata   :   user data pointer passed to callback function
flexframesync flexframesync_create(framesync_callback _callback,
                                   void *             _userdata);

// destroy frame synchronizer
int flexframesync_destroy(flexframesync _q);

// print frame synchronizer internal properties
int flexframesync_print(flexframesync _q);

// reset frame synchronizer internal state
int flexframesync_reset(flexframesync _q);

// has frame been detected?
int flexframesync_is_frame_open(flexframesync _q);

// change length of user-defined region in header
int flexframesync_set_header_len(flexframesync _q,
                                 unsigned int  _len);

// enable or disable soft decoding of header
int flexframesync_decode_header_soft(flexframesync _q,
                                     int           _soft);

// enable or disable soft decoding of payload
int flexframesync_decode_payload_soft(flexframesync _q,
                                      int           _soft);

// set properties for header section
int flexframesync_set_header_props(flexframesync          _q,
                                   flexframegenprops_s * _props);

// push samples through frame synchronizer
//  _q      :   frame synchronizer object
//  _x      :   input samples, [size: _n x 1]
//  _n      :   number of input samples
int flexframesync_execute(flexframesync          _q,
                          liquid_float_complex * _x,
                          unsigned int           _n);

// frame data statistics
int              flexframesync_reset_framedatastats(flexframesync _q);
framedatastats_s flexframesync_get_framedatastats  (flexframesync _q);

// enable/disable debugging
int flexframesync_debug_enable(flexframesync _q);
int flexframesync_debug_disable(flexframesync _q);
int flexframesync_debug_print(flexframesync _q,
                               const char *  _filename);

//
// bpacket : binary packet suitable for data streaming
//

//
// bpacket generator/encoder
//
typedef struct bpacketgen_s * bpacketgen;

// create bpacketgen object
//  _m              :   p/n sequence length (ignored)
//  _dec_msg_len    :   decoded message length (original uncoded data)
//  _crc            :   data validity check (e.g. cyclic redundancy check)
//  _fec0           :   inner forward error-correction code scheme
//  _fec1           :   outer forward error-correction code scheme
bpacketgen bpacketgen_create(unsigned int _m,
                             unsigned int _dec_msg_len,
                             int _crc,
                             int _fec0,
                             int _fec1);

// re-create bpacketgen object from old object
//  _q              :   old bpacketgen object
//  _m              :   p/n sequence length (ignored)
//  _dec_msg_len    :   decoded message length (original uncoded data)
//  _crc            :   data validity check (e.g. cyclic redundancy check)
//  _fec0           :   inner forward error-correction code scheme
//  _fec1           :   outer forward error-correction code scheme
bpacketgen bpacketgen_recreate(bpacketgen _q,
                               unsigned int _m,
                               unsigned int _dec_msg_len,
                               int _crc,
                               int _fec0,
                               int _fec1);

// destroy bpacketgen object, freeing all internally-allocated memory
void bpacketgen_destroy(bpacketgen _q);

// print bpacketgen internals
void bpacketgen_print(bpacketgen _q);

// return length of full packet
unsigned int bpacketgen_get_packet_len(bpacketgen _q);

// encode packet
void bpacketgen_encode(bpacketgen _q,
                       unsigned char * _msg_dec,
                       unsigned char * _packet);

//
// bpacket synchronizer/decoder
//
typedef struct bpacketsync_s * bpacketsync;
typedef int (*bpacketsync_callback)(unsigned char *  _payload,
                                    int              _payload_valid,
                                    unsigned int     _payload_len,
                                    framesyncstats_s _stats,
                                    void *           _userdata);
bpacketsync bpacketsync_create(unsigned int _m,
                               bpacketsync_callback _callback,
                               void * _userdata);
int bpacketsync_destroy(bpacketsync _q);
int bpacketsync_print(bpacketsync _q);
int bpacketsync_reset(bpacketsync _q);

// run synchronizer on array of input bytes
//  _q      :   bpacketsync object
//  _bytes  :   input data array, [size: _n x 1]
//  _n      :   input array size
int bpacketsync_execute(bpacketsync     _q,
                        unsigned char * _bytes,
                        unsigned int    _n);

// run synchronizer on input byte
//  _q      :   bpacketsync object
//  _byte   :   input byte
int bpacketsync_execute_byte(bpacketsync   _q,
                             unsigned char _byte);

// run synchronizer on input symbol
//  _q      :   bpacketsync object
//  _sym    :   input symbol with _bps significant bits
//  _bps    :   number of bits in input symbol
int bpacketsync_execute_sym(bpacketsync   _q,
                            unsigned char _sym,
                            unsigned int  _bps);

// execute one bit at a time
int bpacketsync_execute_bit(bpacketsync   _q,
                            unsigned char _bit);

//
// M-FSK frame generator
//

typedef struct fskframegen_s * fskframegen;

// create M-FSK frame generator
fskframegen fskframegen_create();
int fskframegen_destroy (fskframegen     _fg);
int fskframegen_print   (fskframegen     _fg);
int fskframegen_reset   (fskframegen     _fg);
int fskframegen_assemble(fskframegen     _fg,
                         unsigned char * _header,
                         unsigned char * _payload,
                         unsigned int    _payload_len,
                         crc_scheme      _check,
                         fec_scheme      _fec0,
                         fec_scheme      _fec1);
unsigned int fskframegen_getframelen(fskframegen _q);
int fskframegen_write_samples(fskframegen            _fg,
                              liquid_float_complex * _buf,
                              unsigned int           _buf_len);


//
// M-FSK frame synchronizer
//

typedef struct fskframesync_s * fskframesync;

// create M-FSK frame synchronizer
//  _callback   :   callback function
//  _userdata   :   user data pointer passed to callback function
fskframesync fskframesync_create(framesync_callback _callback,
                                 void *             _userdata);
int fskframesync_destroy(fskframesync _q);
int fskframesync_print  (fskframesync _q);
int fskframesync_reset  (fskframesync _q);
int fskframesync_execute(fskframesync         _q,
                         liquid_float_complex _x);
int fskframesync_execute_block(fskframesync           _q,
                               liquid_float_complex * _x,
                               unsigned int           _n);

// debugging
int fskframesync_debug_enable (fskframesync _q);
int fskframesync_debug_disable(fskframesync _q);
int fskframesync_debug_export (fskframesync _q, const char * _filename);


//
// GMSK frame generator
//

typedef struct gmskframegen_s * gmskframegen;

// create GMSK frame generator with specific parameters
//  _k      :   samples/symbol
//  _m      :   filter delay (symbols)
//  _BT     :   excess bandwidth factor
gmskframegen gmskframegen_create_set(unsigned int _k,
                                     unsigned int _m,
                                     float        _BT);
// create default GMSK frame generator (k=2, m=3, BT=0.5)
gmskframegen gmskframegen_create();
int gmskframegen_destroy       (gmskframegen _q);
int gmskframegen_is_assembled  (gmskframegen _q);
int gmskframegen_print         (gmskframegen _q);
int gmskframegen_set_header_len(gmskframegen _q, unsigned int _len);
int gmskframegen_reset         (gmskframegen _q);
int gmskframegen_assemble      (gmskframegen          _q,
                                const unsigned char * _header,
                                const unsigned char * _payload,
                                unsigned int          _payload_len,
                                crc_scheme            _check,
                                fec_scheme            _fec0,
                                fec_scheme            _fec1);
// assemble default frame with a particular size payload
int gmskframegen_assemble_default(gmskframegen _q,
                                  unsigned int _payload_len);
unsigned int gmskframegen_getframelen(gmskframegen _q);

// write samples of assembled frame
//  _q          :   frame generator object
//  _buf        :   output buffer, [size: _buf_len x 1]
//  _buf_len    :   output buffer length
int gmskframegen_write(gmskframegen           _q,
                       liquid_float_complex * _buf,
                       unsigned int           _buf_len);

// write samples of assembled frame
//  _q          : frame generator object
//  _buf        : output buffer, [size: k x 1]
DEPRECATED("use gmskframegen_write(...) instead",
int gmskframegen_write_samples(gmskframegen           _q,
                               liquid_float_complex * _buf);
)


//
// GMSK frame synchronizer
//

typedef struct gmskframesync_s * gmskframesync;

// create GMSK frame synchronizer
//  _k          :   samples/symbol
//  _m          :   filter delay (symbols)
//  _BT         :   excess bandwidth factor
//  _callback   :   callback function
//  _userdata   :   user data pointer passed to callback function
gmskframesync gmskframesync_create_set(unsigned int       _k,
                                       unsigned int       _m,
                                       float              _BT,
                                       framesync_callback _callback,
                                       void *             _userdata);
// create GMSK frame synchronizer with default parameters (k=2, m=3, BT=0.5)
//  _callback   :   callback function
//  _userdata   :   user data pointer passed to callback function
gmskframesync gmskframesync_create(framesync_callback _callback,
                                   void *             _userdata);
int gmskframesync_destroy(gmskframesync _q);
int gmskframesync_print(gmskframesync _q);
int gmskframesync_set_header_len(gmskframesync _q, unsigned int _len);
int gmskframesync_reset(gmskframesync _q);
int gmskframesync_is_frame_open(gmskframesync _q);
int gmskframesync_execute(gmskframesync _q,
                          liquid_float_complex * _x,
                          unsigned int _n);
// frame data statistics
int              gmskframesync_reset_framedatastats(gmskframesync _q);
framedatastats_s gmskframesync_get_framedatastats  (gmskframesync _q);

// debug methods
DEPRECATED("debug methods add complexity and provide little value",
  int gmskframesync_debug_enable(gmskframesync _q); )
DEPRECATED("debug methods add complexity and provide little value",
  int gmskframesync_debug_disable(gmskframesync _q); )
DEPRECATED("debug methods add complexity and provide little value",
  int gmskframesync_debug_print(gmskframesync _q, const char * _filename); )


//
// DSSS frame generator
//

typedef struct {
    unsigned int check;
    unsigned int fec0;
    unsigned int fec1;
} dsssframegenprops_s;

typedef struct dsssframegen_s * dsssframegen;

dsssframegen dsssframegen_create(dsssframegenprops_s * _props);
int dsssframegen_destroy(dsssframegen _q);
int dsssframegen_reset(dsssframegen _q);
int dsssframegen_is_assembled(dsssframegen _q);
int dsssframegen_getprops(dsssframegen _q, dsssframegenprops_s * _props);
int dsssframegen_setprops(dsssframegen _q, dsssframegenprops_s * _props);
int dsssframegen_set_header_len(dsssframegen _q, unsigned int _len);
int dsssframegen_set_header_props(dsssframegen          _q,
                                  dsssframegenprops_s * _props);
unsigned int dsssframegen_getframelen(dsssframegen _q);

// assemble a frame from an array of data
//  _q              :   frame generator object
//  _header         :   frame header
//  _payload        :   payload data, [size: _payload_len x 1]
//  _payload_len    :   payload data length
int dsssframegen_assemble(dsssframegen          _q,
                          const unsigned char * _header,
                          const unsigned char * _payload,
                          unsigned int          _payload_len);

int dsssframegen_write_samples(dsssframegen           _q,
                               liquid_float_complex * _buffer,
                               unsigned int           _buffer_len);


//
// DSSS frame synchronizer
//

typedef struct dsssframesync_s * dsssframesync;

dsssframesync dsssframesync_create(framesync_callback _callback, void * _userdata);
int dsssframesync_destroy             (dsssframesync _q);
int dsssframesync_print               (dsssframesync _q);
int dsssframesync_reset               (dsssframesync _q);
int dsssframesync_is_frame_open       (dsssframesync _q);
int dsssframesync_set_header_len      (dsssframesync _q, unsigned int  _len);
int dsssframesync_decode_header_soft  (dsssframesync _q, int _soft);
int dsssframesync_decode_payload_soft (dsssframesync _q, int _soft);
int dsssframesync_set_header_props    (dsssframesync _q, dsssframegenprops_s * _props);
int dsssframesync_execute             (dsssframesync _q, liquid_float_complex * _x, unsigned int _n);
int dsssframesync_reset_framedatastats(dsssframesync _q);
int dsssframesync_debug_enable        (dsssframesync _q);
int dsssframesync_debug_disable       (dsssframesync _q);
int dsssframesync_debug_print         (dsssframesync _q, const char * _filename);
framedatastats_s dsssframesync_get_framedatastats  (dsssframesync _q);

//
// Direct sequence/spread spectrum framing with fixed 64-byte payload
//

// frame generator object type
typedef struct dsssframe64gen_s * dsssframe64gen;

// create dsssframe64gen object
dsssframe64gen dsssframe64gen_create();

// copy object
dsssframe64gen dsssframe64gen_copy(dsssframe64gen q_orig);

// destroy dsssframe64gen object
int dsssframe64gen_destroy(dsssframe64gen _q);

// print dsssframe64gen object internals
int dsssframe64gen_print(dsssframe64gen _q);

// get length of assembled frame (samples)
unsigned int dsssframe64gen_get_frame_len(dsssframe64gen _q);

// generate a frame
//  _q          :   frame generator object
//  _header     :   8-byte header data, NULL for random
//  _payload    :   64-byte payload data, NULL for random
//  _frame      :   output frame samples, [size: dsssframegen64gen_get_frame_len() x 1]
int dsssframe64gen_execute(dsssframe64gen         _q,
                           const unsigned char *  _header,
                           const unsigned char *  _payload,
                           liquid_float_complex * _buf);

// get full frame length [samples]
unsigned int dsssframe64gen_get_frame_len(dsssframe64gen _q);

// frame synchronizer object type
typedef struct dsssframe64sync_s * dsssframe64sync;

dsssframe64sync dsssframe64sync_create(framesync_callback _callback, void * _userdata);

// copy object
dsssframe64sync dsssframe64sync_copy(dsssframe64sync q_orig);

int dsssframe64sync_destroy             (dsssframe64sync _q);
int dsssframe64sync_print               (dsssframe64sync _q);
int dsssframe64sync_reset               (dsssframe64sync _q);
int dsssframe64sync_is_frame_open       (dsssframe64sync _q);
int dsssframe64sync_set_callback(dsssframe64sync    _q,
                                 framesync_callback _callback);
int dsssframe64sync_set_context(dsssframe64sync _q,
                                void *          _context);
// execute frame synchronizer
//  _q       : frame synchronizer object
//  _buf     : input sample array, shape: (_buf_len,)
//  _buf_len : number of input samples
int dsssframe64sync_execute(dsssframe64sync _q, liquid_float_complex * _buf, unsigned int _buf_len);

// get detection threshold
float dsssframe64sync_get_threshold(dsssframe64sync _q);

// set detection threshold
int dsssframe64sync_set_threshold(dsssframe64sync _q,
                                  float           _threshold);

// get carrier offset search range [radians/sample]
float dsssframe64sync_get_range(dsssframe64sync _q);

// set carrier offset search range
int dsssframe64sync_set_range(dsssframe64sync _q,
                              float           _dphi_max);

// reset frame data statistics
int dsssframe64sync_reset_framedatastats(dsssframe64sync _q);

// retrieve frame data statistics
framedatastats_s dsssframe64sync_get_framedatastats(dsssframe64sync _q);

//
// OFDM flexframe generator
//

// ofdm frame generator properties
typedef struct {
    unsigned int check;         // data validity check
    unsigned int fec0;          // forward error-correction scheme (inner)
    unsigned int fec1;          // forward error-correction scheme (outer)
    unsigned int mod_scheme;    // modulation scheme
    //unsigned int block_size;  // framing block size
} ofdmflexframegenprops_s;
int ofdmflexframegenprops_init_default(ofdmflexframegenprops_s * _props);

typedef struct ofdmflexframegen_s * ofdmflexframegen;

// create OFDM flexible framing generator object
//  _M          :   number of subcarriers, >10 typical
//  _cp_len     :   cyclic prefix length
//  _taper_len  :   taper length (OFDM symbol overlap)
//  _p          :   subcarrier allocation (null, pilot, data), [size: _M x 1]
//  _fgprops    :   frame properties (modulation scheme, etc.)
ofdmflexframegen ofdmflexframegen_create(unsigned int              _M,
                                         unsigned int              _cp_len,
                                         unsigned int              _taper_len,
                                         unsigned char *           _p,
                                         ofdmflexframegenprops_s * _fgprops);

// destroy ofdmflexframegen object
int ofdmflexframegen_destroy(ofdmflexframegen _q);

// print parameters, properties, etc.
int ofdmflexframegen_print(ofdmflexframegen _q);

// reset ofdmflexframegen object internals
int ofdmflexframegen_reset(ofdmflexframegen _q);

// is frame assembled?
int ofdmflexframegen_is_assembled(ofdmflexframegen _q);

// get properties
int ofdmflexframegen_getprops(ofdmflexframegen _q,
                              ofdmflexframegenprops_s * _props);

// set properties
int ofdmflexframegen_setprops(ofdmflexframegen _q,
                              ofdmflexframegenprops_s * _props);

// set user-defined header length
int ofdmflexframegen_set_header_len(ofdmflexframegen _q,
                                    unsigned int     _len);

int ofdmflexframegen_set_header_props(ofdmflexframegen _q,
                                      ofdmflexframegenprops_s * _props);

// get length of frame (symbols)
//  _q              :   OFDM frame generator object
unsigned int ofdmflexframegen_getframelen(ofdmflexframegen _q);

// assemble a frame from an array of data (NULL pointers will use random data)
//  _q              :   OFDM frame generator object
//  _header         :   frame header [8 bytes]
//  _payload        :   payload data, [size: _payload_len x 1]
//  _payload_len    :   payload data length
int ofdmflexframegen_assemble(ofdmflexframegen      _q,
                              const unsigned char * _header,
                              const unsigned char * _payload,
                              unsigned int          _payload_len);

// write samples of assembled frame
//  _q              :   OFDM frame generator object
//  _buf            :   output buffer, [size: _buf_len x 1]
//  _buf_len        :   output buffer length
int ofdmflexframegen_write(ofdmflexframegen       _q,
                           liquid_float_complex * _buf,
                           unsigned int           _buf_len);

//
// OFDM flex frame synchronizer
//

typedef struct ofdmflexframesync_s * ofdmflexframesync;

// create OFDM flexible framing synchronizer object
//  _M          :   number of subcarriers
//  _cp_len     :   cyclic prefix length
//  _taper_len  :   taper length (OFDM symbol overlap)
//  _p          :   subcarrier allocation (null, pilot, data), [size: _M x 1]
//  _callback   :   user-defined callback function
//  _userdata   :   user-defined data pointer
ofdmflexframesync ofdmflexframesync_create(unsigned int       _M,
                                           unsigned int       _cp_len,
                                           unsigned int       _taper_len,
                                           unsigned char *    _p,
                                           framesync_callback _callback,
                                           void *             _userdata);

int ofdmflexframesync_destroy(ofdmflexframesync _q);
int ofdmflexframesync_print(ofdmflexframesync _q);
// set user-defined header length
int ofdmflexframesync_set_header_len(ofdmflexframesync _q,
                                     unsigned int      _len);

int ofdmflexframesync_decode_header_soft(ofdmflexframesync _q,
                                         int _soft);

int ofdmflexframesync_decode_payload_soft(ofdmflexframesync _q,
                                          int _soft);

int ofdmflexframesync_set_header_props(ofdmflexframesync _q,
                                       ofdmflexframegenprops_s * _props);

int ofdmflexframesync_reset(ofdmflexframesync _q);

// set the callback and userdata fields
int ofdmflexframesync_set_callback(ofdmflexframesync _q, framesync_callback _callback);
int ofdmflexframesync_set_userdata(ofdmflexframesync _q, void *             _userdata);

int  ofdmflexframesync_is_frame_open(ofdmflexframesync _q);
int ofdmflexframesync_execute(ofdmflexframesync _q,
                              liquid_float_complex * _x,
                              unsigned int _n);

// query the received signal strength indication
float ofdmflexframesync_get_rssi(ofdmflexframesync _q);

// query the received carrier offset estimate
float ofdmflexframesync_get_cfo(ofdmflexframesync _q);

// frame data statistics
int              ofdmflexframesync_reset_framedatastats(ofdmflexframesync _q);
framedatastats_s ofdmflexframesync_get_framedatastats  (ofdmflexframesync _q);

// set the received carrier offset estimate
int ofdmflexframesync_set_cfo(ofdmflexframesync _q, float _cfo);

// enable/disable debugging
int ofdmflexframesync_debug_enable(ofdmflexframesync _q);
int ofdmflexframesync_debug_disable(ofdmflexframesync _q);
int ofdmflexframesync_debug_print(ofdmflexframesync _q,
                                  const char *      _filename);



//
// Binary P/N synchronizer
//
#define LIQUID_BSYNC_MANGLE_RRRF(name) LIQUID_CONCAT(bsync_rrrf,name)
#define LIQUID_BSYNC_MANGLE_CRCF(name) LIQUID_CONCAT(bsync_crcf,name)
#define LIQUID_BSYNC_MANGLE_CCCF(name) LIQUID_CONCAT(bsync_cccf,name)

// Macro:
//   BSYNC  : name-mangling macro
//   TO     : output data type
//   TC     : coefficients data type
//   TI     : input data type
#define LIQUID_BSYNC_DEFINE_API(BSYNC,TO,TC,TI)                             \
                                                                            \
/* Binary P/N synchronizer                                              */  \
typedef struct BSYNC(_s) * BSYNC();                                         \
                                                                            \
/* Create bsync object                                                  */  \
/*  _n  : sequence length                                               */  \
/*  _v  : correlation sequence, [size: _n x 1]                          */  \
BSYNC() BSYNC(_create)(unsigned int _n,                                     \
                       TC *         _v);                                    \
                                                                            \
/* Create binary synchronizer from m-sequence                           */  \
/*  _g  :   m-sequence generator polynomial                             */  \
/*  _k  :   samples/symbol (over-sampling factor)                       */  \
BSYNC() BSYNC(_create_msequence)(unsigned int _g,                           \
                                 unsigned int _k);                          \
                                                                            \
/* Destroy binary synchronizer object, freeing all internal memory      */  \
/*  _q  :   bsync object                                                */  \
int BSYNC(_destroy)(BSYNC() _q);                                            \
                                                                            \
/* Print object internals to stdout                                     */  \
/*  _q  :   bsync object                                                */  \
int BSYNC(_print)(BSYNC() _q);                                              \
                                                                            \
/* Correlate input signal against internal sequence                     */  \
/*  _q  :   bsync object                                                */  \
/*  _x  :   input sample                                                */  \
/*  _y  :   pointer to output sample                                    */  \
int BSYNC(_correlate)(BSYNC() _q,                                           \
                      TI      _x,                                           \
                      TO *    _y);                                          \

LIQUID_BSYNC_DEFINE_API(LIQUID_BSYNC_MANGLE_RRRF,
                        float,
                        float,
                        float)

LIQUID_BSYNC_DEFINE_API(LIQUID_BSYNC_MANGLE_CRCF,
                        liquid_float_complex,
                        float,
                        liquid_float_complex)

LIQUID_BSYNC_DEFINE_API(LIQUID_BSYNC_MANGLE_CCCF,
                        liquid_float_complex,
                        liquid_float_complex,
                        liquid_float_complex)


//
// Pre-demodulation synchronizers (binary and otherwise)
//
#define  LIQUID_PRESYNC_MANGLE_CCCF(name) LIQUID_CONCAT( presync_cccf,name)
#define LIQUID_BPRESYNC_MANGLE_CCCF(name) LIQUID_CONCAT(bpresync_cccf,name)

// Macro:
//   PRESYNC   : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_PRESYNC_DEFINE_API(PRESYNC,TO,TC,TI)                         \
                                                                            \
/* Pre-demodulation signal synchronizer                                 */  \
typedef struct PRESYNC(_s) * PRESYNC();                                     \
                                                                            \
/* Create pre-demod synchronizer from external sequence                 */  \
/*  _v          : baseband sequence, [size: _n x 1]                     */  \
/*  _n          : baseband sequence length, _n > 0                      */  \
/*  _dphi_max   : maximum absolute frequency deviation for detection    */  \
/*  _m          : number of correlators, _m > 0                         */  \
PRESYNC() PRESYNC(_create)(TC *         _v,                                 \
                           unsigned int _n,                                 \
                           float        _dphi_max,                          \
                           unsigned int _m);                                \
                                                                            \
/* Destroy pre-demod synchronizer, freeing all internal memory          */  \
int PRESYNC(_destroy)(PRESYNC() _q);                                        \
                                                                            \
/* Print pre-demod synchronizer internal state                          */  \
int PRESYNC(_print)(PRESYNC() _q);                                          \
                                                                            \
/* Reset pre-demod synchronizer internal state                          */  \
int PRESYNC(_reset)(PRESYNC() _q);                                          \
                                                                            \
/* Push input sample into pre-demod synchronizer                        */  \
/*  _q          : pre-demod synchronizer object                         */  \
/*  _x          : input sample                                          */  \
int PRESYNC(_push)(PRESYNC() _q,                                            \
                    TI        _x);                                          \
                                                                            \
/* Correlate original sequence with internal input buffer               */  \
/*  _q          : pre-demod synchronizer object                         */  \
/*  _rxy        : output cross correlation                              */  \
/*  _dphi_hat   : output frequency offset estimate                      */  \
int PRESYNC(_execute)(PRESYNC() _q,                                         \
                       TO *      _rxy,                                      \
                       float *   _dphi_hat);                                \

// non-binary pre-demodulation synchronizer
LIQUID_PRESYNC_DEFINE_API(LIQUID_PRESYNC_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)

// binary pre-demodulation synchronizer
LIQUID_PRESYNC_DEFINE_API(LIQUID_BPRESYNC_MANGLE_CCCF,
                          liquid_float_complex,
                          liquid_float_complex,
                          liquid_float_complex)

//
// Frame detector
//

#define LIQUID_QDETECTOR_MANGLE_CCCF(name) LIQUID_CONCAT(qdetector_cccf,name)

#define LIQUID_QDETECTOR_DEFINE_API(QDETECTOR,TO,TC,TI)                     \
                                                                            \
/* Frame detector and synchronizer; uses a novel correlation method to  */  \
/* detect a synchronization pattern, estimate carrier frequency and     */  \
/* phase offsets as well as timing phase, then correct for these        */  \
/* impairments in a simple interface suitable for custom frame recovery.*/  \
typedef struct QDETECTOR(_s) * QDETECTOR();                                 \
                                                                            \
/* Create detector with generic sequence                                */  \
/*  _s      :   sample sequence                                         */  \
/*  _s_len  :   length of sample sequence                               */  \
QDETECTOR() QDETECTOR(_create)(TI *         _s,                             \
                               unsigned int _s_len);                        \
                                                                            \
/* Create detector from sequence of symbols using internal linear       */  \
/* interpolator                                                         */  \
/*  _sequence       :   symbol sequence                                 */  \
/*  _sequence_len   :   length of symbol sequence                       */  \
/*  _ftype          :   filter prototype (e.g. LIQUID_FIRFILT_RRC)      */  \
/*  _k              :   samples/symbol                                  */  \
/*  _m              :   filter delay                                    */  \
/*  _beta           :   excess bandwidth factor                         */  \
QDETECTOR() QDETECTOR(_create_linear)(TI *         _sequence,               \
                                      unsigned int _sequence_len,           \
                                      int          _ftype,                  \
                                      unsigned int _k,                      \
                                      unsigned int _m,                      \
                                      float        _beta);                  \
                                                                            \
/* create detector from sequence of GMSK symbols                        */  \
/*  _sequence       :   bit sequence                                    */  \
/*  _sequence_len   :   length of bit sequence                          */  \
/*  _k              :   samples/symbol                                  */  \
/*  _m              :   filter delay                                    */  \
/*  _beta           :   excess bandwidth factor                         */  \
QDETECTOR() QDETECTOR(_create_gmsk)(unsigned char * _sequence,              \
                                    unsigned int    _sequence_len,          \
                                    unsigned int    _k,                     \
                                    unsigned int    _m,                     \
                                    float           _beta);                 \
                                                                            \
/* create detector from sequence of CP-FSK symbols (assuming one        */  \
/* bit/symbol)                                                          */  \
/*  _sequence       :   bit sequence                                    */  \
/*  _sequence_len   :   length of bit sequence                          */  \
/*  _bps            :   bits per symbol, 0 < _bps <= 8                  */  \
/*  _h              :   modulation index, _h > 0                        */  \
/*  _k              :   samples/symbol                                  */  \
/*  _m              :   filter delay                                    */  \
/*  _beta           :   filter bandwidth parameter, _beta > 0           */  \
/*  _type           :   filter type (e.g. LIQUID_CPFSK_SQUARE)          */  \
QDETECTOR() QDETECTOR(_create_cpfsk)(unsigned char * _sequence,             \
                                     unsigned int    _sequence_len,         \
                                     unsigned int    _bps,                  \
                                     float           _h,                    \
                                     unsigned int    _k,                    \
                                     unsigned int    _m,                    \
                                     float           _beta,                 \
                                     int             _type);                \
                                                                            \
/* Copy object including all internal objects and state                 */  \
QDETECTOR() QDETECTOR(_copy)(QDETECTOR() _q);                               \
                                                                            \
/* Destroy synchronizer object and free all internal memory             */  \
int QDETECTOR(_destroy)(QDETECTOR() _q);                                    \
                                                                            \
/* Reset synchronizer object's internal buffer                          */  \
int QDETECTOR(_reset)(QDETECTOR() _q);                                      \
                                                                            \
/* Print synchronizer object information to stdout                      */  \
int QDETECTOR(_print)(QDETECTOR() _q);                                      \
                                                                            \
/* run detector, looking for sequence; return pointer to aligned,       */  \
/* buffered samples                                                     */  \
void * QDETECTOR(_execute)(QDETECTOR() _q, TI _x);                          \
                                                                            \
/* get detection threshold                                              */  \
float QDETECTOR(_get_threshold)(QDETECTOR() _q);                            \
                                                                            \
/* Set detection threshold (should be between 0 and 1, good starting    */  \
/* point is 0.5)                                                        */  \
int QDETECTOR(_set_threshold)(QDETECTOR() _q,                               \
                              float       _threshold);                      \
                                                                            \
/* Get carrier offset search range                                      */  \
float QDETECTOR(_get_range)(QDETECTOR() _q);                                \
                                                                            \
/* Set carrier offset search range                                      */  \
int QDETECTOR(_set_range)(QDETECTOR() _q,                                   \
                          float       _dphi_max);                           \
                                                                            \
/* Get sequence length                                                  */  \
unsigned int QDETECTOR(_get_seq_len)(QDETECTOR() _q);                       \
                                                                            \
/* Get pointer to original sequence                                     */  \
const void * QDETECTOR(_get_sequence)(QDETECTOR() _q);                      \
                                                                            \
/* Get buffer length                                                    */  \
unsigned int QDETECTOR(_get_buf_len)(QDETECTOR() _q);                       \
                                                                            \
/* Get correlator output of detected frame                              */  \
float QDETECTOR(_get_rxy)(QDETECTOR() _q);                                  \
                                                                            \
/* Get fractional timing offset estimate of detected frame              */  \
float QDETECTOR(_get_tau)(QDETECTOR() _q);                                  \
                                                                            \
/* Get channel gain of detected frame                                   */  \
float QDETECTOR(_get_gamma)(QDETECTOR() _q);                                \
                                                                            \
/* Get carrier frequency offset estimateof detected frame               */  \
float QDETECTOR(_get_dphi)(QDETECTOR() _q);                                 \
                                                                            \
/* Get carrier phase offset estimate of detected frame                  */  \
float QDETECTOR(_get_phi)(QDETECTOR() _q);                                  \

LIQUID_QDETECTOR_DEFINE_API(LIQUID_QDETECTOR_MANGLE_CCCF,
                         liquid_float_complex,
                         liquid_float_complex,
                         liquid_float_complex)

// metadata struct:
//  - sample count since object was created
//  - sample count since beginning of frame

//
// qdsync
//
#define LIQUID_QDSYNC_MANGLE_CCCF(name) LIQUID_CONCAT(qdsync_cccf,name)

#define LIQUID_QDSYNC_DEFINE_API(QDSYNC,TO,TC,TI)                           \
                                                                            \
/* Frame detector and synchronizer; uses a novel correlation method to  */  \
/* detect a synchronization pattern, estimate carrier frequency and     */  \
/* phase offsets as well as timing phase, then correct for these        */  \
/* impairments in a simple interface suitable for custom frame recovery.*/  \
typedef struct QDSYNC(_s) * QDSYNC();                                       \
                                                                            \
/* synchronization callback, return 0:continue, 1:reset                 */  \
typedef int (*QDSYNC(_callback))(TO *         _buf,                         \
                                 unsigned int _buf_len,                     \
                                 void *       _context);                    \
                                                                            \
/* create detector with generic sequence                                */  \
/*  _s          : sample sequence                                       */  \
/*  _s_len      : length of sample sequence                             */  \
/*  _ftype      : filter type                                           */  \
/*  _k          : samples per symbol                                    */  \
/*  _m          : filter semi-length                                    */  \
/*  _beta       : filter excess bandwidth factor                        */  \
/*  _callback   : user-defined callback                                 */  \
/*  _context    : user-defined context                                  */  \
QDSYNC() QDSYNC(_create_linear)(TI *              _s,                       \
                                unsigned int      _s_len,                   \
                                int               _ftype,                   \
                                unsigned int      _k,                       \
                                unsigned int      _m,                       \
                                float             _beta,                    \
                                QDSYNC(_callback) _callback,                \
                                void *            _context);                \
                                                                            \
/* Copy object recursively including all internal objects and state     */  \
QDSYNC() QDSYNC(_copy)(QDSYNC() _q);                                        \
                                                                            \
/* Destroy synchronizer object and free all internal memory             */  \
int QDSYNC(_destroy)(QDSYNC() _q);                                          \
                                                                            \
/* Reset synchronizer object's internal buffer                          */  \
int QDSYNC(_reset)(QDSYNC() _q);                                            \
                                                                            \
/* Print synchronizer object information to stdout                      */  \
int QDSYNC(_print)(QDSYNC() _q);                                            \
                                                                            \
/* Get detection state                                                  */  \
int QDSYNC(_is_detected)(QDSYNC() _q);                                  \
                                                                            \
/* Get detection threshold                                              */  \
float QDSYNC(_get_threshold)(QDSYNC() _q);                                  \
                                                                            \
/* Set detection threshold                                              */  \
int QDSYNC(_set_threshold)(QDSYNC() _q,                                     \
                           float    _threshold);                            \
                                                                            \
/* Get carrier offset search range                                      */  \
float QDSYNC(_get_range)(QDSYNC() _q);                                      \
                                                                            \
/* Set carrier offset search range                                      */  \
int QDSYNC(_set_range)(QDSYNC() _q,                                         \
                       float    _dphi_max);                                 \
                                                                            \
/* Set callback method                                                  */  \
int QDSYNC(_set_callback)(QDSYNC()          _q,                             \
                          QDSYNC(_callback) _callback);                     \
                                                                            \
/* Set context value                                                    */  \
int QDSYNC(_set_context)(QDSYNC() _q, void * _context);                     \
                                                                            \
/* Set callback buffer size (the number of symbol provided to the       */  \
/* callback whenever it is invoked).                                    */  \
int QDSYNC(_set_buf_len)(QDSYNC() _q, unsigned int _buf_len);               \
                                                                            \
/* execute block of samples                                             */  \
int QDSYNC(_execute)(QDSYNC()     _q,                                       \
                     TI *         _buf,                                     \
                     unsigned int _buf_len);                                \
                                                                            \
/* Return flag indicating if synchronizer actively running.             */  \
int QDSYNC(_is_open)(QDSYNC() _q);                                          \
                                                                            \
/* Get synchronizer correlator output after frame was detected          */  \
float QDSYNC(_get_rxy)  (QDSYNC() _q);                                      \
                                                                            \
/* Get synchronizer fractional timing offset after frame was detected   */  \
float QDSYNC(_get_tau)  (QDSYNC() _q);                                      \
                                                                            \
/* Get synchronizer channel gain after frame was detected               */  \
float QDSYNC(_get_gamma)(QDSYNC() _q);                                      \
                                                                            \
/* Get synchronizer frequency offset estimate after frame was detected  */  \
float QDSYNC(_get_dphi) (QDSYNC() _q);                                      \
                                                                            \
/* Get synchronizer phase offset estimate after frame was detected      */  \
float QDSYNC(_get_phi)  (QDSYNC() _q);                                      \

LIQUID_QDSYNC_DEFINE_API(LIQUID_QDSYNC_MANGLE_CCCF,
                         liquid_float_complex,
                         liquid_float_complex,
                         liquid_float_complex)

//
// Pre-demodulation detector
//

typedef struct detector_cccf_s * detector_cccf;

// create pre-demod detector
//  _s          :   sequence
//  _n          :   sequence length
//  _threshold  :   detection threshold (default: 0.7)
//  _dphi_max   :   maximum carrier offset
detector_cccf detector_cccf_create(liquid_float_complex * _s,
                                   unsigned int           _n,
                                   float                  _threshold,
                                   float                  _dphi_max);

// destroy pre-demo detector object
void detector_cccf_destroy(detector_cccf _q);

// print pre-demod detector internal state
void detector_cccf_print(detector_cccf _q);

// reset pre-demod detector internal state
void detector_cccf_reset(detector_cccf _q);

// Run sample through pre-demod detector's correlator.
// Returns '1' if signal was detected, '0' otherwise
//  _q          :   pre-demod detector
//  _x          :   input sample
//  _tau_hat    :   fractional sample offset estimate (set when detected)
//  _dphi_hat   :   carrier frequency offset estimate (set when detected)
//  _gamma_hat  :   channel gain estimate (set when detected)
int detector_cccf_correlate(detector_cccf        _q,
                            liquid_float_complex _x,
                            float *              _tau_hat,
                            float *              _dphi_hat,
                            float *              _gamma_hat);


//
// symbol streaming for testing (no meaningful data, just symbols)
//
#define LIQUID_SYMSTREAM_MANGLE_CFLOAT(name) LIQUID_CONCAT(symstreamcf,name)

#define LIQUID_SYMSTREAM_DEFINE_API(SYMSTREAM,TO)                           \
                                                                            \
/* Symbol streaming generator object                                    */  \
typedef struct SYMSTREAM(_s) * SYMSTREAM();                                 \
                                                                            \
/* Create symstream object with default parameters.                     */  \
/* This is equivalent to invoking the create_linear() method            */  \
/* with _ftype=LIQUID_FIRFILT_ARKAISER, _k=2, _m=7, _beta=0.3, and      */  \
/* with _ms=LIQUID_MODEM_QPSK                                           */  \
SYMSTREAM() SYMSTREAM(_create)(void);                                       \
                                                                            \
/* Create symstream object with linear modulation                       */  \
/*  _ftype  : filter type (e.g. LIQUID_FIRFILT_RRC)                     */  \
/*  _k      : samples per symbol, _k >= 2                               */  \
/*  _m      : filter delay (symbols), _m > 0                            */  \
/*  _beta   : filter excess bandwidth, 0 < _beta <= 1                   */  \
/*  _ms     : modulation scheme, e.g. LIQUID_MODEM_QPSK                 */  \
SYMSTREAM() SYMSTREAM(_create_linear)(int          _ftype,                  \
                                      unsigned int _k,                      \
                                      unsigned int _m,                      \
                                      float        _beta,                   \
                                      int          _ms);                    \
                                                                            \
/* Copy object recursively including all internal objects and state     */  \
SYMSTREAM() SYMSTREAM(_copy)(SYMSTREAM() _q);                               \
                                                                            \
/* Destroy symstream object, freeing all internal memory                */  \
int SYMSTREAM(_destroy)(SYMSTREAM() _q);                                    \
                                                                            \
/* Print symstream object's parameters                                  */  \
int SYMSTREAM(_print)(SYMSTREAM() _q);                                      \
                                                                            \
/* Reset symstream internal state                                       */  \
int SYMSTREAM(_reset)(SYMSTREAM() _q);                                      \
                                                                            \
/* Set internal linear modulation scheme, leaving the filter parameters */  \
/* (interpolator) unmodified                                            */  \
int SYMSTREAM(_set_scheme)(SYMSTREAM() _q,                                  \
                           int         _ms);                                \
                                                                            \
/* Get internal filter type                                             */  \
int SYMSTREAM(_get_ftype)(SYMSTREAM() _q);                                  \
                                                                            \
/* Get internal samples per symbol                                      */  \
float SYMSTREAM(_get_k)(SYMSTREAM() _q);                                    \
                                                                            \
/* Get internal filter semi-length                                      */  \
unsigned int SYMSTREAM(_get_m)(SYMSTREAM() _q);                             \
                                                                            \
/* Get internal filter excess bandwidth factor                          */  \
float SYMSTREAM(_get_beta)(SYMSTREAM() _q);                                 \
                                                                            \
/* Get internal linear modulation scheme                                */  \
int SYMSTREAM(_get_scheme)(SYMSTREAM() _q);                                 \
                                                                            \
/* Set internal linear gain (before interpolation)                      */  \
int SYMSTREAM(_set_gain)(SYMSTREAM() _q,                                    \
                         float       _gain);                                \
                                                                            \
/* Get internal linear gain (before interpolation)                      */  \
float SYMSTREAM(_get_gain)(SYMSTREAM() _q);                                 \
                                                                            \
/* Get delay in samples                                                 */  \
unsigned int SYMSTREAM(_get_delay)(SYMSTREAM() _q);                         \
                                                                            \
/* Write block of samples to output buffer                              */  \
/*  _q      : synchronizer object                                       */  \
/*  _buf    : output buffer, [size: _buf_len x 1]                       */  \
/*  _buf_len: output buffer size                                        */  \
int SYMSTREAM(_write_samples)(SYMSTREAM()  _q,                              \
                              TO *         _buf,                            \
                              unsigned int _buf_len);                       \

LIQUID_SYMSTREAM_DEFINE_API(LIQUID_SYMSTREAM_MANGLE_CFLOAT, liquid_float_complex)

//
// symbol streaming, as with symstream but arbitrary output rate
//
#define LIQUID_SYMSTREAMR_MANGLE_CFLOAT(name) LIQUID_CONCAT(symstreamrcf,name)

#define LIQUID_SYMSTREAMR_DEFINE_API(SYMSTREAMR,TO)                         \
                                                                            \
/* Symbol streaming generator object                                    */  \
typedef struct SYMSTREAMR(_s) * SYMSTREAMR();                               \
                                                                            \
/* Create symstream object with default parameters.                     */  \
/* This is equivalent to invoking the create_linear() method            */  \
/* with _ftype=LIQUID_FIRFILT_ARKAISER, _k=2, _m=7, _beta=0.3, and      */  \
/* with _ms=LIQUID_MODEM_QPSK                                           */  \
SYMSTREAMR() SYMSTREAMR(_create)(void);                                     \
                                                                            \
/* Create symstream object with linear modulation                       */  \
/*  _ftype  : filter type (e.g. LIQUID_FIRFILT_RRC)                     */  \
/*  _bw     : relative signal bandwidth, 0.001 <= _bw <= 1.0            */  \
/*  _m      : filter delay (symbols), _m > 0                            */  \
/*  _beta   : filter excess bandwidth, 0 < _beta <= 1                   */  \
/*  _ms     : modulation scheme, e.g. LIQUID_MODEM_QPSK                 */  \
SYMSTREAMR() SYMSTREAMR(_create_linear)(int          _ftype,                \
                                        float        _bw,                   \
                                        unsigned int _m,                    \
                                        float        _beta,                 \
                                        int          _ms);                  \
                                                                            \
/* Copy object recursively including all internal objects and state     */  \
SYMSTREAMR() SYMSTREAMR(_copy)(SYMSTREAMR() _q);                            \
                                                                            \
/* Destroy symstream object, freeing all internal memory                */  \
int SYMSTREAMR(_destroy)(SYMSTREAMR() _q);                                  \
                                                                            \
/* Print symstream object's parameters                                  */  \
int SYMSTREAMR(_print)(SYMSTREAMR() _q);                                    \
                                                                            \
/* Reset symstream internal state                                       */  \
int SYMSTREAMR(_reset)(SYMSTREAMR() _q);                                    \
                                                                            \
/* Get internal filter type                                             */  \
int SYMSTREAMR(_get_ftype)(SYMSTREAMR() _q);                                \
                                                                            \
/* Get internal signal bandwidth (symbol rate)                          */  \
float SYMSTREAMR(_get_bw)(SYMSTREAMR() _q);                                 \
                                                                            \
/* Get internal filter semi-length                                      */  \
unsigned int SYMSTREAMR(_get_m)(SYMSTREAMR() _q);                           \
                                                                            \
/* Get internal filter excess bandwidth factor                          */  \
float SYMSTREAMR(_get_beta)(SYMSTREAMR() _q);                               \
                                                                            \
/* Set internal linear modulation scheme, leaving the filter parameters */  \
/* (interpolator) unmodified                                            */  \
int SYMSTREAMR(_set_scheme)(SYMSTREAMR() _q,                                \
                           int         _ms);                                \
                                                                            \
/* Get internal linear modulation scheme                                */  \
int SYMSTREAMR(_get_scheme)(SYMSTREAMR() _q);                               \
                                                                            \
/* Set internal linear gain (before interpolation)                      */  \
int SYMSTREAMR(_set_gain)(SYMSTREAMR() _q,                                  \
                         float       _gain);                                \
                                                                            \
/* Get internal linear gain (before interpolation)                      */  \
float SYMSTREAMR(_get_gain)(SYMSTREAMR() _q);                               \
                                                                            \
/* Get delay in samples                                                 */  \
float SYMSTREAMR(_get_delay)(SYMSTREAMR() _q);                              \
                                                                            \
/* Write block of samples to output buffer                              */  \
/*  _q      : synchronizer object                                       */  \
/*  _buf    : output buffer, [size: _buf_len x 1]                       */  \
/*  _buf_len: output buffer size                                        */  \
int SYMSTREAMR(_write_samples)(SYMSTREAMR()  _q,                            \
                              TO *         _buf,                            \
                              unsigned int _buf_len);                       \

LIQUID_SYMSTREAMR_DEFINE_API(LIQUID_SYMSTREAMR_MANGLE_CFLOAT, liquid_float_complex)



//
// multi-signal source for testing (no meaningful data, just signals)
//

#define LIQUID_MSOURCE_MANGLE_CFLOAT(name) LIQUID_CONCAT(msourcecf,name)

#define LIQUID_MSOURCE_DEFINE_API(MSOURCE,TO)                               \
                                                                            \
/* Multi-signal source generator object                                 */  \
typedef struct MSOURCE(_s) * MSOURCE();                                     \
                                                                            \
/* Create msource object by specifying channelizer parameters           */  \
/*  _M  :   number of channels in analysis channelizer object           */  \
/*  _m  :   prototype channelizer filter semi-length                    */  \
/*  _as :   prototype channelizer filter stop-band suppression (dB)     */  \
MSOURCE() MSOURCE(_create)(unsigned int _M,                                 \
                           unsigned int _m,                                 \
                           float        _as);                               \
                                                                            \
/* Copy object recursively, including all internal objects and state    */  \
MSOURCE() MSOURCE(_copy)(MSOURCE() _q);                                     \
                                                                            \
/* Create default msource object with default parameters:               */  \
/* M = 1200, m = 4, as = 60                                             */  \
MSOURCE() MSOURCE(_create_default)(void);                                   \
                                                                            \
/* Destroy msource object                                               */  \
int MSOURCE(_destroy)(MSOURCE() _q);                                        \
                                                                            \
/* Print msource object                                                 */  \
int MSOURCE(_print)(MSOURCE() _q);                                          \
                                                                            \
/* Reset msource object                                                 */  \
int MSOURCE(_reset)(MSOURCE() _q);                                          \
                                                                            \
/* user-defined callback for generating samples                         */  \
typedef int (*MSOURCE(_callback))(void *       _userdata,                   \
                                  TO *         _v,                          \
                                  unsigned int _n);                         \
                                                                            \
/* Add user-defined signal generator                                    */  \
int MSOURCE(_add_user)(MSOURCE()          _q,                               \
                       float              _fc,                              \
                       float              _bw,                              \
                       float              _gain,                            \
                       void *             _userdata,                        \
                       MSOURCE(_callback) _callback);                       \
                                                                            \
/* Add tone to signal generator, returning id of signal                 */  \
int MSOURCE(_add_tone)(MSOURCE() _q,                                        \
                       float     _fc,                                       \
                       float     _bw,                                       \
                       float     _gain);                                    \
                                                                            \
/* Add chirp to signal generator, returning id of signal                */  \
/*  _q          : multi-signal source object                            */  \
/*  _duration   : duration of chirp [samples]                           */  \
/*  _negate     : negate frequency direction                            */  \
/*  _single     : run single chirp? or repeatedly                       */  \
int MSOURCE(_add_chirp)(MSOURCE() _q,                                       \
                        float     _fc,                                      \
                        float     _bw,                                      \
                        float     _gain,                                    \
                        float     _duration,                                \
                        int       _negate,                                  \
                        int       _repeat);                                 \
                                                                            \
/* Add noise source to signal generator, returning id of signal         */  \
/*  _q          : multi-signal source object                            */  \
/*  _fc         : ...                                                   */  \
/*  _bw         : ...                                                   */  \
/*  _nstd       : ...                                                   */  \
int MSOURCE(_add_noise)(MSOURCE() _q,                                       \
                        float     _fc,                                      \
                        float     _bw,                                      \
                        float     _gain);                                   \
                                                                            \
/* Add modem signal source, returning id of signal                      */  \
/*  _q      : multi-signal source object                                */  \
/*  _ms     : modulation scheme, e.g. LIQUID_MODEM_QPSK                 */  \
/*  _m      : filter delay (symbols), _m > 0                            */  \
/*  _beta   : filter excess bandwidth, 0 < _beta <= 1                   */  \
int MSOURCE(_add_modem)(MSOURCE()    _q,                                    \
                        float        _fc,                                   \
                        float        _bw,                                   \
                        float        _gain,                                 \
                        int          _ms,                                   \
                        unsigned int _m,                                    \
                        float        _beta);                                \
                                                                            \
/* Add frequency-shift keying modem signal source, returning id of      */  \
/* signal                                                               */  \
/*  _q      : multi-signal source object                                */  \
/*  _m      : bits per symbol, _bps > 0                                 */  \
/*  _k      : samples/symbol, _k >= 2^_m                                */  \
int MSOURCE(_add_fsk)(MSOURCE()    _q,                                      \
                      float        _fc,                                     \
                      float        _bw,                                     \
                      float        _gain,                                   \
                      unsigned int _m,                                      \
                      unsigned int _k);                                     \
                                                                            \
/* Add GMSK modem signal source, returning id of signal                 */  \
/*  _q      : multi-signal source object                                */  \
/*  _m      : filter delay (symbols), _m > 0                            */  \
/*  _bt     : filter bandwidth-time factor, 0 < _bt <= 1                */  \
int MSOURCE(_add_gmsk)(MSOURCE()    _q,                                     \
                       float        _fc,                                    \
                       float        _bw,                                    \
                       float        _gain,                                  \
                       unsigned int _m,                                     \
                       float        _bt);                                   \
                                                                            \
/* Remove signal with a particular id, returning 0 upon success         */  \
/*  _q  : multi-signal source object                                    */  \
/*  _id : signal source id                                              */  \
int MSOURCE(_remove)(MSOURCE() _q,                                          \
                     int       _id);                                        \
                                                                            \
/* Enable signal source with a particular id                            */  \
int MSOURCE(_enable)(MSOURCE() _q,                                          \
                     int       _id);                                        \
                                                                            \
/* Disable signal source with a particular id                           */  \
int MSOURCE(_disable)(MSOURCE() _q,                                         \
                      int       _id);                                       \
                                                                            \
/* Get number of samples generated by the object so far                 */  \
/*  _q      : msource object                                            */  \
/*  _return : number of time-domain samples generated                   */  \
unsigned long long int MSOURCE(_get_num_samples)(MSOURCE() _q);             \
                                                                            \
/* Get number of samples generated by specific source so far            */  \
/*  _q              : msource object                                    */  \
/*  _id             : source id                                         */  \
/*  _num_samples    : pointer to number of samples generated            */  \
int MSOURCE(_get_num_samples_source)(MSOURCE()           _q,                \
                                     int                 _id,               \
                                     unsigned long int * _num_samples);     \
                                                                            \
/* Set gain in decibels on signal                                       */  \
/*  _q      : msource object                                            */  \
/*  _id     : source id                                                 */  \
/*  _gain   : signal gain [dB]                                          */  \
int MSOURCE(_set_gain)(MSOURCE() _q,                                        \
                       int       _id,                                       \
                       float     _gain);                                    \
                                                                            \
/* Get gain in decibels on signal                                       */  \
/*  _q      : msource object                                            */  \
/*  _id     : source id                                                 */  \
/*  _gain   : signal gain output [dB]                                   */  \
int MSOURCE(_get_gain)(MSOURCE() _q,                                        \
                       int       _id,                                       \
                       float *   _gain);                                    \
                                                                            \
/* Set carrier offset to signal                                         */  \
/*  _q      : msource object                                            */  \
/*  _id     : source id                                                 */  \
/*  _fc     : normalized carrier frequency offset, -0.5 <= _fc <= 0.5   */  \
int MSOURCE(_set_frequency)(MSOURCE() _q,                                   \
                            int       _id,                                  \
                            float     _dphi);                               \
                                                                            \
/* Get carrier offset to signal                                         */  \
/*  _q      : msource object                                            */  \
/*  _id     : source id                                                 */  \
/*  _fc     : normalized carrier frequency offset                       */  \
int MSOURCE(_get_frequency)(MSOURCE() _q,                                   \
                            int       _id,                                  \
                            float *   _dphi);                               \
                                                                            \
/* Write block of samples to output buffer                              */  \
/*  _q      : synchronizer object                                       */  \
/*  _buf    : output buffer, [size: _buf_len x 1]                       */  \
/*  _buf_len: output buffer size                                        */  \
int MSOURCE(_write_samples)(MSOURCE()    _q,                                \
                            TO *         _buf,                              \
                            unsigned int _buf_len);                         \

LIQUID_MSOURCE_DEFINE_API(LIQUID_MSOURCE_MANGLE_CFLOAT, liquid_float_complex)




//
// Symbol tracking: AGC > symsync > EQ > carrier recovery
//
#define LIQUID_SYMTRACK_MANGLE_RRRF(name) LIQUID_CONCAT(symtrack_rrrf,name)
#define LIQUID_SYMTRACK_MANGLE_CCCF(name) LIQUID_CONCAT(symtrack_cccf,name)

// large macro
//   SYMTRACK   : name-mangling macro
//   T          : data type, primitive
//   TO         : data type, output
//   TC         : data type, coefficients
//   TI         : data type, input
#define LIQUID_SYMTRACK_DEFINE_API(SYMTRACK,T,TO,TC,TI)                     \
                                                                            \
/* Symbol synchronizer and tracking object                              */  \
typedef struct SYMTRACK(_s) * SYMTRACK();                                   \
                                                                            \
/* Create symtrack object, specifying parameters for operation          */  \
/*  _ftype  : filter type (e.g. LIQUID_FIRFILT_RRC)                     */  \
/*  _k      : samples per symbol, _k >= 2                               */  \
/*  _m      : filter delay [symbols], _m > 0                            */  \
/*  _beta   : excess bandwidth factor, 0 <= _beta <= 1                  */  \
/*  _ms     : modulation scheme, _ms(LIQUID_MODEM_BPSK)                 */  \
SYMTRACK() SYMTRACK(_create)(int          _ftype,                           \
                             unsigned int _k,                               \
                             unsigned int _m,                               \
                             float        _beta,                            \
                             int          _ms);                             \
                                                                            \
/* Create symtrack object using default parameters.                     */  \
/* The default parameters are                                           */  \
/* ftype  = LIQUID_FIRFILT_ARKAISER (filter type),                      */  \
/* k      = 2 (samples per symbol),                                     */  \
/* m      = 7 (filter delay),                                           */  \
/* beta   = 0.3 (excess bandwidth factor), and                          */  \
/* ms     = LIQUID_MODEM_QPSK (modulation scheme)                       */  \
SYMTRACK() SYMTRACK(_create_default)();                                     \
                                                                            \
/* Destroy symtrack object, freeing all internal memory                 */  \
int SYMTRACK(_destroy)(SYMTRACK() _q);                                      \
                                                                            \
/* Print symtrack object's parameters                                   */  \
int SYMTRACK(_print)(SYMTRACK() _q);                                        \
                                                                            \
/* Reset symtrack internal state                                        */  \
int SYMTRACK(_reset)(SYMTRACK() _q);                                        \
                                                                            \
/* Get symtrack filter type                                             */  \
int SYMTRACK(_get_ftype)(SYMTRACK() _q);                                    \
                                                                            \
/* Get symtrack samples per symbol                                      */  \
unsigned int SYMTRACK(_get_k)(SYMTRACK() _q);                               \
                                                                            \
/* Get symtrack filter semi-length [symbols]                            */  \
unsigned int SYMTRACK(_get_m)(SYMTRACK() _q);                               \
                                                                            \
/* Get symtrack filter excess bandwidth factor                          */  \
float SYMTRACK(_get_beta)(SYMTRACK() _q);                                   \
                                                                            \
/* Get symtrack modulation scheme                                       */  \
int SYMTRACK(_get_modscheme)(SYMTRACK() _q);                                \
                                                                            \
/* Set symtrack modulation scheme                                       */  \
/*  _q      : symtrack object                                           */  \
/*  _ms     : modulation scheme, _ms(LIQUID_MODEM_BPSK)                 */  \
int SYMTRACK(_set_modscheme)(SYMTRACK() _q,                                 \
                             int        _ms);                               \
                                                                            \
/* Get symtrack internal bandwidth                                      */  \
float SYMTRACK(_get_bandwidth)(SYMTRACK() _q);                              \
                                                                            \
/* Set symtrack internal bandwidth                                      */  \
/*  _q      : symtrack object                                           */  \
/*  _bw     : tracking bandwidth, _bw > 0                               */  \
int SYMTRACK(_set_bandwidth)(SYMTRACK() _q,                                 \
                             float _bw);                                    \
                                                                            \
/* Adjust internal NCO by requested frequency                           */  \
/*  _q      : symtrack object                                           */  \
/*  _dphi   : NCO frequency adjustment [radians/sample]                 */  \
int SYMTRACK(_adjust_frequency)(SYMTRACK() _q,                              \
                                T          _dphi);                          \
                                                                            \
/* Adjust internal NCO by requested phase                               */  \
/*  _q      : symtrack object                                           */  \
/*  _phi    : NCO phase adjustment [radians]                            */  \
int SYMTRACK(_adjust_phase)(SYMTRACK() _q,                                  \
                            T          _phi);                               \
                                                                            \
/* Set symtrack equalization strategy to constant modulus (default)     */  \
int SYMTRACK(_set_eq_cm)(SYMTRACK() _q);                                    \
                                                                            \
/* Set symtrack equalization strategy to decision directed              */  \
int SYMTRACK(_set_eq_dd)(SYMTRACK() _q);                                    \
                                                                            \
/* Disable symtrack equalization                                        */  \
int SYMTRACK(_set_eq_off)(SYMTRACK() _q);                                   \
                                                                            \
/* Execute synchronizer on single input sample                          */  \
/*  _q      : synchronizer object                                       */  \
/*  _x      : input data sample                                         */  \
/*  _y      : output data array, [size: 2 x 1]                          */  \
/*  _ny     : number of samples written to output buffer (0, 1, or 2)   */  \
int SYMTRACK(_execute)(SYMTRACK()     _q,                                   \
                       TI             _x,                                   \
                       TO *           _y,                                   \
                       unsigned int * _ny);                                 \
                                                                            \
/* execute synchronizer on input data array                             */  \
/*  _q      : synchronizer object                                       */  \
/*  _x      : input data array                                          */  \
/*  _nx     : number of input samples                                   */  \
/*  _y      : output data array, [size: 2 _nx x 1]                      */  \
/*  _ny     : number of samples written to output buffer                */  \
int SYMTRACK(_execute_block)(SYMTRACK()     _q,                             \
                             TI *           _x,                             \
                             unsigned int   _nx,                            \
                             TO *           _y,                             \
                             unsigned int * _ny);                           \

LIQUID_SYMTRACK_DEFINE_API(LIQUID_SYMTRACK_MANGLE_RRRF,
                           float,
                           float,
                           float,
                           float)

LIQUID_SYMTRACK_DEFINE_API(LIQUID_SYMTRACK_MANGLE_CCCF,
                           float,
                           liquid_float_complex,
                           liquid_float_complex,
                           liquid_float_complex)



//
// MODULE : math
//

// ln( Gamma(z) )
float liquid_lngammaf(float _z);

// Gamma(z)
float liquid_gammaf(float _z);

// ln( gamma(z,alpha) ) : lower incomplete gamma function
float liquid_lnlowergammaf(float _z, float _alpha);

// ln( Gamma(z,alpha) ) : upper incomplete gamma function
float liquid_lnuppergammaf(float _z, float _alpha);

// gamma(z,alpha) : lower incomplete gamma function
float liquid_lowergammaf(float _z, float _alpha);

// Gamma(z,alpha) : upper incomplete gamma function
float liquid_uppergammaf(float _z, float _alpha);

// n!
float liquid_factorialf(unsigned int _n);



// ln(I_v(z)) : log Modified Bessel function of the first kind
float liquid_lnbesselif(float _nu, float _z);

// I_v(z) : Modified Bessel function of the first kind
float liquid_besselif(float _nu, float _z);

// I_0(z) : Modified Bessel function of the first kind (order zero)
float liquid_besseli0f(float _z);

// J_v(z) : Bessel function of the first kind
float liquid_besseljf(float _nu, float _z);

// J_0(z) : Bessel function of the first kind (order zero)
float liquid_besselj0f(float _z);


// Q function
float liquid_Qf(float _z);

// Marcum Q-function
float liquid_MarcumQf(int _M,
                      float _alpha,
                      float _beta);

// Marcum Q-function (M=1)
float liquid_MarcumQ1f(float _alpha,
                       float _beta);

// sin(pi x) / (pi x)
float sincf(float _x);

// next power of 2 : y = ceil(log2(_x))
unsigned int liquid_nextpow2(unsigned int _x);

// (n choose k) = n! / ( k! (n-k)! )
float liquid_nchoosek(unsigned int _n, unsigned int _k);

//
// Windowing functions
//

// number of window functions available, including "unknown" type
#define LIQUID_WINDOW_NUM_FUNCTIONS (10)

// prototypes
typedef enum {
    LIQUID_WINDOW_UNKNOWN=0,        // unknown/unsupported scheme

    LIQUID_WINDOW_HAMMING,          // Hamming
    LIQUID_WINDOW_HANN,             // Hann
    LIQUID_WINDOW_BLACKMANHARRIS,   // Blackman-harris (4-term)
    LIQUID_WINDOW_BLACKMANHARRIS7,  // Blackman-harris (7-term)
    LIQUID_WINDOW_KAISER,           // Kaiser (beta factor unspecified)
    LIQUID_WINDOW_FLATTOP,          // flat top (includes negative values)
    LIQUID_WINDOW_TRIANGULAR,       // triangular
    LIQUID_WINDOW_RCOSTAPER,        // raised-cosine taper (taper size unspecified)
    LIQUID_WINDOW_KBD,              // Kaiser-Bessel derived window (beta factor unspecified)
} liquid_window_type;

// pretty names for window
extern const char * liquid_window_str[LIQUID_WINDOW_NUM_FUNCTIONS][2];

// Print compact list of existing and available windowing functions
int liquid_print_windows();

// returns window type based on input string
liquid_window_type liquid_getopt_str2window(const char * _str);

// generic window function given type
//  _type   :   window type, e.g. LIQUID_WINDOW_KAISER
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   length of window
//  _arg    :   window-specific argument, if required
float liquid_windowf(liquid_window_type _type,
                     unsigned int       _i,
                     unsigned int       _wlen,
                     float              _arg);

// Kaiser window
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   full window length
//  _beta   :   Kaiser-Bessel window shape parameter
float liquid_kaiser(unsigned int _i,
                    unsigned int _wlen,
                    float        _beta);

// Hamming window
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   full window length
float liquid_hamming(unsigned int _i,
                     unsigned int _wlen);

// Hann window
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   full window length
float liquid_hann(unsigned int _i,
                  unsigned int _wlen);

// Blackman-harris window
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   full window length
float liquid_blackmanharris(unsigned int _i,
                            unsigned int _wlen);

// 7th order Blackman-harris window
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   full window length
float liquid_blackmanharris7(unsigned int _i,
                             unsigned int _wlen);

// Flat-top window
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   full window length
float liquid_flattop(unsigned int _i,
                     unsigned int _wlen);

// Triangular window
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   full window length
//  _n      :   triangle length, _n in {_wlen-1, _wlen, _wlen+1}
float liquid_triangular(unsigned int _i,
                        unsigned int _wlen,
                        unsigned int _n);

// raised-cosine tapering window
//  _i      :   window index
//  _wlen   :   full window length
//  _t      :   taper length, _t in [0,_wlen/2]
float liquid_rcostaper_window(unsigned int _i,
                              unsigned int _wlen,
                              unsigned int _t);

// Kaiser-Bessel derived window (single sample)
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   length of filter (must be even)
//  _beta   :   Kaiser window parameter (_beta > 0)
float liquid_kbd(unsigned int _i,
                 unsigned int _wlen,
                 float        _beta);

// Kaiser-Bessel derived window (full window)
//  _wlen   :   full window length (must be even)
//  _beta   :   Kaiser window parameter (_beta > 0)
//  _w      :   window output buffer, [size: _wlen x 1]
int liquid_kbd_window(unsigned int _wlen,
                      float        _beta,
                      float *      _w);

// shim to support legacy APIs (backwards compatible with 1.3.2)
float kaiser(unsigned int _i,unsigned int _wlen, float _beta, float _dt);
float hamming(unsigned int _i,unsigned int _wlen);
float hann(unsigned int _i,unsigned int _wlen);
float blackmanharris(unsigned int _i,unsigned int _wlen);
float blackmanharris7(unsigned int _i,unsigned int _wlen);
float flattop(unsigned int _i,unsigned int _wlen);
float triangular(unsigned int _i,unsigned int _wlen,unsigned int _n);
float liquid_rcostaper_windowf(unsigned int _i,unsigned int _wlen,unsigned int _t);
float kbd(unsigned int _i,unsigned int _wlen,float _beta);
int   kbd_window(unsigned int _wlen,float _beta,float * _w);


// polynomials


#define LIQUID_POLY_MANGLE_DOUBLE(name)  LIQUID_CONCAT(poly,   name)
#define LIQUID_POLY_MANGLE_FLOAT(name)   LIQUID_CONCAT(polyf,  name)

#define LIQUID_POLY_MANGLE_CDOUBLE(name) LIQUID_CONCAT(polyc,  name)
#define LIQUID_POLY_MANGLE_CFLOAT(name)  LIQUID_CONCAT(polycf, name)

// large macro
//   POLY   : name-mangling macro
//   T      : data type
//   TC     : data type (complex)
#define LIQUID_POLY_DEFINE_API(POLY,T,TC)                                   \
                                                                            \
/* Evaluate polynomial _p at value _x                                   */  \
/*  _p      : polynomial coefficients, [size: _k x 1]                   */  \
/*  _k      : polynomial coefficients length, order is _k - 1           */  \
/*  _x      : input to evaluate polynomial                              */  \
T POLY(_val)(T *          _p,                                               \
             unsigned int _k,                                               \
             T            _x);                                              \
                                                                            \
/* Perform least-squares polynomial fit on data set                     */  \
/*  _x      : x-value sample set, [size: _n x 1]                        */  \
/*  _y      : y-value sample set, [size: _n x 1]                        */  \
/*  _n      : number of samples in _x and _y                            */  \
/*  _p      : polynomial coefficients output, [size: _k x 1]            */  \
/*  _k      : polynomial coefficients length, order is _k - 1           */  \
int POLY(_fit)(T *          _x,                                             \
               T *          _y,                                             \
               unsigned int _n,                                             \
               T *          _p,                                             \
               unsigned int _k);                                            \
                                                                            \
/* Perform Lagrange polynomial exact fit on data set                    */  \
/*  _x      : x-value sample set, size [_n x 1]                         */  \
/*  _y      : y-value sample set, size [_n x 1]                         */  \
/*  _n      : number of samples in _x and _y                            */  \
/*  _p      : polynomial coefficients output, [size: _n x 1]            */  \
int POLY(_fit_lagrange)(T *          _x,                                    \
                        T *          _y,                                    \
                        unsigned int _n,                                    \
                        T *          _p);                                   \
                                                                            \
/* Perform Lagrange polynomial interpolation on data set without        */  \
/* computing coefficients as an intermediate step.                      */  \
/*  _x      : x-value sample set, [size: _n x 1]                        */  \
/*  _y      : y-value sample set, [size: _n x 1]                        */  \
/*  _n      : number of samples in _x and _y                            */  \
/*  _x0     : x-value to evaluate and compute interpolant               */  \
T POLY(_interp_lagrange)(T *          _x,                                   \
                         T *          _y,                                   \
                         unsigned int _n,                                   \
                         T           _x0);                                  \
                                                                            \
/* Compute Lagrange polynomial fit in the barycentric form.             */  \
/*  _x      : x-value sample set, size [_n x 1]                         */  \
/*  _n      : number of samples in _x                                   */  \
/*  _w      : barycentric weights normalized so _w[0]=1, size [_n x 1]  */  \
int POLY(_fit_lagrange_barycentric)(T *          _x,                        \
                                    unsigned int _n,                        \
                                    T *          _w);                       \
                                                                            \
/* Perform Lagrange polynomial interpolation using the barycentric form */  \
/* of the weights.                                                      */  \
/*  _x      : x-value sample set, [size: _n x 1]                        */  \
/*  _y      : y-value sample set, [size: _n x 1]                        */  \
/*  _w      : barycentric weights, [size: _n x 1]                       */  \
/*  _x0     : x-value to evaluate and compute interpolant               */  \
/*  _n      : number of samples in _x, _y, and _w                       */  \
T POLY(_val_lagrange_barycentric)(T *          _x,                          \
                                  T *          _y,                          \
                                  T *          _w,                          \
                                  T            _x0,                         \
                                  unsigned int _n);                         \
                                                                            \
/* Perform binomial expansion on the polynomial                         */  \
/*  \( P_n(x) = (1+x)^n \)                                              */  \
/* as                                                                   */  \
/*  \( P_n(x) = p[0] + p[1]x + p[2]x^2 + ... + p[n]x^n \)               */  \
/* NOTE: _p has order n (coefficients has length n+1)                   */  \
/*  _n      : polynomial order                                          */  \
/*  _p      : polynomial coefficients, [size: _n+1 x 1]                 */  \
int POLY(_expandbinomial)(unsigned int _n,                                  \
                          T *          _p);                                 \
                                                                            \
/* Perform positive/negative binomial expansion on the polynomial       */  \
/*  \( P_n(x) = (1+x)^m (1-x)^k \)                                      */  \
/* as                                                                   */  \
/*  \( P_n(x) = p[0] + p[1]x + p[2]x^2 + ... + p[n]x^n \)               */  \
/* NOTE: _p has order n=m+k (array is length n+1)                       */  \
/*  _m      : number of '1+x' terms                                     */  \
/*  _k      : number of '1-x' terms                                     */  \
/*  _p      : polynomial coefficients, [size: _m+_k+1 x 1]              */  \
int POLY(_expandbinomial_pm)(unsigned int _m,                               \
                             unsigned int _k,                               \
                             T *          _p);                              \
                                                                            \
/* Perform root expansion on the polynomial                             */  \
/*  \( P_n(x) = (x-r[0]) (x-r[1]) ... (x-r[n-1]) \)                     */  \
/* as                                                                   */  \
/*  \( P_n(x) = p[0] + p[1]x + ... + p[n]x^n \)                         */  \
/* where \( r[0],r[1],...,r[n-1]\) are the roots of \( P_n(x) \).       */  \
/* NOTE: _p has order _n (array is length _n+1)                         */  \
/*  _r      : roots of polynomial, [size: _n x 1]                       */  \
/*  _n      : number of roots in polynomial                             */  \
/*  _p      : polynomial coefficients, [size: _n+1 x 1]                 */  \
int POLY(_expandroots)(T *          _r,                                     \
                       unsigned int _n,                                     \
                       T *          _p);                                    \
                                                                            \
/* Perform root expansion on the polynomial                             */  \
/*  \( P_n(x) = (xb[0]-a[0]) (xb[1]-a[1])...(xb[n-1]-a[n-1]) \)         */  \
/* as                                                                   */  \
/*  \( P_n(x) = p[0] + p[1]x + ... + p[n]x^n \)                         */  \
/* NOTE: _p has order _n (array is length _n+1)                         */  \
/*  _a      : subtractant of polynomial rotos, [size: _n x 1]           */  \
/*  _b      : multiplicant of polynomial roots, [size: _n x 1]          */  \
/*  _n      : number of roots in polynomial                             */  \
/*  _p      : polynomial coefficients, [size: _n+1 x 1]                 */  \
int POLY(_expandroots2)(T *          _a,                                    \
                        T *          _b,                                    \
                        unsigned int _n,                                    \
                        T *          _p);                                   \
                                                                            \
/* Find the complex roots of a polynomial.                              */  \
/*  _p      : polynomial coefficients, [size: _n x 1]                   */  \
/*  _k      : polynomial length                                         */  \
/*  _roots  : resulting complex roots, [size: _k-1 x 1]                 */  \
int POLY(_findroots)(T *          _poly,                                    \
                     unsigned int _n,                                       \
                     TC *         _roots);                                  \
                                                                            \
/* Find the complex roots of the polynomial using the Durand-Kerner     */  \
/* method                                                               */  \
/*  _p      : polynomial coefficients, [size: _n x 1]                   */  \
/*  _k      : polynomial length                                         */  \
/*  _roots  : resulting complex roots, [size: _k-1 x 1]                 */  \
int POLY(_findroots_durandkerner)(T *          _p,                          \
                                  unsigned int _k,                          \
                                  TC *         _roots);                     \
                                                                            \
/* Find the complex roots of the polynomial using Bairstow's method.    */  \
/*  _p      : polynomial coefficients, [size: _n x 1]                   */  \
/*  _k      : polynomial length                                         */  \
/*  _roots  : resulting complex roots, [size: _k-1 x 1]                 */  \
int POLY(_findroots_bairstow)(T *          _p,                              \
                              unsigned int _k,                              \
                              TC *         _roots);                         \
                                                                            \
/* Expand the multiplication of two polynomials                         */  \
/*  \( ( a[0] + a[1]x + a[2]x^2 + ...) (b[0] + b[1]x + b[]x^2 + ...) \) */  \
/* as                                                                   */  \
/*  \( c[0] + c[1]x + c[2]x^2 + ... + c[n]x^n \)                        */  \
/* where order(c)  = order(a)  + order(b) + 1                           */  \
/* and  therefore length(c) = length(a) + length(b) - 1                 */  \
/*  _a          : 1st polynomial coefficients (length is _order_a+1)    */  \
/*  _order_a    : 1st polynomial order                                  */  \
/*  _b          : 2nd polynomial coefficients (length is _order_b+1)    */  \
/*  _order_b    : 2nd polynomial order                                  */  \
/*  _c          : output polynomial, [size: _order_a+_order_b+1 x 1]    */  \
int POLY(_mul)(T *          _a,                                             \
               unsigned int _order_a,                                       \
               T *          _b,                                             \
               unsigned int _order_b,                                       \
               T *          _c);                                            \

LIQUID_POLY_DEFINE_API(LIQUID_POLY_MANGLE_DOUBLE,
                       double,
                       liquid_double_complex)

LIQUID_POLY_DEFINE_API(LIQUID_POLY_MANGLE_FLOAT,
                       float,
                       liquid_float_complex)

LIQUID_POLY_DEFINE_API(LIQUID_POLY_MANGLE_CDOUBLE,
                       liquid_double_complex,
                       liquid_double_complex)

LIQUID_POLY_DEFINE_API(LIQUID_POLY_MANGLE_CFLOAT,
                       liquid_float_complex,
                       liquid_float_complex)

#if 0
// expands the polynomial: (1+x)^n
void poly_binomial_expand(unsigned int _n, int * _c);

// expands the polynomial: (1+x)^k * (1-x)^(n-k)
void poly_binomial_expand_pm(unsigned int _n,
                             unsigned int _k,
                             int * _c);
#endif

//
// modular arithmetic, etc.
//

// maximum number of factors
#define LIQUID_MAX_FACTORS (40)

// is number prime?
int liquid_is_prime(unsigned int _n);

// compute number's prime factors
//  _n          :   number to factor
//  _factors    :   pre-allocated array of factors, [size: LIQUID_MAX_FACTORS x 1]
//  _num_factors:   number of factors found, sorted ascending
int liquid_factor(unsigned int   _n,
                  unsigned int * _factors,
                  unsigned int * _num_factors);

// compute number's unique prime factors
//  _n          :   number to factor
//  _factors    :   pre-allocated array of factors, [size: LIQUID_MAX_FACTORS x 1]
//  _num_factors:   number of unique factors found, sorted ascending
int liquid_unique_factor(unsigned int   _n,
                         unsigned int * _factors,
                         unsigned int * _num_factors);

// compute greatest common divisor between to integers \(p\) and \(q\)
unsigned int liquid_gcd(unsigned int _p,
                        unsigned int _q);

// compute c = base^exp (mod n)
unsigned int liquid_modpow(unsigned int _base,
                           unsigned int _exp,
                           unsigned int _n);

// find smallest primitive root of _n
unsigned int liquid_primitive_root(unsigned int _n);

// find smallest primitive root of _n, assuming _n is prime
unsigned int liquid_primitive_root_prime(unsigned int _n);

// Euler's totient function
unsigned int liquid_totient(unsigned int _n);


//
// MODULE : matrix
//

#define LIQUID_MATRIX_MANGLE_DOUBLE(name)  LIQUID_CONCAT(matrix,   name)
#define LIQUID_MATRIX_MANGLE_FLOAT(name)   LIQUID_CONCAT(matrixf,  name)

#define LIQUID_MATRIX_MANGLE_CDOUBLE(name) LIQUID_CONCAT(matrixc,  name)
#define LIQUID_MATRIX_MANGLE_CFLOAT(name)  LIQUID_CONCAT(matrixcf, name)

// large macro
//   MATRIX : name-mangling macro
//   T      : data type
#define LIQUID_MATRIX_DEFINE_API(MATRIX,T)                                  \
                                                                            \
/* Print array as matrix to stdout                                      */  \
/*  _x      : input matrix, [size: _r x _c]                             */  \
/*  _r      : rows in matrix                                            */  \
/*  _c      : columns in matrix                                         */  \
int MATRIX(_print)(T *          _x,                                         \
                   unsigned int _r,                                         \
                   unsigned int _c);                                        \
                                                                            \
/* Perform point-wise addition between two matrices \(\vec{X}\)         */  \
/* and \(\vec{Y}\), saving the result in the output matrix \(\vec{Z}\). */  \
/* That is, \(\vec{Z}_{i,j}=\vec{X}_{i,j}+\vec{Y}_{i,j} \),             */  \
/* \( \forall_{i \in r} \) and \( \forall_{j \in c} \)                  */  \
/*  _x      : input matrix,  [size: _r x _c]                            */  \
/*  _y      : input matrix,  [size: _r x _c]                            */  \
/*  _z      : output matrix, [size: _r x _c]                            */  \
/*  _r      : number of rows in each matrix                             */  \
/*  _c      : number of columns in each matrix                          */  \
int MATRIX(_add)(T *          _x,                                           \
                 T *          _y,                                           \
                 T *          _z,                                           \
                 unsigned int _r,                                           \
                 unsigned int _c);                                          \
                                                                            \
/* Perform point-wise subtraction between two matrices \(\vec{X}\)      */  \
/* and \(\vec{Y}\), saving the result in the output matrix \(\vec{Z}\)  */  \
/* That is, \(\vec{Z}_{i,j}=\vec{X}_{i,j}-\vec{Y}_{i,j} \),             */  \
/* \( \forall_{i \in r} \) and \( \forall_{j \in c} \)                  */  \
/*  _x      : input matrix,  [size: _r x _c]                            */  \
/*  _y      : input matrix,  [size: _r x _c]                            */  \
/*  _z      : output matrix, [size: _r x _c]                            */  \
/*  _r      : number of rows in each matrix                             */  \
/*  _c      : number of columns in each matrix                          */  \
int MATRIX(_sub)(T *          _x,                                           \
                 T *          _y,                                           \
                 T *          _z,                                           \
                 unsigned int _r,                                           \
                 unsigned int _c);                                          \
                                                                            \
/* Perform point-wise multiplication between two matrices \(\vec{X}\)   */  \
/* and \(\vec{Y}\), saving the result in the output matrix \(\vec{Z}\)  */  \
/* That is, \(\vec{Z}_{i,j}=\vec{X}_{i,j} \vec{Y}_{i,j} \),             */  \
/* \( \forall_{i \in r} \) and \( \forall_{j \in c} \)                  */  \
/*  _x      : input matrix,  [size: _r x _c]                            */  \
/*  _y      : input matrix,  [size: _r x _c]                            */  \
/*  _z      : output matrix, [size: _r x _c]                            */  \
/*  _r      : number of rows in each matrix                             */  \
/*  _c      : number of columns in each matrix                          */  \
int MATRIX(_pmul)(T *          _x,                                          \
                  T *          _y,                                          \
                  T *          _z,                                          \
                  unsigned int _r,                                          \
                  unsigned int _c);                                         \
                                                                            \
/* Perform point-wise division between two matrices \(\vec{X}\)         */  \
/* and \(\vec{Y}\), saving the result in the output matrix \(\vec{Z}\)  */  \
/* That is, \(\vec{Z}_{i,j}=\vec{X}_{i,j}/\vec{Y}_{i,j} \),             */  \
/* \( \forall_{i \in r} \) and \( \forall_{j \in c} \)                  */  \
/*  _x      : input matrix,  [size: _r x _c]                            */  \
/*  _y      : input matrix,  [size: _r x _c]                            */  \
/*  _z      : output matrix, [size: _r x _c]                            */  \
/*  _r      : number of rows in each matrix                             */  \
/*  _c      : number of columns in each matrix                          */  \
int MATRIX(_pdiv)(T *          _x,                                          \
                  T *          _y,                                          \
                  T *          _z,                                          \
                  unsigned int _r,                                          \
                  unsigned int _c);                                         \
                                                                            \
/* Multiply two matrices \(\vec{X}\) and \(\vec{Y}\), storing the       */  \
/* result in \(\vec{Z}\).                                               */  \
/* NOTE: _rz = _rx, _cz = _cy, and _cx = _ry                            */  \
/*  _x      : input matrix,  [size: _rx x _cx]                          */  \
/*  _rx     : number of rows in _x                                      */  \
/*  _cx     : number of columns in _x                                   */  \
/*  _y      : input matrix,  [size: _ry x _cy]                          */  \
/*  _ry     : number of rows in _y                                      */  \
/*  _cy     : number of columns in _y                                   */  \
/*  _z      : output matrix, [size: _rz x _cz]                          */  \
/*  _rz     : number of rows in _z                                      */  \
/*  _cz     : number of columns in _z                                   */  \
int MATRIX(_mul)(T * _x, unsigned int _rx, unsigned int _cx,                \
                 T * _y, unsigned int _ry, unsigned int _cy,                \
                 T * _z, unsigned int _rz, unsigned int _cz);               \
                                                                            \
/* Solve \(\vec{X} = \vec{Y} \vec{Z}\) for \(\vec{Z}\) for square       */  \
/* matrices of size \(n\)                                               */  \
/*  _x      : input matrix,  [size: _n x _n]                            */  \
/*  _y      : input matrix,  [size: _n x _n]                            */  \
/*  _z      : output matrix, [size: _n x _n]                            */  \
/*  _n      : number of rows and columns in each matrix                 */  \
int MATRIX(_div)(T *          _x,                                           \
                 T *          _y,                                           \
                 T *          _z,                                           \
                 unsigned int _n);                                          \
                                                                            \
/* Compute the determinant of a square matrix \(\vec{X}\)               */  \
/*  _x      : input matrix, [size: _r x _c]                             */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
T MATRIX(_det)(T *          _x,                                             \
               unsigned int _r,                                             \
               unsigned int _c);                                            \
                                                                            \
/* Compute the in-place transpose of the matrix \(\vec{X}\)             */  \
/*  _x      : input matrix, [size: _r x _c]                             */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
int MATRIX(_trans)(T *          _x,                                         \
                   unsigned int _r,                                         \
                   unsigned int _c);                                        \
                                                                            \
/* Compute the in-place Hermitian transpose of the matrix \(\vec{X}\)   */  \
/*  _x      : input matrix, [size: _r x _c]                             */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
int MATRIX(_hermitian)(T *          _x,                                     \
                       unsigned int _r,                                     \
                       unsigned int _c);                                    \
                                                                            \
/* Compute \(\vec{X}\vec{X}^T\) on a \(m \times n\) matrix.             */  \
/* The result is a \(m \times m\) matrix.                               */  \
/*  _x      : input matrix, [size: _m x _n]                             */  \
/*  _m      : input rows                                                */  \
/*  _n      : input columns                                             */  \
/*  _xxT    : output matrix, [size: _m x _m]                            */  \
int MATRIX(_mul_transpose)(T *          _x,                                 \
                           unsigned int _m,                                 \
                           unsigned int _n,                                 \
                           T *          _xxT);                              \
                                                                            \
/* Compute \(\vec{X}^T\vec{X}\) on a \(m \times n\) matrix.             */  \
/* The result is a \(n \times n\) matrix.                               */  \
/*  _x      : input matrix, [size: _m x _n]                             */  \
/*  _m      : input rows                                                */  \
/*  _n      : input columns                                             */  \
/*  _xTx    : output matrix, [size: _n x _n]                            */  \
int MATRIX(_transpose_mul)(T *          _x,                                 \
                           unsigned int _m,                                 \
                           unsigned int _n,                                 \
                           T *          _xTx);                              \
                                                                            \
/* Compute \(\vec{X}\vec{X}^H\) on a \(m \times n\) matrix.             */  \
/* The result is a \(m \times m\) matrix.                               */  \
/*  _x      : input matrix, [size: _m x _n]                             */  \
/*  _m      : input rows                                                */  \
/*  _n      : input columns                                             */  \
/*  _xxH    : output matrix, [size: _m x _m]                            */  \
int MATRIX(_mul_hermitian)(T *          _x,                                 \
                           unsigned int _m,                                 \
                           unsigned int _n,                                 \
                           T *          _xxH);                              \
                                                                            \
/* Compute \(\vec{X}^H\vec{X}\) on a \(m \times n\) matrix.             */  \
/* The result is a \(n \times n\) matrix.                               */  \
/*  _x      : input matrix, [size: _m x _n]                             */  \
/*  _m      : input rows                                                */  \
/*  _n      : input columns                                             */  \
/*  _xHx    : output matrix, [size: _n x _n]                            */  \
int MATRIX(_hermitian_mul)(T *          _x,                                 \
                           unsigned int _m,                                 \
                           unsigned int _n,                                 \
                           T *          _xHx);                              \
                                                                            \
                                                                            \
/* Augment two matrices \(\vec{X}\) and \(\vec{Y}\), storing the result */  \
/* in \(\vec{Z}\)                                                       */  \
/* NOTE: _rz = _rx = _ry, _rx = _ry, and _cz = _cx + _cy                */  \
/*  _x      : input matrix,  [size: _rx x _cx]                          */  \
/*  _rx     : number of rows in _x                                      */  \
/*  _cx     : number of columns in _x                                   */  \
/*  _y      : input matrix,  [size: _ry x _cy]                          */  \
/*  _ry     : number of rows in _y                                      */  \
/*  _cy     : number of columns in _y                                   */  \
/*  _z      : output matrix, [size: _rz x _cz]                          */  \
/*  _rz     : number of rows in _z                                      */  \
/*  _cz     : number of columns in _z                                   */  \
int MATRIX(_aug)(T * _x, unsigned int _rx, unsigned int _cx,                \
                 T * _y, unsigned int _ry, unsigned int _cy,                \
                 T * _z, unsigned int _rz, unsigned int _cz);               \
                                                                            \
/* Compute the inverse of a square matrix \(\vec{X}\)                   */  \
/*  _x      : input/output matrix, [size: _r x _c]                      */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
int MATRIX(_inv)(T *          _x,                                           \
                 unsigned int _r,                                           \
                 unsigned int _c);                                          \
                                                                            \
/* Generate the identity square matrix of size \(n\)                    */  \
/*  _x      : output matrix, [size: _n x _n]                            */  \
/*  _n      : dimensions of _x                                          */  \
int MATRIX(_eye)(T *          _x,                                           \
                 unsigned int _n);                                          \
                                                                            \
/* Generate the all-ones matrix of size \(n\)                           */  \
/*  _x      : output matrix, [size: _r x _c]                            */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
int MATRIX(_ones)(T *          _x,                                          \
                  unsigned int _r,                                          \
                  unsigned int _c);                                         \
                                                                            \
/* Generate the all-zeros matrix of size \(n\)                          */  \
/*  _x      : output matrix, [size: _r x _c]                            */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
int MATRIX(_zeros)(T *          _x,                                         \
                   unsigned int _r,                                         \
                   unsigned int _c);                                        \
                                                                            \
/* Perform Gauss-Jordan elimination on matrix \(\vec{X}\)               */  \
/*  _x      : input/output matrix, [size: _r x _c]                      */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
int MATRIX(_gjelim)(T *          _x,                                        \
                    unsigned int _r,                                        \
                    unsigned int _c);                                       \
                                                                            \
/* Pivot on element \(\vec{X}_{i,j}\)                                   */  \
/*  _x      : output matrix, [size: _r x _c]                            */  \
/*  _r      : rows of _x                                                */  \
/*  _c      : columns of _x                                             */  \
/*  _i      : pivot row                                                 */  \
/*  _j      : pivot column                                              */  \
int MATRIX(_pivot)(T *          _x,                                         \
                   unsigned int _r,                                         \
                   unsigned int _c,                                         \
                   unsigned int _i,                                         \
                   unsigned int _j);                                        \
                                                                            \
/* Swap rows _r1 and _r2 of matrix \(\vec{X}\)                          */  \
/*  _x      : input/output matrix, [size: _r x _c]                      */  \
/*  _r      : rows of _x                                                */  \
/*  _c      : columns of _x                                             */  \
/*  _r1     : first row to swap                                         */  \
/*  _r2     : second row to swap                                        */  \
int MATRIX(_swaprows)(T *          _x,                                      \
                      unsigned int _r,                                      \
                      unsigned int _c,                                      \
                      unsigned int _r1,                                     \
                      unsigned int _r2);                                    \
                                                                            \
/* Solve linear system of \(n\) equations: \(\vec{A}\vec{x} = \vec{b}\) */  \
/*  _A      : system matrix, [size: _n x _n]                            */  \
/*  _n      : system size                                               */  \
/*  _b      : equality vector, [size: _n x 1]                           */  \
/*  _x      : solution vector, [size: _n x 1]                           */  \
/*  _opts   : options (ignored for now)                                 */  \
int MATRIX(_linsolve)(T *          _A,                                      \
                      unsigned int _n,                                      \
                      T *          _b,                                      \
                      T *          _x,                                      \
                      void *       _opts);                                  \
                                                                            \
/* Solve linear system of equations using conjugate gradient method.    */  \
/*  _A      : symmetric positive definite square matrix                 */  \
/*  _n      : system dimension                                          */  \
/*  _b      : equality, [size: _n x 1]                                  */  \
/*  _x      : solution estimate, [size: _n x 1]                         */  \
/*  _opts   : options (ignored for now)                                 */  \
int MATRIX(_cgsolve)(T *          _A,                                       \
                     unsigned int _n,                                       \
                     T *          _b,                                       \
                     T *          _x,                                       \
                     void *       _opts);                                   \
                                                                            \
/* Perform L/U/P decomposition using Crout's method                     */  \
/*  _x      : input/output matrix, [size: _rx x _cx]                    */  \
/*  _rx     : rows of _x                                                */  \
/*  _cx     : columns of _x                                             */  \
/*  _l      : first row to swap                                         */  \
/*  _u      : first row to swap                                         */  \
/*  _p      : first row to swap                                         */  \
int MATRIX(_ludecomp_crout)(T *          _x,                                \
                            unsigned int _rx,                               \
                            unsigned int _cx,                               \
                            T *          _l,                                \
                            T *          _u,                                \
                            T *          _p);                               \
                                                                            \
/* Perform L/U/P decomposition, Doolittle's method                      */  \
/*  _x      : input/output matrix, [size: _rx x _cx]                    */  \
/*  _rx     : rows of _x                                                */  \
/*  _cx     : columns of _x                                             */  \
/*  _l      : first row to swap                                         */  \
/*  _u      : first row to swap                                         */  \
/*  _p      : first row to swap                                         */  \
int MATRIX(_ludecomp_doolittle)(T *          _x,                            \
                                unsigned int _rx,                           \
                                unsigned int _cx,                           \
                                T *          _l,                            \
                                T *          _u,                            \
                                T *          _p);                           \
                                                                            \
/* Perform orthnormalization using the Gram-Schmidt algorithm           */  \
/*  _A      : input matrix, [size: _r x _c]                             */  \
/*  _r      : rows                                                      */  \
/*  _c      : columns                                                   */  \
/*  _v      : output matrix                                             */  \
int MATRIX(_gramschmidt)(T *          _A,                                   \
                         unsigned int _r,                                   \
                         unsigned int _c,                                   \
                         T *          _v);                                  \
                                                                            \
/* Perform Q/R decomposition using the Gram-Schmidt algorithm such that */  \
/* \( \vec{A} = \vec{Q} \vec{R} \)                                      */  \
/* and \( \vec{Q}^T \vec{Q} = \vec{I}_n \)                              */  \
/* and \(\vec{R\}\) is a diagonal \(m \times m\) matrix                 */  \
/* NOTE: all matrices are square                                        */  \
/*  _a      : input matrix, [size: _m x _m]                             */  \
/*  _m      : rows                                                      */  \
/*  _n      : columns (same as cols)                                    */  \
/*  _q      : output matrix, [size: _m x _m]                            */  \
/*  _r      : output matrix, [size: _m x _m]                            */  \
int MATRIX(_qrdecomp_gramschmidt)(T *          _a,                          \
                                  unsigned int _m,                          \
                                  unsigned int _n,                          \
                                  T *          _q,                          \
                                  T *          _r);                         \
                                                                            \
/* Compute Cholesky decomposition of a symmetric/Hermitian              */  \
/* positive-definite matrix as \( \vec{A} = \vec{L}\vec{L}^T \)         */  \
/*  _a      : input square matrix, [size: _n x _n]                      */  \
/*  _n      : input matrix dimension                                    */  \
/*  _l      : output lower-triangular matrix                            */  \
int MATRIX(_chol)(T *          _a,                                          \
                  unsigned int _n,                                          \
                  T *          _l);                                         \

#define matrix_access(X,R,C,r,c) ((X)[(r)*(C)+(c)])

#define matrixc_access(X,R,C,r,c)   matrix_access(X,R,C,r,c)
#define matrixf_access(X,R,C,r,c)   matrix_access(X,R,C,r,c)
#define matrixcf_access(X,R,C,r,c)  matrix_access(X,R,C,r,c)

LIQUID_MATRIX_DEFINE_API(LIQUID_MATRIX_MANGLE_FLOAT,   float)
LIQUID_MATRIX_DEFINE_API(LIQUID_MATRIX_MANGLE_DOUBLE,  double)

LIQUID_MATRIX_DEFINE_API(LIQUID_MATRIX_MANGLE_CFLOAT,  liquid_float_complex)
LIQUID_MATRIX_DEFINE_API(LIQUID_MATRIX_MANGLE_CDOUBLE, liquid_double_complex)


#define LIQUID_SMATRIX_MANGLE_BOOL(name)  LIQUID_CONCAT(smatrixb,  name)
#define LIQUID_SMATRIX_MANGLE_FLOAT(name) LIQUID_CONCAT(smatrixf,  name)
#define LIQUID_SMATRIX_MANGLE_INT(name)   LIQUID_CONCAT(smatrixi,  name)

// sparse 'alist' matrix type (similar to MacKay, Davey Lafferty convention)
// large macro
//   SMATRIX    : name-mangling macro
//   T          : primitive data type
#define LIQUID_SMATRIX_DEFINE_API(SMATRIX,T)                                \
                                                                            \
/* Sparse matrix object (similar to MacKay, Davey, Lafferty convention) */  \
typedef struct SMATRIX(_s) * SMATRIX();                                     \
                                                                            \
/* Create \(M \times N\) sparse matrix, initialized with zeros          */  \
/*  _m  : number of rows in matrix                                      */  \
/*  _n  : number of columns in matrix                                   */  \
SMATRIX() SMATRIX(_create)(unsigned int _m,                                 \
                           unsigned int _n);                                \
                                                                            \
/* Create \(M \times N\) sparse matrix, initialized on array            */  \
/*  _x  : input matrix, [size: _m x _n]                                 */  \
/*  _m  : number of rows in input matrix                                */  \
/*  _n  : number of columns in input matrix                             */  \
SMATRIX() SMATRIX(_create_array)(T *          _x,                           \
                                 unsigned int _m,                           \
                                 unsigned int _n);                          \
                                                                            \
/* Destroy object, freeing all internal memory                          */  \
int SMATRIX(_destroy)(SMATRIX() _q);                                        \
                                                                            \
/* Print sparse matrix in compact form to stdout                        */  \
int SMATRIX(_print)(SMATRIX() _q);                                          \
                                                                            \
/* Print sparse matrix in expanded form to stdout                       */  \
int SMATRIX(_print_expanded)(SMATRIX() _q);                                 \
                                                                            \
/* Get size of sparse matrix (number of rows and columns)               */  \
/*  _q  : sparse matrix object                                          */  \
/*  _m  : number of rows in matrix                                      */  \
/*  _n  : number of columns in matrix                                   */  \
int SMATRIX(_size)(SMATRIX()      _q,                                       \
                   unsigned int * _m,                                       \
                   unsigned int * _n);                                      \
                                                                            \
/* Zero all elements and retain allocated memory                        */  \
int SMATRIX(_clear)(SMATRIX() _q);                                          \
                                                                            \
/* Zero all elements and clear memory                                   */  \
int SMATRIX(_reset)(SMATRIX() _q);                                          \
                                                                            \
/* Determine if value has been set (allocated memory)                   */  \
/*  _q  : sparse matrix object                                          */  \
/*  _m  : row index of value to query                                   */  \
/*  _n  : column index of value to query                                */  \
int SMATRIX(_isset)(SMATRIX()    _q,                                        \
                    unsigned int _m,                                        \
                    unsigned int _n);                                       \
                                                                            \
/* Insert an element at index, allocating memory as necessary           */  \
/*  _q  : sparse matrix object                                          */  \
/*  _m  : row index of value to insert                                  */  \
/*  _n  : column index of value to insert                               */  \
/*  _v  : value to insert                                               */  \
int SMATRIX(_insert)(SMATRIX()    _q,                                       \
                     unsigned int _m,                                       \
                     unsigned int _n,                                       \
                     T            _v);                                      \
                                                                            \
/* Delete an element at index, freeing memory                           */  \
/*  _q  : sparse matrix object                                          */  \
/*  _m  : row index of value to delete                                  */  \
/*  _n  : column index of value to delete                               */  \
int SMATRIX(_delete)(SMATRIX()    _q,                                       \
                     unsigned int _m,                                       \
                     unsigned int _n);                                      \
                                                                            \
/* Set the value  in matrix at specified row and column, allocating     */  \
/* memory if needed                                                     */  \
/*  _q  : sparse matrix object                                          */  \
/*  _m  : row index of value to set                                     */  \
/*  _n  : column index of value to set                                  */  \
/*  _v  : value to set in matrix                                        */  \
int SMATRIX(_set)(SMATRIX()    _q,                                          \
                  unsigned int _m,                                          \
                  unsigned int _n,                                          \
                  T            _v);                                         \
                                                                            \
/* Get the value from matrix at specified row and column                */  \
/*  _q  : sparse matrix object                                          */  \
/*  _m  : row index of value to get                                     */  \
/*  _n  : column index of value to get                                  */  \
T SMATRIX(_get)(SMATRIX()    _q,                                            \
                unsigned int _m,                                            \
                unsigned int _n);                                           \
                                                                            \
/* Initialize to identity matrix; set all diagonal elements to 1, all   */  \
/* others to 0. This is done with both square and non-square matrices.  */  \
int SMATRIX(_eye)(SMATRIX() _q);                                            \
                                                                            \
/* Multiply two sparse matrices, \( \vec{Z} = \vec{X} \vec{Y} \)        */  \
/*  _x  : sparse matrix object (input)                                  */  \
/*  _y  : sparse matrix object (input)                                  */  \
/*  _z  : sparse matrix object (output)                                 */  \
int SMATRIX(_mul)(SMATRIX() _x,                                             \
                  SMATRIX() _y,                                             \
                  SMATRIX() _z);                                            \
                                                                            \
/* Multiply sparse matrix by vector                                     */  \
/*  _q  : sparse matrix                                                 */  \
/*  _x  : input vector, [size: _n x 1]                                  */  \
/*  _y  : output vector, [size: _m x 1]                                 */  \
int SMATRIX(_vmul)(SMATRIX() _q,                                            \
                   T *       _x,                                            \
                   T *       _y);                                           \

LIQUID_SMATRIX_DEFINE_API(LIQUID_SMATRIX_MANGLE_BOOL,  unsigned char)
LIQUID_SMATRIX_DEFINE_API(LIQUID_SMATRIX_MANGLE_FLOAT, float)
LIQUID_SMATRIX_DEFINE_API(LIQUID_SMATRIX_MANGLE_INT,   short int)

//
// smatrix cross methods
//

// multiply sparse binary matrix by floating-point matrix
//  _q  :   sparse matrix, [size: A->M x A->N]
//  _x  :   input vector,  [size:  mx  x  nx ]
//  _y  :   output vector, [size:  my  x  ny ]
int smatrixb_mulf(smatrixb     _A,
                  float *      _x,
                  unsigned int _mx,
                  unsigned int _nx,
                  float *      _y,
                  unsigned int _my,
                  unsigned int _ny);

// multiply sparse binary matrix by floating-point vector
//  _q  :   sparse matrix
//  _x  :   input vector, [size: _N x 1]
//  _y  :   output vector, [size: _M x 1]
int smatrixb_vmulf(smatrixb _q,
                   float *  _x,
                   float *  _y);


//
// MODULE : modem (modulator/demodulator)
//

// Maximum number of allowed bits per symbol
#define MAX_MOD_BITS_PER_SYMBOL 8

// Modulation schemes available
#define LIQUID_MODEM_NUM_SCHEMES      (53)

typedef enum {
    LIQUID_MODEM_UNKNOWN=0, // Unknown modulation scheme

    // Phase-shift keying (PSK)
    LIQUID_MODEM_PSK2,      LIQUID_MODEM_PSK4,
    LIQUID_MODEM_PSK8,      LIQUID_MODEM_PSK16,
    LIQUID_MODEM_PSK32,     LIQUID_MODEM_PSK64,
    LIQUID_MODEM_PSK128,    LIQUID_MODEM_PSK256,

    // Differential phase-shift keying (DPSK)
    LIQUID_MODEM_DPSK2,     LIQUID_MODEM_DPSK4,
    LIQUID_MODEM_DPSK8,     LIQUID_MODEM_DPSK16,
    LIQUID_MODEM_DPSK32,    LIQUID_MODEM_DPSK64,
    LIQUID_MODEM_DPSK128,   LIQUID_MODEM_DPSK256,

    // amplitude-shift keying
    LIQUID_MODEM_ASK2,      LIQUID_MODEM_ASK4,
    LIQUID_MODEM_ASK8,      LIQUID_MODEM_ASK16,
    LIQUID_MODEM_ASK32,     LIQUID_MODEM_ASK64,
    LIQUID_MODEM_ASK128,    LIQUID_MODEM_ASK256,

    // rectangular quadrature amplitude-shift keying (QAM)
    LIQUID_MODEM_QAM4,
    LIQUID_MODEM_QAM8,      LIQUID_MODEM_QAM16,
    LIQUID_MODEM_QAM32,     LIQUID_MODEM_QAM64,
    LIQUID_MODEM_QAM128,    LIQUID_MODEM_QAM256,

    // amplitude phase-shift keying (APSK)
    LIQUID_MODEM_APSK4,
    LIQUID_MODEM_APSK8,     LIQUID_MODEM_APSK16,
    LIQUID_MODEM_APSK32,    LIQUID_MODEM_APSK64,
    LIQUID_MODEM_APSK128,   LIQUID_MODEM_APSK256,

    // specific modem types
    LIQUID_MODEM_BPSK,      // Specific: binary PSK
    LIQUID_MODEM_QPSK,      // specific: quaternary PSK
    LIQUID_MODEM_OOK,       // Specific: on/off keying
    LIQUID_MODEM_SQAM32,    // 'square' 32-QAM
    LIQUID_MODEM_SQAM128,   // 'square' 128-QAM
    LIQUID_MODEM_V29,       // V.29 star constellation
    LIQUID_MODEM_ARB16OPT,  // optimal 16-QAM
    LIQUID_MODEM_ARB32OPT,  // optimal 32-QAM
    LIQUID_MODEM_ARB64OPT,  // optimal 64-QAM
    LIQUID_MODEM_ARB128OPT, // optimal 128-QAM
    LIQUID_MODEM_ARB256OPT, // optimal 256-QAM
    LIQUID_MODEM_ARB64VT,   // Virginia Tech logo
    LIQUID_MODEM_PI4DQPSK,  // pi/4 differential QPSK

    // arbitrary modem type
    LIQUID_MODEM_ARB        // arbitrary QAM
} modulation_scheme;

// structure for holding full modulation type descriptor
struct modulation_type_s {
    const char * name;          // short name (e.g. 'bpsk')
    const char * fullname;      // full name (e.g. 'binary phase-shift keying')
    modulation_scheme scheme;   // modulation scheme (e.g. LIQUID_MODEM_BPSK)
    unsigned int bps;           // modulation depth (e.g. 1)
};

// full modulation type descriptor
extern const struct modulation_type_s modulation_types[LIQUID_MODEM_NUM_SCHEMES];

// Print compact list of existing and available modulation schemes
int liquid_print_modulation_schemes();

// returns modulation_scheme based on input string
modulation_scheme liquid_getopt_str2mod(const char * _str);

// query basic modulation types
int liquid_modem_is_psk(modulation_scheme _ms);
int liquid_modem_is_dpsk(modulation_scheme _ms);
int liquid_modem_is_ask(modulation_scheme _ms);
int liquid_modem_is_qam(modulation_scheme _ms);
int liquid_modem_is_apsk(modulation_scheme _ms);

// useful functions

// counts the number of different bits between two symbols
unsigned int count_bit_errors(unsigned int _s1, unsigned int _s2);

// counts the number of different bits between two arrays of symbols
//  _msg0   :   original message, [size: _n x 1]
//  _msg1   :   copy of original message, [size: _n x 1]
//  _n      :   message size
unsigned int count_bit_errors_array(unsigned char * _msg0,
                                    unsigned char * _msg1,
                                    unsigned int _n);

// converts binary-coded decimal (BCD) to gray, ensuring successive values
// differ by exactly one bit
unsigned int gray_encode(unsigned int symbol_in);

// converts a gray-encoded symbol to binary-coded decimal (BCD)
unsigned int gray_decode(unsigned int symbol_in);

// pack soft bits into symbol
//  _soft_bits  :   soft input bits, [size: _bps x 1]
//  _bps        :   bits per symbol
//  _sym_out    :   output symbol, value in [0,2^_bps)
int liquid_pack_soft_bits(unsigned char * _soft_bits,
                          unsigned int _bps,
                          unsigned int * _sym_out);

// unpack soft bits into symbol
//  _sym_in     :   input symbol, value in [0,2^_bps)
//  _bps        :   bits per symbol
//  _soft_bits  :   soft output bits, [size: _bps x 1]
int liquid_unpack_soft_bits(unsigned int _sym_in,
                            unsigned int _bps,
                            unsigned char * _soft_bits);


//
// Linear modem
//

#define LIQUID_MODEM_MANGLE_FLOAT(name) LIQUID_CONCAT(modemcf,name)
// temporary shim to support backwards compatibility between "modemcf" and "modem"
#define LIQUID_MODEM_MANGLE_FLOAT_SHIM(name) LIQUID_CONCAT(modem,name)

// FIXME: need to point both modem and modemcf pointers to same struct (shim)
typedef struct modemcf_s * modemcf;
typedef struct modemcf_s * modem;

// Macro    :   MODEM
//  MODEM   :   name-mangling macro
//  T       :   primitive data type
//  TC      :   primitive data type (complex)
#define LIQUID_MODEM_DEFINE_API(MODEM,T,TC)                                 \
                                                                            \
/* Linear modulator/demodulator (modem) object                          */  \
/* FIXME: need to point both modem and modemcf pointers to same struct  */  \
/*typedef struct MODEM(_s) * MODEM();                                   */  \
                                                                            \
/* Create digital modem object with a particular scheme                 */  \
/*  _scheme : linear modulation scheme (e.g. LIQUID_MODEM_QPSK)         */  \
MODEM() MODEM(_create)(modulation_scheme _scheme);                          \
                                                                            \
/* Create linear digital modem object with arbitrary constellation      */  \
/* points defined by an external table of symbols. Sample points are    */  \
/* provided as complex float pairs and converted internally if needed.  */  \
/*  _table  : array of complex constellation points, [size: _M x 1]     */  \
/*  _M      : modulation order and table size, _M must be power of 2    */  \
MODEM() MODEM(_create_arbitrary)(liquid_float_complex * _table,             \
                                 unsigned int           _M);                \
                                                                            \
/* Recreate modulation scheme, re-allocating memory as necessary        */  \
/*  _q      : modem object                                              */  \
/*  _scheme : linear modulation scheme (e.g. LIQUID_MODEM_QPSK)         */  \
MODEM() MODEM(_recreate)(MODEM()           _q,                              \
                         modulation_scheme _scheme);                        \
                                                                            \
/* Copy object including all internal objects and state                 */  \
MODEM() MODEM(_copy)(MODEM() _q);                                           \
                                                                            \
/* Destroy modem object, freeing all allocated memory                   */  \
int MODEM(_destroy)(MODEM() _q);                                            \
                                                                            \
/* Print modem status to stdout                                         */  \
int MODEM(_print)(MODEM() _q);                                              \
                                                                            \
/* Reset internal state of modem object; note that this is only         */  \
/* relevant for modulation types that retain an internal state such as  */  \
/* LIQUID_MODEM_DPSK4 as most linear modulation types are stateless     */  \
int MODEM(_reset)(MODEM() _q);                                              \
                                                                            \
/* Generate random symbol for modulation                                */  \
unsigned int MODEM(_gen_rand_sym)(MODEM() _q);                              \
                                                                            \
/* Get number of bits per symbol (bps) of modem object                  */  \
unsigned int MODEM(_get_bps)(MODEM() _q);                                   \
                                                                            \
/* Get modulation scheme of modem object                                */  \
modulation_scheme MODEM(_get_scheme)(MODEM() _q);                           \
                                                                            \
/* Modulate input symbol (bits) and generate output complex sample      */  \
/*  _q  : modem object                                                  */  \
/*  _s  : input symbol, 0 <= _s <= M-1                                  */  \
/*  _y  : output complex sample                                         */  \
int MODEM(_modulate)(MODEM()      _q,                                       \
                     unsigned int _s,                                       \
                     TC *         _y);                                      \
                                                                            \
/* Demodulate input sample and provide maximum-likelihood estimate of   */  \
/* symbol that would have generated it.                                 */  \
/* The output is a hard decision value on the input sample.             */  \
/* This is performed efficiently by taking advantage of symmetry on     */  \
/* most modulation types.                                               */  \
/* For example, square and rectangular quadrature amplitude modulation  */  \
/* with gray coding can use a bisection search independently on its     */  \
/* in-phase and quadrature channels.                                    */  \
/* Arbitrary modulation schemes are relatively slow, however, for large */  \
/* modulation types as the demodulator must compute the distance        */  \
/* between the received sample and all possible symbols to derive the   */  \
/* optimal symbol.                                                      */  \
/*  _q  :   modem object                                                */  \
/*  _x  :   input sample                                                */  \
/*  _s  : output hard symbol, 0 <= _s <= M-1                            */  \
int MODEM(_demodulate)(MODEM()        _q,                                   \
                       TC             _x,                                   \
                       unsigned int * _s);                                  \
                                                                            \
/* Demodulate input sample and provide (approximate) log-likelihood     */  \
/* ratio (LLR, soft bits) as an output.                                 */  \
/* Similarly to the hard-decision demodulation method, this is computed */  \
/* efficiently for most modulation types.                               */  \
/*  _q          : modem object                                          */  \
/*  _x          : input sample                                          */  \
/*  _s          : output hard symbol, 0 <= _s <= M-1                    */  \
/*  _soft_bits  : output soft bits, [size: log2(M) x 1]                 */  \
int MODEM(_demodulate_soft)(MODEM()         _q,                             \
                            TC              _x,                             \
                            unsigned int  * _s,                             \
                            unsigned char * _soft_bits);                    \
                                                                            \
/* Get demodulator's estimated transmit sample                          */  \
int MODEM(_get_demodulator_sample)(MODEM() _q,                              \
                                   TC *    _x_hat);                         \
                                                                            \
/* Get demodulator phase error                                          */  \
float MODEM(_get_demodulator_phase_error)(MODEM() _q);                      \
                                                                            \
/* Get demodulator error vector magnitude                               */  \
float MODEM(_get_demodulator_evm)(MODEM() _q);                              \

// define modem APIs
LIQUID_MODEM_DEFINE_API(LIQUID_MODEM_MANGLE_FLOAT,float,liquid_float_complex)
LIQUID_MODEM_DEFINE_API(LIQUID_MODEM_MANGLE_FLOAT_SHIM,float,liquid_float_complex)

//
// continuous-phase modulation
//

// gmskmod : GMSK modulator
typedef struct gmskmod_s * gmskmod;

// create gmskmod object
//  _k      :   samples/symbol
//  _m      :   filter delay (symbols)
//  _BT     :   excess bandwidth factor
gmskmod gmskmod_create(unsigned int _k,
                       unsigned int _m,
                       float        _BT);
// Copy object recursively including all internal objects and state
gmskmod gmskmod_copy(gmskmod _q);
int gmskmod_destroy(gmskmod _q);
int gmskmod_print(gmskmod _q);
int gmskmod_reset(gmskmod _q);
int gmskmod_modulate(gmskmod                _q,
                     unsigned int           _sym,
                     liquid_float_complex * _y);


// gmskdem : GMSK demodulator
typedef struct gmskdem_s * gmskdem;

// create gmskdem object
//  _k      :   samples/symbol
//  _m      :   filter delay (symbols)
//  _BT     :   excess bandwidth factor
gmskdem gmskdem_create(unsigned int _k,
                       unsigned int _m,
                       float        _BT);
// Copy object recursively including all internal objects and state
gmskdem gmskdem_copy(gmskdem _q);
int gmskdem_destroy(gmskdem _q);
int gmskdem_print(gmskdem _q);
int gmskdem_reset(gmskdem _q);
int gmskdem_set_eq_bw(gmskdem _q, float _bw);
int gmskdem_demodulate(gmskdem                _q,
                       liquid_float_complex * _y,
                       unsigned int *         _sym);

//
// continuous phase frequency-shift keying (CP-FSK) modems
//

// CP-FSK filter prototypes
typedef enum {
    //LIQUID_CPFSK_UNKNOWN=0
    LIQUID_CPFSK_SQUARE=0,      // square pulse
    LIQUID_CPFSK_RCOS_FULL,     // raised-cosine (full response)
    LIQUID_CPFSK_RCOS_PARTIAL,  // raised-cosine (partial response)
    LIQUID_CPFSK_GMSK,          // Gauss minimum-shift keying pulse
} liquid_cpfsk_filter;

// TODO: rename to cpfskmodcf for consistency
#define LIQUID_CPFSKMOD_MANGLE_FLOAT(name) LIQUID_CONCAT(cpfskmod,name)

#define LIQUID_CPFSKMOD_DEFINE_API(CPFSKMOD,T,TC)                           \
                                                                            \
/* Continuous-Phase Frequency-Shift Keying Modulator                    */  \
typedef struct CPFSKMOD(_s) * CPFSKMOD();                                   \
                                                                            \
/* create cpfskmod object (frequency modulator)                         */  \
/*  _bps    : bits per symbol, _bps > 0                                 */  \
/*  _h      : modulation index, _h > 0                                  */  \
/*  _k      : samples/symbol, _k > 1, _k even                           */  \
/*  _m      : filter delay (symbols), _m > 0                            */  \
/*  _beta   : filter bandwidth parameter, _beta > 0                     */  \
/*  _type   : filter type (e.g. LIQUID_CPFSK_SQUARE)                    */  \
CPFSKMOD() CPFSKMOD(_create)(unsigned int _bps,                             \
                             float        _h,                               \
                             unsigned int _k,                               \
                             unsigned int _m,                               \
                             float        _beta,                            \
                             int          _type);                           \
                                                                            \
/* create modulator object for minimum-shift keying                     */  \
/*  _k      : samples/symbol, _k > 1, _k even                           */  \
CPFSKMOD() CPFSKMOD(_create_msk)(unsigned int _k);                          \
                                                                            \
/* create modulator object for Gauss minimum-shift keying               */  \
/*  _k      : samples/symbol, _k > 1, _k even                           */  \
/*  _m      : filter delay (symbols), _m > 0                            */  \
/*  _BT     : bandwidth-time factor, 0 < _BT < 1                        */  \
CPFSKMOD() CPFSKMOD(_create_gmsk)(unsigned int _k,                          \
                                  unsigned int _m,                          \
                                  float        _BT);                        \
                                                                            \
/* Copy object including all internal objects and state                 */  \
CPFSKMOD() CPFSKMOD(_copy)(CPFSKMOD() _q);                                  \
                                                                            \
/* Destroy modulator object, freeing all allocate memory                */  \
int CPFSKMOD(_destroy)(CPFSKMOD() _q);                                      \
                                                                            \
/* Print modulator status to stdout                                     */  \
int CPFSKMOD(_print)(CPFSKMOD() _q);                                        \
                                                                            \
/* Reset internal state of modulator object                             */  \
int CPFSKMOD(_reset)(CPFSKMOD() _q);                                        \
                                                                            \
/* Get modulator's number of bits per symbol                            */  \
unsigned int CPFSKMOD(_get_bits_per_symbol)(CPFSKMOD() _q);                 \
                                                                            \
/* Get modulator's modulation index                                     */  \
float CPFSKMOD(_get_modulation_index)(CPFSKMOD() _q);                       \
                                                                            \
/* Get modulator's number of samples per symbol                         */  \
unsigned int CPFSKMOD(_get_samples_per_symbol)(CPFSKMOD() _q);              \
                                                                            \
/* Get modulator's filter delay [symbols]                               */  \
unsigned int CPFSKMOD(_get_delay)(CPFSKMOD() _q);                           \
                                                                            \
/* Get modulator's bandwidth parameter                                  */  \
float CPFSKMOD(_get_beta)(CPFSKMOD() _q);                                   \
                                                                            \
/* Get modulator's filter type                                          */  \
int CPFSKMOD(_get_type)(CPFSKMOD() _q);                                     \
                                                                            \
/* modulate sample                                                      */  \
/*  _q      :   frequency modulator object                              */  \
/*  _s      :   input symbol                                            */  \
/*  _y      :   output sample array, [size: _k x 1]                     */  \
int CPFSKMOD(_modulate)(CPFSKMOD()   _q,                                    \
                        unsigned int _s,                                    \
                        TC *         _y);                                   \

// define cpfskmod APIs
LIQUID_CPFSKMOD_DEFINE_API(LIQUID_CPFSKMOD_MANGLE_FLOAT,float,liquid_float_complex)



// TODO: rename to cpfskdemcf for consistency
#define LIQUID_CPFSKDEM_MANGLE_FLOAT(name) LIQUID_CONCAT(cpfskdem,name)

#define LIQUID_CPFSKDEM_DEFINE_API(CPFSKDEM,T,TC)                           \
                                                                            \
/* Continuous-Phase Frequency-Shift Keying Demodulator                  */  \
typedef struct CPFSKDEM(_s) * CPFSKDEM();                                   \
                                                                            \
/* create demodulator object                                            */  \
/*  _bps    :   bits per symbol, _bps > 0                               */  \
/*  _h      :   modulation index, _h > 0                                */  \
/*  _k      :   samples/symbol, _k > 1, _k even                         */  \
/*  _m      :   filter delay (symbols), _m > 0                          */  \
/*  _beta   :   filter bandwidth parameter, _beta > 0                   */  \
/*  _type   :   filter type (e.g. LIQUID_CPFSK_SQUARE)                  */  \
CPFSKDEM() CPFSKDEM(_create)(unsigned int _bps,                             \
                             float        _h,                               \
                             unsigned int _k,                               \
                             unsigned int _m,                               \
                             float        _beta,                            \
                             int          _type);                           \
                                                                            \
/* create demodulator object for minimum-shift keying                   */  \
/*  _k      : samples/symbol, _k > 1, _k even                           */  \
CPFSKDEM() CPFSKDEM(_create_msk)(unsigned int _k);                          \
                                                                            \
/* create demodulator object for Gauss minimum-shift keying             */  \
/*  _k      : samples/symbol, _k > 1, _k even                           */  \
/*  _m      : filter delay (symbols), _m > 0                            */  \
/*  _BT     : bandwidth-time factor, 0 < _BT < 1                        */  \
CPFSKDEM() CPFSKDEM(_create_gmsk)(unsigned int _k,                          \
                                  unsigned int _m,                          \
                                  float        _BT);                        \
                                                                            \
/* Copy object including all internal objects and state                 */  \
CPFSKDEM() CPFSKDEM(_copy)(CPFSKDEM() _q);                                  \
                                                                            \
/* Destroy demodulator object, freeing all internal memory              */  \
int CPFSKDEM(_destroy)(CPFSKDEM() _q);                                      \
                                                                            \
/* Print demodulator object internals                                   */  \
int CPFSKDEM(_print)(CPFSKDEM() _q);                                        \
                                                                            \
/* Reset state                                                          */  \
int CPFSKDEM(_reset)(CPFSKDEM() _q);                                        \
                                                                            \
/* Get demodulator's number of bits per symbol                          */  \
unsigned int CPFSKDEM(_get_bits_per_symbol)(CPFSKDEM() _q);                 \
                                                                            \
/* Get demodulator's modulation index                                   */  \
float CPFSKDEM(_get_modulation_index)(CPFSKDEM() _q);                       \
                                                                            \
/* Get demodulator's number of samples per symbol                       */  \
unsigned int CPFSKDEM(_get_samples_per_symbol)(CPFSKDEM() _q);              \
                                                                            \
/* Get demodulator's transmit delay [symbols]                           */  \
unsigned int CPFSKDEM(_get_delay)(CPFSKDEM() _q);                           \
                                                                            \
/* Get demodulator's bandwidth parameter                                */  \
float CPFSKDEM(_get_beta)(CPFSKDEM() _q);                                   \
                                                                            \
/* Get demodulator's filter type                                        */  \
int CPFSKDEM(_get_type)(CPFSKDEM() _q);                                     \
                                                                            \
/* demodulate array of samples, assuming perfect timing                 */  \
/*  _q      :   continuous-phase frequency demodulator object           */  \
/*  _y      :   input sample array, [size: _k x 1]                      */  \
unsigned int CPFSKDEM(_demodulate)(CPFSKDEM() _q,                           \
                                   TC *       _y);                          \
                                                                            \
/* demodulate_block */                                                      \

// define cpfskmod APIs
LIQUID_CPFSKDEM_DEFINE_API(LIQUID_CPFSKDEM_MANGLE_FLOAT,float,liquid_float_complex)

//
// M-ary frequency-shift keying (MFSK) modems
//

// FSK modulator
typedef struct fskmod_s * fskmod;

// create fskmod object (frequency modulator)
//  _m          :   bits per symbol, _bps > 0
//  _k          :   samples/symbol, _k >= 2^_m
//  _bandwidth  :   total signal bandwidth, (0,0.5)
fskmod fskmod_create(unsigned int _m,
                     unsigned int _k,
                     float        _bandwidth);

// Copy object recursively including all internal objects and state
fskmod fskmod_copy(fskmod _q);

// destroy fskmod object
int fskmod_destroy(fskmod _q);

// print fskmod object internals
int fskmod_print(fskmod _q);

// reset state
int fskmod_reset(fskmod _q);

// modulate sample
//  _q      :   frequency modulator object
//  _s      :   input symbol
//  _y      :   output sample array, [size: _k x 1]
int fskmod_modulate(fskmod                 _q,
                    unsigned int           _s,
                    liquid_float_complex * _y);



// FSK demodulator
typedef struct fskdem_s * fskdem;

// create fskdem object (frequency demodulator)
//  _m          :   bits per symbol, _bps > 0
//  _k          :   samples/symbol, _k >= 2^_m
//  _bandwidth  :   total signal bandwidth, (0,0.5)
fskdem fskdem_create(unsigned int _m,
                     unsigned int _k,
                     float        _bandwidth);

// Copy object recursively including all internal objects and state
fskdem fskdem_copy(fskdem _q);

// destroy fskdem object
int fskdem_destroy(fskdem _q);

// print fskdem object internals
int fskdem_print(fskdem _q);

// reset state
int fskdem_reset(fskdem _q);

// demodulate symbol, assuming perfect symbol timing
//  _q      :   fskdem object
//  _y      :   input sample array, [size: _k x 1]
unsigned int fskdem_demodulate(fskdem                 _q,
                               liquid_float_complex * _y);

// get demodulator frequency error
float fskdem_get_frequency_error(fskdem _q);

// get energy for a particular symbol within a certain range
float fskdem_get_symbol_energy(fskdem       _q,
                               unsigned int _s,
                               unsigned int _range);


//
// Analog frequency modulator
//
#define LIQUID_FREQMOD_MANGLE_FLOAT(name) LIQUID_CONCAT(freqmod,name)

// Macro    :   FREQMOD (analog frequency modulator)
//  FREQMOD :   name-mangling macro
//  T       :   primitive data type
//  TC      :   primitive data type (complex)
#define LIQUID_FREQMOD_DEFINE_API(FREQMOD,T,TC)                             \
                                                                            \
/* Analog frequency modulation object                                   */  \
typedef struct FREQMOD(_s) * FREQMOD();                                     \
                                                                            \
/* Create freqmod object with a particular modulation factor            */  \
/*  _kf :   modulation factor                                           */  \
FREQMOD() FREQMOD(_create)(float _kf);                                      \
                                                                            \
/* Destroy freqmod object, freeing all internal memory                  */  \
int FREQMOD(_destroy)(FREQMOD() _q);                                        \
                                                                            \
/* Print freqmod object internals to stdout                             */  \
int FREQMOD(_print)(FREQMOD() _q);                                          \
                                                                            \
/* Reset state                                                          */  \
int FREQMOD(_reset)(FREQMOD() _q);                                          \
                                                                            \
/* Modulate single sample, producing single output sample at complex    */  \
/* baseband.                                                            */  \
/*  _q  : frequency modulator object                                    */  \
/*  _m  : message signal \( m(t) \)                                     */  \
/*  _s  : complex baseband signal \( s(t) \)                            */  \
int FREQMOD(_modulate)(FREQMOD() _q,                                        \
                       T         _m,                                        \
                       TC *      _s);                                       \
                                                                            \
/* Modulate block of samples                                            */  \
/*  _q  : frequency modulator object                                    */  \
/*  _m  : message signal \( m(t) \), [size: _n x 1]                     */  \
/*  _n  : number of input, output samples                               */  \
/*  _s  : complex baseband signal \( s(t) \),  [size: _n x 1]           */  \
int FREQMOD(_modulate_block)(FREQMOD()    _q,                               \
                             T *          _m,                               \
                             unsigned int _n,                               \
                             TC *         _s);                              \

// define freqmod APIs
LIQUID_FREQMOD_DEFINE_API(LIQUID_FREQMOD_MANGLE_FLOAT,float,liquid_float_complex)

//
// Analog frequency demodulator
//

#define LIQUID_FREQDEM_MANGLE_FLOAT(name) LIQUID_CONCAT(freqdem,name)

// Macro    :   FREQDEM (analog frequency modulator)
//  FREQDEM :   name-mangling macro
//  T       :   primitive data type
//  TC      :   primitive data type (complex)
#define LIQUID_FREQDEM_DEFINE_API(FREQDEM,T,TC)                             \
                                                                            \
/* Analog frequency demodulator                                         */  \
typedef struct FREQDEM(_s) * FREQDEM();                                     \
                                                                            \
/* Create freqdem object (frequency modulator)                          */  \
/*  _kf      :   modulation factor                                      */  \
FREQDEM() FREQDEM(_create)(float _kf);                                      \
                                                                            \
/* Destroy freqdem object                                               */  \
int FREQDEM(_destroy)(FREQDEM() _q);                                        \
                                                                            \
/* Print freqdem object internals                                       */  \
int FREQDEM(_print)(FREQDEM() _q);                                          \
                                                                            \
/* Reset state                                                          */  \
int FREQDEM(_reset)(FREQDEM() _q);                                          \
                                                                            \
/* Demodulate sample                                                    */  \
/*  _q      :   frequency modulator object                              */  \
/*  _r      :   received signal r(t)                                    */  \
/*  _m      :   output message signal m(t)                              */  \
int FREQDEM(_demodulate)(FREQDEM() _q,                                      \
                         TC        _r,                                      \
                         T *       _m);                                     \
                                                                            \
/* Demodulate block of samples                                          */  \
/*  _q      :   frequency demodulator object                            */  \
/*  _r      :   received signal r(t) [size: _n x 1]                     */  \
/*  _n      :   number of input, output samples                         */  \
/*  _m      :   message signal m(t), [size: _n x 1]                     */  \
int FREQDEM(_demodulate_block)(FREQDEM()    _q,                             \
                               TC *         _r,                             \
                               unsigned int _n,                             \
                               T *          _m);                            \

// define freqdem APIs
LIQUID_FREQDEM_DEFINE_API(LIQUID_FREQDEM_MANGLE_FLOAT,float,liquid_float_complex)



// amplitude modulation types
typedef enum {
    LIQUID_AMPMODEM_DSB=0,  // double side-band
    LIQUID_AMPMODEM_USB,    // single side-band (upper)
    LIQUID_AMPMODEM_LSB     // single side-band (lower)
} liquid_ampmodem_type;

typedef struct ampmodem_s * ampmodem;

// create ampmodem object
//  _m                  :   modulation index
//  _type               :   AM type (e.g. LIQUID_AMPMODEM_DSB)
//  _suppressed_carrier :   carrier suppression flag
ampmodem ampmodem_create(float                _mod_index,
                         liquid_ampmodem_type _type,
                         int                  _suppressed_carrier);

// destroy ampmodem object
int ampmodem_destroy(ampmodem _q);

// print ampmodem object internals
int ampmodem_print(ampmodem _q);

// reset ampmodem object state
int ampmodem_reset(ampmodem _q);

// accessor methods
unsigned int ampmodem_get_delay_mod  (ampmodem _q);
unsigned int ampmodem_get_delay_demod(ampmodem _q);

// modulate sample
int ampmodem_modulate(ampmodem               _q,
                      float                  _x,
                      liquid_float_complex * _y);

int ampmodem_modulate_block(ampmodem               _q,
                            float *                _m,
                            unsigned int           _n,
                            liquid_float_complex * _s);

// demodulate sample
int ampmodem_demodulate(ampmodem             _q,
                        liquid_float_complex _y,
                        float *              _x);

int ampmodem_demodulate_block(ampmodem               _q,
                              liquid_float_complex * _r,
                              unsigned int           _n,
                              float *                _m);

//
// MODULE : multichannel
//


#define FIRPFBCH_NYQUIST        0
#define FIRPFBCH_ROOTNYQUIST    1

#define LIQUID_ANALYZER         0
#define LIQUID_SYNTHESIZER      1


//
// Finite impulse response polyphase filterbank channelizer
//

#define LIQUID_FIRPFBCH_MANGLE_CRCF(name) LIQUID_CONCAT(firpfbch_crcf,name)
#define LIQUID_FIRPFBCH_MANGLE_CCCF(name) LIQUID_CONCAT(firpfbch_cccf,name)

// Macro:
//   FIRPFBCH   : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_FIRPFBCH_DEFINE_API(FIRPFBCH,TO,TC,TI)                       \
                                                                            \
/* Finite impulse response polyphase filterbank channelizer             */  \
typedef struct FIRPFBCH(_s) * FIRPFBCH();                                   \
                                                                            \
/* Create finite impulse response polyphase filter-bank                 */  \
/* channelizer object from external coefficients                        */  \
/*  _type   : channelizer type, e.g. LIQUID_ANALYZER                    */  \
/*  _M      : number of channels                                        */  \
/*  _p      : number of coefficients for each channel                   */  \
/*  _h      : coefficients, [size: _M*_p x 1]                           */  \
FIRPFBCH() FIRPFBCH(_create)(int          _type,                            \
                             unsigned int _M,                               \
                             unsigned int _p,                               \
                             TC *         _h);                              \
                                                                            \
/* Create FIR polyphase filterbank channelizer object with              */  \
/* prototype filter based on windowed Kaiser design                     */  \
/*  _type   : type (LIQUID_ANALYZER | LIQUID_SYNTHESIZER)               */  \
/*  _M      : number of channels                                        */  \
/*  _m      : filter delay (symbols)                                    */  \
/*  _As     : stop-band attenuation [dB]                                */  \
FIRPFBCH() FIRPFBCH(_create_kaiser)(int          _type,                     \
                                    unsigned int _M,                        \
                                    unsigned int _m,                        \
                                    float        _As);                      \
                                                                            \
/* Create FIR polyphase filterbank channelizer object with              */  \
/* prototype root-Nyquist filter                                        */  \
/*  _type   : type (LIQUID_ANALYZER | LIQUID_SYNTHESIZER)               */  \
/*  _M      : number of channels                                        */  \
/*  _m      : filter delay (symbols)                                    */  \
/*  _beta   : filter excess bandwidth factor, in [0,1]                  */  \
/*  _ftype  : filter prototype (rrcos, rkaiser, etc.)                   */  \
FIRPFBCH() FIRPFBCH(_create_rnyquist)(int          _type,                   \
                                      unsigned int _M,                      \
                                      unsigned int _m,                      \
                                      float        _beta,                   \
                                      int          _ftype);                 \
                                                                            \
/* Destroy firpfbch object                                              */  \
int FIRPFBCH(_destroy)(FIRPFBCH() _q);                                      \
                                                                            \
/* Clear/reset firpfbch internal state                                  */  \
int FIRPFBCH(_reset)(FIRPFBCH() _q);                                        \
                                                                            \
/* Print firpfbch internal parameters to stdout                         */  \
int FIRPFBCH(_print)(FIRPFBCH() _q);                                        \
                                                                            \
/* Execute filterbank as synthesizer on block of samples                */  \
/*  _q      : filterbank channelizer object                             */  \
/*  _x      : channelized input, [size: num_channels x 1]               */  \
/*  _y      : output time series, [size: num_channels x 1]              */  \
int FIRPFBCH(_synthesizer_execute)(FIRPFBCH() _q,                           \
                                   TI *       _x,                           \
                                   TO *       _y);                          \
                                                                            \
/* Execute filterbank as analyzer on block of samples                   */  \
/*  _q      : filterbank channelizer object                             */  \
/*  _x      : input time series, [size: num_channels x 1]               */  \
/*  _y      : channelized output, [size: num_channels x 1]              */  \
int FIRPFBCH(_analyzer_execute)(FIRPFBCH() _q,                              \
                                TI *       _x,                              \
                                TO *       _y);                             \


LIQUID_FIRPFBCH_DEFINE_API(LIQUID_FIRPFBCH_MANGLE_CRCF,
                           liquid_float_complex,
                           float,
                           liquid_float_complex)

LIQUID_FIRPFBCH_DEFINE_API(LIQUID_FIRPFBCH_MANGLE_CCCF,
                           liquid_float_complex,
                           liquid_float_complex,
                           liquid_float_complex)


//
// Finite impulse response polyphase filterbank channelizer
// with output rate 2 Fs / M
//

#define LIQUID_FIRPFBCH2_MANGLE_CRCF(name) LIQUID_CONCAT(firpfbch2_crcf,name)

// Macro:
//   FIRPFBCH2  : name-mangling macro
//   TO         : output data type
//   TC         : coefficients data type
//   TI         : input data type
#define LIQUID_FIRPFBCH2_DEFINE_API(FIRPFBCH2,TO,TC,TI)                     \
                                                                            \
/* Finite impulse response polyphase filterbank channelizer             */  \
/* with output rate oversampled by a factor of 2                        */  \
typedef struct FIRPFBCH2(_s) * FIRPFBCH2();                                 \
                                                                            \
/* Create firpfbch2 object with prototype filter from external          */  \
/* coefficients                                                         */  \
/*  _type   : channelizer type (e.g. LIQUID_ANALYZER)                   */  \
/*  _M      : number of channels (must be even)                         */  \
/*  _m      : prototype filter semi-length, length=2*M*m                */  \
/*  _h      : prototype filter coefficient array                        */  \
FIRPFBCH2() FIRPFBCH2(_create)(int          _type,                          \
                               unsigned int _M,                             \
                               unsigned int _m,                             \
                               TC *         _h);                            \
                                                                            \
/* Create firpfbch2 object using Kaiser window prototype                */  \
/*  _type   : channelizer type (e.g. LIQUID_ANALYZER)                   */  \
/*  _M      : number of channels (must be even)                         */  \
/*  _m      : prototype filter semi-length, length=2*M*m+1              */  \
/*  _As     : filter stop-band attenuation [dB]                         */  \
FIRPFBCH2() FIRPFBCH2(_create_kaiser)(int          _type,                   \
                                      unsigned int _M,                      \
                                      unsigned int _m,                      \
                                      float        _As);                    \
                                                                            \
/* Copy object recursively including all internal objects and state     */  \
FIRPFBCH2() FIRPFBCH2(_copy)(FIRPFBCH2() _q);                               \
                                                                            \
/* Destroy firpfbch2 object, freeing internal memory                    */  \
int FIRPFBCH2(_destroy)(FIRPFBCH2() _q);                                    \
                                                                            \
/* Reset firpfbch2 object internals                                     */  \
int FIRPFBCH2(_reset)(FIRPFBCH2() _q);                                      \
                                                                            \
/* Print firpfbch2 object internals                                     */  \
int FIRPFBCH2(_print)(FIRPFBCH2() _q);                                      \
                                                                            \
/* Get type, either LIQUID_ANALYZER or LIQUID_SYNTHESIZER               */  \
int FIRPFBCH2(_get_type)(FIRPFBCH2() _q);                                   \
                                                                            \
/* Get number of channels, M                                            */  \
unsigned int FIRPFBCH2(_get_M)(FIRPFBCH2() _q);                             \
                                                                            \
/* Get prototype filter sem-length, m                                   */  \
unsigned int FIRPFBCH2(_get_m)(FIRPFBCH2() _q);                             \
                                                                            \
/* Execute filterbank channelizer                                       */  \
/* LIQUID_ANALYZER:     input: M/2, output: M                           */  \
/* LIQUID_SYNTHESIZER:  input: M,   output: M/2                         */  \
/*  _x      :   channelizer input                                       */  \
/*  _y      :   channelizer output                                      */  \
int FIRPFBCH2(_execute)(FIRPFBCH2() _q,                                     \
                        TI *        _x,                                     \
                        TO *        _y);                                    \


LIQUID_FIRPFBCH2_DEFINE_API(LIQUID_FIRPFBCH2_MANGLE_CRCF,
                            liquid_float_complex,
                            float,
                            liquid_float_complex)

//
// Finite impulse response polyphase filterbank channelizer with output rate
// Fs / decim on each of independent, evenly-spaced channels
//

#define LIQUID_FIRPFBCHR_MANGLE_CRCF(name) LIQUID_CONCAT(firpfbchr_crcf,name)

#define LIQUID_FIRPFBCHR_DEFINE_API(FIRPFBCHR,TO,TC,TI)                     \
                                                                            \
/* Finite impulse response polyphase filterbank channelizer             */  \
/* with output rational output rate \( P / M \)                         */  \
typedef struct FIRPFBCHR(_s) * FIRPFBCHR();                                 \
                                                                            \
/* create rational rate resampling channelizer (firpfbchr) object by    */  \
/* specifying filter coefficients directly                              */  \
/*  _chans  : number of output channels in chanelizer                   */  \
/*  _decim  : output decimation factor (output rate is 1/decim input)   */  \
/*  _m      : prototype filter semi-length, length=2*chans*m            */  \
/*  _h      : prototype filter coefficient array, [size: 2*chans*m x 1] */  \
FIRPFBCHR() FIRPFBCHR(_create)(unsigned int _chans,                         \
                               unsigned int _decim,                         \
                               unsigned int _m,                             \
                               TC *         _h);                            \
                                                                            \
/* create rational rate resampling channelizer (firpfbchr) object by    */  \
/* specifying filter design parameters for Kaiser prototype             */  \
/*  _chans  : number of output channels in chanelizer                   */  \
/*  _decim  : output decimation factor (output rate is 1/decim input)   */  \
/*  _m      : prototype filter semi-length, length=2*chans*m            */  \
/*  _as     : filter stop-band attenuation [dB]                         */  \
FIRPFBCHR() FIRPFBCHR(_create_kaiser)(unsigned int _chans,                  \
                                      unsigned int _decim,                  \
                                      unsigned int _m,                      \
                                      float        _as);                    \
                                                                            \
/* destroy firpfbchr object, freeing internal memory                    */  \
int FIRPFBCHR(_destroy)(FIRPFBCHR() _q);                                    \
                                                                            \
/* reset firpfbchr object internal state and buffers                    */  \
int FIRPFBCHR(_reset)(FIRPFBCHR() _q);                                      \
                                                                            \
/* print firpfbchr object internals to stdout                           */  \
int FIRPFBCHR(_print)(FIRPFBCHR() _q);                                      \
                                                                            \
/* get number of output channels to channelizer                         */  \
DEPRECATED("use firpfbchr_get_num_channels(...) instead",                   \
unsigned int FIRPFBCHR(_get_M)(FIRPFBCHR() _q); )                           \
unsigned int FIRPFBCHR(_get_num_channels)(FIRPFBCHR() _q);                  \
                                                                            \
/* get decimation factor for channelizer                                */  \
DEPRECATED("use firpfbchr_get_decim_rate(...) instead",                     \
unsigned int FIRPFBCHR(_get_P)(FIRPFBCHR() _q); )                           \
unsigned int FIRPFBCHR(_get_decim_rate)(FIRPFBCHR() _q);                    \
                                                                            \
/* get semi-length to channelizer filter prototype                      */  \
unsigned int FIRPFBCHR(_get_m)(FIRPFBCHR() _q);                             \
                                                                            \
/* push buffer of samples into filter bank                              */  \
/*  _q      : channelizer object                                        */  \
/*  _x      : channelizer input, [size: decim x 1]                      */  \
int FIRPFBCHR(_push)(FIRPFBCHR() _q,                                        \
                     TI *        _x);                                       \
                                                                            \
/* execute filterbank channelizer, writing complex baseband samples for */  \
/* each channel into output array                                       */  \
/*  _q      : channelizer object                                        */  \
/*  _y      : channelizer output, [size: chans x 1]                     */  \
int FIRPFBCHR(_execute)(FIRPFBCHR() _q,                                     \
                        TO *        _y);                                    \


LIQUID_FIRPFBCHR_DEFINE_API(LIQUID_FIRPFBCHR_MANGLE_CRCF,
                            liquid_float_complex,
                            float,
                            liquid_float_complex)



#define OFDMFRAME_SCTYPE_NULL   0
#define OFDMFRAME_SCTYPE_PILOT  1
#define OFDMFRAME_SCTYPE_DATA   2

// initialize default subcarrier allocation
//  _M      :   number of subcarriers
//  _p      :   output subcarrier allocation array, [size: _M x 1]
int ofdmframe_init_default_sctype(unsigned int    _M,
                                  unsigned char * _p);

// initialize default subcarrier allocation
//  _M      :   number of subcarriers
//  _f0     :   lower frequency band, _f0 in [-0.5,0.5]
//  _f1     :   upper frequency band, _f1 in [-0.5,0.5]
//  _p      :   output subcarrier allocation array, [size: _M x 1]
int ofdmframe_init_sctype_range(unsigned int    _M,
                                float           _f0,
                                float           _f1,
                                unsigned char * _p);

// validate subcarrier type (count number of null, pilot, and data
// subcarriers in the allocation)
//  _p          :   subcarrier allocation array, [size: _M x 1]
//  _M          :   number of subcarriers
//  _M_null     :   output number of null subcarriers
//  _M_pilot    :   output number of pilot subcarriers
//  _M_data     :   output number of data subcarriers
int ofdmframe_validate_sctype(unsigned char * _p,
                              unsigned int _M,
                              unsigned int * _M_null,
                              unsigned int * _M_pilot,
                              unsigned int * _M_data);

// print subcarrier allocation to screen
//  _p      :   output subcarrier allocation array, [size: _M x 1]
//  _M      :   number of subcarriers
int ofdmframe_print_sctype(unsigned char * _p,
                           unsigned int    _M);


//
// OFDM frame (symbol) generator
//
typedef struct ofdmframegen_s * ofdmframegen;

// create OFDM framing generator object
//  _M          :   number of subcarriers, >10 typical
//  _cp_len     :   cyclic prefix length
//  _taper_len  :   taper length (OFDM symbol overlap)
//  _p          :   subcarrier allocation (null, pilot, data), [size: _M x 1]
ofdmframegen ofdmframegen_create(unsigned int    _M,
                                 unsigned int    _cp_len,
                                 unsigned int    _taper_len,
                                 unsigned char * _p);

int ofdmframegen_destroy(ofdmframegen _q);

int ofdmframegen_print(ofdmframegen _q);

int ofdmframegen_reset(ofdmframegen _q);

// write first S0 symbol
int ofdmframegen_write_S0a(ofdmframegen _q,
                           liquid_float_complex *_y);

// write second S0 symbol
int ofdmframegen_write_S0b(ofdmframegen _q,
                           liquid_float_complex *_y);

// write S1 symbol
int ofdmframegen_write_S1(ofdmframegen _q,
                          liquid_float_complex *_y);

// write data symbol
int ofdmframegen_writesymbol(ofdmframegen _q,
                             liquid_float_complex * _x,
                             liquid_float_complex *_y);

// write tail
int ofdmframegen_writetail(ofdmframegen _q,
                           liquid_float_complex * _x);

//
// OFDM frame (symbol) synchronizer
//
typedef int (*ofdmframesync_callback)(liquid_float_complex * _y,
                                      unsigned char * _p,
                                      unsigned int _M,
                                      void * _userdata);
typedef struct ofdmframesync_s * ofdmframesync;

// create OFDM framing synchronizer object
//  _M          :   number of subcarriers, >10 typical
//  _cp_len     :   cyclic prefix length
//  _taper_len  :   taper length (OFDM symbol overlap)
//  _p          :   subcarrier allocation (null, pilot, data), [size: _M x 1]
//  _callback   :   user-defined callback function
//  _userdata   :   user-defined data pointer
ofdmframesync ofdmframesync_create(unsigned int           _M,
                                   unsigned int           _cp_len,
                                   unsigned int           _taper_len,
                                   unsigned char *        _p,
                                   ofdmframesync_callback _callback,
                                   void *                 _userdata);
int ofdmframesync_destroy(ofdmframesync _q);
int ofdmframesync_print(ofdmframesync _q);
int ofdmframesync_reset(ofdmframesync _q);
int ofdmframesync_is_frame_open(ofdmframesync _q);
int ofdmframesync_execute(ofdmframesync _q,
                          liquid_float_complex * _x,
                          unsigned int _n);

// query methods
float ofdmframesync_get_rssi(ofdmframesync _q); // received signal strength indication
float ofdmframesync_get_cfo(ofdmframesync _q);  // carrier offset estimate

// set methods
int ofdmframesync_set_cfo(ofdmframesync _q, float _cfo);  // set carrier offset estimate

// debugging
int ofdmframesync_debug_enable(ofdmframesync _q);
int ofdmframesync_debug_disable(ofdmframesync _q);
int ofdmframesync_debug_print(ofdmframesync _q, const char * _filename);


//
// MODULE : nco (numerically-controlled oscillator)
//

// oscillator type
typedef enum {
    // numerically-controlled oscillator (fastest in all aspects)
    LIQUID_NCO=0,

    // "voltage"-controlled oscillator (precise)
    LIQUID_VCO,
    LIQUID_VCO_INTERP=LIQUID_VCO,

    // fast at computing output but having separate limited API,
    // and each time frequency being set it does dynamic (re-)allocating and
    // calculating of lookup table with resulting memory size and speed slowdown
    // proportional to divider coefficient value)
    LIQUID_VCO_DIRECT
} liquid_ncotype;

#define LIQUID_NCO_MANGLE_FLOAT(name) LIQUID_CONCAT(nco_crcf, name)

// large macro
//   NCO    : name-mangling macro
//   T      : primitive data type
//   TC     : input/output data type
#define LIQUID_NCO_DEFINE_API(NCO,T,TC)                                     \
                                                                            \
/* Numerically-controlled oscillator object                             */  \
typedef struct NCO(_s) * NCO();                                             \
                                                                            \
/* Create nco object with either fixed-point or floating-point phase    */  \
/*  _type   : oscillator type                                           */  \
NCO() NCO(_create)(liquid_ncotype _type);                                   \
                                                                            \
/* Copy object including all internal objects and state                 */  \
NCO() NCO(_copy)(NCO() _q);                                                 \
                                                                            \
/* Destroy nco object, freeing all internally allocated memory          */  \
int NCO(_destroy)(NCO() _q);                                                \
                                                                            \
/* Print nco object internals to stdout                                 */  \
int NCO(_print)(NCO() _q);                                                  \
                                                                            \
/* Set phase/frequency to zero and reset the phase-locked loop filter   */  \
/* state                                                                */  \
int NCO(_reset)(NCO() _q);                                                  \
                                                                            \
/* Get frequency of nco object in radians per sample                    */  \
T NCO(_get_frequency)(NCO() _q);                                            \
                                                                            \
/* Set frequency of nco object in radians per sample                    */  \
/*  _q      : nco object                                                */  \
/*  _dtheta : input frequency [radians/sample]                          */  \
int NCO(_set_frequency)(NCO() _q,                                           \
                       T      _dtheta);                                     \
                                                                            \
/* Adjust frequency of nco object by a step size in radians per sample  */  \
/*  _q      : nco object                                                */  \
/*  _step   : input frequency step [radians/sample]                     */  \
int NCO(_adjust_frequency)(NCO() _q,                                        \
                           T     _step);                                    \
                                                                            \
/* Get phase of nco object in radians                                   */  \
T NCO(_get_phase)(NCO() _q);                                                \
                                                                            \
/* Set phase of nco object in radians                                   */  \
/*  _q      : nco object                                                */  \
/*  _phi    : input phase of nco object [radians]                       */  \
int NCO(_set_phase)(NCO() _q,                                               \
                    T     _phi);                                            \
                                                                            \
/* Adjust phase of nco object by a step of \(\Delta \phi\) radians      */  \
/*  _q      : nco object                                                */  \
/*  _dphi   : input nco object phase adjustment [radians]               */  \
int NCO(_adjust_phase)(NCO() _q,                                            \
                       T     _dphi);                                        \
                                                                            \
/* Get frequency of nco object as fraction of sample rate (n/m)         */  \
/*  _q      : nco object                                                */  \
/*  _n      : pointer to output multiplier coefficient (normalized)     */  \
/*  _m      : pointer to output divider coefficient (normalized)        */  \
void NCO(_get_vcodirect_frequency)(NCO()         _q,                        \
                                   int*          _n,                        \
                                   unsigned int* _m);                       \
                                                                            \
/* Set frequency of nco object as fraction of sample rate (n/m)         */  \
/*  _q      : nco object                                                */  \
/*  _n      : input multiplier coefficient                              */  \
/*  _m      : input divider coefficient                                 */  \
void NCO(_set_vcodirect_frequency)(NCO()        _q,                         \
                                   int          _n,                         \
                                   unsigned int _m);                        \
                                                                            \
/* Increment phase by internal phase step (frequency)                   */  \
int NCO(_step)(NCO() _q);                                                   \
                                                                            \
/* Compute sine output given internal phase                             */  \
T NCO(_sin)(NCO() _q);                                                      \
                                                                            \
/* Compute cosine output given internal phase                           */  \
T NCO(_cos)(NCO() _q);                                                      \
                                                                            \
/* Compute sine and cosine outputs given internal phase                 */  \
/*  _q      : nco object                                                */  \
/*  _s      : output sine component of phase                            */  \
/*  _c      : output cosine component of phase                          */  \
int NCO(_sincos)(NCO() _q,                                                  \
                 T *   _s,                                                  \
                 T *   _c);                                                 \
                                                                            \
/* Compute complex exponential output given internal phase              */  \
/*  _q      : nco object                                                */  \
/*  _y      : output complex exponential                                */  \
int NCO(_cexpf)(NCO() _q,                                                   \
                TC *  _y);                                                  \
                                                                            \
/* Set bandwidth of internal phase-locked loop                          */  \
/*  _q      : nco object                                                */  \
/*  _bw     : input phase-locked loop bandwidth, _bw >= 0               */  \
int NCO(_pll_set_bandwidth)(NCO() _q,                                       \
                            T     _bw);                                     \
                                                                            \
/* Step internal phase-locked loop given input phase error, adjusting   */  \
/* internal phase and frequency proportional to coefficients defined by */  \
/* internal PLL bandwidth                                               */  \
/*  _q      : nco object                                                */  \
/*  _dphi   : input phase-locked loop phase error                       */  \
int NCO(_pll_step)(NCO() _q,                                                \
                   T     _dphi);                                            \
                                                                            \
/* Rotate input sample up by nco angle.                                 */  \
/* Note that this does not adjust the internal phase or frequency.      */  \
/*  _q      : nco object                                                */  \
/*  _x      : input complex sample                                      */  \
/*  _y      : pointer to output sample location                         */  \
int NCO(_mix_up)(NCO() _q,                                                  \
                 TC    _x,                                                  \
                 TC *  _y);                                                 \
                                                                            \
/* Rotate input sample down by nco angle.                               */  \
/* Note that this does not adjust the internal phase or frequency.      */  \
/*  _q      : nco object                                                */  \
/*  _x      : input complex sample                                      */  \
/*  _y      : pointer to output sample location                         */  \
int NCO(_mix_down)(NCO() _q,                                                \
                   TC    _x,                                                \
                   TC *  _y);                                               \
                                                                            \
/* Rotate input vector up by NCO angle (stepping)                       */  \
/* Note that this *does* adjust the internal phase as the signal steps  */  \
/* through each input sample.                                           */  \
/*  _q      : nco object                                                */  \
/*  _x      : array of input samples,  [size: _n x 1]                   */  \
/*  _y      : array of output samples, [size: _n x 1]                   */  \
/*  _n      : number of input (and output) samples                      */  \
int NCO(_mix_block_up)(NCO()        _q,                                     \
                       TC *         _x,                                     \
                       TC *         _y,                                     \
                       unsigned int _n);                                    \
                                                                            \
/* Rotate input vector down by NCO angle (stepping)                     */  \
/* Note that this *does* adjust the internal phase as the signal steps  */  \
/* through each input sample.                                           */  \
/*  _q      : nco object                                                */  \
/*  _x      : array of input samples,  [size: _n x 1]                   */  \
/*  _y      : array of output samples, [size: _n x 1]                   */  \
/*  _n      : number of input (and output) samples                      */  \
int NCO(_mix_block_down)(NCO()        _q,                                   \
                         TC *         _x,                                   \
                         TC *         _y,                                   \
                         unsigned int _n);                                  \

// Define nco APIs
LIQUID_NCO_DEFINE_API(LIQUID_NCO_MANGLE_FLOAT, float, liquid_float_complex)


// nco utilities

// unwrap phase of array (basic)
void liquid_unwrap_phase(float * _theta, unsigned int _n);

// unwrap phase of array (advanced)
void liquid_unwrap_phase2(float * _theta, unsigned int _n);

#define SYNTH_MANGLE_FLOAT(name)  LIQUID_CONCAT(synth_crcf, name)

// large macro
//   SYNTH  : name-mangling macro
//   T      : primitive data type
//   TC     : input/output data type
#define LIQUID_SYNTH_DEFINE_API(SYNTH,T,TC)                                 \
                                                                            \
/* Numerically-controlled synthesizer (direct digital synthesis)        */  \
/* with internal phase-locked loop (pll) implementation                 */  \
typedef struct SYNTH(_s) * SYNTH();                                         \
                                                                            \
SYNTH() SYNTH(_create)(const TC *_table, unsigned int _length);             \
void SYNTH(_destroy)(SYNTH() _q);                                           \
                                                                            \
void SYNTH(_reset)(SYNTH() _q);                                             \
                                                                            \
/* get/set/adjust internal frequency/phase              */                  \
T    SYNTH(_get_frequency)(   SYNTH() _q);                                  \
void SYNTH(_set_frequency)(   SYNTH() _q, T _f);                            \
void SYNTH(_adjust_frequency)(SYNTH() _q, T _df);                           \
T    SYNTH(_get_phase)(       SYNTH() _q);                                  \
void SYNTH(_set_phase)(       SYNTH() _q, T _phi);                          \
void SYNTH(_adjust_phase)(    SYNTH() _q, T _dphi);                         \
                                                                            \
unsigned int SYNTH(_get_length)(SYNTH() _q);                                \
TC SYNTH(_get_current)(SYNTH() _q);                                         \
TC SYNTH(_get_half_previous)(SYNTH() _q);                                   \
TC SYNTH(_get_half_next)(SYNTH() _q);                                       \
                                                                            \
void SYNTH(_step)(SYNTH() _q);                                              \
                                                                            \
/* pll : phase-locked loop                              */                  \
void SYNTH(_pll_set_bandwidth)(SYNTH() _q, T _bandwidth);                   \
void SYNTH(_pll_step)(SYNTH() _q, T _dphi);                                 \
                                                                            \
/* Rotate input sample up by SYNTH angle (no stepping)    */                \
void SYNTH(_mix_up)(SYNTH() _q, TC _x, TC *_y);                             \
                                                                            \
/* Rotate input sample down by SYNTH angle (no stepping)  */                \
void SYNTH(_mix_down)(SYNTH() _q, TC _x, TC *_y);                           \
                                                                            \
/* Rotate input vector up by SYNTH angle (stepping)       */                \
void SYNTH(_mix_block_up)(SYNTH() _q,                                       \
                          TC *_x,                                           \
                          TC *_y,                                           \
                          unsigned int _N);                                 \
                                                                            \
/* Rotate input vector down by SYNTH angle (stepping)     */                \
void SYNTH(_mix_block_down)(SYNTH() _q,                                     \
                            TC *_x,                                         \
                            TC *_y,                                         \
                            unsigned int _N);                               \
                                                                            \
void SYNTH(_spread)(SYNTH() _q,                                             \
                    TC _x,                                                  \
                    TC *_y);                                                \
                                                                            \
void SYNTH(_despread)(SYNTH() _q,                                           \
                      TC *_x,                                               \
                      TC *_y);                                              \
                                                                            \
void SYNTH(_despread_triple)(SYNTH() _q,                                    \
                             TC *_x,                                        \
                             TC *_early,                                    \
                             TC *_punctual,                                 \
                             TC *_late);                                    \

// Define synth APIs
LIQUID_SYNTH_DEFINE_API(SYNTH_MANGLE_FLOAT, float, liquid_float_complex)



//
// MODULE : optimization
//

// utility function pointer definition
typedef float (*utility_function)(void *       _userdata,
                                  float *      _v,
                                  unsigned int _n);

// One-dimensional utility function pointer definition
typedef float (*liquid_utility_1d)(float  _v,
                                   void * _userdata);

// n-dimensional Rosenbrock utility function (minimum at _v = {1,1,1...}
//  _userdata   :   user-defined data structure (convenience)
//  _v          :   input vector, [size: _n x 1]
//  _n          :   input vector size
float liquid_rosenbrock(void *       _userdata,
                        float *      _v,
                        unsigned int _n);

// n-dimensional inverse Gauss utility function (minimum at _v = {0,0,0...}
//  _userdata   :   user-defined data structure (convenience)
//  _v          :   input vector, [size: _n x 1]
//  _n          :   input vector size
float liquid_invgauss(void *       _userdata,
                      float *      _v,
                      unsigned int _n);

// n-dimensional multimodal utility function (minimum at _v = {0,0,0...}
//  _userdata   :   user-defined data structure (convenience)
//  _v          :   input vector, [size: _n x 1]
//  _n          :   input vector size
float liquid_multimodal(void *       _userdata,
                        float *      _v,
                        unsigned int _n);

// n-dimensional spiral utility function (minimum at _v = {0,0,0...}
//  _userdata   :   user-defined data structure (convenience)
//  _v          :   input vector, [size: _n x 1]
//  _n          :   input vector size
float liquid_spiral(void *       _userdata,
                    float *      _v,
                    unsigned int _n);


//
// Gradient search
//

#define LIQUID_OPTIM_MINIMIZE (0)
#define LIQUID_OPTIM_MAXIMIZE (1)

typedef struct gradsearch_s * gradsearch;

// Create a gradient search object
//   _userdata          :   user data object pointer
//   _v                 :   array of parameters to optimize
//   _num_parameters    :   array length (number of parameters to optimize)
//   _u                 :   utility function pointer
//   _direction         :   search direction (e.g. LIQUID_OPTIM_MAXIMIZE)
gradsearch gradsearch_create(void *           _userdata,
                             float *          _v,
                             unsigned int     _num_parameters,
                             utility_function _utility,
                             int              _direction);

// Destroy a gradsearch object
void gradsearch_destroy(gradsearch _q);

// Prints current status of search
void gradsearch_print(gradsearch _q);

// Iterate once
float gradsearch_step(gradsearch _q);

// Execute the search
float gradsearch_execute(gradsearch   _q,
                         unsigned int _max_iterations,
                         float        _target_utility);


// Quadsection search in one dimension...
//  
//    
//    * yn
//                                * yp
//           *
//                  * y0  
//                         *
//    +------+------+------+------+-->
//   [xn            x0            xp]
typedef struct qs1dsearch_s * qs1dsearch;
qs1dsearch qs1dsearch_create(liquid_utility_1d _u,
                             void *            _userdata,
                             int               _direction);

int          qs1dsearch_destroy       (qs1dsearch _q);
qs1dsearch   qs1dsearch_copy          (qs1dsearch _q);
int          qs1dsearch_print         (qs1dsearch _q);
int          qs1dsearch_reset         (qs1dsearch _q);
int          qs1dsearch_init          (qs1dsearch _q, float _v0);
int          qs1dsearch_init_direction(qs1dsearch _q, float _v_init, float _step);
int          qs1dsearch_init_bounds   (qs1dsearch _q, float _vn, float _vp);
int          qs1dsearch_step          (qs1dsearch _q);
int          qs1dsearch_execute       (qs1dsearch _q);
unsigned int qs1dsearch_get_num_steps (qs1dsearch _q);
float        qs1dsearch_get_opt_v     (qs1dsearch _q);
float        qs1dsearch_get_opt_u     (qs1dsearch _q);

// quasi-Newton search
typedef struct qnsearch_s * qnsearch;

// Create a simple qnsearch object; parameters are specified internally
//   _userdata          :   userdata
//   _v                 :   array of parameters to optimize
//   _num_parameters    :   array length
//   _get_utility       :   utility function pointer
//   _direction         :   search direction (e.g. LIQUID_OPTIM_MAXIMIZE)
qnsearch qnsearch_create(void *           _userdata,
                         float *          _v,
                         unsigned int     _num_parameters,
                         utility_function _u,
                         int              _direction);

// Destroy a qnsearch object
int qnsearch_destroy(qnsearch _g);

// Prints current status of search
int qnsearch_print(qnsearch _g);

// Resets internal state
int qnsearch_reset(qnsearch _g);

// Iterate once
int qnsearch_step(qnsearch _g);

// Execute the search
float qnsearch_execute(qnsearch _g,
                       unsigned int _max_iterations,
                       float _target_utility);

//
// chromosome (for genetic algorithm search)
//
typedef struct chromosome_s * chromosome;

// create a chromosome object, variable bits/trait
chromosome chromosome_create(unsigned int * _bits_per_trait,
                             unsigned int _num_traits);

// create a chromosome object, all traits same resolution
chromosome chromosome_create_basic(unsigned int _num_traits,
                                   unsigned int _bits_per_trait);

// create a chromosome object, cloning a parent
chromosome chromosome_create_clone(chromosome _parent);

// copy existing chromosomes' internal traits (all other internal
// parameters must be equal)
int chromosome_copy(chromosome _parent, chromosome _child);

// Destroy a chromosome object
int chromosome_destroy(chromosome _c);

// get number of traits in chromosome
unsigned int chromosome_get_num_traits(chromosome _c);

// Print chromosome values to screen (binary representation)
int chromosome_print(chromosome _c);

// Print chromosome values to screen (floating-point representation)
int chromosome_printf(chromosome _c);

// clear chromosome (set traits to zero)
int chromosome_reset(chromosome _c);

// initialize chromosome on integer values
int chromosome_init(chromosome _c,
                     unsigned int * _v);

// initialize chromosome on floating-point values
int chromosome_initf(chromosome _c, float * _v);

// Mutates chromosome _c at _index
int chromosome_mutate(chromosome _c, unsigned int _index);

// Resulting chromosome _c is a crossover of parents _p1 and _p2 at _threshold
int chromosome_crossover(chromosome   _p1,
                         chromosome   _p2,
                         chromosome   _c,
                         unsigned int _threshold);

// Initializes chromosome to random value
int chromosome_init_random(chromosome _c);

// Returns integer representation of chromosome
unsigned int chromosome_value(chromosome    _c,
                               unsigned int _index);

// Returns floating-point representation of chromosome
float chromosome_valuef(chromosome _c,
                        unsigned int _index);

//
// genetic algorithm search
//
typedef struct gasearch_s * gasearch;

typedef float (*gasearch_utility)(void * _userdata, chromosome _c);

// Create a simple gasearch object; parameters are specified internally
//  _utility            :   chromosome fitness utility function
//  _userdata           :   user data, void pointer passed to _get_utility() callback
//  _parent             :   initial population parent chromosome, governs precision, etc.
//  _minmax             :   search direction
gasearch gasearch_create(gasearch_utility _u,
                         void * _userdata,
                         chromosome _parent,
                         int _minmax);

// Create a gasearch object, specifying search parameters
//  _utility            :   chromosome fitness utility function
//  _userdata           :   user data, void pointer passed to _get_utility() callback
//  _parent             :   initial population parent chromosome, governs precision, etc.
//  _minmax             :   search direction
//  _population_size    :   number of chromosomes in population
//  _mutation_rate      :   probability of mutating chromosomes
gasearch gasearch_create_advanced(gasearch_utility _utility,
                                  void * _userdata,
                                  chromosome _parent,
                                  int _minmax,
                                  unsigned int _population_size,
                                  float _mutation_rate);


// Destroy a gasearch object
int gasearch_destroy(gasearch _q);

// print search parameter internals
int gasearch_print(gasearch _q);

// set mutation rate
int gasearch_set_mutation_rate(gasearch _q,
                               float    _mutation_rate);

// set population/selection size
//  _q                  :   ga search object
//  _population_size    :   new population size (number of chromosomes)
//  _selection_size     :   selection size (number of parents for new generation)
int gasearch_set_population_size(gasearch     _q,
                                 unsigned int _population_size,
                                 unsigned int _selection_size);

// Execute the search
//  _q              :   ga search object
//  _max_iterations :   maximum number of iterations to run before bailing
//  _target_utility :   target utility
float gasearch_run(gasearch     _q,
                   unsigned int _max_iterations,
                   float        _target_utility);

// iterate over one evolution of the search algorithm
int gasearch_evolve(gasearch _q);

// get optimal chromosome
//  _q              :   ga search object
//  _c              :   output optimal chromosome
//  _utility_opt    :   fitness of _c
int gasearch_getopt(gasearch   _q,
                    chromosome _c,
                    float *    _utility_opt);

//
// MODULE : quantization
//

float compress_mulaw(float _x, float _mu);
float expand_mulaw(float _x, float _mu);

int compress_cf_mulaw(liquid_float_complex _x, float _mu, liquid_float_complex * _y);
int expand_cf_mulaw(liquid_float_complex _y, float _mu, liquid_float_complex * _x);

//float compress_alaw(float _x, float _a);
//float expand_alaw(float _x, float _a);

// inline quantizer: 'analog' signal in [-1, 1]
unsigned int quantize_adc(float _x, unsigned int _num_bits);
float quantize_dac(unsigned int _s, unsigned int _num_bits);

// structured quantizer

typedef enum {
    LIQUID_COMPANDER_NONE=0,
    LIQUID_COMPANDER_LINEAR,
    LIQUID_COMPANDER_MULAW,
    LIQUID_COMPANDER_ALAW
} liquid_compander_type;

#define LIQUID_QUANTIZER_MANGLE_FLOAT(name)  LIQUID_CONCAT(quantizerf,  name)
#define LIQUID_QUANTIZER_MANGLE_CFLOAT(name) LIQUID_CONCAT(quantizercf, name)

// large macro
//   QUANTIZER  : name-mangling macro
//   T          : data type
#define LIQUID_QUANTIZER_DEFINE_API(QUANTIZER,T)                            \
                                                                            \
/* Amplitude quantization object                                        */  \
typedef struct QUANTIZER(_s) * QUANTIZER();                                 \
                                                                            \
/* Create quantizer object given compander type, input range, and the   */  \
/* number of bits to represent the output                               */  \
/*  _ctype      : compander type (linear, mulaw, alaw)                  */  \
/*  _range      : maximum abosolute input range (ignored for now)       */  \
/*  _num_bits   : number of bits per sample                             */  \
QUANTIZER() QUANTIZER(_create)(liquid_compander_type _ctype,                \
                               float                 _range,                \
                               unsigned int          _num_bits);            \
                                                                            \
/* Destroy object, freeing all internally-allocated memory.             */  \
int QUANTIZER(_destroy)(QUANTIZER() _q);                                    \
                                                                            \
/* Print object properties to stdout, including compander type and      */  \
/* number of bits per sample                                            */  \
int QUANTIZER(_print)(QUANTIZER() _q);                                      \
                                                                            \
/* Execute quantizer as analog-to-digital converter, accepting input    */  \
/* sample and returning digitized output bits                           */  \
/*  _q  : quantizer object                                              */  \
/*  _x  : input sample                                                  */  \
/*  _s  : output bits                                                   */  \
int QUANTIZER(_execute_adc)(QUANTIZER()    _q,                              \
                            T              _x,                              \
                            unsigned int * _s);                             \
                                                                            \
/* Execute quantizer as digital-to-analog converter, accepting input    */  \
/* bits and returning representation of original input sample           */  \
/*  _q  : quantizer object                                              */  \
/*  _s  : input bits                                                    */  \
/*  _x  : output sample                                                 */  \
int QUANTIZER(_execute_dac)(QUANTIZER()  _q,                                \
                            unsigned int _s,                                \
                            T *          _x);                               \

LIQUID_QUANTIZER_DEFINE_API(LIQUID_QUANTIZER_MANGLE_FLOAT,  float)
LIQUID_QUANTIZER_DEFINE_API(LIQUID_QUANTIZER_MANGLE_CFLOAT, liquid_float_complex)


//
// MODULE : random (number generators)
//


// Uniform random number generator, [0,1)
float randf();
float randf_pdf(float _x);
float randf_cdf(float _x);

// Uniform random number generator with arbitrary bounds, [a,b)
float randuf(float _a, float _b);
float randuf_pdf(float _x, float _a, float _b);
float randuf_cdf(float _x, float _a, float _b);

// Gauss random number generator, N(0,1)
//   f(x) = 1/sqrt(2*pi*sigma^2) * exp{-(x-eta)^2/(2*sigma^2)}
//
//   where
//     eta   = mean
//     sigma = standard deviation
//
float randnf();
void awgn(float *_x, float _nstd);
void crandnf(liquid_float_complex *_y);
void cawgn(liquid_float_complex *_x, float _nstd);
float randnf_pdf(float _x, float _eta, float _sig);
float randnf_cdf(float _x, float _eta, float _sig);

// Exponential
//  f(x) = lambda exp{ -lambda x }
// where
//  lambda = spread parameter, lambda > 0
//  x >= 0
float randexpf(float _lambda);
float randexpf_pdf(float _x, float _lambda);
float randexpf_cdf(float _x, float _lambda);

// Weibull
//   f(x) = (a/b) (x/b)^(a-1) exp{ -(x/b)^a }
//   where
//     a = alpha : shape parameter
//     b = beta  : scaling parameter
//     g = gamma : location (threshold) parameter
//
float randweibf(float _alpha, float _beta, float _gamma);
float randweibf_pdf(float _x, float _a, float _b, float _g);
float randweibf_cdf(float _x, float _a, float _b, float _g);

// Gamma
//          x^(a-1) exp(-x/b)
//  f(x) = -------------------
//            Gamma(a) b^a
//  where
//      a = alpha : shape parameter, a > 0
//      b = beta  : scale parameter, b > 0
//      Gamma(z) = regular gamma function
//      x >= 0
float randgammaf(float _alpha, float _beta);
float randgammaf_pdf(float _x, float _alpha, float _beta);
float randgammaf_cdf(float _x, float _alpha, float _beta);

// Nakagami-m
//  f(x) = (2/Gamma(m)) (m/omega)^m x^(2m-1) exp{-(m/omega)x^2}
// where
//      m       : shape parameter, m >= 0.5
//      omega   : spread parameter, omega > 0
//      Gamma(z): regular complete gamma function
//      x >= 0
float randnakmf(float _m, float _omega);
float randnakmf_pdf(float _x, float _m, float _omega);
float randnakmf_cdf(float _x, float _m, float _omega);

// Rice-K
//  f(x) = (x/sigma^2) exp{ -(x^2+s^2)/(2sigma^2) } I0( x s / sigma^2 )
// where
//  s     = sqrt( omega*K/(K+1) )
//  sigma = sqrt(0.5 omega/(K+1))
// and
//  K     = shape parameter
//  omega = spread parameter
//  I0    = modified Bessel function of the first kind
//  x >= 0
float randricekf(float _K, float _omega);
float randricekf_cdf(float _x, float _K, float _omega);
float randricekf_pdf(float _x, float _K, float _omega);


// Data scrambler : whiten data sequence
void scramble_data(unsigned char * _x, unsigned int _len);
void unscramble_data(unsigned char * _x, unsigned int _len);
void unscramble_data_soft(unsigned char * _x, unsigned int _len);

//
// MODULE : sequence
//

// Binary sequence (generic)

typedef struct bsequence_s * bsequence;

// Create a binary sequence of a specific length (number of bits)
bsequence bsequence_create(unsigned int num_bits);

// Free memory in a binary sequence
int bsequence_destroy(bsequence _bs);

// Clear binary sequence (set to 0's)
int bsequence_reset(bsequence _bs);

// initialize sequence on external array
int bsequence_init(bsequence       _bs,
                   unsigned char * _v);

// Print sequence to the screen
int bsequence_print(bsequence _bs);

// Push bit into to back of a binary sequence
int bsequence_push(bsequence    _bs,
                   unsigned int _bit);

// circular shift (left)
int bsequence_circshift(bsequence _bs);

// Correlate two binary sequences together
int bsequence_correlate(bsequence _bs1, bsequence _bs2);

// compute the binary addition of two bit sequences
int bsequence_add(bsequence _bs1, bsequence _bs2, bsequence _bs3);

// compute the binary multiplication of two bit sequences
int bsequence_mul(bsequence _bs1, bsequence _bs2, bsequence _bs3);

// accumulate the 1's in a binary sequence
unsigned int bsequence_accumulate(bsequence _bs);

// accessor functions
unsigned int bsequence_get_length(bsequence _bs);
unsigned int bsequence_index(bsequence _bs, unsigned int _i);

// Complementary codes

// initialize two sequences to complementary codes.  sequences must
// be of length at least 8 and a power of 2 (e.g. 8, 16, 32, 64,...)
//  _a      :   sequence 'a' (bsequence object)
//  _b      :   sequence 'b' (bsequence object)
int bsequence_create_ccodes(bsequence _a, bsequence _b);


// M-Sequence

// default m-sequence generators:       g (hex)         m   n
#define LIQUID_MSEQUENCE_GENPOLY_M2     0x00000003  //  2   3
#define LIQUID_MSEQUENCE_GENPOLY_M3     0x00000006  //  3   7
#define LIQUID_MSEQUENCE_GENPOLY_M4     0x0000000c  //  4   15
#define LIQUID_MSEQUENCE_GENPOLY_M5     0x00000014  //  5   31
#define LIQUID_MSEQUENCE_GENPOLY_M6     0x00000030  //  6   63
#define LIQUID_MSEQUENCE_GENPOLY_M7     0x00000060  //  7   127
#define LIQUID_MSEQUENCE_GENPOLY_M8     0x000000b8  //  8   255
#define LIQUID_MSEQUENCE_GENPOLY_M9     0x00000110  //  9   511
#define LIQUID_MSEQUENCE_GENPOLY_M10    0x00000240  // 10   1,023
#define LIQUID_MSEQUENCE_GENPOLY_M11    0x00000500  // 11   2,047
#define LIQUID_MSEQUENCE_GENPOLY_M12    0x00000e08  // 12   4,095
#define LIQUID_MSEQUENCE_GENPOLY_M13    0x00001c80  // 13   8,191
#define LIQUID_MSEQUENCE_GENPOLY_M14    0x00003802  // 14   16,383
#define LIQUID_MSEQUENCE_GENPOLY_M15    0x00006000  // 15   32,767
#define LIQUID_MSEQUENCE_GENPOLY_M16    0x0000d008  // 16   65,535
#define LIQUID_MSEQUENCE_GENPOLY_M17    0x00012000  // 17   131,071
#define LIQUID_MSEQUENCE_GENPOLY_M18    0x00020400  // 18   262,143
#define LIQUID_MSEQUENCE_GENPOLY_M19    0x00072000  // 19   524,287
#define LIQUID_MSEQUENCE_GENPOLY_M20    0x00090000  // 20   1,048,575
#define LIQUID_MSEQUENCE_GENPOLY_M21    0x00140000  // 21   2,097,151
#define LIQUID_MSEQUENCE_GENPOLY_M22    0x00300000  // 22   4,194,303
#define LIQUID_MSEQUENCE_GENPOLY_M23    0x00420000  // 23   8,388,607
#define LIQUID_MSEQUENCE_GENPOLY_M24    0x00e10000  // 24   16,777,215
#define LIQUID_MSEQUENCE_GENPOLY_M25    0x01000004  // 25   33,554,431
#define LIQUID_MSEQUENCE_GENPOLY_M26    0x02000023  // 26   67,108,863
#define LIQUID_MSEQUENCE_GENPOLY_M27    0x04000013  // 27   134,217,727
#define LIQUID_MSEQUENCE_GENPOLY_M28    0x08000004  // 28   268,435,455
#define LIQUID_MSEQUENCE_GENPOLY_M29    0x10000002  // 29   536,870,911
#define LIQUID_MSEQUENCE_GENPOLY_M30    0x20000029  // 30   1,073,741,823
#define LIQUID_MSEQUENCE_GENPOLY_M31    0x40000004  // 31   2,147,483,647

typedef struct msequence_s * msequence;

// create a maximal-length sequence (m-sequence) object with
// an internal shift register length of _m bits.
//  _m      :   generator polynomial length, sequence length is (2^m)-1
//  _g      :   generator polynomial, starting with most-significant bit
//  _a      :   initial shift register state, default: 000...001
msequence msequence_create(unsigned int _m,
                           unsigned int _g,
                           unsigned int _a);

// Copy maximal-length sequence (m-sequence) object
msequence msequence_copy(msequence q_orig);

// create a maximal-length sequence (m-sequence) object from a generator polynomial
msequence msequence_create_genpoly(unsigned int _g);

// creates a default maximal-length sequence
msequence msequence_create_default(unsigned int _m);

// destroy an msequence object, freeing all internal memory
int msequence_destroy(msequence _m);

// prints the sequence's internal state to the screen
int msequence_print(msequence _m);

// advance msequence on shift register, returning output bit
unsigned int msequence_advance(msequence _ms);

// generate pseudo-random symbol from shift register by
// advancing _bps bits and returning compacted symbol
//  _ms     :   m-sequence object
//  _bps    :   bits per symbol of output
unsigned int msequence_generate_symbol(msequence    _ms,
                                       unsigned int _bps);

// reset msequence shift register to original state, typically '1'
int msequence_reset(msequence _ms);

// initialize a bsequence object on an msequence object
//  _bs     :   bsequence object
//  _ms     :   msequence object
int bsequence_init_msequence(bsequence _bs,
                             msequence _ms);

// get the length of the generator polynomial, g (m)
unsigned int msequence_get_genpoly_length(msequence _ms);

// get the length of the sequence (n=2^m-1)
unsigned int msequence_get_length(msequence _ms);

// get the generator polynomial, g
unsigned int msequence_get_genpoly(msequence _ms);

// get the internal state of the sequence
unsigned int msequence_get_state(msequence _ms);

// set the internal state of the sequence
int msequence_set_state(msequence    _ms,
                        unsigned int _a);

// measure the period the shift register (should be 2^m-1 with a proper generator polynomial)
unsigned int msequence_measure_period(msequence _ms);

// measure the period of a generator polynomial
unsigned int msequence_genpoly_period(unsigned int _g);


//
// MODULE : utility
//

// allocate memory and copy from original location
//  _orig   : pointer to original memory array
//  _num    : number of original elements
//  _size   : size of each element
void * liquid_malloc_copy(void * _orig, unsigned int _num, unsigned int _size);

// pack binary array with symbol(s)
//  _src        :   source array, [size: _n x 1]
//  _n          :   input source array length
//  _k          :   bit index to write in _src
//  _b          :   number of bits in input symbol
//  _sym_in     :   input symbol
int liquid_pack_array(unsigned char * _src,
                      unsigned int _n,
                      unsigned int _k,
                      unsigned int _b,
                      unsigned char _sym_in);

// unpack symbols from binary array
//  _src        :   source array, [size: _n x 1]
//  _n          :   input source array length
//  _k          :   bit index to write in _src
//  _b          :   number of bits in output symbol
//  _sym_out    :   output symbol
int liquid_unpack_array(unsigned char * _src,
                        unsigned int _n,
                        unsigned int _k,
                        unsigned int _b,
                        unsigned char * _sym_out);

// pack one-bit symbols into bytes (8-bit symbols)
//  _sym_in             :   input symbols array, [size: _sym_in_len x 1]
//  _sym_in_len         :   number of input symbols
//  _sym_out            :   output symbols
//  _sym_out_len        :   number of bytes allocated to output symbols array
//  _num_written        :   number of output symbols actually written
int liquid_pack_bytes(unsigned char * _sym_in,
                      unsigned int _sym_in_len,
                      unsigned char * _sym_out,
                      unsigned int _sym_out_len,
                      unsigned int * _num_written);

// unpack 8-bit symbols (full bytes) into one-bit symbols
//  _sym_in             :   input symbols array, [size: _sym_in_len x 1]
//  _sym_in_len         :   number of input symbols
//  _sym_out            :   output symbols array
//  _sym_out_len        :   number of bytes allocated to output symbols array
//  _num_written        :   number of output symbols actually written
int liquid_unpack_bytes(unsigned char * _sym_in,
                        unsigned int _sym_in_len,
                        unsigned char * _sym_out,
                        unsigned int _sym_out_len,
                        unsigned int * _num_written);

// repack bytes with arbitrary symbol sizes
//  _sym_in             :   input symbols array, [size: _sym_in_len x 1]
//  _sym_in_bps         :   number of bits per input symbol
//  _sym_in_len         :   number of input symbols
//  _sym_out            :   output symbols array
//  _sym_out_bps        :   number of bits per output symbol
//  _sym_out_len        :   number of bytes allocated to output symbols array
//  _num_written        :   number of output symbols actually written
int liquid_repack_bytes(unsigned char * _sym_in,
                        unsigned int _sym_in_bps,
                        unsigned int _sym_in_len,
                        unsigned char * _sym_out,
                        unsigned int _sym_out_bps,
                        unsigned int _sym_out_len,
                        unsigned int * _num_written);

// shift array to the left _b bits, filling in zeros
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_lbshift(unsigned char * _src,
                   unsigned int _n,
                   unsigned int _b);

// shift array to the right _b bits, filling in zeros
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_rbshift(unsigned char * _src,
                   unsigned int _n,
                   unsigned int _b);

// circular shift array to the left _b bits
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_lbcircshift(unsigned char * _src,
                       unsigned int _n,
                       unsigned int _b);

// circular shift array to the right _b bits
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_rbcircshift(unsigned char * _src,
                       unsigned int _n,
                       unsigned int _b);

// shift array to the left _b bytes, filling in zeros
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_lshift(unsigned char * _src,
                  unsigned int _n,
                  unsigned int _b);

// shift array to the right _b bytes, filling in zeros
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_rshift(unsigned char * _src,
                  unsigned int _n,
                  unsigned int _b);

// circular shift array to the left _b bytes
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_lcircshift(unsigned char * _src,
                      unsigned int _n,
                      unsigned int _b);

// circular shift array to the right _b bytes
//  _src        :   source address, [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_rcircshift(unsigned char * _src,
                      unsigned int _n,
                      unsigned int _b);

// Count the number of ones in an integer
unsigned int liquid_count_ones(unsigned int _x);

// count number of ones in an integer, modulo 2
unsigned int liquid_count_ones_mod2(unsigned int _x);

// compute bindary dot-product between two integers
unsigned int liquid_bdotprod(unsigned int _x,
                             unsigned int _y);

// Count leading zeros in an integer
unsigned int liquid_count_leading_zeros(unsigned int _x);

// Most-significant bit index
unsigned int liquid_msb_index(unsigned int _x);

// Print string of bits to stdout
int liquid_print_bitstring(unsigned int _x, unsigned int _n);

// reverse byte, word, etc.
unsigned char liquid_reverse_byte(  unsigned char _x);
unsigned int  liquid_reverse_uint16(unsigned int  _x);
unsigned int  liquid_reverse_uint24(unsigned int  _x);
unsigned int  liquid_reverse_uint32(unsigned int  _x);

// get scale for constant, particularly for plotting purposes
//  _val    : input value (e.g. 100e6)
//  _unit   : output unit character (e.g. 'M')
//  _scale  : output scale (e.g. 1e-6)
int liquid_get_scale(float   _val,
                     char *  _unit,
                     float * _scale);

//
// MODULE : vector
//

#define LIQUID_VECTOR_MANGLE_RF(name) LIQUID_CONCAT(liquid_vectorf, name)
#define LIQUID_VECTOR_MANGLE_CF(name) LIQUID_CONCAT(liquid_vectorcf,name)

// large macro
//   VECTOR     : name-mangling macro
//   T          : data type
//   TP         : data type (primitive)
#define LIQUID_VECTOR_DEFINE_API(VECTOR,T,TP)                               \
                                                                            \
/* Initialize vector with scalar: x[i] = c (scalar)                     */  \
void VECTOR(_init)(T            _c,                                         \
                   T *          _x,                                         \
                   unsigned int _n);                                        \
                                                                            \
/* Add each element pointwise: z[i] = x[i] + y[i]                       */  \
void VECTOR(_add)(T *          _x,                                          \
                  T *          _y,                                          \
                  unsigned int _n,                                          \
                  T *          _z);                                         \
                                                                            \
/* Add scalar to each element: y[i] = x[i] + c                          */  \
void VECTOR(_addscalar)(T *          _x,                                    \
                        unsigned int _n,                                    \
                        T            _c,                                    \
                        T *          _y);                                   \
                                                                            \
/* Multiply each element pointwise: z[i] = x[i] * y[i]                  */  \
void VECTOR(_mul)(T *          _x,                                          \
                  T *          _y,                                          \
                  unsigned int _n,                                          \
                  T *          _z);                                         \
                                                                            \
/* Multiply each element with scalar: y[i] = x[i] * c                   */  \
void VECTOR(_mulscalar)(T *          _x,                                    \
                        unsigned int _n,                                    \
                        T            _c,                                    \
                        T *          _y);                                   \
                                                                            \
/* Compute complex phase rotation: x[i] = exp{j theta[i]}               */  \
void VECTOR(_cexpj)(TP *         _theta,                                    \
                    unsigned int _n,                                        \
                    T *          _x);                                       \
                                                                            \
/* Compute angle of each element: theta[i] = arg{ x[i] }                */  \
void VECTOR(_carg)(T *          _x,                                         \
                   unsigned int _n,                                         \
                   TP *         _theta);                                    \
                                                                            \
/* Compute absolute value of each element: y[i] = |x[i]|                */  \
void VECTOR(_abs)(T *          _x,                                          \
                  unsigned int _n,                                          \
                  TP *         _y);                                         \
                                                                            \
/* Compute sum of squares: sum{ |x|^2 }                                 */  \
TP VECTOR(_sumsq)(T *          _x,                                          \
                  unsigned int _n);                                         \
                                                                            \
/* Compute l-2 norm: sqrt{ sum{ |x|^2 } }                               */  \
TP VECTOR(_norm)(T *          _x,                                           \
                 unsigned int _n);                                          \
                                                                            \
/* Compute l-p norm: { sum{ |x|^p } }^(1/p)                             */  \
TP VECTOR(_pnorm)(T *          _x,                                          \
                  unsigned int _n,                                          \
                  TP           _p);                                         \
                                                                            \
/* Scale vector elements by l-2 norm: y[i] = x[i]/norm(x)               */  \
void VECTOR(_normalize)(T *          _x,                                    \
                        unsigned int _n,                                    \
                        T *          _y);                                   \

LIQUID_VECTOR_DEFINE_API(LIQUID_VECTOR_MANGLE_RF, float,                float)
LIQUID_VECTOR_DEFINE_API(LIQUID_VECTOR_MANGLE_CF, liquid_float_complex, float)

//
// mixed types
//
#if 0
void liquid_vectorf_add(float *      _a,
                        float *      _b,
                        unsigned int _n,
                        float *      _c);
#endif

#ifdef __cplusplus
} //extern "C"
#endif // __cplusplus

#endif // __LIQUID_H__

