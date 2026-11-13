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

#include <optional>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QLatin1StringView>
#include <QString>

namespace API::ScriptCommandPolicy {

/**
 * @brief Commands no project script needs (spec 0120 R10): extension sources and installs, and
 *        licence changes. A loaded project is trusted, so these are refused, never prompted.
 */
inline constexpr QLatin1StringView kRefused[] = {
  QLatin1StringView("extensions.addRepository"),
  QLatin1StringView("extensions.install"),
  QLatin1StringView("extensions.removeRepository"),
  QLatin1StringView("extensions.uninstall"),
  QLatin1StringView("licensing.activate"),
  QLatin1StringView("licensing.activateOffline"),
  QLatin1StringView("licensing.deactivate"),
  QLatin1StringView("licensing.enableTrial"),
  QLatin1StringView("licensing.setLicense"),
};

/**
 * @brief @p path made comparable: its canonical form when it exists, else its clean absolute form.
 */
[[nodiscard]] inline QString comparablePath(const QString& path)
{
  const QFileInfo info(path);
  const auto canonical = info.canonicalFilePath();
  return canonical.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : canonical;
}

/**
 * @brief Whether @p a and @p b name the same file. Case is compared exactly, even where the volume
 *        ignores it, so a case variant fails closed on a case-sensitive volume.
 */
[[nodiscard]] inline bool samePath(const QString& a, const QString& b)
{
  return comparablePath(a).compare(comparablePath(b), Qt::CaseSensitive) == 0;
}

/**
 * @brief The file project.save writes for @p path: ProjectPersistence::apiSaveJsonFile appends
 *        ".ssproj" when the path lacks it, so a check of the raw path would judge another file.
 */
[[nodiscard]] inline QString savedPath(const QString& path)
{
  if (path.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
    return path;

  return path + QStringLiteral(".ssproj");
}

/**
 * @brief Whether a project.save with @p params writes only the open project's own file, without a
 *        Save dialog: a script never picks the file, and never prompts once per call.
 */
[[nodiscard]] inline bool savesOwnFile(const QJsonObject& params, const QString& projectPath)
{
  if (projectPath.isEmpty() || params.value(QStringLiteral("askPath")).toBool())
    return false;

  const auto target = params.value(QStringLiteral("filePath")).toString();
  return target.isEmpty() || samePath(savedPath(target), projectPath);
}

/**
 * @brief The refusal a project script gets for @p name, or nothing when it may run. Saving to
 *        another file is refused because a launch consent follows the file: a script writing over
 *        an already-trusted project would inherit its grant. The project's own file still saves.
 */
[[nodiscard]] inline std::optional<QString> refusal(const QString& name,
                                                    const QJsonObject& params,
                                                    const QString& projectPath)
{
  for (const auto& refused : kRefused)
    if (name == refused)
      return QStringLiteral("%1 is not available to a project's scripts.").arg(name);

  if (name != QLatin1StringView("project.save") || savesOwnFile(params, projectPath))
    return std::nullopt;

  return QStringLiteral("project.save to another file, or through a Save dialog, is not available "
                        "to a project's scripts; omit filePath and askPath to save the project's "
                        "own file.");
}

}  // namespace API::ScriptCommandPolicy
