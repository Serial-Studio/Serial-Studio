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
#  include <QDir>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QInputDialog>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QJsonParseError>
#  include <QProcess>
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
#  include "Sessions/Export.h"
#  include "Sessions/HtmlReport.h"
#  include "Sessions/Player.h"
#  include "Sessions/ReportData.h"
#  include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// File-local state
//--------------------------------------------------------------------------------------------------

static QString s_dbPathOverride;

static constexpr int kStderrTailBytes = 2000;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

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

  int keep = 0;
  for (int i = safe.size(); i > 0; --i) {
    const QChar c = safe.at(i - 1);
    if (c != QChar('.') && c != QChar(' ')) {
      keep = i;
      break;
    }
  }
  safe.truncate(keep);

  if (safe.isEmpty())
    safe = QStringLiteral("Untitled");

  return safe;
}

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
  , m_csvExportBusy(false)
  , m_csvExportProgress(0.0)
  , m_pdfExportBusy(false)
  , m_pdfExportProgress(0.0)
  , m_pendingPdfSessionId(-1)
  , m_pendingPdfActive(false)
  , m_verifyProcess(nullptr)
  , m_regressActive(false)
  , m_sweepActive(false)
  , m_sweepOwnsCandidate(false)
  , m_nextToken(1)
  , m_outstandingMutations(0)
  , m_workspaceManager(nullptr)
  , m_player(nullptr)
  , m_projectModel(nullptr)
  , m_appState(nullptr)
{
  qRegisterMetaType<Sessions::ReportPayloadPtr>("Sessions::ReportPayloadPtr");
  initWorker();
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
  connect(
    m_worker, &DatabaseWorker::csvExportProgress, this, &DatabaseManager::onWorkerCsvProgress);
  connect(
    m_worker, &DatabaseWorker::csvExportFinished, this, &DatabaseManager::onWorkerCsvFinished);
  connect(
    m_worker, &DatabaseWorker::reportDataReady, this, &DatabaseManager::onWorkerReportDataReady);
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
  m_sweepActive = false;
  m_sweepQueue.clear();

  if (m_verifyProcess) {
    m_verifyProcess->terminate();
    if (!m_verifyProcess->waitForFinished(3000))
      m_verifyProcess->kill();
  }

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
 * @brief Returns the active PDF export's progress as a fraction in [0, 1].
 */
double Sessions::DatabaseManager::pdfExportProgress() const
{
  return m_pdfExportProgress;
}

/**
 * @brief Returns the active CSV export's progress as a fraction in [0, 1].
 */
double Sessions::DatabaseManager::csvExportProgress() const
{
  return m_csvExportProgress;
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

  const QString safeTitle       = sanitiseTitleForPath(projectTitle);
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
  return m_verifyProcess != nullptr;
}

/**
 * @brief Returns true while the shared child slot is running a regression pass (spec 0047).
 */
bool Sessions::DatabaseManager::regressionBusy() const
{
  return m_verifyProcess != nullptr && m_regressActive;
}

/**
 * @brief Returns the last regression report; ephemeral, cleared when a new pass starts.
 */
QVariantMap Sessions::DatabaseManager::lastRegressionReport() const
{
  return m_lastRegressionReport;
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
 * @brief Builds the structured report delivered when a verification or regression child
 *        process fails outside its own reporting (spawn failure, crash, unparseable output),
 *        so no failure path ever concludes with an empty verdict map.
 */
static QVariantMap childFailureReport(const QString& code,
                                      int exitCode,
                                      const QByteArray& stderrData)
{
  static const struct {
    const char* code;
    const char* error;
    const char* hint;
  } kFailures[] = {
    {  "child-spawn-failed",
     "The check could not be started.",                                               "Restart Serial Studio and try again."        },
    {       "child-crashed",
     "The check stopped unexpectedly.", "Try again. If this keeps happening, please report it and include the session file."        },
    {"child-output-invalid",
     "The check did not return a readable result.",                                                "Try again. If this keeps happening, other software on this computer may be "
                                                "interfering with Serial Studio."},
  };

  QVariantMap map;
  map.insert(QStringLiteral("verdict"), QStringLiteral("error"));
  map.insert(QStringLiteral("errorCode"), code);
  map.insert(QStringLiteral("exitCode"), exitCode);
  for (const auto& entry : kFailures) {
    if (code == QLatin1String(entry.code)) {
      map.insert(QStringLiteral("error"), QString::fromLatin1(entry.error));
      map.insert(QStringLiteral("hint"), QString::fromLatin1(entry.hint));
      break;
    }
  }

  const QString tail = QString::fromUtf8(stderrData.right(kStderrTailBytes)).trimmed();
  if (!tail.isEmpty())
    map.insert(QStringLiteral("stderrTail"), tail);

  return map;
}

/**
 * @brief Launches the spec-0044 verifier child process for @p sessionId (-1 = latest completed).
 *        The child appends the verification record itself; this side only parses the verdict
 *        JSON from stdout and refreshes the UI. One verification runs at a time.
 */
void Sessions::DatabaseManager::verifySession(int sessionId)
{
  SS_ASSERT(m_open, return);
  SS_ASSERT(!m_filePath.isEmpty(), return);

  if (m_verifyProcess || m_sweepActive)
    return;

  auto* process   = new QProcess(this);
  m_verifyProcess = process;
  Q_EMIT verificationBusyChanged();

  QStringList args{QStringLiteral("--verify-session"), m_filePath, QStringLiteral("--headless")};
  if (sessionId >= 0)
    args << QStringLiteral("--verify-session-id") << QString::number(sessionId);

  connect(process,
          &QProcess::finished,
          this,
          [this, process, sessionId](int exitCode, QProcess::ExitStatus status) {
            const auto doc = QJsonDocument::fromJson(process->readAllStandardOutput());
            const bool ok  = status == QProcess::NormalExit && doc.isObject();
            auto map       = doc.object().toVariantMap();
            if (!ok)
              map = childFailureReport(status != QProcess::NormalExit
                                         ? QStringLiteral("child-crashed")
                                         : QStringLiteral("child-output-invalid"),
                                       exitCode,
                                       process->readAllStandardError());

            int resolvedId = sessionId;
            if (resolvedId < 0 && map.contains(QStringLiteral("sessionId")))
              resolvedId = map.value(QStringLiteral("sessionId")).toInt();

            concludeVerification(resolvedId, ok && exitCode == 0, map);
          });
  connect(process, &QProcess::errorOccurred, this, [this, sessionId](QProcess::ProcessError err) {
    if (err == QProcess::FailedToStart)
      concludeVerification(
        sessionId,
        false,
        childFailureReport(QStringLiteral("child-spawn-failed"), -1, QByteArray()));
  });

  process->start(QCoreApplication::applicationFilePath(), args);
}

/**
 * @brief Tears the verification child process down and publishes the verdict exactly once.
 */
void Sessions::DatabaseManager::concludeVerification(int sessionId,
                                                     bool success,
                                                     const QVariantMap& verdict)
{
  if (!m_verifyProcess)
    return;

  m_verifyProcess->deleteLater();
  m_verifyProcess = nullptr;
  Q_EMIT verificationBusyChanged();
  Q_EMIT verificationFinished(sessionId, success, verdict);
  Q_EMIT sessionsChanged();
}

/**
 * @brief Publishes a regression start failure through the ephemeral report channel so the
 *        QML/API surfaces never see a silent no-op, then returns false for the caller.
 */
bool Sessions::DatabaseManager::publishRegressStartFailure(int sessionId,
                                                           const QString& code,
                                                           const QString& error,
                                                           const QString& hint)
{
  QVariantMap map;
  map.insert(QStringLiteral("verdict"), QStringLiteral("error"));
  map.insert(QStringLiteral("errorCode"), code);
  map.insert(QStringLiteral("error"), error);
  map.insert(QStringLiteral("hint"), hint);

  m_lastRegressionReport = map;
  Q_EMIT regressionReportChanged();
  Q_EMIT regressionFinished(sessionId, false, map);
  return false;
}

/**
 * @brief Serializes the currently open project to @p path; removes the partial file and
 *        returns false when the write cannot complete.
 */
static bool writeCandidateSnapshot(const QString& path)
{
  static auto& project = DataModel::ProjectModel::instance();
  const auto json      = QJsonDocument(project.serializeToJson()).toJson(QJsonDocument::Compact);

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) || file.write(json) != json.size()) {
    file.close();
    QFile::remove(path);
    return false;
  }

  return true;
}

