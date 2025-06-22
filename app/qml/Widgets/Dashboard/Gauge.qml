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
import QtQuick.Shapes
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
  required property GaugeModel model
  required property var windowRoot

  //
  // Widget layout
  //
  ColumnLayout {
    spacing: 0
    anchors.margins: 8
    anchors.fill: parent

    //
    // Gauge control
    //
    CircularSlider {
      id: control

      //
      // Layout
      //
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.topMargin: trackWidth / 2
      Layout.alignment: Qt.AlignHCenter

      //
      // Colors
      //
      progressColor: root.color
      alarmColor: Cpp_ThemeManager.colors["alarm"]
      trackColor: Cpp_ThemeManager.colors["widget_base"]

      //
      // Angles + rotation
      //
      endAngle: 320
      rotation: 180
      startAngle: 40
      capStyle: Qt.FlatCap

      //
      // Anumations
      //
      Behavior on value {NumberAnimation{}}

      //
      // Track widths
      //
      progressWidth: trackWidth
      trackWidth: Math.min(20, Math.max(5, Math.min(root.width, root.height) / 10)) + 2

      //
      // Dataset/model input
      //
      value: root.model.value
      minValue: root.model.minValue
      maxValue: root.model.maxValue
      alarmValue: root.model.alarmValue
      alarmEnabled: root.model.alarmValue !== 0

      //
      // Alarm line
      //
      Shape {
        layer.samples: 8
        layer.enabled: true
        width: control.width - 2 * control.trackWidth
        height: control.height
        visible: control.alarmEnabled && root.width >= root.height

        Behavior on opacity {NumberAnimation{}}
        opacity: root.model.value >= root.model.alarmValue ? 1 : 0.5

        ShapePath {
          capStyle: Qt.RoundCap
          fillColor: "transparent"
          strokeColor: Cpp_ThemeManager.colors["alarm"]
          strokeWidth: Math.max(control.trackWidth / 4, 2)

          PathAngleArc {
            centerX: control.width / 2
            centerY: control.height / 2
            startAngle: control.alarmStartAngle - 90
            sweepAngle: control.endAngle - control.alarmStartAngle
            radiusX: control.baseRadius + control.trackWidth / 2 + 5
            radiusY: control.baseRadius + control.trackWidth / 2 + 5
          }
        }
      }
    }

    //
    // Spacer
    //
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
      maximumWidth: root.width * 0.3
      Layout.alignment: Qt.AlignHCenter
      Layout.minimumWidth: implicitWidth
      alarm: root.model.alarmValue !== 0 && root.model.value >= root.model.alarmValue
    }

    //
    // Spacer
    //
    Item {
      implicitHeight: 8
    }
  }
}
