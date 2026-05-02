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

// clang-format off
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <shlobj.h>
#endif
// clang-format on

#include <cstring>
#include <QApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QFileOpenEvent>
#include <QLoggingCategory>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSettings>
#include <QStyleFactory>
#include <QSysInfo>
#include <QTimer>

#ifdef Q_OS_LINUX
#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QStandardPaths>
#endif

#include <QtWebEngineQuick>

#include "API/Server.h"
#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/FileTransmission.h"
#include "Misc/ModuleManager.h"
#include "Misc/TimerEvents.h"
#include "UI/Dashboard.h"
#include "UI/TaskbarSettings.h"

#ifdef BUILD_COMMERCIAL
#  include <QAbstractButton>
#  include <QFileInfo>
#  include <QMessageBox>
#  include <QPushButton>

#  include "Console/Export.h"
#  include "CSV/Export.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "MDF4/Export.h"
#  include "Misc/ShortcutGenerator.h"
#  include "Sessions/Export.h"
#endif

//--------------------------------------------------------------------------------------------------
// Declare utility functions
//--------------------------------------------------------------------------------------------------

static void cliShowVersion();
static void cliResetSettings();
static bool argvHasFlag(int argc, char** argv, const char* flag);
static QString argvValueFor(int argc, char** argv, const char* flag);
static QString shortcutIdentityHash(const QString& shortcutPath);
static char** injectPlatformArg(int& argc, char** argv, const char* platform);

static void setupUartConnection(const QCommandLineParser& parser,
                                const QCommandLineOption& uartOpt,
                                const QCommandLineOption& baudOpt);
static void setupTcpConnection(const QString& tcpAddress);
static void setupUdpConnection(const QCommandLineParser& parser,
                               const QCommandLineOption& udpOpt,
                               const QCommandLineOption& udpRemoteOpt,
                               const QCommandLineOption& udpMltcstOpt);

#ifdef BUILD_COMMERCIAL
static int cliActivateLicense(QApplication& app, const QString& licenseKey);
static int cliDeactivateLicense(QApplication& app);
static void applyModbusRegister(const QString& spec);
static void configureCanbusInterface(const QCommandLineParser& parser,
                                     const QCommandLineOption& bitrateOpt,
                                     const QCommandLineOption& fdOpt,
                                     int interfaceIndex,
                                     const QString& interfaceName,
                                     const QString& plugin,
                                     const QStringList& availableInterfaces);

struct ModbusCliOptions {
  const QCommandLineOption& slaveOpt;
  const QCommandLineOption& pollOpt;
  const QCommandLineOption& baudOpt;
  const QCommandLineOption& parityOpt;
  const QCommandLineOption& dataBitsOpt;
  const QCommandLineOption& stopBitsOpt;
  const QCommandLineOption& registerOpt;
};

static void setupModbusRtuConnection(const QCommandLineParser& parser,
                                     const QCommandLineOption& portOpt,
                                     const ModbusCliOptions& opts);
static void setupModbusTcpConnection(const QCommandLineParser& parser,
                                     const QCommandLineOption& addrOpt,
                                     const ModbusCliOptions& opts);
static void setupCanbusConnection(const QCommandLineParser& parser,
                                  const QCommandLineOption& canbusOpt,
                                  const QCommandLineOption& bitrateOpt,
                                  const QCommandLineOption& fdOpt);
#endif

#ifdef Q_OS_WINDOWS
static void attachToConsole();
static void setWindowsAppUserModelId(const QString& shortcutPath);
static void enableWindowsPerformanceMode();
static void registerWindowsFileAssociation();
static char** adjustArgumentsForFreeType(int& argc, char** argv);
#endif

//--------------------------------------------------------------------------------------------------
// File-open event filter (macOS double-click / Linux xdg-open)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Event filter that intercepts QFileOpenEvent to open .ssproj files.
 *
 * On macOS, double-clicking a .ssproj file in Finder sends a QFileOpenEvent
 * to the running (or launching) application. This filter catches it and loads
 * the project.
 */
class FileOpenEventFilter : public QObject {
public:
  using QObject::QObject;

protected:
  /**
   * @brief Intercepts QFileOpenEvent and loads the referenced .ssproj into the project model.
   */
  bool eventFilter(QObject* obj, QEvent* event) override
  {
    if (event->type() == QEvent::FileOpen) {
      auto* fileEvent    = static_cast<QFileOpenEvent*>(event);
      const QString path = fileEvent->file();
      if (path.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive)) {
        AppState::instance().setOperationMode(SerialStudio::ProjectFile);
        DataModel::ProjectModel::instance().openJsonFile(path);
        return true;
      }
    }

    return QObject::eventFilter(obj, event);
  }
};

//--------------------------------------------------------------------------------------------------
// Entry-point function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Entry-point function of the application
 *
 * @param argc argument count
 * @param argv argument data
 *
 * @return qApp exit code
 */
