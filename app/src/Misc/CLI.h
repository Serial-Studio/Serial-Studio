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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>

class QApplication;

namespace Misc {

/**
 * @brief Aggregate of every command-line option the application accepts.
 */
struct CliOptions {
  QCommandLineOption versionOpt{
    {"v", "version"},
    "Displays application version"
  };
  QCommandLineOption resetOpt{
    {"r", "reset"},
    "Resets all application settings"
  };
  QCommandLineOption fullscreenOpt{
    {"f", "fullscreen"},
    "Launches dashboard in fullscreen mode"
  };
  QCommandLineOption headlessOpt{"headless", "Run without GUI (headless/server mode)"};
  QCommandLineOption apiServerOpt{"api-server", "Enable API server on startup (port 7777)"};
  QCommandLineOption projectOpt{
    {"p", "project"},
    "Loads the specified project file", "file"
  };
  QCommandLineOption quickPlotOpt{
    {"q", "quick-plot"},
    "Enables quick plot mode (auto-detect CSV data)"
  };
  QCommandLineOption fpsOpt{
    {"t", "fps"},
    "Sets visualization refresh rate", "Hz"
  };
  QCommandLineOption pointsOpt{
    {"n", "points"},
    "Sets data points per plot", "count"
  };
  QCommandLineOption uartOpt{"uart", "Specifies serial port (e.g., /dev/ttyUSB0, COM3)", "port"};
  QCommandLineOption baudOpt{"baud", "Sets serial baud rate (default: 9600)", "rate"};
  QCommandLineOption tcpOpt{
    "tcp", "Connects to TCP server (e.g., 192.168.1.100:8080)", "host:port"};
  QCommandLineOption udpOpt{"udp", "Binds to UDP local port (e.g., 8080)", "port"};
  QCommandLineOption udpRemoteOpt{
    "udp-remote", "Specifies UDP remote target (e.g., 192.168.1.100:8080)", "host:port"};
  QCommandLineOption udpMulticastOpt{"udp-multicast", "Enables multicast mode for UDP"};
#ifdef BUILD_COMMERCIAL
  QCommandLineOption noToolbarOpt{"no-toolbar", "Hides the main window toolbar at startup (Pro)"};
  QCommandLineOption runtimeOpt{"runtime",
                                "Operator runtime mode: hides toolbar, quits on disconnect (Pro)"};
  QCommandLineOption shortcutPathOpt{
    "shortcut-path", "Path of the shortcut that launched this process (Pro)", "path"};
  QCommandLineOption csvExportOpt{"csv-export", "Enable CSV export immediately on startup (Pro)"};
  QCommandLineOption mdfExportOpt{"mdf-export", "Enable MDF4 export immediately on startup (Pro)"};
  QCommandLineOption sessionExportOpt{"session-export",
                                      "Enable session database export on startup (Pro)"};
  QCommandLineOption consoleExportOpt{"console-export",
                                      "Enable console log export on startup (Pro)"};
  QCommandLineOption actionsPanelOpt{"actions-panel",
                                     "Show the actions panel in operator runtime mode (Pro)"};
  QCommandLineOption fileTransmissionOpt{
    "file-transmission",
    "Allow opening the File Transmission dialog in operator runtime mode (Pro)"};
  QCommandLineOption taskbarModeOpt{
    "taskbar-mode",
    "Operator-mode dashboard taskbar visibility: shown / autohide / hidden (Pro)",
    "mode"};
  QCommandLineOption taskbarButtonsOpt{
    "taskbar-buttons", "Comma-separated taskbar pin IDs for operator mode (Pro)", "ids"};
  QCommandLineOption activateOpt{
    "activate", "Activate a license key and exit (for CI/headless setup)", "key"};
  QCommandLineOption deactivateOpt{
    "deactivate", "Deactivate the current license instance and exit (for CI cleanup)"};
  QCommandLineOption modbusRtuOpt{
    "modbus-rtu", "Connects to ModBus RTU device (e.g., /dev/ttyUSB0, COM3)", "port"};
  QCommandLineOption modbusTcpOpt{
    "modbus-tcp", "Connects to ModBus TCP server (e.g., 192.168.1.100:502)", "host:port"};
  QCommandLineOption modbusSlaveOpt{
    "modbus-slave", "Sets ModBus slave address (1-247, default: 1)", "address"};
  QCommandLineOption modbusPollOpt{
    "modbus-poll", "Sets ModBus poll interval in ms (50-60000, default: 100)", "interval"};
  QCommandLineOption modbusBaudOpt{
    "modbus-baud", "Sets ModBus RTU baud rate (default: 9600)", "rate"};
  QCommandLineOption modbusParityOpt{
    "modbus-parity", "Sets ModBus RTU parity (none/even/odd/space/mark, default: none)", "type"};
  QCommandLineOption modbusDataBitsOpt{
    "modbus-databits", "Sets ModBus RTU data bits (5/6/7/8, default: 8)", "bits"};
  QCommandLineOption modbusStopBitsOpt{
    "modbus-stopbits", "Sets ModBus RTU stop bits (1/1.5/2, default: 1)", "bits"};
  QCommandLineOption modbusRegisterOpt{
    "modbus-register", "Adds ModBus register group: type:start:count (repeatable)", "spec"};
  QCommandLineOption canbusOpt{
    "canbus", "Connects to CAN bus (e.g., socketcan:can0, peakcan:pcan0)", "plugin:interface"};
  QCommandLineOption canbusBitrateOpt{
    "canbus-bitrate", "Sets CAN bus bitrate in bps (default: 500000)", "rate"};
  QCommandLineOption canbusFdOpt{"canbus-fd", "Enables CAN-FD mode"};
#endif
};

/**
 * @brief Parses and applies the command-line interface for Serial Studio.
 *
 * Owns the QCommandLineParser, all CLI options, and every "apply X to runtime"
 * helper. main() instantiates one CLI, calls @c process() before constructing
 * dependent subsystems, then calls the apply* methods after the QML interface
 * is up.
 */
class CLI {
public:
  enum class ProcessResult {
    Continue,
    ExitSuccess,
    ExitFailure
  };

  CLI();

  static bool argvHasFlag(int argc, char** argv, const char* flag);
  static QString argvValueFor(int argc, char** argv, const char* flag);

  ProcessResult process(QApplication& app);

  [[nodiscard]] bool fullscreen() const noexcept;
  [[nodiscard]] bool runtimeMode() const noexcept;
  [[nodiscard]] bool hideToolbar() const noexcept;
  [[nodiscard]] bool apiServerEnabled() const;
  [[nodiscard]] bool quickPlot() const;
  [[nodiscard]] QString projectPath() const;

  [[nodiscard]] bool verifyShortcutProject() const;

  void applyProjectAndAutoConnect(QApplication& app);
  void applyVisualizationOptions();
  void applyBusConfiguration();

#ifdef BUILD_COMMERCIAL
  void applyOperatorRuntimeSettings();
  void applyExportToggles();
#endif

private:
  void registerOptions();

  void setupUartConnection();
  void setupTcpConnection(const QString& tcpAddress);
  void setupUdpConnection();

#ifdef BUILD_COMMERCIAL
  int activateLicense(QApplication& app, const QString& licenseKey);
  int deactivateLicense(QApplication& app);

  void setupModbusRtuConnection();
  void setupModbusTcpConnection();
  void setupCanbusConnection();
  void applyModbusCommonOptions();
  void applyOperatorTaskbarSettings();
#endif

  CliOptions m_opts;
  QCommandLineParser m_parser;
};

}  // namespace Misc
