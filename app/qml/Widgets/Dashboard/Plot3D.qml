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
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root
  clip: true

  //
  // Widget data inputs
  //
  property Plot3DWidget model
  property color color: "transparent"

  //
  // Configure widget to fill the parent once it's set
  //
  onModelChanged: {
    model.z = 0
    model.anchors.fill = parent
    model.anchors.topMargin = toolbar.height + toolbar.y
  }

  NumberAnimation {
    id: zoomAnimation
    target: model
    property: "zoom"
    duration: 400
  }

  NumberAnimation {
    id: angleXAnimation
    target: model
    property: "cameraAngleX"
    duration: 400
  }

  NumberAnimation {
    id: angleYAnimation
    target: model
    property: "cameraAngleY"
    duration: 400
  }

  NumberAnimation {
    id: angleZAnimation
    target: model
    property: "cameraAngleZ"
    duration: 400
  }

  NumberAnimation {
    id: offsetXAnimation
    target: model
    property: "cameraOffsetX"
    duration: 400
  }

  NumberAnimation {
    id: offsetYAnimation
    target: model
    property: "cameraOffsetY"
    duration: 400
  }

  //
  // Moves the model to the given position & setup, with animations
  //
  function animateToView(zoom, angleX, angleY, angleZ, offsetX, offsetY) {
    zoomAnimation.to = zoom
    zoomAnimation.start()

    angleXAnimation.to = angleX
    angleXAnimation.start()

    angleYAnimation.to = angleY
    angleYAnimation.start()

    angleZAnimation.to = angleZ
    angleZAnimation.start()

    offsetXAnimation.to = offsetX
    offsetXAnimation.start()

    offsetYAnimation.to = offsetY
    offsetYAnimation.start()
  }

  //
  // Add toolbar
  //
  Rectangle {
    z: 1000
    id: toolbar
    height: 48
    color: Cpp_ThemeManager.colors["widget_window"]

    anchors {
      topMargin: -1
      leftMargin: -1
      rightMargin: -1
      top: parent.top
      left: parent.left
      right: parent.right
    }

    Rectangle {
      height: 1
      color: Cpp_ThemeManager.colors["widget_border"]
      anchors {
        margins: 0
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
    }

    RowLayout {
      spacing: 4
      anchors.margins: 8
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        checked: model.interpolationEnabled
        onClicked: model.interpolationEnabled = !model.interpolationEnabled
        icon.source: model.interpolationEnabled ?
                       "qrc:/rcc/icons/dashboard/buttons/interpolate-on.svg" :
                       "qrc:/rcc/icons/dashboard/buttons/interpolate-off.svg"
      }

      Rectangle {
        implicitWidth: 1
        implicitHeight: 24
        color: Cpp_ThemeManager.colors["widget_border"]
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        checked: model.orbitNavigation
        onClicked: model.orbitNavigation = true
        icon.source: "qrc:/rcc/icons/dashboard/buttons/orbit.svg"
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        checked: !model.orbitNavigation
        onClicked: model.orbitNavigation = false
        icon.source: "qrc:/rcc/icons/dashboard/buttons/pan.svg"
      }

      Rectangle {
        implicitWidth: 1
        implicitHeight: 24
        color: Cpp_ThemeManager.colors["widget_border"]
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        onClicked: animateToView(0.05, -60, 0, -135, 0, 0)
        icon.source: "qrc:/rcc/icons/dashboard/buttons/orthogonal_view.svg"
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        onClicked: animateToView(0.05, 0, 0, 0, 0, 0)
        icon.source: "qrc:/rcc/icons/dashboard/buttons/top_view.svg"
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        onClicked: animateToView(0.05, -90, 0, -90, 0, 0)
        icon.source: "qrc:/rcc/icons/dashboard/buttons/left_view.svg"
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        onClicked: animateToView(0.05, -90, 0, -180, 0, 0)
        icon.source: "qrc:/rcc/icons/dashboard/buttons/front_view.svg"
      }

      Rectangle {
        implicitWidth: 1
        implicitHeight: 24
        color: Cpp_ThemeManager.colors["widget_border"]
      }

      ToolButton {
        width: 24
        height: 24
        icon.width: 18
        icon.height: 18
        icon.color: "transparent"
        checked: model.anaglyphEnabled
        onClicked: model.anaglyphEnabled = !model.anaglyphEnabled
        icon.source: "qrc:/rcc/icons/dashboard/buttons/anaglyph.svg"
      }

      Slider {
        to: 100
        from: 30
        stepSize: 1
        Layout.fillWidth: true
        Layout.maximumWidth: 128
        visible: model.anaglyphEnabled
        enabled: model.anaglyphEnabled
        value: model.eyeSeparation * 1e3
        onValueChanged: {
          if (model) {
            var separation = value / 1e3
            if (model.eyeSeparation !== separation)
              model.eyeSeparation = separation
          }
        }
      }

      Item {
        Layout.fillWidth: true
      }
    }
  }
}
