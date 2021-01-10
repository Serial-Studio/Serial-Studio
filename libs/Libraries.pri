#
# Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

#-------------------------------------------------------------------------------
# Some libs already come with their *.pri files, include them
#-------------------------------------------------------------------------------

include($$PWD/qtcsv/qtcsv.pri)
include($$PWD/QSimpleUpdater/QSimpleUpdater.pri)

#-------------------------------------------------------------------------------
# CuteLogger stuff
#-------------------------------------------------------------------------------

DEFINES += CUTELOGGER_SRC
INCLUDEPATH += $$PWD/CuteLogger/include

SOURCES += $$PWD/CuteLogger/src/Logger.cpp \
           $$PWD/CuteLogger/src/AbstractAppender.cpp \
           $$PWD/CuteLogger/src/AbstractStringAppender.cpp \
           $$PWD/CuteLogger/src/ConsoleAppender.cpp \
           $$PWD/CuteLogger/src/FileAppender.cpp \
           $$PWD/CuteLogger/src/RollingFileAppender.cpp

HEADERS += $$PWD/CuteLogger/include/Logger.h \
           $$PWD/CuteLogger/include/CuteLogger_global.h \
           $$PWD/CuteLogger/include/AbstractAppender.h \
           $$PWD/CuteLogger/include/AbstractStringAppender.h \
           $$PWD/CuteLogger/include/ConsoleAppender.h \
           $$PWD/CuteLogger/include/FileAppender.h \
           $$PWD/CuteLogger/include/RollingFileAppender.h

win32 {
    SOURCES += $$PWD/CuteLogger/src/OutputDebugAppender.cpp
    HEADERS += $$PWD/CuteLogger/include/OutputDebugAppender.h
}

android {
    SOURCES += $$PWD/CuteLogger/src/AndroidAppender.cpp
    HEADERS += $$PWD/CuteLogger/include/AndroidAppender.h
}
