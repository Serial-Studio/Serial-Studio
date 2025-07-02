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
  required property GPSWidget model

  //
  // Window flags
  //
  property bool hasToolbar: root.height >= 220 && root.width >= toolbar.implicitWidth
  readonly property int toolbarHeight: windowRoot.objectName === "ExternalWindow" ? 47 : 48

  //
  // Configure module widget on load
  //
  onModelChanged: {
    if (model) {
      model.visible = true
      model.parent = container
      model.anchors.fill = container
      _mapType.model = model.mapTypes
      _mapType.currentIndex = model.mapType
      _autoCenter.checked = model.autoCenter
      _showWeather.checked = root.model.showWeather
      _plotTrajectory.checked = model.plotTrajectory
      _showNasaWeather.checked = root.model.showNasaWeather
    }
  }

  //
  // Add toolbar
  //
  RowLayout {
    id: toolbar

    spacing: 4
    height: 48

    anchors {
      leftMargin: 8
      rightMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    ToolButton {
      id: _autoCenter
      icon.width: 24
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/crosshair.svg"
      onClicked: {
        if (root.model) {
          root.model.autoCenter = !root.model.autoCenter
          checked = root.model.autoCenter
        }
      }
    }

    ToolButton {
      id: _plotTrajectory
      icon.width: 24
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/poliline.svg"
      onClicked: {
        if (root.model) {
          root.model.plotTrajectory = !root.model.plotTrajectory
          checked = root.model.plotTrajectory
        }
      }
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    ToolButton {
      icon.width: 24
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/zoom-in.svg"
      onClicked: {
        if (root.model)
          root.model.zoomLevel = root.model.zoomLevel + 1
      }
    }

    ToolButton {
      icon.width: 24
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/zoom-out.svg"
      onClicked: {
        if (root.model)
          root.model.zoomLevel = root.model.zoomLevel - 1
      }
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    ToolButton {
      id: _showWeather
      icon.width: 24
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/weather.svg"
      onClicked: {
        if (root.model) {
          root.model.showWeather = !root.model.showWeather
          _showWeather.checked = root.model.showWeather
          _showNasaWeather.checked = root.model.showNasaWeather
        }
      }
    }

    ToolButton {
      id: _showNasaWeather
      icon.width: 24
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/nasa.svg"
      onClicked: {
        if (root.model) {
          root.model.showNasaWeather = !root.model.showNasaWeather
          _showWeather.checked = root.model.showWeather
          _showNasaWeather.checked = root.model.showNasaWeather
        }
      }
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    ToolButton {
      icon.width: 24
      icon.height: 24
      enabled: false
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/map.svg"
    }

    ComboBox {
      id: _mapType
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      displayText: qsTr("Base Map: %1").arg(currentText)

      onCurrentIndexChanged: {
        if (root.model) {
          if (root.model.mapType !== currentIndex) {
            root.model.mapType = currentIndex
            currentIndex = root.model.mapType
          }
        }
      }
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
