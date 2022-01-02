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
#include <QQuickStyle>
#include <QApplication>
#include <QStyleFactory>

#include <AppInfo.h>
#include <JSON/Frame.h>
#include <Misc/Utilities.h>
#include <Misc/ModuleManager.h>

#ifdef Q_OS_WIN
#    include <windows.h>
#endif

/**
 * Prints the current application version to the console
 */
static void cliShowVersion()
{
    auto appver = QString("%1 version %2").arg(APP_NAME, APP_VERSION);
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
    // Fix console output on Windows (https://stackoverflow.com/a/41701133)
    // This code will only execute if the application is started from the comamnd prompt
#ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        // Open the console's active buffer
        (void)freopen("CONOUT$", "w", stdout);
        (void)freopen("CONOUT$", "w", stderr);

        // Force print new-line (to avoid printing text over user commands)
        printf("\n");
    }
#endif

    // Set application attributes
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    // Init. application
    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName(APP_DEVELOPER);
    app.setOrganizationDomain(APP_SUPPORT_URL);

    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    QQuickStyle::setStyle("Fusion");

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

    // Create module manager
    Misc::ModuleManager moduleManager;
    moduleManager.configureUpdater();

    // Initialize QML interface
    moduleManager.registerQmlTypes();
    moduleManager.initializeQmlInterface();
    if (moduleManager.engine()->rootObjects().isEmpty())
    {
        qCritical() << "Critical QML error";
        return EXIT_FAILURE;
    }

    // Enter application event loop
    return app.exec();
}
