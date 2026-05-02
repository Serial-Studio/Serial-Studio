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

#include "Misc/CLI.h"

#include <cstring>
#include <QApplication>
#include <QSettings>
#include <QTimer>

#include "API/Server.h"
#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/FileTransmission.h"
#include "Misc/TimerEvents.h"
#include "SerialStudio.h"
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
  m_parser.addOption(m_opts.versionOpt);
  m_parser.addOption(m_opts.resetOpt);
  m_parser.addOption(m_opts.fullscreenOpt);
  m_parser.addOption(m_opts.headlessOpt);
  m_parser.addOption(m_opts.apiServerOpt);
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
  m_parser.addOption(m_opts.activateOpt);
  m_parser.addOption(m_opts.deactivateOpt);
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

#ifdef BUILD_COMMERCIAL
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

  return ProcessResult::Continue;
}

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
  return m_parser.isSet(m_opts.projectOpt) ? m_parser.value(m_opts.projectOpt) : QString();
}

//---------------------------------------------------------------------------------------------------
// Project/auto-connect/visualization apply
//---------------------------------------------------------------------------------------------------

/**
 * @brief Applies CLI project/quick-plot mode and schedules runtime auto-connect.
 */
void CLI::applyProjectAndAutoConnect(QApplication& app)
{
  if (apiServerEnabled())
    API::Server::instance().setEnabled(true);

  if (m_parser.isSet(m_opts.projectOpt)) {
    QString projectPath = m_parser.value(m_opts.projectOpt);
    AppState::instance().setOperationMode(SerialStudio::ProjectFile);
    DataModel::ProjectModel::instance().openJsonFile(projectPath);
  }

  else if (quickPlot())
    AppState::instance().setOperationMode(SerialStudio::QuickPlot);

  if (!runtimeMode())
    return;

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

/**
 * @brief Applies CLI dashboard FPS and point-count options if set.
 */
void CLI::applyVisualizationOptions()
{
  if (m_parser.isSet(m_opts.fpsOpt)) {
    bool ok;
    auto fps = m_parser.value(m_opts.fpsOpt).toUInt(&ok);
    if (ok)
      Misc::TimerEvents::instance().setFPS(fps);
  }

  if (m_parser.isSet(m_opts.pointsOpt)) {
    bool ok;
    auto points = m_parser.value(m_opts.pointsOpt).toUInt(&ok);
    if (ok)
      UI::Dashboard::instance().setPoints(points);
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
  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::UART);

  if (m_parser.isSet(m_opts.uartOpt)) {
    const QString device = m_parser.value(m_opts.uartOpt);
    IO::ConnectionManager::instance().uart()->registerDevice(device);
  }

  if (m_parser.isSet(m_opts.baudOpt)) {
    bool ok               = false;
    const qint32 baudRate = m_parser.value(m_opts.baudOpt).toInt(&ok);
    if (!ok)
      qWarning() << "Invalid baud rate:" << m_parser.value(m_opts.baudOpt);
    else
      IO::ConnectionManager::instance().uart()->setBaudRate(baudRate);
  }

  IO::ConnectionManager::instance().connectDevice();
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

  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::Network);
  IO::ConnectionManager::instance().network()->setTcpSocket();
  IO::ConnectionManager::instance().network()->setRemoteAddress(parts[0]);
  IO::ConnectionManager::instance().network()->setTcpPort(port);
  IO::ConnectionManager::instance().connectDevice();
}

namespace {

/**
 * @brief Applies the optional --udp-remote spec to the active network driver.
 */
void applyUdpRemote(const QString& udpRemote)
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

}  // namespace

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

  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::Network);
  IO::ConnectionManager::instance().network()->setUdpSocket();
  IO::ConnectionManager::instance().network()->setUdpLocalPort(localPort);

  if (m_parser.isSet(m_opts.udpMulticastOpt))
    IO::ConnectionManager::instance().network()->setUdpMulticast(true);

  if (m_parser.isSet(m_opts.udpRemoteOpt))
    applyUdpRemote(m_parser.value(m_opts.udpRemoteOpt));

  IO::ConnectionManager::instance().connectDevice();
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

  if (deleteBtn != nullptr && box.clickedButton() == deleteBtn)
    Misc::ShortcutGenerator::instance().deleteShortcut(shortcutPath);

  return false;
}

namespace {

/**
 * @brief Splits a comma-separated taskbar pin list into a trimmed QStringList.
 */
QStringList splitTaskbarButtonIds(const QString& raw)
{
  QStringList ids;
  const auto parts = raw.split(QLatin1Char(','), Qt::SkipEmptyParts);
  ids.reserve(parts.size());
  for (const auto& p : parts)
    ids.append(p.trimmed());

  return ids;
}

}  // namespace

/**
 * @brief Applies taskbar visibility/pinning settings for operator runtime mode.
 */
void CLI::applyOperatorTaskbarSettings()
{
  auto& tbs = UI::TaskbarSettings::instance();
  if (m_parser.isSet(m_opts.taskbarModeOpt)) {
    const QString mode = m_parser.value(m_opts.taskbarModeOpt).toLower();
    tbs.setTaskbarHidden(mode == QStringLiteral("hidden"));
    tbs.setAutohide(mode == QStringLiteral("autohide"));
  } else {
    tbs.setTaskbarHidden(false);
    tbs.setAutohide(false);
  }

  if (m_parser.isSet(m_opts.taskbarButtonsOpt))
    tbs.setPinnedButtons(splitTaskbarButtonIds(m_parser.value(m_opts.taskbarButtonsOpt)));
}

/**
 * @brief Configures runtime/operator-mode export, dashboard, and taskbar overrides.
 */
