#
# Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
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

#-------------------------------------------------------------------------------
# Project setup
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.19)
project(QRealFourier CXX)

#-------------------------------------------------------------------------------
# C++ options
#-------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-------------------------------------------------------------------------------
# Add external dependencies
#-------------------------------------------------------------------------------

find_package(Qt6 COMPONENTS Core REQUIRED)

#-------------------------------------------------------------------------------
# Import source code & resources
#-------------------------------------------------------------------------------

include_directories(headers)
include_directories(fftreal)

set(SOURCES
 sources/qcomplexnumber.cpp
 sources/qfouriercalculator.cpp
 sources/qfourierfixedcalculator.cpp
 sources/qfouriertransformer.cpp
 sources/qfouriervariablecalculator.cpp
 sources/qwindowfunction.cpp
)

set(HEADERS
 fftreal/Array.h
 fftreal/Array.hpp
 fftreal/DynArray.h
 fftreal/DynArray.hpp
 fftreal/FFTReal.h
 fftreal/FFTReal.hpp
 fftreal/FFTRealFixLen.h
 fftreal/FFTRealFixLen.hpp
 fftreal/FFTRealFixLenParam.h
 fftreal/FFTRealPassDirect.h
 fftreal/FFTRealPassDirect.hpp
 fftreal/FFTRealPassInverse.h
 fftreal/FFTRealPassInverse.hpp
 fftreal/FFTRealSelect.h
 fftreal/FFTRealSelect.hpp
 fftreal/FFTRealUseTrigo.h
 fftreal/FFTRealUseTrigo.hpp
 fftreal/OscSinCos.h
 fftreal/OscSinCos.hpp
 fftreal/def.h
 headers/qcomplexnumber.h
 headers/qfouriercalculator.h
 headers/qfourierfixedcalculator.h
 headers/qfouriertransformer.h
 headers/qfouriervariablecalculator.h
 headers/qwindowfunction.h
)

#-------------------------------------------------------------------------------
# Compile & link the library
#-------------------------------------------------------------------------------

add_library(
 QRealFourier
 STATIC
 ${SOURCES}
 ${HEADERS}
)

target_link_libraries(
 QRealFourier PUBLIC
 Qt6::Core
)

target_include_directories(
 QRealFourier PUBLIC
 ${CMAKE_CURRENT_SOURCE_DIR}/headers
)
