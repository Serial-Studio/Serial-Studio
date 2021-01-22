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

import Group 1.0
import Dataset 1.0

import "."

Window {
    id: root

    //
    // Window properties
    //
    spacing: -1
    gradient: true
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    backgroundColor: "#09090c"
    implicitHeight: implicitWidth + 96
    icon.source: "qrc:/icons/refresh.svg"

    //
    // Custom properties
    //
    property real yawAngle: 0
    property real rollAngle: 0
    property int groupIndex: 0
    property real pitchAngle: 0
    property color valueColor: "#8eccff"
    property real gaugeSize: calculateGaugeSize()

    //
    // Update root size automatically
    //
    onWidthChanged: calculateGaugeSize()
    onHeightChanged: calculateGaugeSize()


    //
    // Connections with widget manager
    //
    Connections {
        target: CppWidgetProvider
        function onDataChanged() {
            root.updateValues()
        }
    }

    //
    // Updates the internal values of the bar widget
    //
    function updateValues() {
        if (CppWidgetProvider.gyroGroupCount() > root.groupIndex) {
            root.yawAngle   = CppWidgetProvider.gyroYaw(root.groupIndex)
            root.rollAngle  = CppWidgetProvider.gyroRoll(root.groupIndex)
            root.pitchAngle = CppWidgetProvider.gyroPitch(root.groupIndex)
            root.title      = CppWidgetProvider.gyroGroupAt(root.groupIndex).title
        }

        else {
            root.title      = ""
            root.yawAngle   = 0
            root.rollAngle  = 0
            root.pitchAngle = 0
        }

        calculateGaugeSize()
    }

    //
    // Redraws accelerator root
    //
    function calculateGaugeSize() {
        if (root.width < root.height)
            root.gaugeSize = Math.max(120, root.width - controls.implicitWidth - 12 * app.spacing)

        else
            root.gaugeSize = Math.max(120, root.height - controls.implicitWidth - 4 * app.spacing)
    }

    //
    // Animations
    //
    Behavior on yawAngle {NumberAnimation{}}
    Behavior on rollAngle {NumberAnimation{}}
    Behavior on pitchAngle {NumberAnimation{}}

    //
    // Layout
    //
    RowLayout {
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }

        //
        // Artificial horizon
        //
        ArtificialHorizonDelegate {
            id: artificialHorizon
            rollAngle: root.rollAngle
            pitchAngle: root.pitchAngle
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: root.gaugeSize
            Layout.maximumWidth: root.gaugeSize
            Layout.minimumHeight: root.gaugeSize
            Layout.maximumHeight: root.gaugeSize
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }

        //
        // Values
        //
        Rectangle {
            id: controls
            border.width: 1
            color: "transparent"
            implicitWidth: 120
            Layout.minimumWidth: 120
            Layout.maximumWidth: 120
            Layout.minimumHeight: 120
            Layout.maximumHeight: 120
            border.color: root.valueColor
            Layout.alignment: Qt.AlignVCenter

            function formatAngle(angle) {
                var str

                if (angle < 0)
                    str = '- '
                else
                    str = '+ '

                if (Math.abs(angle) < 100)
                    str += '0'

                if (Math.abs(angle) < 10)
                    str += '0'

                return str + Math.abs(Math.floor(angle))
            }

            ColumnLayout {
                spacing: app.spacing
                anchors.centerIn: parent

                Label {
                    font.pixelSize: 12
                    color: root.valueColor
                    font.family: app.monoFont
                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("%1° YAW").arg(controls.formatAngle(root.yawAngle)) + CppTranslator.dummy
                }

                Label {
                    font.pixelSize: 12
                    color: root.valueColor
                    font.family: app.monoFont
                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("%1° ROLL").arg(controls.formatAngle(root.rollAngle)) + CppTranslator.dummy
                }

                Label {
                    font.pixelSize: 12
                    color: root.valueColor
                    font.family: app.monoFont
                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("%1° PITCH").arg(controls.formatAngle(root.pitchAngle)) + CppTranslator.dummy
                }
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }
    }
}
