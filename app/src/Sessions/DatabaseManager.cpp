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

#ifdef BUILD_COMMERCIAL

#  include "Sessions/DatabaseManager.h"

#  include <QCoreApplication>
#  include <QCryptographicHash>
#  include <QDateTime>
#  include <QDir>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QInputDialog>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QJsonParseError>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QTimer>

#  include "AppState.h"
#  include "DataModel/ProjectModel.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "SerialStudio.h"
#  include "Sessions/Export.h"
#  include "Sessions/HtmlReport.h"
#  include "Sessions/Player.h"
#  include "Sessions/ReportData.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats an ISO 8601 date string into a user-friendly display string.
 */
static QString formatDateForDisplay(const QString& isoDate)
{
  if (isoDate.isEmpty())
    return {};

  const auto dt = QDateTime::fromString(isoDate, Qt::ISODate);
  if (!dt.isValid())
    return isoDate;

  return dt.toString(QStringLiteral("MMM d, yyyy — h:mm AP"));
}

/**
 * @brief Scrubs a project title for use as a folder/file name component.
 */
static QString sanitiseTitleForPath(const QString& title)
{
  QString safe = title;
  safe.remove(QChar('/'));
  safe.remove(QChar('\\'));
  safe.remove(QChar(':'));
  safe.remove(QChar('*'));
  safe.remove(QChar('?'));
  safe.remove(QChar('"'));
  safe.remove(QChar('<'));
  safe.remove(QChar('>'));
  safe.remove(QChar('|'));
  safe.remove(QChar('\0'));
  safe.remove(QStringLiteral(".."));
  safe = safe.simplified();
  if (safe.isEmpty())
    safe = QStringLiteral("Untitled");

  return safe;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes member variables.
 */
Sessions::DatabaseManager::DatabaseManager()
  : m_selectedSessionId(-1)
  , m_csvExportBusy(false)
  , m_pdfExportBusy(false)
  , m_locked(false)
  , m_pdfExportProgress(0.0)
{}

/**
 * @brief Closes the database before the singleton destructs.
 */
Sessions::DatabaseManager::~DatabaseManager()
{
  closeDatabase(false);
}

/**
 * @brief Returns the single application-wide DatabaseManager.
 */
Sessions::DatabaseManager& Sessions::DatabaseManager::instance()
{
  static DatabaseManager singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// External connections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires cross-singleton signals. Called after all singletons are alive.
 */
void Sessions::DatabaseManager::setupExternalConnections()
{
  connect(&Sessions::Export::instance(),
          &Sessions::Export::openChanged,
          this,
          &Sessions::DatabaseManager::refreshSessionList);
}

//--------------------------------------------------------------------------------------------------
// Read-only accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true when a database file is open and queryable.
 */
bool Sessions::DatabaseManager::isOpen() const
{
  return m_db.isOpen();
}

/**
 * @brief Returns the absolute file path of the open .db, or empty.
 */
QString Sessions::DatabaseManager::filePath() const
{
  return m_filePath;
}

/**
 * @brief Returns the base filename of the open database.
 */
QString Sessions::DatabaseManager::fileName() const
{
  return m_filePath.isEmpty() ? QString() : QFileInfo(m_filePath).fileName();
}

/**
 * @brief Returns the number of sessions in the open database.
 */
int Sessions::DatabaseManager::sessionCount() const
{
  return m_sessionList.size();
}

/**
 * @brief Returns the currently selected session id, or -1.
 */
int Sessions::DatabaseManager::selectedSessionId() const
{
  return m_selectedSessionId;
}

/**
 * @brief Returns @c true while a CSV export worker is running.
 */
bool Sessions::DatabaseManager::csvExportBusy() const
{
  return m_csvExportBusy;
}

/**
 * @brief Returns @c true while a PDF report is being rendered.
 */
bool Sessions::DatabaseManager::pdfExportBusy() const
{
  return m_pdfExportBusy;
}

/**
 * @brief Returns the user-facing status label for the active export.
 */
QString Sessions::DatabaseManager::pdfExportStatus() const
{
  return m_pdfExportStatus;
}

/**
 * @brief Returns the active export's progress as a fraction in [0, 1].
 */
double Sessions::DatabaseManager::pdfExportProgress() const
{
  return m_pdfExportProgress;
}

/**
 * @brief Returns @c true while the session file is password-protected against deletion.
 */
bool Sessions::DatabaseManager::locked() const
{
  return m_locked;
}

/**
 * @brief Returns the cached session list as a QVariantList.
 */
QVariantList Sessions::DatabaseManager::sessionList() const
{
  return m_sessionList;
}

/**
 * @brief Returns the cached list of all tags defined in the database.
 */
QVariantList Sessions::DatabaseManager::tagList() const
{
  return m_tagList;
}

/**
 * @brief Returns the tags assigned to the currently selected session.
 */
QVariantList Sessions::DatabaseManager::selectedSessionTags() const
{
  if (m_selectedSessionId < 0)
    return {};

  return tagsForSession(m_selectedSessionId);
}

/**
 * @brief Returns the notes string for the currently selected session.
 */
QString Sessions::DatabaseManager::selectedSessionNotes() const
{
  if (m_selectedSessionId < 0 || !isOpen())
    return {};

  QSqlQuery q(m_db);
  q.prepare("SELECT notes FROM sessions WHERE session_id = ?");
  q.bindValue(0, m_selectedSessionId);
  if (q.exec() && q.next())
    return q.value(0).toString();

  return {};
}

//--------------------------------------------------------------------------------------------------
// Q_INVOKABLE queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the tags for a specific session as a QVariantList.
 */
QVariantList Sessions::DatabaseManager::tagsForSession(int sessionId) const
{
  QVariantList result;
  if (!isOpen())
    return result;

  QSqlQuery q(m_db);
  q.prepare("SELECT t.tag_id, t.label FROM tags t "
            "JOIN session_tags st ON st.tag_id = t.tag_id "
            "WHERE st.session_id = ? ORDER BY t.label");
  q.bindValue(0, sessionId);
  if (!q.exec())
    return result;

  while (q.next()) {
    QVariantMap tag;
    tag["tag_id"] = q.value(0).toInt();
    tag["label"]  = q.value(1).toString();
    result.append(tag);
  }

  return result;
}

/**
 * @brief Returns full metadata for a session as a QVariantMap.
 */
QVariantMap Sessions::DatabaseManager::sessionMetadata(int sessionId) const
{
  QVariantMap result;
  if (!isOpen())
    return result;

  QSqlQuery q(m_db);
  q.prepare(
    "SELECT s.project_title, s.started_at, s.ended_at, s.notes, "
    "       (SELECT COUNT(DISTINCT timestamp_ns) FROM readings WHERE session_id = s.session_id) "
    "FROM sessions s WHERE s.session_id = ?");
  q.bindValue(0, sessionId);
  if (!q.exec() || !q.next())
    return result;

  result["session_id"]    = sessionId;
  result["project_title"] = q.value(0).toString();
  result["started_at"]    = formatDateForDisplay(q.value(1).toString());
  result["ended_at"]      = formatDateForDisplay(q.value(2).toString());
  result["notes"]         = q.value(3).toString();
  result["frame_count"]   = q.value(4).toInt();

  return result;
}

/**
 * @brief Returns the latest project JSON stored in project_metadata, or empty.
 */
QString Sessions::DatabaseManager::projectJsonFromDb() const
{
  if (!isOpen())
    return {};

  QSqlQuery q(m_db);
  q.prepare("SELECT value FROM project_metadata WHERE key = 'project_json'");
  if (q.exec() && q.next())
    return q.value(0).toString();

  return {};
}

/**
 * @brief Returns the project JSON embedded in a specific session row.
 */
QString Sessions::DatabaseManager::sessionProjectJson(int sessionId) const
{
  if (!isOpen())
    return {};

  // Try the per-session snapshot first
  QSqlQuery q(m_db);
  q.prepare("SELECT project_json FROM sessions WHERE session_id = ?");
  q.bindValue(0, sessionId);
  if (q.exec() && q.next()) {
    const auto json = q.value(0).toString();
    if (!json.isEmpty())
      return json;
  }

  // Fall back to the global metadata table
  return projectJsonFromDb();
}

/**
 * @brief Opens a native QFileDialog to pick a logo for the report.
 */
void Sessions::DatabaseManager::pickReportLogo()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select logo image"), QString(), tr("Images (*.png *.jpg *.jpeg *.svg)"));
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    dialog->deleteLater();
    if (!path.isEmpty())
      Q_EMIT reportLogoPicked(path);
  });

  dialog->open();
}

