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
  QCommandLineOption apiExternalOpt{
    "api-external",
    "Allow API connections from other hosts, the non-interactive equivalent of the "
    "\"Allow External API Connections\" confirmation. Any machine that can reach port 7777 "
    "may read live data and send commands once it presents the token, and the transport is "
    "not encrypted: use it on a trusted network or through a tunnel only"};
  QCommandLineOption apiTokenOpt{
    "api-token",
    "Set the API authentication token external clients must present (>= 32 hex characters). "
    "Prefer --api-token-file or SS_API_TOKEN: an argv token is visible to every process",
    "hex"};
  QCommandLineOption apiTokenFileOpt{
    "api-token-file",
    "Read the API authentication token from a file instead of the command line",
    "path"};
  QCommandLineOption apiPortOpt{
    "api-port", QObject::tr("Listen for API clients on <port> instead of 7777."), "port"};
  QCommandLineOption dumpApiSchemaOpt{
    "dump-api-schema",
    "Write the API command registry to a JSON file and exit (SDK generator input)",
    "file"};
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
  QCommandLineOption wsOpt{
    "ws", "Connects to a WebSocket endpoint (e.g., ws://192.168.1.100:8080/feed)", "url"};
  QCommandLineOption httpOpt{
    "http", "Polls an HTTP endpoint (e.g., https://api.example.com/telemetry)", "url"};
  QCommandLineOption httpMethodOpt{"http-method", "Sets the HTTP method (default: GET)", "verb"};
  QCommandLineOption httpIntervalOpt{
    "http-interval",
    "Sets the HTTP poll interval in ms; 0 sends only on write (default: 1000)",
    "ms"};
  QCommandLineOption httpHeaderOpt{
    "http-header", "Adds an HTTP request header; repeatable", "name: value"};
  QCommandLineOption insecureTlsOpt{
    "insecure-tls", "Accepts untrusted certificates on wss:// and https:// endpoints"};
  QCommandLineOption benchmarkHotpathOpt{
    "benchmark-hotpath", "Run the in-process frame-extraction throughput benchmark and exit"};
  QCommandLineOption minFpsOpt{
    "min-fps", "Minimum frames/sec the hotpath benchmark must sustain (default: 256000)", "fps"};
  QCommandLineOption benchmarkFramesOpt{
    "benchmark-frames", "Frames to push through the hotpath benchmark (default: 1000000)", "count"};
  QCommandLineOption benchmarkSecondsOpt{
    "benchmark-seconds",
    "Wall-clock seconds the hotpath benchmark must sustain (default: 10)",
    "seconds"};
  QCommandLineOption benchmarkOutputOpt{
    "benchmark-output",
    "File to write the hotpath benchmark report to (default: stdout only, no file)",
    "file"};
  QCommandLineOption exitAfterOpt{
    "exit-after",
    "Quit gracefully after the given number of seconds (CI runs, PGO training)",
    "seconds"};
#ifdef SS_INAPP_TESTS
  QCommandLineOption selftestOpt{"selftest", "Run the built-in self-test suites and exit"};
  QCommandLineOption selftestSuiteOpt{
    "selftest-suite", "Restrict --selftest to a single suite by name", "suite"};
