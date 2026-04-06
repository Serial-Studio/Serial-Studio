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
  height: Math.max(gradientHeight, _layout.implicitHeight + 16)
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

  enter: Transition {
    NumberAnimation {
      property: "opacity"
      from: 0; to: 1
      duration: 200
      easing.type: Easing.OutCubic
    }
  }

  exit: Transition {
    NumberAnimation {
      property: "opacity"
      from: 1; to: 0
      duration: 120
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
    category: "WindowManagement"
    property alias autoLayout: _autoLayoutBt.checked
    property alias consoleEnabled: _consoleBt.checked
  }

  //
  // Signals
  //
  signal externalWindowClicked()
  signal newWorkspaceRequested()
  signal renameWorkspaceRequested(int workspaceId, string currentName)

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
            if (value >= 1000) {
              _wsContextId = value
              _wsContextName = text
              _wsContextMenu.popup()
            }
          })
        }

        // Build model: workspace model + separator + "New Workspace..."
        var items = taskBar.workspaceModel
        var model = []
        for (var i = 0; i < items.length; ++i)
          model.push(items[i])

        model.push({"id": "__separator__", "text": "",
                     "icon": "", "separator": true})
        model.push({"id": "__new_workspace__", "separator": false,
                     "text": qsTr("New Workspace..."),
                     "icon": "qrc:/rcc/icons/project-editor/toolbar/add-group.svg"})

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
      visible: Cpp_ExtensionManager.installedPlugins.length > 0
      icon.source: "qrc:/rcc/icons/toolbar/extensions.svg"

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

        // Build model: plugins + separator + "Manage Plugins..."
        var items = []
        var plugins = Cpp_ExtensionManager.installedPlugins
        for (var i = 0; i < plugins.length; ++i)
          items.push(plugins[i])

        if (plugins.length > 0)
          items.push({"id": "__separator__", "title": "", "icon": ""})
        items.push({
                     "id": "__manage_plugins__",
                     "title": qsTr("Manage Plugins..."),
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
      visible: !root.isExternalWindow
      checked: !mainWindow.toolbarVisible
      icon.source: "qrc:/rcc/icons/start/full-screen.svg"
      onClicked: mainWindow.toolbarVisible = !mainWindow.toolbarVisible
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
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("CSV Logging")
      checked: Cpp_CSV_Export.exportEnabled
      icon.source: "qrc:/rcc/icons/start/csv-log.svg"
      onClicked: Cpp_CSV_Export.exportEnabled = !Cpp_CSV_Export.exportEnabled
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("MDF4 Logging")
      checked: Cpp_MDF4_Export.exportEnabled
      icon.source: "qrc:/rcc/icons/start/mf4-log.svg"
      onClicked: Cpp_MDF4_Export.exportEnabled = !Cpp_MDF4_Export.exportEnabled
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Console Logging")
      checked: Cpp_Console_Export.exportEnabled
      icon.source: "qrc:/rcc/icons/start/console-log.svg"
      onClicked: Cpp_Console_Export.exportEnabled = !Cpp_Console_Export.exportEnabled
    }

    Rectangle {
      opacity: 0.5
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["start_menu_text"]
    }

    Widgets.MenuButton {
      id: _consoleBt

      checkable: true
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Console")
      checked: Cpp_UI_Dashboard.terminalEnabled
      icon.source: "qrc:/rcc/icons/start/console.svg"
      onCheckedChanged: {
        if (checked !== Cpp_UI_Dashboard.terminalEnabled) {
          root.close()
          Cpp_UI_Dashboard.terminalEnabled = checked
        }
      }
    }

    Widgets.MenuButton {
      expandable: false
      Layout.fillWidth: true
      text: qsTr("Preferences")
      icon.source: "qrc:/rcc/icons/start/settings.svg"
      onClicked: {
        root.close()
        app.showSettingsDialog()
      }
    }

    Widgets.MenuButton {
      expandable: false
      text: qsTr("Help Center")
      Layout.fillWidth: true
      icon.source: "qrc:/rcc/icons/start/help.svg"
      onClicked: {
        root.close()
        app.showHelpCenter()
      }
    }

    Loader {
      Layout.fillWidth: true
      active: Cpp_CommercialBuild
      sourceComponent: Rectangle {
        opacity: 0.5
        implicitHeight: 1
        color: Cpp_ThemeManager.colors["start_menu_text"]
      }
    }

    Loader {
      Layout.fillWidth: true
      active: Cpp_CommercialBuild
      sourceComponent: Component {
        Widgets.MenuButton {
          expandable: false
          text: qsTr("MQTT")
          icon.source: Cpp_MQTT_Client.isConnected ?
                         (Cpp_MQTT_Client.isSubscriber ?
                            "qrc:/rcc/icons/toolbar/mqtt-subscriber.svg" :
                            "qrc:/rcc/icons/toolbar/mqtt-publisher.svg") :
                         "qrc:/rcc/icons/toolbar/mqtt.svg"
          onClicked: {
            root.close()
            app.showMqttConfiguration()
          }
        }
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
      Layout.fillWidth: true
      text: qsTr("Disconnect")
      icon.source: "qrc:/rcc/icons/start/disconnect.svg"
      onClicked: {
        root.close()
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
  // Right-click context menu for user workspaces
  //
  Menu {
    id: _wsContextMenu

    MenuItem {
      text: qsTr("Rename...")
      onTriggered: {
        root.close()
        root.renameWorkspaceRequested(root._wsContextId,
                                      root._wsContextName)
      }
    }

    MenuItem {
      text: qsTr("Delete")
      onTriggered: {
        taskBar.deleteWorkspace(root._wsContextId)
        if (_groups.popup)
          _groups.popup.close()
      }
    }
  }

}