/**
 * @brief Returns the canonical .db path for a project title. Export uses this
 * to decide whether to create a new file or append to an existing one.
 */
QString Sessions::DatabaseManager::canonicalDbPath(const QString& projectTitle)
{
  const QString safeTitle = sanitiseTitleForPath(projectTitle);
  const auto subdir       = Misc::WorkspaceManager::instance().path("Session Databases");
  return QStringLiteral("%1/%2/%2.db").arg(subdir, safeTitle);
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to pick a .db file from the SQLite workspace folder.
 */
void Sessions::DatabaseManager::openDatabase()
{
  auto* dialog = new QFileDialog(nullptr,
                                 tr("Open Session File"),
                                 Misc::WorkspaceManager::instance().path("Session Databases"),
                                 tr("Session files (*.db)"));

  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      openDatabase(path);

    dialog->deleteLater();
  });
  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);
  dialog->open();
}

/**
 * @brief Opens the given .db file, migrates the schema if needed, and loads
 * the session and tag lists.
 */
void Sessions::DatabaseManager::openDatabase(const QString& filePath)
{
  if (filePath.isEmpty())
    return;

  // Wait cursor for the synchronous SQL setup that follows
  QGuiApplication::setOverrideCursor(Qt::WaitCursor);

  // Close without clearing the saved path
  closeDatabase(false);

  // Open with a unique connection name to coexist with Export/Player
  m_filePath       = filePath;
  m_connectionName = QStringLiteral("ss_dbmgr_%1").arg(QDateTime::currentMSecsSinceEpoch());
  m_db             = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(filePath);
  if (!m_db.open()) {
    QGuiApplication::restoreOverrideCursor();
    Misc::Utilities::showMessageBox(
      tr("Cannot open session file"), m_db.lastError().text(), QMessageBox::Critical);
    closeDatabase(true);
    return;
  }

  // WAL: writer and reader coexist without locks
  QSqlQuery pragma(m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA busy_timeout=5000");

  // Ensure the full schema exists before querying
  ensureSchema();

  // Populate caches and persist the path for next launch
  refreshSessionList();
  loadTagList();
  loadLockState();
  m_settings.setValue("Sessions/LastDatabase", m_filePath);

  QGuiApplication::restoreOverrideCursor();
  Q_EMIT openChanged();
}

/**
 * @brief Closes the database and resets all cached state.
 * @param clearSavedPath True to forget the path (user close), false to keep it (app quit).
 */
void Sessions::DatabaseManager::closeDatabase(bool clearSavedPath)
{
  m_selectedSessionId = -1;
  m_sessionList.clear();
  m_tagList.clear();
  m_passwordHash.clear();

  const bool wasLocked = m_locked;
  m_locked             = false;

  if (m_db.isOpen())
    m_db.close();

  const auto conn = m_connectionName;
  m_db            = QSqlDatabase();
  if (!conn.isEmpty())
    QSqlDatabase::removeDatabase(conn);

  m_filePath.clear();
  m_connectionName.clear();

  if (clearSavedPath)
    m_settings.remove("Sessions/LastDatabase");

  Q_EMIT openChanged();
  Q_EMIT sessionsChanged();
  Q_EMIT tagsChanged();
  Q_EMIT selectedSessionChanged();
  if (wasLocked)
    Q_EMIT lockedChanged();
}

//--------------------------------------------------------------------------------------------------
// Lock / unlock
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a password, hashes it, and locks the open session file.
 */
void Sessions::DatabaseManager::lockDatabase()
{
  if (!isOpen() || m_locked)
    return;

  bool ok          = false;
  const auto first = QInputDialog::getText(nullptr,
                                           tr("Lock Session File"),
                                           tr("Choose a password to lock the session file:"),
                                           QLineEdit::Password,
                                           QString(),
                                           &ok);
  if (!ok || first.isEmpty())
    return;

  const auto second = QInputDialog::getText(nullptr,
                                            tr("Lock Session File"),
                                            tr("Confirm the password:"),
                                            QLineEdit::Password,
                                            QString(),
                                            &ok);

  if (first != second || !ok) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Passwords do not match"),
        tr("The two passwords you entered do not match. The session file was not locked."),
        QMessageBox::Warning);
    });
    return;
  }

  m_passwordHash =
    QString::fromLatin1(QCryptographicHash::hash(first.toUtf8(), QCryptographicHash::Md5).toHex());
  m_locked = true;

  persistLockState();
  Q_EMIT lockedChanged();
}