int main(int argc, char** argv)
{
  // Configure application metadata
  QLoggingCategory::setFilterRules("*font*=false");

  // Set application info
  QApplication::setApplicationName(APP_EXECUTABLE);
  QApplication::setOrganizationName(APP_DEVELOPER);
  QApplication::setApplicationVersion(APP_VERSION);
  QApplication::setApplicationDisplayName(APP_NAME);
  QApplication::setOrganizationDomain(APP_SUPPORT_URL);

  // Set application attributes
  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
  QApplication::setAttribute(Qt::AA_DontUseNativeMenuWindows);

  // Handle headless mode and platform-specific initialization
  const bool headless = argvHasFlag(argc, argv, "--headless");
  if (headless)
    argv = injectPlatformArg(argc, argv, "offscreen");

  // Pre-parse --shortcut-path: needed before Qt for taskbar identity and QML settings isolation
  const QString earlyShortcutPath = argvValueFor(argc, argv, "--shortcut-path");

#if defined(Q_OS_WIN)
  // Windows-specific fixes, attach to console and allow fonts to look decent on 1x scaling
  attachToConsole();
  setWindowsAppUserModelId(earlyShortcutPath);
  enableWindowsPerformanceMode();
  argv = adjustArgumentsForFreeType(argc, argv);
#endif

  // Allow fractional scaling
  auto policy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;
  QApplication::setHighDpiScaleFactorRoundingPolicy(policy);

  // Initialize Web Engine Module & application
  QtWebEngineQuick::initialize();
  QApplication app(argc, argv);

  // Install event filter for file-open events (macOS Finder, Linux xdg-open)
  FileOpenEventFilter fileOpenFilter;
  app.installEventFilter(&fileOpenFilter);

#ifdef Q_OS_WIN
  // Register .ssproj file association in the Windows registry
  registerWindowsFileAssociation();
#endif

#if !defined(Q_OS_MAC)
  // Set window icon
  QIcon appIcon(QStringLiteral(":/logo/icon.svg"));
  if (!appIcon.isNull())
    app.setWindowIcon(appIcon);
#endif

  // Set application style to Fusion
  app.setStyle(QStyleFactory::create("Fusion"));
  QQuickStyle::setStyle("Fusion");

  // Define command-line options
  // clang-format off
  typedef QCommandLineOption QCLO;
  QCLO vOpt({"v", "version"}, "Displays application version");
  QCLO rOpt({"r", "reset"}, "Resets all application settings");
  QCLO fOpt({"f", "fullscreen"}, "Launches dashboard in fullscreen mode");
  QCLO headlessOpt("headless", "Run without GUI (headless/server mode)");
  QCLO apiServerOpt("api-server", "Enable API server on startup (port 7777)");
  QCLO pOpt({"p", "project"}, "Loads the specified project file", "file");
  QCLO qOpt({"q", "quick-plot"}, "Enables quick plot mode (auto-detect CSV data)");
  QCLO fpsOpt({"t", "fps"}, "Sets visualization refresh rate", "Hz");
  QCLO pointsOpt({"n", "points"}, "Sets data points per plot", "count");
  QCLO uartOpt("uart", "Specifies serial port (e.g., /dev/ttyUSB0, COM3)", "port");
  QCLO baudOpt("baud", "Sets serial baud rate (default: 9600)", "rate");
  QCLO tcpOpt("tcp", "Connects to TCP server (e.g., 192.168.1.100:8080)", "host:port");
  QCLO udpOpt("udp", "Binds to UDP local port (e.g., 8080)", "port");
  QCLO udpRemoteOpt("udp-remote", "Specifies UDP remote target (e.g., 192.168.1.100:8080)", "host:port");
  QCLO udpMltcstOpt("udp-multicast", "Enables multicast mode for UDP");
#ifdef BUILD_COMMERCIAL
  QCLO noToolbarOpt("no-toolbar", "Hides the main window toolbar at startup (Pro)");
  QCLO runtimeOpt("runtime", "Operator runtime mode: hides toolbar, quits on disconnect (Pro)");
  QCLO shortcutPathOpt("shortcut-path", "Path of the shortcut that launched this process (Pro)", "path");
  QCLO csvExportOpt("csv-export", "Enable CSV export immediately on startup (Pro)");
  QCLO mdfExportOpt("mdf-export", "Enable MDF4 export immediately on startup (Pro)");
  QCLO sessionExportOpt("session-export", "Enable session database export on startup (Pro)");
  QCLO consoleExportOpt("console-export", "Enable console log export on startup (Pro)");
  QCLO actionsPanelOpt("actions-panel", "Show the actions panel in operator runtime mode (Pro)");
  QCLO fileTransmissionOpt("file-transmission", "Allow opening the File Transmission dialog in operator runtime mode (Pro)");
  QCLO taskbarModeOpt("taskbar-mode", "Operator-mode dashboard taskbar visibility: shown / autohide / hidden (Pro)", "mode");
  QCLO taskbarButtonsOpt("taskbar-buttons", "Comma-separated taskbar pin IDs for operator mode (Pro)", "ids");
  QCLO activateOpt("activate", "Activate a license key and exit (for CI/headless setup)", "key");
  QCLO deactivateOpt("deactivate", "Deactivate the current license instance and exit (for CI cleanup)");
  QCLO modbusRtuOpt("modbus-rtu", "Connects to ModBus RTU device (e.g., /dev/ttyUSB0, COM3)", "port");
  QCLO modbusTcpOpt("modbus-tcp", "Connects to ModBus TCP server (e.g., 192.168.1.100:502)", "host:port");
  QCLO modbusSlaveOpt("modbus-slave", "Sets ModBus slave address (1-247, default: 1)", "address");
  QCLO modbusPollOpt("modbus-poll", "Sets ModBus poll interval in ms (50-60000, default: 100)", "interval");
  QCLO modbusBaudOpt("modbus-baud", "Sets ModBus RTU baud rate (default: 9600)", "rate");
  QCLO modbusParityOpt("modbus-parity", "Sets ModBus RTU parity (none/even/odd/space/mark, default: none)", "type");
  QCLO modbusDataBitsOpt("modbus-databits", "Sets ModBus RTU data bits (5/6/7/8, default: 8)", "bits");
  QCLO modbusStopBitsOpt("modbus-stopbits", "Sets ModBus RTU stop bits (1/1.5/2, default: 1)", "bits");
  QCLO modbusRegisterOpt("modbus-register", "Adds ModBus register group: type:start:count (repeatable)", "spec");
  QCLO canbusOpt("canbus", "Connects to CAN bus (e.g., socketcan:can0, peakcan:pcan0)", "plugin:interface");
  QCLO canbusBitrateOpt("canbus-bitrate", "Sets CAN bus bitrate in bps (default: 500000)", "rate");
  QCLO canbusFdOpt("canbus-fd", "Enables CAN-FD mode");
#endif
  // clang-format on

  // Parse command line and handle early-exit flags
  QCommandLineParser parser;
  parser.setApplicationDescription(PROJECT_DESCRIPTION_SUMMARY);
  parser.addHelpOption();
  parser.addOption(vOpt);
  parser.addOption(rOpt);
  parser.addOption(fOpt);
  parser.addOption(headlessOpt);
  parser.addOption(apiServerOpt);
  parser.addOption(pOpt);
  parser.addOption(qOpt);
  parser.addOption(fpsOpt);
  parser.addOption(pointsOpt);
  parser.addOption(uartOpt);
  parser.addOption(baudOpt);
  parser.addOption(tcpOpt);
  parser.addOption(udpOpt);
  parser.addOption(udpRemoteOpt);
  parser.addOption(udpMltcstOpt);
#ifdef BUILD_COMMERCIAL
  parser.addOption(noToolbarOpt);
  parser.addOption(runtimeOpt);
  parser.addOption(shortcutPathOpt);
  parser.addOption(csvExportOpt);
  parser.addOption(mdfExportOpt);
  parser.addOption(sessionExportOpt);
  parser.addOption(consoleExportOpt);
  parser.addOption(actionsPanelOpt);
  parser.addOption(fileTransmissionOpt);
  parser.addOption(taskbarModeOpt);
  parser.addOption(taskbarButtonsOpt);
  parser.addOption(activateOpt);
  parser.addOption(deactivateOpt);
  parser.addOption(modbusRtuOpt);
  parser.addOption(modbusTcpOpt);
  parser.addOption(modbusSlaveOpt);
  parser.addOption(modbusPollOpt);
  parser.addOption(modbusBaudOpt);
  parser.addOption(modbusParityOpt);
  parser.addOption(modbusDataBitsOpt);
  parser.addOption(modbusStopBitsOpt);
  parser.addOption(modbusRegisterOpt);
  parser.addOption(canbusOpt);
  parser.addOption(canbusBitrateOpt);
  parser.addOption(canbusFdOpt);
#endif

  // Process CLI arguments
  parser.process(app);

  // Display application version
  if (parser.isSet(vOpt)) {
    cliShowVersion();
    return EXIT_SUCCESS;
  }

  // Reset application settings
  if (parser.isSet(rOpt)) {
    cliResetSettings();
    return EXIT_SUCCESS;
  }

#ifdef BUILD_COMMERCIAL
  // Activate/deactivate Serial Studio
  if (parser.isSet(activateOpt))
    return cliActivateLicense(app, parser.value(activateOpt));

  if (parser.isSet(deactivateOpt))
    return cliDeactivateLicense(app);
#endif

  // Initialize resources and module manager
  Q_INIT_RESOURCE(rcc);
  Q_INIT_RESOURCE(translations);

  // Determine runtime / fullscreen / hide-toolbar flags.
#ifdef BUILD_COMMERCIAL
  const bool runtimeMode = parser.isSet(runtimeOpt);
  const bool fullscreen  = parser.isSet(fOpt);
  const bool hideToolbar = runtimeMode || parser.isSet(noToolbarOpt) || fullscreen;
#else
  const bool runtimeMode = false;
  const bool fullscreen  = parser.isSet(fOpt);
  const bool hideToolbar = fullscreen;
#endif

#ifdef BUILD_COMMERCIAL
  // Handle missing shortcut project before QML loads, else the runtime window flashes behind it
  if (runtimeMode && parser.isSet(pOpt)) {
    const QString projectPath = parser.value(pOpt);
    if (!QFileInfo::exists(projectPath)) {
      const QString shortcutPath =
        parser.isSet(shortcutPathOpt) ? parser.value(shortcutPathOpt) : QString();

      QMessageBox box;
      box.setIcon(QMessageBox::Warning);
      box.setWindowTitle(QObject::tr("Project file not found"));
      box.setText(QObject::tr("The project file referenced by this shortcut "
                              "could not be found:\n\n%1")
                    .arg(projectPath));
      box.setInformativeText(QObject::tr("Would you like to delete this shortcut?"));

      QAbstractButton* deleteBtn = nullptr;
      if (!shortcutPath.isEmpty())
        deleteBtn = box.addButton(QObject::tr("Delete Shortcut"), QMessageBox::DestructiveRole);

      box.addButton(QObject::tr("Quit"), QMessageBox::RejectRole);
      box.exec();

      if (deleteBtn != nullptr && box.clickedButton() == deleteBtn)
        Misc::ShortcutGenerator::instance().deleteShortcut(shortcutPath);

      return EXIT_SUCCESS;
    }
  }
#endif

  // Initialize application modules
  Misc::ModuleManager moduleManager;
  moduleManager.setHeadless(headless);
  moduleManager.configureUpdater();
  moduleManager.registerQmlTypes();

  // Compute per-shortcut settings suffix so each .lnk has its own QML state
  const QString settingsSuffix = earlyShortcutPath.isEmpty()
                                 ? QString()
                                 : QStringLiteral("_") + shortcutIdentityHash(earlyShortcutPath);

  // Publish CLI flags before main.qml loads (creation-time bindings need them).
  {
    const auto ctx = moduleManager.engine().rootContext();
    ctx->setContextProperty("CLI_START_FULLSCREEN", fullscreen);
    ctx->setContextProperty("CLI_HIDE_TOOLBAR", hideToolbar);
    ctx->setContextProperty("CLI_RUNTIME_MODE", runtimeMode);
    ctx->setContextProperty("CLI_SETTINGS_SUFFIX", settingsSuffix);
  }

  // Now load main.qml and the rest of the QML interface
  moduleManager.initializeQmlInterface();
  if (!headless && moduleManager.engine().rootObjects().isEmpty()) {
    qCritical() << "Critical QML error";
    return EXIT_FAILURE;
  }

  // Apply CLI operation mode and connection settings
  if (parser.isSet(apiServerOpt))
    API::Server::instance().setEnabled(true);

  // Load a project from disk
  if (parser.isSet(pOpt)) {
    QString projectPath = parser.value(pOpt);
    AppState::instance().setOperationMode(SerialStudio::ProjectFile);
    DataModel::ProjectModel::instance().openJsonFile(projectPath);
  }

  // Enable Quick Plot Mode
  else if (parser.isSet(qOpt))
    AppState::instance().setOperationMode(SerialStudio::QuickPlot);

  // Auto-connect runtime-mode launches once the bus type is wired up.
  if (runtimeMode) {
    QTimer::singleShot(0, &app, []() {
#ifdef BUILD_COMMERCIAL
      if (!Licensing::LemonSqueezy::instance().isActivated()
          && !Licensing::Trial::instance().trialEnabled())
        return;
#endif
      auto& cm = IO::ConnectionManager::instance();
      if (cm.configurationOk() && !cm.isConnected())
        cm.connectDevice();
    });
  }

  // CLI export toggles: runtime mode pins each flag non-persistent so shortcuts can't leak settings
#ifdef BUILD_COMMERCIAL
  if (runtimeMode) {
    CSV::Export::instance().setSettingsPersistent(false);
    MDF4::Export::instance().setSettingsPersistent(false);
    Sessions::Export::instance().setSettingsPersistent(false);
    Console::Export::instance().setSettingsPersistent(false);
    UI::Dashboard::instance().setSettingsPersistent(false);
    UI::TaskbarSettings::instance().setSettingsPersistent(false);

    CSV::Export::instance().setExportEnabled(parser.isSet(csvExportOpt));
    MDF4::Export::instance().setExportEnabled(parser.isSet(mdfExportOpt));
    Sessions::Export::instance().setExportEnabled(parser.isSet(sessionExportOpt));
    Console::Export::instance().setExportEnabled(parser.isSet(consoleExportOpt));

    UI::Dashboard::instance().setTerminalEnabled(false);
    UI::Dashboard::instance().setNotificationLogEnabled(false);
    UI::Dashboard::instance().setShowActionPanel(parser.isSet(actionsPanelOpt));
    IO::FileTransmission::instance().setRuntimeAccessAllowed(parser.isSet(fileTransmissionOpt));

    // Operator-mode taskbar layout
    auto& tbs = UI::TaskbarSettings::instance();
    if (parser.isSet(taskbarModeOpt)) {
      const QString mode = parser.value(taskbarModeOpt).toLower();
      tbs.setTaskbarHidden(mode == QStringLiteral("hidden"));
      tbs.setAutohide(mode == QStringLiteral("autohide"));
    } else {
      tbs.setTaskbarHidden(false);
      tbs.setAutohide(false);
    }
    if (parser.isSet(taskbarButtonsOpt)) {
      const QString raw = parser.value(taskbarButtonsOpt);
      QStringList ids;
      const auto parts = raw.split(QLatin1Char(','), Qt::SkipEmptyParts);
      ids.reserve(parts.size());
      for (const auto& p : parts)
        ids.append(p.trimmed());

      tbs.setPinnedButtons(ids);
    }
  } else {
    if (parser.isSet(csvExportOpt))
      CSV::Export::instance().setExportEnabled(true);

    if (parser.isSet(mdfExportOpt))
      MDF4::Export::instance().setExportEnabled(true);

    if (parser.isSet(sessionExportOpt))
      Sessions::Export::instance().setExportEnabled(true);

    if (parser.isSet(consoleExportOpt))
      Console::Export::instance().setExportEnabled(true);
  }
#endif

  // Set dashboard FPS
  if (parser.isSet(fpsOpt)) {
    bool ok;
    auto fps = parser.value(fpsOpt).toUInt(&ok);
    if (ok)
      Misc::TimerEvents::instance().setFPS(fps);
  }

  // Set dashboard point count
  if (parser.isSet(pointsOpt)) {
    bool ok;
    auto points = parser.value(pointsOpt).toUInt(&ok);
    if (ok)
      UI::Dashboard::instance().setPoints(points);
  }

  // Apply CLI bus configuration (UART/TCP/UDP/Modbus/CAN)
  if (parser.isSet(uartOpt) || parser.isSet(baudOpt))
    setupUartConnection(parser, uartOpt, baudOpt);
  else if (parser.isSet(tcpOpt))
    setupTcpConnection(parser.value(tcpOpt));
  else if (parser.isSet(udpOpt))
    setupUdpConnection(parser, udpOpt, udpRemoteOpt, udpMltcstOpt);
#ifdef BUILD_COMMERCIAL
  else if (parser.isSet(modbusRtuOpt))
    setupModbusRtuConnection(parser,
                             modbusRtuOpt,
                             {modbusSlaveOpt,
                              modbusPollOpt,
                              modbusBaudOpt,
                              modbusParityOpt,
                              modbusDataBitsOpt,
                              modbusStopBitsOpt,
                              modbusRegisterOpt});
  else if (parser.isSet(modbusTcpOpt))
    setupModbusTcpConnection(parser,
                             modbusTcpOpt,
                             {modbusSlaveOpt,
                              modbusPollOpt,
                              modbusBaudOpt,
                              modbusParityOpt,
                              modbusDataBitsOpt,
                              modbusStopBitsOpt,
                              modbusRegisterOpt});
  else if (parser.isSet(canbusOpt))
    setupCanbusConnection(parser, canbusOpt, canbusBitrateOpt, canbusFdOpt);
#endif

  // Start the application event loop
  const auto status = app.exec();

#ifdef Q_OS_WIN
  // Free Windows font-config args
  for (int i = 0; i < argc; ++i)
    free(argv[i]);

  free(argv);
#endif

  // Return exist status to the operating system
  return status;
}

