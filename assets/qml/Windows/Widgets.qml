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

import "../Widgets" as Widgets

Control {
    id: root
    property string title
    background: Rectangle {
        color: Cpp_ThemeManager.windowBackground
    }

    //
    // Custom properties
    //
    property var visibleWidgets: []
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

            // Generate gauge widgets
            gaugeGenerator.model = 0
            gaugeGenerator.model = Cpp_UI_WidgetProvider.gaugeDatasetCount()

            // Generate compass widgets
            compassGenerator.model = 0
            compassGenerator.model = Cpp_UI_WidgetProvider.compassDatasetCount()

            // Generate map widgets
            mapGenerator.model = 0
            mapGenerator.model = Cpp_UI_WidgetProvider.mapGroupCount()

            // Update project title
            root.title = Cpp_UI_Provider.title

            // Regenerate visibility switcher
            if (visibleWidgets.length !== Cpp_UI_WidgetProvider.totalWidgetCount) {
                var list = []
                for (var i = 0; i < Cpp_UI_WidgetProvider.totalWidgetCount; ++i)
                    list.push(true)

                visibleWidgets = list
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
        // Widget selector
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
                icon.source: "qrc:/icons/visibility.svg"
                backgroundColor: Cpp_ThemeManager.embeddedWindowBackground

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
                        // Widgets title
                        //
                        RowLayout {
                            spacing: app.spacing
                            visible: Cpp_UI_WidgetProvider.totalWidgetCount > 0

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
                                text: qsTr("Widgets") + ":"
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
                        // Widget switches
                        //
                        Repeater {
                            model: Cpp_UI_WidgetProvider.totalWidgetCount

                            delegate: Switch {
                                checked: true
                                Layout.fillWidth: true
                                text: Cpp_UI_WidgetProvider.widgetNames()[index]
                                palette.highlight: Cpp_ThemeManager.alternativeHighlight
                                onCheckedChanged: Cpp_UI_WidgetProvider.updateWidgetVisibility(index, checked)
                            }
                        }
                    }
                }
            }

            //
            // Widget grid
            //
            Widgets.Window {
                id: dataWin
                gradient: true
                title: qsTr("Widgets")
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                headerDoubleClickEnabled: false
                icon.source: "qrc:/icons/scatter-plot.svg"
                backgroundColor: Cpp_ThemeManager.embeddedWindowBackground

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

                    //
                    // WARNING: the widget generators must be in the same
                    //          order as the list returned by
                    //          Cpp_UI_WidgetProvider.widgetNames()!
                    //

                    ColumnLayout {
                        width: _sv.width - 2 * app.spacing

                        Item {
                            Layout.minimumHeight: 10
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            rowSpacing: app.spacing * 1.5
                            columnSpacing: app.spacing * 1.5
                            columns: Math.floor(_sv.width / (root.minimumWidgetSize + 2 * app.spacing))

                            Repeater {
                                id: mapGenerator

                                delegate: Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.minimumWidth: root.minimumWidgetSize
                                    Layout.minimumHeight: root.minimumWidgetSize

                                    id: mapWidget
                                    visible: opacity > 0
                                    enabled: opacity > 0
                                    Behavior on opacity {NumberAnimation{}}

                                    Connections {
                                        target: Cpp_UI_WidgetProvider

                                        function onWidgetVisiblityChanged() {
                                            mapWidget.opacity = Cpp_UI_WidgetProvider.widgetVisible(index) ? 1 : 0
                                        }
                                    }

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
                                            titleColor: Cpp_ThemeManager.text
                                        }
                                    }
                                }
                            }

                            Repeater {
                                id: gyroGenerator

                                delegate: Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.minimumWidth: root.minimumWidgetSize
                                    Layout.minimumHeight: root.minimumWidgetSize

                                    id: gyroWidget
                                    visible: opacity > 0
                                    enabled: opacity > 0
                                    Behavior on opacity {NumberAnimation{}}

                                    Connections {
                                        target: Cpp_UI_WidgetProvider

                                        function onWidgetVisiblityChanged() {
                                            gyroWidget.opacity = Cpp_UI_WidgetProvider.widgetVisible(
                                                        mapGenerator.count +
                                                        index) ? 1 : 0
                                        }
                                    }

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
                                            titleColor: Cpp_ThemeManager.text
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

                                    id: accWidget
                                    visible: opacity > 0
                                    enabled: opacity > 0
                                    Behavior on opacity {NumberAnimation{}}

                                    Connections {
                                        target: Cpp_UI_WidgetProvider

                                        function onWidgetVisiblityChanged() {
                                            accWidget.opacity = Cpp_UI_WidgetProvider.widgetVisible(
                                                        mapGenerator.count +
                                                        gyroGenerator.count +
                                                        index) ? 1 : 0
                                        }
                                    }

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
                                            titleColor: Cpp_ThemeManager.text
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

                                    id: barWidget
                                    visible: opacity > 0
                                    enabled: opacity > 0
                                    Behavior on opacity {NumberAnimation{}}

                                    Connections {
                                        target: Cpp_UI_WidgetProvider

                                        function onWidgetVisiblityChanged() {
                                            barWidget.opacity = Cpp_UI_WidgetProvider.widgetVisible(
                                                        mapGenerator.count +
                                                        gyroGenerator.count +
                                                        accGenerator.count +
                                                        index) ? 1 : 0
                                        }
                                    }

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
                                            titleColor: Cpp_ThemeManager.text
                                        }
                                    }
                                }
                            }

                            Repeater {
                                id: gaugeGenerator

                                delegate: Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.minimumWidth: root.minimumWidgetSize
                                    Layout.minimumHeight: root.minimumWidgetSize

                                    id: gaugeWidget
                                    visible: opacity > 0
                                    enabled: opacity > 0
                                    Behavior on opacity {NumberAnimation{}}

                                    Connections {
                                        target: Cpp_UI_WidgetProvider

                                        function onWidgetVisiblityChanged() {
                                            gaugeWidget.opacity = Cpp_UI_WidgetProvider.widgetVisible(
                                                        mapGenerator.count +
                                                        gyroGenerator.count +
                                                        accGenerator.count +
                                                        barGenerator.count +
                                                        index) ? 1 : 0
                                        }
                                    }

                                    Widgets.GaugeDelegate {
                                        datasetIndex: index
                                        anchors.fill: parent
                                        onHeaderDoubleClicked: windowGauge.show()
                                    }

                                    QtWindow.Window {
                                        id: windowGauge
                                        width: 640
                                        height: 480
                                        minimumWidth: root.minimumWidgetSize * 1.2
                                        minimumHeight: root.minimumWidgetSize * 1.2
                                        title: gauge.title

                                        Rectangle {
                                            anchors.fill: parent
                                            color: gauge.backgroundColor
                                        }

                                        Widgets.GaugeDelegate {
                                            id: gauge
                                            showIcon: true
                                            gradient: false
                                            headerHeight: 48
                                            datasetIndex: index
                                            anchors.margins: 0
                                            anchors.fill: parent
                                            borderColor: backgroundColor
                                            headerDoubleClickEnabled: false
                                            titleColor: Cpp_ThemeManager.text
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

                                    id: compassWidget
                                    visible: opacity > 0
                                    enabled: opacity > 0
                                    Behavior on opacity {NumberAnimation{}}

                                    Connections {
                                        target: Cpp_UI_WidgetProvider

                                        function onWidgetVisiblityChanged() {
                                            compassWidget.opacity = Cpp_UI_WidgetProvider.widgetVisible(
                                                        mapGenerator.count +
                                                        gyroGenerator.count +
                                                        accGenerator.count +
                                                        barGenerator.count +
                                                        gaugeGenerator.count +
                                                        index) ? 1 : 0
                                        }
                                    }

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
                    color: Cpp_ThemeManager.windowGradient1
                }

                GradientStop {
                    position: 1
                    color: Cpp_ThemeManager.windowGradient2
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
