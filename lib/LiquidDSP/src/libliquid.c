/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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

// Run-time library version numbers, error handling

#include <stdlib.h>
#include <stdio.h>
#include "liquid.internal.h"

const char liquid_version[] = LIQUID_VERSION;

const char * liquid_libversion(void)
{
    return LIQUID_VERSION;
}

int liquid_libversion_number(void)
{
    return LIQUID_VERSION_NUMBER;
}

// report error
int liquid_error_fl(int          _code,
                    const char * _file,
                    int          _line,
                    const char * _format,
                    ...)
{
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    va_list argptr;
    va_start(argptr, _format);
    fprintf(stderr,"error [%d]: %s\n", _code, liquid_error_info(_code));
    fprintf(stderr,"  %s:%u: ", _file, _line);
    vfprintf(stderr, _format, argptr);
    fprintf(stderr,"\n");
    va_end(argptr);
#endif
#if LIQUID_STRICT_EXIT
    exit(_code);
#endif
    return _code;
}

// report error
void * liquid_error_config_fl(const char * _file,
                              int          _line,
                              const char * _format,
                              ...)
{
    int code = LIQUID_EICONFIG;
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    va_list argptr;
    va_start(argptr, _format);
    fprintf(stderr,"error [%d]: %s\n", code, liquid_error_info(code));
    fprintf(stderr,"  %s:%u: ", _file, _line);
    vfprintf(stderr, _format, argptr);
    fprintf(stderr,"\n");
    va_end(argptr);
#endif
#if LIQUID_STRICT_EXIT
    exit(code);
#endif
    return NULL;
}

const char * liquid_error_str[LIQUID_NUM_ERRORS] = {
    "ok",                                                   // LIQUID_OK
    "internal logic error",                                 // LIQUID_EINT
    "invalid object",                                       // LIQUID_EIOBJ,
    "invalid parameter or configuration",                   // LIQUID_EICONFIG
    "input out of range",                                   // LIQUID_EIVAL
    "invalid vector length or dimension",                   // LIQUID_EIRANGE
    "invalid mode",                                         // LIQUID_EIMODE
    "unsupported mode",                                     // LIQUID_EUMODE
    "object has not been created or properly initialized",  // LIQUID_ENOINIT
    "not enough memory allocated for operation",            // LIQUID_EIMEM
    "file input/output",                                    // LIQUID_EIO
    "algorithm could not converge",                         // LIQUID_ENOCONV
    "function or method not yet implemented",               // LIQUID_ENOIMP
};

// get error string given code
const char * liquid_error_info(liquid_error_code _code)
{
    if (_code < 0 || _code >= LIQUID_NUM_ERRORS) {
        liquid_error(LIQUID_EIMODE,"error code %d is out of range", _code);
        return NULL;
    }

    return liquid_error_str[_code];
}

