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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QEvent>
#include <QObject>
#include <QString>

namespace Platform {

/**
 * @brief Event filter that loads .ssproj files dropped onto the running app.
 */
class FileOpenEventFilter : public QObject {
public:
  using QObject::QObject;

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
};

/**
 * @brief Per-platform startup hooks (console, AUMID, power, file association, FreeType).
 */
namespace AppPlatform {
void prepareEnvironment(int& argc, char**& argv, const QString& shortcutPath);
void registerFileAssociation();
void releaseAdjustedArgv(int argc, char** argv);
char** injectPlatformArg(int& argc, char** argv, const char* platform);
QString shortcutIdentityHash(const QString& shortcutPath);
}  // namespace AppPlatform

}  // namespace Platform