//--------------------------------------------------------------------------------------------------
// Implement utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scans raw argv for an exact match of @p flag before Qt parses arguments.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param flag The flag string to search for (e.g. "--headless").
 * @return @c true if the flag is present.
 */
static bool argvHasFlag(int argc, char** argv, const char* flag)
{
  for (int i = 1; i < argc; ++i)
    if (std::strcmp(argv[i], flag) == 0)
      return true;

  return false;
}

/**
 * @brief Reads the value following @p flag in raw argv before Qt parses arguments.
 *
 * Used to read flags like @c --shortcut-path very early -- before QApplication
 * is constructed -- so the per-shortcut taskbar identity and settings suffix
 * can be configured before any window is created.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param flag The flag string to search for (e.g. "--shortcut-path").
 * @return The value string if found, or an empty QString.
 */
static QString argvValueFor(int argc, char** argv, const char* flag)
{
  for (int i = 1; i < argc - 1; ++i)
    if (std::strcmp(argv[i], flag) == 0)
      return QString::fromLocal8Bit(argv[i + 1]);

  return QString();
}

/**
 * @brief Computes a stable short identity hash for a shortcut path.
 *
 * Returns the first 16 hex characters of SHA-1(path) -- distinct enough to
 * separate shortcuts that target the same project file, stable across launches,
 * and short enough to fit cleanly into AUMID and QSettings category strings.
 *
 * @param shortcutPath The absolute path of the .lnk / .desktop / .command file.
 * @return The 16-character hex identity, or an empty string if the path is empty.
 */
