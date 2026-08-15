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

#include "Misc/CLI.h"

#include <cstdio>
#include <cstring>
#include <QApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QTimer>

#include "API/CommandHandler.h"
#include "API/CommandRegistry.h"
#include "API/Server.h"
#include "AppInfo.h"
#include "AppState.h"
#include "Benchmark/HotpathBenchmark.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/FileTransmission.h"
#include "Misc/ModuleManager.h"
#include "Misc/TimerEvents.h"
#include "SerialStudio.h"
#include "SessionContext.h"
#include "UI/Dashboard.h"
#include "UI/TaskbarSettings.h"

#ifdef SS_INAPP_TESTS
#  include "SelfTest/SelfTest.h"
#endif

#ifdef BUILD_COMMERCIAL
#  include <QAbstractButton>
#  include <QFileInfo>
#  include <QMessageBox>
#  include <QPushButton>

#  include "Console/Export.h"
#  include "CSV/Export.h"
#  include "Licensing/GuardSelfTest.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/OfflineSelfTest.h"
#  include "Licensing/Trial.h"
#  include "MDF4/Export.h"
#  include "Misc/ShortcutGenerator.h"
#  include "Misc/ThemeManager.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#  include "Sessions/Verifier.h"
#endif

namespace Misc {

//---------------------------------------------------------------------------------------------------
// Construction & registration
//---------------------------------------------------------------------------------------------------

/**
 * @brief Builds the CLI parser, application description, and option set.
 */
CLI::CLI()
{
  registerOptions();
}

/**
 * @brief Configures the parser application description and registers every option.
 */
void CLI::registerOptions()
{
  m_parser.setApplicationDescription(PROJECT_DESCRIPTION_SUMMARY);
  m_parser.addHelpOption();
  m_parser.addPositionalArgument("project", "Project file to open", "[project]");
  m_parser.addOption(m_opts.versionOpt);
  m_parser.addOption(m_opts.resetOpt);
  m_parser.addOption(m_opts.fullscreenOpt);
  m_parser.addOption(m_opts.headlessOpt);
  m_parser.addOption(m_opts.apiServerOpt);
  m_parser.addOption(m_opts.apiExternalOpt);
  m_parser.addOption(m_opts.apiTokenOpt);
  m_parser.addOption(m_opts.dumpApiSchemaOpt);
  m_parser.addOption(m_opts.projectOpt);
  m_parser.addOption(m_opts.quickPlotOpt);
  m_parser.addOption(m_opts.fpsOpt);
  m_parser.addOption(m_opts.pointsOpt);
  m_parser.addOption(m_opts.uartOpt);
  m_parser.addOption(m_opts.baudOpt);
  m_parser.addOption(m_opts.tcpOpt);
  m_parser.addOption(m_opts.udpOpt);
  m_parser.addOption(m_opts.udpRemoteOpt);
  m_parser.addOption(m_opts.udpMulticastOpt);
  m_parser.addOption(m_opts.benchmarkHotpathOpt);
  m_parser.addOption(m_opts.minFpsOpt);
  m_parser.addOption(m_opts.benchmarkFramesOpt);
  m_parser.addOption(m_opts.benchmarkSecondsOpt);
  m_parser.addOption(m_opts.benchmarkOutputOpt);
  m_parser.addOption(m_opts.exitAfterOpt);
#ifdef SS_INAPP_TESTS
  m_parser.addOption(m_opts.selftestOpt);
  m_parser.addOption(m_opts.selftestSuiteOpt);
#endif
#ifdef BUILD_COMMERCIAL
  m_parser.addOption(m_opts.noToolbarOpt);
  m_parser.addOption(m_opts.runtimeOpt);
  m_parser.addOption(m_opts.shortcutPathOpt);
  m_parser.addOption(m_opts.csvExportOpt);
  m_parser.addOption(m_opts.mdfExportOpt);
  m_parser.addOption(m_opts.sessionExportOpt);
  m_parser.addOption(m_opts.consoleExportOpt);
  m_parser.addOption(m_opts.actionsPanelOpt);
  m_parser.addOption(m_opts.fileTransmissionOpt);
  m_parser.addOption(m_opts.taskbarModeOpt);
  m_parser.addOption(m_opts.taskbarButtonsOpt);
  m_parser.addOption(m_opts.noTaskbarSearchOpt);
  m_parser.addOption(m_opts.themeOpt);
  m_parser.addOption(m_opts.activateOpt);
  m_parser.addOption(m_opts.deactivateOpt);
  m_parser.addOption(m_opts.selftestOfflineOpt);
  m_parser.addOption(m_opts.verifySessionOpt);
  m_parser.addOption(m_opts.verifySessionIdOpt);
  m_parser.addOption(m_opts.verifyKeepRegenOpt);
  m_parser.addOption(m_opts.regressSessionOpt);
  m_parser.addOption(m_opts.regressSessionIdOpt);
  m_parser.addOption(m_opts.regressProjectOpt);
  m_parser.addOption(m_opts.regressKeepRegenOpt);
  m_parser.addOption(m_opts.validateGuardsOpt);
  m_parser.addOption(m_opts.modbusRtuOpt);
  m_parser.addOption(m_opts.modbusTcpOpt);
  m_parser.addOption(m_opts.modbusSlaveOpt);
  m_parser.addOption(m_opts.modbusPollOpt);
  m_parser.addOption(m_opts.modbusBaudOpt);
  m_parser.addOption(m_opts.modbusParityOpt);
  m_parser.addOption(m_opts.modbusDataBitsOpt);
  m_parser.addOption(m_opts.modbusStopBitsOpt);
  m_parser.addOption(m_opts.modbusRegisterOpt);
  m_parser.addOption(m_opts.canbusOpt);
  m_parser.addOption(m_opts.canbusBitrateOpt);
  m_parser.addOption(m_opts.canbusFdOpt);
#endif
}

//---------------------------------------------------------------------------------------------------
// Pre-QApplication argv scanning
//---------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if @p flag is present in raw argv (used before parsing).
 */
bool CLI::argvHasFlag(int argc, char** argv, const char* flag)
{
  for (int i = 1; i < argc; ++i)
    if (std::strcmp(argv[i], flag) == 0)
      return true;

  return false;
}

/**
 * @brief Reads the value following @p flag in raw argv (used before parsing).
 */
QString CLI::argvValueFor(int argc, char** argv, const char* flag)
{
  for (int i = 1; i < argc - 1; ++i)
    if (std::strcmp(argv[i], flag) == 0)
      return QString::fromLocal8Bit(argv[i + 1]);

  return QString();
}

/**
 * @brief True if argv requests a CLI command that prints and exits (no GUI session).
 */
bool CLI::isCliEarlyExit(int argc, char** argv)
{
  static const char* const kFlags[] = {"-v",
                                       "--version",
                                       "-h",
                                       "--help",
                                       "-r",
                                       "--reset",
                                       "--activate",
                                       "--deactivate",
                                       "--selftest-offline-license",
                                       "--validate-guards",
                                       "--verify-session",
                                       "--regress-session",
                                       "--dump-api-schema"};

  for (const char* flag : kFlags)
    if (argvHasFlag(argc, argv, flag))
      return true;

#ifdef SS_INAPP_TESTS
  if (argvHasFlag(argc, argv, "--selftest") || argvHasFlag(argc, argv, "--selftest-suite"))
    return true;
#endif

  return isBenchmarkRequested(argc, argv);
}

/**
 * @brief True if argv requests the hotpath benchmark via any of its flags.
 */
bool CLI::isBenchmarkRequested(int argc, char** argv)
{
  return argvHasFlag(argc, argv, "--benchmark-hotpath")
      || argvHasFlag(argc, argv, "--benchmark-frames")
      || argvHasFlag(argc, argv, "--benchmark-seconds") || argvHasFlag(argc, argv, "--min-fps");
}

//---------------------------------------------------------------------------------------------------
// Top-level processing
//---------------------------------------------------------------------------------------------------

/**
 * @brief Parses the command line and runs early-exit flows (version/reset/license).
 */
CLI::ProcessResult CLI::process(QApplication& app)
{
  m_parser.process(app);

  if (m_parser.isSet(m_opts.versionOpt)) {
    qDebug() << APP_NAME << "version" << APP_VERSION;
    qDebug() << "Written by Alex Spataru <https://github.com/alex-spataru>";
    return ProcessResult::ExitSuccess;
  }

  if (m_parser.isSet(m_opts.resetOpt)) {
    QSettings(APP_SUPPORT_URL, APP_NAME).clear();
    qDebug() << APP_NAME << "settings cleared!";
    return ProcessResult::ExitSuccess;
  }

  if (m_parser.isSet(m_opts.benchmarkHotpathOpt) || m_parser.isSet(m_opts.benchmarkFramesOpt)
      || m_parser.isSet(m_opts.benchmarkSecondsOpt) || m_parser.isSet(m_opts.minFpsOpt))
    return runHotpathBenchmark();

  if (m_parser.isSet(m_opts.dumpApiSchemaOpt))
    return dumpApiSchema(m_parser.value(m_opts.dumpApiSchemaOpt));

#ifdef SS_INAPP_TESTS
  if (m_parser.isSet(m_opts.selftestOpt) || m_parser.isSet(m_opts.selftestSuiteOpt))
    return runSelfTests();
#endif

#ifdef BUILD_COMMERCIAL
  if (m_parser.isSet(m_opts.verifySessionOpt))
    return runSessionVerification();

  if (m_parser.isSet(m_opts.regressSessionOpt))
    return runSessionRegression();
#endif

#ifdef BUILD_COMMERCIAL
  if (m_parser.isSet(m_opts.validateGuardsOpt)) {
    return Licensing::runGuardSelfTest() == 0 ? ProcessResult::ExitSuccess
                                              : ProcessResult::ExitFailure;
  }
#endif

#ifdef BUILD_COMMERCIAL
  if (m_parser.isSet(m_opts.selftestOfflineOpt)) {
    return Licensing::runOfflineSelfTest() == 0 ? ProcessResult::ExitSuccess
                                                : ProcessResult::ExitFailure;
  }

  if (m_parser.isSet(m_opts.activateOpt)) {
    return activateLicense(app, m_parser.value(m_opts.activateOpt)) == EXIT_SUCCESS
           ? ProcessResult::ExitSuccess
           : ProcessResult::ExitFailure;
  }

  if (m_parser.isSet(m_opts.deactivateOpt)) {
    return deactivateLicense(app) == EXIT_SUCCESS ? ProcessResult::ExitSuccess
                                                  : ProcessResult::ExitFailure;
  }
#endif

  scheduleExitAfter(app);

  return ProcessResult::Continue;
}

/**
 * @brief Schedules a graceful quit N seconds after startup when --exit-after is set. Exiting
 *        through the event loop (not a kill) is what lets PGO training runs flush their profile.
 */
void CLI::scheduleExitAfter(QApplication& app)
{
  if (!m_parser.isSet(m_opts.exitAfterOpt))
    return;

  bool ok          = false;
  const double val = SerialStudio::toDouble(m_parser.value(m_opts.exitAfterOpt), &ok);
  if (!ok || val <= 0.0)
    return;

  const int msec = static_cast<int>(qMin(val, 86400.0) * 1000.0);
  QTimer::singleShot(msec, &app, &QCoreApplication::quit);
}

/**
 * @brief Runs the frame-extraction throughput benchmark and maps the result to an exit code.
 *        The benchmark exits before any ModuleManager is built, so it runs the pinned module
 *        order itself: without it every session subsystem the benchmark reaches would be
 *        constructed lazily, out of order, and unowned (spec 0039 M2).
 */
CLI::ProcessResult CLI::runHotpathBenchmark()
{
  quint64 frames = 1'000'000;
  double minFps  = 256'000.0;
  double seconds = 10.0;

  if (m_parser.isSet(m_opts.benchmarkFramesOpt)) {
    bool ok           = false;
    const quint64 val = m_parser.value(m_opts.benchmarkFramesOpt).toULongLong(&ok);
    if (ok && val > 0)
      frames = val;
  }

  if (m_parser.isSet(m_opts.minFpsOpt)) {
    bool ok          = false;
    const double val = SerialStudio::toDouble(m_parser.value(m_opts.minFpsOpt), &ok);
    if (ok && val > 0.0)
      minFps = val;
  }

  if (m_parser.isSet(m_opts.benchmarkSecondsOpt)) {
    bool ok          = false;
    const double val = SerialStudio::toDouble(m_parser.value(m_opts.benchmarkSecondsOpt), &ok);
    if (ok && val >= 0.0)
      seconds = val;
  }

  QString output;
  if (m_parser.isSet(m_opts.benchmarkOutputOpt))
    output = m_parser.value(m_opts.benchmarkOutputOpt).trimmed();

  Misc::ModuleManager::instantiateCoreModules();

  const int rc = Benchmark::HotpathBenchmark::runAndReport(frames, minFps, seconds, output);
  return rc == EXIT_SUCCESS ? ProcessResult::ExitSuccess : ProcessResult::ExitFailure;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Tears a headless CLI session down in the pinned order main() uses for the GUI run: the
 *        Sessions wiring, then every frame-consumer worker, then the adopted core modules. These
 *        paths return from CLI::process() before main() reaches its own teardown, so skipping any
 *        step leaves a worker ticking into modules freed at static destruction.
 */
static void teardownHeadlessSession()
{
  Misc::ModuleManager::teardownHeadlessSessionModules();
  Misc::ModuleManager::stopFrameConsumerWorkers();
  SessionContext::current().shutdown();
}

/**
 * @brief Runs the spec-0044 verifier and exits: builds the pinned module order plus the
 *        headless session wiring, since the GUI wiring phase never runs here. Exit code is
 *        binary (0 = reproduced); the verdict lives in the JSON. Sessions modules tear down
 *        before returning, while qApp is still alive.
 */
CLI::ProcessResult CLI::runSessionVerification()
{
  Sessions::Verifier::Options options;
  options.dbPath          = m_parser.value(m_opts.verifySessionOpt).trimmed();
  options.keepRegenerated = m_parser.isSet(m_opts.verifyKeepRegenOpt);

  if (m_parser.isSet(m_opts.verifySessionIdOpt)) {
    bool ok       = false;
    const int val = m_parser.value(m_opts.verifySessionIdOpt).toInt(&ok);
    if (!ok || val < 0) {
      std::fputs("{\"verdict\":\"error\",\"error\":\"invalid --verify-session-id\"}\n", stdout);
      std::fflush(stdout);
      return ProcessResult::ExitFailure;
    }

    options.sessionId = val;
  }

  Misc::ModuleManager::instantiateCoreModules();
  Misc::ModuleManager::setupHeadlessSessionConnections();

  Sessions::Verifier verifier(options);
  const int code = verifier.run();

  std::fputs(QJsonDocument(verifier.report()).toJson(QJsonDocument::Indented).constData(), stdout);
  std::fflush(stdout);

  teardownHeadlessSession();

  return code == Sessions::Verifier::kExitReproduced ? ProcessResult::ExitSuccess
                                                     : ProcessResult::ExitFailure;
}

/**
 * @brief Runs the spec-0047 regression pass and exits: same headless bootstrap and Sessions
 *        teardown discipline as runSessionVerification(). Exit code is binary (0 = identical);
 *        the drift verdict and report live in the JSON on stdout.
 */
CLI::ProcessResult CLI::runSessionRegression()
{
  Sessions::Verifier::Options options;
  options.mode                 = Sessions::Verifier::Mode::Regress;
  options.dbPath               = m_parser.value(m_opts.regressSessionOpt).trimmed();
  options.candidateProjectPath = m_parser.value(m_opts.regressProjectOpt).trimmed();
  options.keepRegenerated      = m_parser.isSet(m_opts.regressKeepRegenOpt);

  if (options.candidateProjectPath.isEmpty()) {
    std::fputs("{\"verdict\":\"error\",\"errorCode\":\"regress-candidate-unreadable\","
               "\"error\":\"missing --regress-project\",\"hint\":\"Pass the candidate project "
               "file to compare the archived session against.\"}\n",
               stdout);
    std::fflush(stdout);
    return ProcessResult::ExitFailure;
  }

  if (m_parser.isSet(m_opts.regressSessionIdOpt)) {
    bool ok       = false;
    const int val = m_parser.value(m_opts.regressSessionIdOpt).toInt(&ok);
    if (!ok || val < 0) {
      std::fputs("{\"verdict\":\"error\",\"error\":\"invalid --regress-session-id\"}\n", stdout);
      std::fflush(stdout);
      return ProcessResult::ExitFailure;
    }

    options.sessionId = val;
  }

  Misc::ModuleManager::instantiateCoreModules();
  Misc::ModuleManager::setupHeadlessSessionConnections();

  Sessions::Verifier verifier(options);
  const int code = verifier.run();

  std::fputs(QJsonDocument(verifier.report()).toJson(QJsonDocument::Indented).constData(), stdout);
  std::fflush(stdout);

  teardownHeadlessSession();

  return code == Sessions::Verifier::kExitReproduced ? ProcessResult::ExitSuccess
                                                     : ProcessResult::ExitFailure;
}
#endif

/**
 * @brief Dumps the API command registry to a JSON file for the SDK generator.
 */
CLI::ProcessResult CLI::dumpApiSchema(const QString& path)
{
  static auto& commandHandler = API::CommandHandler::instance();
  (void)commandHandler;
  static auto& commandRegistry = API::CommandRegistry::instance();
  const auto& commands         = commandRegistry.commands();

  QJsonArray array;
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    const auto& def = it.value();

    QJsonObject entry;
    entry.insert(QStringLiteral("name"), def.name);
    entry.insert(QStringLiteral("description"), def.description);
    entry.insert(QStringLiteral("properties"),
                 def.inputSchema.value(QStringLiteral("properties")).toObject());
    entry.insert(QStringLiteral("required"),
                 def.inputSchema.value(QStringLiteral("required")).toArray());
    array.append(entry);
  }

  QFile file(path);
  if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
    qWarning() << "Failed to open" << path << "for writing";
    return ProcessResult::ExitFailure;
  }

