/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root
  clip: true

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property GyroscopeModel model

  //
  // Custom properties
  //
  property real yawAngle: root.model.yaw
  property real rollAngle: root.model.roll
  property real pitchAngle: root.model.pitch
  property bool integrateValues: root.model.integrateValues
  onIntegrateValuesChanged: {
    if (root.model)
      root.model.integrateValues = root.integrateValues
  }

  //
  // Constants
  //
  readonly property real pitchStep: 5
  readonly property real maxPitch: 180
  readonly property int pitchMarkCount: Math.round((2 * maxPitch) / pitchStep) + 1

  //
  // Utility function to normalize angles from 0 to 360
  //
  function normalize180(angle) {
    var normalized = (angle + 180) % 360;
    if (normalized < 0)
      normalized += 360;

    return normalized - 180;
  }

  //
  // Window flags
  //
  property bool hasToolbar: root.height >= 296

  //
  // Restore persisted settings
  //
  Component.onCompleted: {
    const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

    if (s["integrateValues"] !== undefined)
      root.integrateValues = s["integrateValues"]
  }

  //
  // Toolbar
  //
  RowLayout {
    id: toolbar

    spacing: 4
    visible: root.hasToolbar
    height: root.hasToolbar ? 48 : 0

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    ToolButton {
      onClicked: {
        root.integrateValues = !root.integrateValues
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "integrateValues", root.integrateValues)
      }
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: root.integrateValues
      text: qsTr("Integrate Angles")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/integral.svg"
    }

    Item {
      Layout.fillWidth: true
    }
  }

  //
  // Main container
  //
  Item {
    id: container

    clip: true

    anchors {
      left: parent.left
      top: toolbar.bottom
      right: parent.right
      bottom: parent.bottom
    }

    //
    // Dark background fills the entire widget
    //
    Rectangle {
      color: "#0e1117"
      anchors.fill: parent
    }

    //
    // Attitude indicator fills entire container (extends behind angles)
    //
    Item {
      id: instrument

      clip: true
      anchors.fill: parent

      readonly property real instrumentHeight: height - (angles.visible ? angles.height + 4 : 0)
      readonly property real pixelsPerDegree: instrumentHeight / 90
      readonly property real dialSize: Math.min(width, instrumentHeight)

        //
        // Moving sky + ground + pitch marks (transforms with pitch/roll)
        //
        Item {
          id: movingLayer

          antialiasing: true
          x: (instrument.width - width) / 2
          y: (instrument.instrumentHeight - height) / 2
          width: Math.max(instrument.width, instrument.height) * 3
          height: Math.max(instrument.width, instrument.height) * 3

          //
          // Sky gradient (Garmin-style blue)
          //
          Rectangle {
            width: parent.width
            height: parent.height / 2
            gradient: Gradient {
              GradientStop { position: 0.0; color: "#1a3060" }
              GradientStop { position: 0.4; color: "#3a6aaa" }
              GradientStop { position: 0.8; color: "#5b93c5" }
              GradientStop { position: 1.0; color: "#7eb3d8" }
            }
          }

          //
          // Ground gradient (warm brown)
          //
          Rectangle {
            width: parent.width
            y: parent.height / 2
            height: parent.height / 2
            gradient: Gradient {
              GradientStop { position: 0.0; color: "#a06848" }
              GradientStop { position: 0.2; color: "#7d5233" }
              GradientStop { position: 0.6; color: "#5a3a22" }
              GradientStop { position: 1.0; color: "#3a2415" }
            }
          }

          //
          // Horizon line
          //
          Rectangle {
            height: 3
            color: "#e8e8e8"
            antialiasing: true
            width: parent.width
            y: parent.height / 2 - 1.5
          }

          //
          // Pitch marks and degree labels
          //
          Repeater {
            model: root.pitchMarkCount
            delegate: Item {
              id: mark

              required property int index

              readonly property real pitch: root.maxPitch - (index * root.pitchStep)
              readonly property bool majorMark: Math.abs(pitch % 10) < 0.001
              readonly property bool horizonMark: Math.abs(pitch) < 0.001

              width: movingLayer.width
              height: majorMark ? 24 : 16
              y: (movingLayer.height / 2)
                 - (pitch * instrument.pixelsPerDegree)
                 - (height / 2)

              opacity: {
                var dist = Math.abs(pitch - root.pitchAngle);
                if (dist > 180)
                  dist = 360 - dist;
                return Math.max(0, 1.0 - dist / 45);
              }

              Rectangle {
                id: pitchLine

                color: "#e8e8e8"
                antialiasing: true
                width: mark.horizonMark
                       ? movingLayer.width
                       : mark.majorMark
                         ? instrument.dialSize * 0.22
                         : instrument.dialSize * 0.12
                anchors.centerIn: parent
                height: mark.horizonMark ? 3 : (mark.majorMark ? 2 : 1.5)
              }

              Label {
                color: "#e8e8e8"
                anchors.rightMargin: 6
                anchors.right: pitchLine.left
                visible: !mark.horizonMark && mark.majorMark
                         && root.height >= 140
                anchors.verticalCenter: pitchLine.verticalCenter
                text: Math.round(Math.abs(mark.pitch)).toString()
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                font.pixelSize: Math.max(9, instrument.dialSize / 22) * Cpp_Misc_CommonFonts.widgetFontScale
              }

              Label {
                color: "#e8e8e8"
                anchors.leftMargin: 6
                anchors.left: pitchLine.right
                visible: !mark.horizonMark && mark.majorMark
                         && root.height >= 140
                anchors.verticalCenter: pitchLine.verticalCenter
                text: Math.round(Math.abs(mark.pitch)).toString()
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                font.pixelSize: Math.max(9, instrument.dialSize / 22) * Cpp_Misc_CommonFonts.widgetFontScale
              }
            }
          }

          transform: [
            Translate {
              y: root.pitchAngle * instrument.pixelsPerDegree
            },
            Rotation {
              angle: -root.rollAngle
              origin.x: movingLayer.width / 2
              origin.y: movingLayer.height / 2
            }
          ]
        }

        //
        // Instrument center point (offset up from full container center)
        //
        readonly property real centerY: instrument.instrumentHeight / 2

        //
        // Roll dial glow effect
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
        // Roll pointer glow effect
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
        // Crosshair glow effect
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
        // Roll dial (square, centered on instrument area)
        //
        Image {
          id: rollDial

          smooth: true
          antialiasing: true
          width: instrument.dialSize
          transform: Rotation {
            angle: -root.rollAngle
            origin.x: rollDial.width / 2
            origin.y: rollDial.height / 2
          }
          height: instrument.dialSize
          x: (instrument.width - width) / 2
          fillMode: Image.PreserveAspectFit
          y: instrument.centerY - height / 2
          sourceSize.height: instrument.dialSize
          source: "qrc:/rcc/instruments/attitude_dial.svg"
        }

        //
        // Roll pointer (square, centered on instrument area)
        //
        Image {
          id: rollPointer

          smooth: true
          antialiasing: true
          width: instrument.dialSize
          height: instrument.dialSize
          x: (instrument.width - width) / 2
          fillMode: Image.PreserveAspectFit
          y: instrument.centerY - height / 2
          sourceSize.height: instrument.dialSize
          source: "qrc:/rcc/instruments/attitude_pointer.svg"
        }

        //
        // Aircraft crosshair (centered on instrument area)
        //
        Image {
          id: crosshair

          smooth: true
          antialiasing: true
          sourceSize.width: width
          x: (instrument.width - width) / 2
          fillMode: Image.PreserveAspectFit
          width: instrument.dialSize * 0.75
          y: instrument.centerY - height / 2
          source: "qrc:/rcc/instruments/attitude_crosshair.svg"
        }
      }

    //
    // Angles indicator strip
    //
    Item {
      id: angles

      height: visible ? 38 : 0
      visible: container.height >= 120

      anchors {
        leftMargin: 4
        rightMargin: 4
        bottomMargin: 4
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }

      RowLayout {
        spacing: 4
        anchors.fill: parent

        Rectangle {
          color: "#333"
          border.width: 1
          implicitHeight: 38
          Layout.fillWidth: true
          border.color: "#bebebe"

          Column {
            spacing: 1
            anchors.centerIn: parent

            Text {
              opacity: 0.6
              color: "#ffffff"
              text: qsTr("ROLL ↔")
              anchors.horizontalCenter: parent.horizontalCenter
              font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.66))
            }

            Text {
              color: "#ffffff"
              anchors.horizontalCenter: parent.horizontalCenter
              text: (root.normalize180(root.rollAngle).toFixed(2) + "").padStart(7, ' ') + "\u00B0"
              font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont())
            }
          }
        }

        Rectangle {
          color: "#333"
          border.width: 1
          implicitHeight: 38
          Layout.fillWidth: true
          border.color: "#bebebe"

          Column {
            spacing: 1
            anchors.centerIn: parent

            Text {
              opacity: 0.6
              color: "#ffffff"
              text: qsTr("YAW ↻")
              anchors.horizontalCenter: parent.horizontalCenter
              font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.66))
            }

            Text {
              color: "#ffffff"
              anchors.horizontalCenter: parent.horizontalCenter
              text: (root.normalize180(root.yawAngle).toFixed(2) + "").padStart(7, ' ') + "\u00B0"
              font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont())
            }
          }
        }

        Rectangle {
          color: "#333"
          border.width: 1
          implicitHeight: 38
          Layout.fillWidth: true
          border.color: "#bebebe"

          Column {
            spacing: 1
            anchors.centerIn: parent

            Text {
              opacity: 0.6
              color: "#ffffff"
              text: qsTr("PITCH ↕")
              anchors.horizontalCenter: parent.horizontalCenter
              font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.66))
            }

            Text {
              color: "#ffffff"
              anchors.horizontalCenter: parent.horizontalCenter
              text: (root.normalize180(root.pitchAngle).toFixed(2) + "").padStart(7, ' ') + "\u00B0"
              font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont())
            }
          }
        }
      }
    }
  }
}