static QString shortcutIdentityHash(const QString& shortcutPath)
{
  if (shortcutPath.isEmpty())
    return QString();

  const QByteArray digest =
    QCryptographicHash::hash(shortcutPath.toUtf8(), QCryptographicHash::Sha1);
  return QString::fromLatin1(digest.toHex().left(16));
}

/**
 * @brief Injects "-platform <platform>" into argv before Qt reads it.
 *
 * Allocates a new argv array with two extra slots prepended after argv[0].
 * The caller owns the returned memory; on Windows, the original argv is already
 * heap-allocated by adjustArgumentsForFreeType and will be freed at exit.
 *
 * @param argc  Reference to the argument count; incremented by 2.
 * @param argv  Original argument vector.
 * @param platform  Platform plugin name (e.g. "offscreen").
 * @return New argv array with the platform arguments injected.
 */
static char** injectPlatformArg(int& argc, char** argv, const char* platform)
{
  char** newArgv = static_cast<char**>(malloc(sizeof(char*) * (argc + 3)));
  if (!newArgv)
    return argv;

  newArgv[0] = argv[0];
  newArgv[1] = const_cast<char*>("-platform");
  newArgv[2] = const_cast<char*>(platform);

  for (int i = 1; i < argc; ++i)
    newArgv[i + 2] = argv[i];

  newArgv[argc + 2] = nullptr;
  argc += 2;
  return newArgv;
}

