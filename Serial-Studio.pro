#
# Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

isEmpty(PREFIX) {
    PREFIX = /usr
}

#-------------------------------------------------------------------------------
# Qt configuration
#-------------------------------------------------------------------------------

TEMPLATE = app
TARGET = serial-studio
CONFIG += qtquickcompiler
CONFIG += utf8_source
QTPLUGIN += qsvg

QT += xml
QT += svg
QT += core
QT += quick
QT += widgets
QT += location
QT += bluetooth
QT += serialport
QT += positioning
QT += printsupport

QT += quickcontrols2

#-------------------------------------------------------------------------------
# Compiler options
#-------------------------------------------------------------------------------

CONFIG += c++11
CONFIG += silent

CONFIG(release, debug|release) {
    CONFIG += ltcg
    *msvc*: {
        QMAKE_CXXFLAGS *= -MP
        INCLUDEPATH += $$OUT_PWD
        INCLUDEPATH += $$OUT_PWD/debug
        INCLUDEPATH += $$OUT_PWD/release
    }
}

#-------------------------------------------------------------------------------
# Serial Studio compile-time settings
#-------------------------------------------------------------------------------

#DEFINES += DISABLE_QS # If enabled, QSimpleUpdater shall not be used by the app
                       # This is the default behaviour for MinGW.

#-------------------------------------------------------------------------------
# Libraries
#-------------------------------------------------------------------------------

include(libs/Libraries.pri)

#-------------------------------------------------------------------------------
# Assets
#-------------------------------------------------------------------------------

include(assets/Assets.pri)

#-------------------------------------------------------------------------------
# Deploy options
#-------------------------------------------------------------------------------

win32* {
    TARGET = SerialStudio
    RC_FILE = deploy/windows/resources/info.rc
    OTHER_FILES += deploy/windows/nsis/setup.nsi
}

macx* {
    TARGET = SerialStudio
    ICON = deploy/macOS/icon.icns
    RC_FILE = deploy/macOS/icon.icns
    QMAKE_INFO_PLIST = deploy/macOS/info.plist
    CONFIG += sdk_no_version_check
}

linux:!android {
    PKGCONFIG += libssl
    target.path = $$PREFIX/bin
    icon.path = $$PREFIX/share/pixmaps
    desktop.path = $$PREFIX/share/applications
    icon.files += deploy/linux/serial-studio.png
    desktop.files += deploy/linux/serial-studio.desktop
    INSTALLS += target copyright desktop icon
}

mingw {
    target.path = $$PREFIX/bin
    license.path = $$PREFIX/share/licenses/$$TARGET
    license.files += LICENSE.md
    INSTALLS += target license
    DEFINES += DISABLE_QSU
}

#-------------------------------------------------------------------------------
# Import source code
#-------------------------------------------------------------------------------

INCLUDEPATH += src

HEADERS += \
    src/AppInfo.h \
    src/CSV/Export.h \
    src/CSV/Player.h \
    src/DataTypes.h \
    src/IO/Checksum.h \
    src/IO/Console.h \
    src/IO/Drivers/BluetoothLE.h \
    src/IO/Drivers/Network.h \
    src/IO/Drivers/Serial.h \
    src/IO/HAL_Driver.h \
    src/IO/Manager.h \
    src/JSON/Dataset.h \
    src/JSON/Frame.h \
    src/JSON/Generator.h \
    src/JSON/Group.h \
    src/MQTT/Client.h \
    src/Misc/ModuleManager.h \
    src/Misc/ThemeManager.h \
    src/Misc/TimerEvents.h \
    src/Misc/Translator.h \
    src/Misc/Utilities.h \
    src/Plugins/Server.h \
    src/Project/CodeEditor.h \
    src/Project/Model.h \
    src/UI/Dashboard.h \
    src/UI/DashboardWidget.h \
    src/UI/DeclarativeWidget.h \
    src/UI/Widgets/Accelerometer.h \
    src/UI/Widgets/Bar.h \
    src/UI/Widgets/Common/AnalogGauge.h \
    src/UI/Widgets/Common/AttitudeIndicator.h \
    src/UI/Widgets/Common/BaseWidget.h \
    src/UI/Widgets/Common/ElidedLabel.h \
    src/UI/Widgets/Common/KLed.h \
    src/UI/Widgets/Compass.h \
    src/UI/Widgets/DataGroup.h \
    src/UI/Widgets/FFTPlot.h \
    src/UI/Widgets/GPS.h \
    src/UI/Widgets/Gauge.h \
    src/UI/Widgets/Gyroscope.h \
    src/UI/Widgets/LEDPanel.h \
    src/UI/Widgets/MultiPlot.h \
    src/UI/Widgets/Plot.h \
    src/UI/Widgets/Terminal.h

SOURCES += \
    src/CSV/Export.cpp \
    src/CSV/Player.cpp \
    src/IO/Checksum.cpp \
    src/IO/Console.cpp \
    src/IO/Drivers/BluetoothLE.cpp \
    src/IO/Drivers/Network.cpp \
    src/IO/Drivers/Serial.cpp \
    src/IO/Manager.cpp \
    src/JSON/Dataset.cpp \
    src/JSON/Frame.cpp \
    src/JSON/Generator.cpp \
    src/JSON/Group.cpp \
    src/MQTT/Client.cpp \
    src/Misc/ModuleManager.cpp \
    src/Misc/ThemeManager.cpp \
    src/Misc/TimerEvents.cpp \
    src/Misc/Translator.cpp \
    src/Misc/Utilities.cpp \
    src/Plugins/Server.cpp \
    src/Project/CodeEditor.cpp \
    src/Project/Model.cpp \
    src/UI/Dashboard.cpp \
    src/UI/DashboardWidget.cpp \
    src/UI/DeclarativeWidget.cpp \
    src/UI/Widgets/Accelerometer.cpp \
    src/UI/Widgets/Bar.cpp \
    src/UI/Widgets/Common/AnalogGauge.cpp \
    src/UI/Widgets/Common/AttitudeIndicator.cpp \
    src/UI/Widgets/Common/BaseWidget.cpp \
    src/UI/Widgets/Common/ElidedLabel.cpp \
    src/UI/Widgets/Common/KLed.cpp \
    src/UI/Widgets/Compass.cpp \
    src/UI/Widgets/DataGroup.cpp \
    src/UI/Widgets/FFTPlot.cpp \
    src/UI/Widgets/GPS.cpp \
    src/UI/Widgets/Gauge.cpp \
    src/UI/Widgets/Gyroscope.cpp \
    src/UI/Widgets/LEDPanel.cpp \
    src/UI/Widgets/MultiPlot.cpp \
    src/UI/Widgets/Plot.cpp \
    src/UI/Widgets/Terminal.cpp \
    src/main.cpp

#-------------------------------------------------------------------------------
# Import QML source code
#-------------------------------------------------------------------------------

DISTFILES += \
    assets/qml/*.qml \
    assets/qml/Panes/*.qml \
    assets/qml/Widgets/*.qml \
    assets/qml/Windows/*.qml \
    assets/qml/Dashboard/*.qml \
    assets/qml/ProjectEditor/*.qml \
    assets/qml/FramelessWindow/*.qml \
    assets/qml/Panes/SetupPanes/*.qml \
    assets/qml/PlatformDependent/*.qml \
    assets/qml/Panes/SetupPanes/Devices/*.qml \

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
