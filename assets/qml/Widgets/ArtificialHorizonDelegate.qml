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
 
//
// NOTE: This file heavily based on some of the widgets created by the developers
//       of QGroundControl (https://github.com/mavlink/qgroundcontrol)
//       Kudos to them!
//

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

Item {
    id: root

    //
    // Custom properties
    //
    property real rollAngle: 0
    property real pitchAngle: 0
    property color borderColor: "#e6e0b2"

    //
    // Instrument
    //
    Item {
        clip: true
        id: instrument
        visible: false
        anchors.fill: parent

        //
        // Artificial horizon
        //
        Item {
            anchors.fill: parent
            id: artificialHorizon

            Rectangle {
                id: sky
                smooth: true
                color: "#5c93c5"
                antialiasing: true
                anchors.fill: parent
                anchors.topMargin: -artificialHorizon.height
            }

            Rectangle {
                id: ground
                smooth: true
                color: "#7d5233"
                antialiasing: true
                height: artificialHorizon.height * 1.5

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

        //
        // Pitch indicator
        //
        Item {
            clip: true
            id: pitchIndicator
            anchors.fill: parent

            Column {
                spacing: 15
                anchors.centerIn: parent

                //
                // Build reticles
                //
                Repeater {
                    model: 36

                    //
                    // Reticle line + labels
                    //
                    Rectangle {
                        id: reticle
                        property int pitch: -(index * 5 - 90)

                        height: 1
                        smooth: true
                        color: "#fff"
                        antialiasing: true
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: (pitch % 10) === 0 ? 40 : 25

                        //
                        // Left pitch label
                        //
                        Label {
                            color: "#fff"
                            smooth: true
                            font.pixelSize: 10
                            text: reticle.pitch
                            anchors.centerIn: parent
                            font.family: app.monoFont
                            anchors.horizontalCenterOffset: -40
                            visible: (reticle.pitch != 0) && ((reticle.pitch % 10) === 0)
                        }

                        //
                        // Right pitch label
                        //
                        Label {
                            color: "#fff"
                            smooth: true
                            font.pixelSize: 10
                            text: reticle.pitch
                            anchors.centerIn: parent
                            font.family: app.monoFont
                            anchors.horizontalCenterOffset: 40
                            visible: (reticle.pitch != 0) && ((reticle.pitch % 10) === 0)
                        }
                    }
                }
            }

            transform: [
                Translate {
                    y: (root.pitchAngle * 16 / 5) - 8
                },
                Rotation {
                    angle: -root.rollAngle
                    origin.x: instrument.width / 2
                    origin.y: instrument.height / 2
                }
            ]
        }

        //
        // Pointer
        //
        Image {
            id: pointer
            mipmap: true
            anchors.fill: parent
            sourceSize.height:  parent.height
            fillMode: Image.PreserveAspectFit
            source: "qrc:/instruments/AttitudePointer.svg"
        }

        //
        // Attitude dial
        //
        Image {
            id: attitudeDial
            mipmap:  true
            anchors.fill: parent
            sourceSize.height: parent.height
            fillMode: Image.PreserveAspectFit
            source: "qrc:/instruments/AttitudeDial.svg"

            transform: Rotation {
                angle: -root.rollAngle
                origin.x: instrument.width / 2
                origin.y: instrument.height / 2
            }
        }

        //
        // Crosshair
        //
        Image {
            id: crosshair
            mipmap: true
            sourceSize.width: width
            anchors.centerIn: parent
            width: parent.width * 0.75
            fillMode: Image.PreserveAspectFit
            source: "qrc:/instruments/Crosshair.svg"
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
        border.color: root.borderColor
        width: instrument.width + 2
        height: instrument.height + 2
    }
}