/**
 * @brief Prints the current application name and version to the console.
 */
static void cliShowVersion()
{
  qDebug() << APP_NAME << "version" << APP_VERSION;
  qDebug() << "Written by Alex Spataru <https://github.com/alex-spataru>";
}

/**
 * @brief Clears all persisted application settings from QSettings.
 */
static void cliResetSettings()
{
  QSettings(APP_SUPPORT_URL, APP_NAME).clear();
  qDebug() << APP_NAME << "settings cleared!";
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Activates a license key against the Lemon Squeezy API and exits.
 *
 * Intended for CI/headless environments where the license must be stored in
 * QSettings before running the application in server mode. Runs the event loop
 * until activation completes (success or failure) or a 30-second timeout fires.
 *
 * @param app        The QApplication instance.
 * @param licenseKey The 36-character UUID license key to activate.
 * @return EXIT_SUCCESS if the license was activated and saved, EXIT_FAILURE otherwise.
 */
static int cliActivateLicense(QApplication& app, const QString& licenseKey)
{
  auto& ls = Licensing::LemonSqueezy::instance();

  ls.setLicense(licenseKey);
  if (!ls.canActivate()) {
    qCritical() << "Invalid license key format:" << licenseKey;
    return EXIT_FAILURE;
  }

  int result = EXIT_FAILURE;

  QTimer timeout;
  timeout.setSingleShot(true);
  timeout.setInterval(30'000);

  QObject::connect(&ls, &Licensing::LemonSqueezy::activatedChanged, &app, [&] {
    result = ls.isActivated() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (ls.isActivated())
      qInfo() << "License activated successfully.";
    else
      qCritical() << "License activation failed.";

    app.quit();
  });

  QObject::connect(&timeout, &QTimer::timeout, &app, [&] {
    qCritical() << "License activation timed out.";
    app.quit();
  });

  QTimer::singleShot(0, &ls, &Licensing::LemonSqueezy::activate);
  timeout.start();

  app.exec();
  return result;
}

/**
 * @brief Deactivates the stored license instance on this machine and exits.
 *
 * Intended for CI environments to release the activation seat after tests complete.
 * Runs the event loop until deactivation completes or a 30-second timeout fires.
 *
 * @param app The QApplication instance.
 * @return EXIT_SUCCESS if the license was deactivated, EXIT_FAILURE otherwise.
 */
static int cliDeactivateLicense(QApplication& app)
{
  auto& ls = Licensing::LemonSqueezy::instance();

  if (!ls.isActivated()) {
    qInfo() << "License is not active on this machine; nothing to deactivate.";
    return EXIT_SUCCESS;
  }

  int result = EXIT_FAILURE;

  QTimer timeout;
  timeout.setSingleShot(true);
  timeout.setInterval(30'000);

  QObject::connect(&ls, &Licensing::LemonSqueezy::activatedChanged, &app, [&] {
    result = !ls.isActivated() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (!ls.isActivated())
      qInfo() << "License deactivated successfully.";
    else
      qCritical() << "License deactivation failed.";

    app.quit();
  });

  QObject::connect(&timeout, &QTimer::timeout, &app, [&] {
    qCritical() << "License deactivation timed out.";
    app.quit();
  });

  QTimer::singleShot(0, &ls, &Licensing::LemonSqueezy::deactivate);
  timeout.start();

  app.exec();
  return result;
}

/**
 * @brief Parses a Modbus register spec string and registers it with the Modbus driver.
 *
 * Expected format: @c type:start:count where type is one of
 * @c holding, @c input, @c coils, or @c discrete.
 *
 * @param spec The register specification string to parse.
 */
static void applyModbusRegister(const QString& spec)
{
  QStringList parts = spec.split(':');
  if (parts.size() != 3) {
    qWarning() << "Invalid register format. Expected: type:start:count";
    return;
  }

  const QString typeStr                              = parts[0].toLower();
  static const QHash<QString, quint8> kRegisterTypes = {
    { QStringLiteral("holding"), 0},
    {   QStringLiteral("input"), 1},
    {   QStringLiteral("coils"), 2},
    {QStringLiteral("discrete"), 3},
  };
  const auto it = kRegisterTypes.constFind(typeStr);
  if (it == kRegisterTypes.cend()) {
    qWarning() << "Invalid register type (holding/input/coils/discrete):" << typeStr;
    return;
  }

  const quint8 registerType = it.value();

  bool startOk        = false;
  bool countOk        = false;
  const quint16 start = parts[1].toUInt(&startOk);
  const quint16 count = parts[2].toUInt(&countOk);
  if (!startOk || !countOk || count < 1 || count > 125) {
    qWarning() << "Invalid register specification (start:0-65535, count:1-125):" << spec;
    return;
  }

  IO::ConnectionManager::instance().modbus()->addRegisterGroup(registerType, start, count);
}

/**
 * @brief Configures the selected CAN bus interface and connects.
 *
 * Validates that @p interfaceIndex is non-negative, then applies optional
 * bitrate and CAN-FD settings before calling @c connectDevice().
 *
 * @param parser            The parsed command-line arguments.
 * @param bitrateOpt        The --canbus-bitrate option descriptor.
 * @param fdOpt             The --canbus-fd option descriptor.
 * @param interfaceIndex    Index of the interface in @p availableInterfaces; negative if not found.
 * @param interfaceName     Human-readable interface name (for warnings).
 * @param plugin            Plugin name (for warnings).
 * @param availableInterfaces List of available interfaces for the selected plugin.
 */
static void configureCanbusInterface(const QCommandLineParser& parser,
                                     const QCommandLineOption& bitrateOpt,
                                     const QCommandLineOption& fdOpt,
                                     int interfaceIndex,
                                     const QString& interfaceName,
                                     const QString& plugin,
                                     const QStringList& availableInterfaces)
{
  if (interfaceIndex < 0) {
    qWarning() << "CAN interface" << interfaceName << "not found for plugin" << plugin;
    qWarning() << "Available interfaces:" << availableInterfaces.join(", ");
    return;
  }

  IO::ConnectionManager::instance().canBus()->setInterfaceIndex(interfaceIndex);

  if (parser.isSet(bitrateOpt)) {
    bool ok;
    quint32 bitrate = parser.value(bitrateOpt).toUInt(&ok);
    if (ok && bitrate > 0)
      IO::ConnectionManager::instance().canBus()->setBitrate(bitrate);
    else
      qWarning() << "Invalid CAN bus bitrate:" << parser.value(bitrateOpt);
  }

  if (parser.isSet(fdOpt))
    IO::ConnectionManager::instance().canBus()->setCanFD(true);

  IO::ConnectionManager::instance().connectDevice();
}

/**
 * @brief Applies the Modbus parity CLI option to the active Modbus driver.
 */
static void applyModbusParity(const QString& parity)
{
  static const QHash<QString, quint8> kParity = {
    { QStringLiteral("none"), 0},
    { QStringLiteral("even"), 1},
    {  QStringLiteral("odd"), 2},
    {QStringLiteral("space"), 3},
    { QStringLiteral("mark"), 4},
  };

  const auto it = kParity.constFind(parity);
  if (it == kParity.cend()) {
    qWarning() << "Invalid ModBus parity (none/even/odd/space/mark):" << parity;
    IO::ConnectionManager::instance().modbus()->setParityIndex(0);
    return;
  }

  IO::ConnectionManager::instance().modbus()->setParityIndex(it.value());
}

/**
 * @brief Applies the Modbus data-bits CLI option to the active Modbus driver.
 */
static void applyModbusDataBits(const QString& dataBits)
{
  static const QHash<QString, quint8> kDataBits = {
    {QStringLiteral("5"), 0},
    {QStringLiteral("6"), 1},
    {QStringLiteral("7"), 2},
    {QStringLiteral("8"), 3},
  };

  const auto it = kDataBits.constFind(dataBits);
  if (it == kDataBits.cend()) {
    qWarning() << "Invalid ModBus data bits (5/6/7/8):" << dataBits;
    IO::ConnectionManager::instance().modbus()->setDataBitsIndex(3);
    return;
  }

  IO::ConnectionManager::instance().modbus()->setDataBitsIndex(it.value());
}

/**
 * @brief Applies the Modbus stop-bits CLI option to the active Modbus driver.
 */
static void applyModbusStopBits(const QString& stopBits)
{
  static const QHash<QString, quint8> kStopBits = {
    {  QStringLiteral("1"), 0},
    {QStringLiteral("1.5"), 1},
    {  QStringLiteral("2"), 2},
  };

  const auto it = kStopBits.constFind(stopBits);
  if (it == kStopBits.cend()) {
    qWarning() << "Invalid ModBus stop bits (1/1.5/2):" << stopBits;
    IO::ConnectionManager::instance().modbus()->setStopBitsIndex(0);
    return;
  }

  IO::ConnectionManager::instance().modbus()->setStopBitsIndex(it.value());
}

/**
 * @brief Applies the Modbus slave CLI option to the active Modbus driver.
 */
static void applyModbusSlave(const QCommandLineParser& parser, const QCommandLineOption& opt)
{
  if (!parser.isSet(opt))
    return;

  bool ok                = false;
  unsigned int slave_val = parser.value(opt).toUInt(&ok);
  if (!ok || slave_val < 1 || slave_val > 247) {
    qWarning() << "Invalid ModBus slave address (1-247):" << parser.value(opt);
    return;
  }

  IO::ConnectionManager::instance().modbus()->setSlaveAddress(static_cast<quint8>(slave_val));
}

/**
 * @brief Applies the Modbus poll-interval CLI option to the active Modbus driver.
 */
static void applyModbusPoll(const QCommandLineParser& parser, const QCommandLineOption& opt)
{
  if (!parser.isSet(opt))
    return;

  bool ok          = false;
  quint16 interval = parser.value(opt).toUInt(&ok);
  if (!ok || interval < 50 || interval > 60000) {
    qWarning() << "Invalid ModBus poll interval (50-60000 ms):" << parser.value(opt);
    return;
  }

  IO::ConnectionManager::instance().modbus()->setPollInterval(interval);
}

/**
 * @brief Applies the Modbus serial baud-rate CLI option to the active Modbus driver.
 */
static void applyModbusBaud(const QCommandLineParser& parser, const QCommandLineOption& opt)
{
  if (!parser.isSet(opt))
    return;

  bool ok         = false;
  qint32 baudRate = parser.value(opt).toInt(&ok);
  if (!ok) {
    qWarning() << "Invalid ModBus baud rate:" << parser.value(opt);
    return;
  }

  IO::ConnectionManager::instance().modbus()->setBaudRate(baudRate);
}

/**
 * @brief Applies all --modbus-register specs and warns if none are present.
 */
static void applyModbusRegisters(const QCommandLineParser& parser, const QCommandLineOption& opt)
{
  IO::ConnectionManager::instance().modbus()->clearRegisterGroups();
  if (!parser.isSet(opt)) {
    qWarning() << "No register groups specified. Use --modbus-register to add registers.";
    return;
  }

  const QStringList registerSpecs = parser.values(opt);
  for (const QString& spec : std::as_const(registerSpecs))
    applyModbusRegister(spec);
}

/**
 * @brief Configures and connects a Modbus RTU bus from CLI options.
 */
static void setupModbusRtuConnection(const QCommandLineParser& parser,
                                     const QCommandLineOption& portOpt,
                                     const ModbusCliOptions& opts)
{
  const QString portPath = parser.value(portOpt);
  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::ModBus);
  IO::ConnectionManager::instance().modbus()->setProtocolIndex(0);

  const QStringList ports = IO::ConnectionManager::instance().modbus()->serialPortList();
  const int portIndex     = ports.indexOf(portPath);
  if (portIndex < 0) {
    qWarning() << "ModBus serial port not found:" << portPath;
    qWarning() << "Available ports:" << ports.join(", ");
    return;
  }

  IO::ConnectionManager::instance().modbus()->setSerialPortIndex(portIndex);
  applyModbusSlave(parser, opts.slaveOpt);
  applyModbusPoll(parser, opts.pollOpt);
  applyModbusBaud(parser, opts.baudOpt);

  if (parser.isSet(opts.parityOpt))
    applyModbusParity(parser.value(opts.parityOpt).toLower());

  if (parser.isSet(opts.dataBitsOpt))
    applyModbusDataBits(parser.value(opts.dataBitsOpt));

  if (parser.isSet(opts.stopBitsOpt))
    applyModbusStopBits(parser.value(opts.stopBitsOpt));

  applyModbusRegisters(parser, opts.registerOpt);
  IO::ConnectionManager::instance().connectDevice();
}

/**
 * @brief Parses a Modbus TCP @c host[:port] string into host/port components.
 */
static bool parseModbusTcpAddress(const QString& tcpAddress, QString& host, quint16& port)
{
  const QStringList parts = tcpAddress.split(':');
  if (parts.size() != 1 && parts.size() != 2)
    return false;

  host = parts[0];
  port = 502;
  if (parts.size() != 2)
    return true;

  bool ok         = false;
  const quint16 p = parts[1].toUInt(&ok);
  if (!ok || p == 0) {
    qWarning() << "Invalid ModBus TCP port:" << parts[1];
    return true;
  }

  port = p;
  return true;
}

/**
 * @brief Configures and connects a Modbus TCP bus from CLI options.
 */
static void setupModbusTcpConnection(const QCommandLineParser& parser,
                                     const QCommandLineOption& addrOpt,
                                     const ModbusCliOptions& opts)
{
  QString host;
  quint16 port = 502;
  if (!parseModbusTcpAddress(parser.value(addrOpt), host, port)) {
    qWarning() << "Invalid ModBus TCP address format. Expected: host[:port]";
    return;
  }

  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::ModBus);
  IO::ConnectionManager::instance().modbus()->setProtocolIndex(1);
  IO::ConnectionManager::instance().modbus()->setHost(host);
  IO::ConnectionManager::instance().modbus()->setPort(port);

  applyModbusSlave(parser, opts.slaveOpt);
  applyModbusPoll(parser, opts.pollOpt);
  applyModbusRegisters(parser, opts.registerOpt);
  IO::ConnectionManager::instance().connectDevice();
}

/**
 * @brief Configures and connects a CAN bus from CLI options.
 */
static void setupCanbusConnection(const QCommandLineParser& parser,
                                  const QCommandLineOption& canbusOpt,
                                  const QCommandLineOption& bitrateOpt,
                                  const QCommandLineOption& fdOpt)
{
  const QStringList parts = parser.value(canbusOpt).split(':');
  if (parts.size() != 2) {
    qWarning() << "Invalid CAN bus format. Expected: plugin:interface";
    return;
  }

  const QString plugin        = parts[0].toLower();
  const QString interfaceName = parts[1];

  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::CanBus);

  const QStringList availablePlugins = IO::ConnectionManager::instance().canBus()->pluginList();
  const int pluginIndex              = availablePlugins.indexOf(plugin);
  if (pluginIndex < 0) {
    qWarning() << "CAN plugin" << plugin << "not found";
    qWarning() << "Available plugins:" << availablePlugins.join(", ");
    return;
  }

  IO::ConnectionManager::instance().canBus()->setPluginIndex(pluginIndex);
  const QStringList availableInterfaces =
    IO::ConnectionManager::instance().canBus()->interfaceList();
  const int interfaceIndex = availableInterfaces.indexOf(interfaceName);

  configureCanbusInterface(
    parser, bitrateOpt, fdOpt, interfaceIndex, interfaceName, plugin, availableInterfaces);
}
#endif

