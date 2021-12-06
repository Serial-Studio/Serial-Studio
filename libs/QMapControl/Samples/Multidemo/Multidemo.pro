include(../../QMapControl.pri)
QT+=network
DEPENDPATH += src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = Multidemo

QT+=network
QT+=gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): cache()

# Input
HEADERS += src/multidemo.h src/gps_modul.h
SOURCES += src/multidemo.cpp src/main.cpp src/gps_modul.cpp
