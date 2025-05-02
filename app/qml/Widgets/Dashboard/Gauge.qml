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
 * SPDX-License-Identifier: GPL-3.0-or-later
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
  property GaugeModel model: GaugeModel{}
  property color color: Cpp_ThemeManager.colors["highlight"]

  //
  // Widget layout
  //
  RowLayout {
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
      Layout.alignment: Qt.AlignVCenter
      Layout.maximumWidth: root.width - range.width - 64

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
        width: control.width + 10
        height: control.height + 10
        visible: control.alarmEnabled

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
      implicitWidth: 4
    }

    //
    // Range/scale + current value display
    //
    VisualRange {
      id: range
      value: model.value
      units: model.units
      maxValue: model.maxValue
      minValue: model.minValue
      maximumWidth: root.width * 0.3
      rangeVisible: root.height >= 120
      alarm: root.model.alarmValue !== 0 && root.model.value >= root.model.alarmValue

      Layout.fillHeight: true
      Layout.minimumWidth: implicitWidth
      Layout.maximumWidth: implicitWidth
    }

    //
    // Spacer
    //
    Item {
      implicitWidth: 8
    }
  }
}