  file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
  file.close();
  qDebug() << "Wrote" << array.size() << "commands to" << path;
  return ProcessResult::ExitSuccess;
}

#ifdef SS_INAPP_TESTS

/**
 * @brief Runs the in-app self-test suites and maps the aggregate result to an exit code. This runs
 *        before the composition root is built, so the suites see no application singleton.
 */
CLI::ProcessResult CLI::runSelfTests()
{
  const QString suite = m_parser.value(m_opts.selftestSuiteOpt).trimmed();
  const int rc        = SelfTest::Runner::runAndReport(suite);
  return rc == EXIT_SUCCESS ? ProcessResult::ExitSuccess : ProcessResult::ExitFailure;
}

#endif

//---------------------------------------------------------------------------------------------------
// Accessors
//---------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if --fullscreen was passed.
 */
bool CLI::fullscreen() const noexcept
{
  return m_parser.isSet(m_opts.fullscreenOpt);
}

/**
 * @brief Returns true if --runtime (Pro operator mode) was passed.
 */
bool CLI::runtimeMode() const noexcept
{
#ifdef BUILD_COMMERCIAL
  return m_parser.isSet(m_opts.runtimeOpt);
#else
  return false;
#endif
}

/**
 * @brief Returns true if the toolbar should be hidden at startup.
 */
