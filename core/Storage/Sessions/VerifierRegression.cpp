/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include <algorithm>
#  include <bit>
#  include <cmath>
#  include <QCoreApplication>
#  include <QCryptographicHash>
#  include <QDir>
#  include <QFile>
#  include <QJsonArray>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QList>
#  include <QMap>
#  include <QSet>
#  include <QSqlQuery>
#  include <QStringList>

#  include "Core/AppInfo.h"
#  include "Core/DataModel/Frame.h"
#  include "Core/SerialStudio.h"
#  include "Core/SSAssert.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/PipelineModules.h"
#  include "DataModel/Scripting/ControlScript.h"
#  include "IO/FrameReader.h"
#  include "Sessions/BlockReader.h"
#  include "Sessions/Verifier.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr qint64 kMaxCandidateBytes = 10 * 1024 * 1024;
static constexpr int kMaxDivergenceSamples = 10;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bit-exact double comparison mirroring the verification diff core.
 */
static bool regressBitEqual(double a, double b)
{
  return std::bit_cast<quint64>(a) == std::bit_cast<quint64>(b);
}

/**
 * @brief Streaming cursor over one dataset's readings in a regenerated database, decoding the
 *        injected timestamp back to the (chunk, rank) provenance key row by row. Reads through
 *        Sessions::ReadingCursor, so a side stored as blocks and a side stored as legacy readings
 *        merge identically; the diff still never materializes a side in memory.
 */
struct RegenCursor {
  Sessions::ReadingCursor cursor;
  bool valid           = false;
  qint64 chunk         = 0;
  qint64 rank          = 0;
  double raw_numeric   = 0.0;
  double final_numeric = 0.0;
  bool is_numeric      = false;
  QString raw_string;
  QString final_string;

  /**
   * @brief Prepares and executes the ordered readings query, then loads the first row.
   */
  [[nodiscard]] bool open(QSqlDatabase& db, int sessionId, qint64 uniqueId)
  {
    if (!cursor.open(db, sessionId, uniqueId))
      return false;

    advance();
    return true;
  }

  /**
   * @brief Whether the walk stopped on a malformed block rather than at the end of the session.
   */
  [[nodiscard]] bool damaged() const noexcept { return cursor.damaged(); }

  /**
   * @brief Steps to the next row and decodes its provenance key; clears valid at the end.
   */
  void advance()
  {
    Sessions::ReadingRow row;
    valid = cursor.next(row);
    if (!valid)
      return;

    chunk         = row.timestampNs / Sessions::Verifier::kChunkStepNs;
    rank          = row.timestampNs % Sessions::Verifier::kChunkStepNs;
    raw_numeric   = row.rawNumeric;
    raw_string    = row.rawString;
    final_numeric = row.finalNumeric;
    final_string  = row.finalString;
    is_numeric    = row.isNumeric;
  }
};

/**
 * @brief Reads the column inventory (uid, title, is_virtual) of one regenerated session.
 */
static bool loadRegenColumns(QSqlDatabase& db,
                             int sessionId,
                             std::map<qint64, QString>& titles,
                             QSet<qint64>& virtuals)
{
  QSqlQuery q(db);
  q.prepare(
    QStringLiteral("SELECT unique_id, title, is_virtual FROM columns WHERE session_id = ?"));
  q.bindValue(0, sessionId);
  if (!q.exec())
    return false;

  while (q.next()) {
    titles[q.value(0).toLongLong()] = q.value(1).toString();
    if (q.value(2).toInt() != 0)
      virtuals.insert(q.value(0).toLongLong());
  }

  return true;
}

/**
 * @brief Opens a regenerated database read-only and resolves its single session id.
 */
static bool openRegen(const QString& path, const QString& connName, int& sessionId)
{
  auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
  db.setDatabaseName(path);
  db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
  if (!db.open())
    return false;

  QSqlQuery q(db);
  if (!q.exec(QStringLiteral("SELECT COUNT(*), MAX(session_id) FROM sessions")) || !q.next()
      || q.value(0).toInt() != 1)
    return false;

  sessionId = q.value(1).toInt();
  return true;
}

/**
 * @brief Extracts the set of source ids a project configuration declares; projects without a
 *        sources array use the single-source convention of device id 0.
 */
