# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
if (NOT DOXYGEN_FOUND)
    find_package(Doxygen COMPONENTS dot mscgen dia)
endif()

message(STATUS "Doxygen Found: " ${DOXYGEN_FOUND})
message(STATUS "Doxygen Version: " ${DOXYGEN_VERSION})
