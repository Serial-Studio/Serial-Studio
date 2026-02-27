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
#include "Console/Export.h"
#include "Console/Handler.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/FrameParser.h"
#include "DataModel/ProjectModel.h"
#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "IO/FileTransmission.h"
#include "IO/Manager.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "Misc/CommonFonts.h"
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
#  include "IO/Drivers/Audio.h"
#  include "IO/Drivers/CANBus.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/USB.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "MQTT/Client.h"
#  include "UI/Widgets/Plot3D.h"
#endif

/**
 * @brief Custom message handler for Qt debug, warning, critical, and fatal
 *        messages.
 *
 * This static function handles messages of various types (debug, warning,
 * critical, fatal) emitted by the Qt framework.
 *
 * It formats the message with a corresponding prefix based on the message type
 * (e.g., "[Debug]", "[Warning]").
 *
 * @param type The type of the message.
 * @param context The message log context (file, line, function).
 * @param msg The actual message that was emitted by the Qt system.
 *
 * @note The function appends the formatted message to the console handler.
 */
static void MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  // Skip empty messages
  (void)context;
  if (msg.isEmpty())
    return;

  // Add function to string
  QString message;
  if (context.function)
    message = QStringLiteral("%1 - %2").arg(context.function, msg);
  else
    message = msg;

  // Filter out unfixable Windows warnings...I tried, Qt won
#ifdef Q_OS_WIN
  if (type == QtWarningMsg) {
    if (message.startsWith("Qt was built without Direct3D 12 support"))
      return;
    if (message.startsWith("setGeometry: Unable to set geometry"))
      return;
  }
#endif

  // Format message with ANSI colors (only if enabled by user)
  const bool useAnsiColors = Console::Handler::instance().ansiColorsEnabled();
  QString output           = Widgets::Terminal::formatDebugMessage(type, message, useAnsiColors);
  if (!output.isEmpty()) {
    // Add a newline at the end
    std::cout << output.toStdString() << std::endl;
    output.append("\n");

    // Display data in console
    Console::Handler::instance().displayDebugData(output);
  }
}

/**
 * Configures the application font and configures application signals/slots to
 * destroy singleton classes before the application quits.
 */
Misc::ModuleManager::ModuleManager()
  : m_automaticUpdates(m_settings.value("App/AutomaticUpdates", true).toBool())
{
  // Init translator
  (void)Misc::Translator::instance();

  // Stop modules when application is about to quit
  connect(&m_engine, &QQmlApplicationEngine::quit, this, &Misc::ModuleManager::onQuit);
}

/**
 * @brief Returns whether automatic update checks are enabled.
 */