bool CLI::hideToolbar() const noexcept
{
#ifdef BUILD_COMMERCIAL
  return runtimeMode() || m_parser.isSet(m_opts.noToolbarOpt) || fullscreen();
#else
  return fullscreen();
#endif
}

/**
 * @brief Returns true if --api-server was passed.
 */
bool CLI::apiServerEnabled() const
{
  return m_parser.isSet(m_opts.apiServerOpt);
}

/**
 * @brief Returns true if --api-external was passed.
 */
bool CLI::apiExternalEnabled() const
{
  return m_parser.isSet(m_opts.apiExternalOpt);
}

/**
 * @brief Returns true if --quick-plot was passed.
 */
bool CLI::quickPlot() const
{
  return m_parser.isSet(m_opts.quickPlotOpt);
}

/**
 * @brief Returns the --project value, or an empty string when unset.
 */
QString CLI::projectPath() const
{
  if (m_parser.isSet(m_opts.projectOpt))
    return m_parser.value(m_opts.projectOpt);

  const auto positional = m_parser.positionalArguments();
  return positional.isEmpty() ? QString() : positional.first();
}

//---------------------------------------------------------------------------------------------------
// Project/auto-connect/visualization apply
//---------------------------------------------------------------------------------------------------

/**
 * @brief Applies CLI project/quick-plot mode and schedules runtime auto-connect.
 */
