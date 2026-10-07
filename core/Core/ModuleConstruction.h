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

#pragma once

#include <QByteArray>
#include <QMetaObject>

/**
 * @file ModuleConstruction.h
 * @brief Which core module the composition root is constructing right now (spec 0095 M4), so a
 *        reach into a module that is not yet available fails naming the module and the reason
 *        instead of one generic message. A plain module-static, never a construction, so every
 *        library can read it without including the session context.
 */

namespace Core::ModuleConstruction {

/**
 * @brief Marks @p module as under construction for the scope's lifetime, restoring the previous
 *        mark on exit. Held by the session context around each module's constructor.
 */
class Scope {
public:
  explicit Scope(const QMetaObject& module) noexcept;
  ~Scope();

  Scope(Scope&&)                 = delete;
  Scope(const Scope&)            = delete;
  Scope& operator=(Scope&&)      = delete;
  Scope& operator=(const Scope&) = delete;

private:
  const QMetaObject* m_previous;
};

[[nodiscard]] const QMetaObject* constructing() noexcept;
[[nodiscard]] QByteArray unavailableReason(const QMetaObject& module);
[[noreturn]] void reportUnavailable(const QMetaObject& module);

}  // namespace Core::ModuleConstruction
