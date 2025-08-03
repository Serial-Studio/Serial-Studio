/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
// Filter API: complex floating-point
//

#include "liquid.internal.h"

// naming extensions (useful for print statements)
#define EXTENSION_SHORT     "f"
#define EXTENSION_FULL      "crcf"

// 
#define AUTOCORR(name)      LIQUID_CONCAT(autocorr_crcf,name)
#define FDELAY(name)        LIQUID_CONCAT(fdelay_crcf,name)
#define FFTFILT(name)       LIQUID_CONCAT(fftfilt_crcf,name)
#define FIRDECIM(name)      LIQUID_CONCAT(firdecim_crcf,name)
#define FIRFARROW(name)     LIQUID_CONCAT(firfarrow_crcf,name)
#define FIRFILT(name)       LIQUID_CONCAT(firfilt_crcf,name)
#define FIRINTERP(name)     LIQUID_CONCAT(firinterp_crcf,name)
#define FIRPFB(name)        LIQUID_CONCAT(firpfb_crcf,name)
#define IIRDECIM(name)      LIQUID_CONCAT(iirdecim_crcf,name)
#define IIRFILT(name)       LIQUID_CONCAT(iirfilt_crcf,name)
#define IIRFILTSOS(name)    LIQUID_CONCAT(iirfiltsos_crcf,name)
#define IIRINTERP(name)     LIQUID_CONCAT(iirinterp_crcf,name)
#define MSRESAMP(name)      LIQUID_CONCAT(msresamp_crcf,name)
#define MSRESAMP2(name)     LIQUID_CONCAT(msresamp2_crcf,name)
// ordfilt
#define RRESAMP(name)       LIQUID_CONCAT(rresamp_crcf,name)
#define RESAMP(name)        LIQUID_CONCAT(resamp_crcf,name)
#define RESAMP2(name)       LIQUID_CONCAT(resamp2_crcf,name)
#define SYMSYNC(name)       LIQUID_CONCAT(symsync_crcf,name)

#define T                   float complex   // general
#define TO                  float complex   // output
#define TC                  float           // coefficients
#define TI                  float complex   // input
#define WINDOW(name)        LIQUID_CONCAT(windowcf,name)
#define DOTPROD(name)       LIQUID_CONCAT(dotprod_crcf,name)
#define POLY(name)          LIQUID_CONCAT(polyf,name)

#define TO_COMPLEX          1
#define TC_COMPLEX          0
#define TI_COMPLEX          1

#define PRINTVAL_TO(X,F)    PRINTVAL_CFLOAT(X,F)
#define PRINTVAL_TC(X,F)    PRINTVAL_FLOAT(X,F)
#define PRINTVAL_TI(X,F)    PRINTVAL_CFLOAT(X,F)

// prototype files
//#include "autocorr.proto.c"
#include "fdelay.proto.c"
#include "fftfilt.proto.c"
#include "firdecim.proto.c"
#include "firfarrow.proto.c"
#include "firfilt.proto.c"
#include "firinterp.proto.c"
#include "firpfb.proto.c"
#include "iirdecim.proto.c"
#include "iirfilt.proto.c"
#include "iirfiltsos.proto.c"
#include "iirinterp.proto.c"
#include "msresamp.proto.c"
#include "msresamp2.proto.c"
// ordfilt
#include "rresamp.proto.c"
//#include "resamp.proto.c"         // floating-point phase version
#include "resamp.fixed.proto.c" // fixed-point phase version
#include "resamp2.proto.c"
#include "symsync.proto.c"
