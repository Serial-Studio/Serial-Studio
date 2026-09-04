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

#  include "Sessions/DatabaseManager/ReproducibilityVerifier.h"

#  include <QCoreApplication>
#  include <QDir>
#  include <QFile>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QProcess>
#  include <QSqlDatabase>
#  include <QSqlQuery>
#  include <QStringList>

#  include "Core/SSAssert.h"
#  include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// File-local constants
//--------------------------------------------------------------------------------------------------

static constexpr int kStderrTailBytes          = 2000;
static constexpr int kVerifierTerminateGraceMs = 3000;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

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
 * @brief Resolves the child process verdict from its exit state and stdout, falling back to a
 *        structured failure report when the child crashed or wrote something unreadable.
 */
static QVariantMap readChildVerdict(QProcess* process,
                                    int exitCode,
                                    QProcess::ExitStatus status,
                                    bool& ok)
{
  SS_ASSERT(process != nullptr, return QVariantMap());

  const auto doc = QJsonDocument::fromJson(process->readAllStandardOutput());
  ok             = status == QProcess::NormalExit && doc.isObject();
  if (ok)
    return doc.object().toVariantMap();

  return childFailureReport(status != QProcess::NormalExit ? QStringLiteral("child-crashed")
                                                           : QStringLiteral("child-output-invalid"),
                            exitCode,
                            process->readAllStandardError());
}

//--------------------------------------------------------------------------------------------------
// Construction & archive binding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an idle verifier bound to no archive.
 */
Sessions::ReproducibilityVerifier::ReproducibilityVerifier(QObject* parent)
  : QObject(parent)
  , m_regressActive(false)
  , m_sweepActive(false)
  , m_sweepOwnsCandidate(false)
  , m_process(nullptr)
{}

/**
 * @brief Points the verifier at the archive checks run against; an empty path means the
 *        historian is closed and every entry point declines.
 */
void Sessions::ReproducibilityVerifier::setArchive(const QString& filePath)
{
  m_archivePath = filePath;
}

/**
 * @brief Aborts any sweep and tears the child process down. Called from DatabaseManager's own
 *        shutdown, before the worker thread joins.
 */
