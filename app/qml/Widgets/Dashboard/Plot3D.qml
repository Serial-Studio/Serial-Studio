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
  required property string widgetId
  required property Plot3DWidget model

  //
  // Window flags
  //
  readonly property int toolbarHeight: windowRoot.objectName === "ExternalWindow" ? 47 : 48
  readonly property bool hasToolbar: root.width >= toolbar.implicitWidth && root.height >= 220

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
  // Configure widget on load, then restore persisted settings
  //
  onModelChanged: {
    if (model) {
      model.visible = true
      model.parent = container
      model.anchors.fill = container

      const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

      if (s["interpolationEnabled"] !== undefined)
        model.interpolationEnabled = s["interpolationEnabled"]

      if (s["autoCenter"] !== undefined)
        model.autoCenter = s["autoCenter"]

      if (s["orbitNavigation"] !== undefined)
        model.orbitNavigation = s["orbitNavigation"]

      if (s["anaglyphEnabled"] !== undefined)
        model.anaglyphEnabled = s["anaglyphEnabled"]

      if (s["invertEyePositions"] !== undefined)
        model.invertEyePositions = s["invertEyePositions"]

      if (s["eyeSeparation"] !== undefined)
        model.eyeSeparation = s["eyeSeparation"]
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
    sequences: [StandardKey.ZoomIn]
    onActivated: model.worldScale *= 1.2
  } Shortcut {
    enabled: windowRoot.focused
    sequences: [StandardKey.ZoomOut]
    onActivated: model.worldScale /= 1.2
  }

  //
  // Animations
  //
  NumberAnimation {
    id: zoomAnimation

    target: model
    duration: 400
    property: "worldScale"
  } NumberAnimation {
    id: angleXAnimation

    target: model
    duration: 400
    property: "cameraAngleX"
  } NumberAnimation {
    id: angleYAnimation

    target: model
    duration: 400
    property: "cameraAngleY"
  } NumberAnimation {
    id: angleZAnimation

    target: model
    duration: 400
    property: "cameraAngleZ"
  } NumberAnimation {
    id: offsetXAnimation

    target: model
    duration: 400
    property: "cameraOffsetX"
  } NumberAnimation {
    id: offsetYAnimation

    target: model
    duration: 400
    property: "cameraOffsetY"
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

    DashboardToolButton {
      id: _interpolate

      onClicked: {
        model.interpolationEnabled = !model.interpolationEnabled
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "interpolationEnabled", model.interpolationEnabled)
      }
      checked: model.interpolationEnabled
      ToolTip.text: qsTr("Interpolate")
      icon.source: model.interpolationEnabled ?
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-on.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-off.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      onClicked: {
        model.orbitNavigation = true
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "orbitNavigation", true)
      }
      checked: model.orbitNavigation
      ToolTip.text: qsTr("Orbit Navigation")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/orbit.svg"
    }

    DashboardToolButton {
      onClicked: {
        model.orbitNavigation = false
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "orbitNavigation", false)
      }
      checked: !model.orbitNavigation
      ToolTip.text: qsTr("Pan Navigation")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/pan.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Orthogonal View")
      onClicked: animateToView(300, 0, 225, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/orthogonal_view.svg"
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Top View")
      onClicked: animateToView(360, 0, 360, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/top_view.svg"
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Left View")
      onClicked: animateToView(270, 0, 270, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/left_view.svg"
    }

    DashboardToolButton {
      ToolTip.text: qsTr("Front View")
      onClicked: animateToView(270, 0, 180, 0, 0)
      icon.source: "qrc:/rcc/icons/dashboard-buttons/front_view.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      onClicked: {
        model.autoCenter = !model.autoCenter
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "autoCenter", model.autoCenter)
      }
      checked: model.autoCenter
      ToolTip.text: qsTr("Auto Center")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/center.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      onClicked: {
        model.anaglyphEnabled = !model.anaglyphEnabled
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "anaglyphEnabled", model.anaglyphEnabled)
      }
      checked: model.anaglyphEnabled
      ToolTip.text: qsTr("Anaglyph 3D")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/anaglyph.svg"
    }

    DashboardToolButton {
      id: _anaglyph

      onClicked: {
        model.invertEyePositions = !model.invertEyePositions
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "invertEyePositions", model.invertEyePositions)
      }
      enabled: model.anaglyphEnabled
      checked: model.invertEyePositions
      opacity: model.anaglyphEnabled ? 1 : 0
      ToolTip.text: qsTr("Invert Eye Positions")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/invert.svg"
    }

    Slider {
      id: _eyeSeparation

      to: 100
      from: 30
      stepSize: 1
      onValueChanged: {
        if (!isNaN(value) && model) {
          var separation = value / 1e3

          if (model.eyeSeparation !== separation) {
            model.eyeSeparation = separation
            Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "eyeSeparation", separation)
          }
        }
      }
      Layout.fillWidth: true
      Layout.maximumWidth: 128
      enabled: model.anaglyphEnabled
      value: model.eyeSeparation * 1e3
      opacity: model.anaglyphEnabled ? 1 : 0
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
