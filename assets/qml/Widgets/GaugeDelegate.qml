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
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import SerialStudio 1.0

import "."

Window {
    id: root

    //
    // Custom properties
    //
    property string units: ""
    property int datasetIndex: 0
    property real currentValue: 0
    property real minimumValue: 0
    property real maximumValue: 0

    //
    // Window properties
    //
    spacing: -1
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    icon.source: "qrc:/icons/gauge.svg"
    implicitHeight: implicitWidth + 96
    borderColor: root.headerVisible ? Cpp_ThemeManager.datasetWindowBorder : "transparent"

    //
    // Connections with widget manager
    //
    Connections {
        target: Cpp_UI_WidgetProvider
        function onDataChanged() {
            root.updateValues()
        }
    }

    //
    // Updates the internal values of the gauge widget
    //
    function updateValues() {
        if (Cpp_UI_WidgetProvider.gaugeDatasetCount() > root.datasetIndex) {
            root.minimumValue = Cpp_UI_WidgetProvider.gaugeMin(root.datasetIndex)
            root.maximumValue = Cpp_UI_WidgetProvider.gaugeMax(root.datasetIndex)
            root.currentValue = Cpp_UI_WidgetProvider.gauge(root.datasetIndex)
            root.title = Cpp_UI_WidgetProvider.gaugeDatasetAt(root.datasetIndex).title
            root.units = Cpp_UI_WidgetProvider.gaugeDatasetAt(root.datasetIndex).units
        }

        else {
            root.title = ""
            root.units = ""
            root.currentValue = 0
            root.minimumValue = 0
            root.maximumValue = 0
        }
    }

    //
    // Layout
    //
    ColumnLayout {
        spacing: app.spacing * 4

        anchors {
            fill: parent
            margins: app.spacing * 4
            leftMargin: app.spacing * 8
            rightMargin: app.spacing * 8
        }

        //
        // Gauge
        //
        CircularGauge {
            id: gauge
            tickmarksVisible: true
            Layout.fillHeight: true
            value: root.currentValue
            minimumValue: root.minimumValue
            maximumValue: root.maximumValue
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: root.width * 0.6
            Layout.maximumWidth: root.width * 0.6

            style: CircularGaugeStyle {
                maximumValueAngle: 144
                minimumValueAngle: -144
                labelStepSize: (root.maximumValue - root.minimumValue) / 10
                tickmarkStepSize: (root.maximumValue - root.minimumValue) / 20

                tickmark: Rectangle {
                    antialiasing: true
                    implicitWidth: outerRadius * 0.02
                    implicitHeight: outerRadius * 0.06
                    visible: styleData.value % labelStepSize == 0
                    color: Cpp_ThemeManager.widgetForegroundSecondary
                }

                minorTickmark: Rectangle {
                    antialiasing: true
                    implicitWidth: outerRadius * 0.01
                    implicitHeight: outerRadius * 0.03
                    color: Cpp_ThemeManager.widgetForegroundSecondary
                }

                tickmarkLabel: Text {
                    antialiasing: true
                    text: styleData.value
                    font.pixelSize: Math.max(6, outerRadius * 0.1)
                    color: Cpp_ThemeManager.widgetForegroundSecondary
                }
            }

            Behavior on value {NumberAnimation{duration: 200}}

            //
            // Value text
            //
            Label {
                font.bold: true
                anchors.centerIn: parent
                font.family: app.monoFont
                color: Cpp_ThemeManager.widgetForegroundPrimary
                font.pixelSize: Math.max(12, gauge.height / 15)
                anchors.verticalCenterOffset: parent.height * 0.17
                text: (root.currentValue > root.maximumValue ? root.maximumValue.toFixed(2) :
                                                               root.currentValue.toFixed(2)) + " " + root.units
            }
        }
    }
}
