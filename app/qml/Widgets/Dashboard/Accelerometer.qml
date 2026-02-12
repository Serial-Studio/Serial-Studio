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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import QtCore as QtCore

import "../"
import "../../Dialogs" as Dialogs

Item {
  id: root
  clip: true

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property AccelerometerModel model

  //
  // Local properties bound to model values (EMA-filtered in C++)
  //
  property real currentG: root.model.g
  property real currentPitch: root.model.pitch
  property real currentRoll: root.model.roll
  property real currentMagnitude: root.model.magnitude
  property real currentTheta: root.model.theta
  property real displayMaxG: root.model.maxG
  property bool inputInG: root.model.inputInG

  //
  // Window flags
  //
  property bool hasToolbar: root.height >= 296

  //
  // Sync configurable properties to model
  //
  onDisplayMaxGChanged: {
    if (root.model)
      root.model.maxG = root.displayMaxG
  }
  onInputInGChanged: {
    if (root.model)
      root.model.inputInG = root.inputInG
  }

  //
  // Save settings between sessions
  //
  QtCore.Settings {
    id: settings
    category: "Accelerometer"
    property alias maxG: root.displayMaxG
    property alias inputInG: root.inputInG
  }

  //
  // Config dialog
  //
  Dialogs.AccelerometerConfigDialog {
    id: configDialog
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
      icon.width: 18
      icon.height: 18
      text: qsTr("Settings")
      icon.color: "transparent"
      onClicked: configDialog.openDialog(root.model)
      icon.source: "qrc:/rcc/icons/toolbar/settings.svg"
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
    // Polar plot area (between toolbar and info strip)
    //
    Item {
      id: polarArea

      anchors {
        margins: 32
        bottomMargin: 24
        top: parent.top
        left: parent.left
        right: parent.right
        bottom: infoStrip.top
      }

      //
      // The gauge is always square, centered in the polar area
      //
      readonly property real gaugeSize: Math.min(width, height)

      //
      // Dark circular background
      //
      Rectangle {
        radius: width / 2
        width: polarArea.gaugeSize
        height: polarArea.gaugeSize
        anchors.centerIn: parent
        color: Cpp_ThemeManager.colors["polar_background"]
      }

      //
      // Polar plot contour rings
      //
      Item {
        id: rings
        width: polarArea.gaugeSize
        height: polarArea.gaugeSize
        anchors.centerIn: parent

        //
        // Concentric grid circles
        //
        Repeater {
          model: 4
          delegate: Rectangle {
            required property int index

            readonly property real fraction: 1.0 - (index * 0.25)

            radius: width / 2
            color: "transparent"
            border.width: 1
            opacity: 0.4
            border.color: Cpp_ThemeManager.colors["polar_foreground"]
            width: rings.width * fraction
            height: rings.height * fraction
            anchors.centerIn: parent
          }
        }

        //
        // Center dot
        //
        Rectangle {
          radius: width / 2
          anchors.centerIn: parent
          width: 4
          height: 4
          opacity: 0.5
          color: Cpp_ThemeManager.colors["polar_foreground"]
        }

        //
        // Crosshair lines
        //
        Rectangle {
          width: 1
          opacity: 0.3
          color: Cpp_ThemeManager.colors["polar_foreground"]
          anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
          }
        }

        Rectangle {
          height: 1
          opacity: 0.3
          color: Cpp_ThemeManager.colors["polar_foreground"]
          anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }
        }

        //
        // Diagonal grid lines (45 degree intervals)
        //
        Rectangle {
          width: 1
          opacity: 0.15
          color: Cpp_ThemeManager.colors["polar_foreground"]
          height: rings.width * Math.SQRT2
          anchors.centerIn: parent
          rotation: 45
        }

        Rectangle {
          width: 1
          opacity: 0.15
          color: Cpp_ThemeManager.colors["polar_foreground"]
          height: rings.width * Math.SQRT2
          anchors.centerIn: parent
          rotation: -45
        }

        //
        // Ring labels at 25%, 50%, 75%, 100% of maxG
        //
        Repeater {
          model: 4

          delegate: Item {
            required property int index

            readonly property real fraction: (index + 1) / 4.0
            readonly property real labelValue: root.displayMaxG * fraction
            readonly property real ringRadius: rings.width / 2 * fraction

            x: rings.width / 2 + 4
            y: rings.height / 2 - ringRadius - height / 2
            width: labelText.width + 6
            height: labelText.height + 2
            visible: rings.width >= 100

            Rectangle {
              anchors.fill: parent
              color: Cpp_ThemeManager.colors["polar_background"]
              opacity: 0.85
              radius: 2
            }

            Text {
              id: labelText
              anchors.centerIn: parent
              text: parent.labelValue.toFixed(
                parent.labelValue >= 10 ? 0 : 1)
              color: Cpp_ThemeManager.colors["polar_foreground"]
              font.pixelSize: Math.max(8, Math.min(11, rings.width / 22))
              font.family: Cpp_Misc_CommonFonts.monoFont.family
            }
          }
        }
      }

      //
      // Polar position indicator (clamped to circle boundary)
      //
      Rectangle {
        id: indicator
        radius: width / 2
        color: Cpp_ThemeManager.colors["polar_indicator"]

        readonly property real dotSize: Math.max(
          6, Math.min(14, polarArea.gaugeSize / 20))

        width: dotSize
        height: dotSize

        property real halfGauge: polarArea.gaugeSize / 2
        property real normalizedMag: Math.min(
          root.currentMagnitude / root.displayMaxG, 1.0)

        x: polarArea.width / 2
           + normalizedMag * halfGauge
             * Math.cos(root.currentTheta * Math.PI / 180)
           - width / 2
        y: polarArea.height / 2
           - normalizedMag * halfGauge
             * Math.sin(root.currentTheta * Math.PI / 180)
           - height / 2

      }
    }

    //
    // Info strip: Total G | Pitch | Roll
    //
    Item {
      id: infoStrip
      visible: root.height >= 120

      height: visible ? 38 : 0

      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
        leftMargin: 4
        rightMargin: 4
        bottomMargin: 4
      }

      RowLayout {
        spacing: 4
        anchors.fill: parent

        Rectangle {
          color: Cpp_ThemeManager.colors["widget_base"]
          border.width: 1
          implicitHeight: 38
          Layout.fillWidth: true
          border.color: root.currentG > (root.displayMaxG * 0.75)
                        ? Cpp_ThemeManager.colors["alarm"]
                        : Cpp_ThemeManager.colors["widget_border"]

          Column {
            anchors.centerIn: parent
            spacing: 1

            Text {
              anchors.horizontalCenter: parent.horizontalCenter
              text: qsTr("G-FORCE")
              color: Cpp_ThemeManager.colors["widget_text"]
              opacity: 0.6
              font.pixelSize: 8
              font.family: Cpp_Misc_CommonFonts.monoFont.family
            }

            Text {
              anchors.horizontalCenter: parent.horizontalCenter
              text: root.currentG.toFixed(2)
              color: Cpp_ThemeManager.colors["widget_text"]
              font: Cpp_Misc_CommonFonts.monoFont
            }
          }
        }

        Rectangle {
          color: Cpp_ThemeManager.colors["widget_base"]
          border.width: 1
          implicitHeight: 38
          border.color: Cpp_ThemeManager.colors["widget_border"]
          Layout.fillWidth: true

          Column {
            anchors.centerIn: parent
            spacing: 1

            Text {
              anchors.horizontalCenter: parent.horizontalCenter
              text: qsTr("PITCH")
              color: Cpp_ThemeManager.colors["widget_text"]
              opacity: 0.6
              font.pixelSize: 8
              font.family: Cpp_Misc_CommonFonts.monoFont.family
            }

            Text {
              anchors.horizontalCenter: parent.horizontalCenter
              text: root.currentPitch.toFixed(2) + "\u00B0"
              color: Cpp_ThemeManager.colors["widget_text"]
              font: Cpp_Misc_CommonFonts.monoFont
            }
          }
        }

        Rectangle {
          color: Cpp_ThemeManager.colors["widget_base"]
          border.width: 1
          implicitHeight: 38
          border.color: Cpp_ThemeManager.colors["widget_border"]
          Layout.fillWidth: true

          Column {
            anchors.centerIn: parent
            spacing: 1

            Text {
              anchors.horizontalCenter: parent.horizontalCenter
              text: qsTr("ROLL")
              color: Cpp_ThemeManager.colors["widget_text"]
              opacity: 0.6
              font.pixelSize: 8
              font.family: Cpp_Misc_CommonFonts.monoFont.family
            }

            Text {
              anchors.horizontalCenter: parent.horizontalCenter
              text: root.currentRoll.toFixed(2) + "\u00B0"
              color: Cpp_ThemeManager.colors["widget_text"]
              font: Cpp_Misc_CommonFonts.monoFont
            }
          }
        }
      }
    }
  }
}
