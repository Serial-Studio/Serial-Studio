# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

# Use the static ZLIB. Currently the line below doesn't work.
# The solution is to use the COMP_DIR if it is defined otherwise
# is the DLL version loaded.
set(ZLIB_USE_STATIC_LIBS ON)

if (NOT ZLIB_FOUND)
    find_package(ZLIB)
    message(STATUS "ZLIB Found (Try 1): " ${ZLIB_FOUND})
    if (NOT ZLIB_FOUND)
        set(ZLIB_ROOT ${COMP_DIR}/zlib/master)
        find_package(ZLIB REQUIRED)
        message(STATUS "ZLIB Found (Try 2): " ${ZLIB_FOUND})
    endif()
endif()

message(STATUS "ZLIB Version: " ${ZLIB_VERSION_STRING})
message(STATUS "ZLIB Include Dirs: " ${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLIB Libraries: " ${ZLIB_LIBRARIES})