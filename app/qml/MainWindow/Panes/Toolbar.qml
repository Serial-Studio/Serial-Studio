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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets
import "../../Commands" as Commands

Rectangle {
  id: root

  //
  // Custom signals
  //
  signal projectEditorClicked()
  property var mwPalette: null
  property var dashboard: null
  property bool autoHide: false
  property bool toolbarEnabled: true
  property bool dashboardVisible: false
  readonly property bool dashboardMode: !showContent && dashboardVisible
  readonly property bool showContent: toolbarEnabled && !(autoHide && dashboardVisible)

  //
  // Calculate offset based on platform
  //
  property int titlebarHeight: Cpp_NativeWindow.titlebarHeight(mainWindow)
  Connections {
    target: mainWindow
    function onVisibilityChanged() {
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(mainWindow)
    }
  }

  //
  // Animated toolbar content height
  //
  property int toolbarContentHeight: showContent ? 64 + 16 : 0
  Behavior on toolbarContentHeight {
    NumberAnimation {
      duration: 250
      easing.type: Easing.OutCubic
    }
  }

  Layout.minimumHeight: titlebarHeight + toolbarContentHeight
  Layout.maximumHeight: titlebarHeight + toolbarContentHeight

  //
  // Command registry plumbing
  //
  Commands.AppCommandBindings {
    id: _tbAppBindings

    mwPalette: root.mwPalette
    dashboard: root.dashboard
    dashboardVisible: root.dashboardVisible
  } Commands.DashboardCommandBindings {
    id: _tbDashBindings
  } Commands.CommandModel {
    id: _tbModel

    context: "app"
    bindingSets: [_tbAppBindings, _tbDashBindings]
  }

  //
  // Top toolbar section
  //
  Rectangle {
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    Behavior on color {
      ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
    }

    height: root.titlebarHeight
    color: (app.runtimeMode || root.dashboardMode)
           ? Cpp_ThemeManager.colors["dashboard_background"]
           : Cpp_ThemeManager.colors["toolbar_top"]
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

    Behavior on color {
      ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
    }

    text: mainWindow.title
    visible: root.titlebarHeight > 0
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    color: (app.runtimeMode || root.dashboardMode)
           ? Cpp_ThemeManager.colors["text"]
           : Cpp_ThemeManager.colors["titlebar_text"]
  }

  //
  // Toolbar background
  //
  Rectangle {
    anchors.fill: parent
    gradient: Gradient {
      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["toolbar_top"]

        Behavior on color {
          ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
        }
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["toolbar_bottom"]

        Behavior on color {
          ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
        }
      }
    }

    anchors.topMargin: root.titlebarHeight
  }

  //
  // Toolbar border
  //
  Rectangle {
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }

    height: 1
    visible: !app.runtimeMode && opacity > 0
    opacity: root.showContent ? 1 : 0
    color: Cpp_ThemeManager.colors["toolbar_border"]

    Behavior on opacity {
      NumberAnimation {
        duration: 250
        easing.type: Easing.OutCubic
      }
    }
  }

  //
  // Drag main window with the toolbar
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        mainWindow.startSystemMove()
    }
  }

  //
  // Command toolbar (registry-driven ribbon)
  //
  Widgets.CommandToolbar {
    id: _commandToolbar

    visible: opacity > 0
    enabled: root.showContent
    opacity: root.showContent ? 1 : 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }

    anchors {
      rightMargin: 4
      left: parent.left
      right: connectSection.left
      verticalCenter: parent.verticalCenter
      verticalCenterOffset: root.titlebarHeight / 2
    }

    height: 64 + 16

    model: _tbModel
    surface: "main-toolbar"
  }

  //
  // Right-pinned section (Connect + Activate, always visible)
  //
  RowLayout {
    id: connectSection

    spacing: 4
    visible: opacity > 0
    opacity: root.showContent ? 1 : 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }

    anchors {
      rightMargin: 4
      right: parent.right
      verticalCenter: parent.verticalCenter
      verticalCenterOffset: root.titlebarHeight / 2
    }

    //
    // License activation (Pro only)
    //
    Widgets.ToolbarButton {
      readonly property var entry: _tbModel.binding("license.activate")

      text: qsTr("Activate")
      Layout.alignment: Qt.AlignVCenter
      visible: Cpp_CommercialBuild && Cpp_Licensing_Trial.trialExpired
               && !Cpp_Licensing_LemonSqueezy.isActivated
      ToolTip.text: qsTr("Manage license and activate Serial Studio Pro")
      icon.source: Cpp_Misc_IconRegistry.iconById("commands/activate", 32)
      onClicked: entry.run()
    }

    //
    // Remote dashboard button (shown instead of Connect while a mirror view is attached)
    //
    Widgets.ToolbarButton {
      readonly property var entry: _tbModel.binding("app.remoteAttach")

      onClicked: entry.run()
      text: qsTr("Remote Dashboard")
      visible: Cpp_API_Mirror.attached
      Layout.alignment: Qt.AlignVCenter
      ToolTip.text: qsTr("Manage the remote dashboard connection")
      icon.source: Cpp_Misc_IconRegistry.iconById("commands/remote-attach", 32)
    }

    //
    // Connect/Disconnect button
    //
    Widgets.ToolbarButton {
      id: _connectButton

      readonly property bool connecting: Cpp_IO_Manager.isConnecting
      readonly property var entry: _tbModel.binding("io.toggleConnection")

      Layout.alignment: Qt.AlignVCenter
      implicitWidth: metrics.width + 16
      font: Cpp_Misc_CommonFonts.boldUiFont
      Layout.minimumWidth: metrics.width + 16
      Layout.maximumWidth: metrics.width + 16
      checked: entry !== null && entry.checked === true
      ToolTip.text: qsTr("Connect or disconnect from the configured device")
      text: connecting ? qsTr("Connecting…") : (checked ? qsTr("Disconnect") : qsTr("Connect"))
      icon.source: Cpp_Misc_IconRegistry.iconById(checked ? "commands/connect" : "commands/disconnect", 32)

      enabled: entry !== null && entry.enabled !== false
      visible: !Cpp_API_Mirror.attached && entry !== null && entry.visible !== false

      onClicked: entry.run()

      TextMetrics {
        id: metrics

        text: " " + qsTr("Disconnect") + " "
        font: Cpp_Misc_CommonFonts.boldUiFont
      }
    }

    Item {
      implicitWidth: 1
    }
  }
}
