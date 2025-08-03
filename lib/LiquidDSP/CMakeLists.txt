# cmake build file for liquid-dsp
cmake_minimum_required(VERSION 3.10)

# project definition
project(liquid LANGUAGES C CXX)

# ---------------------------------------- options ----------------------------------------

option(BUILD_EXAMPLES   "Set to ON to build examples"           OFF)
option(BUILD_AUTOTESTS  "Set to ON to build autotest program"   OFF)
option(BUILD_BENCHMARKS "Set to ON to build benchmark program"  OFF)
option(ENABLE_SIMD      "Set to ON to build SIMD extensions"     ON)
option(BUILD_SANDBOX    "Set to ON to build sandbox programs"   OFF)
option(BUILD_DOC        "Set to ON to build documentation"      OFF)
option(COVERAGE         "Set to ON to build with code coverage" OFF)
#option(PROFILE        "Set to ON to build with profiling"     OFF)

if(COVERAGE)
    set(CMAKE_BUILD_TYPE "Debug")
endif(COVERAGE)

# target_sources() command converts relative paths to absolute.
cmake_policy(SET CMP0076 NEW)

set(LIBNAME liquid)

# require a C99 compiler for all targets
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ version selection")  # or 11, 14, 17, 20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # optional, ensure standard is supported
#set(CMAKE_CXX_EXTENSIONS OFF)  # optional, keep compiler extensions off

include (CheckIncludeFile)
include (CheckFunctionExists)
include (CheckLibraryExists)
include (CheckSymbolExists)

# check for hardware acceleration
include(cmake/FindSIMD.cmake)
# TODO: check for FFTW

include (CheckTypeSize)
check_type_size(int SIZEOF_INT LANGUAGE C)

configure_file(cmake/config.h.cmake config.h)

# this file can only use the @VARIABLE@ syntax
#configure_file(cmake/version.c.in ${PROJECT_BINARY_DIR}/src/version.c @ONLY)

#target_include_directories(test PRIVATE include)
include_directories(include .)

# ---------------------------------------- main sources ----------------------------------------

add_library(agc OBJECT
    src/agc/src/agc_crcf.c
    src/agc/src/agc_rrrf.c)

add_library(audio OBJECT
    src/audio/src/cvsd.c)

add_library(buffer OBJECT
    src/buffer/src/bufferf.c
    src/buffer/src/buffercf.c)

add_library(channel OBJECT
    src/channel/src/channel_cccf.c)

add_library(dotprod OBJECT)

add_library(equalization OBJECT
    src/equalization/src/equalizer_cccf.c
    src/equalization/src/equalizer_rrrf.c)

add_library(fec OBJECT
    src/fec/src/crc.c
    src/fec/src/fec.c
    src/fec/src/fec_conv.c
    src/fec/src/fec_conv_poly.c
    src/fec/src/fec_conv_pmatrix.c
    src/fec/src/fec_conv_punctured.c
    src/fec/src/fec_golay2412.c
    src/fec/src/fec_hamming74.c
    src/fec/src/fec_hamming84.c
    src/fec/src/fec_hamming128.c
    src/fec/src/fec_hamming1511.c
    src/fec/src/fec_hamming3126.c
    src/fec/src/fec_hamming128_gentab.c
    src/fec/src/fec_pass.c
    src/fec/src/fec_rep3.c
    src/fec/src/fec_rep5.c
    src/fec/src/fec_rs.c
    src/fec/src/fec_secded2216.c
    src/fec/src/fec_secded3932.c
    src/fec/src/fec_secded7264.c
    src/fec/src/interleaver.c
    src/fec/src/packetizer.c
    src/fec/src/sumproduct.c)

add_library(fft OBJECT
    src/fft/src/fftf.c
    src/fft/src/spgramcf.c
    src/fft/src/spgramf.c
    src/fft/src/fft_utilities.c)

