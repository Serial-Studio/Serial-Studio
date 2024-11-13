/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <iostream>
#include <QQuickWindow>
#include <QSimpleUpdater.h>

#include "AppInfo.h"
#include "SerialStudio.h"

#include "CSV/Export.h"
#include "CSV/Player.h"

#include "JSON/Group.h"
#include "JSON/Dataset.h"
#include "JSON/FrameParser.h"
#include "JSON/ProjectModel.h"
#include "JSON/FrameBuilder.h"

#include "IO/Manager.h"
#include "IO/Console.h"
#include "IO/Drivers/Serial.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/BluetoothLE.h"

#include "Misc/Utilities.h"
#include "Misc/Translator.h"
#include "Misc/CommonFonts.h"
#include "Misc/TimerEvents.h"
#include "Misc/ThemeManager.h"
#include "Misc/ModuleManager.h"

#include "MQTT/Client.h"
#include "Plugins/Server.h"

#include "UI/Dashboard.h"
#include "UI/DashboardWidget.h"

#include "UI/Widgets/Bar.h"
#include "UI/Widgets/GPS.h"
#include "UI/Widgets/Plot.h"
#include "UI/Widgets/Gauge.h"
#include "UI/Widgets/Compass.h"
#include "UI/Widgets/FFTPlot.h"
#include "UI/Widgets/DataGrid.h"
#include "UI/Widgets/LEDPanel.h"
#include "UI/Widgets/Terminal.h"
#include "UI/Widgets/Gyroscope.h"
#include "UI/Widgets/MultiPlot.h"
#include "UI/Widgets/Accelerometer.h"

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
static void MessageHandler(QtMsgType type, const QMessageLogContext &context,
                           const QString &msg)
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

  // Show message & warning level
  QString output;
  switch (type)
  {
    case QtInfoMsg:
      output = QStringLiteral("[INFO] %1").arg(message);
      break;
    case QtDebugMsg:
      output = QStringLiteral("[DEBG] %1").arg(message);
      break;
    case QtWarningMsg:
      output = QStringLiteral("[WARN] %1").arg(message);
      break;
    case QtCriticalMsg:
      output = QStringLiteral("[CRIT] %1").arg(message);
      break;
    case QtFatalMsg:
      output = QStringLiteral("[FATL] %1").arg(message);
      break;
    default:
      break;
  }

  if (!output.isEmpty())
  {
    // Add a newline at the end
    std::cout << output.toStdString() << std::endl;
    output.append("\n");

    // Use IO::Manager signal to avoid messing up tokens in console
    Q_EMIT IO::Manager::instance().dataReceived(output.toUtf8());
  }
}

/**
 * Configures the application font and configures application signals/slots to
 * destroy singleton classes before the application quits.
 */
Misc::ModuleManager::ModuleManager()
{
  // Init translator
  (void)Misc::Translator::instance();

  // Stop modules when application is about to quit
  connect(&m_engine, &QQmlApplicationEngine::quit, this,
          &Misc::ModuleManager::onQuit);
}

/**
 * Returns a pointer to the QML application engine
 */
const QQmlApplicationEngine &Misc::ModuleManager::engine() const
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
  CSV::Export::instance().closeFile();
  CSV::Player::instance().closeFile();
  IO::Manager::instance().disconnectDevice();
  Misc::TimerEvents::instance().stopTimers();
  Plugins::Server::instance().removeConnection();
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
  qmlRegisterType<Widgets::GPS>("SerialStudio", 1, 0, "GPSModel");
  qmlRegisterType<Widgets::Plot>("SerialStudio", 1, 0, "PlotModel");
  qmlRegisterType<Widgets::Gauge>("SerialStudio", 1, 0, "GaugeModel");
  qmlRegisterType<Widgets::Compass>("SerialStudio", 1, 0, "CompassModel");
  qmlRegisterType<Widgets::FFTPlot>("SerialStudio", 1, 0, "FFTPlotModel");
  qmlRegisterType<Widgets::DataGrid>("SerialStudio", 1, 0, "DataGridModel");
  qmlRegisterType<Widgets::LEDPanel>("SerialStudio", 1, 0, "LEDPanelModel");
  qmlRegisterType<Widgets::Terminal>("SerialStudio", 1, 0, "TerminalWidget");
  qmlRegisterType<Widgets::MultiPlot>("SerialStudio", 1, 0, "MultiPlotModel");
  qmlRegisterType<Widgets::Gyroscope>("SerialStudio", 1, 0, "GyroscopeModel");
  qmlRegisterType<Widgets::Accelerometer>("SerialStudio", 1, 0,
                                          "AccelerometerModel");

  // Register JSON custom items
  qmlRegisterType<JSON::FrameParser>("SerialStudio", 1, 0, "FrameParser");
  qmlRegisterType<JSON::ProjectModel>("SerialStudio", 1, 0, "ProjectModel");

  // Register generic dashboard widget
  qmlRegisterType<UI::DashboardWidget>("SerialStudio", 1, 0, "DashboardWidget");

  // Regsiter common Serial Studio enums & values
  qmlRegisterType<SerialStudio>("SerialStudio", 1, 0, "SerialStudio");
}

