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
# Make options
#-------------------------------------------------------------------------------

UI_DIR = uic
MOC_DIR = moc
RCC_DIR = qrc
OBJECTS_DIR = obj

CONFIG += c++11

#-------------------------------------------------------------------------------
# Qt configuration
#-------------------------------------------------------------------------------

TEMPLATE = app
TARGET = serial-studio

CONFIG += qtc_runnable
CONFIG += resources_big
CONFIG += qtquickcompiler

QT += xml
QT += sql
QT += svg
QT += core
QT += quick
QT += charts
QT += widgets
QT += serialport
QT += quickcontrols2

QTPLUGIN += qsvg

#-------------------------------------------------------------------------------
# Compiler options
#-------------------------------------------------------------------------------

*g++*: {
    QMAKE_CXXFLAGS_RELEASE -= -O
    QMAKE_CXXFLAGS_RELEASE *= -O3
}

*msvc*: {
    QMAKE_CXXFLAGS_RELEASE -= /O
    QMAKE_CXXFLAGS_RELEASE *= /O2
}
    
#-------------------------------------------------------------------------------
# Libraries
#-------------------------------------------------------------------------------

include(libs/Libraries.pri)

#-------------------------------------------------------------------------------
# Deploy options
#-------------------------------------------------------------------------------

win32* {
    TARGET = SerialStudio
    RC_FILE = deploy/windows/resources/info.rc
}

macx* {
    TARGET = SerialStudio
    ICON = deploy/macOS/icon.icns
    RC_FILE = deploy/macOS/icon.icns
    QMAKE_INFO_PLIST = deploy/macOS/info.plist
    CONFIG += sdk_no_version_check # To avoid warnings with Big Sur
}

linux:!android {
    target.path = /usr/bin
    icon.path = /usr/share/pixmaps
    desktop.path = /usr/share/applications
    icon.files += deploy/linux/serial-studio.png
    desktop.files += deploy/linux/serial-studio.desktop

    INSTALLS += target desktop icon
}


#-------------------------------------------------------------------------------
# MSYS2 integration
#-------------------------------------------------------------------------------

win32-g++ {
    target.path = $$(pkgdir)$$(MINGW_PREFIX)/bin
    license.path = $$(pkgdir)$$(MINGW_PREFIX)/share/licenses/$$(_realname)
    license.files += LICENSE.md
    INSTALLS += target license
}

#-------------------------------------------------------------------------------
# Import source code
#-------------------------------------------------------------------------------

RESOURCES += \
    assets/assets.qrc

HEADERS += \
    src/AppInfo.h \
    src/CsvPlayer.h \
    src/DataProvider.h \
    src/Dataset.h \
    src/Export.h \
    src/GraphProvider.h \
    src/Group.h \
    src/JsonGenerator.h \
    src/ModuleManager.h \
    src/SerialManager.h \
    src/Translator.h \
    src/WidgetProvider.h

SOURCES += \
    src/CsvPlayer.cpp \
    src/DataProvider.cpp \
    src/Dataset.cpp \
    src/Export.cpp \
    src/GraphProvider.cpp \
    src/Group.cpp \
    src/JsonGenerator.cpp \
    src/ModuleManager.cpp \
    src/SerialManager.cpp \
    src/Translator.cpp \
    src/WidgetProvider.cpp \
    src/main.cpp

DISTFILES += \
    assets/qml/Widgets/*.qml \
    assets/qml/Windows/*.qml \
    assets/qml/*.qml

TRANSLATIONS += \
    assets/translations/en.ts \
    assets/translations/es.ts \
    assets/translations/zh.ts \
    assets/translations/de.ts
