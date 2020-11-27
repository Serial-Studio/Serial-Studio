#
# Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
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
QT += svg
QT += core
QT += quick
QT += charts
QT += widgets
QT += serialport
QT += quickcontrols2

QTPLUGIN += qsvg

#-------------------------------------------------------------------------------
# Libraries
#-------------------------------------------------------------------------------

include(libs/QSimpleUpdater/QSimpleUpdater.pri)

#-------------------------------------------------------------------------------
# Deploy options
#-------------------------------------------------------------------------------

win32* {
    TARGET = Serial-Studio
    RC_FILE = deploy/windows/resources/info.rc
}

macx* {
    CONFIG += sdk_no_version_check

    TARGET = "Serial Studio"

    ICON = deploy/macOS/icon.icns
    RC_FILE = deploy/macOS/icon.icns
    QMAKE_INFO_PLIST = deploy/macOS/info.plist

    # DMG generation constants
    DMG_FILENAME = "SerialStudio.dmg"
    BUNDLE_FILENAME = "Serial Studio.app"
   
    # Target for pretty DMG generation
    dmg.commands += "macdeployqt $$BUNDLE_FILENAME -qmldir=$$PWD/assets/qml &&"
    dmg.commands += "create-dmg \
          --volname $${TARGET} \
          --background $${PWD}/deploy/macOS/dmg_bg.png \
          --icon $${BUNDLE_FILENAME} 150 218 \
          --window-pos 200 120 \
          --window-size 600 450 \
          --icon-size 100 \
          --hdiutil-quiet \
          --app-drop-link 450 218 \
          $${DMG_FILENAME} \
          $${BUNDLE_FILENAME}"

    QMAKE_EXTRA_TARGETS += dmg
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
# Import source code
#-------------------------------------------------------------------------------

RESOURCES += \
    assets/assets.qrc

HEADERS += \
    src/AppInfo.h \
    src/Dataset.h \
    src/Export.h \
    src/GraphProvider.h \
    src/Group.h \
    src/JsonParser.h \
    src/ModuleManager.h \
    src/QmlBridge.h \
    src/SerialManager.h \
    src/Widgets.h

SOURCES += \
    src/Dataset.cpp \
    src/Export.cpp \
    src/GraphProvider.cpp \
    src/Group.cpp \
    src/JsonParser.cpp \
    src/ModuleManager.cpp \
    src/QmlBridge.cpp \
    src/SerialManager.cpp \
    src/Widgets.cpp \
    src/main.cpp

DISTFILES += \
    assets/qml/Components/Console.qml \
    assets/qml/Components/DataGrid.qml \
    assets/qml/Components/DeviceManager.qml \
    assets/qml/Components/GraphGrid.qml \
    assets/qml/Components/ToolBar.qml \
    assets/qml/UI.qml \
    assets/qml/Widgets/DataDelegate.qml \
    assets/qml/Widgets/GraphDelegate.qml \
    assets/qml/Widgets/GroupDelegate.qml \
    assets/qml/Widgets/LED.qml \
    assets/qml/Widgets/MapDelegate.qml \
    assets/qml/Widgets/Window.qml \
    assets/qml/main.qml \
    assets/qml/About.qml
