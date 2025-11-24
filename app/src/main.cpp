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

#include <QThread>
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
#include "JSON/ProjectModel.h"
#include "JSON/FrameBuilder.h"
#include "IO/Drivers/Network.h"
#include "Misc/ModuleManager.h"

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

static void handleCliArguments(const QCommandLineParser &parser);

#ifdef Q_OS_LINUX
static void setupAppImageIcon(const QString &appExecutableName,
                              const QString &iconResourcePath);
#endif

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
#ifdef Q_OS_WIN
  attachToConsole();
  argv = adjustArgumentsForFreeType(argc, argv);
#endif

  // Linux specific initialization code
#ifdef Q_OS_LINUX
  setupAppImageIcon(APP_EXECUTABLE,
                    QStringLiteral(":/rcc/logo/desktop-icon.png"));
#endif

  // Avoid 200% scaling on 150% scaling...
  auto policy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;
  QApplication::setHighDpiScaleFactorRoundingPolicy(policy);

  // Initialize application
  QApplication app(argc, argv);

  // Set thread priority
  QThread::currentThread()->setPriority(QThread::HighestPriority);

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
    JSON::ProjectModel::instance().openJsonFile(projectPath);
    JSON::FrameBuilder::instance().setOperationMode(SerialStudio::ProjectFile);
  }

  // Enable quick plot mode
  else if (parser.isSet(qOpt))
    JSON::FrameBuilder::instance().setOperationMode(SerialStudio::QuickPlot);

  // Disable frame processing
  else if (parser.isSet(jOpt))
    JSON::FrameBuilder::instance().setOperationMode(
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

  // Enter application event loop
  app.processEvents();
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
// Linux-specific initialization code
//------------------------------------------------------------------------------

#ifdef Q_OS_LINUX
/**
 * @brief Ensures the application icon is set correctly for AppImage deployments
 *        on GNU/Linux.
 *
 * This function copies the application icon from the resource file to the
 * appropriate icon directory on GNU/Linux systems.
 *
 * If the icon file does not already exist in the local user's icons directory,
 * it is copied from the application’s resources to ensure proper display in the
 * desktop environment, even when running the application as an AppImage.
 *
 * The function creates any necessary directories if they do not exist, and
 * performs file checks to prevent redundant copying.
 *
 * @param appExecutableName The name of the application executable, used to name
 *                          the icon file.
 * @param iconResourcePath The path to the icon in the application's resources
 *                         (e.g., `:/rcc/images/icon@2x.png`).
 */
static void setupAppImageIcon(const QString &appExecutableName,
                              const QString &iconResourcePath)
{
  // clang-format off
  const QString pixmapPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/icons/hicolor/256x256/apps/";
  const QString pixmapFile = pixmapPath + appExecutableName + ".png";
  // clang-format on

  // Ensure the directory exists; create it if it doesn't
  QDir dir;
  if (!dir.exists(pixmapPath) && !dir.mkpath(pixmapPath))
    return;

  // Copy the icon from resources to the destination
  QFile resourceFile(iconResourcePath);
  if (resourceFile.open(QIODevice::ReadOnly))
  {
    QFile localFile(pixmapFile);
    if (localFile.open(QIODevice::WriteOnly))
    {
      localFile.write(resourceFile.readAll());
      localFile.close();
    }

    resourceFile.close();
  }
}
#endif

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
    (void)freopen_s(nullptr, "CONOUT$", "w", stdout);
    (void)freopen_s(nullptr, "CONOUT$", "w", stderr);
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