/**
 * @brief Configures and connects the UART bus from CLI options.
 */
static void setupUartConnection(const QCommandLineParser& parser,
                                const QCommandLineOption& uartOpt,
                                const QCommandLineOption& baudOpt)
{
  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::UART);

  if (parser.isSet(uartOpt)) {
    const QString device = parser.value(uartOpt);
    IO::ConnectionManager::instance().uart()->registerDevice(device);
  }

  if (parser.isSet(baudOpt)) {
    bool ok               = false;
    const qint32 baudRate = parser.value(baudOpt).toInt(&ok);
    if (!ok)
      qWarning() << "Invalid baud rate:" << parser.value(baudOpt);
    else
      IO::ConnectionManager::instance().uart()->setBaudRate(baudRate);
  }

  IO::ConnectionManager::instance().connectDevice();
}

/**
 * @brief Configures and connects a TCP socket from a CLI host:port string.
 */
static void setupTcpConnection(const QString& tcpAddress)
{
  const QStringList parts = tcpAddress.split(':');
  if (parts.size() != 2) {
    qWarning() << "Invalid TCP address format. Expected: host:port";
    return;
  }

  bool ok            = false;
  const quint16 port = parts[1].toUInt(&ok);
  if (!ok || port == 0) {
    qWarning() << "Invalid TCP port:" << parts[1];
    return;
  }

  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::Network);
  IO::ConnectionManager::instance().network()->setTcpSocket();
  IO::ConnectionManager::instance().network()->setRemoteAddress(parts[0]);
  IO::ConnectionManager::instance().network()->setTcpPort(port);
  IO::ConnectionManager::instance().connectDevice();
}

