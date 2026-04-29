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
  required property string widgetId

  //
  // Window flags
  //
  property bool hasToolbar: root.height >= 220 && root.width >= toolbar.implicitWidth
  readonly property int toolbarHeight: windowRoot.objectName === "ExternalWindow" ? 47 : 48

  //
  // Configure module widget on load, then restore persisted settings
  //
  onModelChanged: {
    if (model) {
      model.visible = true
      model.parent = container
      model.anchors.fill = container
      _mapType.model = model.mapTypes

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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/crosshair.svg"
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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/poliline.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    DashboardToolButton {
      onClicked: {
        if (root.model)
          root.model.zoomLevel = root.model.zoomLevel + 1
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("Zoom In")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/zoom-in.svg"
    }

    DashboardToolButton {
      onClicked: {
        if (root.model)
          root.model.zoomLevel = root.model.zoomLevel - 1
      }
      icon.width: 24
      icon.height: 24
      ToolTip.text: qsTr("Zoom Out")
      icon.source: "qrc:/rcc/icons/dashboard-buttons/zoom-out.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/weather.svg"
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
      icon.source: "qrc:/rcc/icons/dashboard-buttons/nasa.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    ToolButton {
      icon.width: 24
      enabled: false
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/map.svg"
    }

    ComboBox {
      id: _mapType

      Layout.fillWidth: true
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
