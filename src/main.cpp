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
#include <QApplication>

#include <Logger.h>
#include <AppInfo.h>
#include <Misc/Utilities.h>
#include <Misc/ModuleManager.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

/**
 * Prints the current application version to the console
 */
static void cliShowVersion()
{
    auto appver = QString("%1 version %2").arg(APP_NAME).arg(APP_VERSION);
    auto author = QString("Written by Alex Spataru <https://github.com/alex-spataru>");

    qDebug() << appver.toStdString().c_str();
    qDebug() << author.toStdString().c_str();
}

/**
 * Removes all application settings
 */
static void cliResetSettings()
{
    QSettings(APP_DEVELOPER, APP_NAME).clear();
    qDebug() << APP_NAME << "settings cleared!";
}

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
    // Fix console output on Windows
#ifdef _WIN32
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		qDebug() << "";
	}
#endif

    // Set application attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Init. application
    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName(APP_DEVELOPER);
    app.setOrganizationDomain(APP_SUPPORT_URL);

    // Read arguments
    QString arguments;
    if (app.arguments().count() >= 2)
        arguments = app.arguments().at(1);

    // There are some CLI arguments, read them
    if (!arguments.isEmpty() && arguments.startsWith("-"))
    {
        if (arguments == "-v" || arguments == "--version")
        {
            cliShowVersion();
            return EXIT_SUCCESS;
        }

        else if (arguments == "-r" || arguments == "--reset")
        {
            cliResetSettings();
            return EXIT_SUCCESS;
        }
    }
	
    // Create module manager & configure the logger
    ModuleManager moduleManager;
    moduleManager.configureLogger();

    // Configure dark UI
    Misc::Utilities::configureDarkUi();

    // Begin logging
    LOG_INFO() << QDateTime::currentDateTime();
    LOG_INFO() << APP_NAME << APP_VERSION;
    LOG_INFO() << "Running on" << QSysInfo::prettyProductName().toStdString().c_str();

    // Initialize QML interface
    moduleManager.registerQmlTypes();
    moduleManager.initializeQmlInterface();
    if (moduleManager.engine()->rootObjects().isEmpty())
    {
        LOG_FATAL() << "Critical QML error";
        return EXIT_FAILURE;
    }

    // Configure the updater
    moduleManager.configureUpdater();

    // Enter application event loop
    auto code = app.exec();
    LOG_INFO() << "Application exit code" << code;
    return code;
}
