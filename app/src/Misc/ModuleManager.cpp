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

#include "Misc/ModuleManager.h"

#include <QSimpleUpdater.h>

// code-verify off
#include <iostream>
#include <memory>
// code-verify on

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>
#include <QStringList>
#include <QSysInfo>

#if defined(Q_OS_WIN)
// clang-format off
#  include <windows.h>
#  include <appmodel.h>
// clang-format on
#endif

#include "API/Mirror/MirrorPublisher.h"
#include "API/Mirror/MirrorSession.h"
#include "API/ProcessLauncher.h"
#include "API/Server.h"
#include "AppInfo.h"
#include "AppState.h"
#include "Benchmark/BenchmarkRunner.h"
#include "Console/Export.h"
#include "Console/Handler.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/Editors/ControlScriptEditor.h"
#include "DataModel/Editors/FrameParserModel.h"
#include "DataModel/Editors/JsCodeEditor.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Importers/ProtoImporter.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/RowFilterProxy.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "IO/ConnectionManager.h"
#include "IO/FileTransmission.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "Misc/BackupManager.h"
#include "Misc/CommonFonts.h"
#include "Misc/ConnectionDiagnostics.h"
#include "Misc/CrashTracker.h"
#include "Misc/DemoLauncher.h"
#include "Misc/Examples.h"
#include "Misc/ExtensionManager.h"
#include "Misc/GraphicsBackend.h"
#include "Misc/HelpCenter.h"
#include "Misc/HighDpiScaling.h"
#include "Misc/IconEngine.h"
#include "Misc/IconRegistry.h"
#include "Misc/ProblemCenter.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "Platform/AppPlatform.h"
#include "SerialStudio.h"
#include "SessionContext.h"
#include "UI/AlarmMonitor.h"
#include "UI/CommandRegistry.h"
#include "UI/Dashboard.h"
#include "UI/DashboardWidget.h"
#include "UI/Taskbar.h"
#include "UI/TaskbarSettings.h"
#include "UI/WidgetExtensions.h"
#include "UI/Widgets/Accelerometer.h"
#include "UI/Widgets/Bar.h"
#include "UI/Widgets/Compass.h"
#include "UI/Widgets/DataGrid.h"
#include "UI/Widgets/ExtensionData.h"
#include "UI/Widgets/FFTPlot.h"
#include "UI/Widgets/Gauge.h"
#include "UI/Widgets/GPS.h"
#include "UI/Widgets/Gyroscope.h"
#include "UI/Widgets/LEDPanel.h"
#include "UI/Widgets/Meter.h"
#include "UI/Widgets/MultiPlot.h"
#include "UI/Widgets/Plot.h"
#include "UI/Widgets/PlotAreaFill.h"
#include "UI/Widgets/PlotCurve.h"
#include "UI/Widgets/Terminal.h"
#include "UI/Widgets/WebView.h"
#include "UI/WindowManager.h"

#ifdef BUILD_COMMERCIAL
#  include "AI/Assistant.h"
#  include "DataModel/Editors/PainterCodeEditor.h"
#  include "DataModel/Importers/DBCImporter.h"
#  include "DataModel/Importers/ModbusMapImporter.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/MachineID.h"
#  include "Licensing/OfflineLicense.h"
#  include "Licensing/Trial.h"
#  include "Misc/ShortcutGenerator.h"
#  include "MQTT/Publisher.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#  include "UI/ImageProvider.h"
#  include "UI/Widgets/AudioExport.h"
#  include "UI/Widgets/ImageExport.h"
#  include "UI/Widgets/ImageView.h"
#  include "UI/Widgets/Painter.h"
#  include "UI/Widgets/Plot3D.h"
#  include "UI/Widgets/Waterfall.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

//--------------------------------------------------------------------------------------------------
// Message handler
//--------------------------------------------------------------------------------------------------

/**
 * @brief Custom message handler for Qt debug, warning, critical, and fatal messages.
 */