static QSet<int> projectSourceIds(const QString& projectJson)
{
  QSet<int> ids;
  const auto doc = QJsonDocument::fromJson(projectJson.toUtf8());
  if (!doc.isObject())
    return ids;

  const auto sources = doc.object().value(Keys::Sources).toArray();
  if (sources.isEmpty()) {
    ids.insert(0);
    return ids;
  }

  for (const auto& src : sources)
    ids.insert(src.toObject().value(Keys::SourceId).toInt());

  return ids;
}

/**
 * @brief Collects the frame-parser code of every source in a project (root code plus any
 *        per-source override), keyed by source id, for textual comparison.
 */
static QMap<int, QString> parserCodeBySource(const QJsonObject& project)
{
  QMap<int, QString> code;
  code.insert(-1, project.value(Keys::FrameParserCode).toString());

  const auto sources = project.value(Keys::Sources).toArray();
  for (const auto& src : sources) {
    const auto obj = src.toObject();
    code.insert(obj.value(Keys::SourceId).toInt(), obj.value(Keys::FrameParserCode).toString());
  }

  return code;
}

/**
 * @brief Collects every per-dataset transform code in document order for textual comparison.
 */
static QStringList transformCodes(const QJsonObject& project)
{
  QStringList codes;
  const auto groups = project.value(Keys::Groups).toArray();
  for (const auto& group : groups) {
    const auto datasets = group.toObject().value(Keys::Datasets).toArray();
    for (const auto& dataset : datasets)
      codes.append(dataset.toObject().value(Keys::TransformCode).toString());
  }

  return codes;
}

/**
 * @brief Textually compares the interpretation code of the archived and candidate projects
 *        (control script, frame parsers, value transforms) so the report can say what kind
 *        of change produced the drift; nothing is executed.
 */
static QJsonObject compareProjectCode(const QString& baseJson, const QString& candJson)
{
  const auto base = QJsonDocument::fromJson(baseJson.toUtf8()).object();
  const auto cand = QJsonDocument::fromJson(candJson.toUtf8()).object();

  QJsonObject result;
  result.insert(QStringLiteral("controlScriptChanged"),
                base.value(Keys::ControlScriptCode).toString()
                  != cand.value(Keys::ControlScriptCode).toString());
  result.insert(QStringLiteral("frameParserChanged"),
                parserCodeBySource(base) != parserCodeBySource(cand));
  result.insert(QStringLiteral("transformsChanged"), transformCodes(base) != transformCodes(cand));
  return result;
}

//--------------------------------------------------------------------------------------------------
// Regression flow
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs the spec-0047 regression pass: dual replay of the archived raw bytes (archived
 *        project vs candidate project) with injected provenance timestamps, then a
 *        provenance-joined drift diff. Ephemeral: the archive is never written.
 */
