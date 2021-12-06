include(../../QMapControl.pri)
DEPENDPATH += src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = Citymap

QT+=network
QT+=gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): cache()

# Input
SOURCES += src/citymap.cpp \
           src/main.cpp \
           src/dialogs.cpp
HEADERS += src/citymap.h \
           src/dialogs.h
