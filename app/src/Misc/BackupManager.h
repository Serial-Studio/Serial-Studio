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

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class QTimer;

namespace Misc {

/**
 * @brief Rolling project snapshot manager. Writes .ssproj copies wrapped with a versioned
 *        `_backupMeta` object so future builds can refuse incompatible snapshots.
 */
class BackupManager : public QObject {
  Q_OBJECT

signals:
  void snapshotTaken(const QString& path);
  void restored(const QString& path);

private:
  BackupManager();
  BackupManager(BackupManager&&)                 = delete;
  BackupManager(const BackupManager&)            = delete;
  BackupManager& operator=(BackupManager&&)      = delete;
  BackupManager& operator=(const BackupManager&) = delete;

public:
  [[nodiscard]] static BackupManager& instance();

  void setupExternalConnections();

  Q_INVOKABLE [[nodiscard]] QString snapshot(const QString& label = QString());
  Q_INVOKABLE [[nodiscard]] bool restore(const QString& path);
  Q_INVOKABLE [[nodiscard]] QVariantList list(int limit = 50) const;
  Q_INVOKABLE [[nodiscard]] QString backupDirectory() const;
  Q_INVOKABLE [[nodiscard]] QVariantMap summarize(const QString& path) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap currentSummary() const;

  [[nodiscard]] bool enabled() const noexcept;
  void setEnabled(bool on) noexcept;

public slots:
  void onProjectFileChanged();
  void onProjectModifiedChanged();
  void onProjectContentTouched();

private slots:
  void flushDebounced();

private:
  [[nodiscard]] QString currentProjectStem() const;
  [[nodiscard]] QString resolveBackupDir(const QString& stem) const;
  void enforceRetention(const QString& dir);
  void seedDedupFromNewest(const QString& dir);

  bool m_enabled;
  QString m_lastSnapshotPath;
  QByteArray m_lastContentHash;
  QTimer* m_debounceTimer;
};

}  // namespace Misc
