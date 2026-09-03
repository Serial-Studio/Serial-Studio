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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <cstdio>
#include <QApplication>
#include <QIcon>
#include <QLoggingCategory>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStyleFactory>

#if defined(SS_MIMALLOC_ACTIVE)
#  include <mimalloc.h>
#endif

#include "AppInfo.h"
#include "IO/ConnectionManager.h"
#include "Misc/CLI.h"
#include "Misc/CrashTracker.h"
#include "Misc/GraphicsBackend.h"
#include "Misc/HighDpiScaling.h"
#include "Misc/ModuleManager.h"
#include "Platform/AppPlatform.h"
#include "SessionContext.h"

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
  moduleManager.setEphemeralSession(cli.runtimeMode());
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
 * @brief Prints a teardown stage marker to stderr when SS_TEARDOWN_TRACE is set, so a CI crash
 *        log brackets the exact shutdown region that died.
 */
static void teardownTrace(const char* stage)
{
  if (!qEnvironmentVariableIsSet("SS_TEARDOWN_TRACE"))
    return;

  fprintf(stderr, "[teardown] %s\n", stage);
  fflush(stderr);
}

/**
 * @brief Tears the session down in the pinned reverse order, with qApp alive and the QML engine
 *        already destroyed (INV-6). Every exit from the session scope runs this, including a
 *        failed UI load: returning early instead left nine modules and a live pipeline thread to
 *        the static SessionContext destructor, after ~QApplication (K4).
 */
static void shutdownSession()
{
  teardownTrace("module-manager-destroyed");
  Misc::ModuleManager::stopFrameConsumerWorkers();
  IO::ConnectionManager::instance().shutdownDrivers();
  teardownTrace("drivers-shut-down");

  qInstallMessageHandler(nullptr);
  SessionContext::current().shutdown();
  teardownTrace("session-shutdown-done");
}

/**
 * @brief Brings the session up and runs the event loop, returning the process exit status. A UI
 *        that fails to load returns here rather than out of the caller, so the teardown ladder
 *        below runs on both paths.
 */
static int runConfiguredSession(QApplication& app,
                                Misc::CLI& cli,
                                Misc::ModuleManager& moduleManager,
                                bool headless,
                                const QString& shortcutPath)
{
  if (!bootstrapModuleManager(moduleManager, cli, headless, shortcutPath)) {
    qCritical() << "Critical QML error";
    return EXIT_FAILURE;
  }

  Misc::CrashTracker::instance().setCheckpoint(QStringLiteral("event-loop"));

  if (cli.postRootSelfTestRequested()) {
    const bool ok = cli.runPostRootSelfTests() == Misc::CLI::ProcessResult::ExitSuccess;
    teardownTrace("post-root-selftest-done");
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
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

  const int status = app.exec();
  teardownTrace("event-loop-exited");
  return status;
}

/**
 * @brief Runs the whole application lifecycle inside one QApplication scope: QApplication reads
 *        argv for its entire lifetime, so main() may free the adjusted argv only after this
 *        returns and ~QApplication has run.
 */
static int runApplication(int argc, char** argv, bool headless, const QString& shortcutPath)
{
  Misc::CrashTracker::instance().setCheckpoint(QStringLiteral("qapplication-construct"));
  QApplication app(argc, argv);

  Platform::FileOpenEventFilter fileOpenFilter;
  app.installEventFilter(&fileOpenFilter);

  Platform::TrackpadScrollFilter trackpadScrollFilter;
  app.installEventFilter(&trackpadScrollFilter);

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

  Platform::AppPlatform::inhibitIdleSleep();

  Q_INIT_RESOURCE(rcc);
  Q_INIT_RESOURCE(translations);

  if (!cli.verifyShortcutProject())
    return EXIT_SUCCESS;

#ifdef BUILD_COMMERCIAL
  cli.applyThemeOverride();
#endif

  Misc::CrashTracker::instance().setCheckpoint(QStringLiteral("module-manager-bootstrap"));
  int status = EXIT_SUCCESS;
  {
    Misc::ModuleManager moduleManager;
    status = runConfiguredSession(app, cli, moduleManager, headless, shortcutPath);
  }

  shutdownSession();
  return status;
}

/**
 * @brief Application entry-point: bootstraps Qt, parses CLI flags, runs the event loop. mimalloc:
 *        purge_delay 250 ms batches purges across frames; page_reclaim_on_free=1 lets the freeing
 *        thread adopt cross-thread-freed pages (workers allocate, GUI/sinks free; parked pages
 *        drove the process high-water); arena_purge_mult=4 returns burst memory within a second.
 */
int main(int argc, char** argv)
{
#if defined(SS_MIMALLOC_ACTIVE)
  mi_option_set(mi_option_purge_delay, 250);
  mi_option_set(mi_option_page_reclaim_on_free, 1);
  mi_option_set(mi_option_arena_purge_mult, 4);
#endif

  setupQtApplicationMetadata();

  const bool cliEarlyExit = Misc::CLI::isCliEarlyExit(argc, argv);
  if (!cliEarlyExit)
    Misc::CrashTracker::instance().markStartup();

  const bool benchmark = Misc::CLI::isBenchmarkRequested(argc, argv);
  const bool headless  = Misc::CLI::argvHasFlag(argc, argv, "--headless");
  if (headless || benchmark)
    argv = Platform::AppPlatform::injectPlatformArg(argc, argv, "offscreen");

  const QString shortcutPath = Misc::CLI::argvValueFor(argc, argv, "--shortcut-path");
  Platform::AppPlatform::prepareEnvironment(argc, argv, shortcutPath);
  Platform::AppPlatform::installCrashDumpWriter();

  Misc::CrashTracker::instance().setCheckpoint(QStringLiteral("graphics-backend-apply"));
  Misc::GraphicsBackend::applyConfiguredBackend();
  Misc::HighDpiScaling::applyConfiguredPolicy();

  const int status = runApplication(argc, argv, headless, shortcutPath);
  teardownTrace("qapplication-destroyed");

  Platform::AppPlatform::releaseAdjustedArgv();
  teardownTrace("argv-released");
  return status;
}