/**
 * @brief Applies the optional UDP --udp-remote spec to the active network driver.
 */
static void applyUdpRemote(const QString& udpRemote)
{
  const QStringList parts = udpRemote.split(':');
  if (parts.size() != 2) {
    qWarning() << "Invalid UDP address format. Expected: host:port";
    return;
  }

  bool ok                  = false;
  const quint16 remotePort = parts[1].toUInt(&ok);
  if (!ok || remotePort == 0) {
    qWarning() << "Invalid UDP remote port:" << parts[1];
    return;
  }

  IO::ConnectionManager::instance().network()->setRemoteAddress(parts[0]);
  IO::ConnectionManager::instance().network()->setUdpRemotePort(remotePort);
}

/**
 * @brief Configures and connects a UDP socket from CLI options.
 */
static void setupUdpConnection(const QCommandLineParser& parser,
                               const QCommandLineOption& udpOpt,
                               const QCommandLineOption& udpRemoteOpt,
                               const QCommandLineOption& udpMltcstOpt)
{
  bool ok                 = false;
  const quint16 localPort = parser.value(udpOpt).toUInt(&ok);
  if (!ok || localPort == 0) {
    qWarning() << "Invalid UDP local port:" << parser.value(udpOpt);
    return;
  }

  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::Network);
  IO::ConnectionManager::instance().network()->setUdpSocket();
  IO::ConnectionManager::instance().network()->setUdpLocalPort(localPort);

  if (parser.isSet(udpMltcstOpt))
    IO::ConnectionManager::instance().network()->setUdpMulticast(true);

  if (parser.isSet(udpRemoteOpt))
    applyUdpRemote(parser.value(udpRemoteOpt));

  IO::ConnectionManager::instance().connectDevice();
}

