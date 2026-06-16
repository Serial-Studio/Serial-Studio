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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_UI

import "Dashboard" as DbItems
import "../../Widgets" as Widgets
import "../../Dialogs" as Dialogs

Item {
  id: root

  //
  // Autolayout
  //
  function loadLayout() {
    _mainLayout.loadLayout()
  }

  //
  // Shortcut hooks (proxied to the DashboardLayout's Taskbar)
  //
  function focusTaskbarSearch()      { _mainLayout.focusTaskbarSearch() }
  function toggleStartMenu()         { _mainLayout.toggleStartMenu() }
  function cycleWorkspace(delta)     { _mainLayout.cycleWorkspace(delta) }
  function cycleWindow(delta)        { _mainLayout.cycleWindow(delta) }
  function closeActiveWindow()       { _mainLayout.closeActiveWindow() }
  function minimizeActiveWindow()    { _mainLayout.minimizeActiveWindow() }
  function clearActiveWindow()       { _mainLayout.clearActiveWindow() }
  function toggleAutoLayout()        { _mainLayout.toggleAutoLayout() }
  function jumpToWorkspaceIndex(i)   { _mainLayout.jumpToWorkspaceIndex(i) }

  //
  // Taskbar helper
  //
  property SS_UI.TaskBar taskBar: SS_UI.TaskBar {}

  //
  // Main dashboard layout
  //
  DbItems.DashboardLayout {
    id: _mainLayout

    anchors.fill: parent
    taskBar: root.taskBar
    hostWindow: mainWindow
    onExternalWindowClicked: root.openExternalWindow()
  }

  //
  // Open external dashboard windows, tracked by their stable project id
  //
  property var extWindows: []

  //
  // Suppresses persistence while windows are reconciled against the project file
  //
  property bool restoringExternal: false

  //
  // Clears the restore guard once a reconcile burst settles
  //
  Timer {
    id: _extRestoreSettle

    interval: 600
    repeat: false
    onTriggered: root.restoringExternal = false
  }

  //
  // Coalesces dashboard resets into a single deferred reconcile (avoids re-entrancy)
  //
  Timer {
    id: _extReconcileTimer

    interval: 200
    repeat: false
    onTriggered: root.reconcileExternalWindows()
  }

  //
  // Highest in-use window number, plus one
  //
  function nextWindowNumber() {
    var max = 0
    for (var i = 0; i < extWindows.length; ++i) {
      if (extWindows[i] && extWindows[i].windowNumber > max)
        max = extWindows[i].windowNumber
    }

    return max + 1
  }

  //
  // Spawns an external dashboard window, optionally seeded from a saved record
  //
  function openExternalWindow(record) {
    var rec = record || ({})
    var num = (rec.n > 0) ? rec.n : root.nextWindowNumber()
    var win = _extDashboardComponent.createObject(root, {
      "category": "ExternalDashboard_" + num,
      "windowNumber": num,
      "record": rec
    })

    if (win) {
      extWindows.push(win)
      win.closing.connect(function() {
        var idx = extWindows.indexOf(win)
        if (idx !== -1)
          extWindows.splice(idx, 1)

        win.destroy()
        root.syncExternalWindows()
      })
    }

    root.syncExternalWindows()
  }

  //
  // Serializes the live external windows into the project file
  //
  function syncExternalWindows() {
    if (root.restoringExternal)
      return

    if (Cpp_AppState.operationMode !== SerialStudio.ProjectFile)
      return

    //
    // Never persist while the dashboard is offline: on disconnect every taskbar's
    // active group is forced to -1, which would otherwise overwrite saved workspaces
    //
    if (!Cpp_UI_Dashboard.available)
      return

    var arr = []
    for (var i = 0; i < extWindows.length; ++i) {
      var w = extWindows[i]
      if (!w || !w.extTaskBar)
        continue

      var normal = w.visibility !== Window.Maximized
                   && w.visibility !== Window.FullScreen
                   && w.visibility !== Window.Minimized
      var ws = w.extTaskBar.activeGroupId
      if (ws < 0 && w.record && w.record.workspace !== undefined)
        ws = w.record.workspace

      arr.push({
        "id": w.windowId,
        "n": w.windowNumber,
        "workspace": ws,
        "x": Math.round(normal ? w.x : w.previousX),
        "y": Math.round(normal ? w.y : w.previousY),
        "w": Math.round(normal ? w.width : w.previousWidth),
        "h": Math.round(normal ? w.height : w.previousHeight),
        "maximized": w.isMaximized,
        "minimized": w.visibility === Window.Minimized,
        "fullscreen": w.visibility === Window.FullScreen
      })
    }

    Cpp_JSON_ProjectModel.setExternalWindows(arr)
  }

  //
  // Reconciles open external windows against the project: closes windows that are
  // gone, spawns missing ones, and leaves matching windows untouched (idempotent)
  //
  function reconcileExternalWindows() {
    root.restoringExternal = true
    _extRestoreSettle.restart()

    var ready = Cpp_AppState.operationMode === SerialStudio.ProjectFile
                && Cpp_UI_Dashboard.available
    var saved = ready ? (Cpp_JSON_ProjectModel.externalWindows() || []) : []

    var savedById = ({})
    for (var i = 0; i < saved.length; ++i)
      savedById[saved[i].id] = saved[i]

    var live = ({})
    var snapshot = extWindows.slice()
    for (var j = 0; j < snapshot.length; ++j) {
      var w = snapshot[j]
      if (!w)
        continue

      if (savedById[w.windowId])
        live[w.windowId] = true
      else
        w.close()
    }

    for (var k = 0; k < saved.length; ++k) {
      if (!live[saved[k].id])
        openExternalWindow(saved[k])
    }
  }

  //
  // Reconcile when the dashboard resets or its availability changes (connect/
  // disconnect), and once after this pane is created
  //
  Connections {
    target: Cpp_UI_Dashboard
    function onDataReset() { _extReconcileTimer.restart() }
    function onWidgetCountChanged() { _extReconcileTimer.restart() }
  }

  Component.onCompleted: _extReconcileTimer.restart()

  //
  // Freeze persistence during teardown so destroyed windows can't wipe the saved set
  //
  Component.onDestruction: root.restoringExternal = true

  //
  // External dashboard window component
  //
  Component {
    id: _extDashboardComponent

    Widgets.SmartWindow {
      id: _extWindow

      width: 1024
      height: 600
      visible: true
      minimumWidth: 640
      minimumHeight: 480
      transientParent: null
      title: qsTr("Dashboard %1").arg(windowNumber)
      color: Cpp_ThemeManager.colors["dashboard_background"]


      //
      // Window number for title display
      //
      property int windowNumber: 1

      //
      // Titlebar height for native window integration
      //
      property int titlebarHeight: 0

      //
      // Persisted record (workspace, geometry, state) and resolved identity
      //
      property var record: ({})
      property string windowId: ""
      property int targetWorkspace: -1

      //
      // Re-asserts the saved workspace until it lands (model may settle late on load)
      //
      Timer {
        id: _wsRetry

        repeat: true
        interval: 120
        property int tries: 0
        onTriggered: {
          if (_extWindow.targetWorkspace < 0
              || _extWindow.extTaskBar.activeGroupId === _extWindow.targetWorkspace
              || tries >= 10) {
            tries = 0
            stop()
            return
          }

          _extWindow.extTaskBar.setDesiredGroupId(_extWindow.targetWorkspace)
          ++tries
        }
      }

      //
      // Independent taskbar for this external window
      //
      property SS_UI.TaskBar extTaskBar: SS_UI.TaskBar {
        independentWorkspace: true
        layoutScope: _extWindow.windowId
      }

      //
      // Register with native window system using dashboard background
      //
      onVisibleChanged: {
        if (visible)
          Cpp_NativeWindow.addWindow(
            _extWindow,
            Cpp_ThemeManager.colors["dashboard_background"])
        else
          Cpp_NativeWindow.removeWindow(_extWindow)

        _extWindow.titlebarHeight
            = Cpp_NativeWindow.titlebarHeight(_extWindow)
      }

      //
      // Update native titlebar color on theme changes
      //
      Connections {
        target: Cpp_ThemeManager
        function onThemeChanged() {
          if (_extWindow.visible)
            Cpp_NativeWindow.addWindow(
              _extWindow,
              Cpp_ThemeManager.colors["dashboard_background"])
        }
      }

      //
      // Titlebar background
      //
      Rectangle {
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
        }

        height: _extWindow.titlebarHeight
        color: Cpp_ThemeManager.colors["dashboard_background"]

        DragHandler {
          target: null
          onActiveChanged: {
            if (active)
              _extWindow.startSystemMove()
          }
        }
      }

      //
      // Titlebar text
      //
      Label {
        anchors {
          topMargin: 6
          top: parent.top
          horizontalCenter: parent.horizontalCenter
        }

        text: _extWindow.title
        visible: _extWindow.titlebarHeight > 0
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
      }

      //
      // Dashboard content
      //
      Page {
        anchors.fill: parent
        anchors.topMargin: _extWindow.titlebarHeight
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

        DbItems.DashboardLayout {
          anchors.fill: parent
          isExternalWindow: true
          hostWindow: _extWindow
          taskBar: _extWindow.extTaskBar
          onExternalWindowClicked: root.openExternalWindow()
          onWidgetWindowRequested: (windowId) => _mainLayout.openWidgetWindow(windowId)
        }
      }

      //
      // Persist whenever this window's workspace selection changes
      //
      Connections {
        target: _extWindow.extTaskBar
        function onActiveGroupIdChanged() { root.syncExternalWindows() }
      }

      //
      // Persist geometry/state once each settle/maximize/visibility change lands
      //
      Connections {
        target: _extWindow
        function onPreviousXChanged()      { root.syncExternalWindows() }
        function onPreviousYChanged()      { root.syncExternalWindows() }
        function onPreviousWidthChanged()  { root.syncExternalWindows() }
        function onPreviousHeightChanged() { root.syncExternalWindows() }
        function onIsMaximizedChanged()    { root.syncExternalWindows() }
        function onVisibilityChanged()     { root.syncExternalWindows() }
      }

      Component.onCompleted: {
        var rec = _extWindow.record || ({})
        _extWindow.windowId = (rec.id && rec.id.length > 0)
            ? rec.id
            : ("w" + Date.now() + "_" + _extWindow.windowNumber)

        if (rec.w !== undefined && rec.h !== undefined) {
          _extWindow.width = rec.w
          _extWindow.height = rec.h
          _extWindow.x = rec.x
          _extWindow.y = rec.y
          _extWindow.previousX = rec.x
          _extWindow.previousY = rec.y
          _extWindow.previousWidth = rec.w
          _extWindow.previousHeight = rec.h
          _extWindow.isMaximized = (rec.maximized === true)
        } else {
          var scr = Screen
          if (scr) {
            _extWindow.x = Math.round(scr.virtualX + (scr.width - width) / 2)
            _extWindow.y = Math.round(scr.virtualY + (scr.height - height) / 2)
          }
        }

        _extWindow.displayWindow()

        if (rec.fullscreen === true)
          _extWindow.showFullScreen()
        else if (rec.minimized === true)
          _extWindow.showMinimized()

        if (rec.workspace !== undefined && rec.workspace >= 0) {
          _extWindow.targetWorkspace = rec.workspace
          _extWindow.extTaskBar.setDesiredGroupId(rec.workspace)
          _wsRetry.restart()
        }
      }
    }
  }
}
