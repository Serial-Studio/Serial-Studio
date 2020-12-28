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

import Group 1.0
import Dataset 1.0

import "."

Window {
    id: root

    //
    // Window properties
    //
    spacing: -1
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    icon.source: "qrc:/icons/chart.svg"
    implicitHeight: implicitWidth + 96
    borderColor: Qt.rgba(45/255, 96/255, 115/255, 1)
    backgroundColor: Qt.rgba(9 / 255, 9 / 255, 12 / 255, 1)

    //
    // Custom properties
    //
    property real yawAngle: 0
    property real rollAngle: 0
    property real pitchAngle: 0
    property int groupIndex: 0

    //
    // Colors
    //
    property color gyroColor: Qt.rgba(230/255, 224/255, 178/255, 1)

    //
    // Connections with widget manager
    //
    Connections {
        target: CppWidgets
        function onDataChanged() {
            root.updateValues()
        }
    }

    //
    // Updates the internal values of the bar widget
    //
    function updateValues() {
        if (CppWidgets.gyroGroupCount() > root.groupIndex) {
            //root.yawAngle = CppWidgets.gyroYaw(root.groupIndex)
            //root.rollAngle = CppWidgets.gyroRoll(root.groupIndex)
            //root.pitchAngle = CppWidgets.gyroPitch(root.groupIndex)
            root.title = CppWidgets.gyroGroupAt(root.groupIndex).title
        }

        else {
            root.title = ""
            root.yawAngle = 0
            root.rollAngle = 0
            root.pitchAngle = 0
        }
    }

    //
    // Animations
    //
    Behavior on yawAngle {NumberAnimation{}}
    Behavior on rollAngle {NumberAnimation{}}
    Behavior on pitchAngle {NumberAnimation{}}

    //
    // Instrument
    //
    Item {
        id: instrument
        visible: false
        anchors.centerIn: parent
        width: Math.min(root.width, root.height) * 0.7
        height: Math.min(root.width, root.height) * 0.7

        //
        // Artificial horizon
        //
        Item {
            anchors.fill: parent
            id: artificialHorizon

            Rectangle {
                id: sky
                smooth: true
                antialiasing: true
                anchors.fill: parent
                color: Qt.rgba(92/255, 147/255, 197/255, 1)
                anchors.topMargin: -artificialHorizon.height
            }

            Rectangle {
                id: ground
                smooth: true
                antialiasing: true
                height: artificialHorizon.height * 1.5
                color: Qt.rgba(125/255, 82/255, 51/255, 1)

                anchors {
                    left: sky.left
                    right: sky.right
                    bottom: sky.bottom
                    bottomMargin: -artificialHorizon.height
                }
            }

            transform: [
                Translate {
                    y: root.pitchAngle * instrument.height / 45
                },
                Rotation {
                    angle: -rollAngle
                    origin.x: artificialHorizon.width  / 2
                    origin.y: artificialHorizon.height / 2
                }
            ]
        }
    }

    //
    // Circular mask over artificial horizon
    //
    Rectangle {
        id: mask
        color: "black"
        visible: false
        radius: width / 2
        anchors.fill: instrument
    } OpacityMask {
        maskSource: mask
        source: instrument
        anchors.fill: instrument
    }

    //
    // Border rectangle
    //
    Rectangle {
        border.width: 2
        radius: width / 2
        color: "transparent"
        anchors.centerIn: parent
        border.color: root.gyroColor
        width: instrument.width + 2
        height: instrument.height + 2
    }
}
