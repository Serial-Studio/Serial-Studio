/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Widgets.MiniWindow {
  id: root

  state: "normal"
  width: minimumWidth
  height: minimumHeight
  animationsEnabled: false
  implicitWidth: minimumWidth
  implicitHeight: minimumHeight
  focused: taskBar.activeWindow === root
  windowControlsVisible: !Cpp_UI_TaskbarSettings.taskbarHidden
  shadowEnabled: root.state === "normal" && !windowManager.autoLayoutEnabled
  visible: root.state === "normal" || root.state === "maximized"
           || root.hideAnimationRunning

  //
  // Input properties
  //
  required property int widgetIndex
  required property SS_Ui.TaskBar taskBar
  required property SS_Ui.WindowManager windowManager

  //
  // Emitted by embedded widgets (via windowRoot) to pop another dashboard widget
  // into an external window; handled by DashboardCanvas
  //
  signal externalWidgetRequested(int windowId)

  //
  // Set minimum size
  //
  readonly property int minimumWidth: 356
  readonly property int minimumHeight: 320

  //
  // Button events
  //
  onCloseClicked: {
    taskBar.activeWindow = root
    taskBar.closeWindow(root)
  }
  onRestoreClicked: taskBar.activeWindow = root
  onMinimizeClicked: {
    taskBar.activeWindow = root
    taskBar.minimizeWindow(root)
  }
  onMaximizeClicked: taskBar.activeWindow = root
  onExternalWindowClicked: taskBar.activeWindow = root

  //
  // Auto-layout hacks to avoid issues with animations
  //
  onStateChanged: _timer.start()
  Timer {
    id: _timer

    repeat: false
    interval: 250
    running: false
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
          windowRoot.title       = widgetTitle
          windowRoot.deviceIndex = widgetSourceId
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
                                                    color: dashboardWidget.widgetColor,
                                                    widgetId: dashboardWidget.widgetId
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
      updateWindowState()
    }

    function onHighlightWidget(windowId) {
      if (windowId === root.widgetIndex)
        root.highlighted = true
    }
  }

  //
  // Helper function to update window state from taskbar model
  //
  function updateWindowState() {
    let state = taskBar.windowState(root)
    switch (state) {
    case SS_Ui.TaskbarModel.WindowNormal:
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
      anchors {
        topMargin: 1
        leftMargin: 1
        rightMargin: 1
        top: parent.top
        left: parent.left
        right: parent.right
      }

      color: parent.color
      height: root.radius
    }
  }

  //
  // Per-source connection state (suppressed during replay)
  //
  property bool sourceDisconnected: false
  readonly property bool replayActive: SerialStudio.isAnyPlayerOpen()
  function _refreshSourceConnection() {
    sourceDisconnected = !replayActive
                         && !Cpp_Benchmark_Runner.running
                         && !Cpp_IO_Manager.isDeviceConnected(root.deviceIndex)
  }
  Component.onCompleted: {
    _refreshSourceConnection()
    Qt.callLater(function() { root.animationsEnabled = true })
  }
  onDeviceIndexChanged: _refreshSourceConnection()
  Connections {
    target: Cpp_Benchmark_Runner
    function onRunningChanged() { root._refreshSourceConnection() }
  } Connections {
    target: Cpp_IO_Manager
    function onConnectedChanged() { root._refreshSourceConnection() }
  } Connections {
    target: Cpp_CSV_Player
    function onOpenChanged() { root._refreshSourceConnection() }
  } Connections {
    target: Cpp_MDF4_Player
    function onOpenChanged() { root._refreshSourceConnection() }
  }
  Loader {
    active: Cpp_CommercialBuild
    sourceComponent: Connections {
      target: Cpp_Sessions_Player
      function onOpenChanged() { root._refreshSourceConnection() }
    }
  }

  //
  // Embedded contents
  //
  Item {
    id: container

    clip: true
    anchors.margins: 1
    anchors.fill: parent
    anchors.topMargin: root.captionHeight
    LayoutMirroring.enabled: false
    LayoutMirroring.childrenInherit: true
    Component.onCompleted: widgetLoader.createObject(container, {windowRoot: root})

    //
    // Disable interaction with the widget's controls while disconnected
    //
    enabled: !root.sourceDisconnected

    //
    // Grayscale + slight blur when disconnected
    //
    layer.enabled: root.sourceDisconnected && Cpp_Misc_GraphicsBackend.effectsEnabled
    layer.effect: MultiEffect {
      blur: 0.4
      blurMax: 16
      saturation: -1.0
      brightness: -0.15
      blurEnabled: true
    }
  }

  //
  // Disconnected overlay
  //
  Item {
    id: disconnectedOverlay

    anchors.fill: container
    visible: root.sourceDisconnected

    //
    // Eat clicks and wheel events so the user can't drive a dead widget
    //
    MouseArea {
      hoverEnabled: true
      anchors.fill: parent
      acceptedButtons: Qt.AllButtons
      onWheel: function(wheel) { wheel.accepted = true }
    }

    Rectangle {
      id: badge

      radius: 6
      border.width: 1
      anchors.centerIn: parent
      color: Cpp_ThemeManager.colors["widget_window"]
      border.color: Cpp_ThemeManager.colors["window_border"]
      implicitWidth: badgeRow.implicitWidth + 24
      implicitHeight: badgeRow.implicitHeight + 16
      opacity: 0.95

      RowLayout {
        id: badgeRow

        spacing: 10
        anchors.centerIn: parent

        Image {
          Layout.preferredWidth: 24
          Layout.preferredHeight: 24
          sourceSize: Qt.size(24, 24)
          fillMode: Image.PreserveAspectFit
          source: "qrc:/icons/notifications/warning.svg"
        }

        Label {
          text: qsTr("Device Disconnected")
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.boldUiFont
        }
      }
    }
  }
}
