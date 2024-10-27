/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
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
      value: model.value
      units: model.units
      maxValue: model.maxValue
      minValue: model.minValue
      maximumWidth: root.width * 0.3
      rangeVisible: root.height >= 120
      alarm: root.model.alarmValue !== 0 && root.model.value >= root.model.alarmValue

      Layout.fillHeight: true
      Layout.minimumWidth: implicitWidth
    }

    //
    // Spacer
    //
    Item {
      implicitWidth: 8
    }
  }
}
