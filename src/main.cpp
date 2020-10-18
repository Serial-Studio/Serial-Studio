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
#include <QQuickStyle>
#include <QApplication>
#include <QQmlApplicationEngine>

#include <QSimpleUpdater.h>

#include "Group.h"
#include "Dataset.h"

#include "Export.h"
#include "AppInfo.h"
#include "QmlBridge.h"
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

   // Init application modules
   QQmlApplicationEngine engine;
   auto csvExport = Export::getInstance();
   auto qmlBridge = QmlBridge::getInstance();
   auto graphProvider = GraphProvider::getInstance();
   auto serialManager = SerialManager::getInstance();

   // Register QML types
   qmlRegisterType<Group>("Group", 1, 0, "Group");
   qmlRegisterType<Dataset>("Dataset", 1, 0, "Dataset");

   // Init QML interface
   QQuickStyle::setStyle("Fusion");
   engine.rootContext()->setContextProperty("CppExport", csvExport);
   engine.rootContext()->setContextProperty("CppQmlBridge", qmlBridge);
   engine.rootContext()->setContextProperty("CppGraphProvider", graphProvider);
   engine.rootContext()->setContextProperty("CppSerialManager", serialManager);
   engine.rootContext()->setContextProperty("CppAppName", app.applicationName());
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
   QString url = "https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/master/updates.json";
   QSimpleUpdater::getInstance()->setNotifyOnUpdate(url, true);
   QSimpleUpdater::getInstance()->setNotifyOnFinish(url, false);
   QSimpleUpdater::getInstance()->checkForUpdates(url);

   // Enter application event loop
   return app.exec();
}
