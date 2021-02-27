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

#-----------------------------------------------------------------------------------------
# Make options
#-----------------------------------------------------------------------------------------

UI_DIR = uic
MOC_DIR = moc
RCC_DIR = qrc
OBJECTS_DIR = obj

CONFIG += c++11

isEmpty(PREFIX) {
    PREFIX = /usr
}

#-----------------------------------------------------------------------------------------
# Qt configuration
#-----------------------------------------------------------------------------------------

TEMPLATE = app                                           # Project template
TARGET = serial-studio                                   # Set default target name
CONFIG += resources_big                                  # Avoid isses with large *.qrc
CONFIG += qtquickcompiler                                # Pre-compile QML interface

QTPLUGIN += qsvg                                         # Fixes issues with windeployqt

QT += xml
QT += sql
QT += svg
QT += core
QT += quick
QT += charts
QT += widgets
QT += serialport
QT += printsupport
QT += quickcontrols2

#-----------------------------------------------------------------------------------------
# Compiler options
#-----------------------------------------------------------------------------------------

*g++*: {
    QMAKE_CXXFLAGS_RELEASE -= -O
    QMAKE_CXXFLAGS_RELEASE *= -O3
}

*msvc*: {
    QMAKE_CXXFLAGS_RELEASE -= /O
    QMAKE_CXXFLAGS_RELEASE *= /O2
}

#-----------------------------------------------------------------------------------------
# Libraries
#-----------------------------------------------------------------------------------------

include(libs/Libraries.pri)

#-----------------------------------------------------------------------------------------
# Deploy options
#-----------------------------------------------------------------------------------------

win32* {
    TARGET = SerialStudio                                # Change target name
    RC_FILE = deploy/windows/resources/info.rc           # Set applicaiton icon
}

macx* {
    TARGET = SerialStudio                                # Change target name
    ICON = deploy/macOS/icon.icns                        # icon file
    RC_FILE = deploy/macOS/icon.icns                     # icon file
    QMAKE_INFO_PLIST = deploy/macOS/info.plist           # Add info.plist file
    CONFIG += sdk_no_version_check                       # Avoid warnings with Big Sur
}

target.path = $$PREFIX/bin

linux:!android {
    icon.path = $$PREFIX/share/pixmaps                   # icon instalation path
    desktop.path = $$PREFIX/share/applications           # *.desktop instalation path
    icon.files += deploy/linux/serial-studio.png         # Add application icon
    desktop.files += deploy/linux/serial-studio.desktop  # Add *.desktop file
    INSTALLS += target desktop icon                      # make install targets
}

mingw {
    license.path = $$PREFIX/share/licenses/$$TARGET      # Set license install path
    license.files += LICENSE.md                          # Add LICENSE.md file
    INSTALLS += target license                           # Install target+licence (MSYS2)
    DEFINES += DISABLE_QSU                               # Disable QSimpleUpdater (MSYS2)
}

#-----------------------------------------------------------------------------------------
# Import source code
#-----------------------------------------------------------------------------------------

INCLUDEPATH += src

RESOURCES += \
    assets/assets.qrc

DISTFILES += \
    assets/qml/PlatformDependent/*.qml \
    assets/qml/SetupPanes/*.qml \
    assets/qml/Widgets/*.qml \
    assets/qml/Windows/*.qml \
    assets/qml/SetupPanes/*.qml \
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
    src/JSON/FrameInfo.h \
    src/JSON/Generator.h \
    src/JSON/Group.h \
    src/MQTT/Publisher.h \
    src/Misc/ModuleManager.h \
    src/Misc/TimerEvents.h \
    src/Misc/Translator.h \
    src/Misc/Utilities.h \
    src/UI/DataProvider.h \
    src/UI/GraphProvider.h \
    src/UI/QmlPlainTextEdit.h \
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
    src/JSON/FrameInfo.cpp \
    src/JSON/Generator.cpp \
    src/JSON/Group.cpp \
    src/MQTT/Publisher.cpp \
    src/Misc/ModuleManager.cpp \
    src/Misc/TimerEvents.cpp \
    src/Misc/Translator.cpp \
    src/Misc/Utilities.cpp \
    src/UI/DataProvider.cpp \
    src/UI/GraphProvider.cpp \
    src/UI/QmlPlainTextEdit.cpp \
    src/UI/WidgetProvider.cpp \
    src/main.cpp
