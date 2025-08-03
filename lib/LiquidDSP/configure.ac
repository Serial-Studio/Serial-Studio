# Copyright (c) 2007 - 2025 Joseph Gaeddert
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# 
# liquid-dsp library configure
# Process with autoconf to generate configure script
#

AC_INIT([liquid-dsp],[1.7.0],[joseph@liquidsdr.org])
AC_CONFIG_SRCDIR([src/libliquid.c])
AC_CONFIG_MACRO_DIR([scripts])

# permit auxiliary scripts directory (e.g. config.sub, config.guess, install-sh)
AC_CONFIG_AUX_DIR(scripts/)

# Specify 'C' language
AC_LANG(C)

# uncomment the following line to reset "CFLAGS" variable; it defaults to '-g -O2'
#: ${CFLAGS=""}

# Autoheader
AH_TEMPLATE([LIQUID_FFTOVERRIDE],  [Force internal FFT even if libfftw is available])
AH_TEMPLATE([LIQUID_SIMDOVERRIDE], [Force overriding of SIMD (use portable C code)])
AH_TEMPLATE([LIQUID_SUPPRESS_ERROR_OUTPUT],[Suppress printing errors to stderr])
AH_TEMPLATE([LIQUID_STRICT_EXIT],  [Enable strict program exit on error])

AC_CONFIG_HEADERS([config.h])
AH_TOP([
#ifndef __LIQUID_CONFIG_H__
#define __LIQUID_CONFIG_H__
])
AH_BOTTOM([
#endif // __LIQUID_CONFIG_H__
])

# Configure options

AC_ARG_ENABLE(debug-messages,
    AS_HELP_STRING([--enable-debug-messages],[enable verbose debug messages (-DDEBUG)]),
    [DEBUG_MSG_OPTION="-DDEBUG"],
    [DEBUG_MSG_OPTION=""]
)

AC_ARG_ENABLE(simdoverride,
    AS_HELP_STRING([--enable-simdoverride],[use portable C code for dotprod, etc. even if SIMD extensions are available]),
    [AC_DEFINE(LIQUID_SIMDOVERRIDE)],
    [],
)

AC_ARG_ENABLE(fftoverride,
    AS_HELP_STRING([--enable-fftoverride],[use internal fft even if libfftw is available]),
    [AC_DEFINE(LIQUID_FFTOVERRIDE)],
    [],
)

# code coverage
AC_ARG_ENABLE(coverage,
    AS_HELP_STRING([--enable-coverage],[enable flags to test code coverage]),
    [   # check that gcovr program is installed
        AC_CHECK_PROG(HAVE_GCOVR,gcovr,yes)
        AS_IF([test x"$HAVE_GCOVR" != x"yes"],
              [AC_MSG_ERROR(Need gcovr with coverage option)],
              [COVERAGE_OPTION="--coverage"])],
    [COVERAGE_OPTION=""]
)

AC_ARG_ENABLE(suppress-errors,
    AS_HELP_STRING([--enable-suppress-errors],[suppress printing errors to stderr]),
    [AC_DEFINE(LIQUID_SUPPRESS_ERROR_OUTPUT)],
    [],
)

AC_ARG_ENABLE(strict,
    AS_HELP_STRING([--enable-strict],[use strict error handling at runtime]),
    [AC_DEFINE(LIQUID_STRICT_EXIT)],
    [],
)

# Check for necessary programs
AC_PROG_CC
AC_PROG_SED
AC_PROG_GREP
AC_PROG_INSTALL
AC_PROG_RANLIB

# Check for necessary libraries, library functions
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC

# AC_CHECK_LIB (library, function, [action-if-found], [action-if-not-found], [other-libraries])
AC_CHECK_LIB([c],[main],[],[AC_MSG_ERROR(Could not use standard C library)],   [])
AC_CHECK_LIB([m],[main],[],[AC_MSG_ERROR(Could not use standard math library)],[])

# AC_CHECK_FUNC(function, [action-if-found], [action-if-not-found])
AC_CHECK_FUNCS([malloc realloc free memset memmove],[],
               [AC_MSG_ERROR(Could not use memory allocation functions)])
AC_CHECK_FUNCS([sinf cosf expf cargf cexpf crealf cimagf sqrtf],[],
               [AC_MSG_ERROR(Could not use special math functions)],)

