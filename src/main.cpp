/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
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
#include <QPalette>
#include <QQuickStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QQmlApplicationEngine>

#include <QSimpleUpdater.h>

#include "Group.h"
#include "Dataset.h"

#include "Export.h"
#include "AppInfo.h"
#include "Widgets.h"
#include "QmlBridge.h"
#include "JsonParser.h"
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
   QQmlApplicationEngine engine;
   auto widgets = Widgets::getInstance();
   auto csvExport = Export::getInstance();
   auto qmlBridge = QmlBridge::getInstance();
   auto jsonParser = JsonParser::getInstance();
   auto updater = QSimpleUpdater::getInstance();
   auto graphProvider = GraphProvider::getInstance();
   auto serialManager = SerialManager::getInstance();

   // Register QML types
   qmlRegisterType<Group>("Group", 1, 0, "Group");
   qmlRegisterType<Dataset>("Dataset", 1, 0, "Dataset");

   // Get console welcome text string
   QString welcomeText = QObject::tr("Failed to load welcome text :(");
   QFile file(QObject::tr(":/messages/Welcome_EN.txt"));
   if (file.open(QFile::ReadOnly))
   {
      welcomeText = QString::fromUtf8(file.readAll());
      file.close();
   }

   // Init QML interface
   QQuickStyle::setStyle("Fusion");
   engine.rootContext()->setContextProperty("CppUpdater", updater);
   engine.rootContext()->setContextProperty("CppWidgets", widgets);
   engine.rootContext()->setContextProperty("CppExport", csvExport);
   engine.rootContext()->setContextProperty("CppQmlBridge", qmlBridge);
   engine.rootContext()->setContextProperty("CppJsonParser", jsonParser);
   engine.rootContext()->setContextProperty("CppWelcomeText", welcomeText);
   engine.rootContext()->setContextProperty("CppGraphProvider", graphProvider);
   engine.rootContext()->setContextProperty("CppSerialManager", serialManager);
   engine.rootContext()->setContextProperty("CppAppName", app.applicationName());
   engine.rootContext()->setContextProperty("CppAppUpdaterUrl", APP_UPDATER_URL);
   engine.rootContext()->setContextProperty("CppAppVersion", app.applicationVersion());
   engine.rootContext()->setContextProperty("CppAppOrganization", app.organizationName());
   engine.rootContext()->setContextProperty("CppAppOrganizationDomain", app.organizationDomain());
   engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

   // QML error, exit
   if (engine.rootObjects().isEmpty())
      return EXIT_FAILURE;

   // Create instance of module manager to automatically
   // call destructors of singleton application modules
   ModuleManager moduleManager;
   Q_UNUSED(moduleManager);

   // Check for updates
   updater->setNotifyOnUpdate(APP_UPDATER_URL, true);
   updater->setNotifyOnFinish(APP_UPDATER_URL, false);
   updater->checkForUpdates(APP_UPDATER_URL);

   // Enter application event loop
   return app.exec();
}
