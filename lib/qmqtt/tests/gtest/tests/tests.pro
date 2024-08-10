TARGET = qmqtt_tests
QT = core network qmqtt qmqtt-private testlib

DEFINES += QMQTT_LIBRARY_TESTS

SOURCES += \
    clienttest.cpp \
    tcpserver.cpp \
    main.cpp \
    customprinter.cpp \
    networktest.cpp \
    messagetest.cpp \
    frametest.cpp \
    sockettest.cpp

HEADERS += \
    tcpserver.h \
    customprinter.h \
    networkmock.h \
    socketmock.h \
    timermock.h \
    iodevicemock.h

INCLUDEPATH += \
    ../gtest/googletest/googletest/include \
    ../gtest/googletest/googlemock/include
LIBS += -L../gtest -L../gtest/debug -L../gtest/release -lgtest

unix:!NO_UNIT_TESTS:!NO_RUN_UNIT_TESTS: {
    unit_tests.target = all
    macx: unit_tests.commands = \
        install_name_tool -change libgtest.1.dylib $${OUT_PWD}/../gtest/libgtest.1.dylib $${OUT_PWD}/qmqtt_tests.app/Contents/MacOS/qmqtt_tests; \
        $${OUT_PWD}/qmqtt_tests.app/Contents/MacOS/qmqtt_tests
    else: unit_tests.commands = \
        LD_LIBRARY_PATH=$${OUT_PWD}/../gtest \
        $${OUT_PWD}/qmqtt_tests
    QMAKE_EXTRA_TARGETS += unit_tests
}
