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

#include <QtQml>
#include <QSysInfo>
#include <QPalette>
#include <QQuickStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QQmlApplicationEngine>

#include <Logger.h>
#include <QSimpleUpdater.h>
#include <ConsoleAppender.h>

#include "Group.h"
#include "Dataset.h"

#include "Export.h"
#include "AppInfo.h"
#include "Widgets.h"
#include "CsvPlayer.h"
#include "DataProvider.h"
#include "JsonGenerator.h"
#include "Translator.h"
#include "GraphProvider.h"
#include "SerialManager.h"
#include "ModuleManager.h"

/**
 * @brief Entry-point function of the application
 *
 * @param argc argument count
 * @param argv argument data
 *
 * @return qApp exit code
 */
int main(int argc, char **argv)
{
    // Set application attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Init. application
    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName(APP_DEVELOPER);
    app.setOrganizationDomain(APP_SUPPORT_URL);
    app.setStyle(QStyleFactory::create("Fusion"));

    // Configure CuteLogger
    ConsoleAppender *consoleAppender = new ConsoleAppender;
    consoleAppender->setFormat(
        "[%{time}] %{message:-72} [%{TypeOne}] [%{function}]\n");
    cuteLogger->registerAppender(consoleAppender);

    // Begin logging
    LOG_INFO() << "Running on"
               << QSysInfo::prettyProductName().toStdString().c_str();

    // Change application palette
    QPalette p;
    p.setColor(QPalette::Base, QColor(33, 55, 63));
    p.setColor(QPalette::Text, QColor(255, 255, 255));
    p.setColor(QPalette::Link, QColor(64, 157, 160));
    p.setColor(QPalette::Button, QColor(33, 55, 63));
    p.setColor(QPalette::Window, QColor(33, 55, 63));
    p.setColor(QPalette::Highlight, QColor(64, 157, 160));
    p.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    p.setColor(QPalette::WindowText, QColor(255, 255, 255));
    p.setColor(QPalette::ToolTipBase, QColor(230, 224, 178));
    p.setColor(QPalette::ToolTipText, QColor(230, 224, 178));
    p.setColor(QPalette::BrightText, QColor(255, 255, 255));
    p.setColor(QPalette::HighlightedText, QColor(230, 224, 178));
    app.setPalette(p);

    // Init application modules
    Translator translator;
    QQmlApplicationEngine engine;
    auto widgets = Widgets::getInstance();
    auto csvExport = Export::getInstance();
    auto csvPlayer = CsvPlayer::getInstance();
    auto updater = QSimpleUpdater::getInstance();
    auto dataProvider = DataProvider::getInstance();
    auto jsonGenerator = JsonGenerator::getInstance();
    auto graphProvider = GraphProvider::getInstance();
    auto serialManager = SerialManager::getInstance();

    // Log status
    LOG_INFO() << "Finished creating application modules";

    // Register QML types
    qmlRegisterType<Group>("Group", 1, 0, "Group");
    qmlRegisterType<Dataset>("Dataset", 1, 0, "Dataset");

    // Init QML interface
    auto c = engine.rootContext();
    QQuickStyle::setStyle("Fusion");
    c->setContextProperty("CppUpdater", updater);
    c->setContextProperty("CppWidgets", widgets);
    c->setContextProperty("CppExport", csvExport);
    c->setContextProperty("CppCsvPlayer", csvPlayer);
    c->setContextProperty("CppTranslator", &translator);
    c->setContextProperty("CppDataProvider", dataProvider);
    c->setContextProperty("CppJsonGenerator", jsonGenerator);
    c->setContextProperty("CppGraphProvider", graphProvider);
    c->setContextProperty("CppSerialManager", serialManager);
    c->setContextProperty("CppAppName", app.applicationName());
    c->setContextProperty("CppAppUpdaterUrl", APP_UPDATER_URL);
    c->setContextProperty("CppAppVersion", app.applicationVersion());
    c->setContextProperty("CppAppOrganization", app.organizationName());
    c->setContextProperty("CppAppOrganizationDomain", app.organizationDomain());
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Log QML engine status
    LOG_INFO() << "Finished loading QML interface";

    // QML error, exit
    if (engine.rootObjects().isEmpty())
    {
        LOG_WARNING() << "QML engine error";
        return EXIT_FAILURE;
    }

    // Create instance of module manager to automatically
    // call destructors of singleton application modules
    ModuleManager moduleManager;
    Q_UNUSED(moduleManager);

    // Configure the updater
    LOG_INFO() << "Configuring QSimpleUpdater...";
    updater->setNotifyOnUpdate(APP_UPDATER_URL, true);
    updater->setNotifyOnFinish(APP_UPDATER_URL, false);
    updater->setMandatoryUpdate(APP_UPDATER_URL, false);
    LOG_INFO() << "QSimpleUpdater configuration finished!";

    // Enter application event loop
    auto code = app.exec();
    LOG_INFO() << "Application exit code" << code;
    return code;
}
