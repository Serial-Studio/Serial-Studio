# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG HEAD
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest." FORCE)

FetchContent_MakeAvailable(googletest)
message(STATUS "googletest Populated: " ${googletest_POPULATED})
message(STATUS "googletest Source: " ${googletest_SOURCE_DIR})
message(STATUS "googletest Binary: " ${googletest_BINARY_DIR})