int Sessions::Verifier::runRegression()
{
  SS_ASSERT(m_options.mode == Mode::Regress, return kExitError);
  SS_ASSERT(!m_options.dbPath.isEmpty(), return kExitError);

  if (!openArchive())
    return failRegress(QStringLiteral("open-archive-failed"),
                       QStringLiteral("The session file could not be opened."),
                       QStringLiteral("Check that the file still exists and that you have "
                                      "permission to read it, then try again."));

  if (!loadSession())
    return failRegress(QStringLiteral("session-not-found"),
                       QStringLiteral("That session could not be found in this file."),
                       QStringLiteral("Choose a completed session from the list and try "
                                      "again."));

  classifySession();

  if (sessionIsConsoleOnly()) {
    m_verdict = QStringLiteral("not_verifiable");
    m_notes.append(QStringLiteral("This session contains raw console data only, so there "
                                  "is nothing to compare."));
    return settleRegressVerdict();
  }

  if (m_controlScriptSeen)
    m_notes.append(QStringLiteral("This session used a control script while recording. "
                                  "Scripts do not run during this comparison; values they "
                                  "provided are skipped, and everything parsed from the "
                                  "recorded data is compared."));

  const QString candidateError = loadCandidate();
  if (!candidateError.isEmpty())
    return failRegress(candidateError,
                       QStringLiteral("The project to compare against could not be loaded."),
                       QStringLiteral("Choose a readable Serial Studio project file, or open "
                                      "a valid project in the editor, then try again."));

  m_codeChanges = compareProjectCode(m_projectJson, m_candidateJson);
  if (m_codeChanges.value(QStringLiteral("controlScriptChanged")).toBool())
    m_notes.append(QStringLiteral("The control script differs between the recorded project "
                                  "and the current project."));

  if (m_codeChanges.value(QStringLiteral("frameParserChanged")).toBool())
    m_notes.append(QStringLiteral("The frame parser differs between the recorded project "
                                  "and the current project."));

  if (m_codeChanges.value(QStringLiteral("transformsChanged")).toBool())
    m_notes.append(QStringLiteral("One or more value transforms differ between the recorded "
                                  "project and the current project."));

  auto& controlScript = DataModel::pipelineModules().controlScript;
  controlScript.shutdown();

  const QString replayError = replayBothSides();
  if (replayError == QLatin1String("chunk-budget-exceeded") || m_chunkBudgetExceeded)
    return failRegress(QStringLiteral("chunk-budget-exceeded"),
                       QStringLiteral("This session is too dense to compare reliably."),
                       QStringLiteral("The check was stopped instead of showing results "
                                      "that could be wrong."));

  if (!replayError.isEmpty()) {
    QString reason, hint;
    reparseFailureText(replayError, reason, hint);
    return failRegress(replayError, reason, hint);
  }

  if (!regressDiff())
    return failRegress(QStringLiteral("diff-failed"),
                       QStringLiteral("The results could not be compared."),
                       QStringLiteral("Try again. If this keeps happening, the session file "
                                      "may be damaged."));

  const int code = settleRegressVerdict();
  cleanupRegenerated();
  return code;
}

/**
 * @brief Reads and fingerprints the candidate project file into m_candidateJson; returns an
 *        empty string on success or the regression failure code.
 */
QString Sessions::Verifier::loadCandidate()
{
  SS_ASSERT(!m_options.candidateProjectPath.isEmpty(),
            return QStringLiteral("regress-candidate-unreadable"));

  QFile file(m_options.candidateProjectPath);
  if (!file.open(QIODevice::ReadOnly))
    return QStringLiteral("regress-candidate-unreadable");

  if (file.size() > kMaxCandidateBytes)
    return QStringLiteral("regress-candidate-invalid");

  const QByteArray bytes = file.readAll();
  const auto doc         = QJsonDocument::fromJson(bytes);
  if (!doc.isObject())
    return QStringLiteral("regress-candidate-invalid");

  m_candidateJson = QString::fromUtf8(bytes);
  m_candidateSha256 =
    QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
  m_candidateTitle = doc.object().value(Keys::Title).toString();
  return QString();
}

/**
 * @brief Collects the distinct device ids present in the archived raw stream; @p ok is
 *        cleared on a query failure so emptiness never masquerades as a legitimate result.
 */
QSet<int> Sessions::Verifier::archivedDeviceIds(bool& ok)
{
  QSet<int> ids;
  QSqlQuery q(m_db);
  q.prepare(QStringLiteral("SELECT DISTINCT device_id FROM raw_bytes WHERE session_id = ?"));
  q.bindValue(0, m_sessionId);
  ok = q.exec();
  while (ok && q.next())
    ids.insert(q.value(0).toInt());

  return ids;
}

/**
 * @brief Replays the archive under the archived project (baseline) and the candidate project,
 *        both with injected provenance timestamps, excluding archived devices a side's project
 *        does not declare (never the silent frame-config fallback). Empty string on success.
 */
