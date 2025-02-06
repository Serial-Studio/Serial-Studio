/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  //
  // Widget data inputs
  //
  property color color
  property GyroscopeModel model: GyroscopeModel {}

  //
  // Custom properties
  //
  property real yawAngle: root.model.yaw
  property real rollAngle: root.model.roll
  property real pitchAngle: root.model.pitch
  // New property for airspeed from the gyroscope model
  property real airspeed: root.model.airspeed//NEW LINE FOR AIRSPEED EDITED
  property real altitude: root.model.altitude//NEW LINE FOR altitude EDITED

  //
  // Animations
  //
  Behavior on yawAngle {NumberAnimation{}}
  Behavior on rollAngle {NumberAnimation{}}
  Behavior on pitchAngle {NumberAnimation{}}

  //
  // Artificial horizon
  //
  Item {
    id: artificialHorizon

    antialiasing: true
    anchors.fill: parent
    anchors.margins: -root.height / 2

    Rectangle {
      id: sky
      antialiasing: true
      anchors.fill: parent
      anchors.topMargin: -1 * parent.height
      gradient: Gradient {
        GradientStop {
          position: 0
          color: "#5759A6"
        }

        GradientStop {
          position: 1
          color: "#7A7BBB"
        }
      }
    }

    Rectangle {
      antialiasing: true
      height: parent.height * 1.5
      anchors {
        left: sky.left
        right: sky.right
        bottom: sky.bottom
        bottomMargin: -parent.height
      }

      gradient: Gradient {
        GradientStop {
          position: 1
          color: "#672122"
        }

        GradientStop {
          position: 0
          color: "#A63732"
        }
      }

      Rectangle {
        height: 2
        color: "#fff"
        antialiasing: true
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
        }
      }
    }

    transform: [
      Translate {
        y: root.pitchAngle * root.height / 45
      },
      Rotation {
        angle: -root.rollAngle
        origin.x: artificialHorizon.width  / 2
        origin.y: artificialHorizon.height / 2
      }
    ]
  }

  //
  // Pitch reticles
  //
  Item {
    id: pitchIndicator

    antialiasing: true
    height: root.height - 4 - angles.height

    anchors {
      left: parent.left
      right: parent.right
    }

    readonly property var reticleHeight: Math.max(16, pitchIndicator.height / 20)

    Item {
      antialiasing: true
      width: parent.width
      height: parent.height

      Column {
        id: column
        spacing: -4
        antialiasing: true
        anchors.centerIn: parent

        Repeater {
          model: 37
          delegate: Item {
            id: reticle
            antialiasing: true
            width: pitchIndicator.width
            height: pitchIndicator.reticleHeight

            opacity: {
              var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
              var distance = Math.abs(reticleY - rollDial.height / 2);
              var fadeDistance = rollDial.height / 2;
              return Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
            }

            onVisibleChanged: {
              var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
              var distance = Math.abs(reticleY - rollDial.height / 2);
              var fadeDistance = rollDial.height / 2;
              opacity = Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
            }

            onYChanged: {
              var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
              var distance = Math.abs(reticleY - rollDial.height / 2);
              var fadeDistance = rollDial.height / 2;
              opacity = Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
            }

            visible: Math.abs(pitch - root.pitchAngle) <= 20
            readonly property int pitch: -(index * 5 - 90)

            Rectangle {
              id: _line
              height: 2
              color: "#fff"
              antialiasing: true
              anchors.centerIn: parent
              opacity: (reticle.pitch % 10) === 0 ? 1 : 0.8
              width: (reticle.pitch % 10) === 0 ? pitchIndicator.width / 4 :
                                                  pitchIndicator.width / 8
            }

            Label {
              id: leftLabel
              color: "#fff"
              antialiasing: true
              text: reticle.pitch
              anchors.centerIn: parent
              font: Cpp_Misc_CommonFonts.monoFont
              opacity: root.height >= 120 ? 1 : 0
              visible: (reticle.pitch != 0) && (reticle.pitch % 10) === 0
              anchors.horizontalCenterOffset: - 1 * (_line.width + 32) / 2
            }

            Label {
              color: "#fff"
              antialiasing: true
              text: reticle.pitch
              anchors.centerIn: parent
              font: Cpp_Misc_CommonFonts.monoFont
              opacity: root.height >= 120 ? 1 : 0
              anchors.horizontalCenterOffset: (_line.width + 32) / 2
              visible: (reticle.pitch != 0) && (reticle.pitch % 10) === 0
            }
          }
        }
      }

      transform: [Translate {
          y: (root.pitchAngle * pitchIndicator.reticleHeight / 5) + (pitchIndicator.reticleHeight / 2)
        }
      ]
    }

    transform: [
      Rotation {
        angle: -root.rollAngle
        origin.x: root.width / 2
        origin.y: root.height / 2
      }
    ]
  }

  //
  // Angles indicator bar at bottom
  //
  Item {
    id: angles
    visible: root.height >= 120
    width: parent.width
    height: visible ? 24 : 0

    anchors {
      bottomMargin: 4
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }

    RowLayout {
      spacing: 4
      anchors.fill: parent

      Item {
        implicitWidth: 4
      }

      Rectangle {
        color: "#333"
        border.width: 1
        implicitHeight: 24
        border.color: "#fff"
        Layout.fillWidth: true

        Text {
          color: "#ffffff"
          elide: Qt.ElideLeft
          anchors.fill: parent
          verticalAlignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.monoFont
          horizontalAlignment: Qt.AlignHCenter
          text: qsTr("Roll: %1").arg(root.rollAngle.toFixed(2) + "°")
        }
      }

      Rectangle {
        color: "#333"
        border.width: 1
        implicitHeight: 24
        border.color: "#fff"
        Layout.fillWidth: true

        Text {
          color: "#ffffff"
          elide: Qt.ElideLeft
          anchors.fill: parent
          verticalAlignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.monoFont
          horizontalAlignment: Qt.AlignHCenter
          text: qsTr("Yaw: %1").arg(root.yawAngle.toFixed(2) + "°")
        }
      }

      Rectangle {
        color: "#333"
        border.width: 1
        implicitHeight: 24
        border.color: "#fff"
        Layout.fillWidth: true

        Text {
          color: "#ffffff"
          elide: Qt.ElideLeft
          anchors.fill: parent
          verticalAlignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.monoFont
          horizontalAlignment: Qt.AlignHCenter
          text: qsTr("Pitch: %1").arg(root.pitchAngle.toFixed(2) + "°")
        }
      }

      Item {
        implicitWidth: 4
      }
    }
  }

  //
  // Roll dial visual effects
  //
  MultiEffect {
    blur: 1
    blurMax: 64
    brightness: 0.6
    saturation: 0.1
    blurEnabled: true
    source: rollDial
    anchors.fill: rollDial
  }

  //
  // Roll pointer visual effects
  //
  MultiEffect {
    blur: 1
    blurMax: 64
    brightness: 0.6
    saturation: 0.1
    blurEnabled: true
    source: rollPointer
    anchors.fill: rollPointer
  }

  //
  // Cross hair visual effects
  //
  MultiEffect {
    blur: 1
    blurMax: 64
    brightness: 0.6
    saturation: 0.1
    blurEnabled: true
    source: crosshair
    anchors.fill: crosshair
  }


  //
  // Roll dial - at the top
  //
  Image {
    id: rollDial
    anchors.fill: parent
    sourceSize.height: parent.height
    fillMode: Image.PreserveAspectFit
    source: "qrc:/rcc/instruments/attitude_dial.svg"

    transform: Rotation {
      angle: -root.rollAngle
      origin.x: root.width / 2
      origin.y: root.height / 2
    }
  }

  //
  // Roll pointer - THE RED KNOB AT THE TOP
  //
  Image {
    id: rollPointer
    anchors.fill: parent
    sourceSize.height: parent.height
    fillMode: Image.PreserveAspectFit
    source: "qrc:/rcc/instruments/attitude_pointer.svg"
  }

  //
  // Crosshair - THE YELLOW ARROW THING
  Image {
    id: crosshair
    sourceSize.width: width
    anchors.centerIn: parent
    width: parent.width * 0.75
    fillMode: Image.PreserveAspectFit
    source: "qrc:/rcc/instruments/attitude_crosshair.svg"
  }