add_library(filter OBJECT
    src/filter/src/bessel.c
    src/filter/src/butter.c
    src/filter/src/cheby1.c
    src/filter/src/cheby2.c
    src/filter/src/ellip.c
    src/filter/src/filter_rrrf.c
    src/filter/src/filter_crcf.c
    src/filter/src/filter_cccf.c
    src/filter/src/firdes.c
    src/filter/src/firdespm.c
    src/filter/src/firdespm_halfband.c
    src/filter/src/fnyquist.c
    src/filter/src/gmsk.c
    src/filter/src/group_delay.c
    src/filter/src/hM3.c
    src/filter/src/iirdes.pll.c
    src/filter/src/iirdes.c
    src/filter/src/lpc.c
    src/filter/src/rcos.c
    src/filter/src/rkaiser.c
    src/filter/src/rrcos.c)

add_library(framing OBJECT
    src/framing/src/bpacketgen.c
    src/framing/src/bpacketsync.c
    src/framing/src/detector_cccf.c
    src/framing/src/dsssframe64gen.c
    src/framing/src/dsssframe64sync.c
    src/framing/src/dsssframegen.c
    src/framing/src/dsssframesync.c
    src/framing/src/framedatastats.c
    src/framing/src/framesyncstats.c
    src/framing/src/framegen64.c
    src/framing/src/framesync64.c
    src/framing/src/framingcf.c
    src/framing/src/framing_rrrf.c
    src/framing/src/framing_crcf.c
    src/framing/src/framing_cccf.c
    src/framing/src/flexframegen.c
    src/framing/src/flexframesync.c
    src/framing/src/fskframegen.c
    src/framing/src/fskframesync.c
    src/framing/src/gmskframegen.c
    src/framing/src/gmskframesync.c
    src/framing/src/ofdmflexframegen.c
    src/framing/src/ofdmflexframesync.c
    src/framing/src/qpilotgen.c
    src/framing/src/qpilotsync.c)

add_library(math OBJECT
    src/math/src/poly.c
    src/math/src/polyc.c
    src/math/src/polyf.c
    src/math/src/polycf.c
    src/math/src/math.c
    src/math/src/math.bessel.c
    src/math/src/math.gamma.c
    src/math/src/math.complex.c
    src/math/src/math.trig.c
    src/math/src/modular_arithmetic.c
    src/math/src/poly.findroots.c
    src/math/src/windows.c)

add_library(matrix OBJECT
    src/matrix/src/matrix.c
    src/matrix/src/matrixf.c
    src/matrix/src/matrixc.c
    src/matrix/src/matrixcf.c
    src/matrix/src/smatrix.common.c
    src/matrix/src/smatrixb.c
    src/matrix/src/smatrixf.c
    src/matrix/src/smatrixi.c)

add_library(modem OBJECT
    src/modem/src/ampmodem.c
    src/modem/src/fskdem.c
    src/modem/src/fskmod.c
    src/modem/src/gmskdem.c
    src/modem/src/gmskmod.c
    src/modem/src/modem.shim.c
    src/modem/src/modemcf.c
    src/modem/src/modem_utilities.c
    src/modem/src/modem_apsk_const.c
    src/modem/src/modem_arb_const.c)

add_library(multichannel OBJECT
    src/multichannel/src/firpfbch_crcf.c
    src/multichannel/src/firpfbch_cccf.c
    src/multichannel/src/ofdmframe.common.c
    src/multichannel/src/ofdmframegen.c
    src/multichannel/src/ofdmframesync.c)

add_library(nco OBJECT
    src/nco/src/nco_crcf.c
    src/nco/src/nco.utilities.c)

add_library(optim OBJECT
    src/optim/src/chromosome.c
    src/optim/src/gasearch.c
    src/optim/src/gradsearch.c
    src/optim/src/optim.common.c
    src/optim/src/qnsearch.c
    src/optim/src/qs1dsearch.c
    src/optim/src/utilities.c)

add_library(quantization OBJECT
    src/quantization/src/compand.c
    src/quantization/src/quantizercf.c
    src/quantization/src/quantizerf.c
    src/quantization/src/quantizer.inline.c)

add_library(random OBJECT
    src/random/src/rand.c
    src/random/src/randn.c
    src/random/src/randexp.c
    src/random/src/randweib.c
    src/random/src/randgamma.c
    src/random/src/randnakm.c
    src/random/src/randricek.c
    src/random/src/scramble.c)

add_library(sequence OBJECT
    src/sequence/src/bsequence.c
    src/sequence/src/msequence.c)

