#
# Add the source folder to the include directories to be searched while
# compiling a project, this allows developers to write "#include <qmqtt.h>" in
# their respective projects.
#
INCLUDEPATH += $$PWD/src/mqtt

#
# Do not add DLL import/export symbols as we are compiling QMQTT source code
# directly with the source code of the project that includes this file.
#
DEFINES += MQTT_PROJECT_INCLUDE_SRC

#
# Add header files
#
HEADERS += \
    $$PWD/src/mqtt/qmqtt.h \
    $$PWD/src/mqtt/qmqtt_client.h \
    $$PWD/src/mqtt/qmqtt_client_p.h \
    $$PWD/src/mqtt/qmqtt_frame.h \
    $$PWD/src/mqtt/qmqtt_global.h \
    $$PWD/src/mqtt/qmqtt_message.h \
    $$PWD/src/mqtt/qmqtt_message_p.h \
    $$PWD/src/mqtt/qmqtt_network_p.h \
    $$PWD/src/mqtt/qmqtt_networkinterface.h \
    $$PWD/src/mqtt/qmqtt_routedmessage.h \
    $$PWD/src/mqtt/qmqtt_router.h \
    $$PWD/src/mqtt/qmqtt_routesubscription.h \
    $$PWD/src/mqtt/qmqtt_socket_p.h \
    $$PWD/src/mqtt/qmqtt_socketinterface.h \
    $$PWD/src/mqtt/qmqtt_timer_p.h \
    $$PWD/src/mqtt/qmqtt_timerinterface.h \
    $$PWD/src/mqtt/qmqtt_ssl_socket_p.h

#
# Add source files
#
SOURCES += \
    $$PWD/src/mqtt/qmqtt_client.cpp \
    $$PWD/src/mqtt/qmqtt_client_p.cpp \
    $$PWD/src/mqtt/qmqtt_frame.cpp \
    $$PWD/src/mqtt/qmqtt_message.cpp \
    $$PWD/src/mqtt/qmqtt_network.cpp \
    $$PWD/src/mqtt/qmqtt_router.cpp \
    $$PWD/src/mqtt/qmqtt_routesubscription.cpp \
    $$PWD/src/mqtt/qmqtt_socket.cpp \
    $$PWD/src/mqtt/qmqtt_timer.cpp \
    $$PWD/src/mqtt/qmqtt_ssl_socket.cpp

#
# Add support for websockets
#
QMQTT_WEBSOCKETS {
    HEADERS += \
        $$PWD/src/mqtt/qmqtt_websocket_p.h \
        $$PWD/src/mqtt/qmqtt_websocketiodevice_p.h

    SOURCES += \
        $$PWD/src/mqtt/qmqtt_websocket.cpp \
        $$PWD/src/mqtt/qmqtt_websocketiodevice.cpp
}
