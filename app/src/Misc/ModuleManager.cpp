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
#include <vector>
// code-verify on

#include <QColor>
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>
#include <QStringList>
#include <QSysInfo>

#if defined(Q_OS_WIN)
// clang-format off
#  include <windows.h>
#  include <appmodel.h>
#  include <QVariant>
// clang-format on
#endif

#include "API/CommandHandler.h"
#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/Mirror/MirrorPublisher.h"
#include "API/Mirror/MirrorSession.h"
#include "API/ProcessLauncher.h"
#include "API/Server.h"
#include "API/TerminalBridge.h"
#include "ApiHandlers/UiHandlers.h"
#include "AppState.h"
#include "Benchmark/BenchmarkRunner.h"
#include "Console/Export.h"
#include "Console/Handler.h"
#include "Core/AppInfo.h"
#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/Crypto/MachineKey.h"
#include "Core/DataModel/IBlockSink.h"
#include "Core/DataModel/PropertyValidators.h"
#include "Core/IconRegistry.h"
#include "Core/IO/IRawByteTap.h"
#include "Core/License.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/Runtime.h"
#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "Core/TimerEvents.h"
#include "Core/Translator.h"
#include "Core/WorkspaceManager.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/DataTable.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Importers/ProtoImporter.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/RowFilterProxy.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/MacroRunner.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/ConnectionManager.h"
#include "IO/FileTransmission.h"
#include "IO/PipelineHost.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "Misc/BackupManager.h"
#include "Misc/CommonFonts.h"
#include "Misc/ConnectionDiagnostics.h"
#include "Misc/ContextRegistry.h"
#include "Misc/CrashTracker.h"
#include "Misc/Examples.h"
#include "Misc/ExtensionManager.h"
#include "Misc/GraphicsBackend.h"
#include "Misc/HelpCenter.h"
#include "Misc/HighDpiScaling.h"
#include "Misc/IconEngine.h"
#include "Misc/ProblemCenter.h"
#include "Misc/SimdSettings.h"
#include "Misc/ThemeManager.h"
#include "Misc/Utilities.h"
#include "Platform/AppPlatform.h"
#include "ProjectEditor/Editors/ControlScriptEditor.h"
#include "ProjectEditor/Editors/FrameParserModel.h"
#include "ProjectEditor/Editors/JsCodeEditor.h"
#include "ProjectEditor/Editors/MacroEditor.h"
#include "ProjectEditor/Editors/OutputCodeEditor.h"
#include "ProjectEditor/ProjectEditor.h"
#include "SessionContext.h"
#include "UI/AlarmMonitor.h"
#include "UI/CommandRegistry.h"
#include "UI/Dashboard.h"
#include "UI/DashboardWidget.h"
#include "UI/SerialStudioHelpers.h"
#include "UI/Taskbar.h"
#include "UI/TaskbarSettings.h"
#include "UI/WidgetExtensions.h"
#include "UI/Widgets/Accelerometer.h"
#include "UI/Widgets/Bar.h"
#include "UI/Widgets/BarPanel.h"
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
#  include "API/Handlers/LicensingHandler.h"
#  include "Core/Licensing/CommercialToken.h"
#  include "DataModel/Importers/DBCImporter.h"
#  include "DataModel/Importers/ModbusMapImporter.h"
#  include "InfluxDB/Export.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/MachineID.h"
#  include "Licensing/OfflineLicense.h"
#  include "Licensing/Trial.h"
#  include "Misc/ShortcutGenerator.h"
#  include "MQTT/Publisher.h"
#  include "ProjectEditor/Editors/PainterCodeEditor.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#  include "Sessions/ReportOptionsModel.h"
#  include "UI/ImageProvider.h"
#  include "UI/Widgets/AudioExport.h"
#  include "UI/Widgets/ImageExport.h"
#  include "UI/Widgets/ImageView.h"
#  include "UI/Widgets/Output/Preview.h"
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
  m_simdSettings = std::make_unique<Misc::SimdSettings>();

  connect(&m_engine, &QQmlApplicationEngine::quit, this, &Misc::ModuleManager::onQuit);

  connect(qApp, &QCoreApplication::aboutToQuit, this, &Misc::ModuleManager::onQuit);
}

