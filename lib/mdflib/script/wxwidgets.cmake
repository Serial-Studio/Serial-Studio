# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT wxWidgets_FOUND )
    find_package(wxWidgets COMPONENTS adv core base)
endif()

if (wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
endif()

if (NOT wxWidgets_FOUND)
    if (WIN32)
        set(wxWidgets_ROOT_DIR ${COMP_DIR}/wxwidgets/master)

        if (MINGW)
            set(wxWidgets_LIB_DIR ${COMP_DIR}/wxwidgets/master/lib/gcc_x64_lib)
        else()
            set(wxWidgets_LIB_DIR ${COMP_DIR}/wxwidgets/master/lib/vc_x64_lib)
        endif()
    else()
        set(CMAKE_FIND_ROOT_PATH ${COMP_DIR}/wxwidgets/master)
        set(wxWidgets_USE_STATIC ON)
        set(wxWidgets_USE_UNICODE ON)
        if (CMAKE_BUILD_TYPE MATCHES DEBUG)
            set(wxWidgets_USE_DEBUG ON)
        else()
            set(wxWidgets_USE_DEBUG OFF)
        endif()
        set(wxWidgets_USE_UNIVERSAL OFF)
    endif()
    find_package(wxWidgets COMPONENTS adv core base )
    include(${wxWidgets_USE_FILE})
endif()

message(STATUS "wxWidgets Find Style: " ${wxWidgets_FIND_STYLE})
message(STATUS "wxWidgets Lib Dir: " ${wxWidgets_LIB_DIR})
message(STATUS "wxWidgets Config Exe : " ${wxWidgets_CONFIG_EXECUTABLE})
message(STATUS "wxWidgets Cmake Root Path : " ${CMAKE_FIND_ROOT_PATH})
message(STATUS "wxWidgets Root: " ${wxWidgets_ROOT_DIR})
message(STATUS "wxWidgets Found: " ${wxWidgets_FOUND})
message(STATUS "wxWidgets Include Dirs: " ${wxWidgets_INCLUDE_DIRS})
message(STATUS "wxWidgets Library Dirs: " ${wxWidgets_LIBRARY_DIRS})
message(STATUS "wxWidgets Libraries: " ${wxWidgets_LIBRARIES})
message(STATUS "wxWidgets Include File: " ${wxWidgets_USE_FILE})
message(STATUS "wxWidgets Definitions: " ${wxWidgets_DEFINITIONS})
message(STATUS "wxWidgets Debug Definitions: " ${wxWidgets_DEFINITIONS_DEBUG})
message(STATUS "wxWidgets CXX Flags: " ${wxWidgets_CXX_FLAGS})


message(STATUS "wxWidgets Found: " ${wxWidgets_FOUND})
message(STATUS "wxWidgets Include Dirs: " ${wxWidgets_INCLUDE_DIRS})
message(STATUS "wxWidgets Library Dirs: " ${wxWidgets_LIBRARY_DIRS})
message(STATUS "wxWidgets Libraries: " ${wxWidgets_LIBRARIES})
message(STATUS "wxWidgets Include File: " ${wxWidgets_USE_FILE})
message(STATUS "wxWidgets Definitions: " ${wxWidgets_DEFINITIONS})
message(STATUS "wxWidgets Debug Definitions: " ${wxWidgets_DEFINITIONS_DEBUG})
message(STATUS "wxWidgets CXX Flags: " ${wxWidgets_CXX_FLAGS})



