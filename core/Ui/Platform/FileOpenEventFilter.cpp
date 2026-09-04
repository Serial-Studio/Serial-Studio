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

#include <QCoreApplication>
#include <QFileOpenEvent>

#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "Platform/AppPlatform.h"
#include "SerialStudio.h"

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
  static auto& appState     = AppState::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);
  projectModel.openJsonFile(path);
}

namespace Platform {

/**
 * @brief Intercepts QFileOpenEvent and queues the .ssproj load onto the event loop. The filter is
 *        installed before the composition root, so handling the event inline would reach AppState
 *        and the project model before either is adopted; the first loop spin is past the root
 *        (spec 0039 M2).
 */
bool FileOpenEventFilter::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::FileOpen) {
    auto* fileEvent    = static_cast<QFileOpenEvent*>(event);
    const QString path = fileEvent->file();
    if (path.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive)) {
      QMetaObject::invokeMethod(qApp, [path] { openProjectFile(path); }, Qt::QueuedConnection);
      return true;
    }
  }

  return QObject::eventFilter(obj, event);
}

}  // namespace Platform
