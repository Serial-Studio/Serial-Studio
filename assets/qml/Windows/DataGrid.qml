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
import QtGraphicalEffects 1.0

import QtQuick.Window 2.12 as QtWindow

import Qt.labs.settings 1.0
import "../Widgets" as Widgets

Control {
    id: root
    property string title
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Settings
    //
    Settings {
        property alias numPoints: slider.value
        property alias multiplier: mult.value
    }

    //
    // Group/dataset updating
    //
    Connections {
        target: Cpp_UI_Provider
        function onUpdated() {
            root.title = Cpp_UI_Provider.title
            if (groupGenerator.model !== Cpp_UI_Provider.groupCount) {
                var list = []
                for (var i = 0; i < Cpp_UI_Provider.groupCount; ++i)
                    list.push(true)

                groupGenerator.model = 0
                viewOptions.groups = list
                groupGenerator.model = Cpp_UI_Provider.groupCount
            }
        }
    }

    //
    // Graph data updating
    //
    Connections {
        target: Cpp_UI_GraphProvider
        function onDataUpdated() {
            if (graphGenerator.model !== Cpp_UI_GraphProvider.graphCount) {
                var list = []
                for (var i = 0; i < Cpp_UI_GraphProvider.graphCount; ++i)
                    list.push(true)

                graphGenerator.model = 0
                viewOptions.graphs = list
                graphGenerator.model = Cpp_UI_GraphProvider.graphCount
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
                gradient: true
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                backgroundColor: "#121218"
                icon.source: "qrc:/icons/visibility.svg"
                title: qsTr("View")

                property var groups: []
                property var graphs: []

                ScrollView {
                    clip: true
                    contentWidth: -1
                    anchors.fill: parent
                    anchors.margins: app.spacing
                    anchors.topMargin: viewOptions.borderWidth
                    anchors.bottomMargin: viewOptions.borderWidth

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
                            spacing: 0
                            visible: graphGenerator.count > 0

                            Slider {
                                id: slider
                                to: 25
                                from: 1
                                value: 10
                                live: false
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                onValueChanged: Cpp_UI_GraphProvider.displayedPoints = value * Math.pow(10, mult.value)
                            }

                            Item {
                                width: app.spacing
                            }

                            Label {
                                font.pixelSize: 12
                                font.family: app.monoFont
                                Layout.alignment: Qt.AlignVCenter
                                text: Math.ceil(slider.position * (slider.to)) + "×10^"
                            }

                            Item {
                                width: app.spacing
                            }

                            SpinBox {
                                id: mult
                                to: 6
                                from: 0
                                value: 1
                                editable: true
                                Layout.maximumWidth: 42
                                font.family: app.monoFont
                                Layout.alignment: Qt.AlignVCenter
                                onValueChanged: Cpp_UI_GraphProvider.displayedPoints = slider.value * Math.pow(10, mult.value)
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
                                palette.highlight: "#d72d60"
                                Component.onCompleted: checked = true
                                text: Cpp_UI_Provider.getGroup(index).title
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
                                palette.highlight: "#d72d60"
                                Component.onCompleted: checked = true
                                text: Cpp_UI_GraphProvider.getDataset(index).title
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
                gradient: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                backgroundColor: "#121218"
                icon.source: "qrc:/icons/scatter-plot.svg"
                title: qsTr("Data")

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
                            columns: Math.floor(width / 256)

                            Repeater {
                                id: groupGenerator

                                delegate: Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.minimumHeight: groupDelegate.visible ? 196 : 0

                                    Widgets.GroupDelegate {
                                        id: groupDelegate
                                        groupId: index
                                        anchors.fill: parent
                                        anchors.margins: app.spacing
                                        group: Cpp_UI_Provider.getGroup(index)
                                        enabled: viewOptions.groups[groupId]

                                        Connections {
                                            target: viewOptions
                                            function onGroupsChanged() {
                                                groupDelegate.enabled = viewOptions.groups[groupDelegate.groupId]
                                            }
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onDoubleClicked: groupWindow.show()
                                    }

                                    QtWindow.Window {
                                        id: groupWindow
                                        width: 640
                                        height: 480
                                        minimumWidth: 320
                                        minimumHeight: 256
                                        title: group.title

                                        flags: Qt.Dialog |
                                               Qt.WindowStaysOnTopHint |
                                               Qt.WindowCloseButtonHint |
                                               Qt.WindowTitleHint

                                        Rectangle {
                                            anchors.fill: parent
                                            color: group.backgroundColor
                                        }

                                        Widgets.GroupDelegate {
                                            id: group
                                            groupId: index
                                            showIcon: true
                                            headerHeight: 48
                                            anchors.margins: 0
                                            anchors.fill: parent
                                            borderColor: backgroundColor
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

                                    MouseArea {
                                        anchors.fill: parent
                                        onDoubleClicked: graphWindow.show()
                                    }

                                    QtWindow.Window {
                                        id: graphWindow
                                        width: 640
                                        height: 480
                                        minimumWidth: 320
                                        minimumHeight: 256
                                        title: graph.title

                                        flags: Qt.Dialog |
                                               Qt.WindowStaysOnTopHint |
                                               Qt.WindowCloseButtonHint |
                                               Qt.WindowTitleHint

                                        Rectangle {
                                            anchors.fill: parent
                                            color: graph.backgroundColor
                                        }

                                        Widgets.GraphDelegate {
                                            id: graph
                                            graphId: index
                                            showIcon: true
                                            headerHeight: 48
                                            anchors.margins: 0
                                            anchors.fill: parent
                                            borderColor: backgroundColor
                                            icon.source: "qrc:/icons/chart.svg"
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

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: palette.highlight
                }

                GradientStop {
                    position: 1
                    color: "#058ca7"
                }
            }

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
                    text: root.title
                    font.pixelSize: 16
                    color: palette.brightText
                    font.family: app.monoFont
                }
            }

            Label {
                font.family: app.monoFont
                color: palette.brightText
                visible: !Cpp_CSV_Player.isOpen
                text: Cpp_IO_Manager.receivedDataLength

                anchors {
                    right: parent.right
                    rightMargin: app.spacing
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
