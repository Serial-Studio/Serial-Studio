# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
project(
  MdfViewer
  VERSION 1.0
  DESCRIPTION "Simple MDF file browser"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(mdfview
        src/mdfviewer.cpp src/mdfviewer.h
        src/mainframe.cpp src/mainframe.h
        src/mdfdocument.cpp src/mdfdocument.h
        src/mdfview.cpp src/mdfview.h
        src/mdfviewer.rc
        src/childframe.cpp src/childframe.h
        src/windowid.h
        src/channelobserverlistview.cpp src/channelobserverlistview.h
        src/channelobserverframe.cpp src/channelobserverframe.h
        src/util/csvwriter.cpp src/util/csvwriter.h
        src/util/ilogger.cpp src/util/ilogger.h
        src/util/logconfig.cpp src/util/logconfig.h
        src/util/logconsole.cpp src/util/logconsole.h
        src/util/logfile.cpp src/util/logfile.h
        src/util/logging.cpp src/util/logging.h
        src/util/logmessage.cpp src/util/logmessage.h
        src/util/logstream.cpp src/util/logstream.h
        src/util/stringutil.cpp src/util/stringutil.h
        src/util/timestamp.cpp src/util/timestamp.h
        src/samplereductionframe.cpp
        src/samplereductionframe.h
        src/samplereductionlistview.cpp
        src/samplereductionlistview.h)

target_include_directories(mdfview PRIVATE mdf
                                           "${CMAKE_CURRENT_SOURCE_DIR}/../mdflib/src")

if(MSVC)
  target_link_options(mdfview PRIVATE /SUBSYSTEM:WINDOWS)
  target_compile_definitions(mdfview PRIVATE _WIN32_WINNT=0x0A00)
elseif(MINGW)
  target_link_options(mdfview PRIVATE -mwindows -static -fstack-protector)
endif()

if(MSVC)
  target_compile_definitions(mdfview PRIVATE
          _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
          _CRT_SECURE_NO_WARNINGS
          _SILENCE_CXX20_U8PATH_DEPRECATION_WARNING)
endif()

target_link_libraries(mdfview PRIVATE mdf)

if(VCPKG)
  find_package(boost_process REQUIRED CONFIG)
  target_link_libraries(mdfview PRIVATE Boost::process)

  find_package(boost_filesystem REQUIRED CONFIG)
  target_link_libraries(mdfview PRIVATE Boost::filesystem)

  find_package(boost_locale REQUIRED CONFIG)
  target_link_libraries(mdfview PRIVATE Boost::locale)

  find_package(wxWidgets CONFIG REQUIRED)
  target_link_libraries(mdfview PRIVATE wx::core wx::base)
else()
  include("../script/wxwidgets.cmake")
  include("../script/boost.cmake")

  target_include_directories(mdfview PRIVATE ${wxWidgets_INCLUDE_DIRS})
  target_include_directories(mdfview PRIVATE ${Boost_INCLUDE_DIRS})

  target_link_libraries(mdfview PRIVATE ${wxWidgets_LIBRARIES})
  target_link_libraries(mdfview PRIVATE ${Boost_LIBRARIES})
endif()


if(MINGW)
  target_link_libraries(mdfview PRIVATE iconv)
endif()

if(WIN32)
  target_link_libraries(mdfview PRIVATE bcrypt)
  target_link_libraries(mdfview PRIVATE ws2_32)
endif()

if(WIN32)
  install(
    TARGETS mdfview
    EXPORT MdfViewTargets
    LIBRARY DESTINATION mdf/lib
    ARCHIVE DESTINATION mdf/lib
    RUNTIME DESTINATION mdf/bin
    PUBLIC_HEADER DESTINATION mdf/include/mdf)
else()
  install(
    TARGETS mdfview
    EXPORT MdfViewTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mdf)
endif()