static void MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  (void)context;
  if (msg.isEmpty())
    return;

  if (type == QtInfoMsg && msg.startsWith("OpenType support missing"))
    return;

  if (type == QtWarningMsg) {
    if (msg.startsWith("Qt was built without Direct3D 12 support"))
      return;

    if (msg.contains("setGeometry"))
      return;

    if (msg.contains("Retrying to obtain clipboard."))
      return;

    if (msg.contains("The following paths were searched for Qt WebEngine locales"))
      return;

    if (msg.contains("attempting to set invalid range for value axis:"))
      return;

    if (msg.contains("Invalid path data; path truncated."))
      return;

    if (msg.contains("QSocketNotifier::Exception is not supported on iOS"))
      return;

    if (msg.contains("QQmlVMEMetaObject: Internal error"))
      return;

    if (msg.startsWith("QThread::start: Failed to set thread priority"))
      return;
  }

  QString message;
  if (context.function)
    message = QStringLiteral("%1 - %2").arg(context.function, msg);
  else
    message = msg;

  const bool useAnsiColors = Console::Handler::instance().ansiColorsEnabled();
  const QString output     = Widgets::Terminal::formatDebugMessage(type, message, useAnsiColors);
  if (output.isEmpty())
    return;

  // code-verify off
  std::cout << Widgets::Terminal::formatDebugMessage(type, message, false).toStdString()
            << std::endl;
  // code-verify on

  QMetaObject::invokeMethod(
    &Console::Handler::instance(),
    [output]() { Console::Handler::instance().displayDebugData(output + "\n"); },
    Qt::QueuedConnection);

  const bool isCritical = (type == QtCriticalMsg || type == QtFatalMsg);
  const bool isWarning  = (type == QtWarningMsg);
  if (!isCritical && !isWarning)
    return;

  auto& nc = DataModel::NotificationCenter::instance();
  if (isWarning && !nc.routeWarningsToNotifications())
    return;

  const QString channel = QStringLiteral("System");
  const QString title   = isCritical ? QObject::tr("Critical") : QObject::tr("Warning");

  QMetaObject::invokeMethod(
    &nc,
    [level = isCritical ? 2 : 1, channel, title, msg]() {
      DataModel::NotificationCenter::instance().post(level, channel, title, msg);
    },
    Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ModuleManager singleton.
 */
Misc::ModuleManager::ModuleManager()
  : m_headless(false)
  , m_quitHandled(false)
  , m_ephemeralSession(false)
  , m_automaticUpdates(m_settings.value("App/CheckForUpdates", true).toBool())
  , m_performanceMode(m_settings.value("App/PerformanceMode", true).toBool())
  , m_inhibitIdleSleep(m_settings.value("App/InhibitIdleSleep", true).toBool())
{
  (void)Misc::Translator::instance();

  connect(&m_engine, &QQmlApplicationEngine::quit, this, &Misc::ModuleManager::onQuit);

  connect(qApp, &QCoreApplication::aboutToQuit, this, &Misc::ModuleManager::onQuit);
}

//--------------------------------------------------------------------------------------------------
// Settings access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether automatic update checks are enabled.
 */
bool Misc::ModuleManager::automaticUpdates() const noexcept
{
  return m_automaticUpdates;
}

/**
 * @brief Returns whether OS performance hints are applied at startup; consumed by
 *        AppPlatform::prepareEnvironment() before this singleton exists.
 */
bool Misc::ModuleManager::performanceMode() const noexcept
{
  return m_performanceMode;
}

/**
 * @brief Returns whether system/display idle sleep is inhibited at startup; consumed by
 *        AppPlatform::inhibitIdleSleep().
 */
bool Misc::ModuleManager::inhibitIdleSleep() const noexcept
{
  return m_inhibitIdleSleep;
}

/**
 * @brief Enables headless mode, suppressing QML and GUI loading on init.
 */
void Misc::ModuleManager::setHeadless(const bool headless)
{
  m_headless = headless;
}

/**
 * @brief Marks the session ephemeral (operator runtime) before the pinned order runs, so the
 *        flag reaches AppState ahead of restoreLastProject() without main.cpp reaching a
 *        session singleton before the composition root exists (spec 0039 M2).
 */
void Misc::ModuleManager::setEphemeralSession(const bool ephemeral)
{
  m_ephemeralSession = ephemeral;
}

/**
 * @brief Enables/disables automatic update checks and persists the setting.
 */
void Misc::ModuleManager::setAutomaticUpdates(const bool enabled)
{
  if (m_automaticUpdates != enabled) {
    m_automaticUpdates = enabled;
    m_settings.setValue("App/CheckForUpdates", m_automaticUpdates);
    Q_EMIT automaticUpdatesChanged();
  }
}

/**
 * @brief Enables/disables startup performance hints; takes effect on the next launch.
 */
void Misc::ModuleManager::setPerformanceMode(const bool enabled)
{
  if (m_performanceMode != enabled) {
    m_performanceMode = enabled;
    m_settings.setValue("App/PerformanceMode", m_performanceMode);
    Q_EMIT performanceModeChanged();
  }
}

/**
 * @brief Enables/disables startup idle-sleep inhibition; takes effect on the next launch.
 */
void Misc::ModuleManager::setInhibitIdleSleep(const bool enabled)
{
  if (m_inhibitIdleSleep != enabled) {
    m_inhibitIdleSleep = enabled;
    m_settings.setValue("App/InhibitIdleSleep", m_inhibitIdleSleep);
    Q_EMIT inhibitIdleSleepChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Engine access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a reference to the QML application engine.
 */
const QQmlApplicationEngine& Misc::ModuleManager::engine() const noexcept
{
  return m_engine;
}

/**
 * @brief Returns whether the QSimpleUpdater auto-updater is enabled at build time.
 */
bool Misc::ModuleManager::autoUpdaterEnabled() const noexcept
{
#ifdef DISABLE_QSU
  return false;
#else
  return true;
#endif
}

//--------------------------------------------------------------------------------------------------
// Application shutdown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stops every active module and exits the application event loop.
 */
void Misc::ModuleManager::onQuit()
{
  if (m_quitHandled)
    return;

  m_quitHandled = true;

  qInstallMessageHandler(nullptr);

  DataModel::ControlScript::instance().shutdown();

  Misc::ExtensionManager::instance().stopAllPlugins();
  Misc::TimerEvents::instance().stopTimers();

  DataModel::ProjectModel::instance().flushAutoSave();

  Console::Export::instance().closeFile();
  CSV::Export::instance().closeFile();
  CSV::Player::instance().closeFile();
  MDF4::Export::instance().closeFile();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().closeFile();
  Sessions::Player::instance().closeFile();
  Sessions::Player::instance().shutdown();
  Sessions::DatabaseManager::instance().closeDatabase(false);
  Sessions::DatabaseManager::instance().shutdown();
  Widgets::AudioExport::instance().closeAllSessions();
#endif
  IO::ConnectionManager::instance().disconnectAllDevices();
  API::Server::instance().removeConnection();

#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer::instance().setEnabled(false);
#endif

  stopFrameConsumerWorkers();

  Misc::CrashTracker::instance().markCleanExit();

  qApp->exit(0);
}

/**
 * @brief Joins every FrameConsumer worker while qApp is alive: SessionContext::shutdown() frees
 *        the core modules inside main(), so a worker still ticking at static destruction
 *        dereferences freed modules (Windows 0xC0000374). stopWorker() stays idempotent for the
 *        destructor's repeat call.
 */
void Misc::ModuleManager::stopFrameConsumerWorkers()
{
  Console::Export::instance().stopWorker();
  CSV::Export::instance().stopWorker();
  MDF4::Export::instance().stopWorker();
  API::Server::instance().stopWorker();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().stopWorker();
  MQTT::Publisher::instance().stopWorker();
  Widgets::AudioExport::instance().stopWorker();
  Widgets::ImageExport::instance().stopWorker();
#endif
}

//--------------------------------------------------------------------------------------------------
// Updater configuration
//--------------------------------------------------------------------------------------------------

#if defined(Q_OS_LINUX)
/**
 * @brief Returns the updates.json architecture suffix ("x64"/"arm64") for the running CPU,
 *        or an empty string on architectures the release feed does not carry.
 */
[[nodiscard]] static QString updaterArchSuffix()
{
  const auto arch = QSysInfo::buildCpuArchitecture();
  if (arch == QStringLiteral("x86_64") || arch == QStringLiteral("i386"))
    return QStringLiteral("x64");

  if (arch == QStringLiteral("arm64") || arch == QStringLiteral("aarch64"))
    return QStringLiteral("arm64");

  return QString();
}
#endif

/**
 * @brief Maps the CI-stamped ss-config.json (packageType + arch, shipped beside the executable)
 *        to an appcast platform key; empty when the stamp is absent, unreadable, unknown or
 *        stamped for a different platform (which must degrade, never offer a foreign package).
 */
[[nodiscard]] static QString packageStampKey()
{
  constexpr qint64 kMaxStampBytes = 4096;

  QStringList candidates = {QCoreApplication::applicationDirPath()
                            + QStringLiteral("/ss-config.json")};
#if defined(Q_OS_MACOS)
  candidates.append(QCoreApplication::applicationDirPath()
                    + QStringLiteral("/../Resources/ss-config.json"));
#endif

  for (const auto& path : std::as_const(candidates)) {
    QFile file(path);
    if (file.size() > kMaxStampBytes || !file.open(QIODevice::ReadOnly))
      continue;

    const auto stamp = QJsonDocument::fromJson(file.readAll()).object();
    const auto type  = stamp.value(QStringLiteral("packageType")).toString();

#if defined(Q_OS_WIN)
    if (type == QStringLiteral("msi"))
      return QStringLiteral("windows-msi");

    if (type == QStringLiteral("portable"))
      return QStringLiteral("windows-portable");

    if (type == QStringLiteral("msix"))
      return QStringLiteral("windows-msix");
#elif defined(Q_OS_MACOS)
    if (type == QStringLiteral("dmg"))
      return QStringLiteral("osx-dmg");
#elif defined(Q_OS_LINUX)
    const auto arch = stamp.value(QStringLiteral("arch")).toString();

    QString suffix;
    if (arch == QStringLiteral("x86_64"))
      suffix = QStringLiteral("x64");
    else if (arch == QStringLiteral("arm64"))
      suffix = QStringLiteral("arm64");

    const bool linux_type = (type == QStringLiteral("appimage") || type == QStringLiteral("deb")
                             || type == QStringLiteral("rpm"));
    if (linux_type && !suffix.isEmpty())
      return QStringLiteral("linux-%1-%2").arg(type, suffix);
#endif
  }

  return QString();
}

/**
 * @brief Detects the packaging at runtime when no stamp resolved: the AppImage runtime
 *        environment on Linux, the Win32 package identity (Microsoft Store) on Windows.
 */
[[nodiscard]] static QString probedPackageKey()
{
#if defined(Q_OS_LINUX)
  const auto suffix = updaterArchSuffix();
  if (qEnvironmentVariableIsSet("APPIMAGE") && !suffix.isEmpty())
    return QStringLiteral("linux-appimage-%1").arg(suffix);
#elif defined(Q_OS_WIN)
  UINT32 length = 0;
  if (GetCurrentPackageFullName(&length, nullptr) == ERROR_INSUFFICIENT_BUFFER)
    return QStringLiteral("windows-msix");
#endif

  return QString();
}

/**
 * @brief Configures QSimpleUpdater defaults and resolves the packaging-aware appcast key:
 *        CI stamp first, runtime probing second, the legacy per-OS keys as the last tier.
 */
void Misc::ModuleManager::configureUpdater()
{
  if (!autoUpdaterEnabled())
    return;

  QSimpleUpdater::getInstance()->setNotifyOnUpdate(APP_UPDATER_URL, true);
  QSimpleUpdater::getInstance()->setNotifyOnFinish(APP_UPDATER_URL, false);
  QSimpleUpdater::getInstance()->setMandatoryUpdate(APP_UPDATER_URL, false);

  QString key = packageStampKey();
  if (key.isEmpty())
    key = probedPackageKey();

#if defined(Q_OS_LINUX)
  if (key.isEmpty()) {
    const auto suffix = updaterArchSuffix();
    if (!suffix.isEmpty())
      key = QStringLiteral("linux-%1").arg(suffix);
  }
#endif

  if (!key.isEmpty())
    QSimpleUpdater::getInstance()->setPlatformKey(APP_UPDATER_URL, key);
}

//--------------------------------------------------------------------------------------------------
// Module registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers Serial Studio's custom QML types with the engine.
 */
void Misc::ModuleManager::registerQmlTypes()
{
  qmlRegisterType<DataModel::RowFilterProxy>("SerialStudio", 1, 0, "RowFilterProxy");
  qmlRegisterType<Widgets::Bar>("SerialStudio", 1, 0, "BarModel");
  qmlRegisterType<Widgets::GPS>("SerialStudio", 1, 0, "GPSWidget");
  qmlRegisterType<Widgets::Plot>("SerialStudio", 1, 0, "PlotModel");
  qmlRegisterType<Widgets::PlotAreaFill>("SerialStudio", 1, 0, "PlotAreaFill");
  qmlRegisterType<Widgets::PlotCurve>("SerialStudio", 1, 0, "PlotCurve");
  qmlRegisterType<Widgets::Gauge>("SerialStudio", 1, 0, "GaugeModel");
  qmlRegisterType<Widgets::Compass>("SerialStudio", 1, 0, "CompassModel");
  qmlRegisterType<Widgets::FFTPlot>("SerialStudio", 1, 0, "FFTPlotModel");
  qmlRegisterType<Widgets::DataGrid>("SerialStudio", 1, 0, "DataGridWidget");
  qmlRegisterType<Widgets::LEDPanel>("SerialStudio", 1, 0, "LEDPanelModel");
  qmlRegisterType<Widgets::Terminal>("SerialStudio", 1, 0, "TerminalWidget");
  qmlRegisterType<Widgets::MultiPlot>("SerialStudio", 1, 0, "MultiPlotModel");
  qmlRegisterType<Widgets::Gyroscope>("SerialStudio", 1, 0, "GyroscopeModel");
  qmlRegisterType<Widgets::Accelerometer>("SerialStudio", 1, 0, "AccelerometerModel");
  qmlRegisterType<Widgets::Meter>("SerialStudio", 1, 0, "MeterModel");
  qmlRegisterType<Widgets::WebView>("SerialStudio", 1, 0, "WebViewWidget");
  qmlRegisterType<Widgets::ExtensionData>("SerialStudio", 1, 0, "ExtensionDataModel");

#ifdef BUILD_COMMERCIAL
  qmlRegisterType<Widgets::Plot3D>("SerialStudio", 1, 0, "Plot3DWidget");
  qmlRegisterType<Widgets::ImageView>("SerialStudio", 1, 0, "ImageViewModel");
  qmlRegisterType<Widgets::Waterfall>("SerialStudio", 1, 0, "WaterfallModel");
  qmlRegisterType<Widgets::Painter>("SerialStudio", 1, 0, "PainterWidget");
  qmlRegisterType<DataModel::PainterCodeEditor>("SerialStudio", 1, 0, "PainterCodeEditor");
#endif

  qmlRegisterType<DataModel::JsCodeEditor>("SerialStudio", 1, 0, "JsCodeEditor");
  qmlRegisterType<DataModel::ControlScriptEditor>("SerialStudio", 1, 0, "ControlScriptEditor");
  qmlRegisterType<DataModel::ProjectModel>("SerialStudio", 1, 0, "ProjectModel");
  qmlRegisterType<DataModel::ProjectEditor>("SerialStudio", 1, 0, "ProjectEditor");
  qmlRegisterType<DataModel::OutputCodeEditor>("SerialStudio", 1, 0, "OutputCodeEditor");
  qmlRegisterType<DataModel::FrameParserModel>("SerialStudio", 1, 0, "FrameParserModel");

  qmlRegisterType<UI::DashboardWidget>("SerialStudio", 1, 0, "DashboardWidget");

  qmlRegisterType<UI::Taskbar>("SerialStudio.UI", 1, 0, "TaskBar");
  qmlRegisterType<UI::WindowManager>("SerialStudio.UI", 1, 0, "WindowManager");

  qmlRegisterSingletonType<SerialStudio>(
    "SerialStudio", 1, 0, "SerialStudio", [](QQmlEngine*, QJSEngine*) -> QObject* {
      return new SerialStudio();
    });
}

//--------------------------------------------------------------------------------------------------
// QML initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes all application modules and loads the root QML file.
 */
void Misc::ModuleManager::initializeQmlInterface()
{
#ifdef ENABLE_GRPC
  (void)API::GRPC::GRPCServer::instance();
  const bool grpcAvailable = true;
#else
  const bool grpcAvailable = false;
#endif

  Misc::TimerEvents::instance().startTimers();

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          &m_engine,
          &QQmlApplicationEngine::retranslate);

  setupCrossModuleConnections();

  qInstallMessageHandler(MessageHandler);
  qAddPostRoutine([]() { qInstallMessageHandler(nullptr); });

  Platform::AppPlatform::registerIngestThreadWithMmcss();

  const auto c = m_engine.rootContext();
  registerCoreContextProperties(c);
#ifdef BUILD_COMMERCIAL
  registerCommercialContextProperties(c);
#endif
  registerAppMetadataProperties(c, grpcAvailable);

  if (!m_headless)
    registerImageProvidersAndLoadQml();

#ifdef BUILD_COMMERCIAL
  auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  if (lemonSqueezy.isOnlineActivated())
    QMetaObject::invokeMethod(&lemonSqueezy, &Licensing::LemonSqueezy::validate);
  else
    QMetaObject::invokeMethod(&lemonSqueezy, &Licensing::LemonSqueezy::revalidateCachedLicense);
#endif
}

/**
 * @brief Constructs every core singleton in a pinned, dependency-verified order (ctor-edge
 *        proof in doc/claude/specs/0001-composition-root/). Licensing sits right after
 *        Translator (its ctors emit tr() strings) so the CommercialToken is final-for-startup
 *        before any entitlement consumer constructs or restores state (spec 0042).
 */
void Misc::ModuleManager::instantiateCoreModules()
{
  auto& ctx = SessionContext::current();

  (void)Misc::Translator::instance();
#ifdef BUILD_COMMERCIAL
  (void)Licensing::MachineID::instance();
  (void)Licensing::LemonSqueezy::instance();
  (void)Licensing::OfflineLicense::instance();
  (void)Licensing::Trial::instance();
#endif
  (void)Misc::TimerEvents::instance();
  (void)Misc::CommonFonts::instance();
  (void)Misc::WorkspaceManager::instance();
  ctx.adoptNotifications(SessionContext::create<DataModel::NotificationCenter>());
  (void)Misc::ProblemCenter::instance();
  (void)Misc::ConnectionDiagnostics::instance();
  (void)Misc::ThemeManager::instance();
  (void)Misc::ExtensionManager::instance();
  (void)DataModel::ControlScript::instance();
  ctx.adoptProjectModel(SessionContext::create<DataModel::ProjectModel>());
  ctx.adoptAppState(SessionContext::create<AppState>());
  ctx.adoptFrameBuilder(SessionContext::create<DataModel::FrameBuilder>());
  ctx.adoptConnectionManager(SessionContext::create<IO::ConnectionManager>());
  ctx.adoptConsole(SessionContext::create<Console::Handler>());
  (void)API::Server::instance();
  (void)CSV::Player::instance();
  (void)MDF4::Player::instance();
#ifdef BUILD_COMMERCIAL
  (void)Sessions::Player::instance();
  (void)Sessions::Export::instance();
  (void)Sessions::DatabaseManager::instance();
  (void)MQTT::Publisher::instance();
#endif
  (void)CSV::Export::instance();
  (void)MDF4::Export::instance();
  (void)Console::Export::instance();
  ctx.adoptFrameParser(SessionContext::create<DataModel::FrameParser>());
  (void)UI::WidgetExtensions::instance();
  ctx.adoptDashboard(SessionContext::create<UI::Dashboard>());
}

/**
 * @brief Wires the modules a headless session re-record needs (spec 0044). Such a run builds
 *        the pinned order without the QML interface, so setupCrossModuleConnections() never
 *        runs and neither the frame builder nor the session exporter would see a frame.
 */
void Misc::ModuleManager::setupHeadlessSessionConnections()
{
  DataModel::FrameBuilder::instance().setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().setupExternalConnections();
#endif
}

/**
 * @brief Wires the per-session problem reset on the false-to-true connection edge only
 *        (connectedChanged is also forwarded from config edits). A full run replaces every
 *        checker's findings wholesale, so no separate clear is needed and the dedup keys
 *        survive: unchanged standing findings do not re-notify on every connect.
 */
static void wireProblemCenterSessionReset(IO::ConnectionManager* manager,
                                          Misc::ProblemCenter& problemCenter)
{
  auto wasConnected = std::make_shared<bool>(manager->isConnected());
  QObject::connect(manager,
                   &IO::ConnectionManager::connectedChanged,
                   &problemCenter,
                   [&problemCenter, manager, wasConnected] {
                     const bool connected = manager->isConnected();
                     const bool rising    = connected && !*wasConnected;
                     *wasConnected        = connected;
                     if (!rising)
                       return;

                     problemCenter.runNow();
                   });
}

/**
 * @brief Wires inter-module signals and runs each module's setupExternalConnections. The
 *        session context is published right after the pinned order and before any wiring,
 *        so an injected class can never be constructed before it exists (spec 0039).
 */
void Misc::ModuleManager::setupCrossModuleConnections()
{
  instantiateCoreModules();
  (void)SessionContext::current();

  auto* appState             = &AppState::instance();
  auto* ioManager            = &IO::ConnectionManager::instance();
  auto* pluginsBridge        = &API::Server::instance();
  auto* uiDashboard          = &UI::Dashboard::instance();
  auto* notificationCenter   = &DataModel::NotificationCenter::instance();
  auto* miscExtensionManager = &Misc::ExtensionManager::instance();
  auto* miscThemeManager     = &Misc::ThemeManager::instance();

  appState->setEphemeralSession(m_ephemeralSession);
  appState->setupExternalConnections();
  CSV::Export::instance().setupExternalConnections();
  ioManager->setupExternalConnections();
  MDF4::Export::instance().setupExternalConnections();
  DataModel::FrameParser::instance().setupExternalConnections();
  DataModel::ProjectModel::instance().setupExternalConnections();
  Misc::BackupManager::instance().setupExternalConnections();
  API::ProcessLauncher::instance().setupExternalConnections();
  DataModel::FrameBuilder::instance().setupExternalConnections();
  DataModel::ControlScript::instance().setupExternalConnections();
  Console::Export::instance().setupExternalConnections();
  Console::Handler::instance().setupExternalConnections();
  IO::FileTransmission::instance().setupExternalConnections();
  UI::AlarmMonitor::instance().setupExternalConnections();
  auto& problemCenter = Misc::ProblemCenter::instance();
  problemCenter.setupExternalConnections();
  wireProblemCenterSessionReset(ioManager, problemCenter);
  Misc::ConnectionDiagnostics::instance().setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().setupExternalConnections();
  Sessions::DatabaseManager::instance().setupExternalConnections();
  MQTT::Publisher::instance().setupExternalConnections();
#endif

  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionInstalled,
          miscThemeManager,
          &Misc::ThemeManager::onExtensionInstalled);
  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionUninstalled,
          miscThemeManager,
          &Misc::ThemeManager::onExtensionUninstalled);

  auto* workspaceManager = &Misc::WorkspaceManager::instance();
  connect(workspaceManager,
          &Misc::WorkspaceManager::pathChanged,
          miscExtensionManager,
          &Misc::ExtensionManager::onWorkspacePathChanged);
  connect(workspaceManager,
          &Misc::WorkspaceManager::pathChanged,
          miscThemeManager,
          &Misc::ThemeManager::onWorkspacePathChanged);

  auto* widgetExtensions = &UI::WidgetExtensions::instance();
  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionInstalled,
          widgetExtensions,
          &UI::WidgetExtensions::rescan);
  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionUninstalled,
          widgetExtensions,
          &UI::WidgetExtensions::rescan);
  connect(workspaceManager,
          &Misc::WorkspaceManager::pathChanged,
          widgetExtensions,
          &UI::WidgetExtensions::rescan);
  widgetExtensions->rescan();

  connect(ioManager,
          &IO::ConnectionManager::connectedChanged,
          pluginsBridge,
          [pluginsBridge, ioManager]() {
            pluginsBridge->broadcastLifecycleEvent(ioManager->isConnected()
                                                     ? QStringLiteral("connected")
                                                     : QStringLiteral("disconnected"));
          });

  appState->restoreLastProject();

  miscExtensionManager->refreshRepositories();
  connect(uiDashboard,
          &UI::Dashboard::widgetCountChanged,
          miscExtensionManager,
          &Misc::ExtensionManager::onDashboardAvailableChanged);

  connect(uiDashboard,
          &UI::Dashboard::dataReset,
          notificationCenter,
          &DataModel::NotificationCenter::clearAll);

  (void)API::MirrorPublisher::instance();
  (void)API::MirrorSession::instance();
}

