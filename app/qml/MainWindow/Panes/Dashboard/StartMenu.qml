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

  //
  // Required data inputs
  //
  required property SS_Ui.TaskBar taskBar

  //
  // Custom properties
  //
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
      width: root.gradientWidth
      border.width: _bg.border.width
      border.color: _bg.border.color

      anchors {
        top: _bg.top
        left: _bg.left
        bottom: _bg.bottom
      }

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

        rotation: -90
        font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
        text: Application.displayName + " " + Cpp_AppVersion
        color: Cpp_ThemeManager.colors["start_menu_version_text"]

        anchors {
          bottom: parent.bottom
          bottomMargin: implicitWidth / 2
          horizontalCenter: parent.horizontalCenter
        }
      }
    }
  }

  //
  // Create a column layout with navigation panels
  //
  ColumnLayout {
    id: _layout
    spacing: 4

    anchors {
      margins: 4
      fill: parent
      leftMargin: root.gradientWidth + 4
    }

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
                                        taskBar.activeGroupId = value
                                        root.close()
                                      })
        }

        // Update popup state
        _groups.popup.y = root.y
        _groups.popup.showCheckable = true
        _groups.popup.model = taskBar.groupModel
        _groups.popup.maximumHeight = root.height
        _groups.popup.x = root.x + root.width - 1
        _groups.popup.currentValue = taskBar.activeGroupId
        _groups.popup.placeholderText = qsTr("No Groups Available")

        // Open the popup
        _groups.popup.open()

        // Close actions menu
        if (_actions.popup)
          _actions.popup.close()
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

        // Close group menu
        if (_groups.popup)
          _groups.popup.close()
      }

      onClicked: _actions.showMenu()
      onContainsMouseChanged: {
        if (containsMouse)
          _actions.showMenu()
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
      text: qsTr("Console Logging")
      checked: Cpp_IO_ConsoleExport.exportEnabled
      icon.source: "qrc:/rcc/icons/start/console-log.svg"
      onClicked: Cpp_IO_ConsoleExport.exportEnabled = !Cpp_IO_ConsoleExport.exportEnabled
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
      text: qsTr("Console")
      Layout.fillWidth: true
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
      icon.source: "qrc:/rcc/icons/start/adjust.svg"
      onClicked: {
        root.close()
        app.showSettingsDialog()
      }
    }

    Widgets.MenuButton {
      expandable: false
      text: qsTr("Help")
      Layout.fillWidth: true
      icon.source: "qrc:/rcc/icons/start/help.svg"
      onClicked: {
        root.close()
        Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
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
      onClicked: Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused
      text: Cpp_IO_Manager.paused ? qsTr("Resume") :
                                    qsTr("Pause")
      icon.source: Cpp_IO_Manager.paused ?
                     "qrc:/rcc/icons/start/resume.svg" :
                     "qrc:/rcc/icons/start/pause.svg"
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
}

