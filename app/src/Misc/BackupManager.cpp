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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/BackupManager.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTimer>

#include "DataModel/ProjectModel.h"

static constexpr int kDebounceMs       = 5000;
static constexpr int kRetentionDefault = 50;

/**
 * @brief Filesystem-safe ISO8601 timestamp (colons replaced with dashes).
 */
static QString fsSafeIsoTimestamp()
{
  return QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddTHH-mm-ss-zzz"));
}

/**
 * @brief Sanitise a label into a filename-safe token.
 */
static QString sanitiseLabel(const QString& label)
{
  QString out;
  out.reserve(label.size());
  for (const auto& ch : label)
    if (ch.isLetterOrNumber() || ch == QLatin1Char('-') || ch == QLatin1Char('_'))
      out.append(ch);
    else if (ch == QLatin1Char(' ') || ch == QLatin1Char('.'))
      out.append(QLatin1Char('-'));

  return out.left(32);
}

/**
 * @brief Construct the singleton. Debounce timer + initial state.
 */
Misc::BackupManager::BackupManager() : m_enabled(true), m_debounceTimer(new QTimer(this))
{
  m_debounceTimer->setSingleShot(true);
  m_debounceTimer->setInterval(kDebounceMs);
  connect(m_debounceTimer, &QTimer::timeout, this, &BackupManager::flushDebounced);
}

/**
 * @brief Singleton accessor.
 */
Misc::BackupManager& Misc::BackupManager::instance()
{
  static BackupManager singleton;
  return singleton;
}

/**
 * @brief Wire ProjectModel signals so snapshots fire automatically.
 */
void Misc::BackupManager::setupExternalConnections()
{
  auto& pm = DataModel::ProjectModel::instance();
  connect(
    &pm, &DataModel::ProjectModel::jsonFileChanged, this, &BackupManager::onProjectFileChanged);
  connect(
    &pm, &DataModel::ProjectModel::modifiedChanged, this, &BackupManager::onProjectModifiedChanged);
}

//--------------------------------------------------------------------------------------------------
// Snapshot / restore / list
//--------------------------------------------------------------------------------------------------

/**
 * @brief Write a snapshot of the current ProjectModel state; returns the absolute path (or the
 * existing path if content is byte-identical to the previous snapshot).
 */
QString Misc::BackupManager::snapshot(const QString& label)
{
  if (!m_enabled)
    return {};

  auto& pm        = DataModel::ProjectModel::instance();
  const auto stem = currentProjectStem();
  const auto dir  = resolveBackupDir(stem);
  if (dir.isEmpty())
    return {};

  const auto serialised = pm.serializeToJson();
  if (serialised.isEmpty())
    return {};

  const auto bytes   = QJsonDocument(serialised).toJson(QJsonDocument::Compact);
  const auto newHash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1);

  // First session snapshot: seed m_lastContentHash from newest on-disk snapshot to skip dup write.
  if (m_lastContentHash.isEmpty()) {
    QDir d(dir);
    const auto entries = d.entryInfoList(QStringList{QStringLiteral("*.ssproj")},
                                         QDir::Files | QDir::NoDotAndDotDot,
                                         QDir::Time | QDir::Reversed);
    if (!entries.isEmpty()) {
      QFile previous(entries.first().absoluteFilePath());
      if (previous.open(QIODevice::ReadOnly)) {
        m_lastContentHash  = QCryptographicHash::hash(previous.readAll(), QCryptographicHash::Sha1);
        m_lastSnapshotPath = entries.first().absoluteFilePath();
        previous.close();
      }
    }
  }

  // Same content as the last snapshot? Return the existing path; do not write a duplicate.
  if (!m_lastContentHash.isEmpty() && newHash == m_lastContentHash)
    return m_lastSnapshotPath;

  QString fileName = fsSafeIsoTimestamp();
  if (!label.isEmpty()) {
    const auto cleanLabel = sanitiseLabel(label);
    if (!cleanLabel.isEmpty())
      fileName += QStringLiteral("__") + cleanLabel;
  }
  fileName += QStringLiteral(".ssproj");

  const auto fullPath = QDir(dir).filePath(fileName);
  QFile out(fullPath);
  if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
    return {};

  if (out.write(bytes) != bytes.size()) {
    out.close();
    QFile::remove(fullPath);
    return {};
  }
  out.close();

  m_lastSnapshotPath = fullPath;
  m_lastContentHash  = newHash;
  enforceRetention(dir);
  Q_EMIT snapshotTaken(fullPath);
  return fullPath;
}

