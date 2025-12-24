# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT
project(
  mdflibrary
  VERSION 2.1
  DESCRIPTION "Interface against MDF 3/4 files"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX d)

message(STATUS  "Compiler Version " ${CMAKE_CXX_COMPILER_VERSION})
if (MDF_BUILD_SHARED_LIB_NET)
  message(STATUS  "Enable CLR")
  
  # error D8016: '/clr' and '/MT' command-line options are incompatible
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  add_library(mdflibrary SHARED
            # C++ files
            src/MdfExport.cpp ../include/mdflibrary/MdfExport.h
            
            # Following files are needed for clr
            src/AssemblyInfo.cpp
            src/MdfAttachment.cpp src/MdfAttachment.h
            src/MdfChannel.cpp src/MdfChannel.h
            src/MdfChannelArray.cpp src/MdfChannelArray.h
            src/MdfChannelConversion.cpp src/MdfChannelConversion.h
            src/MdfChannelGroup.cpp src/MdfChannelGroup.h
            src/MdfChannelObserver.cpp src/MdfChannelObserver.h
            src/MdfDataGroup.cpp src/MdfDataGroup.h
            src/MdfETag.cpp src/MdfETag.h
            src/MdfEvent.cpp src/MdfEvent.h
            src/MdfFile.cpp src/MdfFile.h
            src/MdfFileHistory.cpp src/MdfFileHistory.h
            src/MdfHeader.cpp src/MdfHeader.h
            src/mdflibrary.cpp src/mdflibrary.h
            src/MdfMetaData.cpp src/MdfMetaData.h
            src/MdfReader.cpp src/MdfReader.h
            src/MdfSourceInformation.cpp src/MdfSourceInformation.h
            src/Mdfwriter.cpp src/Mdfwriter.h
            src/CanMessage.cpp src/CanMessage.h
            src/MdfNetHelper.cpp src/MdfNetHelper.h
            src/MdfTimestamp.cpp src/MdfTimestamp.h
            src/MdfFileTimestamp.cpp src/MdfFileTimestamp.h
            )
  set_target_properties(mdflibrary PROPERTIES COMMON_LANGUAGE_RUNTIME "")
else()
  add_library(mdflibrary SHARED src/MdfExport.cpp)
endif()

target_link_libraries(mdflibrary PRIVATE mdf)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.3 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0 AND NOT MINGW)
  # fix for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90050
  target_link_libraries(mdflibrary PRIVATE pthread stdc++fs)
endif()

target_include_directories(
  mdflibrary PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

# Hander files for c/cpp, NOT the SAME as the one of c# clr
set(MDFLIBRARY_PUBLIC_HEADERS
            ../include/mdflibrary/MdfExport.h
            ../include/mdflibrary/MdfAttachment.h
            ../include/mdflibrary/MdfChannel.h
            ../include/mdflibrary/MdfChannelConversion.h
            ../include/mdflibrary/MdfChannelGroup.h
            ../include/mdflibrary/MdfChannelObserver.h
            ../include/mdflibrary/MdfDataGroup.h
            ../include/mdflibrary/MdfETag.h
            ../include/mdflibrary/MdfEvent.h
            ../include/mdflibrary/MdfFile.h
            ../include/mdflibrary/MdfFileHistory.h
            ../include/mdflibrary/MdfHeader.h
            ../include/mdflibrary/MdfMetaData.h
            ../include/mdflibrary/MdfReader.h
            ../include/mdflibrary/MdfSourceInformation.h
            ../include/mdflibrary/MdfWriter.h
            ../include/mdflibrary/CanMessage.h)
set_target_properties(mdflibrary PROPERTIES PUBLIC_HEADER
                                            "${MDFLIBRARY_PUBLIC_HEADERS}")
get_target_property(PH1 mdflibrary PUBLIC_HEADER)
message(STATUS "MDFLibrary Target Includes: " "${PH1}")

if(WIN32)
  install(
    TARGETS mdflibrary
    EXPORT MdfLibraryTargets
    LIBRARY DESTINATION mdflibrary/lib
    ARCHIVE DESTINATION mdflibrary/lib
    RUNTIME DESTINATION mdflibrary/bin
    PUBLIC_HEADER DESTINATION mdflibrary/include/mdflibrary)
else()
  install(
    TARGETS mdflibrary
    EXPORT MdfLibraryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mdflibrary)
endif()
