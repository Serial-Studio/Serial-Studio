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
    _mainLayout.loadLayout()
  }

  //
  // Taskbar helper
  //
  property SS_UI.TaskBar taskBar: SS_UI.TaskBar {}

  //
  // Main dashboard layout
  //
  DbItems.DashboardLayout {
    id: _mainLayout
    taskBar: root.taskBar
    anchors.fill: parent
    onExternalWindowClicked: root.openExternalWindow()
  }

  //
  // Track open external windows and provide a counter for unique categories
  //
  property int _extWindowCounter: 0
  property var _extWindows: []

  function openExternalWindow() {
    var num = ++_extWindowCounter
    var win = _extDashboardComponent.createObject(root, {
      "category": "ExternalDashboard_" + num,
      "windowNumber": num
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
      // Independent taskbar for this external window
      //
      property SS_UI.TaskBar extTaskBar: SS_UI.TaskBar {}

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
          if (_extWindow.visible) {
            Cpp_NativeWindow.removeWindow(_extWindow)
            Cpp_NativeWindow.addWindow(
              _extWindow,
              Cpp_ThemeManager.colors["dashboard_background"])
          }
        }
      }

      //
      // Titlebar background
      //
      Rectangle {
        height: _extWindow.titlebarHeight
        color: Cpp_ThemeManager.colors["dashboard_background"]
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
        }

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
        text: _extWindow.title
        visible: _extWindow.titlebarHeight > 0
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

        anchors {
          topMargin: 6
          top: parent.top
          horizontalCenter: parent.horizontalCenter
        }
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
          taskBar: _extWindow.extTaskBar
          onExternalWindowClicked: root.openExternalWindow()
        }
      }

      Component.onCompleted: _extWindow.displayWindow()
    }
  }
}
