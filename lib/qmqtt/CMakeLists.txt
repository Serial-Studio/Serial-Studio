cmake_minimum_required(VERSION 3.19)

project(qmqtt VERSION 1.0.3)

option(${PROJECT_NAME}_SHARED "Build a shared library.  Turn off for static." OFF)
option(${PROJECT_NAME}_WEBSOCKETS "Enable WebSockets for MQTT" OFF)
option(${PROJECT_NAME}_SSL "Enable SSL support for MQTT" ON)

if(${PROJECT_NAME}_SHARED)
 set(library_build_type SHARED)
 set(library_install_component Library)
else()
 set(library_build_type STATIC)
 set(library_install_component Devel)
endif()

set(ws_component)
set(ws_libname)
set(qt5_min_version "5.3.0")
set(qt6_min_version "6.2.4")

if(${PROJECT_NAME}_WEBSOCKETS)
 set(ws_component WebSockets)
 set(qt5_min_version "5.7.0")
endif()

if(NOT ${PROJECT_NAME}_SSL)
 set(ssl_defs QT_NO_SSL)
endif()

find_package(Qt6 ${qt6_min_version} COMPONENTS Core Network ${ws_component} CONFIG)
if(NOT Qt6_FOUND)
 find_package(Qt5 ${qt5_min_version} COMPONENTS Core Network ${ws_component} REQUIRED)
endif()

if(${PROJECT_NAME}_WEBSOCKETS)
 set(ws_libname "Qt${QT_VERSION_MAJOR}::WebSockets")
 find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WebSockets REQUIRED)
endif()

set(CMAKE_AUTOMOC ON)

set(PUBLIC_HEADERS
 src/mqtt/qmqtt_global.h
 src/mqtt/qmqtt.h
 src/mqtt/qmqtt_client.h
 src/mqtt/qmqtt_frame.h
 src/mqtt/qmqtt_message.h
 src/mqtt/qmqtt_routesubscription.h
 src/mqtt/qmqtt_routedmessage.h
 src/mqtt/qmqtt_router.h
 src/mqtt/qmqtt_networkinterface.h
 src/mqtt/qmqtt_socketinterface.h
 src/mqtt/qmqtt_timerinterface.h
)

set(PRIVATE_HEADERS
 src/mqtt/qmqtt_client_p.h
 src/mqtt/qmqtt_message_p.h
 src/mqtt/qmqtt_network_p.h
 src/mqtt/qmqtt_socket_p.h
 src/mqtt/qmqtt_timer_p.h
)

set(SOURCES
 src/mqtt/qmqtt_client_p.cpp
 src/mqtt/qmqtt_client.cpp
 src/mqtt/qmqtt_frame.cpp
 src/mqtt/qmqtt_message.cpp
 src/mqtt/qmqtt_network.cpp
 src/mqtt/qmqtt_routesubscription.cpp
 src/mqtt/qmqtt_router.cpp
 src/mqtt/qmqtt_socket.cpp
 src/mqtt/qmqtt_timer.cpp
)

if(${PROJECT_NAME}_WEBSOCKETS)
 list(APPEND PRIVATE_HEADERS
  src/mqtt/qmqtt_websocket_p.h
  src/mqtt/qmqtt_websocketiodevice_p.h
 )
list(APPEND SOURCES
 src/mqtt/qmqtt_websocket.cpp
 src/mqtt/qmqtt_websocketiodevice.cpp
)
endif()

if(${PROJECT_NAME}_SSL)
 list(APPEND PRIVATE_HEADERS
  src/mqtt/qmqtt_ssl_socket_p.h
 )
list(APPEND SOURCES
 src/mqtt/qmqtt_ssl_socket.cpp
)
endif()


add_library(${PROJECT_NAME} ${library_build_type} ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS})
target_link_libraries(${PROJECT_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network ${ws_libname})

target_include_directories(${PROJECT_NAME}
 PUBLIC
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/mqtt>
 $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(${PROJECT_NAME}
 PROPERTIES
 VERSION                   ${PROJECT_VERSION}
 SOVERSION                 ${PROJECT_VERSION_MAJOR}
 CXX_STANDARD              17
 CXX_STANDARD_REQUIRED     OFF
)
