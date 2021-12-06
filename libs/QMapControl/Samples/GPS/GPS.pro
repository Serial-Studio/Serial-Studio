include(../../QMapControl.pri)
DEPENDPATH += src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = GPS

QT+=network
QT+=gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): cache()

# Input
SOURCES += src/gps.cpp \
src/main.cpp \
src/gps_neo.cpp \
    src/gps_simulation.cpp
HEADERS += src/gps.h \
src/gps_neo.h \
    src/gps_simulation.h
