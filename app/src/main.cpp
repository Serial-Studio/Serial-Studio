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

// clang-format off
#ifdef _WIN32
#  include <windows.h>
#  include <shlobj.h>
#endif
// clang-format on

#include <cstring>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileOpenEvent>
#include <QLoggingCategory>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSettings>
#include <QStyleFactory>
#include <QSysInfo>

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
#include "Misc/ModuleManager.h"
#include "Misc/TimerEvents.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include <QTimer>

#  include "Licensing/LemonSqueezy.h"
#endif

//--------------------------------------------------------------------------------------------------
// Declare utility functions
//--------------------------------------------------------------------------------------------------

static void cliShowVersion();
static void cliResetSettings();
static bool argvHasFlag(int argc, char** argv, const char* flag);
static char** injectPlatformArg(int& argc, char** argv, const char* platform);

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
#endif

#ifdef Q_OS_WINDOWS
static void attachToConsole();
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

#if defined(Q_OS_WIN)
  // Windows-specific fixes, attach to console and allow fonts to look decent on 1x scaling
  attachToConsole();
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
  QIcon appIcon(QStringLiteral(":/rcc/logo/icon.svg"));
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

  // Initialize the module manager & singleton classes
  Misc::ModuleManager moduleManager;
  moduleManager.setHeadless(headless);
  moduleManager.configureUpdater();
  moduleManager.registerQmlTypes();
  moduleManager.initializeQmlInterface();
  if (!headless && moduleManager.engine().rootObjects().isEmpty()) {
    qCritical() << "Critical QML error";
    return EXIT_FAILURE;
  }

  // Apply CLI operation mode and connection settings
  if (parser.isSet(apiServerOpt))
    API::Server::instance().setEnabled(true);

  // Load a project
  if (parser.isSet(pOpt)) {
    QString projectPath = parser.value(pOpt);
    AppState::instance().setOperationMode(SerialStudio::ProjectFile);
    DataModel::ProjectModel::instance().openJsonFile(projectPath);
  }

  // Enable Quick Plot Mode
  else if (parser.isSet(qOpt))
    AppState::instance().setOperationMode(SerialStudio::QuickPlot);

  // Start full screen
  const auto ctx = moduleManager.engine().rootContext();
  ctx->setContextProperty("CLI_START_FULLSCREEN", parser.isSet(fOpt));

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

  // Set IO driver to UART
  if (parser.isSet(uartOpt) || parser.isSet(baudOpt)) {
    IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::UART);

    if (parser.isSet(uartOpt)) {
      QString device = parser.value(uartOpt);
      IO::ConnectionManager::instance().uart()->registerDevice(device);
    }

    if (parser.isSet(baudOpt)) {
      bool ok;
      qint32 baudRate = parser.value(baudOpt).toInt(&ok);
      if (ok)
        IO::ConnectionManager::instance().uart()->setBaudRate(baudRate);
      else
        qWarning() << "Invalid baud rate:" << parser.value(baudOpt);
    }

    IO::ConnectionManager::instance().connectDevice();
  }

  // Set IO driver to TCP socket
  else if (parser.isSet(tcpOpt)) {
    QString tcpAddress = parser.value(tcpOpt);
    QStringList parts  = tcpAddress.split(':');

    if (parts.size() == 2) {
      bool ok;
      quint16 port = parts[1].toUInt(&ok);

      if (ok && port > 0) {
        IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::Network);
        IO::ConnectionManager::instance().network()->setTcpSocket();
        IO::ConnectionManager::instance().network()->setRemoteAddress(parts[0]);
        IO::ConnectionManager::instance().network()->setTcpPort(port);
        IO::ConnectionManager::instance().connectDevice();
      }

      else
        qWarning() << "Invalid TCP port:" << parts[1];
    }

    else
      qWarning() << "Invalid TCP address format. Expected: host:port";
  }

  // Set IO driver to UDP socket
  else if (parser.isSet(udpOpt)) {
    bool ok;
    quint16 localPort = parser.value(udpOpt).toUInt(&ok);

    if (ok && localPort > 0) {
      IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::Network);
      IO::ConnectionManager::instance().network()->setUdpSocket();
      IO::ConnectionManager::instance().network()->setUdpLocalPort(localPort);

      if (parser.isSet(udpMltcstOpt))
        IO::ConnectionManager::instance().network()->setUdpMulticast(true);

      auto applyUdpRemote = [](const QString& udpRemote) {
        QStringList parts = udpRemote.split(':');
        if (parts.size() != 2) {
          qWarning() << "Invalid UDP address format. Expected: host:port";
          return;
        }
        bool portOk;
        quint16 remotePort = parts[1].toUInt(&portOk);
        if (portOk && remotePort > 0) {
          IO::ConnectionManager::instance().network()->setRemoteAddress(parts[0]);
          IO::ConnectionManager::instance().network()->setUdpRemotePort(remotePort);
        }

        else
          qWarning() << "Invalid UDP remote port:" << parts[1];
      };

      if (parser.isSet(udpRemoteOpt))
        applyUdpRemote(parser.value(udpRemoteOpt));

      IO::ConnectionManager::instance().connectDevice();
    }

    else
      qWarning() << "Invalid UDP local port:" << parser.value(udpOpt);
  }
