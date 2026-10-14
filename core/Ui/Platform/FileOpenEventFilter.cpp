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

#include <chrono>
#include <QCoreApplication>
#include <QDebug>
#include <QFileOpenEvent>
#include <QProcess>

#include "AppState.h"
#include "Core/Runtime.h"
#include "Core/SerialStudio.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "Platform/AppPlatform.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const auto s_processStart        = std::chrono::steady_clock::now();
static constexpr auto kColdLaunchWindow = std::chrono::seconds(5);

//--------------------------------------------------------------------------------------------------
// Kept apart from AppPlatform.cpp so the OS-level helpers there link without AppState and the
// project model; eventFilter is the class key function, so its vtable travels with it (spec 0032).
//--------------------------------------------------------------------------------------------------

/**
 * @brief Switches to project mode and opens @p path. Runs from a queued invocation, so both
 *        singletons it reaches are already owned by the session context.
 */
static void openProjectFile(const QString& path)
{
  auto& appState     = DataModel::pipelineModules().appState;
  auto& projectModel = DataModel::pipelineModules().projectModel;
  appState.setOperationMode(SerialStudio::ProjectFile);
  projectModel.openJsonFile(path);
}

/**
 * @brief Opens @p arguments in a separate process: a sealed session never loads a second document,
 *        and a live session is never sealed after the fact (spec 0096). When this process was
 *        started only to open a package (no arguments, event inside the cold-launch window), it
 *        quits once the new process runs.
 */
static void openInNewProcess(const QStringList& arguments, const bool quitIfColdLaunch)
{
  if (!QProcess::startDetached(QCoreApplication::applicationFilePath(), arguments)) {
    qWarning() << "[FileOpenEventFilter] Could not start a process for" << arguments;
    return;
  }

  const bool coldLaunch = QCoreApplication::arguments().size() <= 1
                       && std::chrono::steady_clock::now() - s_processStart < kColdLaunchWindow;
  if (quitIfColdLaunch && coldLaunch && !Core::Runtime::sealed())
    QCoreApplication::quit();
}

namespace Platform {

/**
 * @brief Queues file-open events onto the event loop: the filter is installed before the root, and
 *        the first loop spin is past it (spec 0039 M2). A .ssdeploy always opens in its own sealed
 *        process, and a sealed process forwards a .ssproj to a new one (spec 0096).
 */
bool FileOpenEventFilter::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() != QEvent::FileOpen)
    return QObject::eventFilter(obj, event);

  const QString path = static_cast<QFileOpenEvent*>(event)->file();
#ifdef BUILD_COMMERCIAL
  if (path.endsWith(QStringLiteral(".ssdeploy"), Qt::CaseInsensitive)) {
    const QStringList arguments = {QStringLiteral("--deployment"), path};
    QMetaObject::invokeMethod(
      qApp, [arguments] { openInNewProcess(arguments, true); }, Qt::QueuedConnection);
    return true;
  }
#endif

  if (!path.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
    return QObject::eventFilter(obj, event);

  if (Core::Runtime::sealed())
    QMetaObject::invokeMethod(
      qApp, [path] { openInNewProcess({path}, false); }, Qt::QueuedConnection);
  else
    QMetaObject::invokeMethod(qApp, [path] { openProjectFile(path); }, Qt::QueuedConnection);

  return true;
}

}  // namespace Platform
