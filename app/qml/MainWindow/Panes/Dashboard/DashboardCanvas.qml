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
  // True for canvases hosted in an external dashboard window
  //
  property bool isExternalWindow: false

  //
  // Set by the layout: when a dashboard is fullscreen, pop-out widgets/tools stay on top
  //
  property bool widgetsStayOnTop: false

  //
  // Raised by external-window canvases to forward a pop-out request to the main canvas
  //
  signal externalWidgetWindowRequested(int windowId)

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
    if (root.isExternalWindow) {
      root.externalWidgetWindowRequested(windowId)
      return
    }

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

    win.stayOnTop = Qt.binding(function() { return root.widgetsStayOnTop })
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
    if (root.isExternalWindow)
      return

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
    if (root.isExternalWindow)
      return

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
    if (root.isExternalWindow)
      return

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

    MenuSeparator {

    }

    MenuItem {
      checkable: true
      text: qsTr("Show Grid")
      checked: _wm.gridEnabled
      enabled: !_wm.autoLayoutEnabled
      onTriggered: _wm.gridEnabled = checked
    }

    Menu {
      title: qsTr("Grid Size")
      enabled: !_wm.autoLayoutEnabled

      MenuItem {
        checkable: true
        text: qsTr("%1 px").arg(8)
        checked: _wm.gridSize === 8
        onTriggered: _wm.gridSize = 8
      }

      MenuItem {
        checkable: true
        text: qsTr("%1 px").arg(16)
        checked: _wm.gridSize === 16
        onTriggered: _wm.gridSize = 16
      }

      MenuItem {
        checkable: true
        text: qsTr("%1 px").arg(24)
        checked: _wm.gridSize === 24
        onTriggered: _wm.gridSize = 24
      }

      MenuItem {
        checkable: true
        text: qsTr("%1 px").arg(32)
        checked: _wm.gridSize === 32
        onTriggered: _wm.gridSize = 32
      }

      MenuItem {
        checkable: true
        text: qsTr("%1 px").arg(64)
        checked: _wm.gridSize === 64
        onTriggered: _wm.gridSize = 64
      }
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
  // Workspace-switch slide: the rebuilt canvas slides in vertically from the travel side (up when
  // advancing to a later workspace, down when returning) and fades up.
  //
  property int slideDirection: 0

  Connections {
    target: taskBar

    function onAboutToChangeWorkspace(fromIndex, toIndex) {
      root.slideDirection = (fromIndex < 0 || toIndex < 0 || fromIndex === toIndex)
                            ? 0 : (toIndex > fromIndex ? 1 : -1)
    }

    function onActiveGroupIdChanged() {
      if (root.slideDirection === 0)
        return

      _slideTransform.y = root.slideDirection * windowCanvas.height
      root.slideDirection = 0
      _slideAnimation.restart()
    }
  }

  ParallelAnimation {
    id: _slideAnimation

    NumberAnimation {
      to: 0
      duration: 220
      property: "y"
      target: _slideTransform
      easing.type: Easing.OutCubic
    }

    NumberAnimation {
      to: 1
      from: 0.35
      duration: 220
      property: "opacity"
      target: windowCanvas
      easing.type: Easing.OutCubic
    }
  }

  //
  // Window canvas
  //
  Item {
    id: windowCanvas

    transform: Translate {
      id: _slideTransform
    }

    anchors {
      bottomMargin: -1
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      topMargin: commercialNotification.visible ? -1 : 0
      top: commercialNotification.visible ? commercialNotification.bottom : parent.top
    }

    //
    // Manual-mode reference grid, painted under the windows and above the
    // wallpaper; repaints only on size, grid-size, or theme changes
    //
    Canvas {
      id: _gridCanvas

      anchors.fill: _wm
      visible: _wm.gridEnabled && !_wm.autoLayoutEnabled && !_wm.frozen
      onWidthChanged: requestPaint()
      onHeightChanged: requestPaint()
      onVisibleChanged: if (visible) requestPaint()

      Connections {
        target: _wm

        function onGridSizeChanged() {
          _gridCanvas.requestPaint()
        }
      }

      Connections {
        target: Cpp_ThemeManager

        function onThemeChanged() {
          _gridCanvas.requestPaint()
        }
      }

      onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = Cpp_ThemeManager.colors["canvas_grid"]
        ctx.lineWidth = 1
        ctx.beginPath()

        const cell = Math.max(2, _wm.gridSize)
        for (let gx = cell; gx < width; gx += cell) {
          ctx.moveTo(gx + 0.5, 0)
          ctx.lineTo(gx + 0.5, height)
        }

        for (let gy = cell; gy < height; gy += cell) {
          ctx.moveTo(0, gy + 0.5)
          ctx.lineTo(width, gy + 0.5)
        }

        ctx.stroke()
      }
    }

    //
    // Set window manager
    //
    SS_Ui.WindowManager {
      id: _wm

      anchors.fill: parent
      frozen: Cpp_UI_Dashboard.frozen
      anchors.margins: Cpp_UI_Dashboard.layoutMargin
      onRightClicked: (x, y) => contextMenu.popup(x, y)

      //
      // Passive hover monitor so manual-mode resize cursors show over stacked windows
      //
      HoverHandler {
        id: resizeHover

        enabled: !_wm.autoLayoutEnabled && !_wm.frozen
        property point hoverPosition: resizeHover.point.position
        onHoveredChanged: if (!resizeHover.hovered) _wm.updateHoverCursor(Qt.point(-1, -1))
        onHoverPositionChanged: if (resizeHover.hovered) _wm.updateHoverCursor(hoverPosition)
      }

      //
      // Freeze-mode focus follow: focus the widget under the cursor, since
      // click-to-focus is suppressed while the layout is frozen
      //
      HoverHandler {
        id: freezeFocusHover

        enabled: _wm.frozen && !app.commandPaletteOpen
        property point hoverPosition: freezeFocusHover.point.position
        onHoverPositionChanged: if (freezeFocusHover.hovered)
                                  _wm.focusWindowUnderCursor(hoverPosition)
      }

      //
      // Re-tile when the layout spacing preference changes; margin changes re-lay-out on
      // their own through the canvas inset resize
      //
      Connections {
        target: Cpp_UI_Dashboard
        function onLayoutSpacingChanged() { if (_wm.autoLayoutEnabled) _wm.loadLayout() }
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
    // Snap indicator: true snap geometry, border inset 1px per touched edge
    //
    Item {
      id: _snapIndicator

      z: _wm.zCounter + 9999
      x: _wm.x + _wm.snapIndicator.x
      y: _wm.y + _wm.snapIndicator.y
      width: _wm.snapIndicator.width
      height: _wm.snapIndicator.height
      visible: _wm.snapIndicatorVisible

      readonly property bool touchesTop: _wm.snapIndicator.y <= 0
      readonly property bool touchesLeft: _wm.snapIndicator.x <= 0
      readonly property bool touchesRight: _wm.snapIndicator.x + width >= _wm.width

      Rectangle {
        border.width: 1
        anchors.fill: parent
        anchors.topMargin: _snapIndicator.touchesTop ? 1 : 0
        anchors.leftMargin: _snapIndicator.touchesLeft ? 1 : 0
        anchors.rightMargin: _snapIndicator.touchesRight ? 1 : 0
        color: Cpp_ThemeManager.colors["snap_indicator_background"]
        border.color: Cpp_ThemeManager.colors["snap_indicator_border"]
      }
    }

    //
    // Manual-mode gesture overlay: smart-guide lines, equal-spacing gaps, the
    // size-matched sibling highlight, and the live geometry badge
    //
    Item {
      id: _gestureOverlay

      anchors.fill: _wm
      z: _wm.zCounter + 9999
      visible: _wm.manualGestureActive && !_wm.autoLayoutEnabled && !_wm.frozen

      //
      // Single opacity knob for the smart-guide visuals; keeps them subtle.
      //
      readonly property real guideOpacity: 0.8

      Repeater {
        model: _gestureOverlay.visible ? _wm.alignmentGuides : []

        delegate: Rectangle {
          required property var modelData

          x: modelData.x
          y: modelData.y
          width: Math.max(1, modelData.width)
          height: Math.max(1, modelData.height)
          opacity: _gestureOverlay.guideOpacity
          color: Cpp_ThemeManager.colors["highlight"]
        }
      }

      Repeater {
        model: _gestureOverlay.visible ? _wm.spacingIndicators : []

        delegate: Item {
          id: _spacingDelegate

          required property var modelData

          x: modelData.x
          y: modelData.y
          width: Math.max(1, modelData.width)
          height: Math.max(1, modelData.height)
          opacity: _gestureOverlay.guideOpacity

          Rectangle {
            opacity: 0.2
            anchors.fill: parent
            color: Cpp_ThemeManager.colors["highlight"]
          }

          Label {
            anchors.centerIn: parent
            text: _spacingDelegate.modelData.gap
            color: Cpp_ThemeManager.colors["highlight"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
            visible: implicitWidth < _spacingDelegate.width - 2
                     || implicitWidth < _spacingDelegate.height - 2
          }
        }
      }

      Rectangle {
        border.width: 2
        color: "transparent"
        x: _wm.sizeMatchRect.x
        y: _wm.sizeMatchRect.y
        visible: _wm.sizeMatchVisible
        width: _wm.sizeMatchRect.width
        height: _wm.sizeMatchRect.height
        opacity: _gestureOverlay.guideOpacity
        border.color: Cpp_ThemeManager.colors["highlight"]
      }

      //
      // Wrench-fraction resize preview: the footprint the window lands on, labelled with
      // its width and height as canvas fractions
      //
      Rectangle {
        id: _fractionPreview

        radius: 2
        border.width: 1
        x: _wm.fractionPreviewRect.x
        y: _wm.fractionPreviewRect.y
        width: _wm.fractionPreviewRect.width
        height: _wm.fractionPreviewRect.height
        visible: _wm.fractionPreviewLabel.length > 0
        opacity: _gestureOverlay.guideOpacity
        border.color: Cpp_ThemeManager.colors["highlight"]
        color: Qt.alpha(Cpp_ThemeManager.colors["highlight"], 0.14)

        //
        // Centered on the footprint, but allowed to overflow it (a 1/16 tile is far
        // narrower than the readout) and clamped so it never leaves the canvas
        //
        Rectangle {
          radius: 3
          color: Cpp_ThemeManager.colors["highlight"]
          width: _fractionLabel.implicitWidth + 12
          height: _fractionLabel.implicitHeight + 6
          x: Math.max(-parent.x,
                      Math.min((parent.width - width) / 2,
                               _gestureOverlay.width - parent.x - width))
          y: Math.max(-parent.y,
                      Math.min((parent.height - height) / 2,
                               _gestureOverlay.height - parent.y - height))

          Label {
            id: _fractionLabel

            anchors.centerIn: parent
            text: _wm.fractionPreviewLabel
            color: Cpp_ThemeManager.colors["highlighted_text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
          }
        }
      }
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
          source: Cpp_Misc_IconRegistry.icon("panes", "dashboard", 48)
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
