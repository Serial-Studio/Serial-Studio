include(../../QMapControl.pri)
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = bin
TARGET = Mapviewer
DEPENDPATH += . ../../src
INCLUDEPATH += . ../../src

# Input
HEADERS += src/mapviewer.h
SOURCES += src/main.cpp src/mapviewer.cpp

QT+=network
