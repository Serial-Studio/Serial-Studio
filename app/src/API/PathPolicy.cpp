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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/PathPolicy.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QStringList>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

//--------------------------------------------------------------------------------------------------
// Declared path parameters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every command parameter the allowlist guards, in one place. The registry reads this at
 *        registration time and enforces it once in execute(), so a handler never re-checks and a
 *        new path-taking command cannot ship ungated (spec 0075 I3/I7). system.exec is absent on
 *        purpose: it is control-script only and its program name is resolved from PATH, which no
 *        allowlist root can contain.
 */
static const QHash<QString, QVector<API::PathParamPolicy>>& pathParamTable()
{
  // clang-format off
  static const QHash<QString, QVector<API::PathParamPolicy>> kPathParamTable = {
    {"project.open",                {{QStringLiteral("filePath"), false}}},
    {"project.save",                {{QStringLiteral("filePath"), true}}},
    {"csvPlayer.open",              {{QStringLiteral("filePath"), false}}},
    {"mdf4Player.open",             {{QStringLiteral("filePath"), false}}},
    {"sessions.openDatabase",       {{QStringLiteral("filePath"), true}}},
    {"sessions.regress",            {{QStringLiteral("projectPath"), false}}},
    {"licensing.activateOffline",   {{QStringLiteral("path"), false}}},
    {"assistant.restore",           {{QStringLiteral("path"), false}}},
    {"io.opcua.exportCertificate",  {{QStringLiteral("path"), true}}},
    {"io.opcua.setUserCertificate", {{QStringLiteral("certificate"), false},
                                     {QStringLiteral("key"), false}}},
    {"io.process.setExecutable",    {{QStringLiteral("executable"), false}}},
    {"io.process.setWorkingDir",    {{QStringLiteral("workingDir"), true}}},
    {"io.process.setPipePath",      {{QStringLiteral("pipePath"), true}}},
  };
  // clang-format on

  return kPathParamTable;
}

/**
 * @brief The path parameters declared for one command; empty for a command that takes none.
 */
QVector<API::PathParamPolicy> API::declaredPathParams(const QString& command)
{
  return pathParamTable().value(command);
}

//--------------------------------------------------------------------------------------------------
// Path validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Expand a Windows 8.3 short path (e.g. RUNNER~1) to its long form.
 */
static QString expandShortPath(const QString& path)
{
#ifdef Q_OS_WIN
  if (path.isEmpty())
    return path;

  const std::wstring input = path.toStdWString();
  const DWORD needed       = GetLongPathNameW(input.c_str(), nullptr, 0);
  if (needed == 0)
    return path;

  std::wstring buffer(needed, L'\0');
  const DWORD written = GetLongPathNameW(input.c_str(), buffer.data(), needed);
  if (written == 0 || written >= needed)
    return path;

  buffer.resize(written);
  return QDir::cleanPath(QString::fromStdWString(buffer));
#else
  return path;
#endif
}

/**
 * @brief Returns a canonical absolute path, optionally permitting non-existent files.
 */
static QString normalizedPath(const QString& path, bool allowNonexistent)
{
  QFileInfo info(path);
  if (info.exists())
    return expandShortPath(QDir::cleanPath(info.canonicalFilePath()));

  if (!allowNonexistent)
    return QString();

  const QString absolute = QDir::cleanPath(info.absoluteFilePath());

  QDir ancestor(absolute);
  QStringList tail;
  constexpr int kMaxDepth = 64;
  for (int i = 0; i < kMaxDepth && !ancestor.exists() && !ancestor.isRoot(); ++i) {
    tail.prepend(ancestor.dirName());
    if (!ancestor.cdUp())
      return expandShortPath(absolute);
  }

  const QString canonicalRoot = ancestor.canonicalPath();
  if (canonicalRoot.isEmpty())
    return expandShortPath(absolute);

  tail.prepend(expandShortPath(canonicalRoot));
  return QDir::cleanPath(tail.join(QDir::separator()));
}

/**
 * @brief Validate a file path against the API allowlist, if configured.
 */
bool API::isPathAllowed(const QString& filePath, const bool allowNonexistent)
{
  QStringList roots;
  if (qEnvironmentVariableIsSet("SERIAL_STUDIO_API_ALLOWED_PATHS")) {
    const QString envValue = QString::fromLocal8Bit(qgetenv("SERIAL_STUDIO_API_ALLOWED_PATHS"));
    if (envValue.trimmed().isEmpty())
      return false;

    roots = envValue.split(QDir::listSeparator(), Qt::SkipEmptyParts);
  }

  else
    roots = {QDir::homePath(), QDir::tempPath()};

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

    const QString prefix = rootPath + QLatin1Char('/');
    if (targetPath.startsWith(prefix, sensitivity))
      return true;
  }

  return false;
}
