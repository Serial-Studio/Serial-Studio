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

#include <QString>
#include <QStringList>

namespace IO {

/**
 * @brief The answer to a request to start a program.
 */
enum class LaunchVerdict : quint8 {
  Allowed,
  ConsentRequired,
  Denied,
};

/**
 * @brief Decides whether the open project may start a program (spec 0098 R8). The composition
 *        root binds the API layer's per-project launch consent, so a driver below that layer asks
 *        the same gate a script's system.exec does, whatever chose the program or triggered the
 *        start. Never blocks: an unanswered consent posts its prompt and answers ConsentRequired.
 */
class ILaunchAuthorizer {
public:
  ILaunchAuthorizer()                                    = default;
  ILaunchAuthorizer(ILaunchAuthorizer&&)                 = delete;
  ILaunchAuthorizer(const ILaunchAuthorizer&)            = delete;
  ILaunchAuthorizer& operator=(ILaunchAuthorizer&&)      = delete;
  ILaunchAuthorizer& operator=(const ILaunchAuthorizer&) = delete;
  virtual ~ILaunchAuthorizer()                           = default;

  [[nodiscard]] virtual LaunchVerdict authorizeLaunch(const QString& program,
                                                      const QStringList& arguments,
                                                      const QString& workingDir) = 0;
};

void setLaunchAuthorizer(ILaunchAuthorizer* authorizer) noexcept;
[[nodiscard]] ILaunchAuthorizer* launchAuthorizer() noexcept;
[[nodiscard]] LaunchVerdict authorizeLaunch(const QString& program,
                                            const QStringList& arguments,
                                            const QString& workingDir);

}  // namespace IO
