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

#include <AppInfo.h>

#include <CSV/Export.h>
#include <CSV/Player.h>

#include <JSON/Frame.h>
#include <JSON/Group.h>
#include <JSON/Dataset.h>
#include <JSON/Generator.h>

#include <Project/Model.h>
#include <Project/CodeEditor.h>

#include <IO/Manager.h>
#include <IO/Console.h>
#include <IO/Drivers/Serial.h>
#include <IO/Drivers/Network.h>
#include <IO/Drivers/BluetoothLE.h>

#include <Misc/MacExtras.h>
#include <Misc/Utilities.h>
#include <Misc/Translator.h>
#include <Misc/TimerEvents.h>
#include <Misc/ThemeManager.h>
#include <Misc/ModuleManager.h>

#include <MQTT/Client.h>
#include <Plugins/Server.h>

#include <UI/Dashboard.h>
#include <UI/DashboardWidget.h>
#include <UI/Widgets/Terminal.h>

#include <QQuickWindow>
#include <QSimpleUpdater.h>

/**
 * Configures the application font and configures application signals/slots to destroy
 * singleton classes before the application quits.
 */
Misc::ModuleManager::ModuleManager()
{
    // Init translator
    (void)Misc::Translator::instance();

    // Load Roboto fonts from resources
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/RobotoMono-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/RobotoMono-Regular.ttf");

    // Set Roboto as default app font
    QFont font("Roboto");
#if defined(Q_OS_WIN)
    font.setPointSize(9);
#elif defined(Q_OS_MAC)
    font.setPointSize(13);
#elif defined(Q_OS_LINUX)
    font.setPointSize(10);
#endif
    qApp->setFont(font);

    // Enable software rendering
    if (softwareRendering())
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

    // Stop modules when application is about to quit
    connect(engine(), SIGNAL(quit()), this, SLOT(onQuit()));
}

/**
 * Returns a pointer to the QML application engine
 */
