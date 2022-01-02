/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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
#include <JSON/Editor.h>
#include <JSON/Dataset.h>
#include <JSON/Generator.h>

#include <IO/Manager.h>
#include <IO/Console.h>
#include <IO/DataSources/Serial.h>
#include <IO/DataSources/Network.h>

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
 * Configures the application font, creates a splash screen and configures
 * application signals/slots to destroy singleton classes before the application
 * quits.
 */
Misc::ModuleManager::ModuleManager()
{
    // Init translator (so that splash screen displays text in user's language)
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

    // Get splash screen image
    QPixmap pixmap;
    if (qApp->devicePixelRatio() >= 2)
        pixmap.load(":/images/splash@2x.png");
    else
        pixmap.load(":/images/splash@1x.png");

#ifdef Q_OS_MAC
    // Disable splash screen shadow on macOS
    m_splash.setWindowFlags(Qt::SplashScreen | Qt::NoDropShadowWindowHint);
#endif

    // Enable software rendering
#ifdef SERIAL_STUDIO_SOFTWARE_RENDERING
    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#endif

    // Show splash screen
    m_splash.setPixmap(pixmap);
    m_splash.show();

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
    auto jsonEditor = &JSON::Editor::instance();
    auto mqttClient = &MQTT::Client::instance();
    auto uiDashboard = &UI::Dashboard::instance();
    auto jsonGenerator = &JSON::Generator::instance();
    auto pluginsBridge = &Plugins::Server::instance();
    auto miscUtilities = &Misc::Utilities::instance();
    auto miscMacExtras = &Misc::MacExtras::instance();
    auto miscTranslator = &Misc::Translator::instance();
    auto ioSerial = &IO::DataSources::Serial::instance();
    auto miscTimerEvents = &Misc::TimerEvents::instance();
    auto ioNetwork = &IO::DataSources::Network::instance();
    auto miscThemeManager = &Misc::ThemeManager::instance();

    // Initialize third-party modules
    auto updater = QSimpleUpdater::getInstance();

    // Operating system flags
    bool isWin = false;
    bool isMac = false;
    bool isNix = false;
#if defined(Q_OS_MAC)
    isMac = true;
#elif defined(Q_OS_WIN)
    isWin = true;
#else
    isNix = true;
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
    c->setContextProperty("Cpp_Updater", updater);
    c->setContextProperty("Cpp_IO_Serial", ioSerial);
    c->setContextProperty("Cpp_CSV_Export", csvExport);
    c->setContextProperty("Cpp_CSV_Player", csvPlayer);
    c->setContextProperty("Cpp_IO_Console", ioConsole);
    c->setContextProperty("Cpp_IO_Manager", ioManager);
    c->setContextProperty("Cpp_IO_Network", ioNetwork);
    c->setContextProperty("Cpp_JSON_Editor", jsonEditor);
    c->setContextProperty("Cpp_MQTT_Client", mqttClient);
    c->setContextProperty("Cpp_UI_Dashboard", uiDashboard);
    c->setContextProperty("Cpp_JSON_Generator", jsonGenerator);
    c->setContextProperty("Cpp_Plugins_Bridge", pluginsBridge);
    c->setContextProperty("Cpp_Misc_MacExtras", miscMacExtras);
    c->setContextProperty("Cpp_Misc_Utilities", miscUtilities);
    c->setContextProperty("Cpp_ThemeManager", miscThemeManager);
    c->setContextProperty("Cpp_Misc_Translator", miscTranslator);
    c->setContextProperty("Cpp_Misc_TimerEvents", miscTimerEvents);
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
    qApp->processEvents();
}

/**
 * Hides the splash screen widget
 */
void Misc::ModuleManager::hideSplashscreen()
{
    m_splash.hide();
    m_splash.close();
    qApp->processEvents();
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

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_ModuleManager.cpp"
#endif
