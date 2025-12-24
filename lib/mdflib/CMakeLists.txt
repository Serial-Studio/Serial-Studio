# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0167 NEW)

#cmake_policy(SET CMP0144 NEW)
if("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
  set(VCPKG OFF)
else()
  set(VCPKG ON)
endif()


option(BUILD_SHARED_LIBS "Build static library as default." OFF)
option(MDF_BUILD_SHARED_LIB "Build shared library." OFF)
option(MDF_BUILD_SHARED_LIB_NET "Build shared library with .NET." OFF) # Only for MSVC 19.36+
option(MDF_BUILD_SHARED_LIB_EXAMPLE "Build shared library example." OFF)
option(MDF_BUILD_DOC "Build documentation. Requires Doxygen and Release mode." OFF)
option(MDF_BUILD_TOOL "Build tools like the MDF Viewer. Requires WxWidgets." OFF)
option(MDF_BUILD_TEST "Build Google Unit Tests. Requires Google Test." OFF)
option(MDF_BUILD_PYTHON "Build Python module" OFF)

if(MDF_BUILD_TOOL AND VCPKG)
  list(APPEND VCPKG_MANIFEST_FEATURES "mdf2csv")
  list(APPEND VCPKG_MANIFEST_FEATURES "mdfviewer")
endif()

if(MDF_BUILD_TEST AND VCPKG)
  list(APPEND VCPKG_MANIFEST_FEATURES "mdflibtest")
endif()



# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # for static runtime

project(
  mdflib
  VERSION 2.3
  DESCRIPTION "Interface against MDF 3/4 files"
  LANGUAGES CXX)
  
add_subdirectory(mdflib)

if(MDF_BUILD_SHARED_LIB)
  if(NOT (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.36))
    set(MDF_BUILD_SHARED_LIB_NET OFF)
  endif()
  add_subdirectory(mdflibrary)
endif()

if(MDF_BUILD_TEST)
  enable_testing()
  add_subdirectory(mdflib_test)
endif()

if(MDF_BUILD_SHARED_LIB AND MDF_BUILD_SHARED_LIB_EXAMPLE)
  add_subdirectory(mdflibrary_example)
endif()

if(MDF_BUILD_TOOL)
  add_subdirectory(mdf2csv)
  add_subdirectory(mdfviewer)
endif()

if (MDF_BUILD_PYTHON)
  find_package(Python3 COMPONENTS Interpreter Development)
  cmake_print_properties(TARGETS Python3::Python PROPERTIES
          NAME
          LOCATION
          INTERFACE_INCLUDE_DIRECTORIES)

  cmake_print_variables(
          Python3_FOUND
          Python3_INCLUDE_DIRS
          Python3_RUNTIME_LIBRARY_DIRS)
  include("script/pybind11.cmake")
  add_subdirectory(python)
endif()

if((CMAKE_BUILD_TYPE MATCHES "^[Rr]elease") AND MDF_BUILD_DOC)
  include("script/doxygen.cmake")
  if(DOXYGEN_FOUND)
    set(DOXYGEN_RECURSIVE NO)
    set(DOXYGEN_REPEAT_BRIEF NO)
    set(DOXYGEN_PROJECT_NAME "MDF Lib")
    set(DOXYGEN_PROJECT_NUMBER "2.3")
    set(DOXYGEN_HTML_EXTRA_STYLESHEET doxygen/utillib.css)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/manual)
    doxygen_add_docs(
      doc_libmdf ALL include/mdf doxygen
      COMMENT "MDF Library generating API documentation with Doxygen")
  endif()
endif()

# ##############################################################################
# Packaging
# ##############################################################################
# https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-a-package-configuration-file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mdflib/mdflibConfigVersion.cmake"
  VERSION ${Upstream_VERSION}
  COMPATIBILITY AnyNewerVersion)

export(
  EXPORT MdfLibTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/mdflib/mdflibTargets.cmake"
  NAMESPACE Upstream::)
configure_file(cmake/MdfLibConfig.cmake
               "${CMAKE_CURRENT_BINARY_DIR}/mdflib/MdfLibConfig.cmake" COPYONLY)

set(ConfigPackageLocation lib/cmake/mdflib)
install(
  EXPORT MdfLibTargets
  FILE MdfLibTargets.cmake
  NAMESPACE Upstream::
  DESTINATION ${ConfigPackageLocation})
install(
  FILES cmake/MdfLibConfig.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/mdflib/mdflibConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
  COMPONENT Devel)