void CLI::applyProjectAndAutoConnect(QApplication& app)
{
  applyApiServerOptions();

  const QString project = projectPath();
  if (!project.isEmpty()) {
    static auto& appState = AppState::instance();
    appState.setOperationMode(SerialStudio::ProjectFile);
    static auto& projectModel = DataModel::ProjectModel::instance();
    projectModel.openJsonFile(project);
  }

  else if (quickPlot()) {
    static auto& appState = AppState::instance();
    appState.setOperationMode(SerialStudio::QuickPlot);
  }

  if (!runtimeMode())
    return;

  QTimer::singleShot(0, &app, []() {
#ifdef BUILD_COMMERCIAL
    static auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
    if (!lemonSqueezy.isActivated()) {
      static auto& trial = Licensing::Trial::instance();
      if (!trial.trialEnabled())
        return;
    }
#endif
    static auto& cm = IO::ConnectionManager::instance();
    if (cm.configurationOk() && !cm.isConnected())
      cm.connectDevice();
  });
}

/**
 * @brief Applies the API-server flags. The token is pinned before external connections are
 *        opened so a headless machine is provisioned with the operator's credential rather than
 *        a freshly generated one; each flag acts only when it was passed, so an omitted flag
 *        leaves the persisted settings and the bind address exactly as they were.
 */
