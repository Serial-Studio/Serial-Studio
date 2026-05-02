/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include <QApplication>
#include <QIcon>
#include <QLoggingCategory>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStyleFactory>

#include "AppInfo.h"
#include "Misc/CLI.h"
#include "Misc/ModuleManager.h"
#include "Platform/AppPlatform.h"

/**
 * @brief Configures QApplication name/org/version metadata and global Qt attributes.
 */
static void setupQtApplicationMetadata()
{
  QLoggingCategory::setFilterRules("*font*=false");

  QApplication::setApplicationName(APP_EXECUTABLE);
  QApplication::setOrganizationName(APP_DEVELOPER);
  QApplication::setApplicationVersion(APP_VERSION);
  QApplication::setApplicationDisplayName(APP_NAME);
  QApplication::setOrganizationDomain(APP_SUPPORT_URL);

  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
  QApplication::setAttribute(Qt::AA_DontUseNativeMenuWindows);
}

/**
 * @brief Applies the window icon (non-macOS) and Fusion style to the running QApplication.
 */
static void configureApplicationStyle(QApplication& app)
{
#if !defined(Q_OS_MAC)
  QIcon appIcon(QStringLiteral(":/logo/icon.svg"));
  if (!appIcon.isNull())
    app.setWindowIcon(appIcon);
#endif

  app.setStyle(QStyleFactory::create("Fusion"));
  QQuickStyle::setStyle("Fusion");
}

/**
 * @brief Constructs and initializes the ModuleManager, exposing CLI flags to QML.
 */
static bool bootstrapModuleManager(Misc::ModuleManager& moduleManager,
                                   const Misc::CLI& cli,
                                   bool headless,
                                   const QString& shortcutPath)
{
  moduleManager.setHeadless(headless);
  moduleManager.configureUpdater();
  moduleManager.registerQmlTypes();

  const QString settingsSuffix =
    shortcutPath.isEmpty()
      ? QString()
      : QStringLiteral("_") + Platform::AppPlatform::shortcutIdentityHash(shortcutPath);

  const auto ctx = moduleManager.engine().rootContext();
  ctx->setContextProperty("CLI_START_FULLSCREEN", cli.fullscreen());
  ctx->setContextProperty("CLI_HIDE_TOOLBAR", cli.hideToolbar());
  ctx->setContextProperty("CLI_RUNTIME_MODE", cli.runtimeMode());
  ctx->setContextProperty("CLI_SETTINGS_SUFFIX", settingsSuffix);

  moduleManager.initializeQmlInterface();
  return headless || !moduleManager.engine().rootObjects().isEmpty();
}

/**
 * @brief Application entry-point: bootstraps Qt, parses CLI flags, and runs the event loop.
 */
int main(int argc, char** argv)
{
  setupQtApplicationMetadata();

  const bool headless = Misc::CLI::argvHasFlag(argc, argv, "--headless");
  if (headless)
    argv = Platform::AppPlatform::injectPlatformArg(argc, argv, "offscreen");

  const QString shortcutPath = Misc::CLI::argvValueFor(argc, argv, "--shortcut-path");
  Platform::AppPlatform::prepareEnvironment(argc, argv, shortcutPath);

  QApplication app(argc, argv);

  Platform::FileOpenEventFilter fileOpenFilter;
  app.installEventFilter(&fileOpenFilter);

  Platform::AppPlatform::registerFileAssociation();
  configureApplicationStyle(app);

  Misc::CLI cli;
  switch (cli.process(app)) {
    case Misc::CLI::ProcessResult::ExitSuccess:
      return EXIT_SUCCESS;
    case Misc::CLI::ProcessResult::ExitFailure:
      return EXIT_FAILURE;
    case Misc::CLI::ProcessResult::Continue:
      break;
  }

  Q_INIT_RESOURCE(rcc);
  Q_INIT_RESOURCE(translations);

  if (!cli.verifyShortcutProject())
    return EXIT_SUCCESS;

  Misc::ModuleManager moduleManager;
  if (!bootstrapModuleManager(moduleManager, cli, headless, shortcutPath)) {
    qCritical() << "Critical QML error";
    return EXIT_FAILURE;
  }

  cli.applyProjectAndAutoConnect(app);

#ifdef BUILD_COMMERCIAL
  if (cli.runtimeMode())
    cli.applyOperatorRuntimeSettings();
  else
    cli.applyExportToggles();
#endif

  cli.applyVisualizationOptions();
  cli.applyBusConfiguration();

  const auto status = app.exec();

  Platform::AppPlatform::releaseAdjustedArgv(argc, argv);
  return status;
}
