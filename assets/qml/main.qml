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

import Qt.labs.settings 1.0

import "Windows"

ApplicationWindow {
    id: app

    //
    // Global properties
    //
    readonly property int spacing: 8
    readonly property color windowBackgroundColor: Qt.rgba(18/255, 25/255, 32/255, 1)
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
    // Window geometry
    //
    visible: false
    minimumWidth: 960
    minimumHeight: 640
    title: CppAppName + " v" + CppAppVersion

    //
    // Theme options
    //
    palette.text: Qt.rgba(1, 1, 1, 1)
    palette.buttonText: Qt.rgba(1, 1, 1, 1)
    palette.windowText: Qt.rgba(1, 1, 1, 1)
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Startup code
    //
    Component.onCompleted: {
        timer.start()

        about.hide()
        devices.show()

        data.opacity = 0
        console.opacity = 1
        widgets.opacity = 0

        CppJsonParser.readSettings()
    }

    //
    // Startup timer
    //
    Timer {
        id: timer
        interval: 500
        onTriggered: {
            // Startup verifications to ensure that bad settings
            // do not make our app reside outside screen
            if (x < 0 || x >= Screen.desktopAvailableWidth)
                x = 100
            if (y < 0 || y >= Screen.desktopAvailableHeight)
                y = 100

            // Startup verifications to ensure that app fits in current screen
            if (width > Screen.desktopAvailableWidth) {
                x = 100
                width = Screen.desktopAvailableWidth - x
            }

            // Startup verifications to ensure that app fits in current screen
            if (height > Screen.desktopAvailableHeight) {
                y = 100
                height = Screen.desktopAvailableHeight - y
            }

            // Show app window
            app.visible = true
        }
    }

    //
    // Save window size & position
    //
    Settings {
        property alias appX: app.x
        property alias appY: app.y
        property alias appW: app.width
        property alias appH: app.height
    }

    //
    // About window
    //
    About {
        id: about
    }

    //
    // Main layout
    //
    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        Toolbar {
            z: 1
            id: toolbar
            Layout.fillWidth: true
            Layout.minimumHeight: 48
            Layout.maximumHeight: 48
            dataChecked: data.visible
            aboutChecked: about.visible
            consoleChecked: console.visible
            widgetsChecked: widgets.visible
            devicesChecked: devices.visible
            onAboutClicked: about.visible ? about.hide() : about.show()
            onDevicesClicked: devices.visible ? devices.hide() : devices.show()

            onDataClicked: {
                data.opacity    = 1
                console.opacity = 0
                widgets.opacity = 0
                dataChecked     = true
                consoleChecked  = false
                widgetsChecked  = false
            }

            onConsoleClicked: {
                data.opacity    = 0
                console.opacity = 1
                widgets.opacity = 0
                dataChecked     = false
                consoleChecked  = true
                widgetsChecked  = false
            }

            onWidgetsClicked: {
                data.opacity    = 0
                console.opacity = 0
                widgets.opacity = 1
                dataChecked     = false
                widgetsChecked  = true
                consoleChecked  = false
            }
        }

        RowLayout {
            spacing: 0
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Console {
                    id: console
                    anchors.fill: parent
                    text: CppWelcomeText + "\n\n"

                    // Animate on show
                    enabled: opacity > 0
                    visible: opacity > 0
                    Behavior on opacity {NumberAnimation{}}
                }

                DataGrid {
                    id: data
                    anchors.fill: parent
                    property bool alreadyShown: false

                    // Animate on show
                    visible: opacity > 0
                    Behavior on opacity {NumberAnimation{}}

                    // Show data view when first valid packet is received
                    Connections {
                        target: CppJsonParser
                        function onPacketReceived()  {
                            if (!data.visible && !data.alreadyShown) {
                                data.alreadyShown = true
                                data.opacity      = 1
                                console.opacity   = 0
                                widgets.opacity   = 0
                                devices.hide()
                            }
                        }
                    }
                }

                Widgets {
                    id: widgets
                    anchors.fill: parent

                    // Animate on show
                    enabled: opacity > 0
                    visible: opacity > 0
                    Behavior on opacity {NumberAnimation{}}
                }
            }

            DeviceManager {
                id: devices
                property int displayedWidth: 320

                function show() {
                    opacity = 1
                    displayedWidth = 320
                }

                function hide() {
                    opacity = 0
                    displayedWidth = 0
                }

                visible: opacity > 0
                Layout.fillHeight: true
                Layout.minimumWidth: displayedWidth
                Layout.maximumWidth: displayedWidth

                Behavior on opacity {NumberAnimation{}}
                Behavior on displayedWidth {NumberAnimation{}}
            }
        }
    }
}
