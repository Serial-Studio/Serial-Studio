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

#include <QSysInfo>
#include <QSettings>
#include <QQuickStyle>
#include <QApplication>
#include <QStyleFactory>

#include "AppInfo.h"
#include "Misc/ModuleManager.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#ifdef Q_OS_LINUX
#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QStandardPaths>
#endif

/**
 * Prints the current application version to the console
 */
static void cliShowVersion()
{
  qDebug() << APP_NAME << "version" << APP_VERSION;
  qDebug() << "Written by Alex Spataru <https://github.com/alex-spataru>";
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
  // Set application info
  QApplication::setApplicationName(APP_EXECUTABLE);
  QApplication::setOrganizationName(APP_DEVELOPER);
  QApplication::setApplicationVersion(APP_VERSION);
  QApplication::setApplicationDisplayName(APP_NAME);
  QApplication::setOrganizationDomain(APP_SUPPORT_URL);

  // Fix console output on Windows (https://stackoverflow.com/a/41701133)
  // This code will only execute if the application is started from the comamnd
  // prompt
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

// Fix AppImage icon on GNU/Linux
#ifdef Q_OS_LINUX
  // Define the icon path where the icon should be copied to
  const auto pixmapPath
      = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + "/icons/hicolor/256x256/apps/";
  const auto pixmapFile = pixmapPath + APP_EXECUTABLE + ".png";
  const auto resourcePath = ":/rcc/images/icon@2x.png";

  // Check if the file already exists
  if (!QFileInfo::exists(pixmapFile))
  {
    // Ensure the directory exists, create it if it doesn't
    QDir dir;
    bool pathExists = dir.exists(pixmapPath);
    if (!pathExists)
      pathExists = dir.mkpath(pixmapPath);

    // Copy the icon from resources to the destination
    if (pathExists)
    {
      QFile resourceFile(resourcePath);
      if (resourceFile.open(QIODevice::ReadOnly))
      {
        QFile localFile(pixmapFile);
        if (localFile.open(QIODevice::WriteOnly))
        {
          localFile.write(resourceFile.readAll());
          localFile.close();
        }

        resourceFile.close();
      }
    }
  }
#endif

  // Set application attributes
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  // Avoid 200% scaling on 150% scaling...
  auto policy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;
  QApplication::setHighDpiScaleFactorRoundingPolicy(policy);

  // Init. application
  QApplication app(argc, argv);

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
  if (moduleManager.engine().rootObjects().isEmpty())
  {
    qCritical() << "Critical QML error";
    return EXIT_FAILURE;
  }

  // Enter application event loop
  return app.exec();
}
