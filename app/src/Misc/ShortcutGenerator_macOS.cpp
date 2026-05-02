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

#if defined(BUILD_COMMERCIAL) && defined(Q_OS_MAC)

#  include <QCoreApplication>
#  include <QCryptographicHash>
#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QProcess>
#  include <QRegularExpression>
#  include <QTextStream>
#  include <QUrl>

#  include "Misc/ShortcutGenerator.h"

//--------------------------------------------------------------------------------------------------
// macOS shortcut writer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Removes a file or directory tree at the given path.
 */
static bool removeAnyPath(const QString& path)
{
  QFileInfo info(path);
  if (!info.exists())
    return true;

  if (info.isDir())
    return QDir(path).removeRecursively();

  return QFile::remove(path);
}

/**
 * @brief Sanitizes a bundle base name into a valid CFBundleExecutable identifier.
 */
static QString sanitizeBundleExecName(const QString& bundleName)
{
  QString execName = bundleName;
  execName.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]")), QStringLiteral("_"));
  if (execName.isEmpty())
    execName = QStringLiteral("SerialStudioShortcut");

  return execName;
}

/**
 * @brief Strips file:// and qrc: prefixes from a user-supplied icon path.
 */
static QString resolveIconSource(const QString& iconPath)
{
  QString iconSource = iconPath;
  if (QUrl(iconSource).isLocalFile())
    iconSource = QUrl(iconSource).toLocalFile();

  if (iconSource.startsWith(QStringLiteral("qrc:")))
    iconSource.clear();

  return iconSource;
}

/**
 * @brief Copies the icon into the bundle's Resources folder; returns the file name on success.
 */
static QString copyIconIntoBundle(const QString& iconSource, const QString& resourcesDir)
{
  if (iconSource.isEmpty() || !QFileInfo::exists(iconSource))
    return QString();

  const QString srcSuffix = QFileInfo(iconSource).suffix().toLower();
  const QString iconExt   = srcSuffix.isEmpty() ? QStringLiteral("png") : srcSuffix;
  const QString candidate = QStringLiteral("icon.") + iconExt;
  const QString destPath  = resourcesDir + QLatin1Char('/') + candidate;
  if (QFile::copy(iconSource, destPath))
    return candidate;

  return QString();
}

/**
 * @brief Removes the com.apple.quarantine xattr so Gatekeeper does not block the launcher.
 */
static void stripQuarantine(const QString& bundlePath)
{
  QProcess::execute(QStringLiteral("/usr/bin/xattr"),
                    {QStringLiteral("-dr"), QStringLiteral("com.apple.quarantine"), bundlePath});
}

/**
 * @brief Writes a tiny PkgInfo blob next to Info.plist for legacy Finder compatibility.
 */
static void writePkgInfo(const QString& contentsDir)
{
  QFile file(contentsDir + QStringLiteral("/PkgInfo"));
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    file.write("APPL????");
    file.close();
  }
}

/**
 * @brief Writes the shell launcher inside MacOS/<execName> and chmods it executable.
 */
bool Misc::ShortcutGenerator::writeBundleLauncher(const QString& execPath,
                                                  const QString& target,
                                                  const QStringList& args,
                                                  QString& errorOut)
{
  QFile file(execPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    errorOut = tr("Could not write the bundle launcher: %1").arg(file.errorString());
    return false;
  }

  QStringList quoted;
  quoted.reserve(args.size());
  for (const auto& a : args)
    quoted << quoteArg(a);

  QTextStream out(&file);
  out << QStringLiteral("#!/usr/bin/env bash\n");
  out << QStringLiteral("# Serial Studio operator shortcut\n");
  out << QStringLiteral("exec %1 %2\n").arg(quoteArg(target), quoted.join(QLatin1Char(' ')));
  file.close();

  const auto perms = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
                   | QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther
                   | QFileDevice::ExeOther;
  if (!QFile::setPermissions(execPath, perms)) {
    errorOut = tr("Could not mark the bundle launcher as executable.");
    return false;
  }

  return true;
}

/**
 * @brief Writes the bundle's Info.plist with executable, identifier, and optional icon.
 */
