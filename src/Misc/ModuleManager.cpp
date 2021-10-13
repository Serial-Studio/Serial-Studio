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

#include <JSON/Frame.h>
#include <JSON/Group.h>
#include <JSON/Editor.h>
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
#include <UI/Dashboard.h>

#include <Widgets/Terminal.h>
#include <Widgets/WidgetLoader.h>

#include <QQuickWindow>
#include <QSimpleUpdater.h>

/**
 * Configures the application font, creates a splash screen and configures
 * application signals/slots to destroy singleton classes before the application
 * quits.
 */
ModuleManager::ModuleManager()
{
    // Init translator (so that splash screen displays text in user's language)
    (void)Misc::Translator::getInstance();

    // Load Roboto fonts from resources
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/RobotoMono-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/RobotoMono-Regular.ttf");

    // Set the UI rendering engine
    switch (renderingEngine())
    {
        case 0:
            QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
            break;
        case 1:
            QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
            break;
        case 2:
#if defined(Q_OS_WIN)
            QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
#elif defined(Q_OS_MAC)
            QQuickWindow::setGraphicsApi(QSGRendererInterface::Metal);
#endif
            break;
        default:
            break;
    }

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

    // Show splash screen
    m_splash.setPixmap(QPixmap(":/images/splash.png"));
    m_splash.show();

    // Stop modules when application is about to quit
    setSplashScreenMessage(tr("Initializing..."));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(stopOperations()));
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

    setSplashScreenMessage(tr("Configuring updater..."));
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
void ModuleManager::registerQmlTypes()
{
    qRegisterMetaType<JFI_Object>("JFI_Object");
    qmlRegisterType<JSON::Group>("SerialStudio", 1, 0, "Group");
    qmlRegisterType<JSON::Dataset>("SerialStudio", 1, 0, "Dataset");
    qmlRegisterType<Widgets::Terminal>("SerialStudio", 1, 0, "Terminal");
    qmlRegisterType<Widgets::WidgetLoader>("SerialStudio", 1, 0, "WidgetLoader");
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
    setSplashScreenMessage(tr("Initializing modules..."));
    auto csvExport = CSV::Export::getInstance();
    auto csvPlayer = CSV::Player::getInstance();
    auto ioManager = IO::Manager::getInstance();
    auto ioConsole = IO::Console::getInstance();
    auto updater = QSimpleUpdater::getInstance();
    auto jsonEditor = JSON::Editor::getInstance();
    auto mqttClient = MQTT::Client::getInstance();
    auto uiDashboard = UI::Dashboard::getInstance();
    auto jsonGenerator = JSON::Generator::getInstance();
    auto pluginsBridge = Plugins::Bridge::getInstance();
    auto miscUtilities = Misc::Utilities::getInstance();
    auto miscMacExtras = Misc::MacExtras::getInstance();
    auto miscTranslator = Misc::Translator::getInstance();
    auto ioSerial = IO::DataSources::Serial::getInstance();
    auto miscTimerEvents = Misc::TimerEvents::getInstance();
    auto ioNetwork = IO::DataSources::Network::getInstance();
    auto miscThemeManager = Misc::ThemeManager::getInstance();

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
    bool qt6 = false;
#else
    bool qt6 = true;
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
    setSplashScreenMessage(tr("Loading user interface..."));
    engine()->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Warning! Do not call setSplashScreenMessage() after loading QML user interface
}

/**
 * Returns a pointer to the QML application engine
 */
QQmlApplicationEngine *ModuleManager::engine()
{
    return &m_engine;
}

/**
 * Returns the rendering engine ID to use for rendering the user interface
 */
int ModuleManager::renderingEngine() const
{
    return m_settings.value("renderingEngine", 0).toInt();
}

/**
 * Returns a list with the available UI rendering engines
 */
QVector<QString> ModuleManager::renderingEngines() const
{
    QVector<QString> list;
    list.append("OpenGL");
    list.append("Software");
#if defined(Q_OS_WIN)
    list.append("Direct3D");
#elif defined(Q_OS_MAC)
    list.append("Metal");
#endif

    return list;
}

/**
 * Quits the application
 */
void ModuleManager::quit()
{
    if (JSON::Editor::getInstance()->askSave())
        qApp->quit();
}

/**
 * Hides the splash screen widget
 */
void ModuleManager::hideSplashscreen()
{
    m_splash.hide();
    m_splash.close();
    qApp->processEvents();
}

/**
 * Changes the rendering engine used by the Qt user interface & prompts the user
 * to restart the application.
 */
void ModuleManager::setRenderingEngine(const int engine)
{
    // Validate index
    if (engine >= 0 && engine < renderingEngines().count())
    {
        // Save settings
        m_settings.setValue("renderingEngine", engine);

        // Ask user to quit application
        // clang-format off
        auto ans = Misc::Utilities::showMessageBox(
                    tr("The rendering engine change will take effect after restart"),
                    tr("Do you want to restart %1 now?").arg(APP_NAME), APP_NAME,
                    QMessageBox::Yes | QMessageBox::No);
        // clang-format on

        // Restart application
        if (ans == QMessageBox::Yes)
        {
#ifdef Q_OS_MAC
            auto bundle = qApp->applicationDirPath() + "/../../";
            QProcess::startDetached("open", { "-n", "-a", bundle });
#else
            QProcess::startDetached(qApp->applicationFilePath());
#endif
            qApp->exit();
        }
    }
}

/**
 * Changes the text displayed on the splash screen
 */
void ModuleManager::setSplashScreenMessage(const QString &message)
{
    if (!message.isEmpty())
    {
        m_splash.showMessage(message, Qt::AlignBottom | Qt::AlignLeft, Qt::white);
        qApp->processEvents();
    }
}

/**
 * Calls the functions needed to safely quit the application
 */
void ModuleManager::stopOperations()
{
    Plugins::Bridge::getInstance()->removeConnection();
    CSV::Export::getInstance()->closeFile();
    CSV::Player::getInstance()->closeFile();
    IO::Manager::getInstance()->disconnectDevice();
    Misc::TimerEvents::getInstance()->stopTimers();
    MQTT::Client::getInstance()->disconnectFromHost();
}
