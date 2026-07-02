#
# Copyright (c) 2014-2025 Alex Spataru <https://github.com/alex-spataru>
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

#-------------------------------------------------------------------------------
# Project setup
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(QSimpleUpdater
    VERSION 1.0.0
    DESCRIPTION "Automatic update library for Qt applications"
    LANGUAGES CXX
)

#-------------------------------------------------------------------------------
# C++ options
#-------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-------------------------------------------------------------------------------
# Options
#
# When consumed as a subdirectory (e.g. Serial Studio), turn off tutorial/tests
# and set QSU_QT_VERSION_MAJOR externally if needed.
#-------------------------------------------------------------------------------

option(QSIMPLE_UPDATER_BUILD_TUTORIAL "Build the tutorial application" OFF)
option(QSIMPLE_UPDATER_BUILD_TESTS "Build the unit tests" ON)
option(QSIMPLE_UPDATER_BUILD_SHARED "Build as a shared library instead of static" OFF)

# Install rules only apply when QSimpleUpdater is the top-level project;
# embedded consumers (add_subdirectory) must not inherit them or the
# install-prefix override below
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(QSIMPLE_UPDATER_INSTALL "Generate install rules" ON)
else()
    option(QSIMPLE_UPDATER_INSTALL "Generate install rules" OFF)
endif()

#-------------------------------------------------------------------------------
# Find Qt dependencies
#
# Supports both Qt5 and Qt6 when used standalone. Parent projects can skip
# this by setting QSU_QT_VERSION_MAJOR before add_subdirectory().
#-------------------------------------------------------------------------------

if(NOT DEFINED QSU_QT_VERSION_MAJOR)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
    set(QSU_QT_VERSION_MAJOR ${QT_VERSION_MAJOR})
endif()

find_package(Qt${QSU_QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS Core Gui Network Widgets)

if(QSIMPLE_UPDATER_BUILD_TESTS)
    find_package(Qt${QSU_QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS Test)
endif()

#-------------------------------------------------------------------------------
# Fix linking errors when using .pri include approach
#-------------------------------------------------------------------------------

add_definitions(-DQSU_INCLUDE_MOC=1)

#-------------------------------------------------------------------------------
# Library sources
#-------------------------------------------------------------------------------

set(SOURCES
    src/AuthenticateDialog.cpp
    src/Downloader.cpp
    src/QSimpleUpdater.cpp
    src/Updater.cpp
)

set(HEADERS
    include/QSimpleUpdater.h
    src/AuthenticateDialog.h
    src/Downloader.h
    src/Updater.h
)

set(FORMS
    src/AuthenticateDialog.ui
    src/Downloader.ui
)

#-------------------------------------------------------------------------------
# Windows version information resource
#-------------------------------------------------------------------------------

if(WIN32)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/etc/resources/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY
    )
    set(WINDOWS_RC ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
endif()

#-------------------------------------------------------------------------------
# Compile & link the library
#-------------------------------------------------------------------------------

qt_add_resources(QSU_RCC ${CMAKE_CURRENT_SOURCE_DIR}/etc/resources/qsimpleupdater.qrc)

if(QSIMPLE_UPDATER_BUILD_SHARED)
    add_library(QSimpleUpdater SHARED
        ${SOURCES}
        ${HEADERS}
        ${FORMS}
        ${QSU_RCC}
        ${WINDOWS_RC}
    )
    target_compile_definitions(QSimpleUpdater PRIVATE QSU_SHARED)
else()
    add_library(QSimpleUpdater STATIC
        ${SOURCES}
        ${HEADERS}
        ${FORMS}
        ${QSU_RCC}
    )
endif()

target_include_directories(QSimpleUpdater PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(QSimpleUpdater PUBLIC
    Qt${QSU_QT_VERSION_MAJOR}::Core
    Qt${QSU_QT_VERSION_MAJOR}::Gui
    Qt${QSU_QT_VERSION_MAJOR}::Network
    Qt${QSU_QT_VERSION_MAJOR}::Widgets
)

#-------------------------------------------------------------------------------
# Library properties
#-------------------------------------------------------------------------------

set_target_properties(QSimpleUpdater PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Add debug postfix on Windows to distinguish debug/release builds
if(WIN32)
    set_target_properties(QSimpleUpdater PROPERTIES
        DEBUG_POSTFIX d
    )
endif()

#-------------------------------------------------------------------------------
# Installation rules
#-------------------------------------------------------------------------------

if(QSIMPLE_UPDATER_INSTALL)

include(GNUInstallDirs)

# Set default install directory to project's build/install if not specified
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "Install path prefix" FORCE)
endif()

# Install library
install(TARGETS QSimpleUpdater
    EXPORT QSimpleUpdaterTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install public headers
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/QSimpleUpdater.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QSimpleUpdater
)

# Install CMake config files
install(EXPORT QSimpleUpdaterTargets
    FILE QSimpleUpdaterTargets.cmake
    NAMESPACE QSimpleUpdater::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QSimpleUpdater
)

# Generate and install package config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/QSimpleUpdaterConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QSimpleUpdaterConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QSimpleUpdater
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QSimpleUpdaterConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QSimpleUpdaterConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QSimpleUpdaterConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QSimpleUpdater
)

endif()

#-------------------------------------------------------------------------------
# Tutorial
#-------------------------------------------------------------------------------

if(QSIMPLE_UPDATER_BUILD_TUTORIAL)
    add_subdirectory(tutorial)
endif()

#-------------------------------------------------------------------------------
# Unit tests
#-------------------------------------------------------------------------------

if(QSIMPLE_UPDATER_BUILD_TESTS)
    enable_testing()
    add_executable(UnitTests
        tests/main.cpp
        tests/Test_Versioning.h
        tests/Test_Updater.h
        tests/Test_QSimpleUpdater.h
        tests/Test_Downloader.h
    )
    target_link_libraries(UnitTests PRIVATE
        Qt${QSU_QT_VERSION_MAJOR}::Test
        QSimpleUpdater
    )
    add_test(NAME UnitTests COMMAND UnitTests)
endif()
