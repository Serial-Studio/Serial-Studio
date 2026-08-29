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

import SerialStudio

QtObject {
  id: root

  //
  // Host references supplied by MainWindow; entries stay hidden or disabled
  // via their own guards until each ref is assigned.
  //
  property var dashboard: null
  property var mwPalette: null
  property bool dashboardVisible: false

  //
  // Shared with the Toolbar driver buttons: only one bus may bind while a
  // multi-source ProjectFile is loaded.
  //
  readonly property bool driverSelectionEnabled: app.ioEnabled
      && (Cpp_AppState.operationMode !== SerialStudio.ProjectFile
          || Cpp_JSON_ProjectModel.sourceCount <= 1)

  //
  // Command id -> behavior entry, consumed by CommandModel.join().
  //
  readonly property var map: ({
    "app.projectEditor": root.cmdAppProjectEditor,
    "project.open": root.cmdProjectOpen,
    "csv.open": root.cmdCsvOpen,
    "mdf4.open": root.cmdMdf4Open,
    "app.deploy": root.cmdAppDeploy,
    "app.extensions": root.cmdAppExtensions,
    "app.examples": root.cmdAppExamples,
    "app.about": root.cmdAppAbout,
    "app.problems": root.cmdAppProblems,
    "app.connectionDiagnostics": root.cmdAppConnectionDiagnostics,
    "app.remoteAttach": root.cmdAppRemoteAttach,
    "app.macros": root.cmdAppMacros,
    "app.deepwiki": root.cmdAppDeepwiki,
    "app.quit": root.cmdAppQuit,
    "license.activate": root.cmdLicenseActivate,
    "mode.quickPlot": root.cmdModeQuickPlot,
    "mode.consoleOnly": root.cmdModeConsoleOnly,
    "mode.projectFile": root.cmdModeProjectFile,
    "io.toggleConnection": root.cmdIoToggleConnection,
    "palette.open": root.cmdPaletteOpen,
    "dashboard.startMenu": root.cmdDashboardStartMenu,
    "dashboard.focusSearch": root.cmdDashboardFocusSearch,
    "workspace.next": root.cmdWorkspaceNext,
    "workspace.previous": root.cmdWorkspacePrevious,
    "window.closeActive": root.cmdWindowCloseActive,
    "window.minimizeActive": root.cmdWindowMinimizeActive,
    "window.clearActive": root.cmdWindowClearActive,
    "dashboard.autoLayout": root.cmdDashboardAutoLayout,
    "dashboard.freeze": root.cmdDashboardFreeze,
    "driver.uart": root.cmdDriverUart,
    "driver.audio": root.cmdDriverAudio,
    "driver.usb": root.cmdDriverUsb,
    "driver.network": root.cmdDriverNetwork,
    "driver.modbus": root.cmdDriverModbus,
    "driver.hid": root.cmdDriverHid,
    "driver.bluetooth": root.cmdDriverBluetooth,
    "driver.canbus": root.cmdDriverCanbus,
    "driver.process": root.cmdDriverProcess,
    "driver.opcua": root.cmdDriverOpcUa,
    "driver.s7": root.cmdDriverS7,
    "driver.ethernetip": root.cmdDriverEthernetIp,
    "driver.iec104": root.cmdDriverIec104
  })

  //
  // Toolbar / ribbon actions (MainWindowActions.qml).
  //
  readonly property QtObject cmdAppProjectEditor: QtObject {
    readonly property bool visible: !app.runtimeMode
    function run() { app.showProjectEditor() }
  }

  readonly property QtObject cmdProjectOpen: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool enabled: !Cpp_IO_Manager.isConnected
    function run() {
      Cpp_AppState.operationMode = SerialStudio.ProjectFile
      Cpp_JSON_ProjectModel.openJsonFile()
    }
  }

  readonly property QtObject cmdCsvOpen: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool enabled: !app.runtimeMode && !Cpp_CSV_Player.isOpen
        && !Cpp_IO_Manager.isConnected
    function run() { Cpp_CSV_Player.openFile() }
  }

  readonly property QtObject cmdMdf4Open: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool enabled: !app.runtimeMode && !Cpp_MDF4_Player.isOpen
        && !Cpp_IO_Manager.isConnected
    function run() { Cpp_MDF4_Player.openFile() }
  }

  readonly property QtObject cmdAppDeploy: QtObject {
    readonly property bool visible: !app.runtimeMode
    function run() { app.showShortcutGenerator() }
  }

  readonly property QtObject cmdAppExtensions: QtObject {
    function run() { app.showExtensionManager() }
  }

  readonly property QtObject cmdAppExamples: QtObject {
    function run() { app.showExamplesBrowser() }
  }

  readonly property QtObject cmdAppAbout: QtObject {
    function run() { app.showAboutDialog() }
  }

  readonly property QtObject cmdAppProblems: QtObject {
    readonly property bool visible: !app.runtimeMode
    function run() { app.showProblemCenter() }
  }

  readonly property QtObject cmdAppConnectionDiagnostics: QtObject {
    readonly property bool visible: !app.runtimeMode
    readonly property bool enabled: !Cpp_Misc_ConnectionDiagnostics.running
    function run() { app.runConnectionDiagnostics() }
  }

  //
  // Remote attach stays reachable while attached: the same entry is how a
  // viewer gets back to the dialog to detach.
  //
  readonly property QtObject cmdAppRemoteAttach: QtObject {
    readonly property bool visible: !app.runtimeMode
    function run() { app.showRemoteAttach() }
  }

  readonly property QtObject cmdAppMacros: QtObject {
    readonly property bool visible: !app.runtimeMode
    function run() { app.showMacros() }
  }

  readonly property QtObject cmdAppDeepwiki: QtObject {
    function run() { Qt.openUrlExternally("https://deepwiki.com/Serial-Studio/Serial-Studio") }
  }

  readonly property QtObject cmdAppQuit: QtObject {
    readonly property bool visible: app.runtimeMode
    function run() { app.quitApplication() }
  }

  readonly property QtObject cmdLicenseActivate: QtObject {
    readonly property bool visible: Cpp_CommercialBuild
    function run() { app.showLicenseDialog() }
  }

  //
  // Operation-mode switches; checked marks the active mode, disabled while connected.
  //
  readonly property QtObject cmdModeQuickPlot: QtObject {
    readonly property bool enabled: !Cpp_IO_Manager.isConnected
    readonly property bool checked: Cpp_AppState.operationMode === SerialStudio.QuickPlot
    function run() { Cpp_AppState.operationMode = SerialStudio.QuickPlot }
  }

  readonly property QtObject cmdModeConsoleOnly: QtObject {
    readonly property bool enabled: !Cpp_IO_Manager.isConnected
    readonly property bool checked: Cpp_AppState.operationMode === SerialStudio.ConsoleOnly
    function run() { Cpp_AppState.operationMode = SerialStudio.ConsoleOnly }
  }

  readonly property QtObject cmdModeProjectFile: QtObject {
    readonly property bool enabled: !Cpp_IO_Manager.isConnected
    readonly property bool checked: Cpp_AppState.operationMode === SerialStudio.ProjectFile
    function run() { Cpp_AppState.operationMode = SerialStudio.ProjectFile }
  }

  //
  // Connect/disconnect toggle (Toolbar.qml), incl. the MainWindow fallback.
  //
  readonly property QtObject cmdIoToggleConnection: QtObject {
    readonly property bool checked: Cpp_IO_Manager.isConnected
    readonly property bool visible: Cpp_CommercialBuild
        ? (Cpp_Licensing_Trial.trialExpired && !Cpp_Licensing_LemonSqueezy.isActivated ? false : true)
        : true
    readonly property bool enabled: (Cpp_IO_Manager.isConnected || Cpp_IO_Manager.configurationOk)
        && !Cpp_CSV_Player.isOpen && !Cpp_MDF4_Player.isOpen && !app.sessionPlayerOpen
        && !Cpp_API_Mirror.attached
    function run() {
      if (typeof mainWindow !== "undefined" && mainWindow.userToggleConnection)
        mainWindow.userToggleConnection()
      else
        Cpp_IO_Manager.toggleConnection()
    }
  }

  //
  // Palette and dashboard-pane delegation (MainWindow.qml shortcut bodies);
  // each null-guards the dashboard host ref.
  //
  readonly property QtObject cmdPaletteOpen: QtObject {
    function run() {
      if (root.mwPalette)
        root.mwPalette.toggle()
    }
  }

  readonly property QtObject cmdDashboardStartMenu: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.toggleStartMenu() }
  }

  readonly property QtObject cmdDashboardFocusSearch: QtObject {
    readonly property bool enabled: root.dashboardVisible && Cpp_UI_TaskbarSettings.searchEnabled
    function run() { if (root.dashboard) root.dashboard.focusTaskbarSearch() }
  }

  readonly property QtObject cmdWorkspaceNext: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.cycleWorkspace(+1) }
  }

  readonly property QtObject cmdWorkspacePrevious: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.cycleWorkspace(-1) }
  }

  readonly property QtObject cmdWindowCloseActive: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.closeActiveWindow() }
  }

  readonly property QtObject cmdWindowMinimizeActive: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.minimizeActiveWindow() }
  }

  readonly property QtObject cmdWindowClearActive: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.clearActiveWindow() }
  }

  readonly property QtObject cmdDashboardAutoLayout: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.toggleAutoLayout() }
  }

  readonly property QtObject cmdDashboardFreeze: QtObject {
    readonly property bool enabled: root.dashboardVisible
    function run() { if (root.dashboard) root.dashboard.toggleFreeze() }
  }

  //
  // Driver toggles (Toolbar.qml); enabled state is shared via
  // driverSelectionEnabled.
  //
  readonly property QtObject cmdDriverUart: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.UART
    function run() { Cpp_IO_Manager.busType = SerialStudio.UART }
  }

  readonly property QtObject cmdDriverAudio: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.Audio
    function run() { Cpp_IO_Manager.busType = SerialStudio.Audio }
  }

  readonly property QtObject cmdDriverUsb: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.RawUsb
    function run() { Cpp_IO_Manager.busType = SerialStudio.RawUsb }
  }

  readonly property QtObject cmdDriverNetwork: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.Network
    function run() { Cpp_IO_Manager.busType = SerialStudio.Network }
  }

  readonly property QtObject cmdDriverModbus: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.ModBus
    function run() { Cpp_IO_Manager.busType = SerialStudio.ModBus }
  }

  readonly property QtObject cmdDriverHid: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.HidDevice
    function run() { Cpp_IO_Manager.busType = SerialStudio.HidDevice }
  }

  readonly property QtObject cmdDriverBluetooth: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.BluetoothLE
    function run() { Cpp_IO_Manager.busType = SerialStudio.BluetoothLE }
  }

  readonly property QtObject cmdDriverCanbus: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.CanBus
    function run() { Cpp_IO_Manager.busType = SerialStudio.CanBus }
  }

  readonly property QtObject cmdDriverProcess: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.Process
    function run() { Cpp_IO_Manager.busType = SerialStudio.Process }
  }

  readonly property QtObject cmdDriverOpcUa: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.OpcUa
    function run() { Cpp_IO_Manager.busType = SerialStudio.OpcUa }
  }

  readonly property QtObject cmdDriverS7: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.S7
    function run() { Cpp_IO_Manager.busType = SerialStudio.S7 }
  }

  readonly property QtObject cmdDriverEthernetIp: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.EthernetIp
    function run() { Cpp_IO_Manager.busType = SerialStudio.EthernetIp }
  }

  readonly property QtObject cmdDriverIec104: QtObject {
    readonly property bool enabled: root.driverSelectionEnabled
    readonly property bool checked: Cpp_IO_Manager.busType === SerialStudio.Iec104
    function run() { Cpp_IO_Manager.busType = SerialStudio.Iec104 }
  }
}
