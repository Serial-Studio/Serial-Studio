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
    canvas.windowManager.loadLayout()
  }

  //
  // Taskbar helper
  //
  property SS_UI.TaskBar taskBar: SS_UI.TaskBar {}


  //
  // Default background
  //
  Rectangle {
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["dashboard_background"]
  }

  //
  // User-selected background image
  //
  Image {
    visible: source !== ""
    anchors.fill: parent
    source: canvas.backgroundImage
    fillMode: Image.PreserveAspectCrop
  }

  //
  // Desktop layout
  //
  ColumnLayout {
    spacing: -1
    implicitHeight: 0
    anchors.fill: parent
    anchors.topMargin: 2

    //
    // Actions rectangle
    //
    Rectangle {
      z: 1000
      border.width: 1
      Layout.topMargin: -1
      Layout.leftMargin: -1
      Layout.rightMargin: -1
      Layout.fillWidth: true
      implicitHeight: _actions.implicitHeight + 20
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
      visible: Cpp_UI_Dashboard.actionCount > 0 && Cpp_UI_Dashboard.showActionPanel

      ListView {
        id: _actions

        spacing: 2
        interactive: true
        implicitHeight: 32
        model: Cpp_UI_Dashboard.actions
        orientation: ListView.Horizontal

        anchors {
          leftMargin: 8
          left: parent.left
          right: parent.right
          verticalCenter: parent.verticalCenter
        }

        delegate: Widgets.ToolbarButton {
          required property var model

          iconSize: 24
          implicitHeight: 32
          maxButtonWidth: 256
          text: model["text"]
          toolbarButton: false
          horizontalLayout: true
          checked: model["checked"]
          icon.source: model["icon"]
          enabled: !Cpp_IO_Manager.paused && Cpp_IO_Manager.isConnected
          onClicked: Cpp_UI_Dashboard.activateAction(model["id"], true)
        }
      }
    }

    //
    // Widget windows
    //
    DbItems.DashboardCanvas {
      id: canvas
      taskBar: root.taskBar
      Layout.topMargin: -1
      Layout.leftMargin: -1
      Layout.rightMargin: -1
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 480
    }

    //
    // Task bar
    //
    DbItems.Taskbar {
      id: _taskBar
      taskBar: root.taskBar
      Layout.fillWidth: true
      onStartClicked: {
        if (startMenu.visible)
          startMenu.close()
        else
          startMenu.open()
      }
    }
  }

  //
  // Start menu
  //
  DbItems.StartMenu {
    id: startMenu
    taskBar: root.taskBar
    y: root.height - height - _taskBar.height + 2
    onExternalWindowClicked: root.openExternalWindow()
  }

  //
  // Track open external windows and provide a counter for unique categories
  //
  property int _extWindowCounter: 0
  property var _extWindows: []

  function openExternalWindow() {
    var win = _extDashboardComponent.createObject(root, {
      "category": "ExternalDashboard_" + (++_extWindowCounter)
    })

    if (win) {
      _extWindows.push(win)
      win.closing.connect(function() {
        var idx = _extWindows.indexOf(win)
        if (idx !== -1)
          _extWindows.splice(idx, 1)

        win.destroy()
      })
    }
  }

  //
  // Close all external windows when dashboard data resets
  //
  Connections {
    target: Cpp_UI_Dashboard
    function onDataReset() {
      for (var i = _extWindows.length - 1; i >= 0; --i)
        _extWindows[i].close()

      _extWindows = []
    }
  }

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
      title: Application.displayName + " — " + qsTr("Dashboard")

      //
      // Independent taskbar for this external window
      //
      property SS_UI.TaskBar extTaskBar: SS_UI.TaskBar {}

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

        ColumnLayout {
          spacing: -1
          anchors.fill: parent

          //
          // Actions panel
          //
          Rectangle {
            z: 1000
            border.width: 1
            Layout.topMargin: -1
            Layout.leftMargin: -1
            Layout.rightMargin: -1
            Layout.fillWidth: true
            implicitHeight: _extActions.implicitHeight + 20
            color: Cpp_ThemeManager.colors["groupbox_background"]
            border.color: Cpp_ThemeManager.colors["groupbox_border"]
            visible: Cpp_UI_Dashboard.actionCount > 0
                     && Cpp_UI_Dashboard.showActionPanel

            ListView {
              id: _extActions

              spacing: 2
              interactive: true
              implicitHeight: 32
              model: Cpp_UI_Dashboard.actions
              orientation: ListView.Horizontal

              anchors {
                leftMargin: 8
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
              }

              delegate: Widgets.ToolbarButton {
                required property var model

                iconSize: 24
                implicitHeight: 32
                maxButtonWidth: 256
                text: model["text"]
                toolbarButton: false
                horizontalLayout: true
                checked: model["checked"]
                icon.source: model["icon"]
                enabled: !Cpp_IO_Manager.paused
                         && Cpp_IO_Manager.isConnected
                onClicked: Cpp_UI_Dashboard.activateAction(
                             model["id"], true)
              }
            }
          }

          //
          // Widget canvas
          //
          DbItems.DashboardCanvas {
            id: _extCanvas
            taskBar: _extWindow.extTaskBar
            Layout.topMargin: -1
            Layout.leftMargin: -1
            Layout.rightMargin: -1
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 480
          }

          //
          // Taskbar
          //
          DbItems.Taskbar {
            id: _extTaskbar
            taskBar: _extWindow.extTaskBar
            Layout.fillWidth: true
            onStartClicked: {
              if (_extStartMenu.visible)
                _extStartMenu.close()
              else
                _extStartMenu.open()
            }
          }
        }

        //
        // Start menu (external window variant)
        //
        DbItems.StartMenu {
          id: _extStartMenu
          isExternalWindow: true
          taskBar: _extWindow.extTaskBar
          y: parent.height - height - _extTaskbar.height + 2
          onExternalWindowClicked: root.openExternalWindow()
        }
      }

      Component.onCompleted: _extWindow.displayWindow()
    }
  }
}
