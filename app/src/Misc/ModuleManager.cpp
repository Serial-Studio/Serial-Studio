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

#include "Misc/ModuleManager.h"

#include <QSimpleUpdater.h>

#include <iostream>
#include <QQmlContext>

#include "API/Server.h"
#include "AppInfo.h"
#include "AppState.h"
#include "Console/Export.h"
#include "Console/Handler.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/FrameParser.h"
#include "DataModel/JsCodeEditor.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/OutputCodeEditor.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/FileTransmission.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "Misc/CommonFonts.h"
#include "Misc/Examples.h"
#include "Misc/ExtensionManager.h"
#include "Misc/HelpCenter.h"
#include "Misc/IconEngine.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "SerialStudio.h"
#include "UI/Dashboard.h"
#include "UI/DashboardWidget.h"
#include "UI/Taskbar.h"
#include "UI/TaskbarSettings.h"
#include "UI/Widgets/Accelerometer.h"
#include "UI/Widgets/Bar.h"
#include "UI/Widgets/Compass.h"
#include "UI/Widgets/DataGrid.h"
#include "UI/Widgets/FFTPlot.h"
#include "UI/Widgets/Gauge.h"
#include "UI/Widgets/GPS.h"
#include "UI/Widgets/Gyroscope.h"
#include "UI/Widgets/LEDPanel.h"
#include "UI/Widgets/MultiPlot.h"
#include "UI/Widgets/Plot.h"
#include "UI/Widgets/Terminal.h"
#include "UI/WindowManager.h"

#ifdef BUILD_COMMERCIAL
#  include "DataModel/DBCImporter.h"
#  include "DataModel/ModbusMapImporter.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "Misc/ShortcutGenerator.h"
#  include "MQTT/Client.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#  include "UI/ImageProvider.h"
#  include "UI/Widgets/ImageExport.h"
#  include "UI/Widgets/ImageView.h"
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
 * @brief Custom message handler for Qt debug, warning, critical, and fatal
 *        messages.
 *
 * This static function handles messages of various types (debug, warning,
 * critical, fatal) emitted by the Qt framework.
 *
 * Formats each message twice: once without ANSI codes for stdout (so piped
 * output or terminals that do not support color never see escape sequences),
 * and once with the user's ANSI setting for the in-app console widget.
 *
 * @param type The type of the message.
 * @param context The message log context (file, line, function).
 * @param msg The actual message that was emitted by the Qt system.
 */
static void MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  // Skip empty messages
  (void)context;
  if (msg.isEmpty())
    return;

  // Filter out noisy/unfixable Qt diagnostic messages
  if (type == QtInfoMsg && msg.startsWith("OpenType support missing"))
    return;

  if (type == QtWarningMsg) {
    if (msg.startsWith("Qt was built without Direct3D 12 support"))
      return;

    if (msg.contains("setGeometry"))
      return;

    if (msg.contains("The following paths were searched for Qt WebEngine locales"))
      return;
  }

  // Add function to string
  QString message;
  if (context.function)
    message = QStringLiteral("%1 - %2").arg(context.function, msg);
  else
    message = msg;

  // Get console output
  const bool useAnsiColors = Console::Handler::instance().ansiColorsEnabled();
  const QString output     = Widgets::Terminal::formatDebugMessage(type, message, useAnsiColors);
  if (output.isEmpty())
    return;

  // Print to stdio
  std::cout << Widgets::Terminal::formatDebugMessage(type, message, false).toStdString()
            << std::endl;

  // Print to console
  Console::Handler::instance().displayDebugData(output + "\n");

  // Route Critical unconditionally; Warnings only when user opted in
  const bool isCritical = (type == QtCriticalMsg || type == QtFatalMsg);
  const bool isWarning  = (type == QtWarningMsg);
  if (!isCritical && !isWarning)
    return;

  auto& nc = DataModel::NotificationCenter::instance();
  if (isWarning && !nc.routeWarningsToNotifications())
    return;

  const QString channel = QStringLiteral("System");
  const QString title   = isCritical ? QObject::tr("Critical") : QObject::tr("Warning");

  // Forward on the GUI thread -- post() asserts main-thread affinity
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
 *
 * Configures the application font and configures application signals/slots to
 * destroy singleton classes before the application quits.
 */