void CLI::applyOperatorRuntimeSettings()
{
  CSV::Export::instance().setSettingsPersistent(false);
  MDF4::Export::instance().setSettingsPersistent(false);
  Sessions::Export::instance().setSettingsPersistent(false);
  Console::Export::instance().setSettingsPersistent(false);
  UI::Dashboard::instance().setSettingsPersistent(false);
  UI::TaskbarSettings::instance().setSettingsPersistent(false);

  CSV::Export::instance().setExportEnabled(m_parser.isSet(m_opts.csvExportOpt));
  MDF4::Export::instance().setExportEnabled(m_parser.isSet(m_opts.mdfExportOpt));
  Sessions::Export::instance().setExportEnabled(m_parser.isSet(m_opts.sessionExportOpt));
  Console::Export::instance().setExportEnabled(m_parser.isSet(m_opts.consoleExportOpt));

  UI::Dashboard::instance().setTerminalEnabled(false);
  UI::Dashboard::instance().setNotificationLogEnabled(false);
  UI::Dashboard::instance().setShowActionPanel(m_parser.isSet(m_opts.actionsPanelOpt));
  IO::FileTransmission::instance().setRuntimeAccessAllowed(
    m_parser.isSet(m_opts.fileTransmissionOpt));

  applyOperatorTaskbarSettings();
}

/**
 * @brief Enables CSV/MDF4/session/console exporters when their CLI flags are set.
 */
void CLI::applyExportToggles()
{
  if (m_parser.isSet(m_opts.csvExportOpt))
    CSV::Export::instance().setExportEnabled(true);

  if (m_parser.isSet(m_opts.mdfExportOpt))
    MDF4::Export::instance().setExportEnabled(true);

  if (m_parser.isSet(m_opts.sessionExportOpt))
    Sessions::Export::instance().setExportEnabled(true);

  if (m_parser.isSet(m_opts.consoleExportOpt))
    Console::Export::instance().setExportEnabled(true);
}

//---------------------------------------------------------------------------------------------------
// Commercial: license activate / deactivate
//---------------------------------------------------------------------------------------------------

/**
 * @brief Activates a license key against the Lemon Squeezy API and exits.
 */
int CLI::activateLicense(QApplication& app, const QString& licenseKey)
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
 */
int CLI::deactivateLicense(QApplication& app)
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

//---------------------------------------------------------------------------------------------------
// Commercial: Modbus helpers
//---------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Parses a Modbus register spec string and registers it with the Modbus driver.
 */
void applyModbusRegister(const QString& spec)
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
 * @brief Applies the Modbus parity option to the active Modbus driver.
 */
void applyModbusParity(const QString& parity)
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
 * @brief Applies the Modbus data-bits option to the active Modbus driver.
 */
void applyModbusDataBits(const QString& dataBits)
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
 * @brief Applies the Modbus stop-bits option to the active Modbus driver.
 */
void applyModbusStopBits(const QString& stopBits)
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
 * @brief Applies the Modbus slave option to the active Modbus driver.
 */
void applyModbusSlave(const QCommandLineParser& parser, const QCommandLineOption& opt)
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
 * @brief Applies the Modbus poll-interval option to the active Modbus driver.
 */
void applyModbusPoll(const QCommandLineParser& parser, const QCommandLineOption& opt)
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
 * @brief Applies the Modbus serial baud-rate option to the active Modbus driver.
 */
void applyModbusBaud(const QCommandLineParser& parser, const QCommandLineOption& opt)
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
void applyModbusRegisters(const QCommandLineParser& parser, const QCommandLineOption& opt)
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
 * @brief Parses a Modbus TCP host[:port] string.
 */
bool parseModbusTcpAddress(const QString& tcpAddress, QString& host, quint16& port)
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

}  // namespace

/**
 * @brief Configures and connects a Modbus RTU bus from CLI options.
 */
void CLI::setupModbusRtuConnection()
{
  const QString portPath = m_parser.value(m_opts.modbusRtuOpt);
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
  IO::ConnectionManager::instance().connectDevice();
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

  IO::ConnectionManager::instance().setBusType(SerialStudio::BusType::ModBus);
  IO::ConnectionManager::instance().modbus()->setProtocolIndex(1);
  IO::ConnectionManager::instance().modbus()->setHost(host);
  IO::ConnectionManager::instance().modbus()->setPort(port);

  applyModbusSlave(m_parser, m_opts.modbusSlaveOpt);
  applyModbusPoll(m_parser, m_opts.modbusPollOpt);
  applyModbusRegisters(m_parser, m_opts.modbusRegisterOpt);
  IO::ConnectionManager::instance().connectDevice();
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

  if (interfaceIndex < 0) {
    qWarning() << "CAN interface" << interfaceName << "not found for plugin" << plugin;
    qWarning() << "Available interfaces:" << availableInterfaces.join(", ");
    return;
  }

  IO::ConnectionManager::instance().canBus()->setInterfaceIndex(interfaceIndex);

  if (m_parser.isSet(m_opts.canbusBitrateOpt)) {
    bool ok;
    quint32 bitrate = m_parser.value(m_opts.canbusBitrateOpt).toUInt(&ok);
    if (ok && bitrate > 0)
      IO::ConnectionManager::instance().canBus()->setBitrate(bitrate);
    else
      qWarning() << "Invalid CAN bus bitrate:" << m_parser.value(m_opts.canbusBitrateOpt);
  }

  if (m_parser.isSet(m_opts.canbusFdOpt))
    IO::ConnectionManager::instance().canBus()->setCanFD(true);

  IO::ConnectionManager::instance().connectDevice();
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
