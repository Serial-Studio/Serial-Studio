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
CONFIG += utf8_source                                    # Source code encoding
QTPLUGIN += qsvg                                         # Fixes issues with windeployqt

QT += xml
QT += svg
QT += core
QT += quick
QT += widgets
QT += serialport
QT += printsupport
QT += quickcontrols2

equals(QT_MAJOR_VERSION, 6) {
    QT += core5compat
}

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

#-----------------------------------------------------------------------------------------
# Compiler options
#-----------------------------------------------------------------------------------------

CONFIG += c++11
CONFIG += silent

CONFIG(debug, debug|release) {
    CONFIG += sanitizer
    CONFIG += sanitize_address
}

CONFIG(release, debug|release) {
    CONFIG += unity_build

    *msvc*: {
        QMAKE_CXXFLAGS *= -MP
        INCLUDEPATH += $$OUT_PWD
        INCLUDEPATH += $$OUT_PWD/debug
        INCLUDEPATH += $$OUT_PWD/release
    }
}

CONFIG(unity_build) {
    CONFIG  += ltcg                             # Enable linker optimization
    DEFINES += UNITY_BUILD=1                    # Enable unity build
    SOURCES += src/SingleCompilationUnit.cpp    # Include single compilation unit in code
}

#-----------------------------------------------------------------------------------------
# Serial Studio compile-time settings
#-----------------------------------------------------------------------------------------

DEFINES += SERIAL_STUDIO_INCLUDE_MOC

#DEFINES += DISABLE_QSU     # If enabled, QSimpleUpdater shall not be used by the app.
                            # This is the default behaviour for MinGW.

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
    src/DataTypes.h \
    src/IO/Checksum.h \
    src/IO/Console.h \
    src/IO/DataSources/Network.h \
    src/IO/DataSources/Serial.h \
    src/IO/Manager.h \
    src/JSON/Dataset.h \
    src/JSON/Editor.h \
    src/JSON/Frame.h \
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
    src/UI/DashboardWidget.h \
    src/UI/DeclarativeWidget.h \
    src/UI/Widgets/Terminal.h \
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
    src/UI/Widgets/Plot.h

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
    src/UI/DashboardWidget.cpp \
    src/UI/DeclarativeWidget.cpp \
    src/UI/Widgets/Terminal.cpp \
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
    src/main.cpp

#-----------------------------------------------------------------------------------------
# Import QML source code
#-----------------------------------------------------------------------------------------

DISTFILES += \
    assets/qml/*.qml \
    assets/qml/Panes/*.qml \
    assets/qml/Widgets/*.qml \
    assets/qml/Windows/*.qml \
    assets/qml/Dashboard/*.qml \
    assets/qml/JsonEditor/*.qml \
    assets/qml/FramelessWindow/*.qml \
    assets/qml/Panes/SetupPanes/*.qml \
    assets/qml/PlatformDependent/*.qml

#-----------------------------------------------------------------------------------------
# Deploy files
#-----------------------------------------------------------------------------------------

OTHER_FILES += \
    deploy/linux/* \
    deploy/macOS/* \
    deploy/windows/nsis/* \
    deploy/windows/resources/* \
    .github/workflows/*.yml \
    updates.json
