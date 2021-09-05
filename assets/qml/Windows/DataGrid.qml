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

import Qt.labs.settings 1.0
import QtQuick.Window 2.12 as QtWindow

import "../Widgets" as Widgets

Control {
    id: root
    property string title
    background: Rectangle {
        color: Cpp_ThemeManager.windowBackground
    }

    //
    // Settings
    //
    Settings {
        property alias numPoints: points.value
        property alias multiplier: scale.value
    }

    //
    // Group/dataset updating
    //
    Connections {
        target: Cpp_UI_Provider

        //*! Optimize this function
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
                title: qsTr("View")
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                headerDoubleClickEnabled: false
                backgroundColor: Cpp_ThemeManager.consoleBase
                icon.source: "qrc:/icons/visibility.svg"

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
                            id: ranges
                            spacing: app.spacing * 2
                            visible: graphGenerator.count > 0

                            function updateGraphValue() {
                                Cpp_UI_GraphProvider.displayedPoints = points.value * Math.pow(10, scale.value)
                            }

                            ColumnLayout {
                                spacing: app.spacing
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.alignment: Qt.AlignHCenter

                                Widgets.SimpleDial {
                                    id: points

                                    to: 25
                                    from: 1
                                    value: 10
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.alignment: Qt.AlignHCenter
                                    onPressedChanged: ranges.updateGraphValue()
                                    Component.onCompleted: ranges.updateGraphValue()
                                }

                                Label {
                                    font.pixelSize: 12
                                    text: qsTr("Points")
                                    Layout.fillWidth: true
                                    font.family: app.monoFont
                                    Layout.alignment: Qt.AlignHCenter
                                    horizontalAlignment: Label.AlignHCenter
                                }
                            }

                            ColumnLayout {
                                spacing: app.spacing
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.alignment: Qt.AlignHCenter

                                Widgets.SimpleDial {
                                    id: scale

                                    to: 6
                                    from: 0
                                    value: 1
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    snapMode: Dial.SnapOnRelease
                                    text: "10^" + Math.ceil(scale.value)
                                    Layout.alignment: Qt.AlignHCenter
                                    onPressedChanged: ranges.updateGraphValue()
                                    Component.onCompleted: ranges.updateGraphValue()
                                }

                                Label {
                                    font.pixelSize: 12
                                    Layout.fillWidth: true
                                    font.family: app.monoFont
                                    Layout.alignment: Qt.AlignHCenter
                                    horizontalAlignment: Label.AlignHCenter
                                    text: qsTr("Scale")
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
                                text: Cpp_UI_Provider.getGroup(index).title
                                palette.highlight: Cpp_ThemeManager.alternativeHighlight

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
                                palette.highlight: Cpp_ThemeManager.alternativeHighlight
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
                title: qsTr("Data")
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                headerDoubleClickEnabled: false
                backgroundColor: Cpp_ThemeManager.consoleBase
                icon.source: "qrc:/icons/scatter-plot.svg"

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
                                        enabled: viewOptions.groups[groupId]
                                        group: Cpp_UI_Provider.getGroup(index)
                                        onHeaderDoubleClicked: groupWindow.show()

                                        Connections {
                                            target: viewOptions
                                            function onGroupsChanged() {
                                                groupDelegate.enabled = viewOptions.groups[groupDelegate.groupId]
                                            }
                                        }
                                    }

                                    QtWindow.Window {
                                        id: groupWindow
                                        width: 640
                                        height: 480
                                        minimumWidth: 320
                                        minimumHeight: 256
                                        title: group.title

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
                                            enabled: groupWindow.visible
                                            headerDoubleClickEnabled: false
                                            titleColor: Cpp_ThemeManager.text
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
                                        onHeaderDoubleClicked: graphWindow.show()

                                        Connections {
                                            target: viewOptions
                                            function onGraphsChanged() {
                                                graphDelegate.enabled = viewOptions.graphs[graphDelegate.graphId]
                                            }
                                        }
                                    }

                                    QtWindow.Window {
                                        id: graphWindow
                                        width: 640
                                        height: 480
                                        minimumWidth: 320
                                        minimumHeight: 256
                                        title: graph.title

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
                                            enabled: graphWindow.visible
                                            borderColor: backgroundColor
                                            headerDoubleClickEnabled: false
                                            icon.source: "qrc:/icons/chart.svg"
                                            titleColor: Cpp_ThemeManager.text
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
                    color: Cpp_ThemeManager.windowGradient
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
                text: Cpp_IO_Manager.receivedDataLength //*! Optimize this function

                anchors {
                    right: parent.right
                    rightMargin: app.spacing
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