#endif
#ifdef BUILD_COMMERCIAL
  QCommandLineOption noToolbarOpt{"no-toolbar", "Hides the main window toolbar at startup (Pro)"};
  QCommandLineOption runtimeOpt{"runtime",
                                "Operator runtime mode: hides toolbar, quits on disconnect (Pro)"};
  QCommandLineOption shortcutPathOpt{
    "shortcut-path", "Path of the shortcut that launched this process (Pro)", "path"};
  QCommandLineOption csvExportOpt{"csv-export", "Enable CSV export immediately on startup (Pro)"};
  QCommandLineOption mdfExportOpt{"mdf-export", "Enable MDF4 export immediately on startup (Pro)"};
  QCommandLineOption sessionExportOpt{"session-export", "Enable historian export on startup (Pro)"};
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
  QCommandLineOption noTaskbarSearchOpt{
    "no-taskbar-search", "Hide the taskbar search bar in operator runtime mode (Pro)"};
  QCommandLineOption themeOpt{
    "theme",
    "Override the application theme by name (e.g. \"Iconic\", \"Light\", \"System\") (Pro)",
    "name"};
  QCommandLineOption activateOpt{
    "activate", "Activate a license key and exit (for CI/headless setup)", "key"};
  QCommandLineOption deactivateOpt{
    "deactivate", "Deactivate the current license instance and exit (for CI cleanup)"};
  QCommandLineOption selftestOfflineOpt{
    "selftest-offline-license",
    "Run offline-certificate verifier self-test vectors and exit (Pro)"};
  QCommandLineOption verifySessionOpt{
    "verify-session",
    "Verify an archived historian database's reproducibility and exit; prints a JSON verdict "
    "(spec 0044, Pro). Pair with --headless for offscreen operation",
    "file"};
  QCommandLineOption verifySessionIdOpt{
    "verify-session-id",
    "Session id inside the --verify-session archive (default: latest completed session)",
    "id"};
  QCommandLineOption verifyKeepRegenOpt{
    "verify-keep-regen", "Keep the temporary regenerated database for inspection (Pro)"};
  QCommandLineOption regressSessionOpt{
    "regress-session",
    "Compare an archived session against a candidate project and exit; prints a JSON drift "
    "report (spec 0047, Pro). Pair with --headless for offscreen operation",
    "file"};
  QCommandLineOption regressSessionIdOpt{
    "regress-session-id",
    "Session id inside the --regress-session archive (default: latest completed session)",
    "id"};
  QCommandLineOption regressProjectOpt{
    "regress-project", "Candidate project file for --regress-session (Pro)", "file"};
  QCommandLineOption regressKeepRegenOpt{
    "regress-keep-regen", "Keep both temporary regenerated databases for inspection (Pro)"};
  QCommandLineOption validateGuardsOpt{
    "validate-guards", "Verify all embedded license guards pass in this binary and exit (Pro)"};
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
  QCommandLineOption opcuaOpt{
    "opcua", "Connects to an OPC UA server (e.g., opc.tcp://192.168.1.10:4840)", "url"};
  QCommandLineOption opcuaUserOpt{
    "opcua-user", "Sets the OPC UA username (enables username/password authentication)", "name"};
  QCommandLineOption opcuaPassOpt{"opcua-pass", "Sets the OPC UA password", "password"};
  QCommandLineOption opcuaIntervalOpt{
    "opcua-interval", "Sets the OPC UA publishing interval in ms (10-60000, default: 100)", "ms"};
  QCommandLineOption opcuaTagOpt{
    "opcua-tag",
    "Subscribes to a tag: nodeId[:type[:name]] (repeatable, type default f64)",
    "spec"};
  QCommandLineOption s7Opt{"s7", "Connects to a Siemens S7 controller (e.g., 192.168.0.1)", "host"};
  QCommandLineOption s7RackOpt{"s7-rack", "Sets the S7 rack number (0-7, default: 0)", "rack"};
  QCommandLineOption s7SlotOpt{"s7-slot", "Sets the S7 slot number (0-31, default: 1)", "slot"};
  QCommandLineOption s7IntervalOpt{
    "s7-interval", "Sets the S7 poll interval in ms (50-60000, default: 200)", "ms"};
  QCommandLineOption s7VariableOpt{
    "s7-variable", "Polls a variable: address[:name] (repeatable, e.g. DB5.DBD20:REAL)", "spec"};
  QCommandLineOption eipOpt{
    "ethernetip", "Connects to an EtherNet/IP gateway (e.g., 192.168.0.10)", "host"};
  QCommandLineOption eipPathOpt{
    "ethernetip-path", "Sets the CIP routing path (default: 1,0)", "path"};
  QCommandLineOption eipPlcOpt{
    "ethernetip-plc", "Sets the controller family (default: controllogix)", "family"};
  QCommandLineOption eipIntervalOpt{
    "ethernetip-interval",
    "Sets the EtherNet/IP poll interval in ms (50-60000, default: 250)",
    "ms"};
  QCommandLineOption eipTagOpt{
    "ethernetip-tag", "Polls a tag: tag[:type[:element]] (repeatable, type default f32)", "spec"};
  QCommandLineOption iec104Opt{
    "iec104", "Connects to an IEC 60870-5-104 station (e.g., 192.168.0.20)", "host"};
  QCommandLineOption iec104PortOpt{
    "iec104-port", "Sets the IEC 60870-5-104 port (default: 2404)", "port"};
  QCommandLineOption iec104CaOpt{
    "iec104-ca", "Sets the common address of ASDU (0-65535, default: 1)", "address"};
  QCommandLineOption iec104KOpt{"iec104-k", "Sets the send window k (default: 12)", "frames"};
  QCommandLineOption iec104WOpt{
    "iec104-w", "Sets the acknowledgement window w (default: 8)", "frames"};
  QCommandLineOption iec104T1Opt{
    "iec104-t1", "Sets the send/confirm timeout in ms (default: 15000)", "ms"};
  QCommandLineOption iec104T2Opt{
    "iec104-t2", "Sets the acknowledgement timeout in ms (default: 10000)", "ms"};
  QCommandLineOption iec104T3Opt{
    "iec104-t3", "Sets the idle-test timeout in ms (default: 20000)", "ms"};
#endif
};

/**
 * @brief Parses and applies the command-line interface for Serial Studio.
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
  static bool isCliEarlyExit(int argc, char** argv);
  static bool isBenchmarkRequested(int argc, char** argv);

  ProcessResult process(QApplication& app);
  [[nodiscard]] bool postRootSelfTestRequested() const;
  [[nodiscard]] ProcessResult runPostRootSelfTests();

  [[nodiscard]] bool fullscreen() const noexcept;
  [[nodiscard]] bool runtimeMode() const noexcept;
  [[nodiscard]] bool hideToolbar() const noexcept;
  [[nodiscard]] bool apiServerEnabled() const;
  [[nodiscard]] bool apiExternalEnabled() const;
  [[nodiscard]] bool quickPlot() const;
  [[nodiscard]] QString projectPath() const;

  [[nodiscard]] bool verifyShortcutProject() const;

  void applyProjectAndAutoConnect(QApplication& app);
  void applyVisualizationOptions();
  void applyBusConfiguration();

#ifdef BUILD_COMMERCIAL
  void applyThemeOverride();
  void applyOperatorRuntimeSettings();
  void applyExportToggles();
#endif

private:
  void registerOptions();
  void registerCommercialOptions();
  void applyApiServerOptions();
  void scheduleExitAfter(QApplication& app);
  [[nodiscard]] QString resolveApiToken() const;

  ProcessResult runHotpathBenchmark();
  ProcessResult dumpApiSchema(const QString& path);

#ifdef BUILD_COMMERCIAL
  ProcessResult runSessionVerification();
  ProcessResult runSessionRegression();
#endif

#ifdef SS_INAPP_TESTS
  ProcessResult runSelfTests();
#endif

#ifdef BUILD_COMMERCIAL
  int activateLicense(QApplication& app, const QString& licenseKey);
  int deactivateLicense(QApplication& app);

  void applyOperatorTaskbarSettings();
#endif

  CliOptions m_opts;
  QCommandLineParser m_parser;
};

}  // namespace Misc
