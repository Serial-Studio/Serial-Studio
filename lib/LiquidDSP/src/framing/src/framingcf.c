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

// complex floating-point framing and signal generators

#include "liquid.internal.h"

// naming extensions (useful for print statements)
#define EXTENSION           "cf"

#define TO                  float complex   // output type
#define T                   float           // primitive type

#define TO_COMPLEX          1
#define T_COMPLEX           0

// supporting references
#define FIRINTERP(name)     LIQUID_CONCAT(firinterp_crcf,name)
#define IIRFILT(name)       LIQUID_CONCAT(iirfilt_crcf, name)
#define MODEM(name)         LIQUID_CONCAT(modemcf,      name)
#define MSRESAMP(name)      LIQUID_CONCAT(msresamp_crcf,name)
#define NCO(name)           LIQUID_CONCAT(nco_crcf,     name)
#define SYMSTREAM(name)     LIQUID_CONCAT(symstreamcf,  name)

// object references
#define MSOURCE(name)       LIQUID_CONCAT(msourcecf,    name)
#define QPACKETMODEM(name)  LIQUID_CONCAT(qpacketmodem, name)
#define QSOURCE(name)       LIQUID_CONCAT(qsourcecf,    name)
#define SYMSTREAM(name)     LIQUID_CONCAT(symstreamcf,  name)
#define SYMSTREAMR(name)    LIQUID_CONCAT(symstreamrcf, name)

// prototypes
#include "msource.proto.c"
#include "qpacketmodem.proto.c"
#include "qsource.proto.c"
#include "symstream.proto.c"
#include "symstreamr.proto.c"

