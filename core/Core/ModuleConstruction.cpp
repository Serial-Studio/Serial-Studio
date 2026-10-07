/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "Core/ModuleConstruction.h"

#include <QtGlobal>

static const QMetaObject* s_constructing = nullptr;

/**
 * @brief Marks @p module as under construction; the composition root builds on one thread, so a
 *        plain pointer is enough.
 */
Core::ModuleConstruction::Scope::Scope(const QMetaObject& module) noexcept
  : m_previous(s_constructing)
{
  s_constructing = &module;
}

/**
 * @brief Restores whatever was under construction before this scope opened.
 */
Core::ModuleConstruction::Scope::~Scope()
{
  s_constructing = m_previous;
}

/**
 * @brief The module whose constructor is running, or null outside the composition root.
 */
const QMetaObject* Core::ModuleConstruction::constructing() noexcept
{
  return s_constructing;
}

/**
 * @brief Why @p module is unavailable: still under construction, not yet constructed in the
 *        pinned order (naming the module whose constructor reached it), or outside any session
 *        (before adoption or after shutdown).
 */
QByteArray Core::ModuleConstruction::unavailableReason(const QMetaObject& module)
{
  const QByteArray name = module.className();
  if (s_constructing == &module)
    return name + "::instance() reached while " + name + " is still under construction";

  if (s_constructing != nullptr)
    return name + "::instance() reached before " + name + " was constructed (pinned order; "
         + s_constructing->className() + " is constructing)";

  return name + "::instance() reached outside a session (before adoption or after shutdown)";
}

/**
 * @brief Fails a reach into @p module with its reason. Every build: an unavailable module has no
 *        meaningful recovery.
 */
void Core::ModuleConstruction::reportUnavailable(const QMetaObject& module)
{
  qFatal("%s", unavailableReason(module).constData());
}
