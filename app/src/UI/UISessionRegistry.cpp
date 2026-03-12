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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/UISessionRegistry.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the singleton instance of UISessionRegistry.
 * @return Reference to the global UISessionRegistry instance.
 */
UI::UISessionRegistry& UI::UISessionRegistry::instance()
{
  static UISessionRegistry registry;
  return registry;
}

/**
 * @brief Constructs a UISessionRegistry and initializes all pointers to null.
 * @param parent Optional QObject parent.
 */
UI::UISessionRegistry::UISessionRegistry(QObject* parent)
  : QObject(parent), m_primaryTaskbar(nullptr), m_primaryWindowManager(nullptr)
{}

//--------------------------------------------------------------------------------------------------
// Taskbar registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the first Taskbar instance as the primary taskbar.
 * @param t Pointer to the Taskbar to register.
 */
void UI::UISessionRegistry::registerTaskbar(Taskbar* t)
{
  if (m_primaryTaskbar)
    return;

  m_primaryTaskbar = t;
  Q_EMIT taskbarAvailable();
}

/**
 * @brief Unregisters the primary Taskbar if it matches the given pointer.
 * @param t Pointer to the Taskbar to unregister.
 */
void UI::UISessionRegistry::unregisterTaskbar(Taskbar* t)
{
  if (m_primaryTaskbar != t)
    return;

  m_primaryTaskbar = nullptr;
  Q_EMIT taskbarUnavailable();
}

//--------------------------------------------------------------------------------------------------
// WindowManager registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the first WindowManager instance as the primary window manager.
 * @param wm Pointer to the WindowManager to register.
 */
void UI::UISessionRegistry::registerWindowManager(WindowManager* wm)
{
  if (m_primaryWindowManager)
    return;

  m_primaryWindowManager = wm;
  Q_EMIT windowManagerAvailable();
}

/**
 * @brief Unregisters the primary WindowManager if it matches the given pointer.
 * @param wm Pointer to the WindowManager to unregister.
 */
void UI::UISessionRegistry::unregisterWindowManager(WindowManager* wm)
{
  if (m_primaryWindowManager != wm)
    return;

  m_primaryWindowManager = nullptr;
  Q_EMIT windowManagerUnavailable();
}

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the registered primary Taskbar, or nullptr if none is registered.
 * @return Pointer to the primary Taskbar.
 */
UI::Taskbar* UI::UISessionRegistry::primaryTaskbar() const
{
  return m_primaryTaskbar;
}

/**
 * @brief Returns the registered primary WindowManager, or nullptr if none is registered.
 * @return Pointer to the primary WindowManager.
 */
UI::WindowManager* UI::UISessionRegistry::primaryWindowManager() const
{
  return m_primaryWindowManager;
}
