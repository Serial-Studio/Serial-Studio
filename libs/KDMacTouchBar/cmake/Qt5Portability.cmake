
include_directories(${Qt5Widgets_INCLUDE_DIRS})

if(QT_USE_QTNETWORK)
  find_package(Qt5Network REQUIRED)
  list(APPEND QT_LIBRARIES Qt5::Network)
  include_directories(${Qt5Network_INCLUDE_DIRS})
endif()

if(QT_USE_QTXML)
  find_package(Qt5Xml REQUIRED)
  list(APPEND QT_LIBRARIES Qt5::Xml)
  include_directories(${Qt5Xml_INCLUDE_DIRS})
endif()

if(QT_USE_QTTEST)
  find_package(Qt5Test REQUIRED)
  list(APPEND QT_LIBRARIES Qt5::Test)
  include_directories(${Qt5Test_INCLUDE_DIRS})
endif()

macro(qt4_wrap_ui)
  qt5_wrap_ui(${ARGN})
endmacro()

macro(qt4_wrap_cpp)
  qt5_wrap_cpp(${ARGN})
endmacro()

macro(qt4_add_resources)
  qt5_add_resources(${ARGN})
endmacro()
