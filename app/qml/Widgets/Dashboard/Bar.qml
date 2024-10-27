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
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  property var model: BarModel {}
  property color color: Cpp_ThemeManager.colors["highlight"]

  //
  // Widget layout
  //
  RowLayout {
    spacing: 0
    anchors.margins: 8
    anchors.fill: parent

    //
    // Spacer
    //
    Item {
      Layout.fillWidth: true
    }

    //
    // Bar/Tank rectangle
    //
    Rectangle {
      id: tank
      clip: true
      border.width: 1
      Layout.fillHeight: true
      Layout.minimumWidth: root.width * 0.3
      Layout.maximumWidth: root.width * 0.3
      color: Cpp_ThemeManager.colors["widget_base"]
      border.color: Cpp_ThemeManager.colors["widget_border"]

      //
      // Normal level indicator
      //
      Rectangle {
        id: normalIndicator
        color: root.color
        width: parent.width - 2 * tank.border.width
        height: root.model.alarmFractionalValue > 0
                ? Math.min(root.model.fractionalValue, root.model.alarmFractionalValue) * (tank.height - 2 * tank.border.width)
                : root.model.fractionalValue * (tank.height - 2 * tank.border.width)

        anchors {
          left: parent.left
          right: parent.right
          bottom: parent.bottom
          margins: parent.border.width
        }

      }

      //
      // Alarm level indicator
      //
      Rectangle {
        color: Cpp_ThemeManager.colors["alarm"]
        width: parent.width - 2 * tank.border.width
        height: root.model.alarmFractionalValue > 0
                ? Math.max(0, root.model.fractionalValue - root.model.alarmFractionalValue) * (tank.height - 2 * tank.border.width)
                : 0

        anchors {
          bottomMargin: 0
          left: parent.left
          right: parent.right
          bottom: normalIndicator.top
          margins: parent.border.width
        }

        visible: root.model.alarmValue !== 0 && root.model.value >= root.model.alarmValue
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.fillWidth: true
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