/**
 * @brief Launches the spec-0047 regression child for @p sessionId against @p candidatePath,
 *        or against the currently open project (serialized to a temporary file) when the path
 *        is empty. Shares the single child slot with verification: one pass at a time.
 *        Returns false (with a published error report) when the pass did not start.
 */
bool Sessions::DatabaseManager::regressSession(int sessionId, const QString& candidatePath)
{
  SS_ASSERT(m_open, return false);
  SS_ASSERT(!m_filePath.isEmpty(), return false);

  if (m_verifyProcess)
    return false;

  QString candidate = candidatePath;
  m_regressCandidateTemp.clear();
  if (candidate.isEmpty()) {
    candidate = QDir::temp().filePath(
      QStringLiteral("ss-regress-candidate-%1.json").arg(QCoreApplication::applicationPid()));
    if (!writeCandidateSnapshot(candidate))
      return publishRegressStartFailure(
        sessionId,
        QStringLiteral("candidate-write-failed"),
        QStringLiteral("The current project could not be saved for comparison."),
        QStringLiteral("Check that there is enough free disk space, then try again."));

    m_regressCandidateTemp = candidate;
  }

  auto* process   = new QProcess(this);
  m_verifyProcess = process;
  m_regressActive = true;
  m_lastRegressionReport.clear();
  Q_EMIT verificationBusyChanged();
  Q_EMIT regressionBusyChanged();
  Q_EMIT regressionReportChanged();

  QStringList args{QStringLiteral("--regress-session"),
                   m_filePath,
                   QStringLiteral("--headless"),
                   QStringLiteral("--regress-project"),
                   candidate};
  if (sessionId >= 0)
    args << QStringLiteral("--regress-session-id") << QString::number(sessionId);

  connect(process,
          &QProcess::finished,
          this,
          [this, process, sessionId](int exitCode, QProcess::ExitStatus status) {
            const auto doc = QJsonDocument::fromJson(process->readAllStandardOutput());
            const bool ok  = status == QProcess::NormalExit && doc.isObject();
            auto map       = doc.object().toVariantMap();
            if (!ok)
              map = childFailureReport(status != QProcess::NormalExit
                                         ? QStringLiteral("child-crashed")
                                         : QStringLiteral("child-output-invalid"),
                                       exitCode,
                                       process->readAllStandardError());

            int resolvedId = sessionId;
            if (resolvedId < 0 && map.contains(QStringLiteral("sessionId")))
              resolvedId = map.value(QStringLiteral("sessionId")).toInt();

            concludeRegression(resolvedId, ok && exitCode == 0, map);
          });
  connect(process, &QProcess::errorOccurred, this, [this, sessionId](QProcess::ProcessError err) {
    if (err != QProcess::FailedToStart)
      return;

    QMetaObject::invokeMethod(
      this,
      [this, sessionId]() {
        concludeRegression(
          sessionId,
          false,
          childFailureReport(QStringLiteral("child-spawn-failed"), -1, QByteArray()));
      },
      Qt::QueuedConnection);
  });

  process->start(QCoreApplication::applicationFilePath(), args);
  return true;
}

