/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/DatabaseManager.h"

#  include <QApplication>
#  include <QCoreApplication>
#  include <QDateTime>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QInputDialog>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QJsonParseError>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QThread>
#  include <QTimer>

#  include "AppState.h"
#  include "DataModel/ProjectModel.h"
#  include "Misc/PasswordHash.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "SerialStudio.h"
#  include "Sessions/DatabaseManager/DatabaseSchema.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#  include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// File-local state
//--------------------------------------------------------------------------------------------------

static QString s_dbPathOverride;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes member variables and spins up the worker thread.
 */
Sessions::DatabaseManager::DatabaseManager()
  : m_thread(nullptr)
  , m_worker(nullptr)
  , m_open(false)
  , m_selectedSessionId(-1)
  , m_locked(false)
  , m_nextToken(1)
  , m_outstandingMutations(0)
  , m_workspaceManager(nullptr)
  , m_player(nullptr)
  , m_projectModel(nullptr)
  , m_appState(nullptr)
{
  qRegisterMetaType<Sessions::ReportPayloadPtr>("Sessions::ReportPayloadPtr");
  initWorker();
  wireVerifier();
  wireExporter();
}

/**
 * @brief Stops the worker thread and tears the singleton down.
 */
Sessions::DatabaseManager::~DatabaseManager()
{
  shutdown();
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
// Worker lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the worker, parks it on a dedicated thread, wires signals.
 */
void Sessions::DatabaseManager::initWorker()
{
  m_thread = new QThread;
  m_thread->setObjectName(QStringLiteral("Sessions::DatabaseWorker"));

  m_worker = new DatabaseWorker;
  m_worker->moveToThread(m_thread);

  connect(m_worker, &DatabaseWorker::opened, this, &DatabaseManager::onWorkerOpened);
  connect(m_worker, &DatabaseWorker::openFailed, this, &DatabaseManager::onWorkerOpenFailed);
  connect(m_worker, &DatabaseWorker::closed, this, &DatabaseManager::onWorkerClosed);
  connect(m_worker,
          &DatabaseWorker::sessionListRefreshed,
          this,
          &DatabaseManager::onWorkerSessionListRefreshed);
  connect(
    m_worker, &DatabaseWorker::tagListRefreshed, this, &DatabaseManager::onWorkerTagListRefreshed);
  connect(
    m_worker, &DatabaseWorker::lockStateChanged, this, &DatabaseManager::onWorkerLockStateChanged);
  connect(m_worker, &DatabaseWorker::notesUpdated, this, &DatabaseManager::onWorkerNotesUpdated);
  connect(
    m_worker, &DatabaseWorker::mutationFinished, this, &DatabaseManager::onWorkerMutationFinished);
  m_exporter.setWorker(m_worker);
  connect(
    m_worker, &DatabaseWorker::csvExportProgress, &m_exporter, &SessionExporter::onCsvProgress);
  connect(
    m_worker, &DatabaseWorker::csvExportFinished, &m_exporter, &SessionExporter::onCsvFinished);
  connect(
    m_worker, &DatabaseWorker::reportDataReady, &m_exporter, &SessionExporter::onReportDataReady);
  connect(
    m_worker, &DatabaseWorker::datasetListReady, this, &DatabaseManager::sessionDatasetsReady);
  connect(
    m_worker, &DatabaseWorker::streamStatsReady, this, &DatabaseManager::sessionStreamStatsReady);
  connect(m_worker,
          &DatabaseWorker::globalProjectJsonReady,
          this,
          &DatabaseManager::onWorkerGlobalProjectJsonReady);

  m_thread->start();
}

/**
 * @brief Synchronously closes the worker DB and joins the worker thread. A timed-out join
 *        leaks the worker and thread on purpose: deleting objects a live thread still runs
 *        is a use-after-free. The join keys on thread state alone; only the blocking
 *        closeDatabase hop needs qApp, so a static-dtor call after qApp dies still joins.
 */
void Sessions::DatabaseManager::shutdown()
{
  m_verifier.shutdown();

  if (!m_thread)
    return;

  if (m_worker)
    m_worker->requestCancel();

  bool joined = true;
  if (m_thread->isRunning()) {
    if (qApp && m_worker)
      QMetaObject::invokeMethod(m_worker, "closeDatabase", Qt::BlockingQueuedConnection);

    m_thread->quit();
    joined = m_thread->wait(5000);
  }

  if (!joined) {
    qWarning() << "[Sessions] Database worker did not stop within 5 s; leaking the worker "
                  "thread to avoid tearing down objects it still uses";
    m_worker = nullptr;
    m_thread = nullptr;
    return;
  }

  delete m_worker;
  m_worker = nullptr;

  delete m_thread;
  m_thread = nullptr;
}

//--------------------------------------------------------------------------------------------------
// External connections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires cross-singleton signals. Called after all singletons are alive.
 */
void Sessions::DatabaseManager::setupExternalConnections()
{
  m_workspaceManager = &Misc::WorkspaceManager::instance();
  m_player           = &Sessions::Player::instance();
  m_projectModel     = &DataModel::ProjectModel::instance();
  m_appState         = &AppState::instance();
  m_exporter.setWorkspace(m_workspaceManager);

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
  return m_open;
}

/**
 * @brief Returns @c true while a worker mutation or open is in flight.
 */
bool Sessions::DatabaseManager::busy() const
{
  return m_outstandingMutations > 0;
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
  return m_exporter.csvBusy();
}

/**
 * @brief Returns @c true while a PDF report is being rendered.
 */
bool Sessions::DatabaseManager::pdfExportBusy() const
{
  return m_exporter.pdfBusy();
}

/**
 * @brief Returns the user-facing status label for the active export.
 */
QString Sessions::DatabaseManager::pdfExportStatus() const
{
  return m_exporter.pdfStatus();
}

/**
 * @brief Returns the active PDF export's progress as a fraction in [0, 1].
 */
double Sessions::DatabaseManager::pdfExportProgress() const
{
  return m_exporter.pdfProgress();
}

/**
 * @brief Returns the active CSV export's progress as a fraction in [0, 1].
 */
double Sessions::DatabaseManager::csvExportProgress() const
{
  return m_exporter.csvProgress();
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
  if (m_selectedSessionId < 0)
    return {};

  for (const auto& v : m_sessionList) {
    const auto m = v.toMap();
    if (m.value("session_id").toInt() == m_selectedSessionId)
      return m.value("notes").toString();
  }

  return {};
}

//--------------------------------------------------------------------------------------------------
// Cache lookups (Q_INVOKABLE)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the tags assigned to a specific session from the cache.
 */
QVariantList Sessions::DatabaseManager::tagsForSession(int sessionId) const
{
  for (const auto& v : m_sessionList) {
    const auto m = v.toMap();
    if (m.value("session_id").toInt() == sessionId)
      return m.value("tags").toList();
  }

  return {};
}

/**
 * @brief Returns full metadata for a session from the cached list.
 */
QVariantMap Sessions::DatabaseManager::sessionMetadata(int sessionId) const
{
  QVariantMap result;
  for (const auto& v : m_sessionList) {
    const auto m = v.toMap();
    if (m.value("session_id").toInt() != sessionId)
      continue;

    result["session_id"]    = sessionId;
    result["project_title"] = m.value("project_title");
    result["started_at"]    = m.value("started_at");
    result["ended_at"]      = m.value("ended_at");
    result["notes"]         = m.value("notes");
    result["frame_count"]   = m.value("frame_count");
    result["size_bytes"]    = m.value("size_bytes");
    return result;
  }

  return result;
}

/**
 * @brief Redirects new historian databases to @p path (spec-0044 verifier child process only).
 *        Empty restores the canonical workspace location. The export worker reads this on its
 *        own thread, so the override may only change while no recording session is open: the
 *        worker enable/close handoff is what orders the access.
 */
void Sessions::DatabaseManager::setDbPathOverride(const QString& path)
{
  // code-verify off: the only caller overrides the path after the pinned order built Export
  SS_ASSERT_LOG(!Sessions::Export::instance().isOpen());
  // code-verify on
  s_dbPathOverride = path;
}

/**
 * @brief Returns the canonical .db path for a project title. Pure path arithmetic.
 */
QString Sessions::DatabaseManager::canonicalDbPath(const QString& projectTitle)
{
  if (!s_dbPathOverride.isEmpty())
    return s_dbPathOverride;

  const QString safeTitle       = Sessions::sanitiseTitleForPath(projectTitle);
  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  const auto subdir             = workspaceManager.path("Session Databases");
  return QStringLiteral("%1/%2/%2.db").arg(subdir, safeTitle);
}

//--------------------------------------------------------------------------------------------------
// Reproducibility verification (spec 0044)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while a verification child process is running.
 */
bool Sessions::DatabaseManager::verificationBusy() const
{
  return m_verifier.busy();
}

/**
 * @brief Returns true while the shared child slot is running a regression pass (spec 0047).
 */
bool Sessions::DatabaseManager::regressionBusy() const
{
  return m_verifier.regressionBusy();
}

/**
 * @brief Returns the last regression report; ephemeral, cleared when a new pass starts.
 */
QVariantMap Sessions::DatabaseManager::lastRegressionReport() const
{
  return m_verifier.lastReport();
}

/**
 * @brief Returns the latest stored verification record for @p sessionId (empty map when the
 *        session was never verified or the archive predates the verifications table).
 */
QVariantMap Sessions::DatabaseManager::latestVerification(int sessionId) const
{
  QVariantMap result;
  if (!m_open || m_filePath.isEmpty())
    return result;

  const QString connName = QStringLiteral("ss_dbm_verification_read");
  {
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    db.setDatabaseName(m_filePath);
    db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
    if (db.open()) {
      QSqlQuery q(db);
      q.prepare(
        QStringLiteral("SELECT verified_at, app_version, verdict, detail_json FROM verifications "
                       "WHERE session_id = ? ORDER BY verification_id DESC LIMIT 1"));
      q.bindValue(0, sessionId);
      if (q.exec() && q.next()) {
        result.insert(QStringLiteral("verified_at"), q.value(0).toString());
        result.insert(QStringLiteral("app_version"), q.value(1).toString());
        result.insert(QStringLiteral("verdict"), q.value(2).toString());
        result.insert(
          QStringLiteral("detail"),
          QJsonDocument::fromJson(q.value(3).toString().toUtf8()).object().toVariantMap());
      }

      db.close();
    }
  }

  QSqlDatabase::removeDatabase(connName);
  return result;
}

/**
 * @brief Returns a session_id -> verdict map with the latest stored verdict per session, in a
 *        single query so the session list can badge rows without per-row lookups.
 */
QVariantMap Sessions::DatabaseManager::latestVerdicts() const
{
  QVariantMap result;
  if (!m_open || m_filePath.isEmpty())
    return result;

  const QString connName = QStringLiteral("ss_dbm_verdicts_read");
  {
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    db.setDatabaseName(m_filePath);
    db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
    if (db.open()) {
      QSqlQuery q(db);
      const bool ok = q.exec(QStringLiteral(
        "SELECT v.session_id, v.verdict FROM verifications v JOIN "
        "(SELECT session_id, MAX(verification_id) AS latest_id FROM verifications "
        " GROUP BY session_id) latest "
        "ON v.session_id = latest.session_id AND v.verification_id = latest.latest_id"));
      while (ok && q.next())
        result.insert(q.value(0).toString(), q.value(1).toString());

      db.close();
    }
  }

  QSqlDatabase::removeDatabase(connName);
  return result;
}

/**
 * @brief Republishes the verifier's state through the historian's own property and signal
 *        surface, which QML and the API server were already bound to. A concluded
 *        verification also refreshes the session list: the child appended the record.
 */
void Sessions::DatabaseManager::wireVerifier()
{
  connect(&m_verifier,
          &ReproducibilityVerifier::busyChanged,
          this,
          &DatabaseManager::verificationBusyChanged);
  connect(&m_verifier,
          &ReproducibilityVerifier::regressionBusyChanged,
          this,
          &DatabaseManager::regressionBusyChanged);
  connect(&m_verifier,
          &ReproducibilityVerifier::reportChanged,
          this,
          &DatabaseManager::regressionReportChanged);
  connect(&m_verifier,
          &ReproducibilityVerifier::sweepChanged,
          this,
          &DatabaseManager::regressionSweepChanged);
  connect(&m_verifier,
          &ReproducibilityVerifier::regressionFinished,
          this,
          &DatabaseManager::regressionFinished);
  connect(&m_verifier,
          &ReproducibilityVerifier::verificationFinished,
          this,
          [this](int sessionId, bool success, const QVariantMap& verdict) {
            Q_EMIT verificationFinished(sessionId, success, verdict);
            Q_EMIT sessionsChanged();
          });
}

/**
 * @brief Runs the spec-0044 reproducibility check for @p sessionId (-1 = latest completed).
 */
void Sessions::DatabaseManager::verifySession(int sessionId)
{
  SS_ASSERT(m_open, return);
  SS_ASSERT(!m_filePath.isEmpty(), return);

  m_verifier.verify(sessionId);
}

/**
 * @brief Runs the spec-0047 regression check for @p sessionId against @p candidatePath, or
 *        against the currently open project when the path is empty.
 */
bool Sessions::DatabaseManager::regressSession(int sessionId, const QString& candidatePath)
{
  SS_ASSERT(m_open, return false);
  SS_ASSERT(!m_filePath.isEmpty(), return false);

  return m_verifier.regress(sessionId, candidatePath);
}

/**
 * @brief Runs a golden-tag regression sweep over every completed session carrying @p tag.
 */
bool Sessions::DatabaseManager::regressSessionsByTag(const QString& tag,
                                                     const QString& candidatePath)
{
  SS_ASSERT(m_open, return false);
  SS_ASSERT(!m_filePath.isEmpty(), return false);

  return m_verifier.regressByTag(tag, candidatePath);
}

/**
 * @brief Returns the sweep state for the API poll surface.
 */
QVariantMap Sessions::DatabaseManager::regressionSweepStatus() const
{
  return m_verifier.sweepStatus();
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to pick a .db file from the SQLite workspace folder.
 */
void Sessions::DatabaseManager::openDatabase()
{
  SS_ASSERT(m_workspaceManager != nullptr, return);

  auto* dialog = new QFileDialog(qApp->activeWindow(),
                                 tr("Open Session File"),
                                 m_workspaceManager->path("Session Databases"),
                                 tr("Session files (*.db)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { openDatabase(path); }, Qt::QueuedConnection);
  });
  dialog->open();
}

/**
 * @brief Dispatches an asynchronous open + initial cache fetch to the worker.
 */
void Sessions::DatabaseManager::openDatabase(const QString& filePath)
{
  if (filePath.isEmpty())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(
    m_worker, "openDatabase", Qt::QueuedConnection, Q_ARG(QString, filePath));
}

/**
 * @brief Dispatches an async close to the worker; main-thread cache clears on reply.
 */
void Sessions::DatabaseManager::closeDatabase(bool clearSavedPath)
{
  if (m_worker)
    m_worker->requestCancel();

  if (clearSavedPath)
    m_settings.remove("Sessions/LastDatabase");

  if (m_worker)
    QMetaObject::invokeMethod(m_worker, "closeDatabase", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Lock / unlock
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a password, hashes it, and dispatches the lock write.
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

  const QString hash = Misc::PasswordHash::hashPassword(first);

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "persistLock",
                            Qt::QueuedConnection,
                            Q_ARG(QString, hash),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Prompts for the password, verifies it locally, and dispatches the unlock write.
 */
void Sessions::DatabaseManager::unlockDatabase()
{
  if (!isOpen() || !m_locked)
    return;

  if (m_passwordHash.isEmpty()) {
    setBusy(true);
    QMetaObject::invokeMethod(m_worker,
                              "persistLock",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QString()),
                              Q_ARG(quint64, nextToken()));
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

  if (!Misc::PasswordHash::verifyPassword(pwd, m_passwordHash)) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Incorrect password"),
        tr("The password you entered does not match the one stored in the session file."),
        QMessageBox::Warning);
    });
    return;
  }

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "persistLock",
                            Qt::QueuedConnection,
                            Q_ARG(QString, QString()),
                            Q_ARG(quint64, nextToken()));
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
 * @brief Updates the cached notes locally and dispatches the SQL update.
 */
