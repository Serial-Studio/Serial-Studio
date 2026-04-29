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
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Popup {
  id: root

  modal: true
  focus: true
  width: _layout.implicitWidth + gradientWidth + 32
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  height: Math.max(gradientHeight, _layout.implicitHeight + 16)

  enter: Transition {
    NumberAnimation {
      duration: 200
      from: 0; to: 1
      property: "opacity"
      easing.type: Easing.OutCubic
    }
  }

  exit: Transition {
    NumberAnimation {
      duration: 120
      from: 1; to: 0
      property: "opacity"
      easing.type: Easing.InCubic
    }
  }

  //
  // Required data inputs
  //
  required property SS_Ui.TaskBar taskBar

  //
  // Custom properties
  //
  property bool isExternalWindow: false
  readonly property real gradientWidth: _versionLabel.implicitHeight + 16
  readonly property real gradientHeight: _versionLabel.implicitWidth + 32

  //
  // Save settings
  //
  Settings {
    property alias autoLayout: _autoLayoutBt.checked
    category: "WindowManagement" + app.settingsSuffix
  }

  //
  // Signals
  //
  signal externalWindowClicked()
  signal newWorkspaceRequested()
  signal renameWorkspaceRequested(int workspaceId, string currentName)

  //
  // Returns the list of currently-visible start menu actions whose name
  // matches `filter`. Each entry is `{ name, icon, run }` — `run` is the
  // closure that fires the action. Used by the taskbar's search popup.
  //
  function searchableItems(filter) {
    var runtimeMode = (typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true)
    var items = [
      {
        name: qsTr("Auto Layout"),
        icon: "qrc:/rcc/icons/start/auto-layout.svg",
        visible: true,
        run: function() {
          taskBar.windowManager.autoLayoutEnabled = !taskBar.windowManager.autoLayoutEnabled
        }
      },
      {
        name: qsTr("Full Screen"),
        icon: "qrc:/rcc/icons/start/full-screen.svg",
        visible: !root.isExternalWindow && !app.runtimeMode,
        run: function() { mainWindow.toggleFullScreen() }
      },
      {
        name: qsTr("Add External Window"),
        icon: "qrc:/rcc/icons/start/external-window.svg",
        visible: true,
        run: function() { root.externalWindowClicked() }
      },
      {
        name: qsTr("Console"),
        icon: "qrc:/rcc/icons/start/console.svg",
        visible: !app.runtimeMode,
        run: function() { Cpp_UI_Dashboard.terminalEnabled = !Cpp_UI_Dashboard.terminalEnabled }
      },
      {
        name: qsTr("Notifications"),
        icon: "qrc:/rcc/icons/start/notifications.svg",
        visible: Cpp_CommercialBuild,
        run: function() {
          Cpp_UI_Dashboard.notificationLogEnabled = !Cpp_UI_Dashboard.notificationLogEnabled
        }
      },
      {
        name: qsTr("Preferences"),
        icon: "qrc:/rcc/icons/start/settings.svg",
        visible: !app.runtimeMode,
        run: function() { app.showSettingsDialog() }
      },
      {
        name: qsTr("Help Center"),
        icon: "qrc:/rcc/icons/start/help.svg",
        visible: true,
        run: function() { app.showHelpCenter() }
      },
      {
        name: qsTr("MQTT"),
        icon: "qrc:/rcc/icons/toolbar/mqtt.svg",
        visible: Cpp_CommercialBuild && !app.runtimeMode,
        run: function() { app.showMqttConfiguration() }
      },
      {
        name: qsTr("Sessions"),
        icon: "qrc:/rcc/icons/start/sessions.svg",
        visible: Cpp_CommercialBuild
                 && (!app.runtimeMode || Cpp_Sessions_Export.exportEnabled),
        run: function() { app.showDatabaseExplorer() }
      }
    ]

    var f = (filter || "").trim().toLowerCase()
    var out = []
    for (var i = 0; i < items.length; ++i) {
      if (!items[i].visible)
        continue

      if (f.length > 0 && items[i].name.toLowerCase().indexOf(f) === -1)
        continue

      out.push(items[i])
    }

    return out
  }

  //
  // Custom components
  //
  Component {
    id: _subMenuComponent

    Widgets.SubMenuCombo {}
  }

  //
  // Custom overlay that does not dim everything else
  //
  Overlay.modal: Rectangle {
    color: "transparent"
  }

  //
  // Create the background of the control
  //
  background: Rectangle {
    id: _bg

    border.width: 1
    color: Cpp_ThemeManager.colors["start_menu_background"]
    border.color: Cpp_ThemeManager.colors["start_menu_border"]

    Rectangle {
      anchors {
        top: _bg.top
        left: _bg.left
        bottom: _bg.bottom
      }

      width: root.gradientWidth
      border.width: _bg.border.width
      border.color: _bg.border.color

      gradient: Gradient {
        GradientStop {
          position: 0
          color: Cpp_ThemeManager.colors["start_menu_gradient_top"]
        }

        GradientStop {
          position: 1
          color: Cpp_ThemeManager.colors["start_menu_gradient_bottom"]
        }
      }

      Label {
        id: _versionLabel

        anchors {
          bottom: parent.bottom
          bottomMargin: implicitWidth / 2
          horizontalCenter: parent.horizontalCenter
        }

        rotation: -90
        font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
        text: Application.displayName + " " + Cpp_AppVersion
        color: Cpp_ThemeManager.colors["start_menu_version_text"]
      }
    }
  }

  //
  // Create a column layout with navigation panels
  //
  ColumnLayout {
    id: _layout

    anchors {
      margins: 4
      fill: parent
      leftMargin: root.gradientWidth + 4
    }

    spacing: 4

    Widgets.MenuButton {
      id: _groups

      expandable: true
      Layout.fillWidth: true
      text: qsTr("Workspaces")
      icon.source: "qrc:/rcc/icons/start/groups.svg"

      property var popup: null
      function showMenu() {
        // Create the popup
        if (_groups.popup === null) {
          _groups.popup = _subMenuComponent.createObject(root)
          popup.valueSelected.connect((value) => {
            if (value === "__new_workspace__") {
              if (_groups.popup)
                _groups.popup.close()

              root.close()
              root.newWorkspaceRequested()
            } else {
              taskBar.activeGroupId = value
              root.close()
            }
          })

          popup.valueRightClicked.connect((value, text, gx, gy) => {
            // Suppress edit/hide/delete context menu in operator runtime mode
            var runtimeMode = (typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true)
            if (value >= 0 && !runtimeMode) {
              _wsContextId = value
              _wsContextName = text
              _wsContextMenu.popup()
            }
          })
        }

        // Build model: workspace model + separator + "New Workspace…"
        // (the "New Workspace…" entry is hidden in operator runtime mode so
        //  shortcut-launched dashboards stay locked to the project's layout)
        var items = taskBar.workspaceModel
        var model = []
        for (var i = 0; i < items.length; ++i)
          model.push(items[i])

        var runtimeMode = (typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true)
        if (!runtimeMode) {
          model.push({"id": "__separator__", "text": "",
                       "icon": "", "separator": true})
          model.push({"id": "__new_workspace__", "separator": false,
                       "text": qsTr("New Workspace…"),
                       "icon": "qrc:/rcc/icons/project-editor/toolbar/add-group.svg"})
        }

        // Update popup state
        _groups.popup.y = root.y
        _groups.popup.showCheckable = true
        _groups.popup.model = model
        _groups.popup.maximumHeight = root.height
        _groups.popup.x = root.x + root.width - 1
        _groups.popup.currentValue = taskBar.activeGroupId
        _groups.popup.placeholderText = qsTr("No Workspaces Available")

        // Open the popup
        _groups.popup.open()

        // Close other menus
        if (_actions.popup)
          _actions.popup.close()

        if (_plugins.popup)
          _plugins.popup.close()

        if (_export.popup)
          _export.popup.close()

        if (_tools.popup)
          _tools.popup.close()
      }

      onClicked: _groups.showMenu()
      onContainsMouseChanged: {
        if (containsMouse)
          _groups.showMenu()
      }
    }

    Widgets.MenuButton {
      id: _actions

      expandable: true
      text: qsTr("Actions")
      Layout.fillWidth: true
      visible: Cpp_UI_Dashboard.actions.length > 0
      icon.source: "qrc:/rcc/icons/start/actions.svg"

      property var popup: null
      function showMenu() {
        // Create the popup
        if (_actions.popup === null) {
          _actions.popup = _subMenuComponent.createObject(root)
          popup.valueSelected.connect((value) => {
                                        Cpp_UI_Dashboard.activateAction(value, true)
                                        root.close()
                                      })
        }

        // Update popup state
        _actions.popup.maximumHeight = root.height
        _actions.popup.model = Cpp_UI_Dashboard.actions
        _actions.popup.x = root.x + root.width - 1
        _actions.popup.y = _actions.y + _layout.y + root.y + 4
        _actions.popup.placeholderText = qsTr("No Actions Available")

        // Open the popup
        _actions.popup.open()

        // Close other menus
        if (_groups.popup)
          _groups.popup.close()

        if (_plugins.popup)
          _plugins.popup.close()

        if (_export.popup)
          _export.popup.close()

        if (_tools.popup)
          _tools.popup.close()
      }

      onClicked: _actions.showMenu()
      onContainsMouseChanged: {
        if (containsMouse)
          _actions.showMenu()
      }
    }

    Widgets.MenuButton {
      id: _plugins

      expandable: true
      text: qsTr("Plugins")
      Layout.fillWidth: true
      icon.source: "qrc:/rcc/icons/toolbar/extensions.svg"
      visible: Cpp_ExtensionManager.installedPlugins.length > 0

      property var popup: null
      function showMenu() {
        if (_plugins.popup === null) {
          _plugins.popup = _subMenuComponent.createObject(root)
          _plugins.popup.textRole = "title"
          _plugins.popup.valueRole = "id"
          _plugins.popup.iconRole = "icon"
          popup.valueSelected.connect((value) => {
                                        if (value === "__manage_plugins__") {
                                          Cpp_ExtensionManager.setFilterType("plugin")
                                          app.showExtensionManager()
                                        } else {
                                          Cpp_ExtensionManager.launchPlugin(value)
                                        }

                                        root.close()
                                      })
        }

        // Build model: plugins + separator + "Manage Plugins…"
        var items = []
        var plugins = Cpp_ExtensionManager.installedPlugins
        for (var i = 0; i < plugins.length; ++i)
          items.push(plugins[i])

        if (plugins.length > 0)
          items.push({"id": "__separator__", "title": "", "icon": ""})

        items.push({
                     "id": "__manage_plugins__",
                     "title": qsTr("Manage Plugins…"),
                     "icon": "qrc:/rcc/icons/toolbar/extensions.svg"
                   })

        _plugins.popup.model = items
        _plugins.popup.maximumHeight = root.height
        _plugins.popup.x = root.x + root.width - 1
        _plugins.popup.y = _plugins.y + _layout.y + root.y + 4
        _plugins.popup.placeholderText = qsTr("No Plugins Installed")
        _plugins.popup.open()

        if (_groups.popup)
          _groups.popup.close()

        if (_actions.popup)
          _actions.popup.close()

        if (_export.popup)
          _export.popup.close()

        if (_tools.popup)
          _tools.popup.close()
      }

      onClicked: _plugins.showMenu()
      onContainsMouseChanged: {
        if (containsMouse)
          _plugins.showMenu()
      }
    }

    Rectangle {
      opacity: 0.5
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["start_menu_text"]
    }

    Widgets.MenuButton {
      id: _autoLayoutBt

      checkable: true
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Auto Layout")
      checked: taskBar.windowManager.autoLayoutEnabled
      icon.source: "qrc:/rcc/icons/start/auto-layout.svg"
      onCheckedChanged: {
        if (checked !== taskBar.windowManager.autoLayoutEnabled)
          taskBar.windowManager.autoLayoutEnabled = checked
      }
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Full Screen")
      icon.source: "qrc:/rcc/icons/start/full-screen.svg"
      visible: !root.isExternalWindow && !app.runtimeMode
      checked: mainWindow.visibility === Window.FullScreen
      onClicked: {
        root.close()
        mainWindow.toggleFullScreen()
      }
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Add External Window")
      icon.source: "qrc:/rcc/icons/start/external-window.svg"
      onClicked: {
        root.close()
        root.externalWindowClicked()
      }
    }

    Rectangle {
      opacity: 0.5
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["start_menu_text"]
      visible: !(typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true)
    }

    Widgets.MenuButton {
      id: _export

      expandable: true
      text: qsTr("Export")
      Layout.fillWidth: true
      icon.source: "qrc:/rcc/icons/start/export.svg"
      visible: !(typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true)

      readonly property string kCsv: "csv"
      readonly property string kMdf4: "mdf4"
      readonly property string kConsole: "console"
      readonly property string kDatabase: "database"

      property var popup: null
      function showMenu() {
        // Create the popup on first use
        if (_export.popup === null) {
          _export.popup = _subMenuComponent.createObject(root)
          _export.popup.valueSelected.connect((value) => {
            if (value === _export.kCsv)
              Cpp_CSV_Export.exportEnabled = !Cpp_CSV_Export.exportEnabled
            else if (value === _export.kMdf4)
              Cpp_MDF4_Export.exportEnabled = !Cpp_MDF4_Export.exportEnabled
            else if (value === _export.kConsole)
              Cpp_Console_Export.exportEnabled = !Cpp_Console_Export.exportEnabled
            else if (value === _export.kDatabase && Cpp_CommercialBuild)
              Cpp_Sessions_Export.exportEnabled = !Cpp_Sessions_Export.exportEnabled
          })
        }

        // Build model with per-row enabled state
        var model = [
          {
            "id": _export.kCsv,
            "text": qsTr("CSV File"),
            "icon": "qrc:/rcc/icons/start/csv-log.svg",
            "checked": Cpp_CSV_Export.exportEnabled
          },
          {
            "id": _export.kMdf4,
            "text": qsTr("MDF4 File"),
            "icon": "qrc:/rcc/icons/start/mf4-log.svg",
            "checked": Cpp_MDF4_Export.exportEnabled
          },
          {
            "id": _export.kConsole,
            "text": qsTr("Console Transcript"),
            "icon": "qrc:/rcc/icons/start/console-log.svg",
            "checked": Cpp_Console_Export.exportEnabled
          }
        ]

        if (Cpp_CommercialBuild) {
          model.push({
            "id": _export.kDatabase,
            "text": qsTr("Session Database"),
            "icon": "qrc:/rcc/icons/start/database-export.svg",
            "checked": Cpp_Sessions_Export.exportEnabled
          })
        }

        // Update popup state
        _export.popup.model = model
        _export.popup.showCheckable = true
        _export.popup.maximumHeight = root.height
        _export.popup.x = root.x + root.width - 1
        _export.popup.y = _export.y + _layout.y + root.y + 4
        _export.popup.placeholderText = qsTr("No Export Formats Available")

        // Open the popup
        _export.popup.open()

        // Close other menus
        if (_groups.popup)
          _groups.popup.close()

        if (_actions.popup)
          _actions.popup.close()

        if (_plugins.popup)
          _plugins.popup.close()

        if (_tools.popup)
          _tools.popup.close()
      }

      onClicked: _export.showMenu()
      onContainsMouseChanged: {
        if (containsMouse)
          _export.showMenu()
      }
    }

    Widgets.MenuButton {
      id: _tools

      expandable: true
      text: qsTr("Tools")
      Layout.fillWidth: true
      icon.source: "qrc:/rcc/icons/start/tools.svg"
      visible: !app.runtimeMode || Cpp_CommercialBuild

      readonly property string kMqtt: "mqtt"
      readonly property string kConsole: "console"
      readonly property string kSessions: "sessions"
      readonly property string kPreferences: "preferences"
      readonly property string kNotifications: "notifications"

      property var popup: null
      function showMenu() {
        // Create the popup on first use
        if (_tools.popup === null) {
          _tools.popup = _subMenuComponent.createObject(root)
          _tools.popup.valueSelected.connect((value) => {
            if (value === _tools.kConsole) {
              root.close()
              Cpp_UI_Dashboard.terminalEnabled = !Cpp_UI_Dashboard.terminalEnabled
            } else if (value === _tools.kNotifications && Cpp_CommercialBuild) {
              root.close()
              Cpp_UI_Dashboard.notificationLogEnabled = !Cpp_UI_Dashboard.notificationLogEnabled
            } else if (value === _tools.kPreferences) {
              root.close()
              app.showSettingsDialog()
            } else if (value === _tools.kMqtt && Cpp_CommercialBuild) {
              root.close()
              app.showMqttConfiguration()
            } else if (value === _tools.kSessions && Cpp_CommercialBuild) {
              root.close()
              app.showDatabaseExplorer()
            }
          })
        }

        // Build model conditionally — only items applicable to the current
        // build/runtime mode are shown so the submenu doesn't dead-stub.
        var model = []

        if (!app.runtimeMode) {
          model.push({
            "id": _tools.kConsole,
            "text": qsTr("Console"),
            "icon": "qrc:/rcc/icons/start/console.svg",
            "checked": Cpp_UI_Dashboard.terminalEnabled
          })
        }

        if (Cpp_CommercialBuild) {
          model.push({
            "id": _tools.kNotifications,
            "text": qsTr("Notifications"),
            "icon": "qrc:/rcc/icons/start/notifications.svg",
            "checked": Cpp_UI_Dashboard.notificationLogEnabled
          })
        }

        if (!app.runtimeMode) {
          model.push({
            "id": _tools.kPreferences,
            "text": qsTr("Preferences"),
            "icon": "qrc:/rcc/icons/start/settings.svg"
          })
        }

        if (Cpp_CommercialBuild && !app.runtimeMode) {
          model.push({
            "id": _tools.kMqtt,
            "text": qsTr("MQTT"),
            "icon": Cpp_MQTT_Client.isConnected
                    ? (Cpp_MQTT_Client.isSubscriber
                       ? "qrc:/rcc/icons/toolbar/mqtt-subscriber.svg"
                       : "qrc:/rcc/icons/toolbar/mqtt-publisher.svg")
                    : "qrc:/rcc/icons/toolbar/mqtt.svg"
          })
        }

        if (Cpp_CommercialBuild
            && (!app.runtimeMode || Cpp_Sessions_Export.exportEnabled)) {
          model.push({
            "id": _tools.kSessions,
            "text": qsTr("Sessions"),
            "icon": "qrc:/rcc/icons/start/sessions.svg"
          })
        }

        // Update popup state
        _tools.popup.model = model
        _tools.popup.showCheckable = true
        _tools.popup.maximumHeight = root.height
        _tools.popup.x = root.x + root.width - 1
        _tools.popup.y = _tools.y + _layout.y + root.y + 4
        _tools.popup.placeholderText = qsTr("No Tools Available")

        // Open the popup
        _tools.popup.open()

        // Close other menus
        if (_groups.popup)
          _groups.popup.close()

        if (_actions.popup)
          _actions.popup.close()

        if (_plugins.popup)
          _plugins.popup.close()

        if (_export.popup)
          _export.popup.close()
      }

      onClicked: _tools.showMenu()
      onContainsMouseChanged: {
        if (containsMouse)
          _tools.showMenu()
      }
    }

    Rectangle {
      opacity: 0.5
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["start_menu_text"]
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Help Center")
      icon.source: "qrc:/rcc/icons/start/help.svg"
      onClicked: {
        root.close()
        app.showHelpCenter()
      }
    }

    Rectangle {
      opacity: 0.5
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["start_menu_text"]
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      icon.source: Cpp_IO_Manager.paused ?
                     "qrc:/rcc/icons/start/resume.svg" :
                     "qrc:/rcc/icons/start/pause.svg"
      text: Cpp_IO_Manager.paused ? qsTr("Resume") :
                                    qsTr("Pause")
      onClicked: Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused
    }

    Widgets.MenuButton {
      expandable: false
      text: qsTr("Reset")
      Layout.fillWidth: true
      icon.source: "qrc:/rcc/icons/start/reset.svg"
      onClicked: {
        // Reset dashboard
        root.close()
        Cpp_UI_Dashboard.clearPlotData()

        // Rotate any active recorders so the next frame opens a fresh file/session
        Cpp_CSV_Export.closeFile()
        Cpp_Console_Export.closeFile()
        if (Cpp_CommercialBuild) {
          Cpp_MDF4_Export.closeFile()
          Cpp_Sessions_Export.closeFile()
        }
      }
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Disconnect")
      icon.source: "qrc:/rcc/icons/start/disconnect.svg"
      onClicked: {
        root.close()
        if (typeof mainWindow !== "undefined" && mainWindow.userDisconnect)
          mainWindow.userDisconnect()
        else
          Cpp_IO_Manager.disconnectDevice()
      }
    }
  }

  //
  // Workspace context menu state
  //
  property int _wsContextId: -1
  property string _wsContextName: ""

  //
  // Right-click context menu for workspaces
  //
  Menu {
    id: _wsContextMenu

    MenuItem {
      text: qsTr("Edit…")
      visible: root._wsContextId >= 1000
      height: visible ? implicitHeight : 0
      onTriggered: {
        root.close()
        root.renameWorkspaceRequested(root._wsContextId,
                                      root._wsContextName)
      }
    }

    MenuItem {
      text: root._wsContextId >= 1000 ? qsTr("Delete")
                                      : qsTr("Hide")
      onTriggered: {
        taskBar.deleteWorkspace(root._wsContextId)
        if (_groups.popup)
          _groups.popup.close()
      }
    }
  }

}