#ifdef BUILD_COMMERCIAL
  // Set IO driver to Modbus RTU
  else if (parser.isSet(modbusRtuOpt)) {
    QString portPath = parser.value(modbusRtuOpt);
    IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::ModBus);
    IO::ConnectionManager::instance().modbus()->setProtocolIndex(0);

    QStringList ports = IO::ConnectionManager::instance().modbus()->serialPortList();
    int portIndex     = ports.indexOf(portPath);

    if (portIndex >= 0) {
      IO::ConnectionManager::instance().modbus()->setSerialPortIndex(portIndex);

      if (parser.isSet(modbusSlaveOpt)) {
        bool ok;
        unsigned int slave_val = parser.value(modbusSlaveOpt).toUInt(&ok);
        if (ok && slave_val >= 1 && slave_val <= 247)
          IO::ConnectionManager::instance().modbus()->setSlaveAddress(
            static_cast<quint8>(slave_val));
        else
          qWarning() << "Invalid ModBus slave address (1-247):" << parser.value(modbusSlaveOpt);
      }

      if (parser.isSet(modbusPollOpt)) {
        bool ok;
        quint16 interval = parser.value(modbusPollOpt).toUInt(&ok);
        if (ok && interval >= 50 && interval <= 60000)
          IO::ConnectionManager::instance().modbus()->setPollInterval(interval);
        else
          qWarning() << "Invalid ModBus poll interval (50-60000 ms):"
                     << parser.value(modbusPollOpt);
      }

      if (parser.isSet(modbusBaudOpt)) {
        bool ok;
        qint32 baudRate = parser.value(modbusBaudOpt).toInt(&ok);
        if (ok)
          IO::ConnectionManager::instance().modbus()->setBaudRate(baudRate);
        else
          qWarning() << "Invalid ModBus baud rate:" << parser.value(modbusBaudOpt);
      }

      if (parser.isSet(modbusParityOpt)) {
        QString parity     = parser.value(modbusParityOpt).toLower();
        quint8 parityIndex = 0;

        if (parity == "none")
          parityIndex = 0;
        else if (parity == "even")
          parityIndex = 1;
        else if (parity == "odd")
          parityIndex = 2;
        else if (parity == "space")
          parityIndex = 3;
        else if (parity == "mark")
          parityIndex = 4;
        else {
          qWarning() << "Invalid ModBus parity (none/even/odd/space/mark):" << parity;
          parityIndex = 0;
        }

        IO::ConnectionManager::instance().modbus()->setParityIndex(parityIndex);
      }

      if (parser.isSet(modbusDataBitsOpt)) {
        QString dataBits     = parser.value(modbusDataBitsOpt);
        quint8 dataBitsIndex = 3;

        if (dataBits == "5")
          dataBitsIndex = 0;
        else if (dataBits == "6")
          dataBitsIndex = 1;
        else if (dataBits == "7")
          dataBitsIndex = 2;
        else if (dataBits == "8")
          dataBitsIndex = 3;
        else {
          qWarning() << "Invalid ModBus data bits (5/6/7/8):" << dataBits;
          dataBitsIndex = 3;
        }

        IO::ConnectionManager::instance().modbus()->setDataBitsIndex(dataBitsIndex);
      }

      if (parser.isSet(modbusStopBitsOpt)) {
        QString stopBits     = parser.value(modbusStopBitsOpt);
        quint8 stopBitsIndex = 0;

        if (stopBits == "1")
          stopBitsIndex = 0;
        else if (stopBits == "1.5")
          stopBitsIndex = 1;
        else if (stopBits == "2")
          stopBitsIndex = 2;
        else {
          qWarning() << "Invalid ModBus stop bits (1/1.5/2):" << stopBits;
          stopBitsIndex = 0;
        }

        IO::ConnectionManager::instance().modbus()->setStopBitsIndex(stopBitsIndex);
      }

      IO::ConnectionManager::instance().modbus()->clearRegisterGroups();

      if (parser.isSet(modbusRegisterOpt)) {
        QStringList registerSpecs = parser.values(modbusRegisterOpt);
        for (const QString& spec : std::as_const(registerSpecs))
          applyModbusRegister(spec);
      }

      else {
        qWarning()
          << "Warning: No register groups specified. Use --modbus-register to add registers.";
      }

      IO::ConnectionManager::instance().connectDevice();
    }

    else {
      qWarning() << "ModBus serial port not found:" << portPath;
      qWarning() << "Available ports:" << ports.join(", ");
    }
  }

  // Set IO driver to Modbus TCP
  else if (parser.isSet(modbusTcpOpt)) {
    QString tcpAddress = parser.value(modbusTcpOpt);
    QStringList parts  = tcpAddress.split(':');

    if (parts.size() == 1 || parts.size() == 2) {
      QString host = parts[0];
      quint16 port = 502;

      if (parts.size() == 2) {
        bool ok;
        port = parts[1].toUInt(&ok);
        if (!ok || port == 0) {
          qWarning() << "Invalid ModBus TCP port:" << parts[1];
          port = 502;
        }
      }

      IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::ModBus);
      IO::ConnectionManager::instance().modbus()->setProtocolIndex(1);
      IO::ConnectionManager::instance().modbus()->setHost(host);
      IO::ConnectionManager::instance().modbus()->setPort(port);

      if (parser.isSet(modbusSlaveOpt)) {
        bool ok;
        unsigned int slave_val = parser.value(modbusSlaveOpt).toUInt(&ok);
        if (ok && slave_val >= 1 && slave_val <= 247)
          IO::ConnectionManager::instance().modbus()->setSlaveAddress(
            static_cast<quint8>(slave_val));
        else
          qWarning() << "Invalid ModBus slave address (1-247):" << parser.value(modbusSlaveOpt);
      }

      if (parser.isSet(modbusPollOpt)) {
        bool ok;
        quint16 interval = parser.value(modbusPollOpt).toUInt(&ok);
        if (ok && interval >= 50 && interval <= 60000)
          IO::ConnectionManager::instance().modbus()->setPollInterval(interval);
        else
          qWarning() << "Invalid ModBus poll interval (50-60000 ms):"
                     << parser.value(modbusPollOpt);
      }

      IO::ConnectionManager::instance().modbus()->clearRegisterGroups();
      if (parser.isSet(modbusRegisterOpt)) {
        QStringList registerSpecs = parser.values(modbusRegisterOpt);
        for (const QString& spec : std::as_const(registerSpecs))
          applyModbusRegister(spec);
      }

      else {
        qWarning() << "No register groups specified. Use --modbus-register to add registers.";
      }

      IO::ConnectionManager::instance().connectDevice();
    }

    else
      qWarning() << "Invalid ModBus TCP address format. Expected: host[:port]";
  }

  // Set IO driver to CanBus
  else if (parser.isSet(canbusOpt)) {
    QString canbusSpec = parser.value(canbusOpt);
    QStringList parts  = canbusSpec.split(':');

    if (parts.size() == 2) {
      QString plugin        = parts[0].toLower();
      QString interfaceName = parts[1];

      IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::CanBus);

      QStringList availablePlugins = IO::ConnectionManager::instance().canBus()->pluginList();
      int pluginIndex              = availablePlugins.indexOf(plugin);

      if (pluginIndex >= 0) {
        IO::ConnectionManager::instance().canBus()->setPluginIndex(pluginIndex);

        QStringList availableInterfaces =
          IO::ConnectionManager::instance().canBus()->interfaceList();
        int interfaceIndex = availableInterfaces.indexOf(interfaceName);

        configureCanbusInterface(parser,
                                 canbusBitrateOpt,
                                 canbusFdOpt,
                                 interfaceIndex,
                                 interfaceName,
                                 plugin,
                                 availableInterfaces);
      }

      else {
        qWarning() << "CAN plugin" << plugin << "not found";
        qWarning() << "Available plugins:" << availablePlugins.join(", ");
      }
    }

    else
      qWarning() << "Invalid CAN bus format. Expected: plugin:interface";
  }
