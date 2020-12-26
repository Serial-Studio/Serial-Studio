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
import QtGraphicalEffects 1.0

import "../Widgets" as Widgets

ApplicationWindow {
    id: dataGrid
    title: qsTr("Data")

    //
    // Window geometry & position
    //
    minimumWidth: 860
    minimumHeight: 640

    //
    // Theme options
    //
    palette.text: Qt.rgba(1, 1, 1, 1)
    palette.buttonText: Qt.rgba(1, 1, 1, 1)
    palette.windowText: Qt.rgba(1, 1, 1, 1)
    background: Rectangle {
        color: Qt.rgba(18/255, 25/255, 32/255, 1)
    }

    //
    // Group/dataset updating
    //
    Connections {
        target: CppQmlBridge
        function onUpdated() {
            dataGrid.title = CppQmlBridge.projectTitle
            if (groupGenerator.model !== CppQmlBridge.groupCount) {
                var list = []
                for (var i = 0; i < CppQmlBridge.groupCount; ++i)
                    list.push(true)

                groupGenerator.model = 0
                viewOptions.groups = list
                groupGenerator.model = CppQmlBridge.groupCount
            }
        }
    }

    //
    // Graph data updating
    //
    Connections {
        target: CppGraphProvider
        function onDataUpdated() {
            if (graphGenerator.model !== CppGraphProvider.graphCount) {
                var list = []
                for (var i = 0; i < CppGraphProvider.graphCount; ++i)
                    list.push(true)

                graphGenerator.model = 0
                viewOptions.graphs = list
                graphGenerator.model = CppGraphProvider.graphCount
            }
        }
    }

    //
    // Main layout
    //
    ColumnLayout {
        x: 2 * app.spacing
        anchors.fill: parent
        spacing: app.spacing * 2
        anchors.margins: app.spacing * 1.5

        //
        // Group data & graphs
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true
            Layout.fillHeight: true

            //
            // View options
            //
            Widgets.Window {
                id: viewOptions
                title: qsTr("View")
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                icon.source: "qrc:/icons/visibility.svg"
                borderColor: Qt.rgba(45/255, 96/255, 115/255, 1)
                backgroundColor: Qt.rgba(18 / 255, 18 / 255, 24 / 255, 1)

                property var groups: []
                property var graphs: []

                ScrollView {
                    clip: true
                    contentWidth: -1
                    anchors.fill: parent
                    anchors.margins: app.spacing

                    ColumnLayout {
                        x: app.spacing
                        width: parent.width - 10 - 2 * app.spacing

                        Item {
                            height: app.spacing
                        }

                        //
                        // Horizontal range title
                        //
                        RowLayout {
                            spacing: app.spacing
                            visible: graphGenerator.count > 0

                            Image {
                                width: sourceSize.width
                                height: sourceSize.height
                                sourceSize: Qt.size(18, 18)
                                source: "qrc:/icons/scatter-plot.svg"

                                ColorOverlay {
                                    source: parent
                                    color: palette.text
                                    anchors.fill: parent
                                }
                            }

                            Label {
                                font.bold: true
                                text: qsTr("Horizontal Range") + ":"
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }

                        //
                        // Horizontal range slider
                        //
                        RowLayout {
                            spacing: app.spacing
                            visible: graphGenerator.count > 0

                            Slider {
                                id: slider
                                to: 100
                                from: 1
                                live: false
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                value: CppGraphProvider.displayedPoints
                                onValueChanged: CppGraphProvider.displayedPoints = value
                            }

                            Label {
                                font.family: app.monoFont
                                Layout.alignment: Qt.AlignVCenter
                                text: Math.ceil(slider.position * (slider.to))
                            }
                        }

                        //
                        // Spacer
                        //
                        Item {
                            height: app.spacing
                        }

                        //
                        // Group title
                        //
                        RowLayout {
                            spacing: app.spacing
                            visible: groupGenerator.count > 0

                            Image {
                                width: sourceSize.width
                                height: sourceSize.height
                                sourceSize: Qt.size(18, 18)
                                source: "qrc:/icons/group.svg"

                                ColorOverlay {
                                    source: parent
                                    color: palette.text
                                    anchors.fill: parent
                                }
                            }

                            Label {
                                font.bold: true
                                text: qsTr("Data Groups") + ":"
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }

                        //
                        // Semi-spacer
                        //
                        Item {
                            height: app.spacing / 2
                        }

                        //
                        // Group switches
                        //
                        Repeater {
                            model: groupGenerator.model
                            delegate: Switch {
                                Layout.fillWidth: true
                                Component.onCompleted: checked = true
                                text: CppQmlBridge.getGroup(index).title
                                palette.highlight: Qt.rgba(215/255, 45/255, 96/255, 1)
                                onCheckedChanged: {
                                    viewOptions.groups[index] = checked
                                    viewOptions.groupsChanged()
                                }
                            }
                        }

                        //
                        // Spacer
                        //
                        Item {
                            height: app.spacing
                        }

                        //
                        // Graphs title
                        //
                        RowLayout {
                            spacing: app.spacing
                            visible: groupGenerator.count > 0

                            Image {
                                width: sourceSize.width
                                height: sourceSize.height
                                sourceSize: Qt.size(18, 18)
                                source: "qrc:/icons/chart.svg"

                                ColorOverlay {
                                    source: parent
                                    color: palette.text
                                    anchors.fill: parent
                                }
                            }

                            Label {
                                font.bold: true
                                text: qsTr("Data Plots") + ":"
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }

                        //
                        // Semi-spacer
                        //
                        Item {
                            height: app.spacing / 2
                        }

                        //
                        // Graph switches
                        //
                        Repeater {
                            model: graphGenerator.model
                            delegate: Switch {
                                Layout.fillWidth: true
                                Component.onCompleted: checked = true
                                text: CppGraphProvider.getDataset(index).title
                                palette.highlight: Qt.rgba(215/255, 45/255, 96/255, 1)
                                onCheckedChanged: {
                                    viewOptions.graphs[index] = checked
                                    viewOptions.graphsChanged()
                                }
                            }
                        }
                    }
                }
            }

            //
            // Data grid
            //
            Widgets.Window {
                id: dataWin
                title: qsTr("Data")
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                icon.source: "qrc:/icons/scatter-plot.svg"
                borderColor:  Qt.rgba(45/255, 96/255, 115/255, 1)
                backgroundColor: Qt.rgba(18 / 255, 18 / 255, 24 / 255, 1)

                Rectangle {
                    z: 1
                    color: dataWin.borderColor
                    height: dataWin.borderWidth

                    anchors {
                        leftMargin: 5
                        rightMargin: 5
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                }

                ScrollView {
                    z: 0
                    id: _sv
                    clip: false
                    contentWidth: -1
                    anchors.fill: parent
                    anchors.rightMargin: 10
                    anchors.margins: app.spacing * 2
                    anchors.leftMargin: app.spacing * 2 + 10

                    ColumnLayout {
                        width: _sv.width - 2 * app.spacing

                        Item {
                            Layout.minimumHeight: 10
                        }

                        GridLayout {
                            rowSpacing: 0
                            columnSpacing: 0
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            columns: Math.floor(width / 224)

                            Repeater {
                                id: groupGenerator

                                delegate: Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.minimumHeight: groupDelegate.visible ? 196 : 0

                                    Behavior on Layout.minimumHeight {NumberAnimation{}}

                                    Widgets.GroupDelegate {
                                        id: groupDelegate
                                        groupIndex: index
                                        anchors.fill: parent
                                        anchors.margins: app.spacing
                                        group: CppQmlBridge.getGroup(index)
                                        enabled: viewOptions.groups[groupIndex]

                                        Connections {
                                            target: viewOptions
                                            function onGroupsChanged() {
                                                groupDelegate.enabled = viewOptions.groups[groupDelegate.groupIndex]
                                            }
                                        }
                                    }
                                }
                            }

                            Repeater {
                                id: graphGenerator

                                delegate: Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.minimumHeight: graphDelegate.visible ? 196 : 0

                                    Behavior on Layout.minimumHeight {NumberAnimation{}}

                                    Widgets.GraphDelegate {
                                        id: graphDelegate
                                        graphId: index
                                        anchors.fill: parent
                                        anchors.margins: app.spacing
                                        enabled: viewOptions.graphs[graphId]

                                        Connections {
                                            target: viewOptions
                                            function onGraphsChanged() {
                                                graphDelegate.enabled = viewOptions.graphs[graphDelegate.graphId]
                                            }
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
        }

        //
        // Title
        //
        Rectangle {
            radius: 5
            height: 32
            Layout.fillWidth: true
            color: Qt.rgba(45/255, 96/255, 115/255, 1)

            RowLayout {
                spacing: app.spacing

                anchors {
                    left: parent.left
                    leftMargin: app.spacing
                    verticalCenter: parent.verticalCenter
                }

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(24, 24)
                    source: "qrc:/icons/arrow-right.svg"
                    Layout.alignment: Qt.AlignVCenter

                    ColorOverlay {
                        source: parent
                        anchors.fill: parent
                        color: palette.brightText
                    }
                }

                Label {
                    font.bold: true
                    font.pixelSize: 16
                    text: dataGrid.title
                    color: palette.brightText
                    font.family: app.monoFont
                }
            }

            Label {
                font.family: app.monoFont
                color: palette.brightText
                text: CppSerialManager.receivedBytes

                anchors {
                    right: parent.right
                    rightMargin: app.spacing
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
