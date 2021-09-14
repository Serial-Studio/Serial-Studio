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

#include "ModuleManager.h"

#include <AppInfo.h>

#include <CSV/Export.h>
#include <CSV/Player.h>

#include <UI/DataProvider.h>
#include <UI/GraphProvider.h>
#include <UI/WidgetProvider.h>
#include <UI/QmlPlainTextEdit.h>

#include <JSON/Frame.h>
#include <JSON/Group.h>
#include <JSON/Dataset.h>
#include <JSON/Generator.h>
#include <JSON/FrameInfo.h>

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
#include <Plugins/Bridge.h>

#include <Logger.h>
#include <FileAppender.h>
#include <QSimpleUpdater.h>
#include <ConsoleAppender.h>

/**
 * Connect SIGNALS/SLOTS to call singleton destructors before the application
 * quits.
 */
ModuleManager::ModuleManager()
{
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(stopOperations()));
}

/**
 * Configures the CuteLogger library to write application logs to the console and
 * to a file in the system's temp. folder.
 */
void ModuleManager::configureLogger()
{
    auto fileAppender = new FileAppender;
    auto consoleAppender = new ConsoleAppender;
    fileAppender->setFormat(LOG_FORMAT);
    fileAppender->setFileName(LOG_FILE);
    consoleAppender->setFormat(LOG_FORMAT);
    cuteLogger->registerAppender(fileAppender);
    cuteLogger->registerAppender(consoleAppender);
}

/**
 * Sets the default options for QSimpleUpdater, which are:
 * - Notify user when a new update is found
 * - Do not notify user when we finish checking for updates
 * - Do not close application if update is found
 */
void ModuleManager::configureUpdater()
{
    if (!autoUpdaterEnabled())
        return;

    LOG_TRACE() << "Configuring QSimpleUpdater...";
    QSimpleUpdater::getInstance()->setNotifyOnUpdate(APP_UPDATER_URL, true);
    QSimpleUpdater::getInstance()->setNotifyOnFinish(APP_UPDATER_URL, false);
    QSimpleUpdater::getInstance()->setMandatoryUpdate(APP_UPDATER_URL, false);
    LOG_TRACE() << "QSimpleUpdater configuration finished!";
}

/**
 * Register custom QML types, for the moment, we have:
 * - JSON Frame object
 * - JSON Group object
 * - JSON Dataset object
 */
void ModuleManager::registerQmlTypes()
{
    LOG_TRACE() << "Registering QML types...";
    qRegisterMetaType<JFI_Object>("JFI_Object");
    qmlRegisterType<JSON::Group>("SerialStudio", 1, 0, "Group");
    qmlRegisterType<JSON::Dataset>("SerialStudio", 1, 0, "Dataset");
    qmlRegisterType<UI::QmlPlainTextEdit>("SerialStudio", 1, 0, "QmlPlainTextEdit");
    LOG_TRACE() << "QML types registered!";
}

/**
 * Enables or disables the auto-updater system (QSimpleUpdater).
 *
 * To disable QSimpleUpdater, you need to add DEFINES += DISABLE_QSU in the qmake project
 * file. This option is provided for package managers, users are expected to update the
 * application using the same package manager they used for installing it.
 */
bool ModuleManager::autoUpdaterEnabled()
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
void ModuleManager::initializeQmlInterface()
{
    // Initialize modules
    LOG_INFO() << "Initializing C++ modules";
    auto translator = Misc::Translator::getInstance();
    auto csvExport = CSV::Export::getInstance();
    auto csvPlayer = CSV::Player::getInstance();
    auto updater = QSimpleUpdater::getInstance();
    auto uiDataProvider = UI::DataProvider::getInstance();
    auto uiGraphProvider = UI::GraphProvider::getInstance();
    auto uiWidgetProvider = UI::WidgetProvider::getInstance();
    auto ioManager = IO::Manager::getInstance();
    auto ioConsole = IO::Console::getInstance();
    auto ioSerial = IO::DataSources::Serial::getInstance();
    auto ioNetwork = IO::DataSources::Network::getInstance();
    auto jsonGenerator = JSON::Generator::getInstance();
    auto utilities = Misc::Utilities::getInstance();
    auto themeManager = Misc::ThemeManager::getInstance();
    auto mqttPublisher = MQTT::Client::getInstance();
    auto pluginsBridge = Plugins::Bridge::getInstance();
    auto macExtras = Misc::MacExtras::getInstance();
    LOG_INFO() << "Finished initializing C++ modules";

    // Retranslate the QML interface automagically
    LOG_INFO() << "Loading QML interface...";
    connect(translator, SIGNAL(languageChanged()), engine(), SLOT(retranslate()));

    // Register C++ modules with QML
    auto c = engine()->rootContext();
    c->setContextProperty("Cpp_Updater", updater);
    c->setContextProperty("Cpp_UpdaterEnabled", autoUpdaterEnabled());
    c->setContextProperty("Cpp_Misc_Translator", translator);
    c->setContextProperty("Cpp_Misc_Utilities", utilities);
    c->setContextProperty("Cpp_ThemeManager", themeManager);
    c->setContextProperty("Cpp_CSV_Export", csvExport);
    c->setContextProperty("Cpp_CSV_Player", csvPlayer);
    c->setContextProperty("Cpp_UI_Provider", uiDataProvider);
    c->setContextProperty("Cpp_UI_GraphProvider", uiGraphProvider);
    c->setContextProperty("Cpp_UI_WidgetProvider", uiWidgetProvider);
    c->setContextProperty("Cpp_IO_Console", ioConsole);
    c->setContextProperty("Cpp_IO_Manager", ioManager);
    c->setContextProperty("Cpp_IO_Serial", ioSerial);
    c->setContextProperty("Cpp_IO_Network", ioNetwork);
    c->setContextProperty("Cpp_JSON_Generator", jsonGenerator);
    c->setContextProperty("Cpp_MQTT_Client", mqttPublisher);
    c->setContextProperty("Cpp_Plugins_Bridge", pluginsBridge);
    c->setContextProperty("Cpp_Misc_MacExtras", macExtras);

    // Register app info with QML
    c->setContextProperty("Cpp_AppName", qApp->applicationName());
    c->setContextProperty("Cpp_AppUpdaterUrl", APP_UPDATER_URL);
    c->setContextProperty("Cpp_AppVersion", qApp->applicationVersion());
    c->setContextProperty("Cpp_AppOrganization", qApp->organizationName());
    c->setContextProperty("Cpp_AppOrganizationDomain", qApp->organizationDomain());

    // Load main.qml
    engine()->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Start common event timers
    Misc::TimerEvents::getInstance()->startTimers();

    // Log QML engine status
    LOG_INFO() << "Finished loading QML interface";
}

/**
 * Returns a pointer to the QML application engine
 */
QQmlApplicationEngine *ModuleManager::engine()
{
    return &m_engine;
}

/**
 * Calls the functions needed to safely quit the application
 */
void ModuleManager::stopOperations()
{
    LOG_INFO() << "Stopping application modules...";

    Plugins::Bridge::getInstance()->removeConnection();
    CSV::Export::getInstance()->closeFile();
    CSV::Player::getInstance()->closeFile();
    IO::Manager::getInstance()->disconnectDevice();
    Misc::TimerEvents::getInstance()->stopTimers();
    MQTT::Client::getInstance()->disconnectFromHost();

    LOG_INFO() << "Application modules stopped";
}