# Check for necessary header files
AC_CHECK_HEADERS([stdio.h stdlib.h complex.h string.h getopt.h sys/resource.h float.h inttypes.h limits.h stdlib.h string.h unistd.h],[],
                 [AC_MSG_ERROR(Could not use standard headers)])

# Check for optional header files, libraries, programs
AC_CHECK_HEADERS(fec.h fftw3.h)
AC_CHECK_LIB([fftw3f], [fftwf_plan_dft_1d], [],
             [AC_MSG_WARN(fftw3 library useful but not required)],
             [])
AC_CHECK_LIB([fec], [create_viterbi27], [],
             [AC_MSG_WARN(fec library useful but not required)],
             [])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Check size of certain variables
AC_CHECK_SIZEOF(short int)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long int)
AC_CHECK_SIZEOF(long long int)

# AX_GCC_ARCHFLAG([PORTABLE?], [ACTION-SUCCESS], [ACTION-FAILURE])
# Try to guess the "native" architecture of the target to use with gcc's
# -march or -mtune flags. Default success action adds $ax_cv_gcc_archflag to
# $CFLAGS (sets to "unknown" on failure).
#AX_GCC_ARCHFLAG([no],[],[])

# get canonical target architecture
AC_CANONICAL_TARGET

# override SIMD
if test "${enable_simdoverride+set}" = set; then
    # portable C version
    MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.o \
                   src/dotprod/src/dotprod_crcf.o \
                   src/dotprod/src/dotprod_rrrf.o \
                   src/dotprod/src/sumsq.o"
    ARCH_OPTION=""
else
    # Check canonical system
    case $target_cpu in
    i386|i486|i586|i686|x86|x86_64)
        # check for MMX/SSE/AVX CPU extensions and intrinsics headers
        #   MMX     :   mmintrin.h
        #   SSE     :   xmmintrin.h
        #   SSE2    :   emmintrin.h
        #   SSE3    :   pmmintrin.h
        #   SSSE3   :   tmmintrin.h
        #   SSE4.1/2:   smmintrin.h
        #   AVX     :   immintrin.h
        #   AVX2    :   immintrin.h
        #   AVX512  :   immintrin.h
        AX_EXT

        if [ test "$ax_cv_have_avx512f_ext" = yes ]; then
            # AVX512 extensions
            MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.avx512f.o \
                           src/dotprod/src/dotprod_crcf.avx512f.o \
                           src/dotprod/src/dotprod_rrrf.avx512f.o \
                           src/dotprod/src/sumsq.avx512f.o"
            ARCH_OPTION='-mavx512f'
        elif [ test "$ax_cv_have_avx2_ext" = yes ]; then
            # AVX2 extensions
            MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.avx.o \
                           src/dotprod/src/dotprod_crcf.avx.o \
                           src/dotprod/src/dotprod_rrrf.avx.o \
                           src/dotprod/src/sumsq.avx.o"
            ARCH_OPTION='-mavx2'
        elif [ test "$ax_cv_have_avx_ext" = yes ]; then
            # AVX extensions
            MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.avx.o \
                           src/dotprod/src/dotprod_crcf.avx.o \
                           src/dotprod/src/dotprod_rrrf.avx.o \
                           src/dotprod/src/sumsq.avx.o"
            ARCH_OPTION='-mavx'
        elif [ test "$ax_cv_have_sse41_ext" = yes ]; then
            # SSE4.1/2 extensions
            MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.sse.o \
                           src/dotprod/src/dotprod_crcf.sse.o \
                           src/dotprod/src/dotprod_rrrf.sse.o \
                           src/dotprod/src/sumsq.sse.o"
            ARCH_OPTION='-msse4.1'
        elif [ test "$ax_cv_have_sse3_ext" = yes ]; then
            # SSE3 extensions
            MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.sse.o \
                           src/dotprod/src/dotprod_crcf.sse.o \
                           src/dotprod/src/dotprod_rrrf.sse.o \
                           src/dotprod/src/sumsq.sse.o"
            ARCH_OPTION='-msse3'
        elif [ test "$ax_cv_have_sse2_ext" = yes ]; then
            # SSE2 extensions
            MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.sse.o \
                           src/dotprod/src/dotprod_crcf.sse.o \
                           src/dotprod/src/dotprod_rrrf.sse.o \
                           src/dotprod/src/sumsq.sse.o"
            ARCH_OPTION='-msse2'
        else
            # portable C version
            MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.o \
                           src/dotprod/src/dotprod_crcf.o \
                           src/dotprod/src/dotprod_rrrf.o \
                           src/dotprod/src/sumsq.o"
        fi;;
    powerpc*)
        MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.o \
                       src/dotprod/src/dotprod_rrrf.av.o \
                       src/dotprod/src/dotprod_crcf.av.o \
                       src/dotprod/src/sumsq.o"
        ARCH_OPTION="-fno-common -faltivec";;
    armv1*|armv2*|armv3*|armv4*|armv5*|armv6*)
        # assume neon instructions are NOT available
        MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.o \
                       src/dotprod/src/dotprod_crcf.o \
                       src/dotprod/src/dotprod_rrrf.o \
                       src/dotprod/src/sumsq.o"
        ARCH_OPTION="-ffast-math";;
    arm|armv7*|armv8*|aarch64)
        # assume neon instructions are available
        # TODO: check for Neon availability

        # ARM architecture : use neon extensions
        MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.neon.o \
                       src/dotprod/src/dotprod_crcf.neon.o \
                       src/dotprod/src/dotprod_rrrf.neon.o \
                       src/dotprod/src/sumsq.o"
        case $target_os in
        darwin*)
            # M1 mac, ARM architecture : use neon extensions
            ARCH_OPTION="-ffast-math";;
        *)
            # TODO: check these flags
            #ARCH_OPTION="-ffast-math -mcpu=cortex-a8 -mfloat-abi=softfp -mfpu=neon";;
            ARCH_OPTION="-ffast-math -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4";;
        esac;;
    *)
        # unknown architecture : use portable C version
        MLIBS_DOTPROD="src/dotprod/src/dotprod_cccf.o \
                       src/dotprod/src/dotprod_crcf.o \
                       src/dotprod/src/dotprod_rrrf.o \
                       src/dotprod/src/sumsq.o"
        ARCH_OPTION="";;
    esac