/**
 * @brief Out of line so the forward-declared SimdSettings behind the unique_ptr is complete where
 *        it is deleted, instead of relying on whichever TU happens to destroy the manager.
 */
Misc::ModuleManager::~ModuleManager() = default;

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
 * @brief Stops every active module and exits the application event loop. Latching teardown is
 *        the first statement: past this point the GUI thread stops pumping events, so every
 *        cross-thread marshal must degrade to a no-op instead of blocking on it.
 */
void Misc::ModuleManager::onQuit()
{
  if (m_quitHandled)
    return;

  m_quitHandled = true;

  IO::PipelineHost::beginTeardown();

  qInstallMessageHandler(nullptr);

  DataModel::ControlScript::instance().shutdown();

  Misc::ExtensionManager::instance().stopAllPlugins();
  Misc::TimerEvents::instance().stopTimers();

  DataModel::ProjectModel::instance().flushAutoSave();

  Console::Export::instance().closeFile();
  CSV::Export::instance().closeFile();
  CSV::Player::instance().closeFile();
  MDF4::Export::instance().closeFile();
  teardownHeadlessSessionModules();
#ifdef BUILD_COMMERCIAL
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
  IO::ConnectionManager::instance().stopStreamWorkers();
  IO::PipelineHost::instance().shutdown();
  CSV::Export::instance().stopWorker();
#ifdef BUILD_COMMERCIAL
  Console::Export::instance().stopWorker();
  MDF4::Export::instance().stopWorker();
#endif
  API::Server::instance().stopWorker();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().stopWorker();
  MQTT::Publisher::instance().stopWorker();
  InfluxDB::Export::instance().stopWorker();
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
  qmlRegisterType<API::TerminalBridge>("SerialStudio", 1, 0, "ApiTerminalBridge");
  qmlRegisterType<DataModel::MacroEditor>("SerialStudio", 1, 0, "MacroEditor");
  qmlRegisterType<DataModel::MacroRunner>("SerialStudio", 1, 0, "MacroRunner");
  qmlRegisterType<DataModel::RowFilterProxy>("SerialStudio", 1, 0, "RowFilterProxy");
  qmlRegisterType<Widgets::Bar>("SerialStudio", 1, 0, "BarModel");
  qmlRegisterType<Widgets::BarPanel>("SerialStudio", 1, 0, "BarPanelModel");
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
  qmlRegisterType<Widgets::Output::Preview>("SerialStudio", 1, 0, "OutputWidgetPreview");
  qmlRegisterType<DataModel::PainterCodeEditor>("SerialStudio", 1, 0, "PainterCodeEditor");
  qmlRegisterType<Sessions::ReportOptionsModel>("SerialStudio", 1, 0, "ReportOptionsModel");
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

  qmlRegisterUncreatableMetaObject(SerialStudio::staticMetaObject,
                                   "SerialStudio",
                                   1,
                                   0,
                                   "SerialStudio",
                                   QStringLiteral("SerialStudio is a namespace"));
  qmlRegisterSingletonType<UI::SerialStudioHelpers>(
    "SerialStudio", 1, 0, "SerialStudioHelpers", [](QQmlEngine*, QJSEngine*) -> QObject* {
      return new UI::SerialStudioHelpers();
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

  auto& translator = Misc::Translator::instance();
  connect(&translator,
          &Misc::Translator::languageChanged,
          &m_engine,
          &QQmlApplicationEngine::retranslate);

  const auto applyLayoutDirection = [&translator] {
    QGuiApplication::setLayoutDirection(translator.rtl() ? Qt::RightToLeft : Qt::LeftToRight);
  };
  connect(&translator, &Misc::Translator::languageChanged, &translator, applyLayoutDirection);
  applyLayoutDirection();

  setupCrossModuleConnections();

  qInstallMessageHandler(MessageHandler);
  qAddPostRoutine([]() { qInstallMessageHandler(nullptr); });

  Platform::AppPlatform::registerRenderThreadWithMmcss();
  IO::PipelineHost::instance().registerIngestThread();

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

#ifdef BUILD_COMMERCIAL
/**
 * @brief Publishes the licensing facts into Core::License, the flag every layer reads instead of
 *        the token (spec 0077): once after the licensing block constructs, then on every real
 *        activation transition, so late or offline activation re-derives like startup does.
 */
static void publishLicenseState(Core::Bus::MessageBus& bus, const Licensing::Trial& trial)
{
  const auto& token  = Licensing::CommercialToken::current();
  const auto tier    = static_cast<quint8>(token.featureTier());
  const bool expired = trial.trialExpired();
  Core::License::set(token.isValid() && SS_LICENSE_GUARD(), tier, expired);
  bus.publishState<Core::Bus::LicenseStateChanged>(
    Core::License::activated(), static_cast<int>(tier), expired);
}
#endif

/**
 * @brief Adopts the bus (slot 0) and binds the Core service set (spec 0077 T71): the four Core
 *        singletons construct here, each reaching only Qt. Runs right after QApplication, before
 *        the CLI and the theme override construct Ui singletons that read the set; the pinned
 *        order calls it again and finds the bus already adopted.
 */
void Misc::ModuleManager::bootstrapCoreServices()
{
  auto& ctx = SessionContext::current();
  if (ctx.hasBus())
    return;

  Core::Runtime::setSessionId(ctx.sessionId());
  ctx.adoptBus(std::make_unique<Core::Bus::MessageBus>());
  static Core::Services services{ctx.bus(),
                                 Misc::Translator::instance(),
                                 Misc::TimerEvents::instance(),
                                 Misc::WorkspaceManager::instance(),
                                 Misc::IconRegistry::instance()};
  Core::bindServices(&services);
}

/**
 * @brief Binds the Pipeline module set once the seven modules are adopted; every Storage, Api and
 *        Ui module is constructed after this point, so its constructor may read the set.
 */
static void bindPipelineModuleSet(SessionContext& ctx)
{
  static DataModel::PipelineModules modules{ctx.appState(),
                                            ctx.projectModel(),
                                            ctx.frameBuilder(),
                                            ctx.frameParser(),
                                            DataModel::ControlScript::instance(),
                                            ctx.pipelineHost(),
                                            ctx.notifications()};
  DataModel::bindPipelineModules(&modules);
}

/**
 * @brief Binds the handler context once the dashboard (its frame surface) is adopted: the Devices,
 *        Storage and Api modules the API handlers and the Ui read.
 */
static void bindHandlerContextSet(SessionContext& ctx)
{
#ifdef BUILD_COMMERCIAL
  static API::HandlerContext context{ctx.connectionManager(),
                                     ctx.dashboard(),
                                     API::CommandRegistry::instance(),
                                     API::Server::instance(),
                                     CSV::Player::instance(),
                                     CSV::Export::instance(),
                                     MDF4::Player::instance(),
                                     MDF4::Export::instance(),
                                     Sessions::Player::instance(),
                                     Sessions::Export::instance(),
                                     Sessions::DatabaseManager::instance(),
                                     MQTT::Publisher::instance(),
                                     InfluxDB::Export::instance()};
#else
  static API::HandlerContext context{ctx.connectionManager(),
                                     ctx.dashboard(),
                                     API::CommandRegistry::instance(),
                                     API::Server::instance(),
                                     CSV::Player::instance(),
                                     CSV::Export::instance(),
                                     MDF4::Player::instance(),
                                     MDF4::Export::instance()};
#endif
  API::bindHandlerContext(&context);
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
  bootstrapCoreServices();
  auto& messageBus = ctx.bus();

  Core::Prompt::setPrompter(&Misc::Utilities::instance());
  DataModel::PropertyHooks::setColorValidator(
    [](const QString& color) { return QColor::fromString(color).isValid(); });
  DataModel::PropertyHooks::setActionIconResolver(&Misc::IconEngine::resolveActionIconSource);
  Misc::Translator::instance().attachMessageBus(messageBus);
#ifdef BUILD_COMMERCIAL
  Core::Crypto::setMachineKey(Licensing::MachineID::instance().machineSpecificKey());
  auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  (void)Licensing::OfflineLicense::instance();
  auto& trial = Licensing::Trial::instance();
  publishLicenseState(messageBus, trial);
  QObject::connect(&lemonSqueezy,
                   &Licensing::LemonSqueezy::activatedChanged,
                   &lemonSqueezy,
                   [&messageBus, &trial] { publishLicenseState(messageBus, trial); });
  QObject::connect(&trial, &Licensing::Trial::enabledChanged, &trial, [&messageBus, &trial] {
    publishLicenseState(messageBus, trial);
  });
#endif
  (void)Misc::CommonFonts::instance();
  ctx.adoptNotifications(SessionContext::create<DataModel::NotificationCenter>(messageBus));
  Misc::ProblemCenter::instance().attachMessageBus(messageBus);
  Misc::ConnectionDiagnostics::instance().attachMessageBus(messageBus);
  (void)Misc::ThemeManager::instance();
  (void)Misc::ExtensionManager::instance();
  (void)DataModel::ControlScript::instance();
  ctx.adoptProjectModel(SessionContext::create<DataModel::ProjectModel>(messageBus));
  ctx.adoptAppState(SessionContext::create<AppState>(messageBus));
  ctx.adoptFrameBuilder(SessionContext::create<DataModel::FrameBuilder>(messageBus));
  ctx.adoptPipelineHost(SessionContext::create<IO::PipelineHost>(messageBus));
  ctx.adoptFrameParser(SessionContext::create<DataModel::FrameParser>(messageBus));
  bindPipelineModuleSet(ctx);
  ctx.adoptConnectionManager(
    SessionContext::create<IO::ConnectionManager>(messageBus, ctx.pipelineHost()));
  ctx.adoptConsole(SessionContext::create<Console::Handler>(messageBus, ctx.connectionManager()));
  (void)API::Server::instance();
  CSV::Player::instance().attachMessageBus(messageBus);
  MDF4::Player::instance().attachMessageBus(messageBus);
#ifdef BUILD_COMMERCIAL
  Sessions::Player::instance().attachMessageBus(messageBus);
  Sessions::Export::instance().attachMessageBus(messageBus);
  Sessions::DatabaseManager::instance().attachMessageBus(messageBus);
  MQTT::Publisher::instance().attachMessageBus(messageBus);
#endif
  CSV::Export::instance().attachMessageBus(messageBus);
  MDF4::Export::instance().attachMessageBus(messageBus);
  Console::Export::instance().attachMessageBus(messageBus);
  UI::WidgetExtensions::instance().attachMessageBus(messageBus);
  ctx.adoptDashboard(SessionContext::create<UI::Dashboard>(messageBus, ctx.connectionManager()));
  bindHandlerContextSet(ctx);
}

/**
 * @brief Binds the seams every root needs before any wiring or the first device open (spec 0077):
 *        block sinks (two read-only observers named by slot), raw-byte taps and the per-frame tap.
 *        ONE list for the GUI, headless and benchmark roots: a sink missing here never reaches
 *        the any-async-sink flag and records an empty file; the benchmark tiers need them all.
 */
void Misc::ModuleManager::bindInterfaces()
{
  auto& frameBuilder = DataModel::FrameBuilder::instance();
  auto& pipelineHost = IO::PipelineHost::instance();
  auto& ioManager    = IO::ConnectionManager::instance();
  auto& server       = API::Server::instance();
  auto& console      = Console::Handler::instance();

  std::vector<DataModel::IBlockSink*> sinks{
    &CSV::Export::instance(), &MDF4::Export::instance(), &server};
  std::vector<IO::IRawByteTap*> taps{&server, &console};
  DataModel::IBlockSink* grpc = nullptr;
#ifdef BUILD_COMMERCIAL
  auto& sessions = Sessions::Export::instance();
  auto& mqtt     = MQTT::Publisher::instance();
  sinks.push_back(&sessions);
  sinks.push_back(&mqtt);
  sinks.push_back(&Widgets::AudioExport::instance());
  sinks.push_back(&InfluxDB::Export::instance());
  taps.push_back(&sessions);
  taps.push_back(&mqtt);
  pipelineHost.setRawFrameTap(&mqtt);
  DataModel::setMqttPublisher(&mqtt);
#endif
#ifdef ENABLE_GRPC
  auto& grpcServer = API::GRPC::GRPCServer::instance();
  sinks.push_back(&grpcServer);
  taps.push_back(&grpcServer);
  grpc = &grpcServer;
#endif

  pipelineHost.bindFrameBuilder(frameBuilder);
  frameBuilder.bindBlockSinks(sinks, &server, grpc);
  ioManager.bindRawTaps(taps);

  DataModel::setCommandExecutor(&API::CommandHandler::instance());
  API::CommandRegistry::instance().bindCheckpointStore(Misc::BackupManager::instance());

  auto& dashboard = UI::Dashboard::instance();
  DataModel::setDashboardControl(&dashboard);
  DataModel::setDeviceWriter(&ioManager);
  CSV::Player::instance().setPlotSink(&dashboard);
  CSV::Player::instance().setPayloadInjector(&ioManager);
  MDF4::Player::instance().setPlotSink(&dashboard);
  MDF4::Player::instance().setPayloadInjector(&ioManager);
#ifdef BUILD_COMMERCIAL
  Sessions::Player::instance().setPlotSink(&dashboard);
  Sessions::Player::instance().setPayloadInjector(&ioManager);
#endif
}

/**
 * @brief Registers every API command set in one place (spec 0077 T60/T62): the core set the API
 *        library owns, the user-interface set from the Ui library and the licensing set from the
 *        executable. Idempotent, so the schema dump and the GUI root may both call it.
 */
void Misc::ModuleManager::registerApiHandlers()
{
  static bool registered = false;
  if (registered)
    return;

  registered = true;
  API::CommandHandler::instance().registerCoreHandlers();
  UI::ApiHandlers::registerAll();
#ifdef BUILD_COMMERCIAL
  API::Handlers::LicensingHandler::registerCommands();
#endif
}

/**
 * @brief Wires the modules a headless session re-record needs (spec 0044). Such a run builds
 *        the pinned order without the QML interface, so setupCrossModuleConnections() never
 *        runs and neither the frame builder nor the session exporter would see a frame.
 */
void Misc::ModuleManager::setupHeadlessSessionConnections()
{
  bindInterfaces();
  registerApiHandlers();
  DataModel::NotificationCenter::instance().setupExternalConnections();
  DataModel::FrameBuilder::instance().setupExternalConnections();
  IO::PipelineHost::instance().setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().setupExternalConnections();
#endif
}

/**
 * @brief Tears down the Sessions modules while qApp is still alive, in the pinned order
 *        (exporter file, player file, player worker, database), so the static dtors never
 *        meet a running worker thread. Shared by onQuit() and the headless CLI runs.
 */
void Misc::ModuleManager::teardownHeadlessSessionModules()
{
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().closeFile();
  Sessions::Player::instance().closeFile();
  Sessions::Player::instance().shutdown();
  Sessions::DatabaseManager::instance().closeDatabase(false);
  Sessions::DatabaseManager::instance().shutdown();
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

#ifdef BUILD_COMMERCIAL
/**
 * @brief Runs setupExternalConnections on the Pro modules that own their own singleton, so the
 *        composition root's wiring pass stays one screen long as the Pro set grows.
 */
static void setupCommercialModuleConnections()
{
  Sessions::Export::instance().setupExternalConnections();
  Sessions::DatabaseManager::instance().setupExternalConnections();
  MQTT::Publisher::instance().setupExternalConnections();
  auto& influx = InfluxDB::Export::instance();
  influx.attachMessageBus(SessionContext::current().bus());
  influx.setupExternalConnections();
}
#endif

/**
 * @brief Wires inter-module signals and runs each module's setupExternalConnections. The seams
 *        are bound right after the pinned order and before any wiring, so an injected class is
 *        never constructed before its context exists (spec 0039) and no sink is missing when the
 *        first block flows (spec 0077); the pipeline-thread affinity move runs last (spec 0051).
 */
void Misc::ModuleManager::setupCrossModuleConnections()
{
  instantiateCoreModules();
  bindInterfaces();
  registerApiHandlers();

  auto* appState             = &AppState::instance();
  auto* ioManager            = &IO::ConnectionManager::instance();
  auto* pluginsBridge        = &API::Server::instance();
  auto* uiDashboard          = &UI::Dashboard::instance();
  auto* notificationCenter   = &DataModel::NotificationCenter::instance();
  auto* miscExtensionManager = &Misc::ExtensionManager::instance();
  auto* miscThemeManager     = &Misc::ThemeManager::instance();

  appState->setEphemeralSession(m_ephemeralSession);
  notificationCenter->setupExternalConnections();
  appState->setupExternalConnections();
  CSV::Export::instance().setupExternalConnections();
  API::Server::instance().setupExternalConnections();
  ioManager->setupExternalConnections();
  IO::PipelineHost::instance().setupExternalConnections();
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
  setupCommercialModuleConnections();
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

  miscExtensionManager->checkForUpdatesOnStartup(m_automaticUpdates);
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

  IO::PipelineHost::instance().relocateProcessingObjects();
}

/**
 * @brief Registers every always-available C++ singleton as a QML context property.
 */
void Misc::ModuleManager::registerCoreContextProperties(QQmlContext* ctx)
{
  auto* ioManager = &IO::ConnectionManager::instance();

  Misc::ContextRegistry registry;
  registry.add("Cpp_AppState", &AppState::instance());
  registry.add("Cpp_Updater", m_headless ? nullptr : QSimpleUpdater::getInstance());
  registry.add("Cpp_IO_Serial", ioManager->uart());
  registry.add("Cpp_CSV_Export", &CSV::Export::instance());
  registry.add("Cpp_CSV_Player", &CSV::Player::instance());
  registry.add("Cpp_IO_Manager", ioManager);
  registry.add("Cpp_IO_Network", ioManager->network());
  registry.add("Cpp_MDF4_Export", &MDF4::Export::instance());
  registry.add("Cpp_MDF4_Player", &MDF4::Player::instance());
  registry.add("Cpp_Misc_ModuleManager", this);
  registry.add("Cpp_UI_Dashboard", &UI::Dashboard::instance());
  registry.add("Cpp_UI_TaskbarSettings", &UI::TaskbarSettings::instance());
  registry.add("Cpp_Console_Export", &Console::Export::instance());
  registry.add("Cpp_NativeWindow", &m_nativeWindow);
  registry.add("Cpp_API_Server", &API::Server::instance());
  registry.add("Cpp_API_Mirror", &API::MirrorSession::instance());
  registry.add("Cpp_Misc_Utilities", &Misc::Utilities::instance());
  registry.add("Cpp_IO_Bluetooth_LE", ioManager->bluetoothLE());
  registry.add("Cpp_ThemeManager", &Misc::ThemeManager::instance());
  registry.add("Cpp_Console_Handler", &Console::Handler::instance());
  registry.add("Cpp_Misc_Translator", &Misc::Translator::instance());
  registry.add("Cpp_JSON_ProjectModel", &DataModel::ProjectModel::instance());
  registry.add("Cpp_JSON_ProjectEditor", &DataModel::ProjectEditor::instance());
  registry.add("Cpp_ControlScript", &DataModel::ControlScript::instance());
  registry.add("Cpp_JSON_ProtoImporter", &DataModel::ProtoImporter::instance());
  registry.add("Cpp_JSON_FrameBuilder", &DataModel::FrameBuilder::instance());
  registry.add("Cpp_Notifications", &DataModel::NotificationCenter::instance());
  registry.add("Cpp_Misc_TimerEvents", &Misc::TimerEvents::instance());
  registry.add("Cpp_Misc_CommonFonts", &Misc::CommonFonts::instance());
  registry.add("Cpp_IO_FileTransmission", &IO::FileTransmission::instance());
  registry.add("Cpp_Misc_WorkspaceManager", &Misc::WorkspaceManager::instance());
  registry.add("Cpp_Examples", &Misc::Examples::instance());
  registry.add("Cpp_HelpCenter", &Misc::HelpCenter::instance());
  registry.add("Cpp_ExtensionManager", &Misc::ExtensionManager::instance());
  registry.add("Cpp_Misc_IconEngine", &Misc::IconEngine::instance());
  registry.add("Cpp_Misc_IconRegistry", &Misc::IconRegistry::instance());
  registry.add("Cpp_Misc_ProblemCenter", &Misc::ProblemCenter::instance());
  registry.add("Cpp_Misc_ConnectionDiagnostics", &Misc::ConnectionDiagnostics::instance());
  registry.add("Cpp_UI_CommandRegistry", &UI::CommandRegistry::instance());
  registry.add("Cpp_UI_WidgetExtensions", &UI::WidgetExtensions::instance());
  registry.add("Cpp_Misc_GraphicsBackend", &Misc::GraphicsBackend::instance());
  registry.add("Cpp_Misc_HighDpiScaling", &Misc::HighDpiScaling::instance());
  registry.add("Cpp_Misc_SimdSettings", m_simdSettings.get());
  registry.add("Cpp_Misc_CrashTracker", &Misc::CrashTracker::instance());
  registry.add("Cpp_Misc_BackupManager", &Misc::BackupManager::instance());
  registry.add("Cpp_Benchmark_Runner", &Benchmark::BenchmarkRunner::instance());

  registry.apply(ctx);
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Registers Pro-only C++ singletons as QML context properties.
 */
void Misc::ModuleManager::registerCommercialContextProperties(QQmlContext* ctx)
{
  auto* ioManager = &IO::ConnectionManager::instance();

  Misc::ContextRegistry registry;
  registry.add("Cpp_IO_Audio", ioManager->audio());
  registry.add("Cpp_IO_CANBus", ioManager->canBus());
  registry.add("Cpp_IO_Modbus", ioManager->modbus());
  registry.add("Cpp_IO_OpcUa", ioManager->opcUa());
  registry.add("Cpp_IO_S7", ioManager->s7());
  registry.add("Cpp_IO_Eip", ioManager->ethernetIp());
  registry.add("Cpp_IO_Iec104", ioManager->iec104());
  registry.add("Cpp_IO_USB", ioManager->usb());
  registry.add("Cpp_IO_HID", ioManager->hid());
  registry.add("Cpp_IO_Process", ioManager->process());
  registry.add("Cpp_IO_Mqtt", ioManager->mqtt());
  registry.add("Cpp_MQTT_Publisher", &MQTT::Publisher::instance());
  registry.add("Cpp_JSON_DBCImporter", &DataModel::DBCImporter::instance());
  registry.add("Cpp_JSON_ModbusMapImporter", &DataModel::ModbusMapImporter::instance());
  registry.add("Cpp_Licensing_Trial", &Licensing::Trial::instance());
  registry.add("Cpp_Licensing_LemonSqueezy", &Licensing::LemonSqueezy::instance());
  registry.add("Cpp_Licensing_OfflineLicense", &Licensing::OfflineLicense::instance());
  registry.add("Cpp_Sessions_Export", &Sessions::Export::instance());
  registry.add("Cpp_InfluxDB_Export", &InfluxDB::Export::instance());
  registry.add("Cpp_Sessions_Player", &Sessions::Player::instance());
  registry.add("Cpp_Sessions_Manager", &Sessions::DatabaseManager::instance());
  registry.add("Cpp_ShortcutGenerator", &Misc::ShortcutGenerator::instance());
  registry.add("Cpp_AI_Assistant", &AI::Assistant::instance());

  registry.apply(ctx);
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

  Misc::ContextRegistry registry;
  registry.add("Cpp_AppName", QVariant(QStringLiteral(APP_NAME)));
  registry.add("Cpp_ScreenList", QVariant(screenList));
  registry.add("Cpp_AppVersion", QVariant(QStringLiteral(APP_VERSION)));
  registry.add("Cpp_AppCommit", QVariant(QStringLiteral(APP_COMMIT)));
  registry.add("Cpp_PrimaryScreen", QVariant(primaryScreen));
  registry.add("Cpp_AppUpdaterUrl", QVariant(QStringLiteral(APP_UPDATER_URL)));
  registry.add("Cpp_AppOrganization", QVariant(QStringLiteral(APP_DEVELOPER)));
  registry.add("Cpp_UpdaterEnabled", QVariant(autoUpdaterEnabled()));
  registry.add("Cpp_CommercialBuild", QVariant(qtCommercialAvailable));
  registry.add("Cpp_GrpcAvailable", QVariant(grpcAvailable));
  registry.add("Cpp_HasWebEngine", QVariant(webEngineAvailable));

#ifdef ENABLE_GRPC
  registry.add("Cpp_GRPC_Server", &API::GRPC::GRPCServer::instance());
#endif

  registry.apply(ctx);
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

  Misc::ContextRegistry registry;
  registry.add("Cpp_Image_Export", imageExport);
  registry.apply(m_engine.rootContext());

  Widgets::AudioExport::instance().setupExternalConnections();
#endif

  m_engine.load(QUrl("qrc:/serial-studio.com/gui/qml/main.qml"));

  m_nativeWindow.installMacOSQuitInterceptor();
  connect(&m_nativeWindow, &NativeWindow::quitRequested, this, [this]() {
    auto roots = m_engine.rootObjects();
    if (!roots.isEmpty())
      QMetaObject::invokeMethod(roots.first(), "quitApplication");
  });
}