/**
 * @brief Prompts for the password, verifies it, and clears the lock on success.
 */
void Sessions::DatabaseManager::unlockDatabase()
{
  if (!isOpen() || !m_locked)
    return;

  if (m_passwordHash.isEmpty()) {
    m_locked = false;
    persistLockState();
    Q_EMIT lockedChanged();
    return;
  }

  bool ok        = false;
  const auto pwd = QInputDialog::getText(nullptr,
                                         tr("Unlock Session File"),
                                         tr("Enter the session file password:"),
                                         QLineEdit::Password,
                                         QString(),
                                         &ok);
  if (!ok)
    return;

  const auto hashPwd =
    QString::fromLatin1(QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5).toHex());

  if (hashPwd != m_passwordHash) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Incorrect password"),
        tr("The password you entered does not match the one stored in the session file."),
        QMessageBox::Warning);
    });
    return;
  }

  m_passwordHash.clear();
  m_locked = false;

  persistLockState();
  Q_EMIT lockedChanged();
}

//--------------------------------------------------------------------------------------------------
// Session management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects a session by ID for detail display.
 */
void Sessions::DatabaseManager::setSelectedSessionId(int sessionId)
{
  if (m_selectedSessionId == sessionId)
    return;

  m_selectedSessionId = sessionId;
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Persists the notes for the currently selected session.
 */
void Sessions::DatabaseManager::setSelectedSessionNotes(const QString& notes)
{
  if (m_selectedSessionId < 0 || !isOpen())
    return;

  if (selectedSessionNotes() == notes)
    return;

  QSqlQuery q(m_db);
  q.prepare("UPDATE sessions SET notes = ? WHERE session_id = ?");
  q.bindValue(0, notes);
  q.bindValue(1, m_selectedSessionId);
  if (!q.exec()) [[unlikely]]
    qWarning() << "[Sessions] setSelectedSessionNotes failed:" << q.lastError().text();
}

/**
 * @brief Deletes a session and its associated readings, raw bytes, tags,
 * and table snapshots.
 */
void Sessions::DatabaseManager::deleteSession(int sessionId)
{
  Q_ASSERT(sessionId >= 0);

  if (!isOpen() || m_locked)
    return;

  if (!m_db.transaction()) [[unlikely]] {
    qWarning() << "[Sessions] deleteSession: transaction() failed:" << m_db.lastError().text();
    return;
  }

  // Run each delete and roll back on failure
  const auto runDelete = [&](const char* sql) -> bool {
    QSqlQuery q(m_db);
    q.prepare(QString::fromLatin1(sql));
    q.bindValue(0, sessionId);
    if (!q.exec()) [[unlikely]] {
      qWarning() << "[Sessions] deleteSession:" << sql << "failed:" << q.lastError().text();
      m_db.rollback();
      return false;
    }

    return true;
  };

  // Cascade delete related rows
  if (!runDelete("DELETE FROM readings WHERE session_id = ?"))
    return;

  if (!runDelete("DELETE FROM raw_bytes WHERE session_id = ?"))
    return;

  if (!runDelete("DELETE FROM table_snapshots WHERE session_id = ?"))
    return;

  if (!runDelete("DELETE FROM session_tags WHERE session_id = ?"))
    return;

  if (!runDelete("DELETE FROM columns WHERE session_id = ?"))
    return;

  if (!runDelete("DELETE FROM sessions WHERE session_id = ?"))
    return;

  if (!m_db.commit()) [[unlikely]] {
    qWarning() << "[Sessions] deleteSession: commit() failed:" << m_db.lastError().text();
    m_db.rollback();
    return;
  }

  if (m_selectedSessionId == sessionId) {
    m_selectedSessionId = -1;
    Q_EMIT selectedSessionChanged();
  }

  refreshSessionList();
}

/**
 * @brief Asks the user to confirm before deleting a session.
 */
void Sessions::DatabaseManager::confirmDeleteSession(int sessionId)
{
  if (m_locked) {
    Misc::Utilities::showMessageBox(
      tr("Session file locked"),
      tr("Unlock the session file before deleting recorded sessions."),
      QMessageBox::Information);
    return;
  }

  const auto meta  = sessionMetadata(sessionId);
  const auto title = meta.value("started_at").toString();

  const int choice = Misc::Utilities::showMessageBox(
    tr("Delete session from %1?").arg(title),
    tr("All readings and raw data for this session are permanently removed."),
    QMessageBox::Warning,
    tr("Delete Session"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteSession(sessionId);
}

/**
 * @brief Restores the project embedded in the selected session and starts playback.
 */
void Sessions::DatabaseManager::replaySelectedSession()
{
  if (m_selectedSessionId < 0 || m_filePath.isEmpty())
    return;

  // Warn early if this session lacks an embedded snapshot
  if (sessionProjectJson(m_selectedSessionId).isEmpty()) {
    Misc::Utilities::showMessageBox(tr("No project data"),
                                    tr("This session does not contain an embedded project file — "
                                       "the dashboard falls back to a quick-plot layout."),
                                    QMessageBox::Warning);
  }

  Sessions::Player::instance().openFile(m_filePath, m_selectedSessionId);
}

//--------------------------------------------------------------------------------------------------
// Tag CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new tag label in the database.
 */
void Sessions::DatabaseManager::addTag(const QString& label)
{
  if (!isOpen() || label.trimmed().isEmpty())
    return;

  QSqlQuery q(m_db);
  q.prepare("INSERT OR IGNORE INTO tags (label) VALUES (?)");
  q.bindValue(0, label.trimmed());
  q.exec();

  loadTagList();
}

/**
 * @brief Permanently removes a tag and all its session associations.
 */
void Sessions::DatabaseManager::deleteTag(int tagId)
{
  if (!isOpen() || m_locked)
    return;

  m_db.transaction();
  QSqlQuery q(m_db);
  q.prepare("DELETE FROM session_tags WHERE tag_id = ?");
  q.bindValue(0, tagId);
  q.exec();

  q.prepare("DELETE FROM tags WHERE tag_id = ?");
  q.bindValue(0, tagId);
  q.exec();
  m_db.commit();

  loadTagList();
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Renames an existing tag.
 */
void Sessions::DatabaseManager::renameTag(int tagId, const QString& newLabel)
{
  if (!isOpen() || newLabel.trimmed().isEmpty())
    return;

  QSqlQuery q(m_db);
  q.prepare("UPDATE tags SET label = ? WHERE tag_id = ?");
  q.bindValue(0, newLabel.trimmed());
  q.bindValue(1, tagId);
  q.exec();

  loadTagList();
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Assigns a tag to a session.
 */
void Sessions::DatabaseManager::assignTagToSession(int sessionId, int tagId)
{
  if (!isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("INSERT OR IGNORE INTO session_tags (session_id, tag_id) VALUES (?, ?)");
  q.bindValue(0, sessionId);
  q.bindValue(1, tagId);
  q.exec();

  Q_EMIT selectedSessionChanged();
  refreshSessionList();
}

/**
 * @brief Removes a tag assignment from a session.
 */
void Sessions::DatabaseManager::removeTagFromSession(int sessionId, int tagId)
{
  if (!isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("DELETE FROM session_tags WHERE session_id = ? AND tag_id = ?");
  q.bindValue(0, sessionId);
  q.bindValue(1, tagId);
  q.exec();

  Q_EMIT selectedSessionChanged();
  refreshSessionList();
}

//--------------------------------------------------------------------------------------------------
// CSV export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Exports a single session to a CSV file in the workspace folder.
 */
void Sessions::DatabaseManager::exportSessionToCsv(int sessionId)
{
  if (!isOpen() || m_csvExportBusy)
    return;

  // Nest the output under the project title, like the PDF/HTML reports do
  const auto meta         = sessionMetadata(sessionId);
  const QString projTitle = meta.value("project_title").toString();
  const QString safeProj  = sanitiseTitleForPath(projTitle);
  const QString dir =
    QStringLiteral("%1/%2").arg(Misc::WorkspaceManager::instance().path("CSV"), safeProj);
  QDir().mkpath(dir);

  // Prompt for the final file path
  const QString suggested = QStringLiteral("%1/session_%2.csv").arg(dir).arg(sessionId);
  const auto path         = QFileDialog::getSaveFileName(
    nullptr, tr("Export Session to CSV"), suggested, tr("CSV files (*.csv)"));
  if (path.isEmpty())
    return;

  m_csvExportBusy = true;
  Q_EMIT csvExportBusyChanged();

  // Load column order for this session
  QSqlQuery colQ(m_db);
  colQ.prepare("SELECT unique_id, group_title, title, units FROM columns "
               "WHERE session_id = ? ORDER BY column_id ASC");
  colQ.bindValue(0, sessionId);
  if (!colQ.exec()) {
    m_csvExportBusy = false;
    Q_EMIT csvExportBusyChanged();
    return;
  }

  // Build the CSV header from the column metadata
  std::vector<int> uniqueIds;
  QStringList headerCells;
  headerCells.append("Timestamp (s)");
  while (colQ.next()) {
    uniqueIds.push_back(colQ.value(0).toInt());
    const auto group = colQ.value(1).toString();
    const auto title = colQ.value(2).toString();
    const auto units = colQ.value(3).toString();
    auto label       = group + "/" + title;
    if (!units.isEmpty())
      label += " (" + units + ")";

    headerCells.append(label);
  }

  // Fetch readings in (timestamp, reading_id) order
  QSqlQuery readQ(m_db);
  readQ.setForwardOnly(true);
  readQ.prepare(
    "SELECT timestamp_ns, unique_id, final_numeric_value, final_string_value, is_numeric "
    "FROM readings WHERE session_id = ? ORDER BY timestamp_ns, reading_id");
  readQ.bindValue(0, sessionId);
  if (!readQ.exec()) {
    m_csvExportBusy = false;
    Q_EMIT csvExportBusyChanged();
    return;
  }

  // uniqueId → column index for fast placement while streaming
  QMap<int, int> uidToCol;
  for (int i = 0; i < static_cast<int>(uniqueIds.size()); ++i)
    uidToCol.insert(uniqueIds[static_cast<size_t>(i)], i);

  // Open the output file
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    m_csvExportBusy = false;
    Q_EMIT csvExportBusyChanged();
    return;
  }

  QTextStream out(&file);
  out << headerCells.join(',') << '\n';

  // Row state + helpers for the streaming write
  qint64 currentTs = -1;
  QStringList row;
  const int colCount = static_cast<int>(uniqueIds.size());

  auto flushRow = [&]() {
    if (currentTs < 0 || row.isEmpty())
      return;

    out << QString::number(currentTs / 1e9, 'f', 9);
    for (const auto& cell : std::as_const(row))
      out << ',' << cell;

    out << '\n';
  };

  auto startRow = [&](qint64 ts) {
    currentTs = ts;
    row.clear();
    row.reserve(colCount);
    for (int i = 0; i < colCount; ++i)
      row.append(QString());
  };

  // Emit one CSV row per distinct timestamp
  while (readQ.next()) {
    const qint64 ts  = readQ.value(0).toLongLong();
    const int uid    = readQ.value(1).toInt();
    const auto colIt = uidToCol.constFind(uid);
    if (colIt == uidToCol.constEnd())
      continue;

    if (ts != currentTs) {
      flushRow();
      startRow(ts);
    }

    const bool isNumeric = readQ.value(4).toInt() != 0;
    const int col        = colIt.value();
    if (isNumeric)
      row[col] = QString::number(readQ.value(2).toDouble(), 'g', 17);
    else
      row[col] = readQ.value(3).toString();
  }

  // Flush the last in-flight row and close
  flushRow();
  file.close();

  m_csvExportBusy = false;
  Q_EMIT csvExportBusyChanged();
  Q_EMIT csvExportFinished(path, true);

  Misc::Utilities::revealFile(path);
}

//--------------------------------------------------------------------------------------------------
// Session report export (HTML + PDF via QWebEngine)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders a session report using the options bundle from QML.
 * @param sessionId Session to export.
 * @param options   QVariantMap populated by @c ReportOptionsDialog.qml.
 */
void Sessions::DatabaseManager::exportSessionToPdf(int sessionId, const QVariantMap& options)
{
  if (!isOpen() || m_pdfExportBusy)
    return;

  // Translate the QVariantMap into typed options
  HtmlReportOptions opts;
  opts.outputPath    = options.value("outputPath").toString();
  opts.companyName   = options.value("companyName").toString();
  opts.documentTitle = options.value("documentTitle").toString();
  opts.authorName    = options.value("authorName").toString();
  opts.logoPath      = options.value("logoPath").toString();
  opts.pageSize =
    static_cast<QPageSize::PageSizeId>(options.value("pageSize", QPageSize::A4).toInt());
  opts.includeCover        = options.value("includeCover", true).toBool();
  opts.includeMetadata     = options.value("includeMetadata", true).toBool();
  opts.includeStats        = options.value("includeStats", true).toBool();
  opts.includeCharts       = options.value("includeCharts", true).toBool();
  opts.includeStatsOverlay = options.value("includeStatsOverlay", true).toBool();
  opts.lineWidth           = options.value("lineWidth", 1.4).toDouble();
  opts.lineStyle           = options.value("lineStyle", QStringLiteral("solid")).toString();

  // Decode the output format string — defaults to PDF for backwards compat
  const QString fmtStr = options.value("outputFormat", QStringLiteral("pdf")).toString().toLower();
  if (fmtStr == QStringLiteral("html"))
    opts.format = HtmlReportOptions::Format::Html;
  else if (fmtStr == QStringLiteral("both"))
    opts.format = HtmlReportOptions::Format::Both;
  else
    opts.format = HtmlReportOptions::Format::Pdf;

  // Run the export once the output path is known. Heavy work (SQL data
  // assembly + chart-series load) is chained through QTimer::singleShot so
  // each stage yields to the event loop — the progress dialog gets a chance
  // to paint and refresh its status text instead of freezing for seconds.
  auto startExport = [this, sessionId](HtmlReportOptions opts) {
    // HTML-only is synchronous — skip the progress dialog
    const bool reportBusy = (opts.format != HtmlReportOptions::Format::Html);
    if (reportBusy) {
      // Wait cursor while the progress dialog is preparing to paint
      QGuiApplication::setOverrideCursor(Qt::WaitCursor);

      m_pdfExportBusy     = true;
      m_pdfExportProgress = 0.0;
      m_pdfExportStatus   = tr("Preparing export…");
      Q_EMIT pdfExportBusyChanged();
      Q_EMIT pdfExportProgressChanged();
    }

    // Stage 3: kick off rendering once data + series are ready
    auto runRender = [this, reportBusy](ReportData data,
                                        std::vector<DatasetSeries> series,
                                        HtmlReportOptions opts) {
      auto* renderer = new HtmlReport(this);

      if (reportBusy) {
        connect(renderer,
                &HtmlReport::progress,
                this,
                [this](const QString& status, double percent) {
                  m_pdfExportStatus   = status;
                  m_pdfExportProgress = percent;
                  Q_EMIT pdfExportProgressChanged();
                });
      }

      connect(renderer,
              &HtmlReport::finished,
              this,
              [this, renderer, reportBusy](const QString& outputPath, bool ok) {
                if (reportBusy) {
                  m_pdfExportBusy     = false;
                  m_pdfExportProgress = 1.0;
                  m_pdfExportStatus   = ok ? tr("Done") : tr("Failed");
                  Q_EMIT pdfExportBusyChanged();
                  Q_EMIT pdfExportProgressChanged();
                }
                Q_EMIT pdfExportFinished(outputPath, ok);

                if (ok) {
                  // Reveal in Finder/Explorer
                  Misc::Utilities::revealFile(outputPath);
                } else {
                  Misc::Utilities::showMessageBox(
                    tr("Report Failed"),
                    tr("Could not generate the report. Check the output path and try again."),
                    QMessageBox::Warning);
                }

                renderer->deleteLater();
              });

      renderer->render(std::move(data), std::move(series), opts);
    };

    // Stage 2: load chart series (also heavy when many samples are stored)
    auto runChartLoad = [this, sessionId, reportBusy, runRender](ReportData data,
                                                                 HtmlReportOptions opts) {
      std::vector<DatasetSeries> series;
      if (opts.includeCharts) {
        if (reportBusy) {
          m_pdfExportStatus = tr("Loading chart data…");
          Q_EMIT pdfExportProgressChanged();
        }

        QTimer::singleShot(
          0,
          this,
          [this, sessionId, data = std::move(data), opts = std::move(opts), runRender]() mutable {
            auto series = loadChartSeries(m_db, sessionId, 10000);
            runRender(std::move(data), std::move(series), std::move(opts));
          });
      } else {
        runRender(std::move(data), std::move(series), std::move(opts));
      }
    };

    // Stage 1: assemble the data bundle from SQL — defer so the dialog paints
    QTimer::singleShot(
      0,
      this,
      [this, sessionId, reportBusy, opts = std::move(opts), runChartLoad]() mutable {
        if (reportBusy) {
          // Dialog has painted; the progress bar takes over from the wait cursor
          QGuiApplication::restoreOverrideCursor();
          m_pdfExportStatus = tr("Loading session data…");
          Q_EMIT pdfExportProgressChanged();
        }

        auto data = ReportData::buildFromSession(m_db, sessionId);
        runChartLoad(std::move(data), std::move(opts));
      });
  };

  // Preset path skips the picker
  if (!opts.outputPath.isEmpty()) {
    startExport(std::move(opts));
    return;
  }

  const bool wantsPdf  = (opts.format != HtmlReportOptions::Format::Html);
  const QString ext    = wantsPdf ? QStringLiteral("pdf") : QStringLiteral("html");
  const QString title  = wantsPdf ? tr("Save PDF Report") : tr("Save HTML Report");
  const QString filter = wantsPdf ? tr("PDF files (*.pdf)") : tr("HTML files (*.html)");

  // Default directory matches the per-project nesting of other exporters
  const auto meta         = sessionMetadata(sessionId);
  const QString projTitle = meta.value("project_title").toString();
  const QString safeProj  = sanitiseTitleForPath(projTitle);
  const QString dir =
    QStringLiteral("%1/%2").arg(Misc::WorkspaceManager::instance().path("Reports"), safeProj);
  QDir().mkpath(dir);

  const QString baseName  = opts.documentTitle.isEmpty()
                            ? QStringLiteral("session_%1").arg(sessionId)
                            : sanitiseTitleForPath(opts.documentTitle);
  const QString suggested = QStringLiteral("%1/%2.%3").arg(dir, baseName, ext);

  auto* dialog = new QFileDialog(nullptr, title, suggested, filter);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setFileMode(QFileDialog::AnyFile);

  // fileSelected also fires on cancel (empty path re-enables the QML button)
  connect(dialog,
          &QFileDialog::fileSelected,
          this,
          [this, dialog, opts, ext, startExport](const QString& path) mutable {
            dialog->deleteLater();

            if (path.isEmpty()) {
              Q_EMIT pdfExportFinished(QString(), false);
              return;
            }

            // Enforce the expected extension
            QString finalPath = path;
            const QString dot = QStringLiteral(".") + ext;
            if (!finalPath.endsWith(dot, Qt::CaseInsensitive))
              finalPath += dot;

            opts.outputPath = finalPath;
            startExport(std::move(opts));
          });

  dialog->open();
}

//--------------------------------------------------------------------------------------------------
// Project metadata sync
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the current project JSON in the project_metadata table.
 */
void Sessions::DatabaseManager::storeProjectMetadata()
{
  if (!isOpen())
    return;

  const auto& pm  = DataModel::ProjectModel::instance();
  const auto json = QJsonDocument(pm.serializeToJson()).toJson(QJsonDocument::Compact);
  const auto now  = QDateTime::currentDateTime().toString(Qt::ISODate);

  m_db.transaction();
  QSqlQuery q(m_db);

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_json', ?)");
  q.bindValue(0, QString::fromUtf8(json));
  q.exec();

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_title', ?)");
  q.bindValue(0, pm.title());
  q.exec();

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('last_modified_at', ?)");
  q.bindValue(0, now);
  q.exec();

  // created_at only set once
  q.prepare("INSERT OR IGNORE INTO project_metadata (key, value) VALUES ('created_at', ?)");
  q.bindValue(0, now);
  q.exec();

  m_db.commit();
}

/**
 * @brief Restores the project from the stored JSON in the database.
 */
void Sessions::DatabaseManager::restoreProjectFromDb()
{
  // Read the stored project JSON
  const auto json = projectJsonFromDb();
  if (json.isEmpty()) {
    Misc::Utilities::showMessageBox(tr("No project data"),
                                    tr("This session file does not contain an embedded project."),
                                    QMessageBox::Warning);
    return;
  }

  // Parse to validate before asking the user where to save
  QJsonParseError parseError{};
  const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || doc.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("Invalid project data"),
      tr("The embedded project JSON is malformed and cannot be restored."),
      QMessageBox::Critical);
    return;
  }

  // Suggest a filename based on the stored project title
  const auto title       = doc.object().value("title").toString("Restored Project");
  const auto projectsDir = DataModel::ProjectModel::instance().jsonProjectsPath();
  const auto suggested   = QStringLiteral("%1/%2.ssproj").arg(projectsDir, title);

  // Let the user pick the output path
  const auto path = QFileDialog::getSaveFileName(
    nullptr, tr("Restore Project"), suggested, tr("Serial Studio projects (*.ssproj *.json)"));

  if (path.isEmpty())
    return;

  // Write the project file
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(
      tr("Cannot write file"), tr("Check file permissions and try again."), QMessageBox::Critical);
    return;
  }

  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  // Open the saved project as the active project
  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  DataModel::ProjectModel::instance().openJsonFile(path);

  Q_EMIT projectMetadataRestored();
}

/**
 * @brief Reopens the database that was open during the last application run.
 */
void Sessions::DatabaseManager::restoreLastDatabase()
{
  const auto path = m_settings.value("Sessions/LastDatabase").toString();
  if (path.isEmpty())
    return;

  // Clear stale setting if the file was deleted externally
  if (!QFileInfo::exists(path)) {
    m_settings.remove("Sessions/LastDatabase");
    return;
  }

  openDatabase(path);
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reloads the session list from the database.
 */
void Sessions::DatabaseManager::refreshSessionList()
{
  m_sessionList.clear();
  if (!isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("SELECT s.session_id, s.project_title, s.started_at, s.ended_at, s.notes, "
            "       (SELECT COUNT(DISTINCT timestamp_ns) FROM readings "
            "        WHERE session_id = s.session_id) "
            "FROM sessions s ORDER BY s.started_at DESC");

  if (!q.exec())
    return;

  while (q.next()) {
    QVariantMap row;
    row["session_id"]    = q.value(0).toInt();
    row["project_title"] = q.value(1).toString();
    row["started_at"]    = formatDateForDisplay(q.value(2).toString());
    row["ended_at"]      = formatDateForDisplay(q.value(3).toString());
    row["notes"]         = q.value(4).toString();
    row["frame_count"]   = q.value(5).toInt();

    // Collect tag labels for this session
    const auto tags = tagsForSession(q.value(0).toInt());
    QStringList labels;
    for (const auto& t : tags)
      labels.append(t.toMap().value("label").toString());

    row["tag_labels"] = labels.join(", ");

    m_sessionList.append(row);
  }

  Q_EMIT sessionsChanged();
}

/**
 * @brief Reloads the tag list from the database.
 */
void Sessions::DatabaseManager::loadTagList()
{
  m_tagList.clear();
  if (!isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("SELECT tag_id, label FROM tags ORDER BY label COLLATE NOCASE");
  if (!q.exec())
    return;

  while (q.next()) {
    QVariantMap tag;
    tag["tag_id"] = q.value(0).toInt();
    tag["label"]  = q.value(1).toString();
    m_tagList.append(tag);
  }

  Q_EMIT tagsChanged();
}

/**
 * @brief Loads the password hash from project_metadata and updates the lock flag.
 */
void Sessions::DatabaseManager::loadLockState()
{
  m_passwordHash.clear();
  const bool wasLocked = m_locked;
  m_locked             = false;

  if (isOpen()) {
    QSqlQuery q(m_db);
    q.prepare("SELECT value FROM project_metadata WHERE key = 'lock_password_hash'");
    if (q.exec() && q.next())
      m_passwordHash = q.value(0).toString();

    m_locked = !m_passwordHash.isEmpty();
  }

  if (m_locked != wasLocked)
    Q_EMIT lockedChanged();
}

/**
 * @brief Writes the current password hash into project_metadata, or removes it.
 */
void Sessions::DatabaseManager::persistLockState()
{
  if (!isOpen())
    return;

  QSqlQuery q(m_db);
  if (m_passwordHash.isEmpty()) {
    q.prepare("DELETE FROM project_metadata WHERE key = 'lock_password_hash'");
    if (!q.exec()) [[unlikely]]
      qWarning() << "[Sessions] persistLockState clear failed:" << q.lastError().text();
    return;
  }

  q.prepare(
    "INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('lock_password_hash', ?)");
  q.bindValue(0, m_passwordHash);
  if (!q.exec()) [[unlikely]]
    qWarning() << "[Sessions] persistLockState write failed:" << q.lastError().text();
}

/**
 * @brief Runs the shared CREATE TABLE IF NOT EXISTS block on the open database.
 */
void Sessions::DatabaseManager::ensureSchema()
{
  if (!isOpen())
    return;

  QSqlQuery q(m_db);
  createSchema(q);
}

/**
 * @brief Creates or upgrades the session-log schema on the open database.
 */
void Sessions::DatabaseManager::createSchema(QSqlQuery& q)
{
  // Session header
  q.exec("CREATE TABLE IF NOT EXISTS sessions ("
         "  session_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  project_title TEXT NOT NULL,"
         "  started_at    TEXT NOT NULL,"
         "  ended_at      TEXT,"
         "  project_json  TEXT,"
         "  notes         TEXT"
         ")");

  // Column definitions exported from ExportSchema at session start
  q.exec("CREATE TABLE IF NOT EXISTS columns ("
         "  column_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id  INTEGER NOT NULL REFERENCES sessions,"
         "  unique_id   INTEGER NOT NULL,"
         "  group_title TEXT NOT NULL,"
         "  title       TEXT NOT NULL,"
         "  units       TEXT,"
         "  widget      TEXT,"
         "  is_virtual  INTEGER NOT NULL DEFAULT 0"
         ")");

  // One row per dataset per frame; surrogate PK allows same-ns frames to coexist
  q.exec("CREATE TABLE IF NOT EXISTS readings ("
         "  reading_id          INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id          INTEGER NOT NULL,"
         "  timestamp_ns        INTEGER NOT NULL,"
         "  unique_id           INTEGER NOT NULL,"
         "  raw_numeric_value   REAL,"
         "  raw_string_value    TEXT,"
         "  final_numeric_value REAL,"
         "  final_string_value  TEXT,"
         "  is_numeric          INTEGER NOT NULL DEFAULT 1"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_readings_session_uid_ts "
         "ON readings (session_id, unique_id, timestamp_ns)");
  q.exec("CREATE INDEX IF NOT EXISTS idx_readings_session_ts "
         "ON readings (session_id, timestamp_ns)");

  // Raw device bytes captured in parallel with the frame stream
  q.exec("CREATE TABLE IF NOT EXISTS raw_bytes ("
         "  raw_id       INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL,"
         "  timestamp_ns INTEGER NOT NULL,"
         "  device_id    INTEGER NOT NULL DEFAULT 0,"
         "  data         BLOB NOT NULL"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_raw_bytes_session_ts "
         "ON raw_bytes (session_id, timestamp_ns)");

  // Per-frame snapshots of user data tables
  q.exec("CREATE TABLE IF NOT EXISTS table_snapshots ("
         "  snapshot_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id    INTEGER NOT NULL,"
         "  timestamp_ns  INTEGER NOT NULL,"
         "  table_name    TEXT NOT NULL,"
         "  register_name TEXT NOT NULL,"
         "  numeric_value REAL,"
         "  string_value  TEXT"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_snapshots_session_ts "
         "ON table_snapshots (session_id, timestamp_ns, table_name)");

  // Free-form tags and the session → tag join
  q.exec("CREATE TABLE IF NOT EXISTS tags ("
         "  tag_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  label  TEXT NOT NULL UNIQUE COLLATE NOCASE"
         ")");

  q.exec("CREATE TABLE IF NOT EXISTS session_tags ("
         "  session_id INTEGER NOT NULL,"
         "  tag_id     INTEGER NOT NULL,"
         "  PRIMARY KEY (session_id, tag_id)"
         ") WITHOUT ROWID");

  // Key/value store for project-level metadata
  q.exec("CREATE TABLE IF NOT EXISTS project_metadata ("
         "  key   TEXT PRIMARY KEY,"
         "  value TEXT NOT NULL"
         ") WITHOUT ROWID");
}

#endif  // BUILD_COMMERCIAL
