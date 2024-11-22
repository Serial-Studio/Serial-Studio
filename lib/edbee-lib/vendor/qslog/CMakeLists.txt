CMAKE_MINIMUM_REQUIRED(VERSION 3.1...3.20)

project(QsLog)

OPTION(BUILD_WITH_QT5 "Whether to build with Qt5 or Qt6." OFF)

# Add CMake modules
set(CMAKE_MODULE_PATH
 "${QsLog_SOURCE_DIR}/cmake"
 "${CMAKE_MODULE_PATH}"
)

include(QsLogConfigTargets)

# Add a _d to debug binaries
set(CMAKE_DEBUG_POSTFIX "_d")

# Qt
find_package(Qt6 REQUIRED COMPONENTS Core)
set(QT_LIBRARIES Qt6::Core)

# As moc files are generated in the binary dir, tell to always look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${QsLog_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QsLog_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QsLog_BINARY_DIR}/bin)

set(HEADER_FILES
 QsLog.h
 QsLogDest.h
 QsLogDestConsole.h
 QsLogDestFile.h
 QsLogDestFunctor.h
 QsLogDisableForThisFile.h
 QsLogLevel.h
 QsLogMessage.h
 QsLogSharedLibrary.h
)

set(SOURCE_FILES
 QsLog.cpp
 QsLogDest.cpp
 QsLogDestConsole.cpp
 QsLogDestFile.cpp
 QsLogDestFunctor.cpp
 QsLogMessage.cpp
 QsLogLevel.cpp
)

if(APPLE)
 # Apple's compiler will not find standard includes like <thread> or <mutex> with 10.7 target otherwise
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# Use 11.0 OSX SDK for Qt6 compatibility
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

option(QS_LOG_LINE_NUMBERS "Automatically writes the file and line for each log message" ON)
if(QS_LOG_LINE_NUMBERS)
 add_definitions(-DQS_LOG_LINE_NUMBERS)
endif()

option(QS_LOG_DISABLE "Logging code is replaced with a no-op" OFF)
if(QS_LOG_DISABLE)
 add_definitions(-DQS_LOG_DISABLE)
endif()

option(QS_LOG_SEPARATE_THREAD "Messages are queued and written from a separate thread" OFF)
if(QS_LOG_SEPARATE_THREAD)
 add_definitions(-DQS_LOG_SEPARATE_THREAD)
endif()

option(QS_LOG_WIN_PRINTF_CONSOLE "Use fprintf instead of OutputDebugString on Windows" OFF)
if(QS_LOG_WIN_PRINTF_CONSOLE)
 add_definitions(-DQS_LOG_WIN_PRINTF_CONSOLE)
endif()

option(QS_LOG_IS_SHARED_LIBRARY "Build shared library" OFF)
if(QS_LOG_IS_SHARED_LIBRARY)
 set(QS_LOG_LIBRARY_TYPE SHARED)
 add_definitions(-DQSLOG_IS_SHARED_LIBRARY)
else(QS_LOG_IS_SHARED_LIBRARY)
 set(QS_LOG_LIBRARY_TYPE STATIC)
endif(QS_LOG_IS_SHARED_LIBRARY)

add_library(QsLog ${QS_LOG_LIBRARY_TYPE} ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(QsLog PROPERTIES AUTOMOC ON)

option(QS_LOG_BUILD_WINDOW "Build log window, depends on Qt5::Widgets" OFF)
if(QS_LOG_BUILD_WINDOW)
 find_package(Qt6 REQUIRED COMPONENTS Widgets)
 list(APPEND ADDITIONAL_LIBRARIES Qt6::Widgets)
 add_definitions(-DQS_LOG_WINDOW)
 set_target_properties(QsLog PROPERTIES AUTOUIC ON AUTORCC ON)
endif()

target_link_libraries(QsLog ${QT_LIBRARIES} ${ADDITIONAL_LIBRARIES})

install(FILES ${HEADER_FILES} DESTINATION include/QsLog)
QsLog_install_target(QsLog "")

option(QS_LOG_BUILD_TESTS "Build unit tests" OFF)
if(QS_LOG_BUILD_TESTS)
 add_subdirectory(unittest)
endif()