/**
 * @brief Load a snapshot file back into ProjectModel; returns true on success.
 */
bool Misc::BackupManager::restore(const QString& path)
{
  if (path.isEmpty())
    return false;

  QFile in(path);
  if (!in.open(QIODevice::ReadOnly))
    return false;

  const auto bytes = in.readAll();
  in.close();

  QJsonParseError err{};
  const auto doc = QJsonDocument::fromJson(bytes, &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject())
    return false;

  auto& pm = DataModel::ProjectModel::instance();

  // Capture project file association before load: loadFromJsonDocument overwrites m_filePath.
  const auto originalPath = pm.jsonFilePath();

  pm.setSuppressMessageBoxes(true);
  const bool ok = pm.loadFromJsonDocument(doc, originalPath);
  pm.setSuppressMessageBoxes(false);

  // Persist to disk so a crash before the next manual save can't leave stale content unmodified.
  if (ok && !originalPath.isEmpty())
    (void)pm.apiSaveJsonFile(originalPath);

  if (ok)
    Q_EMIT restored(path);

  return ok;
}

/**
 * @brief Enumerate snapshot files for the current project, newest first.
 */
QVariantList Misc::BackupManager::list(int limit) const
{
  QVariantList rows;
  const auto stem = currentProjectStem();
  const auto dir  = resolveBackupDir(stem);
  if (dir.isEmpty())
    return rows;

  QDir d(dir);
  const auto entries = d.entryInfoList(QStringList{QStringLiteral("*.ssproj")},
                                       QDir::Files | QDir::NoDotAndDotDot,
                                       QDir::Time | QDir::Reversed);
  // QDir::Time | QDir::Reversed gives newest first (Reversed flips ascending->descending).

  int taken = 0;
  for (const auto& info : entries) {
    if (limit > 0 && taken >= limit)
      break;

    const auto base = info.completeBaseName();
    QString label;
    const int sep = base.indexOf(QStringLiteral("__"));
    if (sep > 0)
      label = base.mid(sep + 2);

    QVariantMap row;
    row.insert(QStringLiteral("path"), info.absoluteFilePath());
    row.insert(QStringLiteral("timestamp"), info.lastModified().toUTC().toString(Qt::ISODate));
    row.insert(QStringLiteral("sizeBytes"), info.size());
    row.insert(QStringLiteral("label"), label);
    rows.append(row);
    ++taken;
  }

  return rows;
}

/**
 * @brief Returns the absolute backup directory for the current project, creating it if missing.
 */
QString Misc::BackupManager::backupDirectory() const
{
  return resolveBackupDir(currentProjectStem());
}

/**
 * @brief Returns a structural summary of a snapshot file (title, counts, group titles).
 */
QVariantMap Misc::BackupManager::summarize(const QString& path) const
{
  QVariantMap out;
  if (path.isEmpty())
    return out;

  QFile in(path);
  if (!in.open(QIODevice::ReadOnly))
    return out;

  const auto bytes = in.readAll();
  in.close();

  QJsonParseError err{};
  const auto doc = QJsonDocument::fromJson(bytes, &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject())
    return out;

  const auto project = doc.object();
  out.insert(QStringLiteral("title"), project.value(QStringLiteral("title")).toString());

  const auto groups = project.value(QStringLiteral("groups")).toArray();
  out.insert(QStringLiteral("groupCount"), groups.size());

  int datasetCount = 0;
  QStringList groupTitles;
  for (const auto& gv : groups) {
    const auto g  = gv.toObject();
    datasetCount += g.value(QStringLiteral("datasets")).toArray().size();
    groupTitles.append(g.value(QStringLiteral("title")).toString());
  }
  out.insert(QStringLiteral("datasetCount"), datasetCount);
  out.insert(QStringLiteral("groupTitles"), groupTitles);

  const auto sources = project.value(QStringLiteral("sources")).toArray();
  out.insert(QStringLiteral("sourceCount"), sources.isEmpty() ? 1 : sources.size());
  return out;
}

