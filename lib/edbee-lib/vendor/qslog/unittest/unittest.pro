QT += core

TARGET = QsLogUnitTest
CONFIG += console qtestlib c++11
CONFIG -= app_bundle
TEMPLATE = app

# optionally enable address sanitizer
linux-g++|macx {
    QMAKE_CXXFLAGS += -fsanitize=address
    QMAKE_LFLAGS += -fsanitize=address
}

# test-case sources
SOURCES += TestLog.cpp

HEADERS += TestLog.h

# component sources
include(../QsLog.pri)

SOURCES += \
    ./QtTestUtil/TestRegistry.cpp \
    ./QtTestUtil/SimpleChecker.cpp

HEADERS += \
    ./QtTestUtil/TestRegistry.h \
    ./QtTestUtil/TestRegistration.h \
    ./QtTestUtil/QtTestUtil.h
