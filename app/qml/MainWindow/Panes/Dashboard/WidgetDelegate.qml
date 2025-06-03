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
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Widgets.MiniWindow {
  id: root
  implicitWidth: minimumWidth
  implicitHeight: minimumHeight
  focused: taskBar.activeWindow === root
  icon: SerialStudio.dashboardWidgetIcon(widget.widgetType)

  //
  // Input properties
  //
  required property int widgetIndex
  required property SS_Ui.TaskBar taskBar
  required property SS_Ui.WindowManager windowManager

  //
  // Set minimum size
  //
  readonly property int minimumWidth: 320
  readonly property int minimumHeight: 320

  //
  // Button events
  //
  onCloseClicked: taskBar.closeWindow(root)
  onRestoreClicked: taskBar.activeWindow = root
  onMinimizeClicked: taskBar.minimizeWindow(root)
  onMaximizeClicked: taskBar.activeWindow = root

  //
  // Auto-layout hacks to avoid issues with animations
  //
  onStateChanged: _timer.start()
  Timer {
    id: _timer
    running: false
    interval: 250
    repeat: false
    onTriggered: windowManager.triggerLayoutUpdate()
  }

  //
  // Update window state automatically
  //
  Connections {
    target: taskBar

    function onWindowStatesChanged() {
      let state = taskBar.windowState(root)
      switch (state) {
      case SS_Ui.TaskbarModel.WindowNormal:
        if (root.maximized)
          root.state = "maximized"
        else
          root.state = "normal"
        break
      case SS_Ui.TaskbarModel.WindowClosed:
        root.state = "closed"
        break
      case SS_Ui.TaskbarModel.WindowMinimized:
        root.state = "minimized"
        break
      }
    }
  }

  //
  // Add widget background
  //
  Rectangle {
    clip: true
    border.width: 1
    radius: root.radius
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["widget_window"]
    border.color: Cpp_ThemeManager.colors["window_border"]
    anchors.topMargin: root.captionHeight + (root.hasToolbar ? 48 : 0) - 1

    Rectangle {
      color: parent.color
      height: root.radius
      anchors {
        topMargin: 1
        leftMargin: 1
        rightMargin: 1
        top: parent.top
        left: parent.left
        right: parent.right
      }
    }
  }

  //
  // Render a widget inside the pane
  //
  DashboardWidget {
    id: widget
    widgetIndex: root.widgetIndex
  } Item {
    id: loader
    clip: true
    anchors.margins: 1
    anchors.fill: parent
    anchors.topMargin: root.captionHeight

    property var widgetInstance: null

    Component.onCompleted: {
      const component = Qt.createComponent(widget.widgetQmlPath)
      if (component.status === Component.Ready) {
        if (widgetInstance) {
          if (widgetInstance.settings)
            widgetInstance.settings.sync()

          widgetInstance.destroy()
        }

        widgetInstance = component.createObject(loader, {
                                                  model: widget.widgetModel,
                                                  windowRoot: root,
                                                  color: widget.widgetColor
                                                })

        if (!widgetInstance) {
          console.error("Failed to create widget from", widget.widgetQmlPath)
          return
        }

        if (widgetInstance.hasToolbar !== undefined) {
          root.hasToolbar = widgetInstance.hasToolbar
          if (widgetInstance.hasToolbarChanged !== undefined) {
            widgetInstance.hasToolbarChanged.connect(function () {
              root.hasToolbar = widgetInstance.hasToolbar
            })
          }
        }

        widgetInstance.anchors.fill = loader
      }

      else if (component.status === Component.Error)
        console.error("Component load error:", component.errorString())
    }

    Connections {
      target: Cpp_ThemeManager

      function onThemeChanged() {
        if (loader.widgetInstance !== null)
          loader.widgetInstance.color = widget.widgetColor
      }
    }
  }
}