/**
 * @brief Publishes a finished regression pass: releases the shared child slot, removes the
 *        auto-serialized candidate file, and stores the ephemeral report (never the archive).
 */
void Sessions::DatabaseManager::concludeRegression(int sessionId,
                                                   bool success,
                                                   const QVariantMap& report)
{
  if (!m_verifyProcess)
    return;

  m_verifyProcess->deleteLater();
  m_verifyProcess = nullptr;
  m_regressActive = false;

  if (!m_regressCandidateTemp.isEmpty()) {
    QFile::remove(m_regressCandidateTemp);
    m_regressCandidateTemp.clear();
  }

  m_lastRegressionReport = report;
  if (m_sweepActive)
    advanceRegressionSweep(sessionId, report);

  Q_EMIT verificationBusyChanged();
  Q_EMIT regressionBusyChanged();
  Q_EMIT regressionReportChanged();
  Q_EMIT regressionFinished(sessionId, success, report);
}

/**
 * @brief Starts a golden-tag regression sweep: tagged completed sessions run through
 *        regressSession() sequentially against one shared candidate snapshot. Returns false
 *        (with a published error report) when the sweep did not start; state is only touched
 *        once the candidate is secured, so a failed start never destroys prior results.
 */
bool Sessions::DatabaseManager::regressSessionsByTag(const QString& tag,
                                                     const QString& candidatePath)
{
  SS_ASSERT(m_open, return false);

  if (m_verifyProcess || m_sweepActive || tag.trimmed().isEmpty())
    return false;

  bool queryOk = false;
  QList<int> ids;
  const QString connName = QStringLiteral("ss_dbm_sweep_read");
  {
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    db.setDatabaseName(m_filePath);
    db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
    if (db.open()) {
      QSqlQuery q(db);
      q.prepare(QStringLiteral("SELECT s.session_id FROM sessions s "
                               "JOIN session_tags st ON st.session_id = s.session_id "
                               "JOIN tags t ON t.tag_id = st.tag_id "
                               "WHERE t.label = ? AND s.ended_at IS NOT NULL "
                               "ORDER BY s.session_id"));
      q.bindValue(0, tag.trimmed());
      queryOk = q.exec();
      while (queryOk && q.next())
        ids.append(q.value(0).toInt());

      db.close();
    }
  }

  QSqlDatabase::removeDatabase(connName);

  if (!queryOk)
    return publishRegressStartFailure(-1,
                                      QStringLiteral("sweep-query-failed"),
                                      QStringLiteral("The tagged sessions could not be listed."),
                                      QStringLiteral("Reopen the session file and try again."));

  QString candidate   = candidatePath;
  bool owns_candidate = false;
  if (candidate.isEmpty() && !ids.isEmpty()) {
    candidate = QDir::temp().filePath(
      QStringLiteral("ss-regress-sweep-%1.json").arg(QCoreApplication::applicationPid()));
    if (!writeCandidateSnapshot(candidate))
      return publishRegressStartFailure(
        -1,
        QStringLiteral("candidate-write-failed"),
        QStringLiteral("The current project could not be saved for comparison."),
        QStringLiteral("Check that there is enough free disk space, then try again."));

    owns_candidate = true;
  }

  m_sweepTag           = tag.trimmed();
  m_sweepReports       = QVariantList();
  m_sweepOwnsCandidate = owns_candidate;
  m_sweepCandidate     = candidate;
  m_sweepQueue         = ids;
  m_sweepActive        = true;
  Q_EMIT regressionSweepChanged();

  if (m_sweepQueue.isEmpty()) {
    finishRegressionSweep();
    return true;
  }

  const int first = m_sweepQueue.takeFirst();
  if (!regressSession(first, m_sweepCandidate)) {
    finishRegressionSweep();
    return false;
  }

  return true;
}

