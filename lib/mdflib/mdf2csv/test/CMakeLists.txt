# Copyright 2024 Ingemar Hedvall
# SPDX-License-Identifier: MIT

project(TestMdf2Csv
        VERSION 1.0
        DESCRIPTION "Google unit tests for the mdf2csv application"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(test_mdf2csv
        testarguments.cpp
        ../src/programargument.cpp
        ../src/programargument.h
        ../src/createfilelist.cpp
        ../src/createfilelist.h
        ../src/csvwriter.cpp
        ../src/csvwriter.h
        ../src/logtoconsole.cpp
        ../src/logtoconsole.h
        testcreatefiles.cpp)


target_include_directories(test_mdf2csv PRIVATE ../src)
target_include_directories(test_mdf2csv PRIVATE ../../include)
target_link_libraries(test_mdf2csv PRIVATE mdf)

if(VCPKG)
    find_package(boost_program_options REQUIRED CONFIG)
    target_link_libraries(test_mdf2csv PRIVATE Boost::program_options)

    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(test_mdf2csv PRIVATE GTest::gtest GTest::gtest_main
                                        GTest::gmock GTest::gmock_main)
else()
    include("../../script/boost.cmake")
    include("../../script/googletest.cmake")
    target_include_directories(test_mdf2csv PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(test_mdf2csv PRIVATE ${Boost_LIBRARIES})

    target_link_libraries(test_mdf2csv PRIVATE GTest::gtest GTest::gtest_main)
endif()

if(MSVC)
    target_compile_definitions(test_mdf2csv PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
endif()

include(GoogleTest)
gtest_discover_tests(test_mdf2csv)
