/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <QThread>
#include <QSysInfo>
#include <QSettings>
#include <QQuickStyle>
#include <QApplication>
#include <QStyleFactory>

#include "AppInfo.h"
#include "Misc/ModuleManager.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <cstring>
#endif

#ifdef Q_OS_LINUX
#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QStandardPaths>
#endif

//------------------------------------------------------------------------------
// Declare utility functions
//------------------------------------------------------------------------------

static void cliShowVersion();
static void cliResetSettings();

#ifdef Q_OS_LINUX
static void setupAppImageIcon(const QString &appExecutableName,
                              const QString &iconResourcePath);
#endif

#ifdef Q_OS_WINDOWS
static void attachToConsole();
static char **adjustArgumentsForFreeType(int &argc, char **argv);
#endif

//------------------------------------------------------------------------------
// Entry-point function
//------------------------------------------------------------------------------

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

  // Disable native menubar
  QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

  // Windows specific initialization code
#ifdef Q_OS_WIN
  attachToConsole();
  argv = adjustArgumentsForFreeType(argc, argv);
#endif

  // Linux specific initialization code
#ifdef Q_OS_LINUX
  setupAppImageIcon(APP_EXECUTABLE,
                    QStringLiteral(":/rcc/logo/desktop-icon.png"));
#endif

  // Avoid 200% scaling on 150% scaling...
  auto policy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;
  QApplication::setHighDpiScaleFactorRoundingPolicy(policy);

  // Initialize application
  QApplication app(argc, argv);

  // Set thread priority
  QThread::currentThread()->setPriority(QThread::HighestPriority);

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

  // Ensure resources are loaded
  Q_INIT_RESOURCE(rcc);
  Q_INIT_RESOURCE(translations);

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
  const auto status = app.exec();

  // Free dynamically-generated argv on Windows
#ifdef Q_OS_WIN
  for (int i = 0; i < argc; ++i)
    free(argv[i]);

  free(argv);
#endif

  // Exit application
  return status;
}

//------------------------------------------------------------------------------
// Implement utility functions
//------------------------------------------------------------------------------

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
  QSettings(APP_SUPPORT_URL, APP_NAME).clear();
  qDebug() << APP_NAME << "settings cleared!";
}

//------------------------------------------------------------------------------
// Linux-specific initialization code
//------------------------------------------------------------------------------

#ifdef Q_OS_LINUX
/**
 * @brief Ensures the application icon is set correctly for AppImage deployments
 *        on GNU/Linux.
 *
 * This function copies the application icon from the resource file to the
 * appropriate icon directory on GNU/Linux systems.
 *
 * If the icon file does not already exist in the local user's icons directory,
 * it is copied from the application’s resources to ensure proper display in the
 * desktop environment, even when running the application as an AppImage.
 *
 * The function creates any necessary directories if they do not exist, and
 * performs file checks to prevent redundant copying.
 *
 * @param appExecutableName The name of the application executable, used to name
 *                          the icon file.
 * @param iconResourcePath The path to the icon in the application's resources
 *                         (e.g., `:/rcc/images/icon@2x.png`).
 */
static void setupAppImageIcon(const QString &appExecutableName,
                              const QString &iconResourcePath)
{
  // clang-format off
  const QString pixmapPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/icons/hicolor/256x256/apps/";
  const QString pixmapFile = pixmapPath + appExecutableName + ".png";
  // clang-format on

  // Ensure the directory exists; create it if it doesn't
  QDir dir;
  if (!dir.exists(pixmapPath) && !dir.mkpath(pixmapPath))
    return;

  // Copy the icon from resources to the destination
  QFile resourceFile(iconResourcePath);
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
#endif

//------------------------------------------------------------------------------
// Windows-specific initialization code
//------------------------------------------------------------------------------

#ifdef Q_OS_WIN
/**
 * @brief Attaches the application to the parent console and redirects output
 *        streams on Windows.
 *
 * This function attaches the application to the parent process's console if it
 * was launched from the command prompt. It redirects the `stdout` and `stderr`
 * streams to the console to enable proper output display. Additionally, it
 * prints a newline to avoid overlapping text with any previous user commands
 * in the console.
 */
static void attachToConsole()
{
  if (AttachConsole(ATTACH_PARENT_PROCESS))
  {
    (void)freopen_s(nullptr, "CONOUT$", "w", stdout);
    (void)freopen_s(nullptr, "CONOUT$", "w", stderr);
    printf("\n");
  }
}

/**
 * @brief Adjusts command-line arguments to enable FreeType font rendering on
 *        Windows.
 *
 * This function forces the application to use FreeType font rendering instead
 * of DirectWrite or GDI. This approach fixes pixelated fonts being shown in
 * the user interface in screens whose scale factor is 100%.
 *
 * @param argc Reference to the argument count from `main()`.
 * @param argv Array of command-line arguments from `main()`.
 */
static char **adjustArgumentsForFreeType(int &argc, char **argv)
{
  // Define the additional FreeType arguments for Windows
  const char *platformArgument = "-platform";
  const char *platformOption = "windows:fontengine=freetype";

  // Allocate new argv array
  char **newArgv = static_cast<char **>(malloc(sizeof(char *) * (argc + 2)));
  if (!newArgv)
    return argv;

  // Copy original argv content
  for (int i = 0; i < argc; ++i)
    newArgv[i] = _strdup(argv[i]);

  // Append FreeType platform arguments
  newArgv[argc] = _strdup(platformArgument);
  newArgv[argc + 1] = _strdup(platformOption);

  argc += 2;
  return newArgv;
}
#endif
