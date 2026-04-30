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

#ifdef BUILD_COMMERCIAL

#  include "Misc/ShortcutGenerator.h"

#  include <QCoreApplication>
#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QProcess>
#  include <QStandardPaths>
#  include <QUrl>

#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Singleton accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide ShortcutGenerator instance.
 */
Misc::ShortcutGenerator& Misc::ShortcutGenerator::instance()
{
  static ShortcutGenerator self;
  return self;
}

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Default-constructs the singleton.
 */
Misc::ShortcutGenerator::ShortcutGenerator() = default;

/**
 * @brief Default-destructs the singleton.
 */
Misc::ShortcutGenerator::~ShortcutGenerator() = default;

//--------------------------------------------------------------------------------------------------
// Constant getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief QRC path of the SVG used as the shortcut preview in the dialog.
 */
QString Misc::ShortcutGenerator::defaultIconPath() const
{
  return QStringLiteral("qrc:/rcc/images/shortcut.svg");
}

/**
 * @brief FileDialog filter for the host OS shortcut format.
 */
QString Misc::ShortcutGenerator::shortcutFileFilter() const
{
#  if defined(Q_OS_WIN)
  return tr("Windows Shortcut (*.lnk)");
#  elif defined(Q_OS_MAC)
  return tr("macOS Application (*.app)");
#  else
  return tr("Desktop Entry (*.desktop)");
#  endif
}

/**
 * @brief Default suffix applied when the save dialog has none.
 */
QString Misc::ShortcutGenerator::shortcutDefaultSuffix() const
{
#  if defined(Q_OS_WIN)
  return QStringLiteral("lnk");
#  elif defined(Q_OS_MAC)
  return QStringLiteral("app");
#  else
  return QStringLiteral("desktop");
#  endif
}

/**
 * @brief Footnote describing platform-specific shortcut quirks.
 */
QString Misc::ShortcutGenerator::platformNote() const
{
#  if defined(Q_OS_MAC)
  return tr("Use a .icns icon for the sharpest result in Finder and the Dock.");
#  elif defined(Q_OS_WIN)
  return tr("Leave the icon empty to inherit the Serial Studio executable icon.");
#  else
  return tr("Place the file under ~/.local/share/applications/ to expose it in your "
            "application launcher.");
#  endif
}

/**
 * @brief FileDialog filter for the icon picker on the host OS.
 */
QString Misc::ShortcutGenerator::iconFileFilter() const
{
#  if defined(Q_OS_MAC)
  return tr("Apple Icon Image (*.icns)");
#  elif defined(Q_OS_WIN)
  return tr("Windows Icon (*.ico)");
#  else
  return tr("Vector or Raster Image (*.svg *.png)");
#  endif
}

//--------------------------------------------------------------------------------------------------
// Argument builder
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the CLI argument list that reproduces the dialog selections.
 */
QStringList Misc::ShortcutGenerator::buildArguments(const QString& projectFile,
                                                    bool fullscreen,
                                                    bool actionsPanel,
                                                    bool csvExport,
                                                    bool mdfExport,
                                                    bool sessionExport,
                                                    bool consoleExport) const
{
  QStringList args;
  const QString resolvedProject =
    QUrl(projectFile).isLocalFile() ? QUrl(projectFile).toLocalFile() : projectFile;
  if (!resolvedProject.isEmpty()) {
    args << QStringLiteral("--project");
    args << resolvedProject;
  }

  // --runtime implies hide-toolbar and quit-on-disconnect.
  args << QStringLiteral("--runtime");
  if (fullscreen)
    args << QStringLiteral("--fullscreen");

  if (actionsPanel)
    args << QStringLiteral("--actions-panel");

  if (csvExport)
    args << QStringLiteral("--csv-export");

  if (mdfExport)
    args << QStringLiteral("--mdf-export");

  if (sessionExport)
    args << QStringLiteral("--session-export");

  if (consoleExport)
    args << QStringLiteral("--console-export");

  return args;
}

//--------------------------------------------------------------------------------------------------
// Generate / rewrite
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes a fresh shortcut + sidecar at the user-chosen path.
 */