/**
 * Initializes all the application modules, registers them with the QML engine
 * and loads the "main.qml" file as the root QML file.
 */
void Misc::ModuleManager::initializeQmlInterface()
{
  // Initialize modules
  auto csvExport = &CSV::Export::instance();
  auto csvPlayer = &CSV::Player::instance();
  auto ioManager = &IO::Manager::instance();
  auto ioConsole = &IO::Console::instance();
  auto mqttClient = &MQTT::Client::instance();
  auto uiDashboard = &UI::Dashboard::instance();
  auto ioSerial = &IO::Drivers::Serial::instance();
  auto pluginsBridge = &Plugins::Server::instance();
  auto miscUtilities = &Misc::Utilities::instance();
  auto ioNetwork = &IO::Drivers::Network::instance();
  auto frameBuilder = &JSON::FrameBuilder::instance();
  auto miscTranslator = &Misc::Translator::instance();
  auto projectModel = &JSON::ProjectModel::instance();
  auto miscTimerEvents = &Misc::TimerEvents::instance();
  auto miscCommonFonts = &Misc::CommonFonts::instance();
  auto miscThemeManager = &Misc::ThemeManager::instance();
  auto ioBluetoothLE = &IO::Drivers::BluetoothLE::instance();

  // Initialize third-party modules
  auto updater = QSimpleUpdater::getInstance();

  // Start common event timers
  miscTimerEvents->startTimers();

  // Retranslate the QML interface automatically
  connect(miscTranslator, &Misc::Translator::languageChanged, &m_engine,
          &QQmlApplicationEngine::retranslate);

  // Obtain build date/time
  const auto buildDate = QStringLiteral(__DATE__);
  const auto buildTime = QStringLiteral(__TIME__);

  // Register C++ modules with QML
  const auto c = m_engine.rootContext();
  c->setContextProperty("Cpp_Updater", updater);
  c->setContextProperty("Cpp_IO_Serial", ioSerial);
  c->setContextProperty("Cpp_CSV_Export", csvExport);
  c->setContextProperty("Cpp_CSV_Player", csvPlayer);
  c->setContextProperty("Cpp_IO_Console", ioConsole);
  c->setContextProperty("Cpp_IO_Manager", ioManager);
  c->setContextProperty("Cpp_IO_Network", ioNetwork);
  c->setContextProperty("Cpp_MQTT_Client", mqttClient);
  c->setContextProperty("Cpp_UI_Dashboard", uiDashboard);
  c->setContextProperty("Cpp_NativeWindow", &m_nativeWindow);
  c->setContextProperty("Cpp_Plugins_Bridge", pluginsBridge);
  c->setContextProperty("Cpp_Misc_Utilities", miscUtilities);
  c->setContextProperty("Cpp_IO_Bluetooth_LE", ioBluetoothLE);
  c->setContextProperty("Cpp_ThemeManager", miscThemeManager);
  c->setContextProperty("Cpp_Misc_Translator", miscTranslator);
  c->setContextProperty("Cpp_JSON_ProjectModel", projectModel);
  c->setContextProperty("Cpp_JSON_FrameBuilder", frameBuilder);
  c->setContextProperty("Cpp_Misc_TimerEvents", miscTimerEvents);
  c->setContextProperty("Cpp_Misc_CommonFonts", miscCommonFonts);
  c->setContextProperty("Cpp_UpdaterEnabled", autoUpdaterEnabled());
  c->setContextProperty("Cpp_ModuleManager", this);
  c->setContextProperty("Cpp_BuildDate", buildDate);
  c->setContextProperty("Cpp_BuildTime", buildTime);
  c->setContextProperty("Cpp_PrimaryScreen", qApp->primaryScreen());

  // Register app info with QML
  c->setContextProperty("Cpp_AppName", qApp->applicationDisplayName());
  c->setContextProperty("Cpp_AppUpdaterUrl", APP_UPDATER_URL);
  c->setContextProperty("Cpp_AppVersion", qApp->applicationVersion());
  c->setContextProperty("Cpp_AppOrganization", qApp->organizationName());
  c->setContextProperty("Cpp_AppOrganizationDomain",
                        qApp->organizationDomain());

  // Load main.qml
  m_engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  // Setup singleton module interconnections
  ioSerial->setupExternalConnections();
  csvExport->setupExternalConnections();
  ioConsole->setupExternalConnections();
  ioManager->setupExternalConnections();
  projectModel->setupExternalConnections();
  frameBuilder->setupExternalConnections();

  // Install custom message handler to redirect qDebug output to console
  qInstallMessageHandler(MessageHandler);
}
