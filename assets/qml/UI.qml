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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

import "Widgets" as Widgets
import "Components" as Components


Page {
    id: page

    //
    // Toolbar
    //
    header: Components.ToolBar {
        id: toolbar
        onAboutClicked: about.show()
    }

    //
    // Widgets
    //
    RowLayout {
        spacing: app.spacing
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Data display & console
        //
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            //
            // Data display widget
            //
            Widgets.Window {
                id: dataDis
                gradient: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                opacity: enabled ? 1 : 0
                title: qsTr("Data Display")
                enabled: toolbar.dataDisChecked

                TabBar {
                    id: tabBar
                    contentHeight: 32
                    palette.button: Qt.rgba(45/255, 96/255, 115/255, 1)
                    visible: CppQmlBridge.gpsSupported
                    enabled: CppQmlBridge.groupCount > 0
                    opacity: CppQmlBridge.groupCount > 0 ? 1 : 0
                    Behavior on opacity {NumberAnimation{}}

                    onVisibleChanged: dataButton.clicked()

                    anchors {
                        leftMargin: 3
                        rightMargin: 3
                        topMargin: -2
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }

                    TabButton {
                        id: dataButton
                        text: qsTr("Data")
                    }

                    TabButton {
                        text: qsTr("GPS Map")
                        enabled: CppQmlBridge.gpsSupported
                    }
                }

                Rectangle {
                    height: 1
                    color: palette.base
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        leftMargin: dataDis.borderWidth
                        rightMargin: dataDis.borderWidth
                    }
                }

                SwipeView {
                    clip: true
                    interactive: false
                    anchors.fill: parent
                    anchors.margins: dataDis.borderWidth
                    anchors.topMargin: tabBar.visible ? tabBar.height : dataDis.borderWidth
                    currentIndex: tabBar.currentIndex
                    enabled: CppQmlBridge.groupCount > 0
                    opacity: CppQmlBridge.groupCount > 0 ? 1 : 0
                    Behavior on opacity {NumberAnimation{}}

                    Components.DataGrid {
                        id: dataGrid
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Widgets.MapDelegate {
                            id: gpsMap
                            anchors.fill: parent
                            anchors.margins: app.spacing * 2
                        }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    enabled: CppQmlBridge.groupCount == 0
                    opacity: CppQmlBridge.groupCount == 0 ? 1 : 0

                    Behavior on opacity {NumberAnimation{}}

                    Image {
                        width: sourceSize.width
                        height: sourceSize.height
                        sourceSize: Qt.size(128, 128)
                        source: "qrc:/icons/warning.svg"
                        Layout.alignment: Qt.AlignHCenter

                        ColorOverlay {
                            source: parent
                            color: "#ffaa44"
                            anchors.fill: parent
                        }
                    }

                    Label {
                        font.pixelSize: 18
                        text: qsTr("No data available")
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        opacity: 0.5
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                        text: CppSerialManager.connected ?
                                  qsTr("Check the serial console to troubleshoot the problem") :
                                  qsTr("No device connected, please select a COM port in the device manager pane")
                    }
                }
            }

            //
            // Console
            //
            Components.Console {
                id: consoleWindow
                gradient: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                opacity: enabled ? 1 : 0
                enabled: toolbar.consoleChecked
                Layout.maximumHeight: !dataDis.enabled ? page.height : implicitHeight

                Behavior on Layout.maximumHeight {NumberAnimation{}}
            }
        }

        //
        // Devices hub
        //
        Components.DeviceManager {
            id: devMan
            gradient: true
            Layout.fillHeight: true
            opacity: enabled ? 1 : 0
            enabled: toolbar.devicesChecked
        }
    }

    //
    // About window
    //
    About {
        id: about
        visible: false
        palette: app.palette
    }
}