void Sessions::ReproducibilityVerifier::shutdown()
{
  m_sweepActive = false;
  m_sweepQueue.clear();

  if (!m_process)
    return;

  m_process->terminate();
  if (!m_process->waitForFinished(kVerifierTerminateGraceMs))
    m_process->kill();
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while a check child process is running.
 */
bool Sessions::ReproducibilityVerifier::busy() const noexcept
{
  return m_process != nullptr;
}

/**
 * @brief Returns true while the shared child slot is running a regression pass (spec 0047).
 */
bool Sessions::ReproducibilityVerifier::regressionBusy() const noexcept
{
  return m_process != nullptr && m_regressActive;
}

/**
 * @brief Returns the last regression report; ephemeral, cleared when a new pass starts.
 */
const QVariantMap& Sessions::ReproducibilityVerifier::lastReport() const noexcept
{
  return m_lastReport;
}

/**
 * @brief Returns the sweep state for the API poll surface: activity flag, tag, verdict
 *        counters derived from the collected reports, and the per-session reports.
 */
QVariantMap Sessions::ReproducibilityVerifier::sweepStatus() const
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
// Verification (spec 0044)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Launches the spec-0044 verifier child process for @p sessionId (-1 = latest completed).
 *        The child appends the verification record itself; this side only parses the verdict
 *        JSON from stdout and refreshes the UI. One verification runs at a time.
 */
void Sessions::ReproducibilityVerifier::verify(int sessionId)
{
  SS_ASSERT(!m_archivePath.isEmpty(), return);

  if (m_process || m_sweepActive)
    return;

  auto* process = new QProcess(this);
  m_process     = process;
  Q_EMIT busyChanged();

  QStringList args{QStringLiteral("--verify-session"), m_archivePath, QStringLiteral("--headless")};
  if (sessionId >= 0)
    args << QStringLiteral("--verify-session-id") << QString::number(sessionId);

  connect(process,
          &QProcess::finished,
          this,
          [this, process, sessionId](int exitCode, QProcess::ExitStatus status) {
            bool ok  = false;
            auto map = readChildVerdict(process, exitCode, status, ok);

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
void Sessions::ReproducibilityVerifier::concludeVerification(int sessionId,
                                                             bool success,
                                                             const QVariantMap& verdict)
{
  if (!m_process)
    return;

  m_process->deleteLater();
  m_process = nullptr;
  Q_EMIT busyChanged();
  Q_EMIT verificationFinished(sessionId, success, verdict);
}

//--------------------------------------------------------------------------------------------------
// Regression (spec 0047)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes a regression start failure through the ephemeral report channel so the
 *        QML/API surfaces never see a silent no-op, then returns false for the caller.
 */
bool Sessions::ReproducibilityVerifier::publishStartFailure(int sessionId,
                                                            const QString& code,
                                                            const QString& error,
                                                            const QString& hint)
{
  QVariantMap map;
  map.insert(QStringLiteral("verdict"), QStringLiteral("error"));
  map.insert(QStringLiteral("errorCode"), code);
  map.insert(QStringLiteral("error"), error);
  map.insert(QStringLiteral("hint"), hint);

  m_lastReport = map;
  Q_EMIT reportChanged();
  Q_EMIT regressionFinished(sessionId, false, map);
  return false;
}

/**
 * @brief Launches the spec-0047 regression child for @p sessionId against @p candidatePath,
 *        or against the currently open project (serialized to a temporary file) when the path
 *        is empty. Shares the single child slot with verification: one pass at a time.
 *        Returns false (with a published error report) when the pass did not start.
 */
bool Sessions::ReproducibilityVerifier::regress(int sessionId, const QString& candidatePath)
{
  SS_ASSERT(!m_archivePath.isEmpty(), return false);

  if (m_process)
    return false;

  QString candidate = candidatePath;
  m_regressCandidateTemp.clear();
  if (candidate.isEmpty()) {
    candidate = QDir::temp().filePath(
      QStringLiteral("ss-regress-candidate-%1.json").arg(QCoreApplication::applicationPid()));
    if (!writeCandidateSnapshot(candidate))
      return publishStartFailure(
        sessionId,
        QStringLiteral("candidate-write-failed"),
        QStringLiteral("The current project could not be saved for comparison."),
        QStringLiteral("Check that there is enough free disk space, then try again."));

    m_regressCandidateTemp = candidate;
  }

  auto* process   = new QProcess(this);
  m_process       = process;
  m_regressActive = true;
  m_lastReport.clear();
  Q_EMIT busyChanged();
  Q_EMIT regressionBusyChanged();
  Q_EMIT reportChanged();

  QStringList args{QStringLiteral("--regress-session"),
                   m_archivePath,
                   QStringLiteral("--headless"),
                   QStringLiteral("--regress-project"),
                   candidate};
  if (sessionId >= 0)
    args << QStringLiteral("--regress-session-id") << QString::number(sessionId);

  connect(process,
          &QProcess::finished,
          this,
          [this, process, sessionId](int exitCode, QProcess::ExitStatus status) {
            bool ok  = false;
            auto map = readChildVerdict(process, exitCode, status, ok);

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
void Sessions::ReproducibilityVerifier::concludeRegression(int sessionId,
                                                           bool success,
                                                           const QVariantMap& report)
{
  if (!m_process)
    return;

  m_process->deleteLater();
  m_process       = nullptr;
  m_regressActive = false;

  if (!m_regressCandidateTemp.isEmpty()) {
    QFile::remove(m_regressCandidateTemp);
    m_regressCandidateTemp.clear();
  }

  m_lastReport = report;
  if (m_sweepActive)
    advanceSweep(sessionId, report);

  Q_EMIT busyChanged();
  Q_EMIT regressionBusyChanged();
  Q_EMIT reportChanged();
  Q_EMIT regressionFinished(sessionId, success, report);
}

//--------------------------------------------------------------------------------------------------
// Golden-tag sweep (spec 0047)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lists the completed sessions carrying @p tag, oldest first, through a private
 *        read-only connection to the archive. Sets @p ok to false when the query fails.
 */
QList<int> Sessions::ReproducibilityVerifier::taggedSessions(const QString& tag, bool& ok) const
{
  ok = false;
  QList<int> ids;
  const QString connName = QStringLiteral("ss_dbm_sweep_read");
  {
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    db.setDatabaseName(m_archivePath);
    db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
    if (db.open()) {
      QSqlQuery q(db);
      q.prepare(QStringLiteral("SELECT s.session_id FROM sessions s "
                               "JOIN session_tags st ON st.session_id = s.session_id "
                               "JOIN tags t ON t.tag_id = st.tag_id "
                               "WHERE t.label = ? AND s.ended_at IS NOT NULL "
                               "ORDER BY s.session_id"));
      q.bindValue(0, tag);
      ok = q.exec();
      while (ok && q.next())
        ids.append(q.value(0).toInt());

      db.close();
    }
  }

  QSqlDatabase::removeDatabase(connName);
  return ids;
}

/**
 * @brief Starts a golden-tag regression sweep: tagged completed sessions run through
 *        regress() sequentially against one shared candidate snapshot. Returns false
 *        (with a published error report) when the sweep did not start; state is only touched
 *        once the candidate is secured, so a failed start never destroys prior results.
 */
bool Sessions::ReproducibilityVerifier::regressByTag(const QString& tag,
                                                     const QString& candidatePath)
{
  SS_ASSERT(!m_archivePath.isEmpty(), return false);

  if (m_process || m_sweepActive || tag.trimmed().isEmpty())
    return false;

  bool queryOk   = false;
  const auto ids = taggedSessions(tag.trimmed(), queryOk);
  if (!queryOk)
    return publishStartFailure(-1,
                               QStringLiteral("sweep-query-failed"),
                               QStringLiteral("The tagged sessions could not be listed."),
                               QStringLiteral("Reopen the session file and try again."));

  QString candidate   = candidatePath;
  bool owns_candidate = false;
  if (candidate.isEmpty() && !ids.isEmpty()) {
    candidate = QDir::temp().filePath(
      QStringLiteral("ss-regress-sweep-%1.json").arg(QCoreApplication::applicationPid()));
    if (!writeCandidateSnapshot(candidate))
      return publishStartFailure(
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
  Q_EMIT sweepChanged();

  if (m_sweepQueue.isEmpty()) {
    finishSweep();
    return true;
  }

  const int first = m_sweepQueue.takeFirst();
  if (!regress(first, m_sweepCandidate)) {
    finishSweep();
    return false;
  }

  return true;
}

/**
 * @brief Records one finished sweep entry and launches the next session, or finalizes when
 *        the queue is drained or the archive closed mid-sweep.
 */
void Sessions::ReproducibilityVerifier::advanceSweep(int sessionId, const QVariantMap& report)
{
  QVariantMap entry;
  entry.insert(QStringLiteral("sessionId"), sessionId);
  entry.insert(QStringLiteral("report"), report);
  m_sweepReports.append(entry);

  if (m_sweepQueue.isEmpty() || m_archivePath.isEmpty()) {
    finishSweep();
    return;
  }

  const int next = m_sweepQueue.takeFirst();
  if (!regress(next, m_sweepCandidate))
    finishSweep();
}

/**
 * @brief Ends the sweep: removes the sweep-owned candidate snapshot and publishes the state.
 */
void Sessions::ReproducibilityVerifier::finishSweep()
{
  if (m_sweepOwnsCandidate && !m_sweepCandidate.isEmpty())
    QFile::remove(m_sweepCandidate);

  m_sweepActive        = false;
  m_sweepOwnsCandidate = false;
  m_sweepCandidate.clear();
  m_sweepQueue.clear();
  Q_EMIT sweepChanged();
}

#endif
