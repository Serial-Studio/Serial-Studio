/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QObject>
#  include <QSqlDatabase>
#  include <QString>
#  include <QVariantList>
#  include <QVariantMap>
#  include <atomic>
#  include <memory>
#  include <vector>

#  include "Sessions/ReportData.h"

namespace Sessions {

/**
 * @brief Bundle of SQL-fetched data the manager needs to render a session report.
 */
struct ReportPayload {
  int sessionId;
  bool ok;
  QString error;
  ReportData data;
  std::vector<DatasetSeries> series;
};

using ReportPayloadPtr = std::shared_ptr<ReportPayload>;

/**
 * @brief Worker that owns the session-database SQL connection on a dedicated thread.
 */
class DatabaseWorker : public QObject {
  Q_OBJECT

signals:
  void opened(const QString& filePath, const QVariantList& sessionList,
              const QVariantList& tagList, bool locked, const QString& passwordHash);
  void openFailed(const QString& filePath, const QString& error);
  void closed();
  void sessionListRefreshed(const QVariantList& sessionList);
  void tagListRefreshed(const QVariantList& tagList);
  void lockStateChanged(bool locked, const QString& passwordHash);
  void notesUpdated(int sessionId, const QString& notes);
  void mutationFinished(quint64 token, bool ok, const QString& error);
  void globalProjectJsonReady(const QString& json);
  void csvExportProgress(double percent);
  void csvExportFinished(const QString& outputPath, bool ok, const QString& error);
  void reportDataReady(const Sessions::ReportPayloadPtr& payload);

public:
  explicit DatabaseWorker(QObject* parent = nullptr);
  ~DatabaseWorker() override;

  DatabaseWorker(DatabaseWorker&&)                 = delete;
  DatabaseWorker(const DatabaseWorker&)            = delete;
  DatabaseWorker& operator=(DatabaseWorker&&)      = delete;
  DatabaseWorker& operator=(const DatabaseWorker&) = delete;

  void requestCancel();

public slots:
  void openDatabase(const QString& filePath);
  void closeDatabase();
  void refreshAll();

  void deleteSession(int sessionId, quint64 token);
  void setSessionNotes(int sessionId, const QString& notes, quint64 token);

  void addTag(const QString& label, quint64 token);
  void deleteTag(int tagId, quint64 token);
  void renameTag(int tagId, const QString& newLabel, quint64 token);
  void assignTag(int sessionId, int tagId, quint64 token);
  void unassignTag(int sessionId, int tagId, quint64 token);

  void persistLock(const QString& passwordHash, quint64 token);

  void storeProjectMetadata(const QString& projectJson, const QString& projectTitle, quint64 token);
  void fetchGlobalProjectJson();

  void runCsvExport(int sessionId, const QString& outputPath);
  void runReportDataLoad(int sessionId, bool includeCharts, int chartMaxSamples);

private:
  void refreshSessionListInternal();
  void refreshTagListInternal();
  void refreshLockStateInternal();
  void ensureSchemaInternal();

  [[nodiscard]] QVariantList tagsForSession(int sessionId);
  [[nodiscard]] static QString formatDateForDisplay(const QString& isoDate);

private:
  QSqlDatabase m_db;
  QString m_filePath;
  QString m_connectionName;
  QString m_passwordHash;
  bool m_locked;

  QVariantList m_sessionList;
  QVariantList m_tagList;

  std::atomic<bool> m_cancelRequested;
};

}  // namespace Sessions

Q_DECLARE_METATYPE(Sessions::ReportPayloadPtr)

#endif  // BUILD_COMMERCIAL
