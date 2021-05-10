/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

import QtQuick.Window 2.12 as QtWindow

import "../Widgets" as Widgets

Control {
    id: root
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Custom properties
    //
    readonly property int minimumWidgetSize: 320

    //
    // Group/dataset updating
    //
    Connections {
        target: Cpp_UI_WidgetProvider

        //*! Optimize this function
        function onWidgetCountChanged() {
            // Generate accelerometer widgets
            accGenerator.model = 0
            accGenerator.model = Cpp_UI_WidgetProvider.accelerometerGroupCount()

            // Generate gyro widgets
            gyroGenerator.model = 0
            gyroGenerator.model = Cpp_UI_WidgetProvider.gyroGroupCount()

            // Generate bar widgets
            barGenerator.model = 0
            barGenerator.model = Cpp_UI_WidgetProvider.barDatasetCount()

            // Generate compass widgets
            compassGenerator.model = 0
            compassGenerator.model = Cpp_UI_WidgetProvider.compassDatasetCount()

            // Generate map widgets
            mapGenerator.model = 0
            mapGenerator.model = Cpp_UI_WidgetProvider.mapGroupCount()
        }
    }

    //
    // UI inside scrollview
    //
    ScrollView {
        z: 0
        id: _sv
        clip: false
        contentWidth: -1
        anchors.fill: parent
        anchors.rightMargin: 5
        anchors.margins: app.spacing * 1.5

        ColumnLayout {
            x: 0
            width: _sv.width - 15

            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                rowSpacing: app.spacing
                columnSpacing: app.spacing
                columns: Math.floor(_sv.width / (root.minimumWidgetSize + 2 * app.spacing))

                Repeater {
                    id: gyroGenerator

                    delegate: Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: root.minimumWidgetSize
                        Layout.minimumHeight: root.minimumWidgetSize

                        Widgets.GyroDelegate {
                            groupIndex: index
                            anchors.fill: parent
                            onHeaderDoubleClicked: windowGyro.show()
                        }

                        QtWindow.Window {
                            id: windowGyro
                            width: 640
                            height: 480
                            minimumWidth: root.minimumWidgetSize * 1.2
                            minimumHeight: root.minimumWidgetSize * 1.2
                            title: gyro.title

                            Rectangle {
                                anchors.fill: parent
                                color: gyro.backgroundColor
                            }

                            Widgets.GyroDelegate {
                                id: gyro
                                showIcon: true
                                gradient: false
                                headerHeight: 48
                                groupIndex: index
                                anchors.margins: 0
                                anchors.fill: parent
                                borderColor: backgroundColor
                                headerDoubleClickEnabled: false
                            }
                        }
                    }
                }

                Repeater {
                    id: accGenerator

                    delegate: Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: root.minimumWidgetSize
                        Layout.minimumHeight: root.minimumWidgetSize

                        Widgets.AccelerometerDelegate {
                            groupIndex: index
                            anchors.fill: parent
                            onHeaderDoubleClicked: windowAcc.show()
                        }

                        QtWindow.Window {
                            id: windowAcc
                            width: 640
                            height: 480
                            minimumWidth: root.minimumWidgetSize * 1.2
                            minimumHeight: root.minimumWidgetSize * 1.2
                            title: acc.title

                            Rectangle {
                                anchors.fill: parent
                                color: acc.backgroundColor
                            }

                            Widgets.AccelerometerDelegate {
                                id: acc
                                showIcon: true
                                gradient: false
                                headerHeight: 48
                                groupIndex: index
                                anchors.margins: 0
                                anchors.fill: parent
                                borderColor: backgroundColor
                                headerDoubleClickEnabled: false
                            }
                        }
                    }
                }

                Repeater {
                    id: mapGenerator

                    delegate: Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: root.minimumWidgetSize
                        Layout.minimumHeight: root.minimumWidgetSize

                        Widgets.MapDelegate {
                            groupIndex: index
                            anchors.fill: parent
                            onHeaderDoubleClicked: windowMap.show()
                        }

                        QtWindow.Window {
                            id: windowMap
                            width: 640
                            height: 480
                            minimumWidth: root.minimumWidgetSize * 1.2
                            minimumHeight: root.minimumWidgetSize * 1.2
                            title: map.title

                            Rectangle {
                                anchors.fill: parent
                                color: map.backgroundColor
                            }

                            Widgets.MapDelegate {
                                id: map
                                showIcon: true
                                gradient: false
                                headerHeight: 48
                                groupIndex: index
                                anchors.margins: 0
                                anchors.fill: parent
                                borderColor: backgroundColor
                                headerDoubleClickEnabled: false
                            }
                        }
                    }
                }

                Repeater {
                    id: barGenerator

                    delegate: Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: root.minimumWidgetSize
                        Layout.minimumHeight: root.minimumWidgetSize

                        Widgets.BarDelegate {
                            datasetIndex: index
                            anchors.fill: parent
                            onHeaderDoubleClicked: windowBar.show()
                        }

                        QtWindow.Window {
                            id: windowBar
                            width: 640
                            height: 480
                            minimumWidth: root.minimumWidgetSize * 1.2
                            minimumHeight: root.minimumWidgetSize * 1.2
                            title: bar.title

                            Rectangle {
                                anchors.fill: parent
                                color: bar.backgroundColor
                            }

                            Widgets.BarDelegate {
                                id: bar
                                showIcon: true
                                gradient: false
                                headerHeight: 48
                                datasetIndex: index
                                anchors.margins: 0
                                anchors.fill: parent
                                borderColor: backgroundColor
                                headerDoubleClickEnabled: false
                            }
                        }
                    }
                }

                Repeater {
                    id: compassGenerator

                    delegate: Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: root.minimumWidgetSize
                        Layout.minimumHeight: root.minimumWidgetSize

                        Widgets.CompassDelegate {
                            datasetIndex: index
                            anchors.fill: parent
                            onHeaderDoubleClicked: windowCompass.show()
                        }

                        QtWindow.Window {
                            id: windowCompass
                            width: 640
                            height: 480
                            minimumWidth: root.minimumWidgetSize * 1.2
                            minimumHeight: root.minimumWidgetSize * 1.2
                            title: compass.title

                            Rectangle {
                                anchors.fill: parent
                                color: compass.backgroundColor
                            }

                            Widgets.CompassDelegate {
                                id: compass
                                showIcon: true
                                gradient: false
                                headerHeight: 48
                                datasetIndex: index
                                anchors.margins: 0
                                anchors.fill: parent
                                borderColor: backgroundColor
                                headerDoubleClickEnabled: false
                            }
                        }
                    }
                }
            }

            Item {
                Layout.minimumHeight: 10
            }
        }
    }
}