//--------------------------------------------------------------------------------------------------
// Windows-specific initialization code
//--------------------------------------------------------------------------------------------------

#ifdef Q_OS_WIN
/**
 * @brief Registers the .ssproj file extension with this application in the
 *        Windows registry (per-user, HKCU).
 *
 * Creates a ProgID entry and associates the .ssproj extension with it so that
 * double-clicking a .ssproj file in Explorer launches Serial Studio with the
 * file path as the @c --project argument. This is a per-user registration that
 * does not require administrator privileges.
 */
static void registerWindowsFileAssociation()
{
  // Build the ProgID and command line
  const QString exePath = QCoreApplication::applicationFilePath().replace('/', '\\');
  const QString progId  = QStringLiteral("SerialStudio.ssproj");
  const QString openCmd =
    QStringLiteral("\"%1\" --project \"%2\"").arg(exePath, QStringLiteral("%1"));

  // Helper to write a registry key
  auto writeKey = [](const QString& path, const QString& value) {
    HKEY hKey   = nullptr;
    auto status = RegCreateKeyExW(HKEY_CURRENT_USER,
                                  reinterpret_cast<LPCWSTR>(path.utf16()),
                                  0,
                                  nullptr,
                                  0,
                                  KEY_WRITE,
                                  nullptr,
                                  &hKey,
                                  nullptr);

    if (status == ERROR_SUCCESS) {
      RegSetValueExW(hKey,
                     nullptr,
                     0,
                     REG_SZ,
                     reinterpret_cast<const BYTE*>(value.utf16()),
                     static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
      RegCloseKey(hKey);
    }
  };

  // Register ProgID with description
  const QString progIdPath = QStringLiteral("Software\\Classes\\%1").arg(progId);
  writeKey(progIdPath, QStringLiteral("Serial Studio Project"));

  // Register default icon
  writeKey(progIdPath + QStringLiteral("\\DefaultIcon"), QStringLiteral("\"%1\",0").arg(exePath));

  // Register open command
  writeKey(progIdPath + QStringLiteral("\\shell\\open\\command"), openCmd);

  // Associate .ssproj with our ProgID
  writeKey(QStringLiteral("Software\\Classes\\.ssproj"), progId);

  // Notify the shell that file associations have changed
  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
}

/**
 * @brief Pins the process to a stable Windows AppUserModelID.
 *
 * Without an explicit AUMID, Windows derives one from the executable path and
 * launch arguments -- so normal launches and shortcut launches end up under
 * different taskbar identities, and the taskbar can serve a stale cached icon
 * (often the previous shortcut's custom icon) for the next normal launch.
 * When @p shortcutPath is non-empty, a shortcut-specific AUMID is used so the
 * running window groups under that shortcut's pinned taskbar entry alongside
 * its custom icon; otherwise the default Serial Studio identity is used.
 */
static void setWindowsAppUserModelId(const QString& shortcutPath)
{
  QString aumid = QStringLiteral("AlexSpataru.SerialStudio");
  if (!shortcutPath.isEmpty())
    aumid += QStringLiteral(".Shortcut.") + shortcutIdentityHash(shortcutPath);

  SetCurrentProcessExplicitAppUserModelID(reinterpret_cast<LPCWSTR>(aumid.utf16()));
}

/**
 * @brief Opts the process out of Windows power throttling and prevents idle sleep.
 *
 * Disables EcoQoS execution-speed throttling for this process so the OS does not
 * downclock telemetry-handling threads under power-saver heuristics, and signals
 * that the system is in use to keep it from sleeping while a dashboard is live.
 * Both calls are process-scoped, require no admin rights, and are reverted by
 * Windows automatically when the process exits.
 */
static void enableWindowsPerformanceMode()
{
#  if defined(PROCESS_POWER_THROTTLING_CURRENT_VERSION)
  // Opt out of EcoQoS execution-speed throttling
  PROCESS_POWER_THROTTLING_STATE state = {};
  state.Version                        = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
  state.ControlMask                    = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
  state.StateMask                      = 0;
  SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &state, sizeof(state));
#  endif

  // Prevent system sleep while Serial Studio is running
  SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
}

/**
 * @brief Attaches the application to the parent console and redirects output
 *        streams on Windows.
 *
 * This function attaches the application to the parent process's console if it
 * was launched from the command prompt. It redirects the `stdout` and `stderr`
 * streams to the console to enable proper output display. Additionally, it
 * prints a newline to avoid overlapping text with any previous user commands
 * in the console.
 */
static void attachToConsole()
{
  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    FILE* fp = nullptr;
    (void)freopen_s(&fp, "CONOUT$", "w", stdout);
    (void)freopen_s(&fp, "CONOUT$", "w", stderr);
    printf("\n");
  }
}

/**
 * @brief Adjusts command-line arguments to enable FreeType font rendering on
 *        Windows.
 *
 * This function forces the application to use FreeType font rendering instead
 * of DirectWrite or GDI. This approach fixes pixelated fonts being shown in
 * the user interface in screens whose scale factor is 100%.
 *
 * @param argc Reference to the argument count from `main()`.
 * @param argv Array of command-line arguments from `main()`.
 */
static char** adjustArgumentsForFreeType(int& argc, char** argv)
{
  const char* platformArgument = "-platform";
  const char* platformOption   = "windows:fontengine=freetype";

  char** newArgv = static_cast<char**>(malloc(sizeof(char*) * (argc + 2)));
  if (!newArgv)
    return argv;

  for (int i = 0; i < argc; ++i)
    newArgv[i] = _strdup(argv[i]);

  newArgv[argc]     = _strdup(platformArgument);
  newArgv[argc + 1] = _strdup(platformOption);

  argc += 2;
  return newArgv;
}
#endif
