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
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick
import QtGraphs
import QtQuick.Effects
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
  required property AccelerometerModel model

  //
  // Custom properties
  //
  readonly property int trackWidth: root.height >= 120 ? 4 : 2

  //
  // Widget layout
  //
  ColumnLayout {
    spacing: 0
    anchors.margins: 8
    anchors.fill: parent

    //
    // Create the widget background
    //
    Item {
      id: container
      readonly property real gaugeSize: Math.min(root.width, root.height) - range.implicitHeight - 12

      Layout.minimumWidth: gaugeSize
      Layout.maximumWidth: gaugeSize
      Layout.minimumHeight: gaugeSize
      Layout.maximumHeight: gaugeSize
      Layout.alignment: Qt.AlignHCenter

      //
      // Background gradient + glow
      //
      Item {
        opacity: 0.42
        anchors.fill: parent

        Rectangle {
          id: bg
          radius: width / 2
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["polar_background"]
        }

        MultiEffect {
          source: bg
          anchors.fill: bg
          enabled: !Cpp_Misc_ModuleManager.softwareRendering
          visible: !Cpp_Misc_ModuleManager.softwareRendering

          blur: 1
          blurMax: 64
          brightness: 0.6
          saturation: 0.2
          blurEnabled: true
        }
      }

      //
      // Create the polar plot contour
      //
      Item {
        anchors.fill: container

        Rectangle {
          radius: width / 2
          anchors.fill: parent
          color: "transparent"
          border.width: root.trackWidth
          border.color: Cpp_ThemeManager.colors["polar_foreground"]
        }

        Rectangle {
          radius: width / 2
          color: "transparent"
          anchors.fill: parent
          border.width: root.trackWidth
          anchors.margins: container.width / 4
          border.color: Cpp_ThemeManager.colors["polar_foreground"]
        }

        Rectangle {
          radius: width / 2
          color: "transparent"
          anchors.fill: parent
          border.width: root.trackWidth
          anchors.margins: container.width / 8
          border.color: Cpp_ThemeManager.colors["polar_foreground"]
        }

        Rectangle {
          radius: width / 2
          anchors.centerIn: parent
          width: root.trackWidth * 2
          height: root.trackWidth * 2
          color: Cpp_ThemeManager.colors["polar_foreground"]
        }

        Rectangle {
          width: root.trackWidth
          color: Cpp_ThemeManager.colors["polar_foreground"]
          anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
          }
        }

        Rectangle {
          height: root.trackWidth
          color: Cpp_ThemeManager.colors["polar_foreground"]
          anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }
        }
      }

      //
      // Create polar position indicator
      //
      Rectangle {
        id: indicator
        radius: width / 2
        color: "transparent"
        border.width: root.trackWidth
        border.color: Cpp_ThemeManager.colors["polar_indicator"]
        width: Math.min(Math.max(root.trackWidth * 8, container.height / 10), root.trackWidth * 12)
        height: Math.min(Math.max(root.trackWidth * 8, container.height / 10), root.trackWidth * 12)

        Rectangle {
          radius: width / 2
          anchors.centerIn: parent
          width: parent.width / 2
          height: parent.height / 2
          color: Cpp_ThemeManager.colors["polar_indicator"]
        }

        // Full width represents 16 G, divided by 2 to get the radius
        property real radiusScale: container.width / 2

        // Calculate the position based on magnitude (r) and angle (theta)
        x: container.width / 2 + (model.magnitude / 16) * radiusScale * Math.cos(model.theta * Math.PI / 180) - width / 2
        y: container.height / 2 - (model.magnitude / 16) * radiusScale * Math.sin(model.theta * Math.PI / 180) - height / 2

        Behavior on x {NumberAnimation{}}
        Behavior on y {NumberAnimation{}}
      }

      //
      // Make indicator glow
      //
      MultiEffect {
        source: indicator
        anchors.fill: indicator
        enabled: !Cpp_Misc_ModuleManager.softwareRendering
        visible: !Cpp_Misc_ModuleManager.softwareRendering

        blur: 1
        blurMax: 128
        brightness: 0.2
        saturation: 0.2
        blurEnabled: true
      }

      //
      // Make indicator glow (x2)
      //
      MultiEffect {
        source: indicator
        anchors.fill: indicator
        enabled: !Cpp_Misc_ModuleManager.softwareRendering
        visible: !Cpp_Misc_ModuleManager.softwareRendering

        blur: 0.3
        blurMax: 64
        brightness: 0.2
        saturation: 0.2
        blurEnabled: true
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
      units: "G"
      minValue: 0
      maxValue: 16
      alarm: value > 12
      rangeVisible: false
      Layout.fillHeight: true
      value: root.model.magnitude
      maximumWidth: root.width * 0.3
      Layout.minimumWidth: implicitWidth
      Layout.alignment: Qt.AlignHCenter
    }

    //
    // Spacer
    //
    Item {
      implicitHeight: 8
    }
  }
}