/**
 * @brief Records one finished sweep entry and launches the next session, or finalizes when
 *        the queue is drained or the database closed mid-sweep.
 */
void Sessions::DatabaseManager::advanceRegressionSweep(int sessionId, const QVariantMap& report)
{
  QVariantMap entry;
  entry.insert(QStringLiteral("sessionId"), sessionId);
  entry.insert(QStringLiteral("report"), report);
  m_sweepReports.append(entry);

  if (m_sweepQueue.isEmpty() || !m_open) {
    finishRegressionSweep();
    return;
  }

  const int next = m_sweepQueue.takeFirst();
  if (!regressSession(next, m_sweepCandidate))
    finishRegressionSweep();
}

/**
 * @brief Ends the sweep: removes the sweep-owned candidate snapshot and publishes the state.
 */
void Sessions::DatabaseManager::finishRegressionSweep()
{
  if (m_sweepOwnsCandidate && !m_sweepCandidate.isEmpty())
    QFile::remove(m_sweepCandidate);

  m_sweepActive        = false;
  m_sweepOwnsCandidate = false;
  m_sweepCandidate.clear();
  m_sweepQueue.clear();
  Q_EMIT regressionSweepChanged();
}

/**
 * @brief Buckets one sweep verdict into the aggregate counters.
 */
static void countSweepVerdict(
  const QString& verdict, int& passed, int& drifted, int& notVerifiable, int& failed)
{
  if (verdict == QLatin1String("identical"))
    ++passed;
  else if (verdict.endsWith(QLatin1String("-drift")))
    ++drifted;
  else if (verdict == QLatin1String("not_verifiable"))
    ++notVerifiable;
  else
    ++failed;
}

/**
 * @brief Returns the sweep state for the API poll surface: activity flag, tag, verdict
 *        counters derived from the collected reports, and the per-session reports.
 */
