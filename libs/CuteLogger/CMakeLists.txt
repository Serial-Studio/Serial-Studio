CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

PROJECT(CuteLogger)

FIND_PACKAGE(Qt5Core REQUIRED)

ADD_DEFINITIONS(-DCUTELOGGER_LIBRARY)

INCLUDE_DIRECTORIES(BEFORE include)

SET(sources
  src/Logger.cpp
  src/AbstractAppender.cpp
  src/AbstractStringAppender.cpp
  src/ConsoleAppender.cpp
  src/FileAppender.cpp
  src/RollingFileAppender.cpp
)

SET(includes
  include/Logger.h
  include/FileAppender.h
  include/CuteLogger_global.h
  include/ConsoleAppender.h
  include/AbstractStringAppender.h
  include/AbstractAppender.h
  include/RollingFileAppender.h
)


# OutputDebugAppender is only for Windows systems
IF(WIN32)
  SET(sources ${sources} src/OutputDebugAppender.cpp)
  SET(includes ${includes} include/OutputDebugAppender.h)
ENDIF(WIN32)


SET(library_target CuteLogger)

ADD_LIBRARY(${library_target} SHARED ${sources} ${includes})
TARGET_LINK_LIBRARIES(${library_target} Qt5::Core)
TARGET_INCLUDE_DIRECTORIES(${library_target} PUBLIC include)

INSTALL(TARGETS ${library_target} DESTINATION lib)

SET(ENABLE_TESTS OFF CACHE BOOL "Enable building CuteLogger tests")
IF (ENABLE_TESTS)
  SET(CMAKE_AUTOMOC ON)
  FIND_PACKAGE(Qt5Test REQUIRED)

  ADD_EXECUTABLE(basictest test/basictest.cpp)
  TARGET_LINK_LIBRARIES(basictest Qt5::Core Qt5::Test CuteLogger)
ENDIF ()