Misc::ModuleManager::ModuleManager()
  : m_headless(false), m_automaticUpdates(m_settings.value("App/CheckForUpdates", true).toBool())
{
  // Init translator
  (void)Misc::Translator::instance();

  // Stop modules when application is about to quit
  connect(&m_engine, &QQmlApplicationEngine::quit, this, &Misc::ModuleManager::onQuit);
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
 * @brief Enables headless mode, suppressing QML and GUI loading on init.
 * @param headless Pass @c true to skip the QML engine load.
 */
void Misc::ModuleManager::setHeadless(const bool headless)
{
  m_headless = headless;
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
 *
 * To disable QSimpleUpdater, you need to add DEFINES += DISABLE_QSU in the
 * qmake project file. This option is provided for package managers, users are
 * expected to update the application using the same package manager they used
 * for installing it.
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
  // Restore default handler so late qWarning() during static destruction is safe
  qInstallMessageHandler(nullptr);

  // Stop all plugins and active modules
  Misc::ExtensionManager::instance().stopAllPlugins();
  Misc::TimerEvents::instance().stopTimers();

  // Flush any pending project autosave so a last-second layout drag survives quit
  DataModel::ProjectModel::instance().flushAutoSave();

  CSV::Export::instance().closeFile();
  CSV::Player::instance().closeFile();
  MDF4::Export::instance().closeFile();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().closeFile();
  Sessions::Player::instance().closeFile();
  Sessions::Player::instance().shutdown();
  Sessions::DatabaseManager::instance().closeDatabase(false);
  Sessions::DatabaseManager::instance().shutdown();
#endif
  IO::ConnectionManager::instance().disconnectAllDevices();
  API::Server::instance().removeConnection();

#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer::instance().setEnabled(false);
#endif

  // Terminate the application event loop
  qApp->exit(0);
}

//--------------------------------------------------------------------------------------------------
// Updater configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets default options for QSimpleUpdater (notify on update, no close).
 *
 * Sets the default options for QSimpleUpdater, which are:
 * - Notify user when a new update is found
 * - Do not notify user when we finish checking for updates
 * - Do not close application if update is found
 */
void Misc::ModuleManager::configureUpdater()
{
  // Skip if auto-updater is disabled at build time
  if (!autoUpdaterEnabled())
    return;

  QSimpleUpdater::getInstance()->setNotifyOnUpdate(APP_UPDATER_URL, true);
  QSimpleUpdater::getInstance()->setNotifyOnFinish(APP_UPDATER_URL, false);
  QSimpleUpdater::getInstance()->setMandatoryUpdate(APP_UPDATER_URL, false);
}

//--------------------------------------------------------------------------------------------------
// Module registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers Serial Studio's custom QML types with the engine.
 */
void Misc::ModuleManager::registerQmlTypes()
{
  // Register custom QML widgets & widget models
  qmlRegisterType<Widgets::Bar>("SerialStudio", 1, 0, "BarModel");
  qmlRegisterType<Widgets::GPS>("SerialStudio", 1, 0, "GPSWidget");
  qmlRegisterType<Widgets::Plot>("SerialStudio", 1, 0, "PlotModel");
  qmlRegisterType<Widgets::Gauge>("SerialStudio", 1, 0, "GaugeModel");
  qmlRegisterType<Widgets::Compass>("SerialStudio", 1, 0, "CompassModel");
  qmlRegisterType<Widgets::FFTPlot>("SerialStudio", 1, 0, "FFTPlotModel");
  qmlRegisterType<Widgets::DataGrid>("SerialStudio", 1, 0, "DataGridWidget");
  qmlRegisterType<Widgets::LEDPanel>("SerialStudio", 1, 0, "LEDPanelModel");
  qmlRegisterType<Widgets::Terminal>("SerialStudio", 1, 0, "TerminalWidget");
  qmlRegisterType<Widgets::MultiPlot>("SerialStudio", 1, 0, "MultiPlotModel");
  qmlRegisterType<Widgets::Gyroscope>("SerialStudio", 1, 0, "GyroscopeModel");
  qmlRegisterType<Widgets::Accelerometer>("SerialStudio", 1, 0, "AccelerometerModel");

#ifdef BUILD_COMMERCIAL
  qmlRegisterType<Widgets::Plot3D>("SerialStudio", 1, 0, "Plot3DWidget");
  qmlRegisterType<Widgets::ImageView>("SerialStudio", 1, 0, "ImageViewModel");
  qmlRegisterType<Widgets::Waterfall>("SerialStudio", 1, 0, "WaterfallModel");
#endif

  // Register JSON custom items
  qmlRegisterType<DataModel::JsCodeEditor>("SerialStudio", 1, 0, "JsCodeEditor");
  qmlRegisterType<DataModel::ProjectModel>("SerialStudio", 1, 0, "ProjectModel");
  qmlRegisterType<DataModel::ProjectEditor>("SerialStudio", 1, 0, "ProjectEditor");
  qmlRegisterType<DataModel::OutputCodeEditor>("SerialStudio", 1, 0, "OutputCodeEditor");

  // Register generic dashboard widget
  qmlRegisterType<UI::DashboardWidget>("SerialStudio", 1, 0, "DashboardWidget");

  // Register window manager & taskbar helpers
  qmlRegisterType<UI::Taskbar>("SerialStudio.UI", 1, 0, "TaskBar");
  qmlRegisterType<UI::WindowManager>("SerialStudio.UI", 1, 0, "WindowManager");

  // Regsiter common Serial Studio enums & values
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
 *
 * Initializes all the application modules, registers them with the QML engine
 * and loads the "main.qml" file as the root QML file.
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
  if (!lemonSqueezy.licensingData().isEmpty())
    QMetaObject::invokeMethod(&lemonSqueezy, &Licensing::LemonSqueezy::validate);
#endif
}

/**
 * @brief Wires inter-module signals and runs each module's setupExternalConnections.
 */
void Misc::ModuleManager::setupCrossModuleConnections()
{
  auto* appState             = &AppState::instance();
  auto* ioManager            = &IO::ConnectionManager::instance();
  auto* pluginsBridge        = &API::Server::instance();
  auto* uiDashboard          = &UI::Dashboard::instance();
  auto* notificationCenter   = &DataModel::NotificationCenter::instance();
  auto* miscExtensionManager = &Misc::ExtensionManager::instance();
  auto* miscThemeManager     = &Misc::ThemeManager::instance();

  appState->setupExternalConnections();
  CSV::Export::instance().setupExternalConnections();
  ioManager->setupExternalConnections();
  MDF4::Export::instance().setupExternalConnections();
  DataModel::FrameParser::instance().setupExternalConnections();
  DataModel::ProjectModel::instance().setupExternalConnections();
  DataModel::FrameBuilder::instance().setupExternalConnections();
  Console::Export::instance().setupExternalConnections();
  Console::Handler::instance().setupExternalConnections();
  IO::FileTransmission::instance().setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().setupExternalConnections();
  Sessions::DatabaseManager::instance().setupExternalConnections();
#endif

  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionInstalled,
          miscThemeManager,
          &Misc::ThemeManager::onExtensionInstalled);
  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionUninstalled,
          miscThemeManager,
          &Misc::ThemeManager::onExtensionUninstalled);

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
  ctx->setContextProperty("Cpp_Misc_Utilities", &Misc::Utilities::instance());
  ctx->setContextProperty("Cpp_IO_Bluetooth_LE", ioManager->bluetoothLE());
  ctx->setContextProperty("Cpp_ThemeManager", &Misc::ThemeManager::instance());
  ctx->setContextProperty("Cpp_Console_Handler", &Console::Handler::instance());
  ctx->setContextProperty("Cpp_Misc_Translator", &Misc::Translator::instance());
  ctx->setContextProperty("Cpp_JSON_ProjectModel", &DataModel::ProjectModel::instance());
  ctx->setContextProperty("Cpp_JSON_ProjectEditor", &DataModel::ProjectEditor::instance());
  ctx->setContextProperty("Cpp_JSON_FrameBuilder", &DataModel::FrameBuilder::instance());
  ctx->setContextProperty("Cpp_Notifications", &DataModel::NotificationCenter::instance());
  ctx->setContextProperty("Cpp_Misc_TimerEvents", &Misc::TimerEvents::instance());
  ctx->setContextProperty("Cpp_Misc_CommonFonts", &Misc::CommonFonts::instance());
  ctx->setContextProperty("Cpp_IO_FileTransmission", &IO::FileTransmission::instance());
  ctx->setContextProperty("Cpp_Misc_WorkspaceManager", &Misc::WorkspaceManager::instance());
  ctx->setContextProperty("Cpp_Examples", &Misc::Examples::instance());
  ctx->setContextProperty("Cpp_HelpCenter", &Misc::HelpCenter::instance());
  ctx->setContextProperty("Cpp_ExtensionManager", &Misc::ExtensionManager::instance());
  ctx->setContextProperty("Cpp_Misc_IconEngine", &Misc::IconEngine::instance());
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
  ctx->setContextProperty("Cpp_JSON_DBCImporter", &DataModel::DBCImporter::instance());
  ctx->setContextProperty("Cpp_JSON_ModbusMapImporter", &DataModel::ModbusMapImporter::instance());
  ctx->setContextProperty("Cpp_Licensing_Trial", &Licensing::Trial::instance());
  ctx->setContextProperty("Cpp_MQTT_Client", &MQTT::Client::instance());
  ctx->setContextProperty("Cpp_Licensing_LemonSqueezy", &Licensing::LemonSqueezy::instance());
  ctx->setContextProperty("Cpp_Sessions_Export", &Sessions::Export::instance());
  ctx->setContextProperty("Cpp_Sessions_Player", &Sessions::Player::instance());
  ctx->setContextProperty("Cpp_Sessions_Manager", &Sessions::DatabaseManager::instance());
  ctx->setContextProperty("Cpp_ShortcutGenerator", &Misc::ShortcutGenerator::instance());
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
#endif

  m_engine.load(QUrl("qrc:/serial-studio.com/gui/qml/main.qml"));

  m_nativeWindow.installMacOSQuitInterceptor();
  connect(&m_nativeWindow, &NativeWindow::quitRequested, this, [this]() {
    auto roots = m_engine.rootObjects();
    if (!roots.isEmpty())
      QMetaObject::invokeMethod(roots.first(), "quitApplication");
  });
}
