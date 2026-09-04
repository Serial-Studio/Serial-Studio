/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "UI/UISessionRegistry.h"

#include "Core/SSAssert.h"
#include "UI/Taskbar.h"
#include "UI/WindowManager.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the singleton instance of UISessionRegistry.
 */
UI::UISessionRegistry& UI::UISessionRegistry::instance()
{
  static UISessionRegistry registry;
  return registry;
}

/**
 * @brief Constructs a UISessionRegistry and initializes all pointers to null.
 */
UI::UISessionRegistry::UISessionRegistry(QObject* parent) : QObject(parent) {}

//--------------------------------------------------------------------------------------------------
// Taskbar registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a Taskbar to the live set. Every instance is kept, not just the first: an external
 *        dashboard window builds its own taskbar, and a project load can rebuild the main one,
 *        so which instance is "primary" is only answerable at query time.
 */
void UI::UISessionRegistry::registerTaskbar(Taskbar* t)
{
  SS_ASSERT(t != nullptr, return);

  if (m_taskbars.contains(t))
    return;

  const bool wasEmpty = m_taskbars.isEmpty();
  m_taskbars.append(t);
  if (wasEmpty)
    Q_EMIT taskbarAvailable();
}

/**
 * @brief Drops a Taskbar from the live set, announcing unavailability only once the last one goes.
 */
void UI::UISessionRegistry::unregisterTaskbar(Taskbar* t)
{
  if (!m_taskbars.removeOne(t))
    return;

  if (m_taskbars.isEmpty())
    Q_EMIT taskbarUnavailable();
}

//--------------------------------------------------------------------------------------------------
// WindowManager registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a WindowManager to the live set, for the same reason the taskbars are a set: a
 *        canvas rebuilt under a new project must not leave the API addressing the retired one.
 */
void UI::UISessionRegistry::registerWindowManager(WindowManager* wm)
{
  SS_ASSERT(wm != nullptr, return);

  if (m_windowManagers.contains(wm))
    return;

  const bool wasEmpty = m_windowManagers.isEmpty();
  m_windowManagers.append(wm);
  if (wasEmpty)
    Q_EMIT windowManagerAvailable();
}

/**
 * @brief Drops a WindowManager from the live set, announcing unavailability only once the last
 *        one goes.
 */
void UI::UISessionRegistry::unregisterWindowManager(WindowManager* wm)
{
  if (!m_windowManagers.removeOne(wm))
    return;

  if (m_windowManagers.isEmpty())
    Q_EMIT windowManagerUnavailable();
}

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief The taskbar an API caller means: the newest one that follows the project's active
 *        workspace. An external window's taskbar sets `independentWorkspace` from QML AFTER the
 *        constructor has registered, so the flag can only be read here, not at registration; a
 *        registry that latched the first instance could hand `ui.window.setActiveGroup` an
 *        independent taskbar, whose setter skips the ProjectModel write and leaves the main view
 *        unchanged while the command still reports success.
 */
UI::Taskbar* UI::UISessionRegistry::primaryTaskbar() const
{
  for (qsizetype i = m_taskbars.size() - 1; i >= 0; --i) {
    auto* candidate = m_taskbars.at(i);
    if (candidate && !candidate->independentWorkspace())
      return candidate;
  }

  return m_taskbars.isEmpty() ? nullptr : m_taskbars.constLast();
}

/**
 * @brief Returns the registered primary WindowManager, or nullptr if none is registered.
 */
UI::WindowManager* UI::UISessionRegistry::primaryWindowManager() const
{
  return m_windowManagers.isEmpty() ? nullptr : m_windowManagers.constLast();
}
