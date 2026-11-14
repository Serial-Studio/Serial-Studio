/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Core/Deploy/ResourceExtractor.h"

#  include <QDateTime>
#  include <QDebug>
#  include <QDir>
#  include <QDirIterator>
#  include <QFile>
#  include <QFileInfo>
#  include <QLockFile>
#  include <QTemporaryDir>

#  include "Core/Deploy/PackageFormat.h"
#  include "Core/SSAssert.h"

#  ifdef Q_OS_UNIX
#    include <unistd.h>
#  endif

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr QLatin1StringView kFolderPrefix{"SerialStudio-package-"};
static constexpr QLatin1StringView kLockName{".session.lock"};
static constexpr int kMaxSweptFolders  = 64;
static constexpr int kMaxListedFolders = 4096;

static const QStringList kDeviceNames = {
  QStringLiteral("CON"),  QStringLiteral("PRN"),  QStringLiteral("AUX"),  QStringLiteral("NUL"),
  QStringLiteral("COM1"), QStringLiteral("COM2"), QStringLiteral("COM3"), QStringLiteral("COM4"),
  QStringLiteral("COM5"), QStringLiteral("COM6"), QStringLiteral("COM7"), QStringLiteral("COM8"),
  QStringLiteral("COM9"), QStringLiteral("LPT1"), QStringLiteral("LPT2"), QStringLiteral("LPT3"),
  QStringLiteral("LPT4"), QStringLiteral("LPT5"), QStringLiteral("LPT6"), QStringLiteral("LPT7"),
  QStringLiteral("LPT8"), QStringLiteral("LPT9")};

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether every part of @p entryPath is a plain file name on Windows too: no reserved
 *        device name (which would send the bytes to a port, not a file) and no trailing dot or
 *        space. A package is refused the same way on every platform.
 */
[[nodiscard]] static bool isPortableName(const QString& entryPath)
{
  const auto parts = entryPath.split(QLatin1Char('/'));
  for (const auto& part : parts) {
    const auto stem = part.section(QLatin1Char('.'), 0, 0).trimmed().toUpper();
    if (kDeviceNames.contains(stem) || part.endsWith(QLatin1Char('.'))
        || part.endsWith(QLatin1Char(' ')))
      return false;
  }

  return true;
}

/**
 * @brief Whether the sweep may delete @p folder: a real directory, not a link, that this user
 *        owns. In a shared temp folder another user can plant a matching name.
 */