QString Sessions::Verifier::replayBothSides()
{
  SS_ASSERT(m_db.isOpen(), return QStringLiteral("feed-failed"));
  SS_ASSERT(!m_candidateJson.isEmpty(), return QStringLiteral("regress-candidate-invalid"));

  bool devicesOk          = false;
  const QSet<int> devices = archivedDeviceIds(devicesOk);
  if (!devicesOk)
    return QStringLiteral("feed-failed");

  const QString pidSuffix = QString::number(QCoreApplication::applicationPid());

  m_feedExcludedDevices = devices - projectSourceIds(m_projectJson);
  const QString baselineError =
    reparseSession(m_projectJson,
                   QDir::temp().filePath(QStringLiteral("ss-regress-base-%1.db").arg(pidSuffix)),
                   true);
  if (!baselineError.isEmpty())
    return baselineError;

  if (m_chunkBudgetExceeded)
    return QStringLiteral("chunk-budget-exceeded");

  m_replayStats.insert(QStringLiteral("baseline"), replaySideStats());

  m_readers.clear();
  const QSet<int> candidateExcluded = devices - projectSourceIds(m_candidateJson);
  m_candidateRemovedDevices = QList<int>(candidateExcluded.begin(), candidateExcluded.end());
  std::sort(m_candidateRemovedDevices.begin(), m_candidateRemovedDevices.end());

  m_feedExcludedDevices = candidateExcluded;
  const QString candidateError =
    reparseSession(m_candidateJson,
                   QDir::temp().filePath(QStringLiteral("ss-regress-cand-%1.db").arg(pidSuffix)),
                   true);
  if (!candidateError.isEmpty())
    return candidateError;

  m_replayStats.insert(QStringLiteral("candidate"), replaySideStats());
  m_feedExcludedDevices.clear();
  return QString();
}

/**
 * @brief Snapshots one replay side's pipeline statistics: feed totals plus the FrameBuilder
 *        counters (reset at each replay start), so a lossy pass is visible in the report.
 */
QJsonObject Sessions::Verifier::replaySideStats() const
{
  auto& builder = DataModel::pipelineModules().frameBuilder;
  return QJsonObject{
    {      QStringLiteral("chunksFed"),                                   m_lastFeedChunks},
    {QStringLiteral("framesExtracted"),                                   m_lastFeedFrames},
    {QStringLiteral("firstFrameChunk"),                                  m_firstFrameChunk},
    {   QStringLiteral("framesParsed"),    static_cast<qint64>(builder.parsedFrameCount())},
    {  QStringLiteral("framesSkipped"),   static_cast<qint64>(builder.skippedFrameCount())},
    {QStringLiteral("transformErrors"), static_cast<qint64>(builder.transformErrorCount())}
  };
}

/**
 * @brief Maps chunk indices back to the archived capture timestamps (raw_id order).
 */
std::vector<qint64> Sessions::Verifier::chunkTimestamps()
{
  std::vector<qint64> stamps;
  QSqlQuery q(m_db);
  q.prepare(
    QStringLiteral("SELECT timestamp_ns FROM raw_bytes WHERE session_id = ? ORDER BY raw_id"));
  q.bindValue(0, m_sessionId);
  if (q.exec())
    while (q.next())
      stamps.push_back(q.value(0).toLongLong());

  return stamps;
}

/**
 * @brief Merge-joins one dataset's baseline and candidate reading streams on the provenance
 *        key and accumulates the drift figures into a per-dataset report object.
 */
