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

cmake_minimum_required(VERSION 3.20)

include(FetchContent)


function(target_link_openssl TARGET)
  if(ENABLE_GRPC)
    return()
  endif()

  set(VERSION "4.0.0")
  set(BASE_URL "https://github.com/alex-spataru/OpenSSL-Builder/releases/download/${VERSION}")

  if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENSSL REQUIRED openssl)
    target_link_libraries(${TARGET} PRIVATE ${OPENSSL_LIBRARIES})
    target_include_directories(${TARGET} PRIVATE ${OPENSSL_INCLUDE_DIRS})
    target_compile_options(${TARGET} PRIVATE ${OPENSSL_CFLAGS_OTHER})
    return()
  endif()

  if(WIN32 AND MSVC)
    set(ASSET "OpenSSL-${VERSION}-Windows-MSVC-amd64.zip")
  elseif(WIN32 AND MINGW)
    set(ASSET "OpenSSL-${VERSION}-Windows-MinGW-amd64.zip")
  elseif(APPLE)
    set(ASSET "OpenSSL-${VERSION}-macOS-Universal.zip")
  else()
    message(FATAL_ERROR "target_link_openssl: unsupported platform/toolchain.")
  endif()

  FetchContent_Declare(openssl_prebuilt URL ${BASE_URL}/${ASSET})
  FetchContent_MakeAvailable(openssl_prebuilt)

  set(ROOT "${openssl_prebuilt_SOURCE_DIR}")
  if(EXISTS "${ROOT}/openssl-out")
    set(ROOT "${ROOT}/openssl-out")
  endif()

  set(LIBDIR "${ROOT}/lib")
  if(EXISTS "${ROOT}/lib64")
    set(LIBDIR "${ROOT}/lib64")
  endif()

  if(WIN32 AND MSVC)
    set(SSL_LIB "${LIBDIR}/libssl.lib")
    set(CRYPTO_LIB "${LIBDIR}/libcrypto.lib")
  else()
    set(SSL_LIB "${LIBDIR}/libssl.a")
    set(CRYPTO_LIB "${LIBDIR}/libcrypto.a")
  endif()

  if(NOT EXISTS "${SSL_LIB}" OR NOT EXISTS "${CRYPTO_LIB}")
    message(FATAL_ERROR "target_link_openssl: prebuilt OpenSSL not found in ${LIBDIR}.")
  endif()

  target_include_directories(${TARGET} PRIVATE "${ROOT}/include")
  target_compile_definitions(${TARGET} PRIVATE OPENSSL_USE_STATIC_LIBS)

  if(WIN32)
    target_link_libraries(${TARGET} PRIVATE
      ws2_32
      gdi32
      advapi32
      crypt32
      user32
      "${SSL_LIB}"
      "${CRYPTO_LIB}"
    )
  else()
    target_link_libraries(${TARGET} PRIVATE "${SSL_LIB}" "${CRYPTO_LIB}")
  endif()
endfunction()