void CLI::applyApiServerOptions()
{
  if (!apiServerEnabled() && !apiExternalEnabled() && !m_parser.isSet(m_opts.apiTokenOpt))
    return;

  static auto& apiServer = API::Server::instance();

  if (m_parser.isSet(m_opts.apiTokenOpt)
      && !apiServer.setAuthToken(m_parser.value(m_opts.apiTokenOpt)))
    qWarning() << "[CLI] --api-token ignored: expected at least 32 hexadecimal characters";

  if (apiExternalEnabled())
    apiServer.allowExternalConnections();

  if (apiServerEnabled())
    apiServer.setEnabled(true);
}

/**
 * @brief Applies CLI dashboard FPS and point-count options if set.
 */
void CLI::applyVisualizationOptions()
{
  if (m_parser.isSet(m_opts.fpsOpt)) {
    bool ok;
    auto fps = m_parser.value(m_opts.fpsOpt).toUInt(&ok);
    if (ok) {
      static auto& timerEvents = Misc::TimerEvents::instance();
      timerEvents.setFPS(fps);
    }
  }

  if (m_parser.isSet(m_opts.pointsOpt)) {
    bool ok;
    auto points = m_parser.value(m_opts.pointsOpt).toUInt(&ok);
    if (ok) {
      static auto& dashboard = UI::Dashboard::instance();
      dashboard.setPoints(points);
    }
  }
}

/**
 * @brief Dispatches CLI bus configuration to the matching driver setup helper.
 */
void CLI::applyBusConfiguration()
{
  if (m_parser.isSet(m_opts.uartOpt) || m_parser.isSet(m_opts.baudOpt))
    setupUartConnection();
  else if (m_parser.isSet(m_opts.tcpOpt))
    setupTcpConnection(m_parser.value(m_opts.tcpOpt));
  else if (m_parser.isSet(m_opts.udpOpt))
    setupUdpConnection();
#ifdef BUILD_COMMERCIAL
  else if (m_parser.isSet(m_opts.modbusRtuOpt))
    setupModbusRtuConnection();
  else if (m_parser.isSet(m_opts.modbusTcpOpt))
    setupModbusTcpConnection();
  else if (m_parser.isSet(m_opts.canbusOpt))
    setupCanbusConnection();
#endif
}

//---------------------------------------------------------------------------------------------------
// UART / TCP / UDP setup
//---------------------------------------------------------------------------------------------------

/**
 * @brief Configures and connects the UART bus from CLI options.
 */
void CLI::setupUartConnection()
{
  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.setBusType(SerialStudio::BusType::UART);

  if (m_parser.isSet(m_opts.uartOpt)) {
    const QString device = m_parser.value(m_opts.uartOpt);
    connectionManager.uart()->registerDevice(device);
  }

  if (m_parser.isSet(m_opts.baudOpt)) {
    bool ok               = false;
    const qint32 baudRate = m_parser.value(m_opts.baudOpt).toInt(&ok);
    if (!ok)
      qWarning() << "Invalid baud rate:" << m_parser.value(m_opts.baudOpt);
    else
      connectionManager.uart()->setBaudRate(baudRate);
  }

  connectionManager.connectDevice();
}

/**
 * @brief Configures and connects a TCP socket from a CLI host:port string.
 */
void CLI::setupTcpConnection(const QString& tcpAddress)
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

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.setBusType(SerialStudio::BusType::Network);
  connectionManager.network()->setTcpSocket();
  connectionManager.network()->setRemoteAddress(parts[0]);
  connectionManager.network()->setTcpPort(port);
  connectionManager.connectDevice();
}

