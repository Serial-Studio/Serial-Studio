include(../../QMapControl.pri)
QT+=network
QT+=gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): cache()
DEPENDPATH += src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = Linesandpoints

# Input
HEADERS += src/linesandpoints.h
SOURCES += src/linesandpoints.cpp src/main.cpp

