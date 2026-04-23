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
#  include <QSettings>
#  include <QSqlDatabase>
#  include <QVariantList>

namespace Sessions {

/**
 * @brief Owns the session database file and backs the Database Explorer UI.
 */
class DatabaseManager : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
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
  // clang-format on

signals:
  void openChanged();
  void sessionsChanged();
  void selectedSessionChanged();
  void tagsChanged();
  void csvExportBusyChanged();
  void csvExportFinished(const QString& outputPath, bool success);
  void pdfExportBusyChanged();
  void pdfExportProgressChanged();
  void pdfExportFinished(const QString& outputPath, bool success);
  void reportLogoPicked(const QString& path);
  void projectMetadataRestored();

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
  [[nodiscard]] QString filePath() const;
  [[nodiscard]] QString fileName() const;
  [[nodiscard]] int sessionCount() const;
  [[nodiscard]] int selectedSessionId() const;
  [[nodiscard]] bool csvExportBusy() const;
  [[nodiscard]] bool pdfExportBusy() const;
  [[nodiscard]] QString pdfExportStatus() const;
  [[nodiscard]] double pdfExportProgress() const;
  [[nodiscard]] QVariantList sessionList() const;
  [[nodiscard]] QVariantList tagList() const;
  [[nodiscard]] QVariantList selectedSessionTags() const;
  [[nodiscard]] QString selectedSessionNotes() const;

  Q_INVOKABLE [[nodiscard]] QVariantList tagsForSession(int sessionId) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap sessionMetadata(int sessionId) const;
  Q_INVOKABLE [[nodiscard]] QString projectJsonFromDb() const;
  Q_INVOKABLE [[nodiscard]] QString sessionProjectJson(int sessionId) const;
  Q_INVOKABLE [[nodiscard]] static QString canonicalDbPath(const QString& projectTitle);

  static void createSchema(QSqlQuery& q);

public slots:
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

private:
  void ensureSchema();
  void loadTagList();

private:
  QSqlDatabase m_db;
  QString m_filePath;
  QSettings m_settings;
  QString m_connectionName;

  int m_selectedSessionId;
  bool m_csvExportBusy;
  bool m_pdfExportBusy;
  double m_pdfExportProgress;
  QString m_pdfExportStatus;

  QVariantList m_sessionList;
  QVariantList m_tagList;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