bool Misc::ShortcutGenerator::writeInfoPlist(const QString& plistPath,
                                             const QString& bundlePath,
                                             const QString& bundleName,
                                             const QString& execName,
                                             const QString& title,
                                             const QString& iconFileName,
                                             QString& errorOut)
{
  QFile file(plistPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    errorOut = tr("Could not write Info.plist: %1").arg(file.errorString());
    return false;
  }

  QString display = title.isEmpty() ? bundleName : title;
  display.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
  display.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
  display.replace(QLatin1Char('>'), QStringLiteral("&gt;"));

  QTextStream out(&file);
  out << QStringLiteral(R"(<?xml version="1.0" encoding="UTF-8"?>)") << '\n';
  out << QStringLiteral(R"(<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" )"
                        R"("http://www.apple.com/DTDs/PropertyList-1.0.dtd">)")
      << '\n';
  out << QStringLiteral("<plist version=\"1.0\">\n");
  out << QStringLiteral("<dict>\n");
  out << QStringLiteral("  <key>CFBundleName</key><string>%1</string>\n").arg(display);
  out << QStringLiteral("  <key>CFBundleDisplayName</key><string>%1</string>\n").arg(display);
  out << QStringLiteral("  <key>CFBundleExecutable</key><string>%1</string>\n").arg(execName);

  const QString pathHash =
    QString::fromLatin1(QCryptographicHash::hash(QFileInfo(bundlePath).absoluteFilePath().toUtf8(),
                                                 QCryptographicHash::Sha1)
                          .toHex()
                          .left(12));
  out << QStringLiteral("  <key>CFBundleIdentifier</key>"
                        "<string>com.serial-studio.shortcut.%1.%2</string>\n")
           .arg(execName, pathHash);
  out << QStringLiteral("  <key>CFBundlePackageType</key><string>APPL</string>\n");
  out << QStringLiteral("  <key>CFBundleVersion</key><string>1.0</string>\n");
  out << QStringLiteral("  <key>CFBundleShortVersionString</key><string>1.0</string>\n");
  out << QStringLiteral("  <key>CFBundleInfoDictionaryVersion</key><string>6.0</string>\n");
  out << QStringLiteral("  <key>LSMinimumSystemVersion</key><string>10.13</string>\n");
  out << QStringLiteral("  <key>NSHighResolutionCapable</key><true/>\n");
  if (!iconFileName.isEmpty())
    out << QStringLiteral("  <key>CFBundleIconFile</key><string>%1</string>\n").arg(iconFileName);

  out << QStringLiteral("</dict>\n");
  out << QStringLiteral("</plist>\n");
  file.close();
  return true;
}

/**
 * @brief Writes a .app bundle that launches Serial Studio with snapshot arguments.
 */
bool Misc::ShortcutGenerator::writeMacCommand(const QString& outputPath,
                                              const QString& target,
                                              const QStringList& args,
                                              const QString& title,
                                              const QString& iconPath,
                                              QString& errorOut)
{
  QString bundlePath = outputPath;
  if (!bundlePath.endsWith(QStringLiteral(".app"), Qt::CaseInsensitive))
    bundlePath += QStringLiteral(".app");

  if (!removeAnyPath(bundlePath)) {
    errorOut = tr("Could not replace the existing shortcut at %1.").arg(bundlePath);
    return false;
  }

  const QString contentsDir  = bundlePath + QStringLiteral("/Contents");
  const QString macosDir     = contentsDir + QStringLiteral("/MacOS");
  const QString resourcesDir = contentsDir + QStringLiteral("/Resources");

  QDir root;
  if (!root.mkpath(macosDir) || !root.mkpath(resourcesDir)) {
    errorOut = tr("Could not create the .app bundle directory layout.");
    return false;
  }

  const QString bundleName = QFileInfo(bundlePath).completeBaseName();
  const QString execName   = sanitizeBundleExecName(bundleName);
  const QString execPath   = macosDir + QLatin1Char('/') + execName;

  if (!writeBundleLauncher(execPath, target, args, errorOut))
    return false;

  const QString iconSource   = resolveIconSource(iconPath);
  const QString iconFileName = copyIconIntoBundle(iconSource, resourcesDir);

  const QString plistPath = contentsDir + QStringLiteral("/Info.plist");
  if (!writeInfoPlist(plistPath, bundlePath, bundleName, execName, title, iconFileName, errorOut))
    return false;

  writePkgInfo(contentsDir);

  // Strip quarantine so Gatekeeper does not block the launcher
  stripQuarantine(bundlePath);

  return true;
}

//--------------------------------------------------------------------------------------------------
// Stubs (only the platform writer is compiled in)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Windows writer is unavailable on macOS builds.
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
 * @brief Linux writer is unavailable on macOS builds.
 */
bool Misc::ShortcutGenerator::writeLinuxDesktop(const QString&,
                                                const QString&,
                                                const QStringList&,
                                                const QString&,
                                                const QString&,
                                                QString& errorOut)
{
  errorOut = tr("Linux shortcut writer is not available on this platform.");
  return false;
}

#endif  // BUILD_COMMERCIAL && Q_OS_MAC
