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

#include "API/PathPolicy.h"

#include <QDir>
#include <QFileInfo>

namespace
{
QString normalizedPath(const QString &path, bool allowNonexistent)
{
  QFileInfo info(path);
  if (info.exists())
    return QDir::cleanPath(info.canonicalFilePath());

  if (allowNonexistent)
    return QDir::cleanPath(info.absoluteFilePath());

  return QString();
}
} // namespace

/**
 * @brief Validate a file path against the API allowlist, if configured.
 *
 * If the SERIAL_STUDIO_API_ALLOWED_PATHS environment variable is unset,
 * the path is allowed. When set, it must be inside one of the allowed roots.
 *
 * @param filePath Path to validate
 * @param allowNonexistent Allow non-existent paths (for save operations)
 * @return true if the path is allowed
 */
bool API::isPathAllowed(const QString &filePath, const bool allowNonexistent)
{
  // Create a list of allowed paths
  QStringList roots;

  // Extract data from environment variables
  if (qEnvironmentVariableIsSet("SERIAL_STUDIO_API_ALLOWED_PATHS"))
  {
    const QString envValue
        = QString::fromLocal8Bit(qgetenv("SERIAL_STUDIO_API_ALLOWED_PATHS"));
    if (envValue.trimmed().isEmpty())
      return false;

    roots = envValue.split(QDir::listSeparator(), Qt::SkipEmptyParts);
  }

  // Default to home path & temp. path
  else
    roots = {QDir::homePath(), QDir::tempPath()};

  // Obtain clean path
  const QString targetPath = normalizedPath(filePath, allowNonexistent);
  if (targetPath.isEmpty())
    return false;

  // Verify if path is inside allowed paths
  for (const auto &root : std::as_const(roots))
  {
    // Get clean path for allowed paths
    const QString rootPath = normalizedPath(root.trimmed(), true);
    if (rootPath.isEmpty())
      continue;

    // Set case sensitivity
#ifdef Q_OS_WIN
    const Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#endif

    // Target path is exactly the same as the allowed path
    if (targetPath.compare(rootPath, sensitivity) == 0)
      return true;

    // Target path is inside the allowed path
    const QString prefix = rootPath + QDir::separator();
    if (targetPath.startsWith(prefix, sensitivity))
      return true;
  }

  // Target path is not in allowed paths
  return false;
}