QVariantMap Sessions::DatabaseManager::regressionSweepStatus() const
{
  int passed = 0, drifted = 0, notVerifiable = 0, failed = 0;
  for (const auto& entryVariant : m_sweepReports) {
    const auto report = entryVariant.toMap().value(QStringLiteral("report")).toMap();
    countSweepVerdict(
      report.value(QStringLiteral("verdict")).toString(), passed, drifted, notVerifiable, failed);
  }

  QVariantMap summary;
  summary.insert(QStringLiteral("passed"), passed);
  summary.insert(QStringLiteral("drifted"), drifted);
  summary.insert(QStringLiteral("notVerifiable"), notVerifiable);
  summary.insert(QStringLiteral("failed"), failed);

  QVariantMap status;
  status.insert(QStringLiteral("active"), m_sweepActive);
  status.insert(QStringLiteral("tag"), m_sweepTag);
  status.insert(QStringLiteral("remaining"), m_sweepQueue.size());
  status.insert(QStringLiteral("summary"), summary);
  status.insert(QStringLiteral("reports"), m_sweepReports);
  return status;
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
// CSV export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks an output path and dispatches a streaming CSV export to the worker.
 */
void Sessions::DatabaseManager::exportSessionToCsv(int sessionId)
{
  if (!isOpen() || m_csvExportBusy)
    return;

  SS_ASSERT(m_workspaceManager != nullptr, return);

  const auto meta         = sessionMetadata(sessionId);
  const QString projTitle = meta.value("project_title").toString();
  const QString safeProj  = sanitiseTitleForPath(projTitle);
  const QString dir       = QStringLiteral("%1/%2").arg(m_workspaceManager->path("CSV"), safeProj);
  QDir().mkpath(dir);

  const QString suggested =
    QStringLiteral("%1/session_%2.csv").arg(dir, QString::number(sessionId));
  const auto path = QFileDialog::getSaveFileName(
    nullptr, tr("Export Session to CSV"), suggested, tr("CSV files (*.csv)"));
  if (path.isEmpty())
    return;

  m_csvExportBusy     = true;
  m_csvExportProgress = 0.0;
  m_pendingCsvPath    = path;
  Q_EMIT csvExportBusyChanged();
  Q_EMIT csvExportProgressChanged();

  QMetaObject::invokeMethod(
    m_worker, "runCsvExport", Qt::QueuedConnection, Q_ARG(int, sessionId), Q_ARG(QString, path));
}

//--------------------------------------------------------------------------------------------------
// PDF / HTML report export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Translates QML options + path picker into a worker fetch + main-thread render.
 */
void Sessions::DatabaseManager::exportSessionToPdf(int sessionId, const QVariantMap& options)
{
  if (!isOpen() || m_pdfExportBusy)
    return;

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
  opts.lineWidth           = SerialStudio::toDouble(options.value("lineWidth", 1.4));
  opts.lineStyle           = options.value("lineStyle", QStringLiteral("solid")).toString();

  const auto selList = options.value("selectedUniqueIds").toList();
  opts.selectedUniqueIds.reserve(selList.size());
  for (const auto& v : selList)
    opts.selectedUniqueIds.push_back(v.toInt());

  const QString fmtStr = options.value("outputFormat", QStringLiteral("pdf")).toString().toLower();
  if (fmtStr == QStringLiteral("html"))
    opts.format = HtmlReportOptions::Format::Html;
  else if (fmtStr == QStringLiteral("both"))
    opts.format = HtmlReportOptions::Format::Both;
  else
    opts.format = HtmlReportOptions::Format::Pdf;

#  ifndef SERIAL_STUDIO_WITH_WEBENGINE
  opts.format = HtmlReportOptions::Format::Html;
#  endif

  if (!opts.outputPath.isEmpty()) {
    launchPdfExport(sessionId, std::move(opts));
    return;
  }

  requestPdfOutputPath(sessionId, std::move(opts));
}

/**
 * @brief Stages PDF render context and asks the worker for the session payload.
 */
void Sessions::DatabaseManager::launchPdfExport(int sessionId, HtmlReportOptions opts)
{
  m_pendingPdfOpts      = std::move(opts);
  m_pendingPdfSessionId = sessionId;
  m_pendingPdfActive    = true;

  const bool reportBusy = (m_pendingPdfOpts.format != HtmlReportOptions::Format::Html);
  if (reportBusy) {
    m_pdfExportBusy     = true;
    m_pdfExportProgress = 0.0;
    m_pdfExportStatus   = tr("Loading session data…");
    Q_EMIT pdfExportBusyChanged();
    Q_EMIT pdfExportProgressChanged();
  }

  QVariantList selectedUniqueIds;
  selectedUniqueIds.reserve(static_cast<int>(m_pendingPdfOpts.selectedUniqueIds.size()));
  for (const int uid : m_pendingPdfOpts.selectedUniqueIds)
    selectedUniqueIds.append(uid);

  QMetaObject::invokeMethod(m_worker,
                            "runReportDataLoad",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(bool, m_pendingPdfOpts.includeCharts),
                            Q_ARG(int, 10000),
                            Q_ARG(QVariantList, selectedUniqueIds));
}

/**
 * @brief Asks the worker thread to enumerate a session's datasets for the selection UI.
 */
void Sessions::DatabaseManager::requestSessionDatasets(int sessionId)
{
  if (!isOpen())
    return;

  QMetaObject::invokeMethod(
    m_worker, "runDatasetListLoad", Qt::QueuedConnection, Q_ARG(int, sessionId));
}

/**
 * @brief Asks the worker thread to summarise a session's recorded stream data (spec 0054).
 */
void Sessions::DatabaseManager::requestStreamStats(int sessionId)
{
  if (!isOpen())
    return;

  QMetaObject::invokeMethod(
    m_worker, "runStreamStatsLoad", Qt::QueuedConnection, Q_ARG(int, sessionId));
}

/**
 * @brief Opens a Save dialog for the report path and launches the export on accept.
 */
void Sessions::DatabaseManager::requestPdfOutputPath(int sessionId, HtmlReportOptions opts)
{
  const bool wantsPdf  = (opts.format != HtmlReportOptions::Format::Html);
  const QString ext    = wantsPdf ? QStringLiteral("pdf") : QStringLiteral("html");
  const QString title  = wantsPdf ? tr("Save PDF Report") : tr("Save HTML Report");
  const QString filter = wantsPdf ? tr("PDF files (*.pdf)") : tr("HTML files (*.html)");

  SS_ASSERT(m_workspaceManager != nullptr, return);

  const auto meta         = sessionMetadata(sessionId);
  const QString projTitle = meta.value("project_title").toString();
  const QString safeProj  = sanitiseTitleForPath(projTitle);
  const QString dir = QStringLiteral("%1/%2").arg(m_workspaceManager->path("Reports"), safeProj);
  QDir().mkpath(dir);

  const QString baseName  = opts.documentTitle.isEmpty()
                            ? QStringLiteral("session_%1").arg(sessionId)
                            : sanitiseTitleForPath(opts.documentTitle);
  const QString suggested = QStringLiteral("%1/%2.%3").arg(dir, baseName, ext);

  auto* dialog = new QFileDialog(qApp->activeWindow(), title, suggested, filter);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setFileMode(QFileDialog::AnyFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog,
          &QFileDialog::fileSelected,
          this,
          [this, opts, ext, sessionId](const QString& path) mutable {
            if (path.isEmpty()) {
              Q_EMIT pdfExportFinished(QString(), false);
              return;
            }

            QMetaObject::invokeMethod(
              this,
              [this, opts = std::move(opts), ext, sessionId, path]() mutable {
                QString finalPath = path;
                const QString dot = QStringLiteral(".") + ext;
                if (!finalPath.endsWith(dot, Qt::CaseInsensitive))
                  finalPath += dot;

                opts.outputPath = finalPath;
                launchPdfExport(sessionId, std::move(opts));
              },
              Qt::QueuedConnection);
          });

  connect(
    dialog, &QFileDialog::rejected, this, [this] { Q_EMIT pdfExportFinished(QString(), false); });

  dialog->open();
}

/**
 * @brief Continues the PDF flow on the main thread once the worker has shipped data.
 */
void Sessions::DatabaseManager::renderReportFromPayload(const ReportPayloadPtr& payload)
{
  if (!m_pendingPdfActive)
    return;

  if (!payload || payload->sessionId != m_pendingPdfSessionId)
    return;

  if (!payload->ok) {
    if (m_pdfExportBusy) {
      m_pdfExportBusy     = false;
      m_pdfExportProgress = 0.0;
      m_pdfExportStatus   = tr("Failed");
      Q_EMIT pdfExportBusyChanged();
      Q_EMIT pdfExportProgressChanged();
    }

    Misc::Utilities::showMessageBox(tr("Report Failed"),
                                    payload->error.isEmpty() ? tr("Could not generate the report.")
                                                             : payload->error,
                                    QMessageBox::Warning);

    Q_EMIT pdfExportFinished(QString(), false);
    m_pendingPdfActive = false;
    return;
  }

  const bool reportBusy = (m_pendingPdfOpts.format != HtmlReportOptions::Format::Html);

  auto* renderer = new HtmlReport(this);

  if (reportBusy) {
    connect(renderer, &HtmlReport::progress, this, [this](const QString& status, double percent) {
      m_pdfExportStatus   = status;
      m_pdfExportProgress = percent;
      Q_EMIT pdfExportProgressChanged();
    });
  }

  connect(renderer,
          &HtmlReport::finished,
          this,
          [this, renderer, reportBusy](const QString& outputPath, bool ok, const QString& error) {
            if (reportBusy) {
              m_pdfExportBusy     = false;
              m_pdfExportProgress = 1.0;
              m_pdfExportStatus   = ok ? tr("Done") : tr("Failed");
              Q_EMIT pdfExportBusyChanged();
              Q_EMIT pdfExportProgressChanged();
            }
            Q_EMIT pdfExportFinished(outputPath, ok);

            if (ok) {
              Misc::Utilities::revealFile(outputPath);
            } else {
              Misc::Utilities::showMessageBox(tr("Report Failed"),
                                              error.isEmpty() ? tr("Could not generate the report.")
                                                              : error,
                                              QMessageBox::Warning);
            }

            m_pendingPdfActive = false;
            renderer->deleteLater();
          });

  renderer->render(payload->data, payload->series, m_pendingPdfOpts);
}

/**
 * @brief Opens a native QFileDialog to pick a logo for the report.
 */
void Sessions::DatabaseManager::pickReportLogo()
{
  auto* dialog = new QFileDialog(qApp->activeWindow(),
                                 tr("Select logo image"),
                                 QString(),
                                 tr("Images (*.png *.jpg *.jpeg *.svg)"));
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this, [this, path]() { Q_EMIT reportLogoPicked(path); }, Qt::QueuedConnection);
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
  m_filePath        = filePath;
  m_open            = true;
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
 * @brief Worker is streaming CSV -- update the percentage cache.
 */
void Sessions::DatabaseManager::onWorkerCsvProgress(double percent)
{
  m_csvExportProgress = percent;
  Q_EMIT csvExportProgressChanged();
}

/**
 * @brief Worker finished writing the CSV -- report status, reveal in the OS shell.
 */
void Sessions::DatabaseManager::onWorkerCsvFinished(const QString& outputPath,
                                                    bool ok,
                                                    const QString& error)
{
  Q_UNUSED(error)
  m_csvExportBusy     = false;
  m_csvExportProgress = ok ? 1.0 : 0.0;
  m_pendingCsvPath.clear();
  Q_EMIT csvExportBusyChanged();
  Q_EMIT csvExportProgressChanged();
  Q_EMIT csvExportFinished(outputPath, ok);

  if (ok)
    Misc::Utilities::revealFile(outputPath);
}

/**
 * @brief Worker shipped the report data bundle -- kick off rendering on the main thread.
 */
void Sessions::DatabaseManager::onWorkerReportDataReady(const ReportPayloadPtr& payload)
{
  renderReportFromPayload(payload);
}

/**
 * @brief Worker handed back the global project JSON -- finish the restore flow.
 */
void Sessions::DatabaseManager::onWorkerGlobalProjectJsonReady(const QString& json)
{
  runRestoreProjectFromJson(json);
}

//--------------------------------------------------------------------------------------------------
// Schema (shared with Sessions::Export at session creation time)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates or upgrades the session-log schema on the open database.
 */
void Sessions::DatabaseManager::createSchema(QSqlQuery& q)
{
  createSchemaSessionTables(q);
  migrateColumnsTable(q);
  migrateSessionsTable(q);
  createSchemaSampleTables(q);
  createSchemaStreamTables(q);
  createSchemaBlockTable(q);
  createSchemaTagTables(q);
  createSchemaProjectMetadata(q);
  createSchemaVerifications(q);
  q.exec(QStringLiteral("PRAGMA user_version = %1").arg(kUserVersion));
}

/**
 * @brief Creates the sessions header and columns metadata tables.
 */
void Sessions::DatabaseManager::createSchemaSessionTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS sessions ("
         "  session_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  project_title TEXT NOT NULL,"
         "  started_at    TEXT NOT NULL,"
         "  ended_at      TEXT,"
         "  project_json  TEXT,"
         "  notes         TEXT"
         ")");

  q.exec("CREATE TABLE IF NOT EXISTS columns ("
         "  column_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL REFERENCES sessions,"
         "  unique_id    INTEGER NOT NULL,"
         "  source_id    INTEGER NOT NULL DEFAULT 0,"
         "  source_title TEXT NOT NULL DEFAULT '',"
         "  group_title  TEXT NOT NULL,"
         "  title        TEXT NOT NULL,"
         "  units        TEXT,"
         "  widget       TEXT,"
         "  is_virtual   INTEGER NOT NULL DEFAULT 0"
         ")");
}

