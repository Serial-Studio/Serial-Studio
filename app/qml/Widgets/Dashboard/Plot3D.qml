/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
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
  required property Plot3DWidget model

  //
  // Window flags
  //
  readonly property bool hasToolbar: root.width >= toolbar.implicitWidth
  readonly property int toolbarHeight: windowRoot.objectName === "ExternalWindow" ? 47 : 48

  //
  // Disable navigation
  //
  Connections {
    target: windowRoot

    function onFocusedChanged() {
      if (model)
        model.enabled = windowRoot.focused
    }
  }

  //
  // Configure widget on load
  //
  onModelChanged: {
    if (model) {
      model.visible = true
      model.parent = container
      model.anchors.fill = container
    }
  }

  //
  // Load default camera
  //
  onVisibleChanged: {
    if (visible)
      animateToView(300, 0, 225, 0, 0)
  }

  //
  // Zoom in/out shortcuts
  //
  Shortcut {
    enabled: windowRoot.focused
    onActivated: model.worldScale += 0.1
    sequences: [StandardKey.ZoomIn]
  } Shortcut {
    enabled: windowRoot.focused
    onActivated: model.worldScale -= 0.1
    sequences: [StandardKey.ZoomOut]
  }

  //
  // Animations
  //
  NumberAnimation {
    id: zoomAnimation
    target: model
    property: "worldScale"
    duration: 400
  } NumberAnimation {
    id: angleXAnimation
    target: model
    property: "cameraAngleX"
    duration: 400
  } NumberAnimation {
    id: angleYAnimation
    target: model
    property: "cameraAngleY"
    duration: 400
  } NumberAnimation {
    id: angleZAnimation
    target: model
    property: "cameraAngleZ"
    duration: 400
  } NumberAnimation {
    id: offsetXAnimation
    target: model
    property: "cameraOffsetX"
    duration: 400
  } NumberAnimation {
    id: offsetYAnimation
    target: model
    property: "cameraOffsetY"
    duration: 400
  }

  //
  // Moves the model to the given position & setup, with animations
  //
  function animateToView(angleX, angleY, angleZ, offsetX, offsetY) {
    zoomAnimation.to = model.idealWorldScale
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
  RowLayout {
    id: toolbar

    spacing: 4
    visible: root.hasToolbar
    height: root.hasToolbar ? 48 : 0

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    ToolButton {
      id: _interpolate

      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: model.interpolationEnabled
      onClicked: model.interpolationEnabled = !model.interpolationEnabled
      icon.source: model.interpolationEnabled ?
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-on.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-off.svg"
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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/orbit.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: !model.orbitNavigation
      onClicked: model.orbitNavigation = false
      icon.source: "qrc:/rcc/icons/dashboard-buttons/pan.svg"
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
      onClicked: animateToView(300, 0, 225, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/orthogonal_view.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      onClicked: animateToView(360, 0, 360, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/top_view.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      onClicked: animateToView(270, 0, 270, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/left_view.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      onClicked: animateToView(270, 0, 180, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/front_view.svg"
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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/anaglyph.svg"
    }

    ToolButton {
      id: _anaglyph

      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      enabled: model.anaglyphEnabled
      checked: model.invertEyePositions
      opacity: model.anaglyphEnabled ? 1 : 0
      icon.source: "qrc:/rcc/icons/dashboard-buttons/invert.svg"
      onClicked: model.invertEyePositions = !model.invertEyePositions
    }

    Slider {
      id: _eyeSeparation

      to: 100
      from: 30
      stepSize: 1
      Layout.fillWidth: true
      Layout.maximumWidth: 128
      enabled: model.anaglyphEnabled
      value: model.eyeSeparation * 1e3
      opacity: model.anaglyphEnabled ? 1 : 0
      onValueChanged: {
        if (!isNaN(value) && model) {
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

  //
  // Widget view
  //
  Item {
    id: container
    anchors.fill: parent
    anchors.topMargin: root.hasToolbar ? root.toolbarHeight : 0
  }
}
