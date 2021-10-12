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

TRANSLATIONS += \
    $$PWD/translations/en.ts \
    $$PWD/translations/es.ts \
    $$PWD/translations/zh.ts \
    $$PWD/translations/de.ts \
    $$PWD/translations/ru.ts

RESOURCES += \
    $$PWD/Resources.qrc

DISTFILES += \
    $$PWD/qml/Dashboard/DashboardTitle.qml \
    $$PWD/qml/Dashboard/ViewOptions.qml \
    $$PWD/qml/Dashboard/ViewOptionsDelegate.qml \
    $$PWD/qml/Dashboard/WidgetDelegate.qml \
    $$PWD/qml/Dashboard/WidgetGrid.qml \
    $$PWD/qml/Dashboard/WidgetModel.qml \
    $$PWD/qml/JsonEditor/JsonDatasetDelegate.qml \
    $$PWD/qml/JsonEditor/JsonGroupDelegate.qml \
    $$PWD/qml/Panes/Console.qml \
    $$PWD/qml/Panes/Dashboard.qml \
    $$PWD/qml/Panes/Setup.qml \
    $$PWD/qml/Panes/SetupPanes/MQTT.qml \
    $$PWD/qml/Panes/SetupPanes/Network.qml \
    $$PWD/qml/Panes/SetupPanes/Serial.qml \
    $$PWD/qml/Panes/SetupPanes/Settings.qml \
    $$PWD/qml/Panes/Toolbar.qml \
    $$PWD/qml/PlatformDependent/DecentMenuItem.qml \
    $$PWD/qml/PlatformDependent/Menubar.qml \
    $$PWD/qml/PlatformDependent/MenubarMacOS.qml \
    $$PWD/qml/Widgets/Icon.qml \
    $$PWD/qml/Widgets/JSONDropArea.qml \
    $$PWD/qml/Widgets/LED.qml \
    $$PWD/qml/Widgets/Shadow.qml \
    $$PWD/qml/Widgets/Window.qml \
    $$PWD/qml/Windows/About.qml \
    $$PWD/qml/Windows/Acknowledgements.qml \
    $$PWD/qml/Windows/CsvPlayer.qml \
    $$PWD/qml/Windows/Donate.qml \
    $$PWD/qml/Windows/JsonEditor.qml \
    $$PWD/qml/Windows/MainWindow.qml \
    $$PWD/qml/main.qml
