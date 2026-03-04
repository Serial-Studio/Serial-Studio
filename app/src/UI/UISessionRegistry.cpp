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

UI::UISessionRegistry& UI::UISessionRegistry::instance()
{
  static UISessionRegistry registry;
  return registry;
}

UI::UISessionRegistry::UISessionRegistry(QObject* parent)
  : QObject(parent), m_primaryTaskbar(nullptr), m_primaryWindowManager(nullptr)
{}

void UI::UISessionRegistry::registerTaskbar(Taskbar* t)
{
  if (m_primaryTaskbar)
    return;

  m_primaryTaskbar = t;
  emit taskbarAvailable();
}

void UI::UISessionRegistry::unregisterTaskbar(Taskbar* t)
{
  if (m_primaryTaskbar != t)
    return;

  m_primaryTaskbar = nullptr;
  emit taskbarUnavailable();
}

void UI::UISessionRegistry::registerWindowManager(WindowManager* wm)
{
  if (m_primaryWindowManager)
    return;

  m_primaryWindowManager = wm;
  emit windowManagerAvailable();
}

void UI::UISessionRegistry::unregisterWindowManager(WindowManager* wm)
{
  if (m_primaryWindowManager != wm)
    return;

  m_primaryWindowManager = nullptr;
  emit windowManagerUnavailable();
}

UI::Taskbar* UI::UISessionRegistry::primaryTaskbar() const
{
  return m_primaryTaskbar;
}

UI::WindowManager* UI::UISessionRegistry::primaryWindowManager() const
{
  return m_primaryWindowManager;
}