/**
 * @brief Applies the optional --udp-remote spec to the active network driver.
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

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.network()->setRemoteAddress(parts[0]);
  connectionManager.network()->setUdpRemotePort(remotePort);
}

/**
 * @brief Configures and connects a UDP socket from CLI options.
 */
void CLI::setupUdpConnection()
{
  bool ok                 = false;
  const quint16 localPort = m_parser.value(m_opts.udpOpt).toUInt(&ok);
  if (!ok || localPort == 0) {
    qWarning() << "Invalid UDP local port:" << m_parser.value(m_opts.udpOpt);
    return;
  }

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.setBusType(SerialStudio::BusType::Network);
  connectionManager.network()->setUdpSocket();
  connectionManager.network()->setUdpLocalPort(localPort);

  if (m_parser.isSet(m_opts.udpMulticastOpt))
    connectionManager.network()->setUdpMulticast(true);

  if (m_parser.isSet(m_opts.udpRemoteOpt))
    applyUdpRemote(m_parser.value(m_opts.udpRemoteOpt));

  connectionManager.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// Commercial: shortcut/runtime/exports
//---------------------------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

/**
 * @brief Confirms the runtime-mode shortcut's project file exists; prompts cleanup if missing.
 */
bool CLI::verifyShortcutProject() const
{
  if (!runtimeMode() || !m_parser.isSet(m_opts.projectOpt))
    return true;

  const QString projectPath = m_parser.value(m_opts.projectOpt);
  if (QFileInfo::exists(projectPath))
    return true;

  const QString shortcutPath =
    m_parser.isSet(m_opts.shortcutPathOpt) ? m_parser.value(m_opts.shortcutPathOpt) : QString();

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

  if (deleteBtn != nullptr && box.clickedButton() == deleteBtn) {
    static auto& shortcutGenerator = Misc::ShortcutGenerator::instance();
    shortcutGenerator.deleteShortcut(shortcutPath);
  }

  return false;
}

/**
 * @brief Splits a comma-separated taskbar pin list into a trimmed QStringList.
 */
static QStringList splitTaskbarButtonIds(const QString& raw)
{
  QStringList ids;
  const auto parts = raw.split(QLatin1Char(','), Qt::SkipEmptyParts);
  ids.reserve(parts.size());
  for (const auto& p : parts)
    ids.append(p.trimmed());

  return ids;
}

/**
 * @brief Applies taskbar visibility/pinning settings for operator runtime mode.
 */
void CLI::applyOperatorTaskbarSettings()
{
  static auto& tbs = UI::TaskbarSettings::instance();
  if (m_parser.isSet(m_opts.taskbarModeOpt)) {
    const QString mode = m_parser.value(m_opts.taskbarModeOpt).toLower();
    tbs.setTaskbarHidden(mode == QStringLiteral("hidden"));
    tbs.setAutohide(mode == QStringLiteral("autohide"));
  } else {
    tbs.setTaskbarHidden(false);
    tbs.setAutohide(false);
  }

  const QStringList pinned = m_parser.isSet(m_opts.taskbarButtonsOpt)
                             ? splitTaskbarButtonIds(m_parser.value(m_opts.taskbarButtonsOpt))
                             : QStringList{};
  tbs.setPinnedButtons(pinned);

  tbs.setSearchEnabled(!m_parser.isSet(m_opts.noTaskbarSearchOpt));
}

/**
 * @brief Activates the theme named by --theme, if any.
 */
void CLI::applyThemeOverride()
{
  if (!m_parser.isSet(m_opts.themeOpt))
    return;

  const QString name = m_parser.value(m_opts.themeOpt).trimmed();
  if (name.isEmpty())
    return;

  static auto& tm           = Misc::ThemeManager::instance();
  const QStringList& themes = tm.availableThemes();
  const int idx             = themes.indexOf(name);
  if (idx < 0) {
    qWarning().noquote() << "[CLI] Unknown --theme value:" << name
                         << "-- available themes:" << themes;
    return;
  }

  if (runtimeMode())
    tm.setSettingsPersistent(false);

  if (tm.theme() != idx)
    tm.setTheme(idx);
}

/**
 * @brief Configures runtime/operator-mode export, dashboard, and taskbar overrides.
 */
void CLI::applyOperatorRuntimeSettings()
{
  static auto& csvExport        = CSV::Export::instance();
  static auto& mdf4Export       = MDF4::Export::instance();
  static auto& sessionsExport   = Sessions::Export::instance();
  static auto& consoleExport    = Console::Export::instance();
  static auto& dashboard        = UI::Dashboard::instance();
  static auto& taskbarSettings  = UI::TaskbarSettings::instance();
  static auto& themeManager     = Misc::ThemeManager::instance();
  static auto& fileTransmission = IO::FileTransmission::instance();

  csvExport.setSettingsPersistent(false);
  mdf4Export.setSettingsPersistent(false);
  sessionsExport.setSettingsPersistent(false);
  consoleExport.setSettingsPersistent(false);
  dashboard.setSettingsPersistent(false);
  taskbarSettings.setSettingsPersistent(false);
  themeManager.setSettingsPersistent(false);

  csvExport.setExportEnabled(m_parser.isSet(m_opts.csvExportOpt));
  mdf4Export.setExportEnabled(m_parser.isSet(m_opts.mdfExportOpt));
  sessionsExport.setExportEnabled(m_parser.isSet(m_opts.sessionExportOpt));
  consoleExport.setExportEnabled(m_parser.isSet(m_opts.consoleExportOpt));

  dashboard.setTerminalEnabled(false);
  dashboard.setNotificationLogEnabled(false);
  dashboard.setShowActionPanel(m_parser.isSet(m_opts.actionsPanelOpt));
  fileTransmission.setRuntimeAccessAllowed(m_parser.isSet(m_opts.fileTransmissionOpt));

  applyOperatorTaskbarSettings();
}

/**
 * @brief Enables CSV/MDF4/session/console exporters when their CLI flags are set.
 */
void CLI::applyExportToggles()
{
  if (m_parser.isSet(m_opts.csvExportOpt)) {
    static auto& csvExport = CSV::Export::instance();
    csvExport.setExportEnabled(true);
  }

  if (m_parser.isSet(m_opts.mdfExportOpt)) {
    static auto& mdf4Export = MDF4::Export::instance();
    mdf4Export.setExportEnabled(true);
  }

  if (m_parser.isSet(m_opts.sessionExportOpt)) {
    static auto& sessionsExport = Sessions::Export::instance();
    sessionsExport.setExportEnabled(true);
  }

  if (m_parser.isSet(m_opts.consoleExportOpt)) {
    static auto& consoleExport = Console::Export::instance();
    consoleExport.setExportEnabled(true);
  }
}

//---------------------------------------------------------------------------------------------------
// Commercial: license activate / deactivate
//---------------------------------------------------------------------------------------------------

/**
 * @brief Activates a license key against the Lemon Squeezy API and exits. Quit is deferred to
 *        the next loop pass: activatedChanged fires inside the network reply's finished handler,
 *        and a synchronous quit() would tear the manager down under that reply's stack frame.
 */
int CLI::activateLicense(QApplication& app, const QString& licenseKey)
{
  static auto& ls = Licensing::LemonSqueezy::instance();

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

    QTimer::singleShot(0, &app, &QCoreApplication::quit);
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
 * @brief Deactivates the stored license instance on this machine and exits. Quit is deferred
 *        to the next loop pass for the same reply-stack reason as activateLicense().
 */
int CLI::deactivateLicense(QApplication& app)
{
  static auto& ls = Licensing::LemonSqueezy::instance();

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

    QTimer::singleShot(0, &app, &QCoreApplication::quit);
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

//---------------------------------------------------------------------------------------------------
// Commercial: Modbus helpers
//---------------------------------------------------------------------------------------------------

/**
 * @brief Parses a Modbus register spec string and registers it with the Modbus driver.
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

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.modbus()->addRegisterGroup(registerType, start, count);
}

/**
 * @brief Applies the Modbus parity option to the active Modbus driver.
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

  const auto it                  = kParity.constFind(parity);
  static auto& connectionManager = IO::ConnectionManager::instance();
  if (it == kParity.cend()) {
    qWarning() << "Invalid ModBus parity (none/even/odd/space/mark):" << parity;
    connectionManager.modbus()->setParityIndex(0);
    return;
  }

  connectionManager.modbus()->setParityIndex(it.value());
}

/**
 * @brief Applies the Modbus data-bits option to the active Modbus driver.
 */
static void applyModbusDataBits(const QString& dataBits)
{
  static const QHash<QString, quint8> kDataBits = {
    {QStringLiteral("5"), 0},
    {QStringLiteral("6"), 1},
    {QStringLiteral("7"), 2},
    {QStringLiteral("8"), 3},
  };

  const auto it                  = kDataBits.constFind(dataBits);
  static auto& connectionManager = IO::ConnectionManager::instance();
  if (it == kDataBits.cend()) {
    qWarning() << "Invalid ModBus data bits (5/6/7/8):" << dataBits;
    connectionManager.modbus()->setDataBitsIndex(3);
    return;
  }

  connectionManager.modbus()->setDataBitsIndex(it.value());
}

/**
 * @brief Applies the Modbus stop-bits option to the active Modbus driver.
 */
static void applyModbusStopBits(const QString& stopBits)
{
  static const QHash<QString, quint8> kStopBits = {
    {  QStringLiteral("1"), 0},
    {QStringLiteral("1.5"), 1},
    {  QStringLiteral("2"), 2},
  };

  const auto it                  = kStopBits.constFind(stopBits);
  static auto& connectionManager = IO::ConnectionManager::instance();
  if (it == kStopBits.cend()) {
    qWarning() << "Invalid ModBus stop bits (1/1.5/2):" << stopBits;
    connectionManager.modbus()->setStopBitsIndex(0);
    return;
  }

  connectionManager.modbus()->setStopBitsIndex(it.value());
}

/**
 * @brief Applies the Modbus slave option to the active Modbus driver.
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

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.modbus()->setSlaveAddress(static_cast<quint8>(slave_val));
}

/**
 * @brief Applies the Modbus poll-interval option to the active Modbus driver.
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

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.modbus()->setPollInterval(interval);
}

/**
 * @brief Applies the Modbus serial baud-rate option to the active Modbus driver.
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

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.modbus()->setBaudRate(baudRate);
}

/**
 * @brief Applies all --modbus-register specs and warns if none are present.
 */
static void applyModbusRegisters(const QCommandLineParser& parser, const QCommandLineOption& opt)
{
  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.modbus()->clearRegisterGroups();
  if (!parser.isSet(opt)) {
    qWarning() << "No register groups specified. Use --modbus-register to add registers.";
    return;
  }

  const QStringList registerSpecs = parser.values(opt);
  for (const QString& spec : std::as_const(registerSpecs))
    applyModbusRegister(spec);
}

/**
 * @brief Parses a Modbus TCP host[:port] string.
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
 * @brief Configures and connects a Modbus RTU bus from CLI options.
 */
void CLI::setupModbusRtuConnection()
{
  static auto& connectionManager = IO::ConnectionManager::instance();
  const QString portPath         = m_parser.value(m_opts.modbusRtuOpt);
  connectionManager.setBusType(SerialStudio::BusType::ModBus);
  connectionManager.modbus()->setProtocolIndex(0);

  const QStringList ports = connectionManager.modbus()->serialPortList();
  const int portIndex     = ports.indexOf(portPath);
  if (portIndex < 0) {
    qWarning() << "ModBus serial port not found:" << portPath;
    qWarning() << "Available ports:" << ports.join(", ");
    return;
  }

  connectionManager.modbus()->setSerialPortIndex(portIndex);
  applyModbusSlave(m_parser, m_opts.modbusSlaveOpt);
  applyModbusPoll(m_parser, m_opts.modbusPollOpt);
  applyModbusBaud(m_parser, m_opts.modbusBaudOpt);

  if (m_parser.isSet(m_opts.modbusParityOpt))
    applyModbusParity(m_parser.value(m_opts.modbusParityOpt).toLower());

  if (m_parser.isSet(m_opts.modbusDataBitsOpt))
    applyModbusDataBits(m_parser.value(m_opts.modbusDataBitsOpt));

  if (m_parser.isSet(m_opts.modbusStopBitsOpt))
    applyModbusStopBits(m_parser.value(m_opts.modbusStopBitsOpt));

  applyModbusRegisters(m_parser, m_opts.modbusRegisterOpt);
  connectionManager.connectDevice();
}

/**
 * @brief Configures and connects a Modbus TCP bus from CLI options.
 */
void CLI::setupModbusTcpConnection()
{
  QString host;
  quint16 port = 502;
  if (!parseModbusTcpAddress(m_parser.value(m_opts.modbusTcpOpt), host, port)) {
    qWarning() << "Invalid ModBus TCP address format. Expected: host[:port]";
    return;
  }

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.setBusType(SerialStudio::BusType::ModBus);
  connectionManager.modbus()->setProtocolIndex(1);
  connectionManager.modbus()->setHost(host);
  connectionManager.modbus()->setPort(port);

  applyModbusSlave(m_parser, m_opts.modbusSlaveOpt);
  applyModbusPoll(m_parser, m_opts.modbusPollOpt);
  applyModbusRegisters(m_parser, m_opts.modbusRegisterOpt);
  connectionManager.connectDevice();
}

/**
 * @brief Configures and connects a CAN bus from CLI options.
 */
void CLI::setupCanbusConnection()
{
  const QStringList parts = m_parser.value(m_opts.canbusOpt).split(':');
  if (parts.size() != 2) {
    qWarning() << "Invalid CAN bus format. Expected: plugin:interface";
    return;
  }

  const QString plugin        = parts[0].toLower();
  const QString interfaceName = parts[1];

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.setBusType(SerialStudio::BusType::CanBus);

  const QStringList availablePlugins = connectionManager.canBus()->pluginList();
  const int pluginIndex              = availablePlugins.indexOf(plugin);
  if (pluginIndex < 0) {
    qWarning() << "CAN plugin" << plugin << "not found";
    qWarning() << "Available plugins:" << availablePlugins.join(", ");
    return;
  }

  connectionManager.canBus()->setPluginIndex(pluginIndex);
  const QStringList availableInterfaces = connectionManager.canBus()->interfaceList();
  const int interfaceIndex              = availableInterfaces.indexOf(interfaceName);

  if (interfaceIndex < 0) {
    qWarning() << "CAN interface" << interfaceName << "not found for plugin" << plugin;
    qWarning() << "Available interfaces:" << availableInterfaces.join(", ");
    return;
  }

  connectionManager.canBus()->setInterfaceIndex(interfaceIndex);

  if (m_parser.isSet(m_opts.canbusBitrateOpt)) {
    bool ok;
    quint32 bitrate = m_parser.value(m_opts.canbusBitrateOpt).toUInt(&ok);
    if (ok && bitrate > 0)
      connectionManager.canBus()->setBitrate(bitrate);
    else
      qWarning() << "Invalid CAN bus bitrate:" << m_parser.value(m_opts.canbusBitrateOpt);
  }

  if (m_parser.isSet(m_opts.canbusFdOpt))
    connectionManager.canBus()->setCanFD(true);

  connectionManager.connectDevice();
}

#else

/**
 * @brief GPL build stub: shortcut verification always succeeds.
 */
bool CLI::verifyShortcutProject() const
{
  return true;
}

#endif

}  // namespace Misc
