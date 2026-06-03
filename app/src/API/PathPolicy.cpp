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
#include <QStringList>

//--------------------------------------------------------------------------------------------------
// Path validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a canonical absolute path, optionally permitting non-existent files.
 */
static QString normalizedPath(const QString& path, bool allowNonexistent)
{
  QFileInfo info(path);
  if (info.exists())
    return QDir::cleanPath(info.canonicalFilePath());

  if (!allowNonexistent)
    return QString();

  // Canonicalize the nearest existing ancestor so an 8.3 short path resolves.
  const QString absolute = QDir::cleanPath(info.absoluteFilePath());

  QDir ancestor(absolute);
  QStringList tail;
  constexpr int kMaxDepth = 64;
  for (int i = 0; i < kMaxDepth && !ancestor.exists() && !ancestor.isRoot(); ++i) {
    tail.prepend(ancestor.dirName());
    if (!ancestor.cdUp())
      return absolute;
  }

  const QString canonicalRoot = ancestor.canonicalPath();
  if (canonicalRoot.isEmpty())
    return absolute;

  tail.prepend(canonicalRoot);
  return QDir::cleanPath(tail.join(QDir::separator()));
}

/**
 * @brief Validate a file path against the API allowlist, if configured.
 */
bool API::isPathAllowed(const QString& filePath, const bool allowNonexistent)
{
  // Build allowed-root list from environment or defaults
  QStringList roots;
  if (qEnvironmentVariableIsSet("SERIAL_STUDIO_API_ALLOWED_PATHS")) {
    const QString envValue = QString::fromLocal8Bit(qgetenv("SERIAL_STUDIO_API_ALLOWED_PATHS"));
    if (envValue.trimmed().isEmpty())
      return false;

    roots = envValue.split(QDir::listSeparator(), Qt::SkipEmptyParts);
  }

  else
    roots = {QDir::homePath(), QDir::tempPath()};

  // Normalize and verify the target is under an allowed root
  const QString targetPath = normalizedPath(filePath, allowNonexistent);
  if (targetPath.isEmpty())
    return false;

  for (const auto& root : std::as_const(roots)) {
    const QString rootPath = normalizedPath(root.trimmed(), true);
    if (rootPath.isEmpty())
      continue;

#ifdef Q_OS_WIN
    const Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#endif

    if (targetPath.compare(rootPath, sensitivity) == 0)
      return true;

    // Canonical paths use '/'; QDir::separator() ('\' on Windows) would never match.
    const QString prefix = rootPath + QLatin1Char('/');
    if (targetPath.startsWith(prefix, sensitivity))
      return true;
  }

  return false;
}
