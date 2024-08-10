TEMPLATE = app
TARGET = qmqtt_example
QT = core network qmqtt

SOURCES += \
    example.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qmqtt/client
INSTALLS += target
