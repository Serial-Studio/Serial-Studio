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
      Layout.maximumWidth: root.width - range.width - 64

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
      id: range
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