/**
 * @brief Adds source_id / source_title to legacy columns tables in older databases.
 */
void Sessions::DatabaseManager::migrateColumnsTable(QSqlQuery& q)
{
  auto columnExists = [&q](const QString& column) {
    if (!q.exec(QStringLiteral("PRAGMA table_info(\"columns\")"))) {
      qWarning() << "[Sessions] PRAGMA table_info failed:" << q.lastError().text();
      return false;
    }
    while (q.next())
      if (q.value(1).toString().compare(column, Qt::CaseInsensitive) == 0)
        return true;

    return false;
  };

  if (!columnExists(QStringLiteral("source_id"))) {
    if (!q.exec("ALTER TABLE \"columns\" ADD COLUMN source_id INTEGER NOT NULL DEFAULT 0"))
      qWarning() << "[Sessions] ALTER add source_id failed:" << q.lastError().text();
  }

  if (!columnExists(QStringLiteral("source_title"))) {
    if (!q.exec("ALTER TABLE \"columns\" ADD COLUMN source_title TEXT NOT NULL DEFAULT ''"))
      qWarning() << "[Sessions] ALTER add source_title failed:" << q.lastError().text();
  }
}

/**
 * @brief Adds the spec-0044 fingerprint/classification columns and the spec-0062 view-state
 *        bundle column to legacy sessions tables (nullable, so old archives keep reading).
 */
