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
// buffer autotest (static)
//

#include "autotest/autotest.h"
#include "liquid.h"

#define SBUFFER_AUTOTEST_DEFINE_API(X,T)        \
    T v[] = {1, 2, 3, 4, 5, 6, 7, 8};           \
    T test1[] = {1, 2, 3, 4};                   \
    T test2[] = {1, 2, 3, 4, 5, 6, 7, 8};       \
    T test3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; \
    T *r;                                       \
    unsigned int n;                             \
                                                \
    X() cb = X(_create)(STATIC,10);             \
                                                \
    X(_write)(cb, v, 4);                        \
    n = 4;                                      \
    X(_read)(cb, &r, &n);                       \
                                                \
    CONTEND_EQUALITY(n,4);                      \
    CONTEND_SAME_DATA(r,test1,4*sizeof(T));     \
                                                \
    X(_release)(cb, 0);                         \
    X(_write)(cb, v, 8);                        \
    n = 8;                                      \
    X(_read)(cb, &r, &n);                       \
    CONTEND_EQUALITY(n,8);                      \
    CONTEND_SAME_DATA(r,test2,8*sizeof(T));     \
                                                \
    X(_zero)(cb);                               \
    n = 10;                                     \
    X(_read)(cb, &r, &n);                       \
    CONTEND_EQUALITY(n,10);                     \
    CONTEND_SAME_DATA(r,test3,10*sizeof(T));    \
                                                \
    X(_destroy)(cb);


//
// AUTOTEST: static float buffer
//
void autotest_fbuffer_static()
{
    SBUFFER_AUTOTEST_DEFINE_API(BUFFER_MANGLE_FLOAT, float)
}


//
// AUTOTEST: static complex float buffer
//
void autotest_cfbuffer_static()
{
    SBUFFER_AUTOTEST_DEFINE_API(BUFFER_MANGLE_CFLOAT, float complex)
}


#if 0
//
// AUTOTEST: static unsigned int buffer
//
void xautotest_uibuffer_static()
{
    SBUFFER_AUTOTEST_DEFINE_API(BUFFER_MANGLE_UINT, unsigned int)
}
#endif

