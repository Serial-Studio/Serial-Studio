/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "Windows"

Item {
    id: app

    //
    // Global properties
    //
    readonly property int spacing: 8
    readonly property string monoFont: {
        switch (Qt.platform.os) {
        case "osx":
            return "Menlo"
        case "windows":
            return "Consolas"
        default:
            return "Monospace"
        }
    }

    //
    // Startup code
    //
    Component.onCompleted: {
        mainWindow.visible = true
        devicesWindow.visible = true
        CppJsonParser.readSettings()
    }

    //
    // About window
    //
    About {
        id: aboutWindow
    }

    //
    // Console window
    //
    Console {
        id: consoleWindow
        x: 2 * app.spacing
        y: 2 * app.spacing + mainWindow.y + mainWindow.height + 32
        height: Screen.desktopAvailableHeight - y - 2 * app.spacing
        width: Screen.desktopAvailableWidth - 6 * app.spacing - devicesWindow.width
    }

    //
    // Data window
    //
    DataGrid {
        id: dataWindow
        x: 2 * app.spacing
        y: 2 * app.spacing + mainWindow.y + mainWindow.height + 32
        height: Screen.desktopAvailableHeight - y - 2 * app.spacing
        width: Screen.desktopAvailableWidth - 6 * app.spacing - devicesWindow.width

        property bool alreadyShown: false

        Timer {
            id: timer
            interval: 1000
            onTriggered: {
                dataWindow.show()
                dataWindow.alreadyShown = true
            }
        }

        Connections {
            target: CppJsonParser
            function onPacketReceived()  {
                if (!dataWindow.visible && !dataWindow.alreadyShown)
                    timer.start()
            }
        }
    }

    //
    // Devices window
    //
    DeviceManager {
        id: devicesWindow
        x: Screen.desktopAvailableWidth - width - 2 * app.spacing
        y: 2 * app.spacing + mainWindow.y + mainWindow.height + 32
    }

    //
    // Main window
    //
    MainWindow {
        id: mainWindow
        dataChecked: dataWindow.visible
        aboutChecked: aboutWindow.visible
        consoleChecked: consoleWindow.visible
        widgetsChecked: widgetsWindow.visible
        devicesChecked: devicesWindow.visible
        onDataClicked: dataWindow.visible ? dataWindow.hide() : dataWindow.show()
        onAboutClicked: aboutWindow.visible ? aboutWindow.hide() : aboutWindow.show()
        onConsoleClicked: consoleWindow.visible ? consoleWindow.hide() : consoleWindow.show()
        onWidgetsClicked: widgetsWindow.visible ? widgetsWindow.hide() : widgetsWindow.show()
        onDevicesClicked: devicesWindow.visible ? devicesWindow.hide() : devicesWindow.show()
    }

    //
    // Widgets window
    //
    Widgets {
        id: widgetsWindow
        x: 2 * app.spacing
        y: 2 * app.spacing + mainWindow.y + mainWindow.height + 32
        height: Screen.desktopAvailableHeight - y - 2 * app.spacing
        width: Screen.desktopAvailableWidth - 6 * app.spacing - devicesWindow.width
    }
}
