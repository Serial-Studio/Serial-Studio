/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

import SerialStudio

QtObject {
  id: root

  //
  // Host references supplied by the Dashboard pane; entries stay hidden or
  // disabled via their own guards until each ref is assigned.
  //
  property var taskBar: null
  property var hostWindow: null
  property bool allowFullScreen: true
  property bool allowExternalWindow: true

  //
  // Window-owner actions raised by items that need a host to fulfil them.
  //
  signal fullScreenRequested()
  signal externalWindowRequested()

  //
  // Command id -> behavior entry, consumed by CommandModel.join().
  //
  readonly property var map: ({
    "dashboard.autoLayout": root.cmdDashboardAutoLayout,
    "app.fullScreen": root.cmdAppFullScreen,
    "window.external": root.cmdWindowExternal,
    "dashboard.console": root.cmdDashboardConsole,
    "dashboard.notifications": root.cmdDashboardNotifications,
    "dashboard.clock": root.cmdDashboardClock,
    "dashboard.stopwatch": root.cmdDashboardStopwatch,
    "app.preferences": root.cmdAppPreferences,
    "app.helpCenter": root.cmdAppHelpCenter,
    "app.sessions": root.cmdAppSessions,
    "app.fileTransmission": root.cmdAppFileTransmission,
    "app.ai": root.cmdAppAi,
    "dashboard.freeze": root.cmdDashboardFreeze,
    "dashboard.reset": root.cmdDashboardReset,
    "io.pause": root.cmdIoPause,
    "io.disconnect": root.cmdIoDisconnect,
    "app.quit": root.cmdAppQuit,
    "export.csv": root.cmdExportCsv,
    "export.mdf4": root.cmdExportMdf4,
    "export.console": root.cmdExportConsole,
    "export.database": root.cmdExportDatabase,
    "console.echo": root.cmdConsoleEcho,
    "console.timestamp": root.cmdConsoleTimestamp,
    "console.clear": root.cmdConsoleClear,
    "console.hex": root.cmdConsoleHex,
    "console.collapseDuplicates": root.cmdConsoleCollapse
  })

  //
  // Start-menu / taskbar tool actions (ToolActions.qml + StartMenu.qml).
  //
  readonly property QtObject cmdDashboardAutoLayout: QtObject {
    readonly property bool enabled: !Cpp_UI_Dashboard.frozen
    readonly property bool visible: root.taskBar !== null
        && !(app.runtimeMode && Cpp_UI_Dashboard.frozen)
    readonly property bool checked: root.taskBar && root.taskBar.windowManager
        ? root.taskBar.windowManager.autoLayoutEnabled : false
    function run() {
      if (root.taskBar && root.taskBar.windowManager)
        root.taskBar.windowManager.autoLayoutEnabled = !root.taskBar.windowManager.autoLayoutEnabled
    }
  }

  readonly property QtObject cmdAppFullScreen: QtObject {
    readonly property bool visible: root.allowFullScreen
    readonly property bool checked: root.hostWindow
        ? root.hostWindow.visibility === Window.FullScreen : false
    function run() { root.fullScreenRequested() }
  }

  readonly property QtObject cmdWindowExternal: QtObject {
    readonly property bool visible: root.allowExternalWindow
    function run() { root.externalWindowRequested() }
  }

  readonly property QtObject cmdDashboardConsole: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool checked: Cpp_UI_Dashboard.terminalEnabled
    function run() { Cpp_UI_Dashboard.terminalEnabled = !Cpp_UI_Dashboard.terminalEnabled }
  }

  readonly property QtObject cmdDashboardNotifications: QtObject {
    readonly property bool visible: Cpp_CommercialBuild
    readonly property bool checked: Cpp_UI_Dashboard.notificationLogEnabled
    function run() {
      Cpp_UI_Dashboard.notificationLogEnabled = !Cpp_UI_Dashboard.notificationLogEnabled
    }
  }

  readonly property QtObject cmdDashboardClock: QtObject {
    readonly property bool checked: Cpp_UI_Dashboard.clockEnabled
    function run() { Cpp_UI_Dashboard.clockEnabled = !Cpp_UI_Dashboard.clockEnabled }
  }

  readonly property QtObject cmdDashboardStopwatch: QtObject {
    readonly property bool checked: Cpp_UI_Dashboard.stopwatchEnabled
    function run() { Cpp_UI_Dashboard.stopwatchEnabled = !Cpp_UI_Dashboard.stopwatchEnabled }
  }

  readonly property QtObject cmdAppPreferences: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool enabled: !app.runtimeMode
    function run() { app.showSettingsDialog() }
  }

  readonly property QtObject cmdAppHelpCenter: QtObject {
    function run() { app.showHelpCenter() }
  }

  readonly property QtObject cmdAppSessions: QtObject {
    readonly property bool visible: Cpp_CommercialBuild
        && (!app.runtimeMode || Cpp_Sessions_Export.exportEnabled)
    function run() { app.showDatabaseExplorer() }
  }

  readonly property QtObject cmdAppFileTransmission: QtObject {
    readonly property bool visible: Cpp_CommercialBuild
        && (!app.runtimeMode || Cpp_IO_FileTransmission.runtimeAccessAllowed)
    function run() { app.showFileTransmission() }
  }

  readonly property QtObject cmdAppAi: QtObject {
    readonly property bool visible: Cpp_CommercialBuild && !app.runtimeMode
    function run() { app.showAIAssistant() }
  }

  //
  // Freeze/reset/pause/disconnect (StartMenu.qml); freeze re-derives
  // freezeAllowed inline since only run() needs it here.
  //
  readonly property QtObject cmdDashboardFreeze: QtObject {
    readonly property bool checked: Cpp_UI_Dashboard.frozen
    readonly property bool visible: Cpp_AppState.operationMode === SerialStudio.ProjectFile
        && !app.runtimeMode
    readonly property bool enabled: Cpp_AppState.operationMode === SerialStudio.ProjectFile
        && !app.runtimeMode
    function run() {
      var allowed = Cpp_CommercialBuild
          && (Cpp_Licensing_LemonSqueezy.isActivated || Cpp_Licensing_Trial.trialEnabled)
      if (allowed)
        Cpp_UI_Dashboard.setFrozen(!Cpp_UI_Dashboard.frozen)
      else
        app.showLicenseDialog()
    }
  }

  readonly property QtObject cmdDashboardReset: QtObject {
    function run() {
      Cpp_UI_Dashboard.clearPlotData()
      Cpp_CSV_Export.closeFile()
      Cpp_Console_Export.closeFile()
      if (Cpp_CommercialBuild)
        Cpp_MDF4_Export.closeFile()

      if (Cpp_CommercialBuild)
        Cpp_Sessions_Export.closeFile()
    }
  }

  readonly property QtObject cmdIoPause: QtObject {
    readonly property bool checked: Cpp_IO_Manager.paused
    function run() { Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused }
  }

  readonly property QtObject cmdIoDisconnect: QtObject {
    readonly property bool visible: !app.runtimeMode
    function run() {
      if (typeof mainWindow !== "undefined" && mainWindow.userDisconnect)
        mainWindow.userDisconnect()
      else
        Cpp_IO_Manager.disconnectDevice()
    }
  }

  //
  // Quit is offered only in operator mode; the editor host has its own exit paths.
  //
  readonly property QtObject cmdAppQuit: QtObject {
    readonly property bool visible: app.runtimeMode
    function run() { app.quitApplication() }
  }

  //
  // Export toggles (StartMenu.qml Export sub-menu).
  //
  readonly property QtObject cmdExportCsv: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool checked: Cpp_CSV_Export.exportEnabled
    function run() { Cpp_CSV_Export.exportEnabled = !Cpp_CSV_Export.exportEnabled }
  }

  readonly property QtObject cmdExportMdf4: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool checked: Cpp_MDF4_Export.exportEnabled
    function run() { Cpp_MDF4_Export.exportEnabled = !Cpp_MDF4_Export.exportEnabled }
  }

  readonly property QtObject cmdExportConsole: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool checked: Cpp_Console_Export.exportEnabled
    function run() { Cpp_Console_Export.exportEnabled = !Cpp_Console_Export.exportEnabled }
  }

  readonly property QtObject cmdExportDatabase: QtObject {
    readonly property bool visible: Cpp_CommercialBuild && !app.runtimeMode
    readonly property bool checked: Cpp_CommercialBuild ? Cpp_Sessions_Export.exportEnabled : false
    function run() {
      if (Cpp_CommercialBuild)
        Cpp_Sessions_Export.exportEnabled = !Cpp_Sessions_Export.exportEnabled
    }
  }

  //
  // Console display toggles (GPL; the console pane is always available).
  //
  readonly property QtObject cmdConsoleEcho: QtObject {
    readonly property bool checked: Cpp_Console_Handler.echo
    function run() { Cpp_Console_Handler.echo = !Cpp_Console_Handler.echo }
  }

  readonly property QtObject cmdConsoleTimestamp: QtObject {
    readonly property bool checked: Cpp_Console_Handler.showTimestamp
    function run() { Cpp_Console_Handler.showTimestamp = !Cpp_Console_Handler.showTimestamp }
  }

  readonly property QtObject cmdConsoleClear: QtObject {
    function run() { Cpp_Console_Handler.clear() }
  }

  readonly property QtObject cmdConsoleHex: QtObject {
    readonly property bool checked: Cpp_Console_Handler.displayMode === 1
    function run() { Cpp_Console_Handler.displayMode = (Cpp_Console_Handler.displayMode === 1 ? 0 : 1) }
  }

  readonly property QtObject cmdConsoleCollapse: QtObject {
    readonly property bool checked: Cpp_Console_Handler.collapseDuplicates
    function run() {
      Cpp_Console_Handler.collapseDuplicates = !Cpp_Console_Handler.collapseDuplicates
    }
  }
}
