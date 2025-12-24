cmake_minimum_required(VERSION 3.20)
project(mdfTestReadExample
        VERSION 1.0
        DESCRIPTION "Test find_package mdf"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(MdfLib REQUIRED)

add_executable(TestMDFCMake ../../docs/mdfreaderexample.cpp)
target_link_libraries(TestMDFCMake Upstream::mdf)
target_include_directories(TestMDFCMake PRIVATE ${MdfLibTargets_INCLUDE_DIR})
