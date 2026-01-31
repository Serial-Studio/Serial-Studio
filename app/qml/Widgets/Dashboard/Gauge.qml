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
  // Gauge parameters
  //
  readonly property real startAngleDeg: 135
  readonly property real endAngleDeg: 45
  readonly property real angleRangeDeg: 360 - startAngleDeg + endAngleDeg
  readonly property real fontSize: Math.max(8, Math.min(11, Math.min(width, height) / 28))
  readonly property int subTicksPerMajor: 4
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

      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

      from: model.minValue
      to: model.maxValue
      value: model.value

      snapMode: Dial.NoSnap
      inputMode: Dial.Vertical
      startAngle: startAngleDeg
      endAngle: endAngleDeg

      background: Item {
        implicitWidth: 200
        implicitHeight: 200

        readonly property real gaugeSize: Math.min(control.width, control.height) * 0.75

        Rectangle {
          id: gaugeFace
          anchors.centerIn: parent
          width: parent.gaugeSize
          height: parent.gaugeSize
          radius: width / 2
          color: Cpp_ThemeManager.colors["widget_base"]
          border.width: Math.max(8, width / 18)
          border.color: Cpp_ThemeManager.colors["widget_border"]

          Repeater {
            model: tickCount
            delegate: Item {
              required property int index
              readonly property real frac: index / (tickCount - 1)
              readonly property real tickValue: model.minValue + (1 - frac) * (model.maxValue - model.minValue)
              readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
              readonly property real angleRad: angleDeg * Math.PI / 180
              readonly property real tickRadius: gaugeFace.width / 2 - gaugeFace.border.width / 2
              readonly property real labelRadius: gaugeFace.width / 2 + Math.max(16, fontSize * 1.8)

              Rectangle {
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
                width: 4
                height: gaugeFace.border.width * 0.7
                color: Cpp_ThemeManager.colors["widget_text"]
                rotation: parent.angleDeg + 90
                transformOrigin: Item.Center
                antialiasing: true
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
              }
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
              readonly property real angleRad: angleDeg * Math.PI / 180
              readonly property real tickRadius: gaugeFace.width / 2 - gaugeFace.border.width / 2

              Rectangle {
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
                width: 2
                height: gaugeFace.border.width * 0.4
                color: Cpp_ThemeManager.colors["widget_border"]
                rotation: parent.angleDeg + 90
                transformOrigin: Item.Center
                antialiasing: true
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
            color: root.color
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
          width: Math.max(3, parent.width * 0.015)
          height: parent.height * 0.38
          radius: width / 2
          color: root.color
          antialiasing: true
          transformOrigin: Item.Bottom
          rotation: control.angle

          Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height - height / 2
            width: Math.max(14, needle.parent.width * 0.08)
            height: width
            radius: width / 2
            color: Cpp_ThemeManager.colors["widget_border"]
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