static QJsonObject diffRegressDataset(const QString& title,
                                      qint64 uniqueId,
                                      bool compareFinals,
                                      const std::vector<qint64>& stamps,
                                      RegenCursor& base,
                                      RegenCursor& cand)
{
  SS_ASSERT_LOG(uniqueId >= 0);

  qint64 compared = 0, changed = 0, onlyBaseline = 0, onlyCandidate = 0;
  double maxDelta = 0.0;
  QJsonArray divergenceSamples;

  // code-verify off -- every branch advances at least one finite SQL cursor
  while (base.valid && cand.valid) {
    const auto keyOrder = (base.chunk != cand.chunk)
                          ? (base.chunk < cand.chunk ? -1 : 1)
                          : (base.rank == cand.rank ? 0 : (base.rank < cand.rank ? -1 : 1));
    if (keyOrder < 0) {
      ++onlyBaseline;
      base.advance();
      continue;
    }

    if (keyOrder > 0) {
      ++onlyCandidate;
      cand.advance();
      continue;
    }

    ++compared;
    const bool rawMatch =
      regressBitEqual(base.raw_numeric, cand.raw_numeric) && base.raw_string == cand.raw_string;
    const bool finalMatch = !compareFinals
                         || (regressBitEqual(base.final_numeric, cand.final_numeric)
                             && base.final_string == cand.final_string);
    if (!rawMatch || !finalMatch) {
      ++changed;
      if (base.is_numeric && cand.is_numeric)
        maxDelta = std::max(maxDelta, std::fabs(base.final_numeric - cand.final_numeric));

      if (divergenceSamples.size() < kMaxDivergenceSamples) {
        const qint64 captureNs =
          (base.chunk >= 0 && base.chunk < static_cast<qint64>(stamps.size())) ? stamps[base.chunk]
                                                                               : -1;
        divergenceSamples.append(QJsonObject{
          {   QStringLiteral("captureTimestampNs"),captureNs                                                   },
          {                QStringLiteral("stage"),
           !rawMatch ? QStringLiteral("parse") : QStringLiteral("transform")},
          {          QStringLiteral("baselineRaw"),          base.raw_string},
          {         QStringLiteral("candidateRaw"),          cand.raw_string},
          {        QStringLiteral("baselineFinal"),        base.final_string},
          {       QStringLiteral("candidateFinal"),        cand.final_string},
          { QStringLiteral("baselineFinalNumeric"),       base.final_numeric},
          {QStringLiteral("candidateFinalNumeric"),       cand.final_numeric}
        });
      }
    }

    base.advance();
    cand.advance();
  }

  while (base.valid) {
    ++onlyBaseline;
    base.advance();
  }

  while (cand.valid) {
    ++onlyCandidate;
    cand.advance();
  }
  // code-verify on

  QJsonObject result;
  result.insert(Keys::UniqueId, uniqueId);
  result.insert(QStringLiteral("title"), title);
  if (base.damaged() || cand.damaged()) {
    result.insert(QStringLiteral("error"), QStringLiteral("archive damaged"));
    return result;
  }

  result.insert(QStringLiteral("finalsCompared"), compareFinals);
  result.insert(QStringLiteral("compared"), compared);
  result.insert(QStringLiteral("changed"), changed);
  result.insert(QStringLiteral("onlyBaseline"), onlyBaseline);
  result.insert(QStringLiteral("onlyCandidate"), onlyCandidate);
  if (changed > 0)
    result.insert(QStringLiteral("maxDelta"), maxDelta);

  if (!divergenceSamples.isEmpty()) {
    result.insert(QStringLiteral("firstDivergence"), divergenceSamples.first().toObject());
    result.insert(QStringLiteral("divergences"), divergenceSamples);
  }

  return result;
}

/**
 * @brief Lists every dataset of a single regenerated database as one-sided structural drift;
 *        used when the other replay produced no readings at all (its DB file was never
 *        created), which is drift to report honestly, never an infrastructure error.
 */
bool Sessions::Verifier::reportOneSidedStructural(const QString& regenPath, bool sideIsBaseline)
{
  const QString connName = QStringLiteral("ss_regress_onesided");
  bool ok                = false;
  {
    int sessionId = -1;
    if (openRegen(regenPath, connName, sessionId)) {
      auto db = QSqlDatabase::database(connName);
      std::map<qint64, QString> titles;
      QSet<qint64> virtuals;
      ok = loadRegenColumns(db, sessionId, titles, virtuals);
      if (ok)
        for (const auto& [uid, title] : titles)
          m_datasetReports.append(QJsonObject{
            {              Keys::UniqueId,uid                                          },
            {     QStringLiteral("title"),                                  title},
            {QStringLiteral("structural"),
             sideIsBaseline ? QStringLiteral("removed") : QStringLiteral("added")}
          });

      db.close();
    }
  }

  QSqlDatabase::removeDatabase(connName);
  return ok;
}

/**
 * @brief Diffs the two regenerated databases: opens both read-only, delegates to
 *        diffRegenPair(), and tears the connections down on every path. A side whose DB was
 *        never created (its replay decoded zero frames) reads as total structural drift.
 */
