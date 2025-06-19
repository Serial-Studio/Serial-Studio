/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
#include "IO/ConsoleExport.h"
#include "IO/FileTransmission.h"

#include "IO/Drivers/UART.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/BluetoothLE.h"

#include "Misc/Utilities.h"
#include "Misc/Translator.h"
#include "Misc/CommonFonts.h"
#include "Misc/TimerEvents.h"
#include "Misc/ThemeManager.h"
#include "Misc/ModuleManager.h"
#include "Misc/WorkspaceManager.h"

#include "Plugins/Server.h"

#include "UI/Taskbar.h"
#include "UI/Dashboard.h"
#include "UI/WindowManager.h"
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

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Client.h"
#  include "UI/Widgets/Plot3D.h"
#  include "Licensing/LemonSqueezy.h"
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

  // Read software rendering settings
  m_softwareRendering = m_settings.value("SoftwareRendering", false).toBool();
  if (m_softwareRendering)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

  // Load native graphics API for each platform
  else
  {
#if defined(Q_OS_WIN)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11Rhi);
#elif defined(Q_OS_MACOS)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::MetalRhi);
#else
    QQuickWindow::setGraphicsApi(QSGRendererInterface::VulkanRhi);
#endif
  }

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
 * @brief Returns whether software rendering is currently enabled.
 *
 * This reflects the user's preference for using Qt's software-based
 * scene graph renderer instead of the default hardware-accelerated backend.
 *
 * @return @c true if software rendering is enabled, @c false otherwise.
 *
 * @see QQuickWindow::setGraphicsApi()
 */