void Misc::ShortcutGenerator::generate(const QString& outputPath,
                                       const QString& title,
                                       const QString& iconPath,
                                       const QString& projectFile,
                                       bool fullscreen,
                                       bool actionsPanel,
                                       bool csvExport,
                                       bool mdfExport,
                                       bool sessionExport,
                                       bool consoleExport)
{
  if (!hasProLicense()) {
    Q_EMIT shortcutFailed(tr("A Pro license is required to generate shortcuts."));
    return;
  }

  if (outputPath.isEmpty()) {
    Q_EMIT shortcutFailed(tr("No output path was provided."));
    return;
  }

  QString resolvedPath =
    QUrl(outputPath).isLocalFile() ? QUrl(outputPath).toLocalFile() : outputPath;

#  if defined(Q_OS_MAC)
  // Force .app suffix so revealFile targets the bundle the writer produces.
  if (!resolvedPath.endsWith(QStringLiteral(".app"), Qt::CaseInsensitive))
    resolvedPath += QStringLiteral(".app");
#  endif

  const QString resolvedProject =
    QUrl(projectFile).isLocalFile() ? QUrl(projectFile).toLocalFile() : projectFile;

  QStringList args = buildArguments(
    resolvedProject, fullscreen, actionsPanel, csvExport, mdfExport, sessionExport, consoleExport);

  // Pass shortcut path so a relaunch can offer to delete it if the project is gone
  args << QStringLiteral("--shortcut-path") << resolvedPath;

  QString resolvedIcon = iconPath;
  if (QUrl(resolvedIcon).isLocalFile())
    resolvedIcon = QUrl(resolvedIcon).toLocalFile();

  if (resolvedIcon.startsWith(QStringLiteral("qrc:")))
    resolvedIcon.clear();

  if (resolvedIcon.isEmpty() || !QFileInfo::exists(resolvedIcon))
    resolvedIcon = extractDefaultIcon();

  QString error;
  if (!writeShortcutFile(resolvedPath, title, resolvedIcon, args, error)) {
    Q_EMIT shortcutFailed(error.isEmpty() ? tr("Failed to write shortcut file.") : error);
    return;
  }

  // `open -R` selects the .app bundle in Finder; revealFile would step into it.
#  if defined(Q_OS_MAC)
  QProcess::startDetached(QStringLiteral("/usr/bin/open"), {QStringLiteral("-R"), resolvedPath});
#  else
  Misc::Utilities::revealFile(resolvedPath);
#  endif

  Q_EMIT shortcutGenerated(resolvedPath);
}

/**
 * @brief Removes a shortcut on disk. macOS .app bundles are removed recursively.
 */
void Misc::ShortcutGenerator::deleteShortcut(const QString& shortcutPath)
{
  const QString resolved =
    QUrl(shortcutPath).isLocalFile() ? QUrl(shortcutPath).toLocalFile() : shortcutPath;
  if (resolved.isEmpty())
    return;

  QFileInfo info(resolved);
  if (!info.exists())
    return;

  if (info.isDir())
    QDir(resolved).removeRecursively();
  else
    QFile::remove(resolved);
}

//--------------------------------------------------------------------------------------------------
// File-write dispatcher
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches to the per-OS shortcut writer for the host platform.
 */
bool Misc::ShortcutGenerator::writeShortcutFile(const QString& outputPath,
                                                const QString& title,
                                                const QString& iconPath,
                                                const QStringList& args,
                                                QString& errorOut)
{
  const QString target = QCoreApplication::applicationFilePath();

#  if defined(Q_OS_WIN)
  return writeWindowsLnk(outputPath, target, args, title, iconPath, errorOut);
#  elif defined(Q_OS_MAC)
  return writeMacCommand(outputPath, target, args, title, iconPath, errorOut);
#  else
  return writeLinuxDesktop(outputPath, target, args, title, iconPath, errorOut);
#  endif
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wraps an argument in double quotes when it contains whitespace.
 */
QString Misc::ShortcutGenerator::quoteArg(const QString& arg) const
{
  if (arg.isEmpty())
    return QStringLiteral("\"\"");

  if (!arg.contains(QLatin1Char(' ')) && !arg.contains(QLatin1Char('\t'))
      && !arg.contains(QLatin1Char('"')))
    return arg;

  QString escaped = arg;
  escaped.replace(QLatin1Char('"'), QStringLiteral("\\\""));
  return QStringLiteral("\"%1\"").arg(escaped);
}

/**
 * @brief Returns whether the running build has an active Pro entitlement.
 */
bool Misc::ShortcutGenerator::hasProLicense() const
{
  return Licensing::LemonSqueezy::instance().isActivated()
      || Licensing::Trial::instance().trialEnabled();
}

/**
 * @brief Caches the bundled default shortcut icon on disk and returns its path.
 */
QString Misc::ShortcutGenerator::extractDefaultIcon() const
{
#  if defined(Q_OS_WIN)
  const QString src    = QStringLiteral(":/rcc/images/shortcut.ico");
  const QString dirRel = QStringLiteral("/Shortcuts/Icons");
  const QString file   = QStringLiteral("default.ico");
  const QString base   = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#  elif defined(Q_OS_MAC)
  const QString src    = QStringLiteral(":/rcc/images/shortcut.icns");
  const QString dirRel = QStringLiteral("/Shortcuts/Icons");
  const QString file   = QStringLiteral("default.icns");
  const QString base   = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#  else
  const QString src    = QStringLiteral(":/rcc/images/shortcut.svg");
  const QString dirRel = QString();
  const QString file   = QStringLiteral("serial-studio-shortcut.svg");
  const QString base   = QDir::homePath() + QStringLiteral("/.local/share/icons");
#  endif

  const QString dir = base + dirRel;
  QDir().mkpath(dir);

  const QString dst = dir + QLatin1Char('/') + file;

  // Re-extract whenever the cached copy is missing or has a different size.
  QFile in(src);
  if (!in.open(QIODevice::ReadOnly))
    return QFileInfo::exists(dst) ? dst : QString();

  const QByteArray payload = in.readAll();
  in.close();
  if (payload.isEmpty())
    return QFileInfo::exists(dst) ? dst : QString();

  const auto cachedSize = QFileInfo(dst).size();
  if (!QFileInfo::exists(dst) || cachedSize != payload.size()) {
    QFile out(dst);
    if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      out.write(payload);
      out.close();
    }
  }

  return QFileInfo::exists(dst) ? dst : QString();
}

#endif  // BUILD_COMMERCIAL
