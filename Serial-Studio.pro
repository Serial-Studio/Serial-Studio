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

CONFIG += c++11
CONFIG += silent
CONFIG += strict_c++

sanitize {
    CONFIG += sanitizer
    CONFIG += sanitize_address
    CONFIG *= sanitize_undefined
}

QMAKE_CXXFLAGS *= -fno-math-errno
QMAKE_CXXFLAGS *= -funsafe-math-optimizations

#-----------------------------------------------------------------------------------------
# Serial Studio compile-time settings
#-----------------------------------------------------------------------------------------

#DEFINES += DISABLE_QSU     # If enabled, QSimpleUpdater shall not be used by the app.
                            # This is the default behaviour for MinGW.

DEFINES += LAZY_WIDGETS     # Compile-time option to reduce the CPU usage of the widgets.
                            # If disabled, widgets shall update title, units, value, etc.
                            # If enabled, widgets shall only update their value.

#-----------------------------------------------------------------------------------------
# Libraries
#-----------------------------------------------------------------------------------------

include(libs/Libraries.pri)

#-----------------------------------------------------------------------------------------
# Assets
#-----------------------------------------------------------------------------------------

include(assets/Assets.pri)

#-----------------------------------------------------------------------------------------
# Deploy options
#-----------------------------------------------------------------------------------------

win32* {
    TARGET = SerialStudio                                # Change target name
    RC_FILE = deploy/windows/resources/info.rc           # Set applicaiton icon
    OTHER_FILES += deploy/windows/nsis/setup.nsi         # Setup script
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
    icon.files += deploy/linux/serial-studio.svg         # Add application icon
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

HEADERS += \
    src/AppInfo.h \
    src/CSV/Export.h \
    src/CSV/Player.h \
    src/IO/Checksum.h \
    src/IO/Console.h \
    src/IO/DataSources/Network.h \
    src/IO/DataSources/Serial.h \
    src/IO/Manager.h \
    src/JSON/Dataset.h \
    src/JSON/Editor.h \
    src/JSON/Frame.h \
    src/JSON/FrameInfo.h \
    src/JSON/Generator.h \
    src/JSON/Group.h \
    src/MQTT/Client.h \
    src/Misc/MacExtras.h \
    src/Misc/ModuleManager.h \
    src/Misc/ThemeManager.h \
    src/Misc/TimerEvents.h \
    src/Misc/Translator.h \
    src/Misc/Utilities.h \
    src/Plugins/Bridge.h \
    src/UI/Dashboard.h \
    src/Widgets/Accelerometer.h \
    src/Widgets/AnalogGauge.h \
    src/Widgets/Bar.h \
    src/Widgets/Compass.h \
    src/Widgets/DataGroup.h \
    src/Widgets/Gauge.h \
    src/Widgets/Gyroscope.h \
    src/Widgets/Plot.h \
    src/Widgets/Terminal.h \
    src/Widgets/Thermometer.h \
    src/Widgets/WidgetLoader.h

SOURCES += \
    src/UI/Dashboard.cpp \
    src/Widgets/AnalogGauge.cpp \
    src/Widgets/WidgetLoader.cpp \
    src/main.cpp \
    src/CSV/Export.cpp \
    src/CSV/Player.cpp \
    src/IO/Checksum.cpp \
    src/IO/Console.cpp \
    src/IO/DataSources/Network.cpp \
    src/IO/DataSources/Serial.cpp \
    src/IO/Manager.cpp \
    src/JSON/Dataset.cpp \
    src/JSON/Editor.cpp \
    src/JSON/Frame.cpp \
    src/JSON/FrameInfo.cpp \
    src/JSON/Generator.cpp \
    src/JSON/Group.cpp \
    src/MQTT/Client.cpp \
    src/Misc/MacExtras.cpp \
    src/Misc/ModuleManager.cpp \
    src/Misc/ThemeManager.cpp \
    src/Misc/TimerEvents.cpp \
    src/Misc/Translator.cpp \
    src/Misc/Utilities.cpp \
    src/Plugins/Bridge.cpp \
    src/Widgets/Accelerometer.cpp \
    src/Widgets/Bar.cpp \
    src/Widgets/Compass.cpp \
    src/Widgets/DataGroup.cpp \
    src/Widgets/Gauge.cpp \
    src/Widgets/Gyroscope.cpp \
    src/Widgets/Plot.cpp \
    src/Widgets/Terminal.cpp \
    src/Widgets/Thermometer.cpp

DISTFILES += \
    assets/qml/Dashboard/DashboardTitle.qml \
    assets/qml/Dashboard/ViewOptions.qml \
    assets/qml/Dashboard/ViewOptionsDelegate.qml \
    assets/qml/Dashboard/WidgetDelegate.qml \
    assets/qml/Dashboard/WidgetGrid.qml \
    assets/qml/Dashboard/WidgetModel.qml \
    assets/qml/JsonEditor/JsonDatasetDelegate.qml \
    assets/qml/JsonEditor/JsonGroupDelegate.qml \
    assets/qml/Panes/Console.qml \
    assets/qml/Panes/Dashboard.qml \
    assets/qml/Panes/Setup.qml \
    assets/qml/Panes/SetupPanes/MQTT.qml \
    assets/qml/Panes/SetupPanes/Network.qml \
    assets/qml/Panes/SetupPanes/Serial.qml \
    assets/qml/Panes/SetupPanes/Settings.qml \
    assets/qml/Panes/Toolbar.qml \
    assets/qml/PlatformDependent/DecentMenuItem.qml \
    assets/qml/PlatformDependent/Menubar.qml \
    assets/qml/PlatformDependent/MenubarMacOS.qml \
    assets/qml/Widgets/JSONDropArea.qml \
    assets/qml/Widgets/LED.qml \
    assets/qml/Widgets/Shadow.qml \
    assets/qml/Widgets/Window.qml \
    assets/qml/Windows/About.qml \
    assets/qml/Windows/Acknowledgements.qml \
    assets/qml/Windows/CsvPlayer.qml \
    assets/qml/Windows/Donate.qml \
    assets/qml/Windows/JsonEditor.qml \
    assets/qml/Windows/MainWindow.qml \
    assets/qml/main.qml

#-------------------------------------------------------------------------------
# Deploy files
#-------------------------------------------------------------------------------

OTHER_FILES += \
    deploy/linux/* \
    deploy/macOS/* \
    deploy/windows/nsis/* \
    deploy/windows/resources/* \
    .github/workflows/Build.yml
