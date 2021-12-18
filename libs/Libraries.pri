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
# Definitions to avoid adding DLL imports/exports
#-------------------------------------------------------------------------------

DEFINES += QTCSV_STATIC_LIB

#-------------------------------------------------------------------------------
# Fix MSVC math constants redefinition warning
#-------------------------------------------------------------------------------

DEFINES += _USE_MATH_DEFINES

#-------------------------------------------------------------------------------
# Useful hack for the unity build
#-------------------------------------------------------------------------------

INCLUDEPATH += $$PWD

#-------------------------------------------------------------------------------
# Include *.pri files
#-------------------------------------------------------------------------------

include($$PWD/qwt/qwt.pri)
include($$PWD/qtcsv/qtcsv.pri)
include($$PWD/qmqtt/qmqtt.pri)
include($$PWD/QMapControl/QMapControl.pri)
include($$PWD/QRealFourier/QRealFourier.pri)
include($$PWD/QSimpleUpdater/QSimpleUpdater.pri)

macx* {
    DEFINES += KDMACTOUCHBAR_BUILD_KDMACTOUCHBAR_SRC
    LIBS += -framework Cocoa
    INCLUDEPATH += $$PWD/KDMacTouchBar/src
    SOURCES += \
        $$PWD/KDMacTouchBar/src/kdmactouchbar.mm
    HEADERS += \
        $$PWD/KDMacTouchBar/src/kdmactouchbar.h \
        $$PWD/KDMacTouchBar/src/kdmactouchbar_global.h
}
