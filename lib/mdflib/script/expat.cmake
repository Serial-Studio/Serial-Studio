# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

set(EXPAT_USE_STATIC_LIBS ON)

if (NOT EXPAT_FOUND)
    if (TARGET expat OR TARGET expat::expat)
        set(EXPAT_FOUND TRUE)
        if (NOT EXPAT_INCLUDE_DIRS)
            get_target_property(EXPAT_INCLUDE_DIRS expat INTERFACE_INCLUDE_DIRECTORIES)
        endif()
        if (NOT EXPAT_LIBRARIES)
            if (TARGET expat::expat)
                set(EXPAT_LIBRARIES expat::expat)
            else()
                set(EXPAT_LIBRARIES expat)
            endif()
        endif()
        message(STATUS "EXPAT Found via FetchContent target")
    else()
        find_package(EXPAT)
        message(STATUS "EXPAT Found (Try 1): " ${EXPAT_FOUND})
        if (NOT EXPAT_FOUND)
            set(EXPAT_ROOT ${COMP_DIR}/expat/master)
            find_package(EXPAT REQUIRED)
        endif()
    endif()
endif()

message(STATUS "EXPAT Found: "  ${EXPAT_FOUND})
message(STATUS "EXPAT Include Dirs: "  ${EXPAT_INCLUDE_DIRS})
message(STATUS "EXPAT Libraries: " ${EXPAT_LIBRARIES})