bool Sessions::Verifier::regressDiff()
{
  SS_ASSERT(m_regenPaths.size() == 2, return false);

  const bool baseExists = QFile::exists(m_regenPaths[0]);
  const bool candExists = QFile::exists(m_regenPaths[1]);
  if (!baseExists && !candExists) {
    m_notes.append(QStringLiteral("Neither the saved project nor the current project "
                                  "produced any values from this recording."));
    return true;
  }

  if (baseExists != candExists) {
    m_notes.append(candExists ? QStringLiteral("The project saved with this session no longer "
                                               "produces any values, so every dataset from the "
                                               "current project is shown as added.")
                              : QStringLiteral("The current project produces no values from this "
                                               "recording, so every recorded dataset is shown as "
                                               "removed."));
    return reportOneSidedStructural(baseExists ? m_regenPaths[0] : m_regenPaths[1], baseExists);
  }

  const QString baseConn = QStringLiteral("ss_regress_base");
  const QString candConn = QStringLiteral("ss_regress_cand");
  bool ok                = false;
  {
    int baseSession = -1, candSession = -1;
    if (openRegen(m_regenPaths[0], baseConn, baseSession)
        && openRegen(m_regenPaths[1], candConn, candSession)) {
      auto baseDb = QSqlDatabase::database(baseConn);
      auto candDb = QSqlDatabase::database(candConn);
      ok          = diffRegenPair(baseDb, candDb, baseSession, candSession);
      baseDb.close();
      candDb.close();
    }
  }

  QSqlDatabase::removeDatabase(baseConn);
  QSqlDatabase::removeDatabase(candConn);
  return ok;
}

/**
 * @brief Structural drift from the two column inventories, then coverage/value drift from the
 *        provenance-joined readings of every dataset both sides share.
 */
bool Sessions::Verifier::diffRegenPair(QSqlDatabase& baseDb,
                                       QSqlDatabase& candDb,
                                       int baseSession,
                                       int candSession)
{
  SS_ASSERT(baseDb.isOpen(), return false);
  SS_ASSERT(candDb.isOpen(), return false);

  std::map<qint64, QString> baseTitles, candTitles;
  QSet<qint64> baseVirtuals, candVirtuals;
  if (!loadRegenColumns(baseDb, baseSession, baseTitles, baseVirtuals)
      || !loadRegenColumns(candDb, candSession, candTitles, candVirtuals))
    return false;

  const auto stamps = chunkTimestamps();
  for (const auto& [uid, title] : candTitles) {
    if (baseTitles.contains(uid))
      continue;

    m_datasetReports.append(QJsonObject{
      {              Keys::UniqueId,                     uid},
      {     QStringLiteral("title"),                   title},
      {QStringLiteral("structural"), QStringLiteral("added")}
    });
  }

  for (const auto& [uid, title] : baseTitles) {
    if (!candTitles.contains(uid)) {
      m_datasetReports.append(QJsonObject{
        {              Keys::UniqueId,                       uid},
        {     QStringLiteral("title"),                     title},
        {QStringLiteral("structural"), QStringLiteral("removed")}
      });
      continue;
    }

    if (baseVirtuals.contains(uid) || candVirtuals.contains(uid)
        || m_virtualDatasets.contains(uid)) {
      m_datasetReports.append(QJsonObject{
        {           Keys::UniqueId,                               uid},
        {  QStringLiteral("title"),                             title},
        {QStringLiteral("skipped"), QStringLiteral("virtual-dataset")}
      });
      continue;
    }

    RegenCursor base, cand;
    if (!base.open(baseDb, baseSession, uid) || !cand.open(candDb, candSession, uid))
      return false;

    m_datasetReports.append(diffRegressDataset(title, uid, m_finalsVerifiable, stamps, base, cand));
  }

  return true;
}

/**
 * @brief Rolls the per-dataset reports up into the severity-ordered drift verdict
 *        (structural > coverage > value > identical) and builds the regression report.
 */