//
// Airspeed ladder V2 (copying the way he did pitch reticle)
//
  /*Item {
       id: airspeedTapeV2
       antialiasing: true //scaling and rotation
       height: root.height - 1 - angles.height // CONFIGURE: 1 was chosen randomly configure later

       anchors { //spans the full width of its parent (root). Its vertical position is determined by its height and the parent's layout
            left: parent.left
            right: parent.right
        }
       readonly property var tickHeight: Math.max(16, airspeedTapeV2.height / 20) //the vertical spacing or thickness used by tick lines

       Item {
             antialiasing: true
             width: parent.width //full width and height of parent (aka the screen of gyro
             height: parent.height

             Column { //column layer that will stack items vertically
               id: columnairspeed
               spacing: -4
               antialiasing: true
               anchors.left: parent.left //CHANGED THIS TO BE LEFT ALIGNED

               Repeater {
                 model: 37 // creates 37 instances of the 'delegate: Item' below
                 delegate: Item {
                   id: tick
                   antialiasing: true
                   width: airspeedTapeV2.width //LIKLEY NEED TO CHANGE THIS WIDTH
                   height: airspeedTapeV2.tickHeight

                   // opacity: { //The opacity depends on how close the reticle is to the center of the rollDial.
                   //   var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
                   //   var distance = Math.abs(reticleY - rollDial.height / 2);
                   //   var fadeDistance = rollDial.height / 2;
                   //   return Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
                   // }

                   // onVisibleChanged: {
                   //   var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
                   //   var distance = Math.abs(reticleY - rollDial.height / 2);
                   //   var fadeDistance = rollDial.height / 2;
                   //   opacity = Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
                   // }

                   // onYChanged: {
                   //   var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
                   //   var distance = Math.abs(reticleY - rollDial.height / 2);
                   //   var fadeDistance = rollDial.height / 2;
                   //   opacity = Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
                   // }

                   visible: Math.abs(airspeedInterval - root.airspeed) <= 20 //Only show the reticle if its pitch value is within ±20 degrees of root.pitchAngle.
                                                                                // This helps to avoid clutter by hiding lines that are too far outside the current pitch angle range.
                   readonly property int airspeedInterval: (index * 5)

                   //Inside each Item (the reticle), there is a Rectangle for the line and two Labels for the numeric markings:
                   Rectangle { //draw horizontal tick line
                     id: _line
                     height: 2 //thickness of tick
                     color: "#fff"
                     antialiasing: true
                     anchors.centerIn: parent // this must change
                     opacity: (tick.pitch % 10) === 0 ? 1 : 0.8
                     width: (tick.pitch % 10) === 0 ? pitchIndicator.width / 4 :
                                                         pitchIndicator.width / 8
                   }

                   Label {
                     id: leftLabel
                     color: "#fff"
                     antialiasing: true
                     text: tick.pitch
                     anchors.centerIn: parent
                     font: Cpp_Misc_CommonFonts.monoFont
                     opacity: root.height >= 120 ? 1 : 0
                     visible: (tick.pitch != 0) && (tick.pitch % 10) === 0
                     anchors.horizontalCenterOffset: - 1 * (_line.width + 32) / 2
                   }

                   Label {
                     color: "#fff"
                     antialiasing: true
                     text: tick.pitch
                     anchors.centerIn: parent
                     font: Cpp_Misc_CommonFonts.monoFont
                     opacity: root.height >= 120 ? 1 : 0
                     anchors.horizontalCenterOffset: (_line.width + 32) / 2
                     visible: (tick.pitch != 0) && (tick.pitch % 10) === 0
                   }

                 }
               }
             }
    }
  } //end of Item {id: airspeedTapeV2
  */


  //
    // Airspeed ladder V1 gpt fail
    //
 /*Item {
      id: airspeedTape
      z: 100
      width: 80                            // Narrow strip for the airspeed tape
      anchors {
          left: parent.left
          top: parent.top
          bottom: parent.bottom
          leftMargin: 8
          topMargin: 8
          bottomMargin: 8
      }

      // Background for the tape
      // Rectangle {
      //     anchors.fill: parent
      //     color: "black"
      //     radius: 4
      //     opacity: 0.8
      // }

      // The “ladder” item that draws ticks
      Item {
          id: ladder
          anchors.fill: parent
          // Keep the same logic as before but
          // tailor the geometry to this container:
          property real scaleRange: 20     // ±20 around current speed
          property real stepSize: 5
          property real pxPerMps: 4        // 4 or 5 px per m/s is often enough

          // Shortcut to the container’s height
          property real ladderHeight: height

          // The center line (where actual airspeed is shown)
          property real ladderCenter: ladderHeight / 2

          // Because we have a separate container, refer to root.airspeed or model:
          property real currentSpeed: root.airspeed

          // Calculate min & max speeds for the tape
          property real minSpeed: {
              var s = Math.floor((currentSpeed - scaleRange) / stepSize) * stepSize
              // If negative speeds don’t make sense, clamp to zero:
              return Math.max(s, 0)
          }
          property real maxSpeed: Math.ceil((currentSpeed + scaleRange) / stepSize) * stepSize

          // A small horizontal marker line showing the center (actual airspeed)
          Rectangle {
              id: centerMarker
              anchors.horizontalCenter: parent.horizontalCenter
              width: parent.width
              height: 2
              color: "yellow"
              y: ladderCenter - height/2
          }

          // The Repeater for tick marks
          Repeater {
              model: {
                  var list = []
                  for (var s = minSpeed; s <= maxSpeed; s += stepSize) {
                      list.push(s)
                  }
                  return list
              }

              delegate: Item {
                  width: parent.width
                  height: 20

                  // position so that the currentSpeed is exactly at ladderCenter
                  // Higher speeds appear above the center (smaller y), lower are below
                  y: ladder.ladderCenter - ((modelData - ladder.currentSpeed) * ladder.pxPerMps)

                  // The tick mark
                  Rectangle {
                      anchors {
                          left: parent.left
                          verticalCenter: parent.verticalCenter
                      }
                      width: 15
                      height: 2
                      color: "#ffffff"
                  }

                  // The numeric label
                  Text {
                      anchors.verticalCenter: parent.verticalCenter
                      anchors.left: parent.left
                      anchors.leftMargin: 20
                      color: "#ffffff"
                      font.pixelSize: 12
                      text: modelData.toFixed(0)

                      // Alternatively, anchor right if you prefer
                      // anchors.right: parent.right
                      // anchors.rightMargin: 4
                  }

                  // Hide or fade ticks if they fall out of view
                  visible: (y + height > 0) && (y < ladder.ladderHeight)
              }
          }
      }
  }

*/

    Rectangle { //NEW LINE FOR altitude EDITED
      id: altitudeBox
      z: 100 // Ensure it's on top of other items
      //width: 100
      //height: 30
      color: "black"
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.rightMargin: 8
      radius: 4
      opacity: 0.8

      // Give some padding around the text
      property int horizontalPadding: 6
      property int verticalPadding: 6

      // Make the Rectangle’s implicit size dependent on the text’s size
      implicitWidth: altitudeText.implicitWidth + (horizontalPadding * 2)
      implicitHeight: altitudeText.implicitHeight + (verticalPadding * 2)

      Text { //NEW LINE FOR AIRSPEED EDITED
        id: altitudeText
        anchors.centerIn: parent
        color: "white"
        font.pixelSize: 12
        text: qsTr("Alt: %1").arg(root.altitude.toFixed(1)+ "m")
      }
    }
}

