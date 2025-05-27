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

function(target_link_openssl TARGET BASE_DIR)
  if(WIN32)
    target_include_directories(${TARGET} PRIVATE ${BASE_DIR}/include)
    target_compile_definitions(${TARGET} PRIVATE OPENSSL_USE_STATIC_LIBS)

    if(MSVC)
      target_link_libraries(${TARGET} PRIVATE
        ws2_32
        gdi32
        advapi32
        crypt32
        user32
        ${BASE_DIR}/dll/Windows/msvc/libssl.lib
        ${BASE_DIR}/dll/Windows/msvc/libcrypto.lib
      )
    elseif(MINGW)
      target_link_libraries(${TARGET} PRIVATE
        ws2_32
        gdi32
        advapi32
        crypt32
        user32
        ${BASE_DIR}/dll/Windows/mingw/libssl.a
        ${BASE_DIR}/dll/Windows/mingw/libcrypto.a
      )
    else()
      message(FATAL_ERROR "Unsupported Windows compiler — only MSVC and MinGW are handled.")
    endif()

  elseif(APPLE)
    target_include_directories(${TARGET} PRIVATE ${BASE_DIR}/include)
    target_compile_definitions(${TARGET} PRIVATE OPENSSL_USE_STATIC_LIBS)
    target_link_libraries(${TARGET} PRIVATE
      ${BASE_DIR}/dll/macOS/libssl.a
      ${BASE_DIR}/dll/macOS/libcrypto.a
    )

  elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENSSL REQUIRED openssl)
    target_link_libraries(${TARGET} PRIVATE ${OPENSSL_LIBRARIES})
    target_include_directories(${TARGET} PRIVATE ${OPENSSL_INCLUDE_DIRS})
    target_compile_options(${TARGET} PRIVATE ${OPENSSL_CFLAGS_OTHER})
  endif()
endfunction()
