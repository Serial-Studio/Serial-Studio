cmake_minimum_required(VERSION 3.20)
project(QCodeEditor)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_EXAMPLE "Example building required" OFF)

if (${BUILD_EXAMPLE})
 message(STATUS "QCodeEditor example will be built.")
 add_subdirectory(example)
endif()

set(RESOURCES_FILE
 resources/qcodeeditor_resources.qrc
)

set(INCLUDE_FILES
 include/QHighlightRule
 include/QHighlightBlockRule
 include/QCodeEditor
 include/QCXXHighlighter
 include/QJavascriptHighlighter
 include/QLineNumberArea
 include/QStyleSyntaxHighlighter
 include/QSyntaxStyle
 include/QGLSLCompleter
 include/QGLSLHighlighter
 include/QLanguage
 include/QXMLHighlighter
 include/QJSONHighlighter
 include/QLuaCompleter
 include/QLuaHighlighter
 include/QPythonHighlighter
 include/QFramedTextAttribute
 include/QJavascriptCompleter
 include/internal/QHighlightRule.hpp
 include/internal/QHighlightBlockRule.hpp
 include/internal/QCodeEditor.hpp
 include/internal/QCXXHighlighter.hpp
 include/internal/QJavascriptHighlighter.hpp
 include/internal/QLineNumberArea.hpp
 include/internal/QStyleSyntaxHighlighter.hpp
 include/internal/QSyntaxStyle.hpp
 include/internal/QGLSLCompleter.hpp
 include/internal/QGLSLHighlighter.hpp
 include/internal/QLanguage.hpp
 include/internal/QXMLHighlighter.hpp
 include/internal/QJSONHighlighter.hpp
 include/internal/QLuaCompleter.hpp
 include/internal/QLuaHighlighter.hpp
 include/internal/QPythonCompleter.hpp
 include/internal/QPythonHighlighter.hpp
 include/internal/QFramedTextAttribute.hpp
 include/internal/QJavascriptCompleter.hpp
)

set(SOURCE_FILES
 src/internal/QCodeEditor.cpp
 src/internal/QLineNumberArea.cpp
 src/internal/QCXXHighlighter.cpp
 src/internal/QJavascriptHighlighter.cpp
 src/internal/QSyntaxStyle.cpp
 src/internal/QStyleSyntaxHighlighter.cpp
 src/internal/QGLSLCompleter.cpp
 src/internal/QGLSLHighlighter.cpp
 src/internal/QLanguage.cpp
 src/internal/QXMLHighlighter.cpp
 src/internal/QJSONHighlighter.cpp
 src/internal/QLuaCompleter.cpp
 src/internal/QLuaHighlighter.cpp
 src/internal/QPythonCompleter.cpp
 src/internal/QPythonHighlighter.cpp
 src/internal/QFramedTextAttribute.cpp
 src/internal/QJavascriptCompleter.cpp
)

# Create code for QObjects
set(CMAKE_AUTOMOC On)

# Create code from resource files
set(CMAKE_AUTORCC ON)

# Find includes in corresponding build directories
find_package(Qt6Core    CONFIG REQUIRED)
find_package(Qt6Widgets CONFIG REQUIRED)
find_package(Qt6Gui     CONFIG REQUIRED)

add_library(QCodeEditor STATIC
 ${RESOURCES_FILE}
 ${SOURCE_FILES}
 ${INCLUDE_FILES}
)

target_include_directories(QCodeEditor PUBLIC
 include
)

target_link_libraries(QCodeEditor
 Qt6::Core
 Qt6::Widgets
 Qt6::Gui
)
