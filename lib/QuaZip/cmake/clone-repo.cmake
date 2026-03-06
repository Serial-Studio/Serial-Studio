# Checkout remote repository
macro(clone_repo name url)
    string(TOLOWER ${name} name_lower)
    string(TOUPPER ${name} name_upper)

    if(NOT ${name_upper}_REPOSITORY)
        set(${name_upper}_REPOSITORY ${url})
    endif()
    if(NOT ${name_upper}_TAG)
        set(${name_upper}_TAG master)
    endif()

    message(STATUS "Fetching ${name} ${${name_upper}_REPOSITORY} ${${name_upper}_TAG}")

    # Check for FetchContent cmake support
    if(${CMAKE_VERSION} VERSION_LESS "3.11")
        message(FATAL_ERROR "CMake 3.11 required to fetch ${name}")
    else()
        include(FetchContent)

        FetchContent_Declare(${name}
            GIT_REPOSITORY ${${name_upper}_REPOSITORY}
            GIT_TAG ${${name_upper}_TAG}
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${name_lower})

        FetchContent_GetProperties(${name} POPULATED ${name_lower}_POPULATED)

        if(NOT ${name_lower}_POPULATED)
            FetchContent_Populate(${name})
        endif()

        set(${name_upper}_SOURCE_DIR ${${name_lower}_SOURCE_DIR})
        set(${name_upper}_BINARY_DIR ${${name_lower}_BINARY_DIR})
    endif()
endmacro()