fi


# for now all vector operations are portable C versions
MLIBS_VECTOR="src/vector/src/vectorf_add.port.o   \
              src/vector/src/vectorf_norm.port.o  \
              src/vector/src/vectorf_mul.port.o   \
              src/vector/src/vectorf_trig.port.o  \
              src/vector/src/vectorcf_add.port.o  \
              src/vector/src/vectorcf_norm.port.o \
              src/vector/src/vectorcf_mul.port.o  \
              src/vector/src/vectorcf_trig.port.o"

case $target_os in
darwin*)
    AN_MAKEVAR([LIBTOOL], [AC_PROG_LIBTOOL])
    AN_PROGRAM([libtool], [AC_PROG_LIBTOOL])
    AC_DEFUN([AC_PROG_LIBTOOL], [AC_CHECK_TOOL(LIBTOOL, libtool, :)])
    AC_PROG_LIBTOOL

    AR_LIB=ar	    # archive extension
    SH_LIB=dylib    # shared library extension
    REBIND=""
    ;;
*)
    AN_MAKEVAR([AR], [AC_PROG_AR])
    AN_PROGRAM([ar], [AC_PROG_AR])
    AC_DEFUN([AC_PROG_AR], [AC_CHECK_TOOL(AR, ar, :)])
    AC_PROG_AR

    AR_LIB=a	    # archive extension
    SH_LIB=so	    # shared library extension
    REBIND=ldconfig
    ;;
esac

#
# autoconf variable substitutions
#
AC_SUBST(LIBS)                      # shared libraries (-lc, -lm, etc.)
AC_SUBST(MLIBS_DOTPROD)             # 
AC_SUBST(MLIBS_VECTOR)              #

AC_SUBST(AR_LIB)                    # archive library
AC_SUBST(SH_LIB)                    # output shared library target
AC_SUBST(REBIND)                    # rebinding tool (e.g. ldconfig)
AC_SUBST(ARCH_OPTION)               # compiler architecture option

AC_SUBST(DEBUG_MSG_OPTION)          # debug messages option (.e.g -DDEBUG)
AC_SUBST(COVERAGE_OPTION)           # source code coverage option (e.g. -fprofile-arcs -ftest-coverage)
AC_SUBST(CLIB)                      # C library linkage (e.g. '-lc')

AC_CONFIG_FILES([makefile])
AC_OUTPUT
