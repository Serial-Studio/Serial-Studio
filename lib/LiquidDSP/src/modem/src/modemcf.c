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

// linear modem API, floating-point (single precision)

#include <math.h>
#include <stdint.h>
#include "liquid.internal.h"

// Macro definitions
#define CPFSKDEM(name)      LIQUID_CONCAT(cpfskdem,name)
#define CPFSKMOD(name)      LIQUID_CONCAT(cpfskmod,name)
#define MODEM(name)         LIQUID_CONCAT(modemcf,name)
#define FREQMOD(name)       LIQUID_CONCAT(freqmod,name)
#define FREQDEM(name)       LIQUID_CONCAT(freqdem,name)

#define T                   float           /* primitive type */
#define TC                  float complex   /* primitive type (complex) */

#define PRINTVAL_T(X,F)     PRINTVAL_FLOAT(X,F)
#define PRINTVAL_TC(X,F)    PRINTVAL_CFLOAT(X,F)

// naming extension (useful for print statements)
#define EXTENSION           "cf"

// common source must come first (object definition)
#include "modem_common.proto.c"

// generic modem specifications
#include "modem_psk.proto.c"
#include "modem_dpsk.proto.c"
#include "modem_ask.proto.c"
#include "modem_qam.proto.c"
#include "modem_apsk.proto.c"

// specific modems
#include "modem_bpsk.proto.c"
#include "modem_qpsk.proto.c"
#include "modem_ook.proto.c"
#include "modem_pi4dqpsk.proto.c"
#include "modem_sqam32.proto.c"
#include "modem_sqam128.proto.c"

// arbitrary modems
#include "modem_arb.proto.c"

// non-linear modems
#include "cpfskdem.proto.c"
#include "cpfskmod.proto.c"

// analog modems
#include "freqmod.proto.c"
#include "freqdem.proto.c"