/**
 * @brief Returns a structural summary of the current in-memory ProjectModel.
 */
QVariantMap Misc::BackupManager::currentSummary() const
{
  const auto& pm = DataModel::ProjectModel::instance();
  QVariantMap out;
  out.insert(QStringLiteral("title"), pm.title());
  out.insert(QStringLiteral("groupCount"), pm.groupCount());
  out.insert(QStringLiteral("datasetCount"), pm.datasetCount());
  out.insert(QStringLiteral("sourceCount"), pm.sourceCount());
  out.insert(QStringLiteral("filePath"), pm.jsonFilePath());

  QStringList groupTitles;
  for (const auto& g : pm.groups())
    groupTitles.append(g.title);

  out.insert(QStringLiteral("groupTitles"), groupTitles);
  return out;
}

//--------------------------------------------------------------------------------------------------
// Enable / disable
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when snapshotting is active.
 */
bool Misc::BackupManager::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Toggle the backup pipeline (used by tests / headless tooling).
 */
void Misc::BackupManager::setEnabled(bool on) noexcept
{
  m_enabled = on;
  if (!on)
    m_debounceTimer->stop();
}

//--------------------------------------------------------------------------------------------------
// Slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Take a baseline snapshot every time a different project file is loaded.
 */
void Misc::BackupManager::onProjectFileChanged()
{
  m_debounceTimer->stop();

  // Drop the previous project's last-hash so re-seeding picks up the new backup dir.
  m_lastContentHash.clear();
  m_lastSnapshotPath.clear();
  (void)snapshot(QStringLiteral("load"));
}

/**
 * @brief Debounce a snapshot when the project becomes modified.
 */
void Misc::BackupManager::onProjectModifiedChanged()
{
  if (!m_enabled)
    return;

  if (!DataModel::ProjectModel::instance().modified())
    return;

  m_debounceTimer->start();
}

/**
 * @brief Debounce timer expired -- write a snapshot now.
 */
void Misc::BackupManager::flushDebounced()
{
  (void)snapshot(QStringLiteral("auto"));
}

//--------------------------------------------------------------------------------------------------
// Internals
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stem name used as the backup subdirectory; "untitled" when no project file is open.
 */
QString Misc::BackupManager::currentProjectStem() const
{
  const auto path = DataModel::ProjectModel::instance().jsonFilePath();
  if (path.isEmpty())
    return QStringLiteral("untitled");

  const auto stem = QFileInfo(path).baseName();
  return stem.isEmpty() ? QStringLiteral("untitled") : stem;
}

/**
 * @brief Compute (and create if needed) the backup directory for one project stem.
 */
QString Misc::BackupManager::resolveBackupDir(const QString& stem) const
{
  const auto root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (root.isEmpty())
    return {};

  QDir d(root);
  if (!d.exists(QStringLiteral("backups")) && !d.mkpath(QStringLiteral("backups")))
    return {};

  const auto sub = QDir::cleanPath(d.absoluteFilePath(QStringLiteral("backups/") + stem));
  QDir subDir(sub);
  if (!subDir.exists() && !subDir.mkpath(QStringLiteral(".")))
    return {};

  return sub;
}

/**
 * @brief Evict the oldest snapshots once the retention cap is exceeded.
 */
void Misc::BackupManager::enforceRetention(const QString& dir)
{
  QDir d(dir);
  const auto entries = d.entryInfoList(QStringList{QStringLiteral("*.ssproj")},
                                       QDir::Files | QDir::NoDotAndDotDot,
                                       QDir::Time | QDir::Reversed);

  for (int i = kRetentionDefault; i < entries.size(); ++i)
    QFile::remove(entries.at(i).absoluteFilePath());
}
