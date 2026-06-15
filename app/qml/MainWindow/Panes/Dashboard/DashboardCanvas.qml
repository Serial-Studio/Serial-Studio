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

Item {
  id: root

  //
  // Custom input properties
  //
  required property SS_Ui.TaskBar taskBar
  property var taskbarView: null
  onTaskBarChanged: {
    taskBar.windowManager = _wm
  }

  //
  // Window manager access
  //
  property alias windowManager: _wm
  property alias backgroundImage: _wm.backgroundImage

  //
  // External widget windows, keyed by windowId and parented to the canvas so they
  // survive workspace switches; they only close when widget indices remap
  //
  property var externalWidgetWindows: ({})

  //
  // Suppresses open-state persistence during forced closes, restores, and app
  // teardown so only user-driven open/close actions rewrite the saved set
  //
  property bool persistExternalWindows: true

  function openExternalWidgetWindow(windowId) {
    const existing = externalWidgetWindows[windowId]
    if (existing) {
      existing.displayWindow()
      return
    }

    const win = externalWidgetWindowComponent.createObject(root, {
      "widgetIndex": windowId
    })

    if (!win)
      return

    externalWidgetWindows[windowId] = win
    win.externalWidgetRequested.connect(root.openExternalWidgetWindow)
    win.closing.connect(function() {
      const toolType = SerialStudio.isDashboardTool(win.stableType) ? win.stableType : -1
      delete root.externalWidgetWindows[windowId]
      win.destroy()
      if (root.persistExternalWindows && !app.quitting) {
        if (toolType >= 0)
          root.setToolEnabled(toolType, false)
        else
          root.saveExternalWindowStates()
      }
    })

    if (persistExternalWindows)
      saveExternalWindowStates()
  }

  function closeExternalWidgetWindows() {
    persistExternalWindows = false
    const windows = externalWidgetWindows
    externalWidgetWindows = {}
    for (const id in windows)
      windows[id].close()

    persistExternalWindows = true
  }

  //
  // Writes the stable identities of the open external windows into the project
  // file so they reopen on project reload
  //
  function saveExternalWindowStates() {
    if (Cpp_AppState.operationMode !== SerialStudio.ProjectFile)
      return

    let states = []
    for (const id in externalWidgetWindows) {
      const win = externalWidgetWindows[id]
      if (SerialStudio.isDashboardTool(win.stableType))
        continue

      states.push({
        "widgetType": win.stableType,
        "relativeIndex": win.stableIndex
      })
    }

    Cpp_JSON_ProjectModel.saveWidgetSetting("externalWindows", "data", states)
  }

  //
  // Reopens the external windows recorded in the project, resolving each stable
  // identity to its current windowId; unresolvable entries are skipped, not pruned
  //
  function restoreExternalWindowStates() {
    if (Cpp_AppState.operationMode !== SerialStudio.ProjectFile)
      return

    const states = Cpp_JSON_ProjectModel.widgetSettings("externalWindows")["data"]
    if (!states || !states.length)
      return

    persistExternalWindows = false
    const count = Cpp_UI_Dashboard.totalWidgetCount
    for (let i = 0; i < states.length; ++i) {
      if (SerialStudio.isDashboardTool(states[i]["widgetType"]))
        continue

      for (let id = 0; id < count; ++id) {
        if (Cpp_UI_Dashboard.widgetType(id) === states[i]["widgetType"]
            && Cpp_UI_Dashboard.relativeIndex(id) === states[i]["relativeIndex"]) {
          root.openExternalWidgetWindow(id)
          break
        }
      }
    }

    persistExternalWindows = true
  }

  //
  // Dashboard tools (console, notifications, clock, stopwatch) only exist as
  // external windows; their enabled flags map one-to-one to window visibility
  //
  readonly property var dashboardTools: [
    SerialStudio.DashboardTerminal,
    SerialStudio.DashboardNotificationLog,
    SerialStudio.DashboardClock,
    SerialStudio.DashboardStopwatch
  ]

  function toolEnabled(type) {
    switch (type) {
    case SerialStudio.DashboardTerminal:        return Cpp_UI_Dashboard.terminalEnabled
    case SerialStudio.DashboardNotificationLog: return Cpp_UI_Dashboard.notificationLogEnabled
    case SerialStudio.DashboardClock:           return Cpp_UI_Dashboard.clockEnabled
    case SerialStudio.DashboardStopwatch:       return Cpp_UI_Dashboard.stopwatchEnabled
    }

    return false
  }

  function setToolEnabled(type, enabled) {
    switch (type) {
    case SerialStudio.DashboardTerminal:
      Cpp_UI_Dashboard.terminalEnabled = enabled
      break
    case SerialStudio.DashboardNotificationLog:
      Cpp_UI_Dashboard.notificationLogEnabled = enabled
      break
    case SerialStudio.DashboardClock:
      Cpp_UI_Dashboard.clockEnabled = enabled
      break
    case SerialStudio.DashboardStopwatch:
      Cpp_UI_Dashboard.stopwatchEnabled = enabled
      break
    }
  }

  //
  // Opens or closes each tool window so visibility matches its enabled flag
  //
  function syncToolWindows() {
    const count = Cpp_UI_Dashboard.totalWidgetCount
    for (let i = 0; i < dashboardTools.length; ++i) {
      const type = dashboardTools[i]

      let windowId = -1
      for (let id = 0; id < count; ++id) {
        if (Cpp_UI_Dashboard.widgetType(id) === type) {
          windowId = id
          break
        }
      }

      if (windowId < 0)
        continue

      const win = externalWidgetWindows[windowId]
      if (toolEnabled(type) && !win)
        openExternalWidgetWindow(windowId)

      else if (!toolEnabled(type) && win) {
        persistExternalWindows = false
        win.close()
        persistExternalWindows = true
      }
    }
  }

  Component {
    id: externalWidgetWindowComponent

    ExternalWidgetWindow {}
  }

  //
  // Debounces restore until the dashboard rebuild settles
  //
  Timer {
    id: externalWindowRestoreTimer

    interval: 250
    repeat: false
    onTriggered: {
      root.restoreExternalWindowStates()
      root.syncToolWindows()
    }
  }

  //
  // The canvas is created lazily after the dashboard is already built (dataReset and
  // widgetCountChanged fired before the Connections below existed), so restore once here
  //
  Component.onCompleted: externalWindowRestoreTimer.restart()

  //
  // Close pop-out windows with the rest of the app instead of dying at C++ teardown;
  // the quitting guard in each window's closing handler keeps the saved open-state.
  //
  Connections {
    target: app

    function onQuittingChanged() {
      if (app.quitting)
        root.closeExternalWidgetWindows()
    }
  }

  Connections {
    target: Cpp_UI_Dashboard

    function onDataReset() {
      root.closeExternalWidgetWindows()
      externalWindowRestoreTimer.restart()
    }

    function onWidgetCountChanged() {
      root.closeExternalWidgetWindows()
      externalWindowRestoreTimer.restart()
    }

    function onTerminalEnabledChanged() {
      root.syncToolWindows()
    }

    function onNotificationLogEnabledChanged() {
      root.syncToolWindows()
    }

    function onClockEnabledChanged() {
      root.syncToolWindows()
    }

    function onStopwatchEnabledChanged() {
      root.syncToolWindows()
    }
  }

  //
  // Desktop context menu
  //
  Menu {
    id: contextMenu

    MenuItem {
      text: qsTr("Set Wallpaper…")
      onTriggered: _wm.selectBackgroundImage()
    }

    MenuItem {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Clear Wallpaper")
      onTriggered: _wm.clearBackgroundImage()
      enabled: _wm.backgroundImage.length > 0
    }

    MenuSeparator {

    }

    MenuItem {
      text: qsTr("Tile Windows")
      onTriggered: _wm.autoLayout()
    }
  }

  //
  // Commercial features notification
  //
  Widgets.ProNotice {
    id: commercialNotification

    anchors {
      margins: -1
      top: parent.top
      left: parent.left
      right: parent.right
    }

    z: 2
    titleText: qsTr("Pro features detected in this project.")
    activationFlag: Cpp_UI_Dashboard.containsCommercialFeatures
    subtitleText: qsTr("Fallback widgets are active. Purchase a license for full functionality.")
  }

  //
  // Window canvas
  //
  Item {
    id: windowCanvas

    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      topMargin: commercialNotification.visible ? -1 : 0
      top: commercialNotification.visible ? commercialNotification.bottom : parent.top
    }

    //
    // Set window manager
    //
    SS_Ui.WindowManager {
      id: _wm

      anchors.fill: parent
      onRightClicked: (x, y) => contextMenu.popup(x, y)

      //
      // Passive hover monitor so manual-mode resize cursors show over stacked windows
      //
      HoverHandler {
        id: resizeHover

        enabled: !_wm.autoLayoutEnabled
        property point hoverPosition: resizeHover.point.position
        onHoveredChanged: if (!resizeHover.hovered) _wm.updateHoverCursor(Qt.point(-1, -1))
        onHoverPositionChanged: if (resizeHover.hovered) _wm.updateHoverCursor(hoverPosition)
      }
    }

    //
    // Initialize widget windows
    //
    Instantiator {
      id: loader

      model: taskBar.taskbarButtons

      delegate: WidgetDelegate {
        required property var model

        parent: _wm
        windowManager: _wm
        taskBar: root.taskBar
        title: model.widgetName
        widgetIndex: model.windowId

        Component.onDestruction: {
          if (root.taskBar) {
            root.taskBar.unregisterWindow(this)
            if (root.taskBar.activeWindow === this)
              root.taskBar.activeWindow = null
          }
        }

        Component.onCompleted: root.taskBar.registerWindow(widgetIndex, this)

        onExternalWindowClicked: root.openExternalWidgetWindow(widgetIndex)
        onExternalWidgetRequested: (windowId) => root.openExternalWidgetWindow(windowId)
      }

      onCountChanged: {
        if (count !== taskBar.taskbarButtons.rowCount())
          return

        const firstWindow = taskBar.firstWindow()
        if (firstWindow)
          taskBar.activeWindow = firstWindow
      }
    }

    //
    // Snap indicator
    //
    Rectangle {
      border.width: 1
      z: _wm.zCounter + 9999
      x: _wm.snapIndicator.x
      y: _wm.snapIndicator.y
      width: _wm.snapIndicator.width
      height: _wm.snapIndicator.height
      visible: _wm.snapIndicatorVisible
      color: Cpp_ThemeManager.colors["snap_indicator_background"]
      border.color: Cpp_ThemeManager.colors["snap_indicator_border"]
    }

    //
    // Empty workspace placeholder
    //
    Item {
      z: _wm.zCounter + 20
      anchors.fill: parent
      visible: taskBar.activeGroupId >= 1000
               && taskBar.taskbarButtons.rowCount() === 0

      ColumnLayout {
        spacing: 16
        anchors.centerIn: parent

        Image {
          opacity: 0.4
          sourceSize: Qt.size(64, 64)
          Layout.alignment: Qt.AlignHCenter
          source: "qrc:/icons/panes/dashboard.svg"
        }

        Label {
          opacity: 0.8
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("Empty Workspace")
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
          font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
        }

        Label {
          opacity: 0.5
          wrapMode: Text.WordWrap
          Layout.maximumWidth: 300
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
          text: qsTr("Use the search bar to find and add widgets, "
                    + "or right-click a widget in another workspace "
                    + "to add it here.")
        }

        Item {
          implicitHeight: 4
        }

        Widgets.IconButton {
          iconSize: 16
          topPadding: 8
          leftPadding: 16
          bottomPadding: 8
          rightPadding: 20
          text: qsTr("Search Widgets")
          Layout.alignment: Qt.AlignHCenter
          icon.source: "qrc:/icons/buttons/search.svg"
          onClicked: {
            if (root.taskbarView)
              root.taskbarView.focusSearch()
          }
        }
      }
    }
  }
}