[[nodiscard]] static bool isOwnFolder(const QFileInfo& folder)
{
  if (folder.isSymLink() || !folder.isDir())
    return false;

#  ifdef Q_OS_UNIX
  return folder.ownerId() == static_cast<uint>(geteuid());
#  else
  return true;
#  endif
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an extractor that owns no folder yet.
 */
Deploy::ResourceExtractor::ResourceExtractor() : m_lock(nullptr) {}

/**
 * @brief Removes the folder, so a session that ends normally leaves nothing behind.
 */
Deploy::ResourceExtractor::~ResourceExtractor()
{
  remove();
}

//--------------------------------------------------------------------------------------------------
// Path rules
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a carried file is a resource: under resources/ with a name after the prefix.
 */
bool Deploy::ResourceExtractor::isResourceEntry(const QString& entryPath)
{
  return entryPath.size() > kResourceDir.size() && entryPath.startsWith(kResourceDir);
}

/**
 * @brief The files a resource choice stands for: the file itself, or every regular file under a
 *        folder, sorted. Symbolic links are skipped, and the list stops one past the cap so the
 *        caller can tell a folder that is too large from one that fits.
 */
QStringList Deploy::ResourceExtractor::filesUnder(const QString& path)
{
  const QFileInfo info(path);
  if (info.isSymLink() || !info.exists())
    return {};

  if (info.isFile())
    return {info.canonicalFilePath()};

  QStringList files;
  QDirIterator it(info.canonicalFilePath(),
                  QDir::Files | QDir::NoSymLinks | QDir::Hidden,
                  QDirIterator::Subdirectories);
  while (it.hasNext() && files.size() <= kMaxResourceFiles)
    files.append(QFileInfo(it.next()).canonicalFilePath());

  files.sort();
  return files;
}

/**
 * @brief The path a resource is carried under: resources/ plus its path relative to the project
 *        folder. Empty for a file outside that folder or with a name the codec refuses.
 */
QString Deploy::ResourceExtractor::entryPathFor(const QString& projectFolder, const QString& file)
{
  const auto folder = QFileInfo(projectFolder).canonicalFilePath();
  const auto source = QFileInfo(file).canonicalFilePath();
  if (folder.isEmpty() || source.isEmpty() || !source.startsWith(folder + QLatin1Char('/')))
    return {};

  const QString carried = kResourceDir + source.mid(folder.size() + 1);
  if (!isSafeEntryPath(carried) || carried.endsWith(QLatin1Char('/') + kLockName))
    return {};

  return carried;
}

//--------------------------------------------------------------------------------------------------
// Leftovers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Removes the folders that crashed sessions left under @p baseDir. A folder is a leftover
 *        when its lock can be taken, which a live session's cannot; one younger than
 *        @p minimumAgeSeconds is kept, because its session may not have locked it yet.
 *        Only this user's own folders are touched, and ownership is checked again before removal.
 */
void Deploy::ResourceExtractor::sweepStale(const QString& baseDir, const int minimumAgeSeconds)
{
  SS_ASSERT(!baseDir.isEmpty(), return);
  SS_ASSERT(minimumAgeSeconds >= 0, return);

  const QDir base(baseDir);
  const auto now     = QDateTime::currentDateTimeUtc();
  const auto folders = base.entryInfoList({QString(kFolderPrefix) + QLatin1Char('*')},
                                          QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

  int swept        = 0;
  const auto count = qMin<qsizetype>(folders.size(), kMaxListedFolders);
  for (qsizetype i = 0; i < count && swept < kMaxSweptFolders; ++i) {
    const auto& folder = folders.at(i);
    if (!isOwnFolder(folder) || folder.lastModified().secsTo(now) < minimumAgeSeconds)
      continue;

    ++swept;

    QLockFile probe(folder.absoluteFilePath() + QLatin1Char('/') + kLockName);
    probe.setStaleLockTime(0);
    if (!probe.tryLock(0))
      continue;

    probe.unlock();
    if (!isOwnFolder(QFileInfo(folder.absoluteFilePath())))
      continue;

    if (!QDir(folder.absoluteFilePath()).removeRecursively())
      qWarning() << "[ResourceExtractor] Could not remove leftover" << folder.absoluteFilePath();
  }
}

//--------------------------------------------------------------------------------------------------
// Extraction
//--------------------------------------------------------------------------------------------------

/**
 * @brief The session's folder; empty until extract() succeeds and after remove().
 */
const QString& Deploy::ResourceExtractor::folder() const noexcept
{
  return m_folder;
}

/**
 * @brief Creates a folder only this user can read under @p baseDir, locks it and writes every
 *        resource entry of @p files into it. Any failure removes what was written and reports
 *        the reason in @p error; other carried files are never written.
 */
bool Deploy::ResourceExtractor::extract(const QList<FileEntry>& files,
                                        const QString& baseDir,
                                        QString& error)
{
  SS_ASSERT(m_folder.isEmpty(), return false);
  SS_ASSERT(static_cast<quint64>(files.size()) <= kMaxFiles, return false);

  QTemporaryDir created(baseDir + QLatin1Char('/') + kFolderPrefix + QStringLiteral("XXXXXX"));
  created.setAutoRemove(false);
  if (!created.isValid()) {
    error = created.errorString();
    return false;
  }

  m_folder = QFileInfo(created.path()).canonicalFilePath();
  m_lock   = std::make_unique<QLockFile>(m_folder + QLatin1Char('/') + kLockName);
  m_lock->setStaleLockTime(0);
  if (m_folder.isEmpty() || !m_lock->tryLock(0)) {
    error = QStringLiteral("the resources folder could not be locked");
    remove();
    return false;
  }

  for (const auto& entry : files) {
    if (!isResourceEntry(entry.path) || writeEntry(entry, error))
      continue;

    remove();
    return false;
  }

  return true;
}

/**
 * @brief Writes one resource at its relative path. The cleaned target must still sit inside the
 *        folder, so an entry name can never place a file anywhere else, and a write the disk did
 *        not take is a failure, never a truncated helper.
 */
bool Deploy::ResourceExtractor::writeEntry(const FileEntry& entry, QString& error) const
{
  SS_ASSERT(!m_folder.isEmpty(), return false);
  SS_ASSERT(isResourceEntry(entry.path), return false);

  const QString root   = m_folder + QLatin1Char('/');
  const QString target = QDir::cleanPath(root + entry.path.mid(kResourceDir.size()));
  if (!isSafeEntryPath(entry.path) || !target.startsWith(root) || target == root + kLockName
      || !isPortableName(entry.path)) {
    error = QStringLiteral("unsafe resource path: %1").arg(entry.path);
    return false;
  }

  if (!QDir().mkpath(QFileInfo(target).absolutePath())) {
    error = QStringLiteral("%1: its folder could not be created").arg(entry.path);
    return false;
  }

  QFile file(target);
  if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly)
      || file.write(entry.data) != entry.data.size() || !file.flush()) {
    error = QStringLiteral("%1: %2").arg(entry.path, file.errorString());
    return false;
  }

  file.close();
  (void)file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
  return true;
}

/**
 * @brief Releases the lock and deletes the folder with everything in it. A file still held open
 *        by a helper that has not exited stays for the next session's sweep.
 */
void Deploy::ResourceExtractor::remove()
{
  if (m_lock)
    m_lock->unlock();

  m_lock.reset();
  if (m_folder.isEmpty())
    return;

  if (!QDir(m_folder).removeRecursively())
    qWarning() << "[ResourceExtractor] Could not remove" << m_folder;

  m_folder.clear();
}

#endif  // BUILD_COMMERCIAL