int Sessions::Verifier::settleRegressVerdict()
{
  SS_ASSERT_LOG(m_sessionId >= 0);
  SS_ASSERT_LOG(m_options.mode == Mode::Regress);

  bool structural = !m_candidateRemovedDevices.isEmpty();
  bool coverage   = false;
  bool value      = false;
  bool anySkipped = false;

  for (const auto& entryRef : std::as_const(m_datasetReports)) {
    const auto entry = entryRef.toObject();
    if (entry.contains(QStringLiteral("structural")))
      structural = true;

    if (entry.value(QStringLiteral("onlyBaseline")).toInteger() > 0
        || entry.value(QStringLiteral("onlyCandidate")).toInteger() > 0)
      coverage = true;

    if (entry.value(QStringLiteral("changed")).toInteger() > 0)
      value = true;

    if (entry.contains(QStringLiteral("skipped"))
        || (entry.contains(QStringLiteral("finalsCompared"))
            && !entry.value(QStringLiteral("finalsCompared")).toBool()))
      anySkipped = true;
  }

  int code = kExitNotVerifiable;
  if (m_verdict.isEmpty()) {
    const bool drifted = structural || coverage || value;
    m_verdict          = structural ? QStringLiteral("structural-drift")
                       : coverage   ? QStringLiteral("coverage-drift")
                       : value      ? QStringLiteral("value-drift")
                                    : QStringLiteral("identical");
    code               = drifted ? kExitDiverged : kExitReproduced;
  }

  if (anySkipped)
    m_notes.append(QStringLiteral("Some values were skipped because they depend on data "
                                  "that is not stored in the session."));

  if (m_legacyCapture)
    m_notes.append(QStringLiteral("This session was recorded by an older version of "
                                  "Serial Studio, so the comparison is best-effort."));

  for (const int deviceId : std::as_const(m_candidateRemovedDevices))
    m_notes.append(
      QStringLiteral("Recorded device %1 has no matching device in the current project, so "
                     "its data was left out of the comparison.")
        .arg(deviceId));

  QJsonObject candidate;
  candidate.insert(QStringLiteral("path"), m_options.candidateProjectPath);
  if (!m_candidateSha256.isEmpty()) {
    candidate.insert(QStringLiteral("title"), m_candidateTitle);
    candidate.insert(QStringLiteral("sha256"), m_candidateSha256);
  }

  m_report = QJsonObject();
  m_report.insert(QStringLiteral("mode"), QStringLiteral("regression"));
  m_report.insert(QStringLiteral("verdict"), m_verdict);
  m_report.insert(QStringLiteral("archive"), m_options.dbPath);
  m_report.insert(QStringLiteral("sessionId"), m_sessionId);
  m_report.insert(QStringLiteral("appVersion"), QStringLiteral(APP_VERSION));
  m_report.insert(QStringLiteral("captureAppVersion"), m_captureAppVersion);
  m_report.insert(QStringLiteral("legacyCapture"), m_legacyCapture);
  m_report.insert(QStringLiteral("candidate"), candidate);
  m_report.insert(QStringLiteral("replay"), m_replayStats);
  m_report.insert(QStringLiteral("codeChanges"), m_codeChanges);
  m_report.insert(QStringLiteral("baselineReproduction"), latestStoredVerdict());
  m_report.insert(QStringLiteral("datasets"), m_datasetReports);
  m_report.insert(QStringLiteral("notes"), m_notes);

  if (m_options.keepRegenerated && m_regenPaths.size() == 2) {
    m_report.insert(QStringLiteral("baselineDb"), m_regenPaths[0]);
    m_report.insert(QStringLiteral("candidateDb"), m_regenPaths[1]);
  }

  return code;
}

/**
 * @brief Returns the latest stored spec-0044 verdict for the session, or "never-verified".
 */
QString Sessions::Verifier::latestStoredVerdict()
{
  QSqlQuery q(m_db);
  q.prepare(QStringLiteral("SELECT verdict FROM verifications WHERE session_id = ? "
                           "ORDER BY verification_id DESC LIMIT 1"));
  q.bindValue(0, m_sessionId);
  if (q.exec() && q.next())
    return q.value(0).toString();

  return QStringLiteral("never-verified");
}

/**
 * @brief Builds a structured regression error report; ephemeral, never written to the archive.
 */
int Sessions::Verifier::failRegress(const QString& code, const QString& reason, const QString& hint)
{
  m_verdict = QStringLiteral("error");
  m_report  = QJsonObject{
     {     QStringLiteral("mode"), QStringLiteral("regression")},
     {  QStringLiteral("verdict"),                    m_verdict},
     {    QStringLiteral("error"),                       reason},
     {QStringLiteral("errorCode"),                         code},
     {     QStringLiteral("hint"),                         hint},
     {  QStringLiteral("archive"),             m_options.dbPath},
     {QStringLiteral("sessionId"),                  m_sessionId}
  };
  cleanupRegenerated();
  return kExitError;
}

#endif  // BUILD_COMMERCIAL