/**
 * @brief Registers every always-available C++ singleton as a QML context property.
 */
void Misc::ModuleManager::registerCoreContextProperties(QQmlContext* ctx)
{
  auto* ioManager = &IO::ConnectionManager::instance();

  ctx->setContextProperty("Cpp_AppState", &AppState::instance());
  ctx->setContextProperty("Cpp_Updater", m_headless ? nullptr : QSimpleUpdater::getInstance());
  ctx->setContextProperty("Cpp_IO_Serial", ioManager->uart());
  ctx->setContextProperty("Cpp_CSV_Export", &CSV::Export::instance());
  ctx->setContextProperty("Cpp_CSV_Player", &CSV::Player::instance());
  ctx->setContextProperty("Cpp_IO_Manager", ioManager);
  ctx->setContextProperty("Cpp_IO_Network", ioManager->network());
  ctx->setContextProperty("Cpp_MDF4_Export", &MDF4::Export::instance());
  ctx->setContextProperty("Cpp_MDF4_Player", &MDF4::Player::instance());
  ctx->setContextProperty("Cpp_Misc_ModuleManager", this);
  ctx->setContextProperty("Cpp_UI_Dashboard", &UI::Dashboard::instance());
  ctx->setContextProperty("Cpp_UI_TaskbarSettings", &UI::TaskbarSettings::instance());
  ctx->setContextProperty("Cpp_Console_Export", &Console::Export::instance());
  ctx->setContextProperty("Cpp_NativeWindow", &m_nativeWindow);
  ctx->setContextProperty("Cpp_API_Server", &API::Server::instance());
  ctx->setContextProperty("Cpp_API_Mirror", &API::MirrorSession::instance());
  ctx->setContextProperty("Cpp_Misc_Utilities", &Misc::Utilities::instance());
  ctx->setContextProperty("Cpp_IO_Bluetooth_LE", ioManager->bluetoothLE());
  ctx->setContextProperty("Cpp_ThemeManager", &Misc::ThemeManager::instance());
  ctx->setContextProperty("Cpp_Console_Handler", &Console::Handler::instance());
  ctx->setContextProperty("Cpp_Misc_Translator", &Misc::Translator::instance());
  ctx->setContextProperty("Cpp_JSON_ProjectModel", &DataModel::ProjectModel::instance());
  ctx->setContextProperty("Cpp_JSON_ProjectEditor", &DataModel::ProjectEditor::instance());
  ctx->setContextProperty("Cpp_ControlScript", &DataModel::ControlScript::instance());
  ctx->setContextProperty("Cpp_JSON_ProtoImporter", &DataModel::ProtoImporter::instance());
  ctx->setContextProperty("Cpp_JSON_FrameBuilder", &DataModel::FrameBuilder::instance());
  ctx->setContextProperty("Cpp_Notifications", &DataModel::NotificationCenter::instance());
  ctx->setContextProperty("Cpp_Misc_TimerEvents", &Misc::TimerEvents::instance());
  ctx->setContextProperty("Cpp_Misc_CommonFonts", &Misc::CommonFonts::instance());
  ctx->setContextProperty("Cpp_IO_FileTransmission", &IO::FileTransmission::instance());
  ctx->setContextProperty("Cpp_Misc_WorkspaceManager", &Misc::WorkspaceManager::instance());
  ctx->setContextProperty("Cpp_Examples", &Misc::Examples::instance());
  ctx->setContextProperty("Cpp_Misc_Demo", &Misc::DemoLauncher::instance());
  ctx->setContextProperty("Cpp_HelpCenter", &Misc::HelpCenter::instance());
  ctx->setContextProperty("Cpp_ExtensionManager", &Misc::ExtensionManager::instance());
  ctx->setContextProperty("Cpp_Misc_IconEngine", &Misc::IconEngine::instance());
  ctx->setContextProperty("Cpp_Misc_IconRegistry", &Misc::IconRegistry::instance());
  ctx->setContextProperty("Cpp_Misc_ProblemCenter", &Misc::ProblemCenter::instance());
  ctx->setContextProperty("Cpp_Misc_ConnectionDiagnostics",
                          &Misc::ConnectionDiagnostics::instance());
  ctx->setContextProperty("Cpp_UI_CommandRegistry", &UI::CommandRegistry::instance());
  ctx->setContextProperty("Cpp_UI_WidgetExtensions", &UI::WidgetExtensions::instance());
  ctx->setContextProperty("Cpp_Misc_GraphicsBackend", &Misc::GraphicsBackend::instance());
  ctx->setContextProperty("Cpp_Misc_HighDpiScaling", &Misc::HighDpiScaling::instance());
  ctx->setContextProperty("Cpp_Misc_CrashTracker", &Misc::CrashTracker::instance());
  ctx->setContextProperty("Cpp_Misc_BackupManager", &Misc::BackupManager::instance());
  ctx->setContextProperty("Cpp_Benchmark_Runner", &Benchmark::BenchmarkRunner::instance());
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Registers Pro-only C++ singletons as QML context properties.
 */
void Misc::ModuleManager::registerCommercialContextProperties(QQmlContext* ctx)
{
  auto* ioManager = &IO::ConnectionManager::instance();

  ctx->setContextProperty("Cpp_IO_Audio", ioManager->audio());
  ctx->setContextProperty("Cpp_IO_CANBus", ioManager->canBus());
  ctx->setContextProperty("Cpp_IO_Modbus", ioManager->modbus());
  ctx->setContextProperty("Cpp_IO_USB", ioManager->usb());
  ctx->setContextProperty("Cpp_IO_HID", ioManager->hid());
  ctx->setContextProperty("Cpp_IO_Process", ioManager->process());
  ctx->setContextProperty("Cpp_IO_Mqtt", ioManager->mqtt());
  ctx->setContextProperty("Cpp_MQTT_Publisher", &MQTT::Publisher::instance());
  ctx->setContextProperty("Cpp_JSON_DBCImporter", &DataModel::DBCImporter::instance());
  ctx->setContextProperty("Cpp_JSON_ModbusMapImporter", &DataModel::ModbusMapImporter::instance());
  ctx->setContextProperty("Cpp_Licensing_Trial", &Licensing::Trial::instance());
  ctx->setContextProperty("Cpp_Licensing_LemonSqueezy", &Licensing::LemonSqueezy::instance());
  ctx->setContextProperty("Cpp_Licensing_OfflineLicense", &Licensing::OfflineLicense::instance());
  ctx->setContextProperty("Cpp_Sessions_Export", &Sessions::Export::instance());
  ctx->setContextProperty("Cpp_Sessions_Player", &Sessions::Player::instance());
  ctx->setContextProperty("Cpp_Sessions_Manager", &Sessions::DatabaseManager::instance());
  ctx->setContextProperty("Cpp_ShortcutGenerator", &Misc::ShortcutGenerator::instance());
  ctx->setContextProperty("Cpp_AI_Assistant", &AI::Assistant::instance());
}
#endif

/**
 * @brief Registers app metadata, build info, and screen list QML context properties.
 */
void Misc::ModuleManager::registerAppMetadataProperties(QQmlContext* ctx, bool grpcAvailable)
{
  QVariantList screenList;
  QVariantMap primaryScreen;
  if (!m_headless) {
    for (int i = 0; i < qApp->screens().count(); ++i) {
      QVariantMap map;
      map["name"]     = qApp->screens()[i]->name();
      map["geometry"] = QVariant::fromValue(qApp->screens()[i]->geometry());
      screenList.append(map);
    }

    primaryScreen["name"]     = qApp->primaryScreen()->name();
    primaryScreen["geometry"] = qApp->primaryScreen()->geometry();
  }

#ifdef BUILD_COMMERCIAL
  const bool qtCommercialAvailable = true;
#else
  const bool qtCommercialAvailable = false;
#endif

#ifdef SERIAL_STUDIO_WITH_WEBENGINE
  const bool webEngineAvailable = true;
#else
  const bool webEngineAvailable = false;
#endif

  ctx->setContextProperty("Cpp_AppName", APP_NAME);
  ctx->setContextProperty("Cpp_BuildDate", QStringLiteral(__DATE__));
  ctx->setContextProperty("Cpp_BuildTime", QStringLiteral(__TIME__));
  ctx->setContextProperty("Cpp_ScreenList", screenList);
  ctx->setContextProperty("Cpp_AppVersion", APP_VERSION);
  ctx->setContextProperty("Cpp_PrimaryScreen", primaryScreen);
  ctx->setContextProperty("Cpp_AppUpdaterUrl", APP_UPDATER_URL);
  ctx->setContextProperty("Cpp_AppOrganization", APP_DEVELOPER);
  ctx->setContextProperty("Cpp_UpdaterEnabled", autoUpdaterEnabled());
  ctx->setContextProperty("Cpp_CommercialBuild", qtCommercialAvailable);
  ctx->setContextProperty("Cpp_GrpcAvailable", grpcAvailable);
  ctx->setContextProperty("Cpp_HasWebEngine", webEngineAvailable);
  ctx->setContextProperty("Cpp_AppOrganizationDomain", APP_SUPPORT_URL);

#ifdef ENABLE_GRPC
  ctx->setContextProperty("Cpp_GRPC_Server", &API::GRPC::GRPCServer::instance());
#endif
}

/**
 * @brief Installs QML image providers, loads main.qml, and wires the macOS quit interceptor.
 */
void Misc::ModuleManager::registerImageProvidersAndLoadQml()
{
  m_engine.addImageProvider(QStringLiteral("actionicon"), new Misc::ActionIconProvider());

#ifdef BUILD_COMMERCIAL
  auto* imgProvider = new UI::ImageProvider();
  UI::ImageProvider::setGlobal(imgProvider);
  m_engine.addImageProvider(QStringLiteral("serial-studio-img"), imgProvider);

  auto* imageExport = &Widgets::ImageExport::instance();
  imageExport->setupExternalConnections();
  m_engine.rootContext()->setContextProperty("Cpp_Image_Export", imageExport);

  auto* audioExport = &Widgets::AudioExport::instance();
  audioExport->setupExternalConnections();
  m_engine.rootContext()->setContextProperty("Cpp_Audio_Export", audioExport);
#endif

  m_engine.load(QUrl("qrc:/serial-studio.com/gui/qml/main.qml"));

  m_nativeWindow.installMacOSQuitInterceptor();
  connect(&m_nativeWindow, &NativeWindow::quitRequested, this, [this]() {
    auto roots = m_engine.rootObjects();
    if (!roots.isEmpty())
      QMetaObject::invokeMethod(roots.first(), "quitApplication");
  });
}
