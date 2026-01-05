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

#include <QSysInfo>
#include <QSettings>
#include <QQmlContext>
#include <QQuickStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QCommandLineParser>

#include "AppInfo.h"
#include "IO/Manager.h"
#include "UI/Dashboard.h"
#include "IO/Drivers/UART.h"
#include "Misc/TimerEvents.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/FrameBuilder.h"
#include "IO/Drivers/Network.h"
#include "Misc/ModuleManager.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/CANBus.h"
#endif

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <cstring>
#endif

#ifdef Q_OS_LINUX
#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QStandardPaths>
#endif

//------------------------------------------------------------------------------
// Declare utility functions
//------------------------------------------------------------------------------

static void cliShowVersion();
static void cliResetSettings();

#ifdef Q_OS_WINDOWS
static void attachToConsole();
static char **adjustArgumentsForFreeType(int &argc, char **argv);
#endif

//------------------------------------------------------------------------------
// Entry-point function
//------------------------------------------------------------------------------

/**
 * @brief Entry-point function of the application
 *
 * @param argc argument count
 * @param argv argument data
 *
 * @return qApp exit code
 */
int main(int argc, char **argv)
{
  // Set application info
  QApplication::setApplicationName(APP_EXECUTABLE);
  QApplication::setOrganizationName(APP_DEVELOPER);
  QApplication::setApplicationVersion(APP_VERSION);
  QApplication::setApplicationDisplayName(APP_NAME);
  QApplication::setOrganizationDomain(APP_SUPPORT_URL);

  // Disable native menubar
  QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

  // Windows specific initialization code
#if defined(Q_OS_WIN)
  attachToConsole();
  argv = adjustArgumentsForFreeType(argc, argv);
#endif

  // Avoid 200% scaling on 150% scaling...
  auto policy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;
  QApplication::setHighDpiScaleFactorRoundingPolicy(policy);

  // Initialize application
  QApplication app(argc, argv);

  // Set window icon, skip on macOS as AppBundle already does this for us
#if !defined(Q_OS_MAC)
  QIcon appIcon(QStringLiteral(":/rcc/logo/icon.svg"));
  if (!appIcon.isNull())
    app.setWindowIcon(appIcon);
#endif

  // Set application style
  app.setStyle(QStyleFactory::create("Fusion"));
  QQuickStyle::setStyle("Fusion");

  // Define CLI options
  // clang-format off
  typedef QCommandLineOption QCLO;
  // General options
  QCLO vOpt({"v", "version"}, "Displays application version");
  QCLO rOpt({"r", "reset"}, "Resets all application settings");
  QCLO fOpt({"f", "fullscreen"}, "Launches dashboard in fullscreen mode");
  QCLO pOpt({"p", "project"}, "Loads the specified project file", "file");
  QCLO qOpt({"q", "quick-plot"}, "Enables quick plot mode (auto-detect CSV data)");
  QCLO jOpt({"j", "device-sends-json"}, "Expects pre-formatted JSON from device");

  // Dashboard options
  QCLO fpsOpt({"t", "fps"}, "Sets visualization refresh rate", "Hz");
  QCLO pointsOpt({"n", "points"}, "Sets data points per plot", "count");

  // UART/Serial port options
  QCLO uartOpt("uart", "Specifies serial port (e.g., /dev/ttyUSB0, COM3)", "port");
  QCLO baudOpt("baud", "Sets serial baud rate (default: 9600)", "rate");

  // TCP client options
  QCLO tcpOpt("tcp", "Connects to TCP server (e.g., 192.168.1.100:8080)", "host:port");

  // UDP options
  QCLO udpOpt("udp", "Binds to UDP local port (e.g., 8080)", "port");
  QCLO udpRemoteOpt("udp-remote", "Specifies UDP remote target (e.g., 192.168.1.100:8080)", "host:port");
  QCLO udpMltcstOpt("udp-multicast", "Enables multicast mode for UDP");

#ifdef BUILD_COMMERCIAL
  // ModBus RTU options
  QCLO modbusRtuOpt("modbus-rtu", "Connects to ModBus RTU device (e.g., /dev/ttyUSB0, COM3)", "port");
  QCLO modbusTcpOpt("modbus-tcp", "Connects to ModBus TCP server (e.g., 192.168.1.100:502)", "host:port");
  QCLO modbusSlaveOpt("modbus-slave", "Sets ModBus slave address (1-247, default: 1)", "address");
  QCLO modbusPollOpt("modbus-poll", "Sets ModBus poll interval in ms (50-60000, default: 100)", "interval");
  QCLO modbusBaudOpt("modbus-baud", "Sets ModBus RTU baud rate (default: 9600)", "rate");
  QCLO modbusParityOpt("modbus-parity", "Sets ModBus RTU parity (none/even/odd/space/mark, default: none)", "type");
  QCLO modbusDataBitsOpt("modbus-databits", "Sets ModBus RTU data bits (5/6/7/8, default: 8)", "bits");
  QCLO modbusStopBitsOpt("modbus-stopbits", "Sets ModBus RTU stop bits (1/1.5/2, default: 1)", "bits");
  QCLO modbusRegisterOpt("modbus-register", "Adds ModBus register group: type:start:count (repeatable)", "spec");

  // CANBus options
  QCLO canbusOpt("canbus", "Connects to CAN bus (e.g., socketcan:can0, peakcan:pcan0)", "plugin:interface");
  QCLO canbusBitrateOpt("canbus-bitrate", "Sets CAN bus bitrate in bps (default: 500000)", "rate");
  QCLO canbusFdOpt("canbus-fd", "Enables CAN-FD mode");
#endif
  // clang-format on

  // Setup command line parser
  QCommandLineParser parser;
  parser.setApplicationDescription(PROJECT_DESCRIPTION_SUMMARY);
  parser.addHelpOption();
  parser.addOption(vOpt);
  parser.addOption(rOpt);
  parser.addOption(fOpt);
  parser.addOption(pOpt);
  parser.addOption(qOpt);
  parser.addOption(jOpt);
  parser.addOption(fpsOpt);
  parser.addOption(pointsOpt);
  parser.addOption(uartOpt);
  parser.addOption(baudOpt);
  parser.addOption(tcpOpt);
  parser.addOption(udpOpt);
  parser.addOption(udpRemoteOpt);
  parser.addOption(udpMltcstOpt);
#ifdef BUILD_COMMERCIAL
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
  parser.process(app);

  // Handle version option
  if (parser.isSet(vOpt))
  {
    cliShowVersion();
    return EXIT_SUCCESS;
  }

  // Handle reset option
  if (parser.isSet(rOpt))
  {
    cliResetSettings();
    return EXIT_SUCCESS;
  }

  // Ensure resources are loaded
  Q_INIT_RESOURCE(rcc);
  Q_INIT_RESOURCE(translations);

  // Create module manager
  Misc::ModuleManager moduleManager;
  moduleManager.configureUpdater();

  // Initialize QML interface
  moduleManager.registerQmlTypes();
  moduleManager.initializeQmlInterface();
  if (moduleManager.engine().rootObjects().isEmpty())
  {
    qCritical() << "Critical QML error";
    return EXIT_FAILURE;
  }

  // Handle project file option
  if (parser.isSet(pOpt))
  {
    QString projectPath = parser.value(pOpt);
    DataModel::ProjectModel::instance().openJsonFile(projectPath);
    DataModel::FrameBuilder::instance().setOperationMode(
        SerialStudio::ProjectFile);
  }

  // Enable quick plot mode
  else if (parser.isSet(qOpt))
    DataModel::FrameBuilder::instance().setOperationMode(
        SerialStudio::QuickPlot);

  // Disable frame processing
  else if (parser.isSet(jOpt))
    DataModel::FrameBuilder::instance().setOperationMode(
        SerialStudio::DeviceSendsJSON);

  // Handle fullscreen option
  const auto ctx = moduleManager.engine().rootContext();
  ctx->setContextProperty("CLI_START_FULLSCREEN", parser.isSet(fOpt));

  // Set FPS
  if (parser.isSet(fpsOpt))
  {
    bool ok;
    auto fps = parser.value(fpsOpt).toUInt(&ok);
    if (ok)
      Misc::TimerEvents::instance().setFPS(fps);
  }

  // Set plot point count
  if (parser.isSet(pointsOpt))
  {
    bool ok;
    auto points = parser.value(pointsOpt).toUInt(&ok);
    if (ok)
      UI::Dashboard::instance().setPoints(points);
  }

  // Handle UART device and baud rate options
  if (parser.isSet(uartOpt) || parser.isSet(baudOpt))
  {
    // Set bus type to UART
    IO::Manager::instance().setBusType(SerialStudio::BusType::UART);

    // Set device if provided
    if (parser.isSet(uartOpt))
    {
      QString device = parser.value(uartOpt);
      IO::Drivers::UART::instance().registerDevice(device);
    }

    // Set baud rate if provided
    if (parser.isSet(baudOpt))
    {
      bool ok;
      qint32 baudRate = parser.value(baudOpt).toInt(&ok);
      if (ok)
        IO::Drivers::UART::instance().setBaudRate(baudRate);
      else
        qWarning() << "Invalid baud rate:" << parser.value(baudOpt);
    }

    // Connect to device
    IO::Manager::instance().connectDevice();
  }

  // Handle TCP connection option
  else if (parser.isSet(tcpOpt))
  {
    QString tcpAddress = parser.value(tcpOpt);
    QStringList parts = tcpAddress.split(':');

    if (parts.size() == 2)
    {
      bool ok;
      quint16 port = parts[1].toUInt(&ok);

      if (ok && port > 0)
      {
        IO::Manager::instance().setBusType(SerialStudio::BusType::Network);
        IO::Drivers::Network::instance().setTcpSocket();
        IO::Drivers::Network::instance().setRemoteAddress(parts[0]);
        IO::Drivers::Network::instance().setTcpPort(port);
        IO::Manager::instance().connectDevice();
      }

      else
        qWarning() << "Invalid TCP port:" << parts[1];
    }

    else
      qWarning() << "Invalid TCP address format. Expected: host:port";
  }

  // Handle UDP connection options
  else if (parser.isSet(udpOpt))
  {
    bool ok;
    quint16 localPort = parser.value(udpOpt).toUInt(&ok);

    if (ok && localPort > 0)
    {
      IO::Manager::instance().setBusType(SerialStudio::BusType::Network);
      IO::Drivers::Network::instance().setUdpSocket();
      IO::Drivers::Network::instance().setUdpLocalPort(localPort);

      if (parser.isSet(udpMltcstOpt))
        IO::Drivers::Network::instance().setUdpMulticast(true);

      if (parser.isSet(udpRemoteOpt))
      {
        QString udpRemote = parser.value(udpRemoteOpt);
        QStringList parts = udpRemote.split(':');

        if (parts.size() == 2)
        {
          bool portOk;
          quint16 remotePort = parts[1].toUInt(&portOk);

          if (portOk && remotePort > 0)
          {
            IO::Drivers::Network::instance().setRemoteAddress(parts[0]);
            IO::Drivers::Network::instance().setUdpRemotePort(remotePort);
          }

          else
            qWarning() << "Invalid UDP remote port:" << parts[1];
        }

        else
          qWarning() << "Invalid UDP address format. Expected: host:port";
      }

      IO::Manager::instance().connectDevice();
    }

    else
      qWarning() << "Invalid UDP local port:" << parser.value(udpOpt);
  }

#ifdef BUILD_COMMERCIAL
  // Handle ModBus RTU connection options
  else if (parser.isSet(modbusRtuOpt))
  {
    QString portPath = parser.value(modbusRtuOpt);
    IO::Manager::instance().setBusType(SerialStudio::BusType::ModBus);
    IO::Drivers::Modbus::instance().setProtocolIndex(0);

    // Find serial port index
    QStringList ports = IO::Drivers::Modbus::instance().serialPortList();
    int portIndex = ports.indexOf(portPath);

    if (portIndex >= 0)
    {
      IO::Drivers::Modbus::instance().setSerialPortIndex(portIndex);

      // Set slave address if provided
      if (parser.isSet(modbusSlaveOpt))
      {
        bool ok;
        quint8 slave = parser.value(modbusSlaveOpt).toUInt(&ok);
        if (ok && slave >= 1 && slave <= 247)
          IO::Drivers::Modbus::instance().setSlaveAddress(slave);
        else
          qWarning() << "Invalid ModBus slave address (1-247):"
                     << parser.value(modbusSlaveOpt);
      }

      // Set poll interval if provided
      if (parser.isSet(modbusPollOpt))
      {
        bool ok;
        quint16 interval = parser.value(modbusPollOpt).toUInt(&ok);
        if (ok && interval >= 50 && interval <= 60000)
          IO::Drivers::Modbus::instance().setPollInterval(interval);
        else
          qWarning() << "Invalid ModBus poll interval (50-60000 ms):"
                     << parser.value(modbusPollOpt);
      }

      // Set baud rate if provided
      if (parser.isSet(modbusBaudOpt))
      {
        bool ok;
        qint32 baudRate = parser.value(modbusBaudOpt).toInt(&ok);
        if (ok)
          IO::Drivers::Modbus::instance().setBaudRate(baudRate);
        else
          qWarning() << "Invalid ModBus baud rate:"
                     << parser.value(modbusBaudOpt);
      }

      // Set parity if provided
      if (parser.isSet(modbusParityOpt))
      {
        QString parity = parser.value(modbusParityOpt).toLower();
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
        else
        {
          qWarning() << "Invalid ModBus parity (none/even/odd/space/mark):"
                     << parity;
          parityIndex = 0;
        }

        IO::Drivers::Modbus::instance().setParityIndex(parityIndex);
      }

      // Set data bits if provided
      if (parser.isSet(modbusDataBitsOpt))
      {
        QString dataBits = parser.value(modbusDataBitsOpt);
        quint8 dataBitsIndex = 3;

        if (dataBits == "5")
          dataBitsIndex = 0;
        else if (dataBits == "6")
          dataBitsIndex = 1;
        else if (dataBits == "7")
          dataBitsIndex = 2;
        else if (dataBits == "8")
          dataBitsIndex = 3;
        else
        {
          qWarning() << "Invalid ModBus data bits (5/6/7/8):" << dataBits;
          dataBitsIndex = 3;
        }

        IO::Drivers::Modbus::instance().setDataBitsIndex(dataBitsIndex);
      }

      // Set stop bits if provided
      if (parser.isSet(modbusStopBitsOpt))
      {
        QString stopBits = parser.value(modbusStopBitsOpt);
        quint8 stopBitsIndex = 0;

        if (stopBits == "1")
          stopBitsIndex = 0;
        else if (stopBits == "1.5")
          stopBitsIndex = 1;
        else if (stopBits == "2")
          stopBitsIndex = 2;
        else
        {
          qWarning() << "Invalid ModBus stop bits (1/1.5/2):" << stopBits;
          stopBitsIndex = 0;
        }

        IO::Drivers::Modbus::instance().setStopBitsIndex(stopBitsIndex);
      }

      // Clear and add register groups
      IO::Drivers::Modbus::instance().clearRegisterGroups();

      if (parser.isSet(modbusRegisterOpt))
      {
        QStringList registerSpecs = parser.values(modbusRegisterOpt);
        for (const QString &spec : std::as_const(registerSpecs))
        {
          QStringList parts = spec.split(':');
          if (parts.size() == 3)
          {
            QString typeStr = parts[0].toLower();
            quint8 registerType = 0;

            if (typeStr == "holding")
              registerType = 0;
            else if (typeStr == "input")
              registerType = 1;
            else if (typeStr == "coils")
              registerType = 2;
            else if (typeStr == "discrete")
              registerType = 3;
            else
            {
              qWarning() << "Invalid register type "
                            "(holding/input/coils/discrete):"
                         << typeStr;
              continue;
            }

            bool startOk, countOk;
            quint16 start = parts[1].toUInt(&startOk);
            quint16 count = parts[2].toUInt(&countOk);

            if (startOk && countOk && count >= 1 && count <= 125)
              IO::Drivers::Modbus::instance().addRegisterGroup(registerType,
                                                               start, count);
            else
              qWarning() << "Invalid register specification (start:0-65535, "
                            "count:1-125):"
                         << spec;
          }

          else
            qWarning() << "Invalid register format. Expected: type:start:count";
        }
      }

      else
      {
        qWarning() << "Warning: No register groups specified. Use "
                      "--modbus-register to add registers.";
      }

      IO::Manager::instance().connectDevice();
    }

    else
    {
      qWarning() << "ModBus serial port not found:" << portPath;
      qWarning() << "Available ports:" << ports.join(", ");
    }
  }

  // Handle ModBus TCP connection options
  else if (parser.isSet(modbusTcpOpt))
  {
    QString tcpAddress = parser.value(modbusTcpOpt);
    QStringList parts = tcpAddress.split(':');

    if (parts.size() == 1 || parts.size() == 2)
    {
      QString host = parts[0];
      quint16 port = 502;

      if (parts.size() == 2)
      {
        bool ok;
        port = parts[1].toUInt(&ok);
        if (!ok || port == 0)
        {
          qWarning() << "Invalid ModBus TCP port:" << parts[1];
          port = 502;
        }
      }

      IO::Manager::instance().setBusType(SerialStudio::BusType::ModBus);
      IO::Drivers::Modbus::instance().setProtocolIndex(1);
      IO::Drivers::Modbus::instance().setHost(host);
      IO::Drivers::Modbus::instance().setPort(port);

      // Set slave address if provided
      if (parser.isSet(modbusSlaveOpt))
      {
        bool ok;
        quint8 slave = parser.value(modbusSlaveOpt).toUInt(&ok);
        if (ok && slave >= 1 && slave <= 247)
          IO::Drivers::Modbus::instance().setSlaveAddress(slave);
        else
          qWarning() << "Invalid ModBus slave address (1-247):"
                     << parser.value(modbusSlaveOpt);
      }

      // Set poll interval if provided
      if (parser.isSet(modbusPollOpt))
      {
        bool ok;
        quint16 interval = parser.value(modbusPollOpt).toUInt(&ok);
        if (ok && interval >= 50 && interval <= 60000)
          IO::Drivers::Modbus::instance().setPollInterval(interval);
        else
          qWarning() << "Invalid ModBus poll interval (50-60000 ms):"
                     << parser.value(modbusPollOpt);
      }

      // Clear and add register groups
      IO::Drivers::Modbus::instance().clearRegisterGroups();
      if (parser.isSet(modbusRegisterOpt))
      {
        QStringList registerSpecs = parser.values(modbusRegisterOpt);
        for (const QString &spec : std::as_const(registerSpecs))
        {
          QStringList regParts = spec.split(':');
          if (regParts.size() == 3)
          {
            QString typeStr = regParts[0].toLower();
            quint8 registerType = 0;

            if (typeStr == "holding")
              registerType = 0;
            else if (typeStr == "input")
              registerType = 1;
            else if (typeStr == "coils")
              registerType = 2;
            else if (typeStr == "discrete")
              registerType = 3;
            else
            {
              qWarning() << "Invalid register type "
                            "(holding/input/coils/discrete):"
                         << typeStr;
              continue;
            }

            bool startOk, countOk;
            quint16 start = regParts[1].toUInt(&startOk);
            quint16 count = regParts[2].toUInt(&countOk);

            if (startOk && countOk && count >= 1 && count <= 125)
              IO::Drivers::Modbus::instance().addRegisterGroup(registerType,
                                                               start, count);
            else
              qWarning() << "Invalid register specification (start:0-65535, "
                            "count:1-125):"
                         << spec;
          }

          else
            qWarning() << "Invalid register format. Expected: type:start:count";
        }
      }

      else
      {
        qWarning() << "Warning: No register groups specified. Use "
                      "--modbus-register to add registers.";
      }

      IO::Manager::instance().connectDevice();
    }

    else
      qWarning() << "Invalid ModBus TCP address format. Expected: host[:port]";
  }

  // Handle CANBus connection options
  else if (parser.isSet(canbusOpt))
  {
    QString canbusSpec = parser.value(canbusOpt);
    QStringList parts = canbusSpec.split(':');

    if (parts.size() == 2)
    {
      QString plugin = parts[0].toLower();
      QString interfaceName = parts[1];

      IO::Manager::instance().setBusType(SerialStudio::BusType::CanBus);

      // Get available plugins and find the requested one
      QStringList availablePlugins
          = IO::Drivers::CANBus::instance().pluginList();
      int pluginIndex = availablePlugins.indexOf(plugin);

      if (pluginIndex >= 0)
      {
        IO::Drivers::CANBus::instance().setPluginIndex(pluginIndex);

        // Get available interfaces for this plugin
        QStringList availableInterfaces
            = IO::Drivers::CANBus::instance().interfaceList();
        int interfaceIndex = availableInterfaces.indexOf(interfaceName);

        if (interfaceIndex >= 0)
        {
          IO::Drivers::CANBus::instance().setInterfaceIndex(interfaceIndex);

          // Set bitrate if provided
          if (parser.isSet(canbusBitrateOpt))
          {
            bool ok;
            quint32 bitrate = parser.value(canbusBitrateOpt).toUInt(&ok);
            if (ok && bitrate > 0)
              IO::Drivers::CANBus::instance().setBitrate(bitrate);
            else
              qWarning() << "Invalid CAN bus bitrate:"
                         << parser.value(canbusBitrateOpt);
          }

          // Set CAN-FD mode if flag is present
          if (parser.isSet(canbusFdOpt))
            IO::Drivers::CANBus::instance().setCanFD(true);

          IO::Manager::instance().connectDevice();
        }

        else
        {
          qWarning() << "CAN interface" << interfaceName
                     << "not found for plugin" << plugin;
          qWarning() << "Available interfaces:"
                     << availableInterfaces.join(", ");
        }
      }

      else
      {
        qWarning() << "CAN plugin" << plugin << "not found";
        qWarning() << "Available plugins:" << availablePlugins.join(", ");
      }
    }

    else
      qWarning() << "Invalid CAN bus format. Expected: plugin:interface";
  }
#endif

  // Enter application event loop
  const auto status = app.exec();

  // Free dynamically-generated argv on Windows
#ifdef Q_OS_WIN
  for (int i = 0; i < argc; ++i)
    free(argv[i]);

  free(argv);
#endif

  // Exit application
  return status;
}

//------------------------------------------------------------------------------
// Implement utility functions
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Windows-specific initialization code
//------------------------------------------------------------------------------

#ifdef Q_OS_WIN
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
  if (AttachConsole(ATTACH_PARENT_PROCESS))
  {
    FILE *fp = nullptr;
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
static char **adjustArgumentsForFreeType(int &argc, char **argv)
{
  // Define the additional FreeType arguments for Windows
  const char *platformArgument = "-platform";
  const char *platformOption = "windows:fontengine=freetype";

  // Allocate new argv array
  char **newArgv = static_cast<char **>(malloc(sizeof(char *) * (argc + 2)));
  if (!newArgv)
    return argv;

  // Copy original argv content
  for (int i = 0; i < argc; ++i)
    newArgv[i] = _strdup(argv[i]);

  // Append FreeType platform arguments
  newArgv[argc] = _strdup(platformArgument);
  newArgv[argc + 1] = _strdup(platformOption);

  argc += 2;
  return newArgv;
}
#endif
