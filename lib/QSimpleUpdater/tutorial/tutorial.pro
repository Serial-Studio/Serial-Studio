#
# Copyright (c) 2014-2025 Alex Spataru <https://github.com/alex-spataru>
#
# This work is free. You can redistribute it and/or modify it under the
# terms of the Do What The Fuck You Want To Public License, Version 2,
# as published by Sam Hocevar. See the COPYING file for more details.
#

TEMPLATE = app

QT += gui
QT += core
QT += network
QT += widgets

TARGET = "QSU Tutorial"

FORMS   += $$PWD/src/Window.ui
HEADERS += $$PWD/src/Window.h
SOURCES += $$PWD/src/Window.cpp \
           $$PWD/src/main.cpp

include ($$PWD/../QSimpleUpdater.pri)