#endif

  const auto status = app.exec();

#ifdef Q_OS_WIN
  for (int i = 0; i < argc; ++i)
    free(argv[i]);

  free(argv);
#endif

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

  // const_cast is safe: Qt copies argv strings during QApplication init
  newArgv[1] = const_cast<char*>("-platform");
  newArgv[2] = const_cast<char*>(platform);

  for (int i = 1; i < argc; ++i)
    newArgv[i + 2] = argv[i];

  newArgv[argc + 2] = nullptr;
  argc += 2;
  return newArgv;
}

/**
 * Prints the current application version to the console
 */
static void cliShowVersion()
{
  qDebug() << APP_NAME << "version" << APP_VERSION;
  qDebug() << "Written by Alex Spataru <https://github.com/alex-spataru>";
}

/**
 * Removes all application settings
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

  const QString typeStr = parts[0].toLower();
  quint8 registerType   = 0;

  if (typeStr == "holding")
    registerType = 0;
  else if (typeStr == "input")
    registerType = 1;
  else if (typeStr == "coils")
    registerType = 2;
  else if (typeStr == "discrete")
    registerType = 3;
  else {
    qWarning() << "Invalid register type (holding/input/coils/discrete):" << typeStr;
    return;
  }

  bool startOk, countOk;
  quint16 start = parts[1].toUInt(&startOk);
  quint16 count = parts[2].toUInt(&countOk);

  if (startOk && countOk && count >= 1 && count <= 125)
    IO::ConnectionManager::instance().modbus()->addRegisterGroup(registerType, start, count);
  else
    qWarning() << "Invalid register specification (start:0-65535, count:1-125):" << spec;
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
#endif

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