bool Misc::ModuleManager::softwareRendering() const
{
  return m_softwareRendering;
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
  IO::Manager::instance().disconnectDevice();
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
  qmlRegisterType<Widgets::DataGrid>("SerialStudio", 1, 0, "DataGridWidget");
  qmlRegisterType<Widgets::LEDPanel>("SerialStudio", 1, 0, "LEDPanelModel");
  qmlRegisterType<Widgets::Terminal>("SerialStudio", 1, 0, "TerminalWidget");
  qmlRegisterType<Widgets::MultiPlot>("SerialStudio", 1, 0, "MultiPlotModel");
  qmlRegisterType<Widgets::Gyroscope>("SerialStudio", 1, 0, "GyroscopeModel");
  qmlRegisterType<Widgets::Accelerometer>("SerialStudio", 1, 0,
                                          "AccelerometerModel");

#ifdef BUILD_COMMERCIAL
  qmlRegisterType<Widgets::Plot3D>("SerialStudio", 1, 0, "Plot3DWidget");
#endif

  // Register JSON custom items
  qmlRegisterType<JSON::FrameParser>("SerialStudio", 1, 0, "FrameParser");
  qmlRegisterType<JSON::ProjectModel>("SerialStudio", 1, 0, "ProjectModel");

  // Register generic dashboard widget
  qmlRegisterType<UI::DashboardWidget>("SerialStudio", 1, 0, "DashboardWidget");

  // Register window manager & taskbar helpers
  qmlRegisterType<UI::Taskbar>("SerialStudio.UI", 1, 0, "TaskBar");
  qmlRegisterType<UI::WindowManager>("SerialStudio.UI", 1, 0, "WindowManager");

  // Regsiter common Serial Studio enums & values
  qmlRegisterSingletonType<SerialStudio>(
      "SerialStudio", 1, 0, "SerialStudio",
      [](QQmlEngine *, QJSEngine *) -> QObject * {
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
#endif

  // Initialize heavily used modules first
  auto miscTranslator = &Misc::Translator::instance();
  auto miscThemeManager = &Misc::ThemeManager::instance();

  // Initialize modules
  auto csvExport = &CSV::Export::instance();
  auto csvPlayer = &CSV::Player::instance();
  auto ioManager = &IO::Manager::instance();
  auto ioConsole = &IO::Console::instance();
  auto uiDashboard = &UI::Dashboard::instance();
  auto ioSerial = &IO::Drivers::UART::instance();
  auto pluginsBridge = &Plugins::Server::instance();
  auto miscUtilities = &Misc::Utilities::instance();
  auto ioNetwork = &IO::Drivers::Network::instance();
  auto frameBuilder = &JSON::FrameBuilder::instance();
  auto projectModel = &JSON::ProjectModel::instance();
  auto miscTimerEvents = &Misc::TimerEvents::instance();
  auto miscCommonFonts = &Misc::CommonFonts::instance();
  auto ioConsoleExport = &IO::ConsoleExport::instance();
  auto ioBluetoothLE = &IO::Drivers::BluetoothLE::instance();
  auto ioFileTransmission = &IO::FileTransmission::instance();
  auto miscWorkspaceManager = &Misc::WorkspaceManager::instance();

  // Initialize commercial modules
#ifdef BUILD_COMMERCIAL
  const bool qtCommercialAvailable = true;
  auto mqttClient = &MQTT::Client::instance();
#else
  const bool qtCommercialAvailable = false;
#endif

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
  c->setContextProperty("Cpp_Misc_ModuleManager", this);
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
  c->setContextProperty("Cpp_IO_ConsoleExport", ioConsoleExport);
  c->setContextProperty("Cpp_IO_FileTransmission", ioFileTransmission);
  c->setContextProperty("Cpp_Misc_WorkspaceManager", miscWorkspaceManager);
  c->setContextProperty("Cpp_QtCommercial_Available", qtCommercialAvailable);

  // Register commercial C++ modules with QML
#ifdef BUILD_COMMERCIAL
  c->setContextProperty("Cpp_MQTT_Client", mqttClient);
  c->setContextProperty("Cpp_Licensing_LemonSqueezy", lemonSqueezy);
#endif

  // Register app info with QML
  c->setContextProperty("Cpp_BuildDate", buildDate);
  c->setContextProperty("Cpp_BuildTime", buildTime);
  c->setContextProperty("Cpp_AppName", APP_NAME);
  c->setContextProperty("Cpp_AppVersion", APP_VERSION);
  c->setContextProperty("Cpp_AppUpdaterUrl", APP_UPDATER_URL);
  c->setContextProperty("Cpp_AppOrganization", APP_DEVELOPER);
  c->setContextProperty("Cpp_UpdaterEnabled", autoUpdaterEnabled());
  c->setContextProperty("Cpp_PrimaryScreen", qApp->primaryScreen());
  c->setContextProperty("Cpp_AppOrganizationDomain", APP_SUPPORT_URL);

  // Load main.qml
  const auto url = QStringLiteral("qrc:/serial-studio.com/gui/qml/main.qml");
  m_engine.load(QUrl(url));

  // Setup singleton module interconnections
  ioSerial->setupExternalConnections();
  csvExport->setupExternalConnections();
  ioConsole->setupExternalConnections();
  ioManager->setupExternalConnections();
  projectModel->setupExternalConnections();
  frameBuilder->setupExternalConnections();
  ioConsoleExport->setupExternalConnections();

  // Install custom message handler to redirect qDebug output to console
  qInstallMessageHandler(MessageHandler);

  // Try to contact activation server to validate license
#ifdef BUILD_COMMERCIAL
  if (!lemonSqueezy->licensingData().isEmpty())
    QMetaObject::invokeMethod(lemonSqueezy, &Licensing::LemonSqueezy::validate);
#endif
}

/**
 * @brief Enables or disables software rendering for the QML scene graph.
 *
 * This function updates the internal setting that controls whether Qt's
 * software renderer should be used. If the new value differs from the current
 * one, the setting is saved to persistent storage and the user is prompted
 * to restart the application in order for the change to take effect.
 *
 * If the user agrees, the application will attempt to restart automatically.
 *
 * @param enabled Set to @c true to enable software rendering, @c false to use
 *        the default (hardware-accelerated) renderer.
 *
 * @see QQuickWindow::setGraphicsApi()
 * @see Misc::Utilities::rebootApplication()
 */
void Misc::ModuleManager::setSoftwareRendering(const bool enabled)
{
  if (m_softwareRendering != enabled)
  {
    m_softwareRendering = enabled;
    m_settings.setValue("SoftwareRendering", enabled);

    auto ans = Misc::Utilities::showMessageBox(
        tr("To apply this change, %1 needs to restart.").arg(APP_NAME),
        tr("Would you like to restart now?"), QMessageBox::Question, APP_NAME,
        QMessageBox::Yes | QMessageBox::No);

    if (ans == QMessageBox::Yes)
      Misc::Utilities::rebootApplication();
  }
}