add_library(utility OBJECT
    src/utility/src/bshift_array.c
    src/utility/src/byte_utilities.c
    src/utility/src/memory.c
    src/utility/src/msb_index.c
    src/utility/src/pack_bytes.c
    src/utility/src/shift_array.c
    src/utility/src/utility.c)

add_library(vector OBJECT
    src/vector/src/vectorf_add.port.c
    src/vector/src/vectorf_norm.port.c
    src/vector/src/vectorf_mul.port.c
    src/vector/src/vectorf_trig.port.c
    src/vector/src/vectorcf_add.port.c
    src/vector/src/vectorcf_norm.port.c
    src/vector/src/vectorcf_mul.port.c
    src/vector/src/vectorcf_trig.port.c)


# ---------------------------------------- SIMD extensions ----------------------------------------
if (C_NEON_FOUND AND ENABLE_SIMD)
    target_sources(dotprod PUBLIC
        src/dotprod/src/dotprod_cccf.neon.c
        src/dotprod/src/dotprod_crcf.neon.c
        src/dotprod/src/dotprod_rrrf.neon.c
        src/dotprod/src/sumsq.c)
    target_compile_options(dotprod PUBLIC ${C_NEON_FLAGS})
elseif (C_AVX512_FOUND AND ENABLE_SIMD)
    target_sources(dotprod PUBLIC
        src/dotprod/src/dotprod_cccf.avx512f.c
        src/dotprod/src/dotprod_crcf.avx512f.c
        src/dotprod/src/dotprod_rrrf.avx512f.c
        src/dotprod/src/sumsq.avx512f.c)
    target_compile_options(dotprod PUBLIC ${C_AVX512_FLAGS})
elseif (C_AVX2_FOUND AND ENABLE_SIMD)
    target_sources(dotprod PUBLIC
        src/dotprod/src/dotprod_cccf.avx.c
        src/dotprod/src/dotprod_crcf.avx.c
        src/dotprod/src/dotprod_rrrf.avx.c
        src/dotprod/src/sumsq.avx.c)
    target_compile_options(dotprod PUBLIC ${C_AVX2_FLAGS})
elseif (C_AVX_FOUND AND ENABLE_SIMD)
    target_sources(dotprod PUBLIC
        src/dotprod/src/dotprod_cccf.avx.c
        src/dotprod/src/dotprod_crcf.avx.c
        src/dotprod/src/dotprod_rrrf.avx.c
        src/dotprod/src/sumsq.avx.c)
    target_compile_options(dotprod PUBLIC ${C_AVX_FLAGS})
elseif (C_SSE4_FOUND AND ENABLE_SIMD)
    target_sources(dotprod PUBLIC
        src/dotprod/src/dotprod_cccf.sse.c
        src/dotprod/src/dotprod_crcf.sse.c
        src/dotprod/src/dotprod_rrrf.sse.c
        src/dotprod/src/sumsq.sse.c)
    target_compile_options(dotprod PUBLIC ${C_SSE4_FLAGS})
elseif (C_ALTIVEC_FOUND AND ENABLE_SIMD)
    target_sources(dotprod PUBLIC
        src/dotprod/src/dotprod_cccf.c
        src/dotprod/src/dotprod_crcf.av.c
        src/dotprod/src/dotprod_rrrf.av.c
        src/dotprod/src/sumsq.c)
else()
    # portable C version
    target_sources(dotprod PUBLIC
        src/dotprod/src/dotprod_cccf.c
        src/dotprod/src/dotprod_crcf.c
        src/dotprod/src/dotprod_rrrf.c
        src/dotprod/src/sumsq.c)
endif()

# ---------------------------------------- main library ----------------------------------------

# necessary to include this for the generated "config.h" header
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(${LIBNAME} STATIC
    src/libliquid.c
    $<TARGET_OBJECTS:agc>
    $<TARGET_OBJECTS:audio>
    $<TARGET_OBJECTS:buffer>
    $<TARGET_OBJECTS:channel>
    $<TARGET_OBJECTS:dotprod>
    $<TARGET_OBJECTS:equalization>
    $<TARGET_OBJECTS:fec>
    $<TARGET_OBJECTS:fft>
    $<TARGET_OBJECTS:filter>
    $<TARGET_OBJECTS:framing>
    $<TARGET_OBJECTS:math>
    $<TARGET_OBJECTS:matrix>
    $<TARGET_OBJECTS:modem>
    $<TARGET_OBJECTS:multichannel>
    $<TARGET_OBJECTS:nco>
    $<TARGET_OBJECTS:optim>
    $<TARGET_OBJECTS:quantization>
    $<TARGET_OBJECTS:random>
    $<TARGET_OBJECTS:sequence>
    $<TARGET_OBJECTS:utility>
    $<TARGET_OBJECTS:vector>)

