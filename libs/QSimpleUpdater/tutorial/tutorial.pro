#
# Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
#
# This work is free. You can redistribute it and/or modify it under the
# terms of the Do What The Fuck You Want To Public License, Version 2,
# as published by Sam Hocevar. See the COPYING file for more details.
#

TEMPLATE = app

TARGET = "QSU Tutorial"

FORMS   += $$PWD/src/Window.ui
HEADERS += $$PWD/src/Window.h
SOURCES += $$PWD/src/Window.cpp \
           $$PWD/src/main.cpp

include ($$PWD/../QSimpleUpdater.pri)
