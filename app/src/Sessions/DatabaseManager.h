/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QHash>
#  include <QObject>
#  include <QSettings>
#  include <QSqlQuery>
#  include <QVariantList>

#  include "Sessions/DatabaseWorker.h"
#  include "Sessions/HtmlReport.h"

class QThread;

namespace Sessions {

/**
 * @brief Owns the session database file and backs the Database Explorer UI.
 *
 * The actual SQLite connection is owned by a @c DatabaseWorker that runs on a
 * dedicated thread. This object keeps the QML-facing caches and dispatches
 * mutating operations to the worker via queued slots.
 */
class DatabaseManager : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(bool busy
             READ busy
             NOTIFY busyChanged)
  Q_PROPERTY(QString filePath
             READ filePath
             NOTIFY openChanged)
  Q_PROPERTY(QString fileName
             READ fileName
             NOTIFY openChanged)
  Q_PROPERTY(int sessionCount
             READ sessionCount
             NOTIFY sessionsChanged)
  Q_PROPERTY(int selectedSessionId
             READ selectedSessionId
             WRITE setSelectedSessionId
             NOTIFY selectedSessionChanged)
  Q_PROPERTY(QVariantList sessionList
             READ sessionList
             NOTIFY sessionsChanged)
  Q_PROPERTY(QVariantList tagList
             READ tagList
             NOTIFY tagsChanged)
  Q_PROPERTY(QVariantList selectedSessionTags
             READ selectedSessionTags
             NOTIFY selectedSessionChanged)
  Q_PROPERTY(QString selectedSessionNotes
             READ selectedSessionNotes
             WRITE setSelectedSessionNotes
             NOTIFY selectedSessionChanged)
  Q_PROPERTY(bool csvExportBusy
             READ csvExportBusy
             NOTIFY csvExportBusyChanged)
  Q_PROPERTY(bool pdfExportBusy
             READ pdfExportBusy
             NOTIFY pdfExportBusyChanged)
  Q_PROPERTY(QString pdfExportStatus
             READ pdfExportStatus
             NOTIFY pdfExportProgressChanged)
  Q_PROPERTY(double pdfExportProgress
             READ pdfExportProgress
             NOTIFY pdfExportProgressChanged)
  Q_PROPERTY(double csvExportProgress
             READ csvExportProgress
             NOTIFY csvExportProgressChanged)
  Q_PROPERTY(bool locked
             READ locked
             NOTIFY lockedChanged)
  // clang-format on

signals:
  void openChanged();
  void busyChanged();
  void sessionsChanged();
  void selectedSessionChanged();
  void tagsChanged();
  void csvExportBusyChanged();
  void csvExportProgressChanged();
  void csvExportFinished(const QString& outputPath, bool success);
  void pdfExportBusyChanged();
  void pdfExportProgressChanged();
  void pdfExportFinished(const QString& outputPath, bool success);
  void reportLogoPicked(const QString& path);
  void projectMetadataRestored();
  void lockedChanged();

private:
  explicit DatabaseManager();
  DatabaseManager(DatabaseManager&&)                 = delete;
  DatabaseManager(const DatabaseManager&)            = delete;
  DatabaseManager& operator=(DatabaseManager&&)      = delete;
  DatabaseManager& operator=(const DatabaseManager&) = delete;
  ~DatabaseManager();

public:
  [[nodiscard]] static DatabaseManager& instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool busy() const;
  [[nodiscard]] QString filePath() const;
  [[nodiscard]] QString fileName() const;
  [[nodiscard]] int sessionCount() const;
  [[nodiscard]] int selectedSessionId() const;
  [[nodiscard]] bool locked() const;
  [[nodiscard]] bool csvExportBusy() const;
  [[nodiscard]] bool pdfExportBusy() const;
  [[nodiscard]] QString pdfExportStatus() const;
  [[nodiscard]] double pdfExportProgress() const;
  [[nodiscard]] double csvExportProgress() const;
  [[nodiscard]] QVariantList sessionList() const;
  [[nodiscard]] QVariantList tagList() const;
  [[nodiscard]] QVariantList selectedSessionTags() const;
  [[nodiscard]] QString selectedSessionNotes() const;

  Q_INVOKABLE [[nodiscard]] QVariantList tagsForSession(int sessionId) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap sessionMetadata(int sessionId) const;
  Q_INVOKABLE [[nodiscard]] static QString canonicalDbPath(const QString& projectTitle);

  static void createSchema(QSqlQuery& q);

  void shutdown();

public slots:
  void lockDatabase();
  void unlockDatabase();

  void openDatabase();
  void openDatabase(const QString& filePath);
  void closeDatabase(bool clearSavedPath = true);
  void setupExternalConnections();

  void setSelectedSessionId(int sessionId);
  void setSelectedSessionNotes(const QString& notes);
  void deleteSession(int sessionId);
  void confirmDeleteSession(int sessionId);
  void replaySelectedSession();

  void addTag(const QString& label);
  void deleteTag(int tagId);
  void renameTag(int tagId, const QString& newLabel);
  void assignTagToSession(int sessionId, int tagId);
  void removeTagFromSession(int sessionId, int tagId);

  void exportSessionToCsv(int sessionId);
  void exportSessionToPdf(int sessionId, const QVariantMap& options);
  void pickReportLogo();

  void storeProjectMetadata();
  void restoreProjectFromDb();
  void restoreLastDatabase();

public slots:
  void refreshSessionList();

private slots:
  void onWorkerOpened(const QString& filePath,
                      const QVariantList& sessionList,
                      const QVariantList& tagList,
                      bool locked,
                      const QString& passwordHash);
  void onWorkerOpenFailed(const QString& filePath, const QString& error);
  void onWorkerClosed();
  void onWorkerSessionListRefreshed(const QVariantList& sessionList);
  void onWorkerTagListRefreshed(const QVariantList& tagList);
  void onWorkerLockStateChanged(bool locked, const QString& passwordHash);
  void onWorkerNotesUpdated(int sessionId, const QString& notes);
  void onWorkerMutationFinished(quint64 token, bool ok, const QString& error);
  void onWorkerCsvProgress(double percent);
  void onWorkerCsvFinished(const QString& outputPath, bool ok, const QString& error);
  void onWorkerReportDataReady(const Sessions::ReportPayloadPtr& payload);
  void onWorkerGlobalProjectJsonReady(const QString& json);

private:
  void initWorker();
  [[nodiscard]] quint64 nextToken();
  void setBusy(bool busy);
  void renderReportFromPayload(const ReportPayloadPtr& payload);
  void runRestoreProjectFromJson(const QString& json);

private:
  QThread* m_thread;
  DatabaseWorker* m_worker;

  bool m_open;
  QString m_filePath;
  QSettings m_settings;

  int m_selectedSessionId;
  bool m_locked;
  QString m_passwordHash;

  bool m_csvExportBusy;
  double m_csvExportProgress;
  bool m_pdfExportBusy;
  double m_pdfExportProgress;
  QString m_pdfExportStatus;

  QVariantList m_sessionList;
  QVariantList m_tagList;

  // Pending PDF render context -- paired with worker reply by sessionId
  HtmlReportOptions m_pendingPdfOpts;
  int m_pendingPdfSessionId;
  bool m_pendingPdfActive;

  // CSV export tracking
  QString m_pendingCsvPath;

  // Outstanding mutation tokens awaiting worker confirmation
  quint64 m_nextToken;
  int m_outstandingMutations;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
