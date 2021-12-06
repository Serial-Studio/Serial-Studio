include(../../QMapControl.pri)
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = bin
TARGET = Phonebook
DEPENDPATH += . ../../src
INCLUDEPATH += . ../../src

# Input
SOURCES += phonebook.cpp \
           main.cpp
HEADERS += phonebook.h

QT+=network

