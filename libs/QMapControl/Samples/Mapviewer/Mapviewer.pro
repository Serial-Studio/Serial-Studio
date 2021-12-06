include(../../QMapControl.pri)
QT+=network
QT+=gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): cache()

DEPENDPATH += src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = Mapviewer

# Input
HEADERS += src/mapviewer.h
SOURCES += src/main.cpp src/mapviewer.cpp

