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
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Widgets.MiniWindow {
  id: root
  visible: false
  implicitWidth: minimumWidth
  implicitHeight: minimumHeight
  focused: taskBar.activeWindow === root
  shadowEnabled: root.state === "normal"

  //
  // Input properties
  //
  required property int widgetIndex
  required property SS_Ui.TaskBar taskBar
  required property SS_Ui.WindowManager windowManager

  //
  // Set minimum size
  //
  readonly property int minimumWidth: 356
  readonly property int minimumHeight: 320

  //
  // Button events
  //
  onCloseClicked: taskBar.closeWindow(root)
  onRestoreClicked: taskBar.activeWindow = root
  onMinimizeClicked: taskBar.minimizeWindow(root)
  onMaximizeClicked: taskBar.activeWindow = root
  onExternalWindowClicked: externalWindowLoader.active = true

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
  // QML loader component
  //
  Component {
    id: widgetLoader

    Item {
      id: loader

      anchors.fill: parent
      property var windowRoot: null
      property var widgetInstance: null

      DashboardWidget {
        id: dashboardWidget
        widgetIndex: root.widgetIndex
        Component.onCompleted: {
          windowRoot.title = widgetTitle
          if (windowRoot.icon !== undefined)
            windowRoot.icon = SerialStudio.dashboardWidgetIcon(widgetType)
        }
      }

      Component.onCompleted: {
        const component = Qt.createComponent(dashboardWidget.widgetQmlPath)
        if (component.status === Component.Ready) {
          if (widgetInstance) {
            if (widgetInstance.settings)
              widgetInstance.settings.sync()
            widgetInstance.destroy()
          }

          widgetInstance = component.createObject(loader, {
                                                    model: dashboardWidget.widgetModel,
                                                    windowRoot: loader.windowRoot,
                                                    color: dashboardWidget.widgetColor
                                                  })

          if (!widgetInstance) {
            console.error("Failed to create widget from", dashboardWidget.widgetQmlPath)
            return
          }

          if (widgetInstance.hasToolbar !== undefined) {
            windowRoot.hasToolbar = widgetInstance.hasToolbar
            if (widgetInstance.hasToolbarChanged !== undefined) {
              widgetInstance.hasToolbarChanged.connect(function () {
                windowRoot.hasToolbar = widgetInstance.hasToolbar
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
          if (widgetInstance !== null)
            widgetInstance.color = dashboardWidget.widgetColor
        }
      }
    }
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
  // Embedded contents
  //
  Item {
    clip: true
    id: container
    anchors.margins: 1
    anchors.fill: parent
    anchors.topMargin: root.captionHeight
    Component.onCompleted: widgetLoader.createObject(container, {windowRoot: root})
  }

  //
  // External window
  //
  Loader {
    id: externalWindowLoader

    active: false
    asynchronous: true

    sourceComponent: Component {
      Window {
        id: window

        width: 640
        height: 480
        visible: true
        transientParent: null
        objectName: "ExternalWindow"
        minimumWidth: root.minimumWidth
        minimumHeight: root.minimumHeight
        onClosing: externalWindowLoader.active = false

        property bool hasToolbar: false
        readonly property bool focused: true

        Page {
          anchors.fill: parent
          palette.mid: Cpp_ThemeManager.colors["mid"]
          palette.dark: Cpp_ThemeManager.colors["dark"]
          palette.text: Cpp_ThemeManager.colors["text"]
          palette.base: Cpp_ThemeManager.colors["base"]
          palette.link: Cpp_ThemeManager.colors["link"]
          palette.light: Cpp_ThemeManager.colors["light"]
          palette.window: Cpp_ThemeManager.colors["window"]
          palette.shadow: Cpp_ThemeManager.colors["shadow"]
          palette.accent: Cpp_ThemeManager.colors["accent"]
          palette.button: Cpp_ThemeManager.colors["button"]
          palette.midlight: Cpp_ThemeManager.colors["midlight"]
          palette.highlight: Cpp_ThemeManager.colors["highlight"]
          palette.windowText: Cpp_ThemeManager.colors["window_text"]
          palette.brightText: Cpp_ThemeManager.colors["bright_text"]
          palette.buttonText: Cpp_ThemeManager.colors["button_text"]
          palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
          palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
          palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
          palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
          palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
          palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

          Rectangle {
            height: 48
            border.width: 1
            visible: window.hasToolbar
            border.color: Cpp_ThemeManager.colors["window_border"]
            color: Cpp_ThemeManager.colors["window_toolbar_background"]

            anchors {
              margins: -1
              top: parent.top
              left: parent.left
              right: parent.right
            }
          }

          Item {
            anchors.fill: parent
            Component.onCompleted: {
              window.showNormal()
              widgetLoader.createObject(this, {windowRoot: window})
            }
          }
        }
      }
    }
  }
}
