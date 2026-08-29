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
import QtQuick.Layouts
import QtQuick.Controls

import "Settings" as Pages
import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  title: qsTr("Preferences")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Window controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent

    //
    // Tab bar
    //
    TabBar {
      id: _tab

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        text: qsTr("General")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Startup")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Plotting")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Layout")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Taskbar")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Console")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Export")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        height: _tab.height + 3
        text: qsTr("Notifications")
        visible: Cpp_CommercialBuild
        width: visible ? implicitWidth + 2 * 8 : 0
      }
    }

    //
    // Tab contents
    //
    StackLayout {
      id: stack

      clip: true
      Layout.fillWidth: true
      Layout.minimumWidth: 480
      currentIndex: _tab.currentIndex
      Layout.topMargin: -parent.spacing - 1
      implicitHeight: Math.max(
                        generalTab.implicitHeight,
                        startupTab.implicitHeight,
                        plottingTab.implicitHeight,
                        layoutTab.implicitHeight,
                        taskbarTab.implicitHeight,
                        consoleTab.implicitHeight,
                        exportTab.implicitHeight,
                        Cpp_CommercialBuild ? notificationsTab.implicitHeight : 0
                        )

      Pages.SettingsGeneralPage {
        id: generalTab
      }

      Pages.SettingsStartupPage {
        id: startupTab
      }

      Pages.SettingsPlottingPage {
        id: plottingTab
      }

      Pages.SettingsLayoutPage {
        id: layoutTab
      }

      Pages.SettingsTaskbarPage {
        id: taskbarTab
      }

      Pages.SettingsConsolePage {
        id: consoleTab
      }

      Pages.SettingsExportPage {
        id: exportTab
      }

      Pages.SettingsNotificationsPage {
        id: notificationsTab
      }
    }

    //
    // Buttons
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.IconButton {
        text: qsTr("Reset")
        horizontalPadding: 8
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/refresh.svg"
        onClicked: {
          Cpp_ThemeManager.theme = 0
          if (Cpp_NativeWindow.csdAvailable)
            Cpp_NativeWindow.csdEnabled = true

          Cpp_API_Server.enabled = false
          Cpp_API_Server.externalConnections = false
          if (Cpp_Misc_GraphicsBackend.configurable)
            Cpp_Misc_GraphicsBackend.currentBackend = 0

          if (Cpp_Misc_HighDpiScaling.configurable) {
            Cpp_Misc_HighDpiScaling.currentMode = 1
            Cpp_Misc_HighDpiScaling.customPercent = 100
          }
          Cpp_Misc_ModuleManager.automaticUpdates = true
          Cpp_Misc_ModuleManager.performanceMode = true
          Cpp_Misc_ModuleManager.inhibitIdleSleep = true
          Cpp_UI_Dashboard.plotTimeRange = 10
          Cpp_UI_Dashboard.points = 1000
          Cpp_Misc_TimerEvents.fps = 60
          Cpp_UI_Dashboard.autoHideToolbar = false
          Cpp_UI_Dashboard.showActionPanel = true
          Cpp_UI_Dashboard.showAlignmentGuides = false
          Cpp_UI_Dashboard.layoutMargin = 0
          Cpp_UI_Dashboard.layoutSpacing = -1
          Cpp_UI_TaskbarSettings.resetToDefaults()
          Cpp_Console_Handler.fontFamily = Cpp_Misc_CommonFonts.monoFont.family
          Cpp_Console_Handler.fontSize = Cpp_Misc_CommonFonts.monoFont.pointSize
          Cpp_Console_Handler.scrollbackLines = 1000
          Cpp_Console_Handler.echo = true
          Cpp_Console_Handler.showTimestamp = false
          Cpp_Console_Handler.vt100Emulation = true
          Cpp_Console_Handler.ansiColors = true
          Cpp_Console_Handler.dataMode = 0
          Cpp_Console_Handler.displayMode = 0
          Cpp_Console_Handler.lineEnding = 0
          Cpp_Console_Handler.encoding = 0
          Cpp_Console_Handler.checksumMethod = 0
          Cpp_Misc_CommonFonts.widgetFontScale = 1.0
          Cpp_Misc_CommonFonts.widgetFontFamily = Cpp_Misc_CommonFonts.monoFont.family
          Cpp_Notifications.systemNotificationsEnabled = false
          Cpp_Notifications.routeWarningsToNotifications = false
          if (Cpp_CommercialBuild)
            Cpp_Image_Export.exportEnabled = false
        }
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("Close")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        text: qsTr("Apply")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/apply.svg"
      }
    }
  }
}
