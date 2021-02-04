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

isEmpty(PREFIX) {
    PREFIX = /usr
}

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

target.path = $$PREFIX/bin

linux:!android {
    icon.path = $$PREFIX/share/pixmaps
    desktop.path = $$PREFIX/share/applications
    icon.files += deploy/linux/serial-studio.png
    desktop.files += deploy/linux/serial-studio.desktop

    INSTALLS += target desktop icon
}

mingw {
    license.path = $$PREFIX/share/licenses/$$TARGET
    license.files += LICENSE.md
    INSTALLS += target license
}

#-------------------------------------------------------------------------------
# Import source code
#-------------------------------------------------------------------------------

INCLUDEPATH += src

RESOURCES += \
    assets/assets.qrc

DISTFILES += \
    assets/qml/Widgets/*.qml \
    assets/qml/Windows/*.qml \
    assets/qml/*.qml

TRANSLATIONS += \
    assets/translations/en.ts \
    assets/translations/es.ts \
    assets/translations/zh.ts \
    assets/translations/de.ts

HEADERS += \
    src/AppInfo.h \
    src/CSV/Export.h \
    src/CSV/Player.h \
    src/IO/Console.h \
    src/IO/DataSources/Network.h \
    src/IO/DataSources/Serial.h \
    src/IO/Manager.h \
    src/JSON/Dataset.h \
    src/JSON/Frame.h \
    src/JSON/Generator.h \
    src/JSON/Group.h \
    src/Misc/ModuleManager.h \
    src/Misc/Translator.h \
    src/Misc/Utilities.h \
    src/UI/DataProvider.h \
    src/UI/GraphProvider.h \
    src/UI/WidgetProvider.h

SOURCES += \
    src/CSV/Export.cpp \
    src/CSV/Player.cpp \
    src/IO/Console.cpp \
    src/IO/DataSources/Network.cpp \
    src/IO/DataSources/Serial.cpp \
    src/IO/Manager.cpp \
    src/JSON/Dataset.cpp \
    src/JSON/Frame.cpp \
    src/JSON/Generator.cpp \
    src/JSON/Group.cpp \
    src/Misc/ModuleManager.cpp \
    src/Misc/Translator.cpp \
    src/Misc/Utilities.cpp \
    src/UI/DataProvider.cpp \
    src/UI/GraphProvider.cpp \
    src/UI/WidgetProvider.cpp \
    src/main.cpp
