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
        target: CppWidgets
        function onDataChanged() {
            // Generate accelerometer widgets
            if (accGenerator.model !== CppWidgets.accelerometerGroupCount()) {
                accGenerator.model = 0
                accGenerator.model = CppWidgets.accelerometerGroupCount()
            }

            // Generate gyro widgets
            if (gyroGenerator.model !== CppWidgets.gyroGroupCount()) {
                gyroGenerator.model = 0
                gyroGenerator.model = CppWidgets.gyroGroupCount()
            }

            // Generate bar widgets
            if (barGenerator.model !== CppWidgets.barDatasetCount()) {
                barGenerator.model = 0
                barGenerator.model = CppWidgets.barDatasetCount()
            }

            // Generate map widgets
            if (mapGenerator.model !== CppWidgets.mapGroupCount()) {
                mapGenerator.model = 0
                mapGenerator.model = CppWidgets.mapGroupCount()
            }
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