QQmlApplicationEngine *Misc::ModuleManager::engine()
{
    return &m_engine;
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
 * - JSON Frame object
 * - JSON Group object
 * - JSON Dataset object
 */
void Misc::ModuleManager::registerQmlTypes()
{
    qmlRegisterType<Widgets::Terminal>("SerialStudio", 1, 0, "Terminal");
    qmlRegisterType<UI::DashboardWidget>("SerialStudio", 1, 0, "DashboardWidget");
}

/**
 * Returns @c true if software rendering is enabled.
 * By default, software rendering is enabled so that Serial Studio
 * works with most computers and virtual machines.
 */
bool Misc::ModuleManager::softwareRendering()
{
    QSettings settings(APP_DEVELOPER, APP_NAME);
    return settings.value("SoftwareRendering", true).toBool();
}

/**
 * Enables or disables the auto-updater system (QSimpleUpdater).
 *
 * To disable QSimpleUpdater, you need to add DEFINES += DISABLE_QSU in the qmake project
 * file. This option is provided for package managers, users are expected to update the
 * application using the same package manager they used for installing it.
 */
bool Misc::ModuleManager::autoUpdaterEnabled()
{
#ifdef DISABLE_QSU
    return false;
#else
    return true;
#endif
}

/**
 * Initializes all the application modules, registers them with the QML engine and loads
 * the "main.qml" file as the root QML file.
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
    auto projectModel = &Project::Model::instance();
    auto ioSerial = &IO::Drivers::Serial::instance();
    auto jsonGenerator = &JSON::Generator::instance();
    auto pluginsBridge = &Plugins::Server::instance();
    auto miscUtilities = &Misc::Utilities::instance();
    auto miscMacExtras = &Misc::MacExtras::instance();
    auto ioNetwork = &IO::Drivers::Network::instance();
    auto miscTranslator = &Misc::Translator::instance();
    auto miscTimerEvents = &Misc::TimerEvents::instance();
    auto miscThemeManager = &Misc::ThemeManager::instance();
    auto projectCodeEditor = &Project::CodeEditor::instance();
    auto ioBluetoothLE = &IO::Drivers::BluetoothLE::instance();

    // Initialize third-party modules
    auto updater = QSimpleUpdater::getInstance();

    // Operating system flags
    bool isWin = false;
    bool isMac = false;
    bool isNix = false;
    QString osName = tr("Unknown OS");
#if defined(Q_OS_MAC)
    isMac = true;
    osName = "macOS";
#elif defined(Q_OS_WIN)
    isWin = true;
    osName = "Windows";
#elif defined(Q_OS_LINUX)
    isNix = true;
    osName = "GNU/Linux";
#else
    isNix = true;
    osName = "UNIX";
#endif

    // Qt version QML flag
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const bool qt6 = false;
#else
    const bool qt6 = true;
#endif

    // Start common event timers
    miscTimerEvents->startTimers();

    // Retranslate the QML interface automagically
    connect(miscTranslator, SIGNAL(languageChanged()), engine(), SLOT(retranslate()));

    // Register C++ modules with QML
    auto c = engine()->rootContext();
    c->setContextProperty("Cpp_Qt6", qt6);
    c->setContextProperty("Cpp_IsWin", isWin);
    c->setContextProperty("Cpp_IsMac", isMac);
    c->setContextProperty("Cpp_IsNix", isNix);
    c->setContextProperty("Cpp_OSName", osName);
    c->setContextProperty("Cpp_Updater", updater);
    c->setContextProperty("Cpp_IO_Serial", ioSerial);
    c->setContextProperty("Cpp_CSV_Export", csvExport);
    c->setContextProperty("Cpp_CSV_Player", csvPlayer);
    c->setContextProperty("Cpp_IO_Console", ioConsole);
    c->setContextProperty("Cpp_IO_Manager", ioManager);
    c->setContextProperty("Cpp_IO_Network", ioNetwork);
    c->setContextProperty("Cpp_MQTT_Client", mqttClient);
    c->setContextProperty("Cpp_UI_Dashboard", uiDashboard);
    c->setContextProperty("Cpp_Project_Model", projectModel);
    c->setContextProperty("Cpp_JSON_Generator", jsonGenerator);
    c->setContextProperty("Cpp_Plugins_Bridge", pluginsBridge);
    c->setContextProperty("Cpp_Misc_MacExtras", miscMacExtras);
    c->setContextProperty("Cpp_Misc_Utilities", miscUtilities);
    c->setContextProperty("Cpp_IO_Bluetooth_LE", ioBluetoothLE);
    c->setContextProperty("Cpp_ThemeManager", miscThemeManager);
    c->setContextProperty("Cpp_Misc_Translator", miscTranslator);
    c->setContextProperty("Cpp_Misc_TimerEvents", miscTimerEvents);
    c->setContextProperty("Cpp_Project_CodeEditor", projectCodeEditor);
    c->setContextProperty("Cpp_UpdaterEnabled", autoUpdaterEnabled());
    c->setContextProperty("Cpp_ModuleManager", this);

    // Register app info with QML
    c->setContextProperty("Cpp_AppName", qApp->applicationName());
    c->setContextProperty("Cpp_AppUpdaterUrl", APP_UPDATER_URL);
    c->setContextProperty("Cpp_AppVersion", qApp->applicationVersion());
    c->setContextProperty("Cpp_AppOrganization", qApp->organizationName());
    c->setContextProperty("Cpp_AppOrganizationDomain", qApp->organizationDomain());

    // Load main.qml
    engine()->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
}

/**
 * Calls the functions needed to safely quit the application
 */
void Misc::ModuleManager::onQuit()
{
    CSV::Export::instance().closeFile();
    CSV::Player::instance().closeFile();
    IO::Manager::instance().disconnectDriver();
    Misc::TimerEvents::instance().stopTimers();
    Plugins::Server::instance().removeConnection();
}

/**
 * Enables or disables software rendering, which might be useful when
 * dealing with virtual machine environments.
 */
void Misc::ModuleManager::setSoftwareRenderingEnabled(const bool enabled)
{
    // Change settings
    QSettings settings(APP_DEVELOPER, APP_NAME);
    settings.setValue("SoftwareRendering", enabled);

    // Ask user to quit application
    // clang-format off
    auto ans = Utilities::showMessageBox(
        tr("The change will take effect after restart"),
        tr("Do you want to restart %1 now?").arg(APP_NAME), APP_NAME,
        QMessageBox::Yes | QMessageBox::No);
    // clang-format on

    // Restart application
    if (ans == QMessageBox::Yes)
        Utilities::rebootApplication();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_ModuleManager.cpp"
#endif
