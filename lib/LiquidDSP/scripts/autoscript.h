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

#ifndef __LIQUID_AUTOSCRIPT_H__
#define __LIQUID_AUTOSCRIPT_H__

typedef struct autoscript_s * autoscript;

// create autoscript generator object
autoscript autoscript_create(char * _type,
                             char _delim);

// parse file
void autoscript_parse(autoscript _q,
                      char * _filename);

void autoscript_print(autoscript _q);

void autoscript_destroy(autoscript _q);


//
// internal methods
//

// parse filename
//  _q              :   generator object
//  _filename       :   name of file
//  _package_name   :   output package name (stripped filename)
void autoscript_parsefilename(autoscript _q,
                              char * _filename,
                              char * _package_name);

// parse file
//  _q              :   generator object
//  _filename       :   name of file
//  _package_name   :   input package name (stripped filename)
void autoscript_parsefile(autoscript _q,
                          char * _filename,
                          char * _package_name);

void autoscript_addpackage(autoscript _q,
                           char * _package_name,
                           char * _filename);

void autoscript_addscript(autoscript _q,
                          char * _package_name,
                          char * _script_name);

#endif // __LIQUID_AUTOSCRIPT_H__