bool Misc::ModuleManager::automaticUpdates() const
{
  return m_automaticUpdates;
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

/**
 * Returns a pointer to the QML application engine
 */
const QQmlApplicationEngine& Misc::ModuleManager::engine() const
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
bool Misc::ModuleManager::autoUpdaterEnabled() const
{
#ifdef DISABLE_QSU
  return false;
#else
  return true;
#endif
}

/**
 * Calls the functions needed to safely quit the application
 */
void Misc::ModuleManager::onQuit()
{
  Misc::TimerEvents::instance().stopTimers();

  CSV::Export::instance().closeFile();
  CSV::Player::instance().closeFile();
  MDF4::Export::instance().closeFile();
  IO::Manager::instance().disconnectDevice();
  API::Server::instance().removeConnection();
}

/**
 * Sets the default options for QSimpleUpdater, which are:
 * - Notify user when a new update is found
 * - Do not notify user when we finish checking for updates
 * - Do not close application if update is found
 */
void Misc::ModuleManager::configureUpdater()
{
  if (!autoUpdaterEnabled())
    return;

  QSimpleUpdater::getInstance()->setNotifyOnUpdate(APP_UPDATER_URL, true);
  QSimpleUpdater::getInstance()->setNotifyOnFinish(APP_UPDATER_URL, false);
  QSimpleUpdater::getInstance()->setMandatoryUpdate(APP_UPDATER_URL, false);
}

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
#endif

  // Register JSON custom items
  qmlRegisterType<DataModel::FrameParser>("SerialStudio", 1, 0, "FrameParser");
  qmlRegisterType<DataModel::ProjectModel>("SerialStudio", 1, 0, "ProjectModel");

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
  auto ioManager            = &IO::Manager::instance();
  auto csvExport            = &CSV::Export::instance();
  auto csvPlayer            = &CSV::Player::instance();
  auto mdf4Export           = &MDF4::Export::instance();
  auto mdf4Player           = &MDF4::Player::instance();
  auto uiDashboard          = &UI::Dashboard::instance();
  auto ioSerial             = &IO::Drivers::UART::instance();
  auto pluginsBridge        = &API::Server::instance();
  auto miscUtilities        = &Misc::Utilities::instance();
  auto consoleExport        = &Console::Export::instance();
  auto ioNetwork            = &IO::Drivers::Network::instance();
  auto frameBuilder         = &DataModel::FrameBuilder::instance();
  auto projectModel         = &DataModel::ProjectModel::instance();
  auto consoleHandler       = &Console::Handler::instance();
  auto miscTimerEvents      = &Misc::TimerEvents::instance();
  auto miscCommonFonts      = &Misc::CommonFonts::instance();
  auto ioBluetoothLE        = &IO::Drivers::BluetoothLE::instance();
  auto ioFileTransmission   = &IO::FileTransmission::instance();
  auto miscWorkspaceManager = &Misc::WorkspaceManager::instance();

  // Initialize commercial modules
#ifdef BUILD_COMMERCIAL
  const bool qtCommercialAvailable = true;
  auto mqttClient                  = &MQTT::Client::instance();
  auto dbcImporter                 = &DataModel::DBCImporter::instance();
  auto audioDriver                 = &IO::Drivers::Audio::instance();
  auto canBusDriver                = &IO::Drivers::CANBus::instance();
  auto modbusDriver                = &IO::Drivers::Modbus::instance();
  auto usbDriver                   = &IO::Drivers::USB::instance();
#else
  const bool qtCommercialAvailable = false;
#endif

  // Initialize third-party modules
  auto updater = QSimpleUpdater::getInstance();

  // Start common event timers
  miscTimerEvents->startTimers();

  // Retranslate the QML interface automatically
  connect(miscTranslator,
          &Misc::Translator::languageChanged,
          &m_engine,
          &QQmlApplicationEngine::retranslate);

  // Setup singleton module interconnections
  ioSerial->setupExternalConnections();
  csvExport->setupExternalConnections();
  ioManager->setupExternalConnections();
  mdf4Export->setupExternalConnections();
  projectModel->setupExternalConnections();
  frameBuilder->setupExternalConnections();
  consoleExport->setupExternalConnections();
  consoleHandler->setupExternalConnections();

#ifdef BUILD_COMMERCIAL
  modbusDriver->setupExternalConnections();
  canBusDriver->setupExternalConnections();
  usbDriver->setupExternalConnections();
#endif

  // Install custom message handler to redirect qDebug output to console
  qInstallMessageHandler(MessageHandler);

  // Obtain build date/time
  const auto buildDate = QStringLiteral(__DATE__);
  const auto buildTime = QStringLiteral(__TIME__);

  // Construct a QML-friendly list of available screens
  QVariantList screenList;
  for (int i = 0; i < qApp->screens().count(); ++i) {
    QVariantMap map;
    map["name"]     = qApp->screens()[i]->name();
    map["geometry"] = QVariant::fromValue(qApp->screens()[i]->geometry());
    screenList.append(map);
  }

  // Get primary screen
  QVariantMap primaryScreen;
  primaryScreen["name"]     = qApp->primaryScreen()->name();
  primaryScreen["geometry"] = qApp->primaryScreen()->geometry();

  // Register C++ modules with QML
  const auto c = m_engine.rootContext();
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
  c->setContextProperty("Cpp_JSON_FrameBuilder", frameBuilder);
  c->setContextProperty("Cpp_Misc_TimerEvents", miscTimerEvents);
  c->setContextProperty("Cpp_Misc_CommonFonts", miscCommonFonts);
  c->setContextProperty("Cpp_IO_FileTransmission", ioFileTransmission);
  c->setContextProperty("Cpp_Misc_WorkspaceManager", miscWorkspaceManager);

  // Register commercial C++ modules with QML
#ifdef BUILD_COMMERCIAL
  c->setContextProperty("Cpp_IO_Audio", audioDriver);
  c->setContextProperty("Cpp_IO_CANBus", canBusDriver);
  c->setContextProperty("Cpp_IO_Modbus", modbusDriver);
  c->setContextProperty("Cpp_IO_USB", usbDriver);
  c->setContextProperty("Cpp_JSON_DBCImporter", dbcImporter);
  c->setContextProperty("Cpp_Licensing_Trial", trial);
  c->setContextProperty("Cpp_MQTT_Client", mqttClient);
  c->setContextProperty("Cpp_Licensing_LemonSqueezy", lemonSqueezy);
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
  c->setContextProperty("Cpp_AppOrganizationDomain", APP_SUPPORT_URL);

  // Load main.qml
  m_engine.load(QUrl("qrc:/serial-studio.com/gui/qml/main.qml"));

  // Try to contact activation server to validate license
#ifdef BUILD_COMMERCIAL
  if (!lemonSqueezy->licensingData().isEmpty())
    QMetaObject::invokeMethod(lemonSqueezy, &Licensing::LemonSqueezy::validate);
#endif
}
