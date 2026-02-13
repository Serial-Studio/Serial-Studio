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
  readonly property bool angleLabelsVisible: {
    const minGaugeSize = 200
    const labelMargin = 40
    const noLabelMargin = 12
    const leftPanelMaxWidth = 120
    const containerLeftMargin = 8
    const containerRightMargin = 8
    const containerTopMargin = 8
    const containerBottomMargin = 8
    const spacingWithLabels = 16
    const toolbarHeight = 48

    const totalHorizontalOverhead = containerLeftMargin + containerRightMargin +
                                     leftPanelMaxWidth + spacingWithLabels
    const totalVerticalOverhead = containerTopMargin + containerBottomMargin +
                                   (root.hasToolbar ? toolbarHeight : 0)

    const polarAreaWidth = root.width - totalHorizontalOverhead
    const polarAreaHeight = root.height - totalVerticalOverhead

    const widthWithLabels = polarAreaWidth - labelMargin * 2
    const heightWithLabels = polarAreaHeight - labelMargin * 2
    const gaugeSizeWithLabels = Math.min(widthWithLabels, heightWithLabels)

    const widthWithoutLabels = polarAreaWidth - noLabelMargin * 2
    const heightWithoutLabels = polarAreaHeight - noLabelMargin * 2
    const gaugeSizeWithoutLabels = Math.min(widthWithoutLabels, heightWithoutLabels)

    if (gaugeSizeWithLabels < minGaugeSize)
      return false

    if (polarAreaWidth < labelMargin * 2 + minGaugeSize ||
        polarAreaHeight < labelMargin * 2 + minGaugeSize)
      return false

    const aspectRatioOk = (widthWithLabels / heightWithLabels) >= 0.75 &&
                          (heightWithLabels / widthWithLabels) >= 0.75

    const shrinkageAcceptable = gaugeSizeWithLabels >= (gaugeSizeWithoutLabels * 0.8)

    return aspectRatioOk && shrinkageAcceptable && root.height >= 320 && root.width >= Math.min(380, Math.max(320, root.height * 0.8))
  }

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
  // Utility function to normalize angles from 0 to 360
  //
  function normalize360(angle) {
    var normalized = angle % 360;
    if (normalized < 0)
      normalized += 360;

    return normalized;
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
  // Main container - Row layout with indicators on left, polar plot on right
  //
  RowLayout {
    id: container
    spacing: root.angleLabelsVisible ? 16 : 8

    anchors {
      topMargin: 8
      leftMargin: 8
      rightMargin: 8
      bottomMargin: 8
      top: toolbar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }

    //
    // Vertical indicator strip on the left
    //
    ColumnLayout {
      spacing: 4
      Layout.maximumWidth: 120
      Layout.alignment: Qt.AlignVCenter

      Item {
        Layout.fillHeight: true
      }

      Rectangle {
        border.width: 1
        implicitHeight: 38
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["widget_base"]
        border.color: root.currentG > (root.displayMaxG * 0.75)
                      ? Cpp_ThemeManager.colors["alarm"]
                      : Cpp_ThemeManager.colors["widget_border"]

        Column {
          anchors.fill: parent
          anchors.margins: 4
          spacing: 1

          Label {
            width: parent.width
            opacity: 0.6
            text: qsTr("G-FORCE")
            color: Cpp_ThemeManager.colors["widget_text"]
            font: Cpp_Misc_CommonFonts.customMonoFont(0.66)
            horizontalAlignment: Text.AlignHCenter
          }

          Label {
            width: parent.width
            color: Cpp_ThemeManager.colors["widget_text"]
            font: Cpp_Misc_CommonFonts.monoFont
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: (root.currentG.toFixed(2) + "").padStart(5, ' ') + " @ " +
                  (root.normalize360(root.currentTheta).toFixed(0) + "").padStart(3, ' ') + "°"
          }
        }
      }

      Rectangle {
        border.width: 1
        implicitHeight: 38
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["widget_base"]
        border.color: Cpp_ThemeManager.colors["widget_border"]

        Column {
          anchors.fill: parent
          anchors.margins: 4
          spacing: 1

          Label {
            width: parent.width
            opacity: 0.6
            text: qsTr("PITCH ↕")
            color: Cpp_ThemeManager.colors["widget_text"]
            font: Cpp_Misc_CommonFonts.customMonoFont(0.66)
            horizontalAlignment: Text.AlignHCenter
          }

          Label {
            width: parent.width
            font: Cpp_Misc_CommonFonts.monoFont
            text: (root.currentPitch.toFixed(2) + "").padStart(7, ' ') + "°"
            color: Cpp_ThemeManager.colors["widget_text"]
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
          }
        }
      }

      Rectangle {
        border.width: 1
        implicitHeight: 38
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["widget_base"]
        border.color: Cpp_ThemeManager.colors["widget_border"]

        Column {
          anchors.fill: parent
          anchors.margins: 4
          spacing: 1

          Label {
            width: parent.width
            opacity: 0.6
            text: qsTr("ROLL ↔")
            color: Cpp_ThemeManager.colors["widget_text"]
            font: Cpp_Misc_CommonFonts.customMonoFont(0.66)
            horizontalAlignment: Text.AlignHCenter
          }

          Label {
            width: parent.width
            font: Cpp_Misc_CommonFonts.monoFont
            text: (root.currentRoll.toFixed(2) + "").padStart(7, ' ') + "°"
            color: Cpp_ThemeManager.colors["widget_text"]
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
          }
        }
      }

      Item {
        Layout.fillHeight: true
      }
    }

    //
    // Polar plot area on the right
    //
    Item {
      id: polarArea
      Layout.fillWidth: true
      Layout.fillHeight: true
      property real margin: root.angleLabelsVisible ? 40 : 12

      //
      // The gauge is always square, centered in the polar area
      //
      readonly property real gaugeSize: Math.min(width - margin * 2, height - margin * 2)

      //
      // Dark circular background
      //
      Rectangle {
        id: polarCircle
        radius: width / 2
        anchors.centerIn: parent
        width: polarArea.gaugeSize
        height: polarArea.gaugeSize
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
        // Radial dotted lines for each 30° angle
        //
        Canvas {
          anchors.fill: parent
          opacity: 0.15

          onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.strokeStyle = Cpp_ThemeManager.colors["polar_foreground"];
            ctx.lineWidth = 1;
            ctx.setLineDash([4, 4]);

            var centerX = width / 2;
            var centerY = height / 2;
            var radius = Math.min(width, height) / 2;

            for (var i = 0; i < 12; i++) {
              var angle = (i * 30) * Math.PI / 180;
              var x = centerX + radius * Math.cos(angle);
              var y = centerY - radius * Math.sin(angle);

              ctx.beginPath();
              ctx.moveTo(centerX, centerY);
              ctx.lineTo(x, y);
              ctx.stroke();
            }
          }

          Connections {
            target: Cpp_ThemeManager
            function onColorsChanged() {
              parent.requestPaint();
            }
          }

          Component.onCompleted: requestPaint()
        }

        //
        // Concentric grid circles (drawn over radial lines)
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
            z: 1
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
          z: 2
        }

        //
        // Crosshair lines (main axes)
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
          z: 1
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
          z: 1
        }


        //
        // Angle labels around the perimeter (every 30 degrees)
        //
        Repeater {
          model: 12

          delegate: Text {
            required property int index

            readonly property real angle: index * 30
            readonly property real angleRad: angle * Math.PI / 180
            readonly property real radius: rings.width / 2 + 18

            x: rings.width / 2 + radius * Math.cos(angleRad) - width / 2
            y: rings.height / 2 - radius * Math.sin(angleRad) - height / 2
            visible: rings.width >= 180

            text: angle + "°"
            color: Cpp_ThemeManager.colors["polar_foreground"]
            opacity: 0.5
            font: Cpp_Misc_CommonFonts.customMonoFont(0.6)
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

      //
      // Mouse area for cursor tracking
      //
      MouseArea {
        id: cursorTracker
        anchors.fill: rings
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: true

        property real cursorMagnitude: 0
        property real cursorAngle: 0
        property bool isInsideCircle: false

        onPositionChanged: (mouse) => {
                             var centerX = rings.width / 2
                             var centerY = rings.height / 2
                             var dx = mouse.x - centerX
                             var dy = centerY - mouse.y

                             var distanceFromCenter = Math.sqrt(dx * dx + dy * dy)
                             var maxRadius = rings.width / 2

                             isInsideCircle = distanceFromCenter <= maxRadius

                             if (isInsideCircle) {
                               cursorMagnitude = (distanceFromCenter / maxRadius) * root.displayMaxG
                               cursorAngle = root.normalize360(Math.atan2(dy, dx) * 180 / Math.PI)
                             }
                           }

        //
        // Cursor crosshair (vertical arms)
        //
        Rectangle {
          width: 1
          height: 12
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
          x: cursorTracker.mouseX - width / 2
          y: cursorTracker.mouseY - height - 4
        }

        Rectangle {
          width: 1
          height: 12
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
          x: cursorTracker.mouseX - width / 2
          y: cursorTracker.mouseY + 4
        }

        //
        // Cursor crosshair (horizontal arms)
        //
        Rectangle {
          width: 12
          height: 1
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
          x: cursorTracker.mouseX - width - 4
          y: cursorTracker.mouseY - height / 2
        } Rectangle {
          width: 12
          height: 1
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
          x: cursorTracker.mouseX + 4
          y: cursorTracker.mouseY - height / 2
        }

        //
        // Cursor value label (positioned below and right of cursor to stay visible)
        //
        Rectangle {
          visible: cursorTracker.containsMouse && cursorTracker.isInsideCircle
          x: Math.min(cursorTracker.mouseX + 16, rings.width - width - 4)
          y: Math.max(4, Math.min(cursorTracker.mouseY + 16, rings.height - height - 4))
          width: valueLabel.width + 8
          height: valueLabel.height + 4
          color: Cpp_ThemeManager.colors["tooltip_base"]
          radius: 3
          border.width: 1
          border.color: Cpp_ThemeManager.colors["tooltip_text"]

          Label {
            id: valueLabel
            anchors.centerIn: parent
            text: cursorTracker.cursorMagnitude.toFixed(2) + "G @ " +
                  cursorTracker.cursorAngle.toFixed(0) + "°"
            color: Cpp_ThemeManager.colors["tooltip_text"]
            font: Cpp_Misc_CommonFonts.customMonoFont(0.7)
            elide: Text.ElideRight
          }
        }
      }
    }
  }
}
