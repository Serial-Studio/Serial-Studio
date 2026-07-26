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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../" as Widgets

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property GPSWidget model
  required property string widgetId

  //
  // Window flags
  //
  readonly property bool hasToolbar: toolbar.shown

  //
  // Configure module widget on load
  //
  onModelChanged: {
    if (model) {
      model.visible = true
      model.parent = container
      model.anchors.fill = container
      _mapType.model = model.mapTypes
    }
  }

  //
  // Restore persisted settings; runs after ALL initial properties are applied, since
  // onModelChanged can fire while widgetId is still empty during object creation
  //
  Component.onCompleted: {
    if (model) {
      const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

      model.autoCenter = s["autoCenter"] !== undefined ? s["autoCenter"] : model.autoCenter
      model.plotTrajectory = s["plotTrajectory"] !== undefined ? s["plotTrajectory"] : model.plotTrajectory
      model.showWeather = s["showWeather"] !== undefined ? s["showWeather"] : model.showWeather
      model.showNasaWeather = s["showNasaWeather"] !== undefined ? s["showNasaWeather"] : model.showNasaWeather
      model.mapType = s["mapType"] !== undefined ? s["mapType"] : model.mapType

      _mapType.currentIndex = model.mapType
      _autoCenter.checked = model.autoCenter
      _showWeather.checked = model.showWeather
      _plotTrajectory.checked = model.plotTrajectory
      _showNasaWeather.checked = model.showNasaWeather
    }
  }

  //
  // Add toolbar
  //
  WidgetToolbar {
    id: toolbar

    windowRoot: root.windowRoot

    anchors {
      leftMargin: 8
      rightMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    Widgets.Combo {
      id: _mapType

      Layout.fillWidth: true
      Layout.maximumWidth: 220
      onCurrentIndexChanged: {
        if (root.model) {
          if (root.model.mapType !== currentIndex) {
            root.model.mapType = currentIndex
            currentIndex = root.model.mapType
            Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "mapType", root.model.mapType)
          }
        }
      }
      Layout.alignment: Qt.AlignVCenter
      displayText: qsTr("Base Map: %1").arg(currentText)
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      id: _plotTrajectory

      onClicked: {
        if (root.model) {
          root.model.plotTrajectory = !root.model.plotTrajectory
          checked = root.model.plotTrajectory
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "plotTrajectory", root.model.plotTrajectory)
        }
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("Plot Trajectory")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "poliline", 24)
    }

    DashboardToolButton {
      id: _showWeather

      onClicked: {
        if (root.model) {
          root.model.showWeather = !root.model.showWeather
          _showWeather.checked = root.model.showWeather
          _showNasaWeather.checked = root.model.showNasaWeather
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showWeather", root.model.showWeather)
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showNasaWeather", root.model.showNasaWeather)
        }
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("Show Weather")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "weather", 24)
    }

    DashboardToolButton {
      id: _showNasaWeather

      onClicked: {
        if (root.model) {
          root.model.showNasaWeather = !root.model.showNasaWeather
          _showWeather.checked = root.model.showWeather
          _showNasaWeather.checked = root.model.showNasaWeather
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showWeather", root.model.showWeather)
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showNasaWeather", root.model.showNasaWeather)
        }
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("NASA Weather Overlay")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "nasa", 24)
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      id: _autoCenter

      onClicked: {
        if (root.model) {
          root.model.autoCenter = !root.model.autoCenter
          checked = root.model.autoCenter
          Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "autoCenter", root.model.autoCenter)
        }
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("Auto Center")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "crosshair", 24)
    }

    DashboardToolButton {
      onClicked: {
        if (root.model)
          root.model.zoomLevel = root.model.zoomLevel + 1
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("Zoom In")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "zoom-in", 24)
    }

    DashboardToolButton {
      onClicked: {
        if (root.model)
          root.model.zoomLevel = root.model.zoomLevel - 1
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("Zoom Out")
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "zoom-out", 24)
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
    anchors.topMargin: toolbar.height
  }
}
