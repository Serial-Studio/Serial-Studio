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
  property CompassModel model: CompassModel{}
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
    Item {
      id: control

      //
      // Set widget size
      //
      readonly property real widgetSize: Math.min(control.width, control.height)

      //
      // Layout
      //
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignVCenter

      //
      // Compass background
      //
      Image {
        anchors.centerIn: parent
        source: "qrc:/rcc/instruments/compass_background.svg"
        sourceSize: Qt.size(control.widgetSize, control.widgetSize)
      }

      //
      // Compass indicator
      //
      Image {
        anchors.centerIn: parent
        rotation: root.model.value
        source: "qrc:/rcc/instruments/compass_needle.svg"
        sourceSize: Qt.size(control.widgetSize, control.widgetSize)

        Behavior on rotation {NumberAnimation{}}
      }

      //
      // Compass case
      //
      Image {
        anchors.centerIn: parent
        source: "qrc:/rcc/instruments/compass_case.svg"
        sourceSize: Qt.size(control.widgetSize, control.widgetSize)
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
      units: "°"
      alarm: false
      minValue: 0
      maxValue: 360
      textValue: root.model.text
      maximumWidth: root.width * 0.3
      rangeVisible: root.height >= 120

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