void Sessions::DatabaseManager::migrateSessionsTable(QSqlQuery& q)
{
  auto columnExists = [&q](const QString& column) {
    if (!q.exec(QStringLiteral("PRAGMA table_info(\"sessions\")"))) {
      qWarning() << "[Sessions] PRAGMA table_info failed:" << q.lastError().text();
      return false;
    }
    while (q.next())
      if (q.value(1).toString().compare(column, Qt::CaseInsensitive) == 0)
        return true;

    return false;
  };

  static constexpr struct {
    const char* name;
    const char* type;
  } kColumns[] = {
    {     "raw_sha256",    "TEXT"},
    {"readings_sha256",    "TEXT"},
    {    "app_version",    "TEXT"},
    { "capture_format", "INTEGER"},
    {    "repro_class",    "TEXT"},
    { "frames_dropped", "INTEGER"},
    { "overflow_bytes", "INTEGER"},
    {  "stream_sha256",    "TEXT"},
    {     "view_state",    "TEXT"},
  };

  for (const auto& col : kColumns) {
    if (columnExists(QLatin1String(col.name)))
      continue;

    const auto sql = QStringLiteral("ALTER TABLE \"sessions\" ADD COLUMN %1 %2")
                       .arg(QLatin1String(col.name), QLatin1String(col.type));
    if (!q.exec(sql))
      qWarning() << "[Sessions] ALTER add" << col.name << "failed:" << q.lastError().text();
  }
}

