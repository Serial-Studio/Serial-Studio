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
// Filter API: floating-point
//

#include "liquid.internal.h"

// naming extensions (useful for print statements)
#define EXTENSION_SHORT     "f"
#define EXTENSION_FULL      "rrrf"

// 
#define AUTOCORR(name)      LIQUID_CONCAT(autocorr_rrrf,name)
#define FDELAY(name)        LIQUID_CONCAT(fdelay_rrrf,name)
#define FFTFILT(name)       LIQUID_CONCAT(fftfilt_rrrf,name)
#define FIRDECIM(name)      LIQUID_CONCAT(firdecim_rrrf,name)
#define FIRFARROW(name)     LIQUID_CONCAT(firfarrow_rrrf,name)
#define FIRFILT(name)       LIQUID_CONCAT(firfilt_rrrf,name)
#define FIRINTERP(name)     LIQUID_CONCAT(firinterp_rrrf,name)
#define FIRHILB(name)       LIQUID_CONCAT(firhilbf,name)
#define FIRPFB(name)        LIQUID_CONCAT(firpfb_rrrf,name)
#define IIRDECIM(name)      LIQUID_CONCAT(iirdecim_rrrf,name)
#define IIRFILT(name)       LIQUID_CONCAT(iirfilt_rrrf,name)
#define IIRFILTSOS(name)    LIQUID_CONCAT(iirfiltsos_rrrf,name)
#define IIRHILB(name)       LIQUID_CONCAT(iirhilbf,name)
#define IIRINTERP(name)     LIQUID_CONCAT(iirinterp_rrrf,name)
#define MSRESAMP(name)      LIQUID_CONCAT(msresamp_rrrf,name)
#define MSRESAMP2(name)     LIQUID_CONCAT(msresamp2_rrrf,name)
#define ORDFILT(name)       LIQUID_CONCAT(ordfilt_rrrf,name)
#define RRESAMP(name)       LIQUID_CONCAT(rresamp_rrrf,name)
#define RESAMP(name)        LIQUID_CONCAT(resamp_rrrf,name)
#define RESAMP2(name)       LIQUID_CONCAT(resamp2_rrrf,name)
#define SYMSYNC(name)       LIQUID_CONCAT(symsync_rrrf,name)

#define T                   float   // general
#define TO                  float   // output
#define TC                  float   // coefficients
#define TI                  float   // input
#define WINDOW(name)        LIQUID_CONCAT(windowf,name)
#define DOTPROD(name)       LIQUID_CONCAT(dotprod_rrrf,name)
#define POLY(name)          LIQUID_CONCAT(polyf,name)

#define TO_COMPLEX          0
#define TC_COMPLEX          0
#define TI_COMPLEX          0

#define PRINTVAL_TO(X,F)    PRINTVAL_FLOAT(X,F)
#define PRINTVAL_TC(X,F)    PRINTVAL_FLOAT(X,F)
#define PRINTVAL_TI(X,F)    PRINTVAL_FLOAT(X,F)

// prototype files
#include "autocorr.proto.c"
#include "fdelay.proto.c"
#include "fftfilt.proto.c"
#include "firdecim.proto.c"
#include "firfarrow.proto.c"
#include "firfilt.proto.c"
#include "firinterp.proto.c"
#include "firhilb.proto.c"
#include "firpfb.proto.c"
#include "iirdecim.proto.c"
#include "iirfilt.proto.c"
#include "iirfiltsos.proto.c"
#include "iirhilb.proto.c"
#include "iirinterp.proto.c"
#include "msresamp.proto.c"
#include "msresamp2.proto.c"
#include "ordfilt.proto.c"
#include "rresamp.proto.c"
//#include "resamp.proto.c"         // floating-point phase version
#include "resamp.fixed.proto.c" // fixed-point phase version
#include "resamp2.proto.c"
#include "symsync.proto.c"
