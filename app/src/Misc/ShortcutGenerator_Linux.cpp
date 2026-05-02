/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <QtGlobal>

#if defined(BUILD_COMMERCIAL) && defined(Q_OS_LINUX)

#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QProcess>
#  include <QStandardPaths>
#  include <QTextStream>

#  include "Misc/ShortcutGenerator.h"

//--------------------------------------------------------------------------------------------------
// Linux shortcut writer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Escapes control characters per the Desktop Entry spec.
 */
static QString escapeDesktopValue(const QString& value)
{
  QString out;
  out.reserve(value.size());
  for (const QChar ch : value) {
    switch (ch.unicode()) {
      case u'\\':
        out += QStringLiteral("\\\\");
        break;
      case u'\n':
        out += QStringLiteral("\\n");
        break;
      case u'\r':
        out += QStringLiteral("\\r");
        break;
      case u'\t':
        out += QStringLiteral("\\t");
        break;
      default:
        out += ch;
        break;
    }
  }

  return out;
}

/**
 * @brief Doubles literal `%` so the launcher does not treat them as field codes.
 */
static QString escapeExecPercent(const QString& value)
{
  QString out = value;
  out.replace(QLatin1Char('%'), QStringLiteral("%%"));
  return out;
}

/**
 * @brief Writes a freedesktop.org Desktop Entry that launches Serial Studio with args.
 */
bool Misc::ShortcutGenerator::writeLinuxDesktop(const QString& outputPath,
                                                const QString& target,
                                                const QStringList& args,
                                                const QString& title,
                                                const QString& iconPath,
                                                QString& errorOut)
{
  QStringList quoted;
  quoted.reserve(args.size());
  for (const auto& a : args)
    quoted << quoteArg(a);

  QFile file(outputPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    errorOut = tr("Could not open the shortcut path for writing: %1").arg(file.errorString());
    return false;
  }

  const QString safeTitle   = escapeDesktopValue(title);
  const QString safeComment = escapeDesktopValue(tr("Serial Studio shortcut"));
  const QString safeIcon    = escapeDesktopValue(iconPath);
  const QString safeExec    = escapeDesktopValue(escapeExecPercent(
    QStringLiteral("%1 %2").arg(quoteArg(target), quoted.join(QLatin1Char(' ')))));

  QTextStream out(&file);
  out << QStringLiteral("[Desktop Entry]\n");
  out << QStringLiteral("Type=Application\n");
  out << QStringLiteral("Version=1.0\n");
  out << QStringLiteral("Name=%1\n").arg(safeTitle);
  out << QStringLiteral("Comment=%1\n").arg(safeComment);
  out << QStringLiteral("Exec=%1\n").arg(safeExec);
  out << QStringLiteral("Icon=%1\n").arg(safeIcon);
  out << QStringLiteral("Terminal=false\n");
  out << QStringLiteral("Categories=Development;Engineering;\n");
  file.close();

  const auto perms = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
                   | QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther
                   | QFileDevice::ExeOther;
  if (!QFile::setPermissions(outputPath, perms)) {
    errorOut = tr("Could not mark the shortcut as executable.");
    return false;
  }

  // Refresh the desktop database when shipping into the user-applications dir
  const QString applicationsDir = QDir::homePath() + QStringLiteral("/.local/share/applications");
  if (QFileInfo(outputPath).absolutePath() == applicationsDir) {
    const QString tool = QStandardPaths::findExecutable(QStringLiteral("update-desktop-database"));
    if (!tool.isEmpty())
      QProcess::startDetached(tool, {applicationsDir});
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Stubs (only the platform writer is compiled in)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Windows writer is unavailable on Linux builds.
 */
bool Misc::ShortcutGenerator::writeWindowsLnk(const QString&,
                                              const QString&,
                                              const QStringList&,
                                              const QString&,
                                              const QString&,
                                              QString& errorOut)
{
  errorOut = tr("Windows shortcut writer is not available on this platform.");
  return false;
}

/**
 * @brief macOS writer is unavailable on Linux builds.
 */
bool Misc::ShortcutGenerator::writeMacCommand(const QString&,
                                              const QString&,
                                              const QStringList&,
                                              const QString&,
                                              const QString&,
                                              QString& errorOut)
{
  errorOut = tr("macOS shortcut writer is not available on this platform.");
  return false;
}

#endif  // BUILD_COMMERCIAL && Q_OS_LINUX
