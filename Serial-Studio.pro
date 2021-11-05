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
CONFIG += qtquickcompiler                                # Pre-compile QML interface

QTPLUGIN += qsvg                                         # Fixes issues with windeployqt

QT += xml
QT += sql
QT += svg
QT += core
QT += quick
QT += widgets
QT += serialport
QT += core5compat
QT += printsupport
QT += quickcontrols2

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

#-----------------------------------------------------------------------------------------
# Compiler options
#-----------------------------------------------------------------------------------------

*g++*: {
    QMAKE_CXXFLAGS_RELEASE -= -O1
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -Ofast
    QMAKE_CXXFLAGS_RELEASE *= -flto
}

*clang*: {
    QMAKE_CXXFLAGS_RELEASE -= -O1
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -Ofast
    QMAKE_CXXFLAGS_RELEASE *= -flto
}

*msvc*: {
    QMAKE_CXXFLAGS += -MP
    QMAKE_CXXFLAGS_RELEASE -= /O
    QMAKE_CXXFLAGS_RELEASE *= /O2
    QMAKE_CXXFLAGS_RELEASE *= /GL
    QMAKE_CXXFLAGS_RELEASE *= /LTCG

    INCLUDEPATH += $$OUT_PWD
    INCLUDEPATH += $$OUT_PWD/debug
    INCLUDEPATH += $$OUT_PWD/release
}

CONFIG += c++11
CONFIG += silent

#-----------------------------------------------------------------------------------------
# Enable/Disable single unit build depending on build configuration
#-----------------------------------------------------------------------------------------

CONFIG(debug, debug|release) {
    DEFINES += UNITY_BUILD=0
} else {
    DEFINES += UNITY_BUILD=1
}

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

linux:!android {
    PKGCONFIG += libssl                                  # Add OpenSSL library
    target.path = $$PREFIX/bin                           # Set binary installation path
    icon.path = $$PREFIX/share/pixmaps                   # icon instalation path
    desktop.path = $$PREFIX/share/applications           # *.desktop instalation path
    icon.files += deploy/linux/serial-studio.png         # Add application icon
    desktop.files += deploy/linux/serial-studio.desktop  # Add *.desktop file
    copyright.files += deploy/linux/copyright            # Libc6 file for linuxdeployqt
    copyright.path = $$PREFIX/share/doc/libc6            # libc6 copyright path
    INSTALLS += target copyright desktop icon            # make install targets
}

mingw {
    target.path = $$PREFIX/bin
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
    src/Plugins/Server.h \
    src/UI/Dashboard.h \
    src/UI/WidgetLoader.h \
    src/Widgets/Accelerometer.h \
    src/Widgets/Bar.h \
    src/Widgets/Common/AnalogGauge.h \
    src/Widgets/Common/AttitudeIndicator.h \
    src/Widgets/Common/BaseWidget.h \
    src/Widgets/Common/KLed.h \
    src/Widgets/Compass.h \
    src/Widgets/DataGroup.h \
    src/Widgets/FFTPlot.h \
    src/Widgets/GPS.h \
    src/Widgets/Gauge.h \
    src/Widgets/Gyroscope.h \
    src/Widgets/LEDPanel.h \
    src/Widgets/MultiPlot.h \
    src/Widgets/Plot.h \
    src/Widgets/Terminal.h

SOURCES += \
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
    src/Plugins/Server.cpp \
    src/UI/Dashboard.cpp \
    src/UI/WidgetLoader.cpp \
    src/Widgets/Accelerometer.cpp \
    src/Widgets/Bar.cpp \
    src/Widgets/Common/AnalogGauge.cpp \
    src/Widgets/Common/AttitudeIndicator.cpp \
    src/Widgets/Common/BaseWidget.cpp \
    src/Widgets/Common/KLed.cpp \
    src/Widgets/Compass.cpp \
    src/Widgets/DataGroup.cpp \
    src/Widgets/FFTPlot.cpp \
    src/Widgets/GPS.cpp \
    src/Widgets/Gauge.cpp \
    src/Widgets/Gyroscope.cpp \
    src/Widgets/LEDPanel.cpp \
    src/Widgets/MultiPlot.cpp \
    src/Widgets/Plot.cpp \
    src/Widgets/Terminal.cpp \
    src/main.cpp \
    src/SingleCompilationUnit.cpp

#-------------------------------------------------------------------------------
# Deploy files
#-------------------------------------------------------------------------------

OTHER_FILES += \
    deploy/linux/* \
    deploy/macOS/* \
    deploy/windows/nsis/* \
    deploy/windows/resources/* \
    .github/workflows/*.yml \
    updates.json