/**
 * @brief Creates the per-sample tables (readings, raw_bytes, table_snapshots) and indexes.
 */
void Sessions::DatabaseManager::createSchemaSampleTables(QSqlQuery& q)
{
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

  q.exec("CREATE TABLE IF NOT EXISTS raw_bytes ("
         "  raw_id       INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL,"
         "  timestamp_ns INTEGER NOT NULL,"
         "  device_id    INTEGER NOT NULL DEFAULT 0,"
         "  data         BLOB NOT NULL"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_raw_bytes_session_ts "
         "ON raw_bytes (session_id, timestamp_ns)");

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
}

/**
 * @brief Creates the full-rate stream-block table (spec 0054). One row per acquisition block
 *        per dataset: `samples` is `frames` IEEE-754 float64 values written little-endian, and
 *        `t0_ns` + `dt_ns` date each of them as t0 + i * dt, so the source keeps owning time.
 *        Additive only -- a v1 database gains the empty table and replays unchanged.
 */
void Sessions::DatabaseManager::createSchemaStreamTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS stream_blocks ("
         "  stream_block_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id      INTEGER NOT NULL,"
         "  source_id       INTEGER NOT NULL,"
         "  unique_id       INTEGER NOT NULL,"
         "  block_number    INTEGER NOT NULL,"
         "  t0_ns           INTEGER NOT NULL,"
         "  dt_ns           INTEGER NOT NULL,"
         "  frames          INTEGER NOT NULL,"
         "  samples         BLOB NOT NULL"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_stream_blocks_session_uid_t0 "
         "ON stream_blocks (session_id, unique_id, t0_ns)");
}

/**
 * @brief Creates the unified block table (spec 0055 R7): one row per dataset per published block,
 *        for BOTH lanes. Little-endian float64 values plus their pre-transform twin, length-
 *        prefixed UTF-8 texts, and explicit per-sample times when `dt_ns` is 0. Additive -- v1/v2
 *        databases keep `readings` and `stream_blocks` and still replay.
 */
void Sessions::DatabaseManager::createSchemaBlockTable(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS blocks ("
         "  block_id     INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL,"
         "  source_id    INTEGER NOT NULL,"
         "  unique_id    INTEGER NOT NULL,"
         "  block_number INTEGER NOT NULL,"
         "  t0_ns        INTEGER NOT NULL,"
         "  t_end_ns     INTEGER NOT NULL,"
         "  dt_ns        INTEGER NOT NULL,"
         "  frames       INTEGER NOT NULL,"
         "  is_numeric   INTEGER NOT NULL DEFAULT 1,"
         "  min_value    REAL,"
         "  max_value    REAL,"
         "  sum_value    REAL,"
         "  finite_count INTEGER NOT NULL DEFAULT 0,"
         "  values_blob  BLOB NOT NULL,"
         "  raw_values   BLOB,"
         "  texts        BLOB,"
         "  raw_texts    BLOB,"
         "  times        BLOB"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_blocks_session_uid_t0 "
         "ON blocks (session_id, unique_id, t0_ns)");
  q.exec("CREATE INDEX IF NOT EXISTS idx_blocks_session_t0 "
         "ON blocks (session_id, t0_ns, t_end_ns)");
}

/**
 * @brief Creates the tags catalog and the session -> tag join table.
 */
void Sessions::DatabaseManager::createSchemaTagTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS tags ("
         "  tag_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  label  TEXT NOT NULL UNIQUE COLLATE NOCASE"
         ")");

  q.exec("CREATE TABLE IF NOT EXISTS session_tags ("
         "  session_id INTEGER NOT NULL,"
         "  tag_id     INTEGER NOT NULL,"
         "  PRIMARY KEY (session_id, tag_id)"
         ") WITHOUT ROWID");
}

/**
 * @brief Creates the project_metadata key/value store.
 */
void Sessions::DatabaseManager::createSchemaProjectMetadata(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS project_metadata ("
         "  key   TEXT PRIMARY KEY,"
         "  value TEXT NOT NULL"
         ") WITHOUT ROWID");
}

/**
 * @brief Creates the append-only verification-record table (spec 0044).
 */
void Sessions::DatabaseManager::createSchemaVerifications(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS verifications ("
         "  verification_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id      INTEGER NOT NULL REFERENCES sessions,"
         "  verified_at     TEXT NOT NULL,"
         "  app_version     TEXT NOT NULL,"
         "  verdict         TEXT NOT NULL,"
         "  detail_json     TEXT"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_verifications_session "
         "ON verifications (session_id, verification_id)");
}

#endif  // BUILD_COMMERCIAL
