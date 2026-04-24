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
#  include "MQTT/Client.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#  include "UI/ImageProvider.h"
#  include "UI/Widgets/ImageExport.h"
#  include "UI/Widgets/ImageView.h"
#  include "UI/Widgets/Plot3D.h"
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

  // Filter out noisy/unfixable Qt info messages
  if (type == QtInfoMsg) {
    if (msg.startsWith("OpenType support missing"))
      return;
  }

  // Filter out noisy/unfixable Qt warning messages
  else if (type == QtWarningMsg) {
    if (msg.startsWith("Qt was built without Direct3D 12 support"))
      return;
    else if (msg.contains("setGeometry"))
      return;
    else if (msg.contains("The following paths were searched for Qt WebEngine locales"))
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

  // Forward on the GUI thread — post() asserts main-thread affinity
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
 * Configures the application font and configures application signals/slots to
 * destroy singleton classes before the application quits.
 */
Misc::ModuleManager::ModuleManager()
  : m_headless(false), m_automaticUpdates(m_settings.value("App/AutomaticUpdates", true).toBool())
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
    m_settings.setValue("App/AutomaticUpdates", m_automaticUpdates);
    Q_EMIT automaticUpdatesChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Engine access
//--------------------------------------------------------------------------------------------------

/**
 * Returns a pointer to the QML application engine
 */
const QQmlApplicationEngine& Misc::ModuleManager::engine() const noexcept
{
  return m_engine;
}

/**
 * Enables or disables the auto-updater system (QSimpleUpdater).
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
 * Calls the functions needed to safely quit the application
 */
void Misc::ModuleManager::onQuit()
{
  // Stop all plugins and active modules
  Misc::ExtensionManager::instance().stopAllPlugins();
  Misc::TimerEvents::instance().stopTimers();

  CSV::Export::instance().closeFile();
  CSV::Player::instance().closeFile();
  MDF4::Export::instance().closeFile();
#ifdef BUILD_COMMERCIAL
  Sessions::Export::instance().closeFile();
  Sessions::Player::instance().closeFile();
  Sessions::DatabaseManager::instance().closeDatabase(false);
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
 * Register custom QML types, for the moment, we have:
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
 * Initializes all the application modules, registers them with the QML engine
 * and loads the "main.qml" file as the root QML file.
 */
void Misc::ModuleManager::initializeQmlInterface()
{
  // Initialize licensing module first
#ifdef BUILD_COMMERCIAL
  auto lemonSqueezy = &Licensing::LemonSqueezy::instance();
  auto trial        = &Licensing::Trial::instance();
#endif

  // Initialize heavily used modules first
  auto miscTranslator   = &Misc::Translator::instance();
  auto miscThemeManager = &Misc::ThemeManager::instance();

  // Initialize modules
  auto appState             = &AppState::instance();
  auto ioManager            = &IO::ConnectionManager::instance();
  auto csvExport            = &CSV::Export::instance();
  auto csvPlayer            = &CSV::Player::instance();
  auto mdf4Export           = &MDF4::Export::instance();
  auto mdf4Player           = &MDF4::Player::instance();
  auto uiDashboard          = &UI::Dashboard::instance();
  auto ioSerial             = ioManager->uart();
  auto pluginsBridge        = &API::Server::instance();
  auto miscUtilities        = &Misc::Utilities::instance();
  auto consoleExport        = &Console::Export::instance();
  auto ioNetwork            = ioManager->network();
  auto frameBuilder         = &DataModel::FrameBuilder::instance();
  auto notificationCenter   = &DataModel::NotificationCenter::instance();
  auto projectModel         = &DataModel::ProjectModel::instance();
  auto projectEditor        = &DataModel::ProjectEditor::instance();
  auto consoleHandler       = &Console::Handler::instance();
  auto miscTimerEvents      = &Misc::TimerEvents::instance();
  auto miscCommonFonts      = &Misc::CommonFonts::instance();
  auto ioBluetoothLE        = ioManager->bluetoothLE();
  auto ioFileTransmission   = &IO::FileTransmission::instance();
  auto miscWorkspaceManager = &Misc::WorkspaceManager::instance();
  auto miscExamples         = &Misc::Examples::instance();
  auto miscHelpCenter       = &Misc::HelpCenter::instance();
  auto miscExtensionManager = &Misc::ExtensionManager::instance();
  auto miscIconEngine       = &Misc::IconEngine::instance();
  auto frameParser          = &DataModel::FrameParser::instance();

  // Initialize commercial modules
#ifdef BUILD_COMMERCIAL
  const bool qtCommercialAvailable = true;
  auto mqttClient                  = &MQTT::Client::instance();
  auto sqliteExport                = &Sessions::Export::instance();
  auto sqlitePlayer                = &Sessions::Player::instance();
  auto sqliteDbManager             = &Sessions::DatabaseManager::instance();
  auto dbcImporter                 = &DataModel::DBCImporter::instance();
  auto modbusMapImporter           = &DataModel::ModbusMapImporter::instance();
  auto audioDriver                 = ioManager->audio();
  auto canBusDriver                = ioManager->canBus();
  auto modbusDriver                = ioManager->modbus();
  auto usbDriver                   = ioManager->usb();
  auto hidDriver                   = ioManager->hid();
  auto processDriver               = ioManager->process();
#else
  const bool qtCommercialAvailable = false;
#endif

  // Initialize gRPC server (optional)
#ifdef ENABLE_GRPC
  auto grpcServer          = &API::GRPC::GRPCServer::instance();
  const bool grpcAvailable = true;
#else
  const bool grpcAvailable = false;
#endif

  // Initialize third-party modules (not needed in headless mode)
  QSimpleUpdater* updater = m_headless ? nullptr : QSimpleUpdater::getInstance();

  // Start common event timers
  miscTimerEvents->startTimers();

  // Retranslate the QML interface automatically
  connect(miscTranslator,
          &Misc::Translator::languageChanged,
          &m_engine,
          &QQmlApplicationEngine::retranslate);

  // Setup singleton module interconnections
  appState->setupExternalConnections();
  csvExport->setupExternalConnections();
  ioManager->setupExternalConnections();
  mdf4Export->setupExternalConnections();
  frameParser->setupExternalConnections();
  projectModel->setupExternalConnections();
  frameBuilder->setupExternalConnections();
  consoleExport->setupExternalConnections();
  consoleHandler->setupExternalConnections();
  ioFileTransmission->setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  sqliteExport->setupExternalConnections();
  sqliteDbManager->setupExternalConnections();
#endif

  // Wire addon manager signals to theme manager for hot-reloading user themes
  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionInstalled,
          miscThemeManager,
          &Misc::ThemeManager::onExtensionInstalled);
  connect(miscExtensionManager,
          &Misc::ExtensionManager::extensionUninstalled,
          miscThemeManager,
          &Misc::ThemeManager::onExtensionUninstalled);

  // Broadcast lifecycle events to API clients so plugins can save/restore state
  connect(ioManager,
          &IO::ConnectionManager::connectedChanged,
          pluginsBridge,
          [pluginsBridge, ioManager]() {
            pluginsBridge->broadcastLifecycleEvent(ioManager->isConnected()
                                                     ? QStringLiteral("connected")
                                                     : QStringLiteral("disconnected"));
          });

  // Restore last project after all modules are wired so all signals fire correctly
  appState->restoreLastProject();

  // Refresh extensions; plugins auto-launch/stop with the dashboard
  miscExtensionManager->refreshRepositories();
  connect(uiDashboard,
          &UI::Dashboard::widgetCountChanged,
          miscExtensionManager,
          &Misc::ExtensionManager::onDashboardAvailableChanged);

  // Clear notification history when the dashboard resets (disconnect/project reload)
  connect(uiDashboard,
          &UI::Dashboard::dataReset,
          notificationCenter,
          &DataModel::NotificationCenter::clearAll);

  // Install custom message handler to redirect qDebug output to console
  qInstallMessageHandler(MessageHandler);

  // Obtain build date/time
  const auto buildDate = QStringLiteral(__DATE__);
  const auto buildTime = QStringLiteral(__TIME__);

  // Construct a QML-friendly list of available screens
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

  // Register C++ modules with QML
  const auto c = m_engine.rootContext();
  c->setContextProperty("Cpp_AppState", appState);
  c->setContextProperty("Cpp_Updater", updater);
  c->setContextProperty("Cpp_IO_Serial", ioSerial);
  c->setContextProperty("Cpp_CSV_Export", csvExport);
  c->setContextProperty("Cpp_CSV_Player", csvPlayer);
  c->setContextProperty("Cpp_IO_Manager", ioManager);
  c->setContextProperty("Cpp_IO_Network", ioNetwork);
  c->setContextProperty("Cpp_MDF4_Export", mdf4Export);
  c->setContextProperty("Cpp_MDF4_Player", mdf4Player);
  c->setContextProperty("Cpp_Misc_ModuleManager", this);
  c->setContextProperty("Cpp_UI_Dashboard", uiDashboard);
  c->setContextProperty("Cpp_Console_Export", consoleExport);
  c->setContextProperty("Cpp_NativeWindow", &m_nativeWindow);
  c->setContextProperty("Cpp_API_Server", pluginsBridge);
  c->setContextProperty("Cpp_Misc_Utilities", miscUtilities);
  c->setContextProperty("Cpp_IO_Bluetooth_LE", ioBluetoothLE);
  c->setContextProperty("Cpp_ThemeManager", miscThemeManager);
  c->setContextProperty("Cpp_Console_Handler", consoleHandler);
  c->setContextProperty("Cpp_Misc_Translator", miscTranslator);
  c->setContextProperty("Cpp_JSON_ProjectModel", projectModel);
  c->setContextProperty("Cpp_JSON_ProjectEditor", projectEditor);
  c->setContextProperty("Cpp_JSON_FrameBuilder", frameBuilder);
  c->setContextProperty("Cpp_Notifications", notificationCenter);
  c->setContextProperty("Cpp_Misc_TimerEvents", miscTimerEvents);
  c->setContextProperty("Cpp_Misc_CommonFonts", miscCommonFonts);
  c->setContextProperty("Cpp_IO_FileTransmission", ioFileTransmission);
  c->setContextProperty("Cpp_Misc_WorkspaceManager", miscWorkspaceManager);
  c->setContextProperty("Cpp_Examples", miscExamples);
  c->setContextProperty("Cpp_HelpCenter", miscHelpCenter);
  c->setContextProperty("Cpp_ExtensionManager", miscExtensionManager);
  c->setContextProperty("Cpp_Misc_IconEngine", miscIconEngine);

  // Register commercial C++ modules with QML
#ifdef BUILD_COMMERCIAL
  c->setContextProperty("Cpp_IO_Audio", audioDriver);
  c->setContextProperty("Cpp_IO_CANBus", canBusDriver);
  c->setContextProperty("Cpp_IO_Modbus", modbusDriver);
  c->setContextProperty("Cpp_IO_USB", usbDriver);
  c->setContextProperty("Cpp_IO_HID", hidDriver);
  c->setContextProperty("Cpp_IO_Process", processDriver);
  c->setContextProperty("Cpp_JSON_DBCImporter", dbcImporter);
  c->setContextProperty("Cpp_JSON_ModbusMapImporter", modbusMapImporter);
  c->setContextProperty("Cpp_Licensing_Trial", trial);
  c->setContextProperty("Cpp_MQTT_Client", mqttClient);
  c->setContextProperty("Cpp_Licensing_LemonSqueezy", lemonSqueezy);
  c->setContextProperty("Cpp_Sessions_Export", sqliteExport);
  c->setContextProperty("Cpp_Sessions_Player", sqlitePlayer);
  c->setContextProperty("Cpp_Sessions_Manager", sqliteDbManager);
#endif

  // Register app info with QML
  c->setContextProperty("Cpp_AppName", APP_NAME);
  c->setContextProperty("Cpp_BuildDate", buildDate);
  c->setContextProperty("Cpp_BuildTime", buildTime);
  c->setContextProperty("Cpp_ScreenList", screenList);
  c->setContextProperty("Cpp_AppVersion", APP_VERSION);
  c->setContextProperty("Cpp_PrimaryScreen", primaryScreen);
  c->setContextProperty("Cpp_AppUpdaterUrl", APP_UPDATER_URL);
  c->setContextProperty("Cpp_AppOrganization", APP_DEVELOPER);
  c->setContextProperty("Cpp_UpdaterEnabled", autoUpdaterEnabled());
  c->setContextProperty("Cpp_CommercialBuild", qtCommercialAvailable);
  c->setContextProperty("Cpp_GrpcAvailable", grpcAvailable);
  c->setContextProperty("Cpp_AppOrganizationDomain", APP_SUPPORT_URL);

#ifdef ENABLE_GRPC
  c->setContextProperty("Cpp_GRPC_Server", grpcServer);
#endif

  if (!m_headless) {
    // Register image provider for inline SVG action icons
    m_engine.addImageProvider(QStringLiteral("actionicon"), new Misc::ActionIconProvider());

    // Register image provider for Image View widget (pro only)
#ifdef BUILD_COMMERCIAL
    auto* imgProvider = new UI::ImageProvider();
    UI::ImageProvider::setGlobal(imgProvider);
    m_engine.addImageProvider(QStringLiteral("serial-studio-img"), imgProvider);

    auto imageExport = &Widgets::ImageExport::instance();
    imageExport->setupExternalConnections();
    c->setContextProperty("Cpp_Image_Export", imageExport);
#endif

    // Load main.qml
    m_engine.load(QUrl("qrc:/serial-studio.com/gui/qml/main.qml"));

    // Install macOS quit interceptor and wire it to QML
    m_nativeWindow.installMacOSQuitInterceptor();
    connect(&m_nativeWindow, &NativeWindow::quitRequested, this, [this]() {
      auto roots = m_engine.rootObjects();
      if (!roots.isEmpty())
        QMetaObject::invokeMethod(roots.first(), "quitApplication");
    });
  }

  // Try to contact activation server to validate license
#ifdef BUILD_COMMERCIAL
  if (!lemonSqueezy->licensingData().isEmpty())
    QMetaObject::invokeMethod(lemonSqueezy, &Licensing::LemonSqueezy::validate);
#endif
}