target_include_directories(
 ${LIBNAME} PUBLIC
 ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#target_sources(${LIBNAME} PUBLIC FILE_SET HEADERS FILES include/liquid.h)

if(COVERAGE)
    target_compile_options(agc          PRIVATE -coverage -O0 -g)
    target_compile_options(audio        PRIVATE -coverage -O0 -g)
    target_compile_options(buffer       PRIVATE -coverage -O0 -g)
    target_compile_options(channel      PRIVATE -coverage -O0 -g)
    target_compile_options(dotprod      PRIVATE -coverage -O0 -g)
    target_compile_options(equalization PRIVATE -coverage -O0 -g)
    target_compile_options(fec          PRIVATE -coverage -O0 -g)
    target_compile_options(fft          PRIVATE -coverage -O0 -g)
    target_compile_options(filter       PRIVATE -coverage -O0 -g)
    target_compile_options(framing      PRIVATE -coverage -O0 -g)
    target_compile_options(math         PRIVATE -coverage -O0 -g)
    target_compile_options(matrix       PRIVATE -coverage -O0 -g)
    target_compile_options(modem        PRIVATE -coverage -O0 -g)
    target_compile_options(multichannel PRIVATE -coverage -O0 -g)
    target_compile_options(nco          PRIVATE -coverage -O0 -g)
    target_compile_options(optim        PRIVATE -coverage -O0 -g)
    target_compile_options(quantization PRIVATE -coverage -O0 -g)
    target_compile_options(random       PRIVATE -coverage -O0 -g)
    target_compile_options(sequence     PRIVATE -coverage -O0 -g)
    target_compile_options(utility      PRIVATE -coverage -O0 -g)
    target_compile_options(vector       PRIVATE -coverage -O0 -g)

    target_compile_options(${LIBNAME} PRIVATE -coverage -O0 -g)
    target_link_options(${LIBNAME} PRIVATE -coverage)
endif(COVERAGE)

target_link_libraries(${LIBNAME} m)

# ---------------------------------------- examples ----------------------------------------

# enable examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(BUILD_EXAMPLES)

# ---------------------------------------- sandbox ----------------------------------------

# enable sandbox
if(BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif(BUILD_SANDBOX)

# ---------------------------------------- autotest ----------------------------------------

set(AUTOTESTS
    ${PROJECT_SOURCE_DIR}/autotest/null_autotest.c
    ${PROJECT_SOURCE_DIR}/src/agc/tests/agc_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/audio/tests/cvsd_autotest.c
    ${PROJECT_SOURCE_DIR}/src/buffer/tests/cbuffer_autotest.c
    ${PROJECT_SOURCE_DIR}/src/buffer/tests/wdelay_autotest.c
    ${PROJECT_SOURCE_DIR}/src/buffer/tests/buffer_window_autotest.c
    ${PROJECT_SOURCE_DIR}/src/channel/tests/channel_copy_autotest.c
    ${PROJECT_SOURCE_DIR}/src/channel/tests/tvmpch_copy_autotest.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/tests/dotprod_rrrf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/tests/dotprod_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/tests/dotprod_cccf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/tests/sumsqf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/tests/sumsqcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/equalization/tests/eqlms_cccf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/equalization/tests/eqrls_rrrf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/crc_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_config_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_copy_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_soft_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_golay2412_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_hamming74_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_hamming84_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_hamming128_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_hamming1511_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_hamming3126_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_reedsolomon_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_rep3_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_rep5_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_secded2216_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_secded3932_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/fec_secded7264_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/interleaver_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/packetizer_copy_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fec/tests/packetizer_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/asgram_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/fft_small_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/fft_radix2_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/fft_composite_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/fft_prime_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/fft_r2r_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/fft_shift_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/spgram_autotest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/spwaterfall_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/dds_cccf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/fdelay_rrrf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/fftfilt_xxxf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/filter_crosscorr_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firdecim_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firdecim_xxxf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firdes_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firdespm_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firfilt_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firfilt_cccf_notch_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firfilt_coefficients_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firfilt_rnyquist_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firfilt_xxxf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firfilt_copy_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firhilb_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firinterp_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firpfb_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/groupdelay_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirdecim_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirdes_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirdes_support_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirfilt_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirfilt_xxxf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirfiltsos_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirhilb_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirinterp_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/lpc_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/msresamp_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/msresamp2_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/ordfilt_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/rresamp_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/rresamp_crcf_partition_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/resamp_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/resamp2_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/rkaiser_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/symsync_copy_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/symsync_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/symsync_rrrf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/bpacketsync_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/bsync_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/detector_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/dsssframe64_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/dsssframesync_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/flexframesync_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/framesync64_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/fskframesync_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/gmskframe_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/msource_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/ofdmflexframe_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/qdetector_cccf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/qdetector_cccf_copy_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/qdsync_cccf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/qpacketmodem_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/qpilotsync_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/qsource_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/symstreamcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/symstreamcf_delay_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/symstreamrcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/symstreamrcf_delay_autotest.c
    ${PROJECT_SOURCE_DIR}/src/framing/tests/symtrack_cccf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/gcd_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/math_window_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/math_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/math_bessel_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/math_gamma_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/math_complex_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/polynomial_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/polynomial_findroots_autotest.c
    ${PROJECT_SOURCE_DIR}/src/math/tests/prime_autotest.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/matrixcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/matrixf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/smatrixb_autotest.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/smatrixf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/smatrixi_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/ampmodem_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/cpfskmodem_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/freqmodem_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/fskmodem_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/gmskmodem_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/modem_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/modem_config_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/modem_demodsoft_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/modem_demodstats_autotest.c
    ${PROJECT_SOURCE_DIR}/src/modem/tests/modem_utilities_autotest.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/tests/firpfbch_crcf_synthesizer_autotest.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/tests/firpfbch_crcf_analyzer_autotest.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/tests/firpfbch_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/tests/firpfbch2_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/tests/firpfbchr_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/tests/ofdmframe_autotest.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/nco_crcf_autotest.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/nco_crcf_frequency_autotest.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/nco_crcf_mix_autotest.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/nco_crcf_phase_autotest.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/nco_crcf_pll_autotest.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/nco_crcf_spectrum_autotest.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/unwrap_phase_autotest.c
    ${PROJECT_SOURCE_DIR}/src/optim/tests/gasearch_autotest.c
    ${PROJECT_SOURCE_DIR}/src/optim/tests/gradsearch_autotest.c
    ${PROJECT_SOURCE_DIR}/src/optim/tests/qnsearch_autotest.c
    ${PROJECT_SOURCE_DIR}/src/optim/tests/qs1dsearch_autotest.c
    ${PROJECT_SOURCE_DIR}/src/optim/tests/utility_autotest.c
    ${PROJECT_SOURCE_DIR}/src/quantization/tests/compand_autotest.c
    ${PROJECT_SOURCE_DIR}/src/quantization/tests/quantize_autotest.c
    ${PROJECT_SOURCE_DIR}/src/random/tests/scramble_autotest.c
    ${PROJECT_SOURCE_DIR}/src/random/tests/random_autotest.c
    ${PROJECT_SOURCE_DIR}/src/random/tests/random_distributions_autotest.c
    ${PROJECT_SOURCE_DIR}/src/sequence/tests/bsequence_autotest.c
    ${PROJECT_SOURCE_DIR}/src/sequence/tests/complementary_codes_autotest.c
    ${PROJECT_SOURCE_DIR}/src/sequence/tests/msequence_autotest.c
    ${PROJECT_SOURCE_DIR}/src/utility/tests/bshift_array_autotest.c
    ${PROJECT_SOURCE_DIR}/src/utility/tests/count_bits_autotest.c
    ${PROJECT_SOURCE_DIR}/src/utility/tests/pack_bytes_autotest.c
    ${PROJECT_SOURCE_DIR}/src/utility/tests/shift_array_autotest.c
    )

set(AUTOTEST_EXTRA
    ${PROJECT_SOURCE_DIR}/src/fft/tests/fft_runtest.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_2.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_3.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_4.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_5.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_6.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_7.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_8.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_9.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_10.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_16.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_17.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_20.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_21.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_22.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_24.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_26.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_30.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_32.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_35.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_36.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_43.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_48.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_63.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_64.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_79.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_92.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_96.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_120.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_130.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_157.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_192.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_317.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_data_509.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_r2rdata_8.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_r2rdata_27.c
    ${PROJECT_SOURCE_DIR}/src/fft/tests/data/fft_r2rdata_32.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/fftfilt_runtest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_rrrf_data_h4x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_crcf_data_h4x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_cccf_data_h4x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_rrrf_data_h7x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_crcf_data_h7x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_cccf_data_h7x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_rrrf_data_h13x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_crcf_data_h13x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_cccf_data_h13x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_rrrf_data_h23x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_crcf_data_h23x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/fftfilt_cccf_data_h23x256.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firdecim_runtest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_rrrf_data_M2h4x20.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_crcf_data_M2h4x20.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_cccf_data_M2h4x20.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_rrrf_data_M3h7x30.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_crcf_data_M3h7x30.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_cccf_data_M3h7x30.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_rrrf_data_M4h13x40.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_crcf_data_M4h13x40.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_cccf_data_M4h13x40.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_rrrf_data_M5h23x50.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_crcf_data_M5h23x50.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firdecim_cccf_data_M5h23x50.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/firfilt_runtest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_rrrf_data_h4x8.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_crcf_data_h4x8.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_cccf_data_h4x8.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_rrrf_data_h7x16.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_crcf_data_h7x16.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_cccf_data_h7x16.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_rrrf_data_h13x32.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_crcf_data_h13x32.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_cccf_data_h13x32.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_rrrf_data_h23x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_crcf_data_h23x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/firfilt_cccf_data_h23x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/iirfilt_runtest.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_rrrf_data_h3x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_crcf_data_h3x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_cccf_data_h3x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_rrrf_data_h5x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_crcf_data_h5x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_cccf_data_h5x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_rrrf_data_h7x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_crcf_data_h7x64.c
    ${PROJECT_SOURCE_DIR}/src/filter/tests/data/iirfilt_cccf_data_h7x64.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_add.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_aug.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_cgsolve.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_chol.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_gramschmidt.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_inv.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_linsolve.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_ludecomp.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_mul.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_qrdecomp.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixf_data_transmul.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_add.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_aug.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_chol.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_inv.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_linsolve.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_ludecomp.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_mul.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_qrdecomp.c
    ${PROJECT_SOURCE_DIR}/src/matrix/tests/data/matrixcf_data_transmul.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/data/nco_sincos_fsqrt1_2.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/data/nco_sincos_fsqrt1_3.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/data/nco_sincos_fsqrt1_5.c
    ${PROJECT_SOURCE_DIR}/src/nco/tests/data/nco_sincos_fsqrt1_7.c
    )

if(BUILD_AUTOTESTS)
    add_executable(autoscript scripts/autoscript.c scripts/main.c scripts/autoscript.h)
    add_custom_command(
        OUTPUT  autotest_include.h
        DEPENDS
            ${PROJECT_BINARY_DIR}/autoscript
            ${AUTOTESTS}
        COMMAND ${PROJECT_BINARY_DIR}/autoscript / autotest ${AUTOTESTS} > autotest_include.h
        )

    # create output logging directory
    add_custom_command(
        OUTPUT autotest/logs
        COMMAND mkdir -p autotest/logs
        )

    add_executable(xautotest
        autotest/xautotest.c
        ${PROJECT_BINARY_DIR}/autotest_include.h
        autotest/autotest.h
        autotest/autotest.c
        autotest/libliquid_autotest.c
        autotest/logs
        ${AUTOTESTS}
        ${AUTOTEST_EXTRA}
        )
    target_include_directories(xautotest PRIVATE ${INCLUDE})

    target_link_libraries(xautotest ${LIBNAME})
    if(COVERAGE)
        target_compile_options(xautotest PRIVATE -coverage -O0 -g)
        target_link_options(xautotest PRIVATE -coverage)
    endif(COVERAGE)

    # enable testing; might consider splitting these up for parallel execution
    enable_testing()
    add_test(NAME test COMMAND xautotest)
endif(BUILD_AUTOTESTS)


# ---------------------------------------- benchmarks ----------------------------------------

set(BENCHMARKS
    ${PROJECT_SOURCE_DIR}/bench/null_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/agc/bench/agc_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/audio/bench/cvsd_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/buffer/bench/cbuffercf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/buffer/bench/window_push_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/buffer/bench/window_read_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/bench/dotprod_cccf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/bench/dotprod_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/bench/dotprod_rrrf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/bench/sumsqf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/dotprod/bench/sumsqcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/equalization/bench/eqlms_cccf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/equalization/bench/eqrls_cccf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fec/bench/crc_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fec/bench/fec_encode_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fec/bench/fec_decode_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fec/bench/fecsoft_decode_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fec/bench/sumproduct_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fec/bench/interleaver_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fec/bench/packetizer_decode_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fft/bench/fft_composite_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fft/bench/fft_prime_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fft/bench/fft_radix2_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fft/bench/fft_r2r_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/fft/bench/spgramcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/fftfilt_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/firdecim_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/firhilb_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/firinterp_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/firfilt_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/iirdecim_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/iirfilt_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/iirinterp_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/rresamp_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/resamp_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/resamp2_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/filter/bench/symsync_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/presync_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/bpacketsync_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/bpresync_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/bsync_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/detector_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/flexframesync_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/framesync64_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/gmskframesync_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/framing/bench/qdetector_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/math/bench/polyfit_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/matrix/bench/matrixf_inv_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/matrix/bench/matrixf_linsolve_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/matrix/bench/matrixf_mul_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/matrix/bench/smatrixf_mul_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/freqdem_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/freqmod_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/fskdem_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/fskmod_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/gmskmodem_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/modem_modulate_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/modem_demodulate_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/modem/bench/modem_demodsoft_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/bench/firpfbch_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/bench/firpfbch2_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/bench/firpfbchr_crcf_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/bench/ofdmframesync_acquire_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/multichannel/bench/ofdmframesync_rxsymbol_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/nco/bench/nco_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/nco/bench/vco_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/quantization/bench/quantizer_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/quantization/bench/compander_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/random/bench/random_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/sequence/bench/bsequence_benchmark.c
    ${PROJECT_SOURCE_DIR}/src/utility/bench/byte_utilities_benchmark.c
    )

set(BENCHMARKS_EXTRA
    ${PROJECT_SOURCE_DIR}/src/fft/bench/fft_runbench.c
    )

if(BUILD_BENCHMARKS)
    add_custom_command(
        OUTPUT  benchmark_include.h
        DEPENDS
            ${PROJECT_BINARY_DIR}/autoscript
            ${BENCHMARKS}
        COMMAND ${PROJECT_BINARY_DIR}/autoscript / benchmark ${BENCHMARKS} > benchmark_include.h
        )

    # create output logging directory
    #add_custom_command(
    #    OUTPUT benchmark/logs
    #    COMMAND mkdir -p benchmark/logs
    #    )

    #benchmark/logs
    add_executable(benchmark
        bench/bench.c
        ${PROJECT_BINARY_DIR}/benchmark_include.h
        ${BENCHMARKS}
        ${BENCHMARKS_EXTRA}
        )
    target_include_directories(benchmark PRIVATE ${INCLUDE})
    target_link_libraries(benchmark ${LIBNAME})
endif(BUILD_BENCHMARKS)


# ---------------------------------------- documentation ----------------------------------------

# enable examples
if(BUILD_DOC)
    add_subdirectory(doc)
endif(BUILD_DOC)

# ---------------------------------------- packaging ----------------------------------------

set(CPACK_PACKAGE_NAME liquid)
set(CPACK_PACKAGE_VENDOR "Joseph D. Gaeddert")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Software-Defined Radio Digital Signal Processing Library")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_VERBATIM_VARIABLES TRUE)
include(CPack)

# ---------------------------------------- summary ----------------------------------------

# debug cmake variables
#include(cmake/debug.cmake)

# cmake config summary
include(cmake/summary.cmake)
liquid_print_config_summary()

