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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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
  headerVisible: !root.frozen
  implicitWidth: minimumWidth
  implicitHeight: minimumHeight
  focused: taskBar.activeWindow === root
  windowControlsVisible: !Cpp_UI_TaskbarSettings.taskbarHidden
  shadowEnabled: !root.frozen && root.state === "normal"
                 && (!windowManager.autoLayoutEnabled
                     || Cpp_UI_Dashboard.autoLayoutSpacing > -1)

  //
  // Effective freeze state: WidgetToolbar instances read this via windowRoot
  //
  readonly property bool frozen: Cpp_UI_Dashboard.frozen

  //
  // Per-widget freeze-title mode ("bar"/"painted"/"hidden"): the project model resolves
  // the per-type default; the QML fallback only covers Quick Plot mode (no project)
  //
  property int entityUniqueId: -1
  property int entityWidgetType: -1
  property string freezeTitleMode: ""
  readonly property bool paintsOwnTitle: entityWidgetType >= 0
                                         && SerialStudio.dashboardWidgetPaintsTitle(entityWidgetType)
  readonly property string effectiveFreezeTitle: freezeTitleMode !== ""
                                                 ? freezeTitleMode
                                                 : (paintsOwnTitle ? "painted" : "bar")
  readonly property bool frozenHeaderVisible: root.frozen
                                              && root.effectiveFreezeTitle === "bar"
                                              && root.title.length > 0
  readonly property real frozenHeaderHeight: frozenHeaderVisible
      ? Math.max(32, frozenHeaderMetrics.height + 14) : 0

  function refreshFreezeTitleMode() {
    if (entityWidgetType >= 0 && entityUniqueId >= 0)
      freezeTitleMode = Cpp_JSON_ProjectModel.freezeTitleMode(entityWidgetType, entityUniqueId)
  }

  onEntityUniqueIdChanged: refreshFreezeTitleMode()

  Connections {
    target: Cpp_JSON_ProjectModel

    function onWidgetDisplayChanged() {
      root.refreshFreezeTitleMode()
    }
  }

  //
  // Extension identity: set by the embedded DashboardWidget, empty for a compiled-in widget
  //
  property string extensionId: ""
  property string extensionWidgetId: ""
  readonly property var extensionConfig: root.extensionId.length > 0
                                         ? Cpp_UI_WidgetExtensions.configProperties(root.extensionId)
                                         : []

  //
  // Generic settings form, rendered from the package's declarations only when asked for
  //
  function showExtensionSettings() {
    if (extensionSettingsLoader.item) {
      extensionSettingsLoader.item.openDialog(root.extensionId,
                                              root.extensionWidgetId, root.title)
      return
    }

    extensionSettingsLoader.active = true
  }

  Loader {
    id: extensionSettingsLoader

    active: false
    source: "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/ExtensionWidgetSettings.qml"
    onLoaded: item.openDialog(root.extensionId, root.extensionWidgetId, root.title)
  }

  //
  // Caption menu (menu button, left of the title): rename, freeze-title mode, pop-out
  //
  readonly property bool displayEditable: root.entityUniqueId >= 0
                                          && Cpp_AppState.operationMode === SerialStudio.ProjectFile
                                          && !app.runtimeMode

  onMenuClicked: {
    taskBar.activeWindow = root
    widgetMenu.popup(4, root.captionHeight)
  }

  Menu {
    id: widgetMenu

    Component.onCompleted: {
      if (app.runtimeMode) {
        widgetMenu.removeMenu(freezeTitleMenu)
        widgetMenu.removeItem(renameItem)
        widgetMenu.removeItem(captionSeparator)
      }
    }

    MenuItem {
      id: renameItem

      icon.width: 16
      icon.height: 16
      text: qsTr("Rename Widget…")
      enabled: root.displayEditable
      icon.source: "qrc:/icons/buttons/rename.svg"
      onTriggered: Cpp_JSON_ProjectModel.promptRenameWidget(root.entityWidgetType,
                                                            root.entityUniqueId, root.title)
    }

    Menu {
      id: freezeTitleMenu

      icon.width: 16
      icon.height: 16
      title: qsTr("Freeze Title")
      icon.source: "qrc:/icons/buttons/freeze.svg"

      MenuItem {
        checkable: true
        text: qsTr("Title Bar")
        enabled: root.displayEditable
        checked: root.effectiveFreezeTitle === "bar"
        onTriggered: Cpp_JSON_ProjectModel.setFreezeTitleMode(root.entityWidgetType,
                                                              root.entityUniqueId, "bar")
      }

      MenuItem {
        checkable: true
        text: qsTr("Painted Title")
        visible: root.paintsOwnTitle
        enabled: root.displayEditable
        height: visible ? implicitHeight : 0
        checked: root.effectiveFreezeTitle === "painted"
        onTriggered: Cpp_JSON_ProjectModel.setFreezeTitleMode(root.entityWidgetType,
                                                              root.entityUniqueId, "painted")
      }

      MenuItem {
        checkable: true
        text: qsTr("Hidden")
        enabled: root.displayEditable
        checked: root.effectiveFreezeTitle === "hidden"
        onTriggered: Cpp_JSON_ProjectModel.setFreezeTitleMode(root.entityWidgetType,
                                                              root.entityUniqueId, "hidden")
      }
    }

    MenuItem {
      id: extensionSettingsItem

      icon.width: 16
      icon.height: 16
      text: qsTr("Widget Settings…")
      height: visible ? implicitHeight : 0
      visible: root.extensionConfig.length > 0
      onTriggered: root.showExtensionSettings()
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "settings", 16)
    }

    MenuSeparator {
      id: captionSeparator
    }

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Open in External Window")
      icon.source: "qrc:/icons/buttons/expand.svg"
      onTriggered: root.externalWindowClicked()
    }
  }

  TextMetrics {
    id: frozenHeaderMetrics

    text: "0"
    font: Cpp_Misc_CommonFonts.customUiFont(1, true)
  }
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
  // Emitted by the caption menu's pop-out entry; handled by DashboardCanvas
  //
  signal externalWindowClicked()

  //
  // Set minimum size: manual mode allows dense instrument tiles down to 48x48,
  // auto-layout keeps the packing-friendly floor
  //
  readonly property int minimumWidth: windowManager.autoLayoutEnabled ? 356 : 48
  readonly property int minimumHeight: windowManager.autoLayoutEnabled ? 320 : 48

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
          windowRoot.title       = Qt.binding(function() { return dashboardWidget.widgetTitle })
          windowRoot.deviceIndex = widgetSourceId
          if (windowRoot.entityUniqueId !== undefined) {
            windowRoot.entityWidgetType = widgetType
            windowRoot.entityUniqueId   = widgetUniqueId
          }
          if (windowRoot.extensionId !== undefined) {
            windowRoot.extensionId       = dashboardWidget.widgetExtensionId
            windowRoot.extensionWidgetId = dashboardWidget.widgetId
          }
          if (windowRoot.icon !== undefined)
            windowRoot.icon = loader.widgetIcon()
        }
      }

      //
      // Mirrors the widget's toolbar state onto the window chrome
      //
      function bindToolbar() {
        if (widgetInstance.hasToolbar === undefined)
          return

        windowRoot.hasToolbar = widgetInstance.hasToolbar
        if (widgetInstance.hasToolbarChanged !== undefined) {
          widgetInstance.hasToolbarChanged.connect(function () {
            windowRoot.hasToolbar = widgetInstance.hasToolbar
          })
        }
      }

      //
      // Caption artwork: a package's declared icon, falling back to the built-in table
      //
      function widgetIcon() {
        if (dashboardWidget.widgetIsExtension) {
          const art = Cpp_UI_WidgetExtensions.iconUrl(dashboardWidget.widgetExtensionId, 16)
          if (art.length > 0)
            return art
        }

        return SerialStudio.dashboardWidgetIcon(dashboardWidget.widgetType)
      }

      //
      // A widget that fails to load shows an explained tile, never an empty slot
      //
      function showPlaceholder(reason) {
        const path = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/ExtensionPlaceholder.qml"
        const placeholder = Qt.createComponent(path)
        if (placeholder.status !== Component.Ready)
          return

        widgetInstance = placeholder.createObject(loader, {
                                                    reason: reason,
                                                    title: dashboardWidget.widgetTitle,
                                                    extensionId: dashboardWidget.widgetExtensionId
                                                  })
        if (widgetInstance)
          widgetInstance.anchors.fill = loader
      }

      //
      // A package allowed to run (or updated) after this slot was built rebuilds in place
      //
      Connections {
        target: Cpp_UI_WidgetExtensions

        function onCatalogChanged() {
          if (!dashboardWidget.widgetIsExtension)
            return

          if (widgetInstance) {
            widgetInstance.destroy()
            widgetInstance = null
          }

          dashboardWidget.reloadWidget()
          loader.buildWidget()
        }
      }

      Component.onCompleted: loader.buildWidget()

      function buildWidget() {
        if (dashboardWidget.widgetIsExtension) {
          widgetInstance = dashboardWidget.createExtensionItem(loader, {
                                                                model: dashboardWidget.widgetModel,
                                                                windowRoot: loader.windowRoot,
                                                                color: dashboardWidget.widgetColor,
                                                                widgetId: dashboardWidget.widgetId
                                                              })
          if (!widgetInstance) {
            showPlaceholder(dashboardWidget.widgetExtensionError)
            return
          }

          bindToolbar()
          widgetInstance.anchors.fill = loader
          return
        }

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
            showPlaceholder(qsTr("The widget could not be created."))
            return
          }

          bindToolbar()
          widgetInstance.anchors.fill = loader
        }

        else if (component.status === Component.Error)
          showPlaceholder(component.errorString())
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
    radius: root.radius
    anchors.fill: parent
    border.width: 1
    color: Cpp_ThemeManager.colors["widget_window"]
    border.color: Cpp_ThemeManager.colors["window_border"]
    anchors.topMargin: Math.max(0, root.captionHeight + (root.hasToolbar ? 48 : 0) - 1)

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
  // Frozen-mode panel header: gradient title bar in place of the hidden caption
  //
  Rectangle {
    id: frozenHeader

    visible: root.frozenHeaderVisible
    height: visible ? root.frozenHeaderHeight : 0
    anchors {
      margins: 1
      top: parent.top
      left: parent.left
      right: parent.right
    }

    gradient: Gradient {
      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["table_bg_header_top"]
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["table_bg_header_bottom"]
      }
    }

    Rectangle {
      height: 1
      color: Cpp_ThemeManager.colors["table_border_header"]
      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
    }

    Label {
      text: root.title
      elide: Label.ElideRight
      anchors.centerIn: parent
      verticalAlignment: Label.AlignVCenter
      horizontalAlignment: Label.AlignHCenter
      color: Cpp_ThemeManager.colors["table_fg_header"]
      width: Math.min(implicitWidth, parent.width - 16)
      font: Cpp_Misc_CommonFonts.customUiFont(1, true)
    }
  }

  //
  // Per-source connection state (suppressed during replay)
  //
  property bool sourceDisconnected: false
  function _refreshSourceConnection() {
    sourceDisconnected = !SerialStudio.isAnyPlayerOpen()
                         && !Cpp_Benchmark_Runner.running
                         && !Cpp_API_Mirror.attached
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
    target: Cpp_API_Mirror
    function onAttachedChanged() { root._refreshSourceConnection() }
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
    anchors.topMargin: Math.max(1, root.captionHeight + root.frozenHeaderHeight
                                + (root.frozenHeaderVisible ? 1 : 0))
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
          source: Cpp_Misc_IconRegistry.icon("notifications", "warning", 24)
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
