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

#include "Core/IO/LaunchAuthorizer.h"

#include <QDebug>

#include "Core/SSAssert.h"

// The authorizer the composition root bound; null until then
static IO::ILaunchAuthorizer* s_launchAuthorizer = nullptr;

/**
 * @brief Binds the gate every program start asks; the root calls it once per session.
 */
void IO::setLaunchAuthorizer(ILaunchAuthorizer* authorizer) noexcept
{
  s_launchAuthorizer = authorizer;
}

/**
 * @brief The bound gate, or null before the root binds one.
 */
IO::ILaunchAuthorizer* IO::launchAuthorizer() noexcept
{
  return s_launchAuthorizer;
}

/**
 * @brief Asks the bound gate whether a program may start. A root that never wires the launcher
 *        (the verify and regress children, test harnesses) refuses every launch, and says so.
 */
IO::LaunchVerdict IO::authorizeLaunch(const QString& program,
                                      const QStringList& arguments,
                                      const QString& workingDir)
{
  SS_ASSERT(!program.isEmpty(), return LaunchVerdict::Denied);
  if (s_launchAuthorizer == nullptr) {
    qWarning() << "[LaunchAuthorizer] No launch authorizer is bound; refusing to start" << program;
    return LaunchVerdict::Denied;
  }

  return s_launchAuthorizer->authorizeLaunch(program, arguments, workingDir);
}