void Sessions::DatabaseManager::setSelectedSessionNotes(const QString& notes)
{
  if (m_selectedSessionId < 0 || !isOpen())
    return;

  if (selectedSessionNotes() == notes)
    return;

  for (auto& v : m_sessionList) {
    auto m = v.toMap();
    if (m.value("session_id").toInt() == m_selectedSessionId) {
      m["notes"] = notes;
      v          = m;
      break;
    }
  }

  Q_EMIT sessionsChanged();
  Q_EMIT selectedSessionChanged();

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "setSessionNotes",
                            Qt::QueuedConnection,
                            Q_ARG(int, m_selectedSessionId),
                            Q_ARG(QString, notes),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches an async cascade-delete to the worker.
 */
void Sessions::DatabaseManager::deleteSession(int sessionId)
{
  SS_ASSERT(sessionId >= 0, return);

  if (!isOpen() || m_locked)
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "deleteSession",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(quint64, nextToken()));

  if (m_selectedSessionId == sessionId) {
    m_selectedSessionId = -1;
    Q_EMIT selectedSessionChanged();
  }
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
 * @brief Hands the selected session off to the SQLite player for replay.
 */
bool Sessions::DatabaseManager::replaySelectedSession()
{
  if (m_selectedSessionId < 0 || m_filePath.isEmpty())
    return false;

  SS_ASSERT(m_player != nullptr, return false);
  m_player->openFile(m_filePath, m_selectedSessionId);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Tag CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches a tag insert to the worker.
 */
void Sessions::DatabaseManager::addTag(const QString& label)
{
  if (!isOpen() || label.trimmed().isEmpty())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(
    m_worker, "addTag", Qt::QueuedConnection, Q_ARG(QString, label), Q_ARG(quint64, nextToken()));
}

/**
 * @brief Inserts a tag if needed and assigns it to a session in one worker operation.
 */
void Sessions::DatabaseManager::addTagAndAssign(int sessionId, const QString& label)
{
  if (!isOpen() || label.trimmed().isEmpty())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "addTagAndAssign",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(QString, label),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a tag delete to the worker.
 */
void Sessions::DatabaseManager::deleteTag(int tagId)
{
  if (!isOpen() || m_locked)
    return;

  setBusy(true);
  QMetaObject::invokeMethod(
    m_worker, "deleteTag", Qt::QueuedConnection, Q_ARG(int, tagId), Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a tag rename to the worker.
 */
void Sessions::DatabaseManager::renameTag(int tagId, const QString& newLabel)
{
  if (!isOpen() || newLabel.trimmed().isEmpty())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "renameTag",
                            Qt::QueuedConnection,
                            Q_ARG(int, tagId),
                            Q_ARG(QString, newLabel),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a session/tag association insert to the worker.
 */
void Sessions::DatabaseManager::assignTagToSession(int sessionId, int tagId)
{
  if (!isOpen())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "assignTag",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(int, tagId),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a session/tag association delete to the worker.
 */
void Sessions::DatabaseManager::removeTagFromSession(int sessionId, int tagId)
{
  if (!isOpen())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "unassignTag",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(int, tagId),
                            Q_ARG(quint64, nextToken()));
}

//--------------------------------------------------------------------------------------------------
// Export dispatch (SessionExporter owns the flows)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Republishes the exporter's state through the historian's own property and signal surface,
 *        which QML and the API server were already bound to.
 */
void Sessions::DatabaseManager::wireExporter()
{
  connect(
    &m_exporter, &SessionExporter::csvBusyChanged, this, &DatabaseManager::csvExportBusyChanged);
  connect(&m_exporter,
          &SessionExporter::csvProgressChanged,
          this,
          &DatabaseManager::csvExportProgressChanged);
  connect(&m_exporter, &SessionExporter::csvFinished, this, &DatabaseManager::csvExportFinished);
  connect(
    &m_exporter, &SessionExporter::pdfBusyChanged, this, &DatabaseManager::pdfExportBusyChanged);
  connect(&m_exporter,
          &SessionExporter::pdfProgressChanged,
          this,
          &DatabaseManager::pdfExportProgressChanged);
  connect(&m_exporter, &SessionExporter::pdfFinished, this, &DatabaseManager::pdfExportFinished);
  connect(&m_exporter, &SessionExporter::logoPicked, this, &DatabaseManager::reportLogoPicked);
}

/**
 * @brief Returns the cached project title of @p sessionId, the name every export path derives its
 *        suggested output folder from.
 */
QString Sessions::DatabaseManager::sessionProjectTitle(int sessionId) const
{
  return sessionMetadata(sessionId).value("project_title").toString();
}

/**
 * @brief Exports a session to CSV; the flow belongs to the exporter.
 */
void Sessions::DatabaseManager::exportSessionToCsv(int sessionId)
{
  if (!isOpen())
    return;

  m_exporter.exportToCsv(sessionId, sessionProjectTitle(sessionId));
}

/**
 * @brief Exports a session to a PDF/HTML report; the flow belongs to the exporter.
 */
void Sessions::DatabaseManager::exportSessionToPdf(int sessionId, const QVariantMap& options)
{
  if (!isOpen())
    return;

  m_exporter.exportToPdf(sessionId, options, sessionProjectTitle(sessionId));
}

/**
 * @brief Asks for a session's dataset list, for the export selection UI.
 */
void Sessions::DatabaseManager::requestSessionDatasets(int sessionId)
{
  if (!isOpen())
    return;

  m_exporter.requestDatasets(sessionId);
}

/**
 * @brief Asks for a summary of a session's recorded stream data (spec 0054).
 */
void Sessions::DatabaseManager::requestStreamStats(int sessionId)
{
  if (!isOpen())
    return;

  m_exporter.requestStreamStats(sessionId);
}

/**
 * @brief Opens the report logo picker; the dialog belongs to the exporter.
 */
void Sessions::DatabaseManager::pickReportLogo()
{
  m_exporter.pickReportLogo();
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

  SS_ASSERT(m_projectModel != nullptr, return);

  const auto json = QJsonDocument(m_projectModel->serializeToJson()).toJson(QJsonDocument::Compact);

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "storeProjectMetadata",
                            Qt::QueuedConnection,
                            Q_ARG(QString, QString::fromUtf8(json)),
                            Q_ARG(QString, m_projectModel->title()),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches the JSON fetch to the worker; reply continues in
 * onWorkerGlobalProjectJsonReady.
 */
void Sessions::DatabaseManager::restoreProjectFromDb()
{
  if (!isOpen())
    return;

  QMetaObject::invokeMethod(m_worker, "fetchGlobalProjectJson", Qt::QueuedConnection);
}

/**
 * @brief Worker handed back the global project JSON -- finish the restore on the main thread.
 */
void Sessions::DatabaseManager::runRestoreProjectFromJson(const QString& json)
{
  SS_ASSERT(m_projectModel != nullptr, return);
  SS_ASSERT(m_appState != nullptr, return);

  if (json.isEmpty()) {
    Misc::Utilities::showMessageBox(tr("No project data"),
                                    tr("This session file does not contain an embedded project."),
                                    QMessageBox::Warning);
    return;
  }

  QJsonParseError parseError{};
  const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || doc.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("Invalid project data"),
      tr("The embedded project JSON is malformed and cannot be restored."),
      QMessageBox::Critical);
    return;
  }

  const auto title       = doc.object().value("title").toString("Restored Project");
  const auto projectsDir = m_projectModel->jsonProjectsPath();
  const auto suggested   = QStringLiteral("%1/%2.ssproj").arg(projectsDir, title);

  const auto path = QFileDialog::getSaveFileName(
    nullptr, tr("Restore Project"), suggested, tr("Serial Studio projects (*.ssproj *.json)"));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(
      tr("Cannot write file"), tr("Check file permissions and try again."), QMessageBox::Critical);
    return;
  }

  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  m_appState->setOperationMode(SerialStudio::ProjectFile);
  m_projectModel->openJsonFile(path);

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

  if (!QFileInfo::exists(path)) {
    m_settings.remove("Sessions/LastDatabase");
    return;
  }

  openDatabase(path);
}

//--------------------------------------------------------------------------------------------------
// Refresh + helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Triggers a full cache refresh on the worker.
 */
void Sessions::DatabaseManager::refreshSessionList()
{
  if (!isOpen())
    return;

  QMetaObject::invokeMethod(m_worker, "refreshAll", Qt::QueuedConnection);
}

/**
 * @brief Returns the next mutation token used to correlate worker replies.
 */
quint64 Sessions::DatabaseManager::nextToken()
{
  return ++m_nextToken;
}

/**
 * @brief Toggles the busy counter and emits busyChanged on transitions.
 */
void Sessions::DatabaseManager::setBusy(bool busy)
{
  const bool was = m_outstandingMutations > 0;
  if (busy)
    ++m_outstandingMutations;
  else if (m_outstandingMutations > 0)
    --m_outstandingMutations;

  const bool now = m_outstandingMutations > 0;
  if (was != now)
    Q_EMIT busyChanged();
}

//--------------------------------------------------------------------------------------------------
// Worker reply slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Worker finished opening the file -- populate caches and emit signals.
 */
void Sessions::DatabaseManager::onWorkerOpened(const QString& filePath,
                                               const QVariantList& sessionList,
                                               const QVariantList& tagList,
                                               bool locked,
                                               const QString& passwordHash)
{
  m_filePath = filePath;
  m_open     = true;
  m_verifier.setArchive(filePath);
  m_sessionList     = sessionList;
  m_tagList         = tagList;
  const bool wasLck = m_locked;
  m_locked          = locked;
  m_passwordHash    = passwordHash;

  m_settings.setValue("Sessions/LastDatabase", filePath);

  setBusy(false);

  Q_EMIT openChanged();
  Q_EMIT sessionsChanged();
  Q_EMIT tagsChanged();
  Q_EMIT selectedSessionChanged();
  if (m_locked != wasLck)
    Q_EMIT lockedChanged();
}

/**
 * @brief Worker reported an open failure -- surface it to the user.
 */
void Sessions::DatabaseManager::onWorkerOpenFailed(const QString& filePath, const QString& error)
{
  Q_UNUSED(filePath)
  setBusy(false);

  Misc::Utilities::showMessageBox(tr("Cannot open session file"), error, QMessageBox::Critical);
}

/**
 * @brief Worker confirmed the file is closed -- clear caches and notify QML.
 */
void Sessions::DatabaseManager::onWorkerClosed()
{
  const bool wasOpen   = m_open;
  const bool wasLocked = m_locked;

  m_open = false;
  m_filePath.clear();
  m_verifier.setArchive(QString());
  m_sessionList.clear();
  m_tagList.clear();
  m_passwordHash.clear();
  m_locked            = false;
  m_selectedSessionId = -1;

  if (wasOpen)
    Q_EMIT openChanged();

  Q_EMIT sessionsChanged();
  Q_EMIT tagsChanged();
  Q_EMIT selectedSessionChanged();
  if (wasLocked)
    Q_EMIT lockedChanged();
}

/**
 * @brief Worker shipped a refreshed session list -- rebroadcast to QML.
 */
void Sessions::DatabaseManager::onWorkerSessionListRefreshed(const QVariantList& sessionList)
{
  m_sessionList = sessionList;
  Q_EMIT sessionsChanged();
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Worker shipped a refreshed tag list -- rebroadcast to QML.
 */
void Sessions::DatabaseManager::onWorkerTagListRefreshed(const QVariantList& tagList)
{
  m_tagList = tagList;
  Q_EMIT tagsChanged();
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Worker reported a lock-state change -- update local mirror.
 */
void Sessions::DatabaseManager::onWorkerLockStateChanged(bool locked, const QString& passwordHash)
{
  const bool was = m_locked;
  m_locked       = locked;
  m_passwordHash = passwordHash;
  if (was != locked)
    Q_EMIT lockedChanged();
}

/**
 * @brief Worker confirmed the notes write -- propagate to any QML waiting on signal.
 */
void Sessions::DatabaseManager::onWorkerNotesUpdated(int sessionId, const QString& notes)
{
  Q_UNUSED(notes)
  if (sessionId == m_selectedSessionId)
    Q_EMIT selectedSessionChanged();
}

/**
 * @brief Worker finished a mutation -- clear the busy bit and report errors.
 */
void Sessions::DatabaseManager::onWorkerMutationFinished(quint64 token,
                                                         bool ok,
                                                         const QString& error)
{
  Q_UNUSED(token)
  setBusy(false);

  if (!ok && !error.isEmpty())
    qWarning() << "[Sessions] mutation failed:" << error;
}

/**
 * @brief Worker handed back the global project JSON -- finish the restore flow.
 */
void Sessions::DatabaseManager::onWorkerGlobalProjectJsonReady(const QString& json)
{
  runRestoreProjectFromJson(json);
}

//--------------------------------------------------------------------------------------------------
// Schema entry points (shared with Sessions::Export and the verifier child)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates or upgrades the session-log schema on the open database.
 */
void Sessions::DatabaseManager::createSchema(QSqlQuery& q)
{
  DatabaseSchema::createAll(q, kUserVersion);
}

/**
 * @brief Creates the append-only verification-record table (spec 0044).
 */
void Sessions::DatabaseManager::createSchemaVerifications(QSqlQuery& q)
{
  DatabaseSchema::createVerifications(q);
}

#endif  // BUILD_COMMERCIAL
