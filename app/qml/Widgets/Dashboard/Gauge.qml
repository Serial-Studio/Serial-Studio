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

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property GaugeModel model

  //
  // Custom properties
  //
  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {NumberAnimation{duration: 100}}

  //
  // Helper properties
  //
  readonly property color fillColor: model.alarmTriggered ? Cpp_ThemeManager.colors["alarm"] : root.color

  //
  // Gauge parameters
  //
  readonly property real endAngleDeg: 135
  readonly property int subTicksPerMajor: 4
  readonly property real startAngleDeg: -135
  readonly property real angleRangeDeg: endAngleDeg - startAngleDeg
  readonly property real fontSize: Math.max(8, Math.min(11, Math.min(width, height) / 28))
  readonly property int tickCount: {
    const size = Math.min(width, height) * 0.75
    const radius = size / 2
    const arcLength = (angleRangeDeg * Math.PI / 180) * radius
    const labelWidth = fontSize * 5
    const minTicks = radius < 60 ? 3 : 5
    const maxTicks = radius < 100 ? 7 : 9
    return Math.max(minTicks, Math.min(maxTicks, Math.floor(arcLength / (labelWidth * 3))))
  }

  function formatValue(val) {
    return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
  }

  //
  // Widget layout
  //
  ColumnLayout {
    spacing: 4
    anchors.fill: parent
    anchors.margins: 8

    Item {
      implicitHeight: 4
    }

    //
    // Gauge control
    //
    Dial {
      id: control

      enabled: false
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

      to: model.maxValue
      value: model.value
      from: model.minValue
      endAngle: endAngleDeg
      snapMode: Dial.NoSnap
      inputMode: Dial.Vertical
      startAngle: startAngleDeg

      background: Item {
        implicitWidth: 200
        implicitHeight: 200

        readonly property real gaugeSize: Math.min(control.width, control.height) * 0.65

        Rectangle {
          id: gaugeFace
          radius: width / 2
          anchors.centerIn: parent
          width: parent.gaugeSize
          height: parent.gaugeSize
          border.color: "transparent"
          border.width: Math.max(8, width / 18)


          gradient: Gradient {
            GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_base"] }
            GradientStop { position: 0.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.05) }
            GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.05) }
          }

          Rectangle {
            border.width: 1
            radius: width / 2
            color: "transparent"
            anchors.fill: parent
            border.color: Qt.rgba(0, 0, 0, 0.1)
            anchors.margins: parent.border.width
          }

          Rectangle {
            opacity: 0.5
            border.width: 2
            radius: width / 2
            color: "transparent"
            width: parent.width
            height: parent.height
            anchors.centerIn: parent
            Behavior on border.color {ColorAnimation{duration: 300}}
            border.color: fillColor
          }

          Repeater {
            model: tickCount
            delegate: Item {
              required property int index
              readonly property real frac: index / (tickCount - 1)
              readonly property real tickValue: root.model.minValue + frac * (root.model.maxValue - root.model.minValue)
              readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property real tickRadius: gaugeFace.width / 2 - gaugeFace.border.width / 2
              readonly property real labelRadius: gaugeFace.width / 2 + Math.max(24, fontSize * 2.8)

              Rectangle {
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
                width: gaugeFace.border.width * 0.7
                height: 2
                radius: 1
                color: Qt.darker(fillColor, 1.3)
                rotation: parent.angleDeg + 90
                transformOrigin: Item.Center
                antialiasing: true
                opacity: 0.8
              }

              Text {
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.labelRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.labelRadius - height / 2
                text: formatValue(parent.tickValue)
                font.pixelSize: fontSize
                font.family: Cpp_Misc_CommonFonts.monoFont.family
                color: Cpp_ThemeManager.colors["widget_text"]
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                style: Text.Raised
                styleColor: Qt.rgba(0, 0, 0, 0.3)
              }
            }
          }

          Item {
            visible: root.model.alarmsDefined
            readonly property real alarmLowFrac: (root.model.alarmLow - root.model.minValue) / (root.model.maxValue - root.model.minValue)
            readonly property real angleDeg: startAngleDeg + alarmLowFrac * angleRangeDeg
            readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
            readonly property real tickRadius: gaugeFace.width / 2 - gaugeFace.border.width / 2

            Rectangle {
              x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
              y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
              width: gaugeFace.border.width * 1.0
              height: 4
              radius: 2
              color: Cpp_ThemeManager.colors["alarm"]
              rotation: parent.angleDeg + 90
              transformOrigin: Item.Center
              antialiasing: true
            }
          }

          Item {
            visible: root.model.alarmsDefined
            readonly property real alarmHighFrac: (root.model.alarmHigh - root.model.minValue) / (root.model.maxValue - root.model.minValue)
            readonly property real angleDeg: startAngleDeg + alarmHighFrac * angleRangeDeg
            readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
            readonly property real tickRadius: gaugeFace.width / 2 - gaugeFace.border.width / 2

            Rectangle {
              x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
              y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
              width: gaugeFace.border.width * 1.0
              height: 4
              radius: 2
              color: Cpp_ThemeManager.colors["alarm"]
              rotation: parent.angleDeg + 90
              transformOrigin: Item.Center
              antialiasing: true
            }
          }

          Repeater {
            model: (tickCount - 1) * subTicksPerMajor
            delegate: Item {
              required property int index
              readonly property int majorIndex: Math.floor(index / subTicksPerMajor)
              readonly property int subIndex: (index % subTicksPerMajor) + 1
              readonly property real frac: (majorIndex + subIndex / (subTicksPerMajor + 1)) / (tickCount - 1)
              readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property real tickRadius: gaugeFace.width / 2 - gaugeFace.border.width / 2

              Rectangle {
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
                width: gaugeFace.border.width * 0.5
                height: 1
                radius: 0.5
                color: Cpp_ThemeManager.colors["widget_border"]
                rotation: parent.angleDeg + 90
                transformOrigin: Item.Center
                antialiasing: true
                opacity: 0.6
              }
            }
          }

          Text {
            visible: model.units.length > 0 && gaugeFace.width > 120
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.border.width + 8
            text: model.units
            font.pixelSize: fontSize
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            color: fillColor
            style: Text.Raised
            styleColor: Qt.rgba(0, 0, 0, 0.3)
          }
        }
      }

      handle: Item {
        anchors.centerIn: parent
        width: control.background.gaugeSize
        height: control.background.gaugeSize

        Rectangle {
          id: needle
          x: parent.width / 2 - width / 2
          y: parent.height / 2 - height
          width: Math.max(5, parent.width * 0.025)
          height: parent.height * 0.38
          radius: width / 2
          antialiasing: true
          transformOrigin: Item.Bottom
          rotation: control.angle

          gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(fillColor, 1.3) }
            GradientStop { position: 0.5; color: fillColor }
            GradientStop { position: 1.0; color: Qt.darker(fillColor, 1.2) }
          }

          Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            radius: width / 2
            color: Qt.rgba(1, 1, 1, 0.4)
          }

          Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height - height / 2
            width: Math.max(14, needle.parent.width * 0.08)
            height: width
            radius: width / 2

            gradient: Gradient {
              GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_border"], 1.2) }
              GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_border"] }
              GradientStop { position: 1.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.3) }
            }

            Rectangle {
              anchors.centerIn: parent
              width: parent.width * 0.4
              height: parent.height * 0.4
              radius: width / 2
              color: Qt.rgba(0, 0, 0, 0.3)
            }
          }
        }
      }

      //
      // Mouse area for cursor tooltip
      //
      MouseArea {
        id: cursorTracker
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: true

        property real cursorValue: model.minValue
        property real cursorAngleDeg: startAngleDeg
        property bool isValidAngle: false

        onPositionChanged: (mouse) => {
          var centerX = width / 2
          var centerY = height / 2
          var dx = mouse.x - centerX
          var dy = mouse.y - centerY

          var screenAngleRad = Math.atan2(dy, dx)
          var screenAngleDeg = screenAngleRad * 180 / Math.PI

          var logicalAngleDeg = screenAngleDeg + 90
          if (logicalAngleDeg > 180)
            logicalAngleDeg -= 360

          if (logicalAngleDeg >= startAngleDeg && logicalAngleDeg <= endAngleDeg) {
            cursorAngleDeg = logicalAngleDeg
            var normalizedAngle = (logicalAngleDeg - startAngleDeg) / angleRangeDeg
            cursorValue = model.minValue + normalizedAngle * (model.maxValue - model.minValue)
            isValidAngle = true
          } else {
            isValidAngle = false
          }
        }

        //
        // Cursor indicator line from gauge center to cursor position
        //
        Rectangle {
          readonly property real gaugeSize: control.background.gaugeSize
          readonly property real borderWidth: Math.max(8, gaugeSize / 18)
          readonly property real tickRadius: gaugeSize / 2 - borderWidth / 2

          width: 2
          height: tickRadius
          radius: 1
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse && cursorTracker.isValidAngle ? 0.6 : 0
          antialiasing: true

          x: cursorTracker.width / 2 - width / 2
          y: cursorTracker.height / 2 - height

          transformOrigin: Item.Bottom
          rotation: cursorTracker.cursorAngleDeg
        }

        //
        // Cursor crosshair (vertical arms)
        //
        Rectangle {
          width: 1
          height: 12
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: cursorTracker.mouseX - width / 2
          y: cursorTracker.mouseY - height - 4
        }

        Rectangle {
          width: 1
          height: 12
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
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
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: cursorTracker.mouseX - width - 4
          y: cursorTracker.mouseY - height / 2
        }

        Rectangle {
          width: 12
          height: 1
          color: Cpp_ThemeManager.colors["polar_indicator"]
          opacity: cursorTracker.containsMouse ? 0.6 : 0
          x: cursorTracker.mouseX + 4
          y: cursorTracker.mouseY - height / 2
        }

        //
        // Cursor value label (tooltip style)
        //
        Rectangle {
          visible: cursorTracker.containsMouse
          x: Math.min(cursorTracker.mouseX + 16, cursorTracker.width - width - 4)
          y: Math.max(4, Math.min(cursorTracker.mouseY + 16, cursorTracker.height - height - 4))
          width: tooltipLabel.width + 8
          height: tooltipLabel.height + 4
          color: Cpp_ThemeManager.colors["tooltip_base"]
          radius: 3
          border.width: 1
          border.color: Cpp_ThemeManager.colors["tooltip_text"]

          Label {
            id: tooltipLabel
            anchors.centerIn: parent
            text: formatValue(cursorTracker.cursorValue) + " " + model.units
            color: Cpp_ThemeManager.colors["tooltip_text"]
            font: Cpp_Misc_CommonFonts.customMonoFont(0.7)
            elide: Text.ElideRight
          }
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    //
    // Range/scale + current value display
    //
    VisualRange {
      id: range
      value: model.value
      units: model.units
      rangeVisible: false
      maxValue: model.maxValue
      minValue: model.minValue
      alarm: model.alarmTriggered
      maximumWidth: Math.min(root.width * 0.8, 200)
      Layout.alignment: Qt.AlignHCenter
      Layout.minimumWidth: implicitWidth
    }

    Item {
      implicitHeight: 8
    }
  }
}
