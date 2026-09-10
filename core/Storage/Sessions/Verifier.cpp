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

#  include "Sessions/Verifier.h"

#  include <bit>
#  include <QCoreApplication>
#  include <QCryptographicHash>
#  include <QDateTime>
#  include <QDir>
#  include <QFile>
#  include <QJsonDocument>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QSqlRecord>
#  include <QVariant>

#  include "AppState.h"
#  include "Core/AppInfo.h"
#  include "Core/Bus/MessageBus.h"
#  include "Core/Bus/Messages.h"
#  include "Core/DataModel/Frame.h"
#  include "Core/IO/FrameConfigBuilder.h"
#  include "Core/IO/HAL_Driver.h"
#  include "Core/License.h"
#  include "Core/SerialStudio.h"
#  include "Core/SSAssert.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/PipelineModules.h"
#  include "DataModel/ProjectModel.h"
#  include "DataModel/Scripting/FrameParser.h"
#  include "IO/FrameReader.h"
#  include "Sessions/BlockReader.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr quint64 kFlushEveryFrames = 4096;

static const IO::CapturedData::SteadyTimePoint kInjectionEpoch{};

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bit-exact double comparison: the stored IEEE-754 pattern must match, NaN included.
 */
static bool bitEqual(double a, double b)
{
  return std::bit_cast<quint64>(a) == std::bit_cast<quint64>(b);
}

/**
 * @brief Returns the column value of the current row, or an invalid QVariant when absent.
 */
static QVariant fieldOr(const QSqlQuery& query, const char* column)
{
  const int index = query.record().indexOf(QLatin1String(column));
  return index >= 0 ? query.value(index) : QVariant();
}

//--------------------------------------------------------------------------------------------------
// Construction & top-level flow
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the options; all work happens in run().
 */
Sessions::Verifier::Verifier(const Options& options, Core::Bus::MessageBus& bus)
  : m_bus(bus)
  , m_options(options)
  , m_sessionId(-1)
  , m_framesDropped(-1)
  , m_overflowBytes(-1)
  , m_legacyCapture(true)
  , m_rawIntegrity(QStringLiteral("missing"))
  , m_readingsIntegrity(QStringLiteral("missing"))
  , m_controlScriptSeen(false)
  , m_finalsVerifiable(true)
  , m_firstFrameChunk(-1)
  , m_lastFeedChunks(0)
  , m_lastFeedFrames(0)
  , m_chunkBudgetExceeded(false)
{}

/**
 * @brief Drops the archive connection.
 */
Sessions::Verifier::~Verifier()
{
  m_readers.clear();
  m_db = QSqlDatabase();
  if (!m_archiveConnName.isEmpty())
    QSqlDatabase::removeDatabase(m_archiveConnName);
}

/**
 * @brief Maps a re-parse failure code to its user-facing reason and remediation hint.
 */
void Sessions::Verifier::reparseFailureText(const QString& code, QString& reason, QString& hint)
{
  static const struct {
    const char* code;
    const char* reason;
    const char* hint;
  } kFailures[] = {
    { "stored-project-invalid",
     "The project settings saved with this session are damaged.",   "The session cannot be checked. Restore the session file from a backup copy."},
    {"stored-project-rejected",
     "The project settings saved with this session cannot be loaded by this version of "
     "Serial Studio.",                                                  "Update Serial Studio to a version at least as new as the one that recorded the "
                                                  "session, then try again."                                               },
    {    "export-not-licensed",
     "Session recording is not activated on this computer.",                                                           "Checking a session requires a Pro license. Activate a license or start a trial, "
                                                           "then try again."         },
    {    "export-start-failed",
     "A temporary working file could not be created.",                   "Check that there is enough free disk space, then try again."           },
    {            "feed-failed",
     "The recorded data in this session could not be replayed.", "The session file may be incomplete or damaged. Restore it from a backup copy." },
  };

  for (const auto& entry : kFailures) {
    if (code == QLatin1String(entry.code)) {
      reason = QString::fromLatin1(entry.reason);
      hint   = QString::fromLatin1(entry.hint);
      return;
    }
  }

  reason = QStringLiteral("The session could not be checked.");
  hint   = QStringLiteral("Try again. If this keeps happening, the session file may be "
                          "damaged.");
}

/**
 * @brief Runs every verification stage and returns the process exit code.
 */
int Sessions::Verifier::run()
{
  if (m_options.mode == Mode::Regress)
    return runRegression();

  if (!openArchive())
    return fail(QStringLiteral("open-archive-failed"),
                QStringLiteral("open-archive"),
                QStringLiteral("The session file could not be opened."),
                QStringLiteral("Check that the file still exists and that you have "
                               "permission to read it, then try again."));

  if (!loadSession())
    return fail(QStringLiteral("session-not-found"),
                QStringLiteral("load-session"),
                QStringLiteral("That session could not be found in this file."),
                QStringLiteral("Choose a completed session from the list and try again."));

  if (!verifyIntegrity())
    return fail(QStringLiteral("archive-unreadable"),
                QStringLiteral("integrity"),
                QStringLiteral("The session file could not be read."),
                QStringLiteral("The file may be incomplete or damaged. Restore it from a "
                               "backup copy."));

  classifySession();

  const bool consoleOnly = sessionIsConsoleOnly();
  if (!consoleOnly && !m_controlScriptSeen) {
    const QString regenPath = QDir::temp().filePath(
      QStringLiteral("ss-verify-regen-%1.db").arg(QCoreApplication::applicationPid()));
    const QString reparseError = reparseSession(m_projectJson, regenPath, false);
    if (!reparseError.isEmpty()) {
      QString reason, hint;
      reparseFailureText(reparseError, reason, hint);
      return fail(reparseError, QStringLiteral("reparse"), reason, hint);
    }

    if (!diffReadings())
      return fail(QStringLiteral("diff-failed"),
                  QStringLiteral("diff"),
                  QStringLiteral("The results could not be compared."),
                  QStringLiteral("Try again. If this keeps happening, the session file may "
                                 "be damaged."));
  }

  const int code = settleVerdict();
  appendVerificationRecord();
  cleanupRegenerated();
  return code;
}

/**
 * @brief Returns the verdict report the CLI prints as JSON.
 */
const QJsonObject& Sessions::Verifier::report() const noexcept
{
  return m_report;
}

/**
 * @brief Builds a structured error report (code, stage, reason, remediation hint), persists it
 *        as a verification record when the session id is known (so polling API clients observe
 *        the failure), and returns the error exit code.
 */
int Sessions::Verifier::fail(const QString& code,
                             const QString& stage,
                             const QString& reason,
                             const QString& hint)
{
  m_verdict = QStringLiteral("error");
  m_report  = QJsonObject{
     {  QStringLiteral("verdict"),        m_verdict},
     {    QStringLiteral("error"),           reason},
     {QStringLiteral("errorCode"),             code},
     {    QStringLiteral("stage"),            stage},
     {     QStringLiteral("hint"),             hint},
     {  QStringLiteral("archive"), m_options.dbPath},
     {QStringLiteral("sessionId"),      m_sessionId}
  };
  appendVerificationRecord();
  cleanupRegenerated();
  return kExitError;
}

//--------------------------------------------------------------------------------------------------
// Archive loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the archived database read-only.
 */
bool Sessions::Verifier::openArchive()
{
  SS_ASSERT(!m_options.dbPath.isEmpty(), return false);

  if (!QFile::exists(m_options.dbPath))
    return false;

  m_archiveConnName = QStringLiteral("ss_verify_archive");
  m_db              = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_archiveConnName);
  m_db.setDatabaseName(m_options.dbPath);
  m_db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
  return m_db.open();
}

/**
 * @brief Loads the target session row (given id, or the latest completed one) plus the archived
 *        column map. Fingerprint columns may not exist in a pre-0044 archive: absent means
 *        legacy capture, never an error.
 */
bool Sessions::Verifier::loadSession()
{
  SS_ASSERT(m_db.isOpen(), return false);

  QSqlQuery q(m_db);
  if (m_options.sessionId >= 0) {
    q.prepare(QStringLiteral("SELECT * FROM sessions WHERE session_id = ?"));
    q.bindValue(0, m_options.sessionId);
  } else {
    q.prepare(QStringLiteral(
      "SELECT * FROM sessions WHERE ended_at IS NOT NULL ORDER BY session_id DESC LIMIT 1"));
  }

  if (!q.exec() || !q.next())
    return false;

  m_sessionId            = q.value(QStringLiteral("session_id")).toInt();
  m_projectJson          = q.value(QStringLiteral("project_json")).toString();
  m_storedRawSha256      = fieldOr(q, "raw_sha256").toString();
  m_storedReadingsSha256 = fieldOr(q, "readings_sha256").toString();
  m_captureAppVersion    = fieldOr(q, "app_version").toString();
  m_reproClassJson       = fieldOr(q, "repro_class").toString();
  m_legacyCapture        = fieldOr(q, "capture_format").isNull();

  const auto dropped  = fieldOr(q, "frames_dropped");
  const auto overflow = fieldOr(q, "overflow_bytes");
  m_framesDropped     = dropped.isNull() ? -1 : dropped.toLongLong();
  m_overflowBytes     = overflow.isNull() ? -1 : overflow.toLongLong();

  if (m_projectJson.isEmpty()) {
    QSqlQuery meta(m_db);
    meta.prepare(QStringLiteral("SELECT value FROM project_metadata WHERE key = 'project_json'"));
    if (meta.exec() && meta.next())
      m_projectJson = meta.value(0).toString();
  }

  QSqlQuery cols(m_db);
  cols.prepare(QStringLiteral("SELECT unique_id, is_virtual FROM columns WHERE session_id = ?"));
  cols.bindValue(0, m_sessionId);
  if (cols.exec()) {
    while (cols.next())
      if (cols.value(1).toInt() != 0)
        m_virtualDatasets.insert(cols.value(0).toLongLong());
  }

  return true;
}

/**
 * @brief True for sessions captured without an interpretation pipeline (ConsoleOnly stub
 *        project or a project with no groups): only raw-integrity can be verified.
 */
bool Sessions::Verifier::sessionIsConsoleOnly() const
{
  const auto doc = QJsonDocument::fromJson(m_projectJson.toUtf8());
  if (!doc.isObject())
    return true;

  return doc.object().value(Keys::Groups).toArray().isEmpty();
}

//--------------------------------------------------------------------------------------------------
// Integrity & classification stages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-hashes the archived raw_bytes and readings streams with the shared canonical layout
 *        and compares against the capture-time digests. A mismatch means the archive itself
 *        changed since capture; missing digests mean a legacy capture. Returns false when a
 *        table cannot be read at all: an unreadable archive is an error, never a "mismatch".
 */
bool Sessions::Verifier::verifyIntegrity()
{
  SS_ASSERT(m_db.isOpen(), return false);
  SS_ASSERT(m_sessionId >= 0, return false);

  QCryptographicHash rawHash(QCryptographicHash::Sha256);
  QSqlQuery raw(m_db);
  raw.prepare(QStringLiteral("SELECT timestamp_ns, device_id, data FROM raw_bytes "
                             "WHERE session_id = ? ORDER BY raw_id"));
  raw.bindValue(0, m_sessionId);
  if (!raw.exec())
    return false;

  while (raw.next())
    hashRawChunk(
      rawHash, raw.value(0).toLongLong(), raw.value(1).toInt(), raw.value(2).toByteArray());

  QCryptographicHash readingsHash(QCryptographicHash::Sha256);
  if (!hashSampleStream(readingsHash))
    return false;

  const auto rawHex      = QString::fromLatin1(rawHash.result().toHex());
  const auto readingsHex = QString::fromLatin1(readingsHash.result().toHex());

  if (!m_storedRawSha256.isEmpty())
    m_rawIntegrity =
      rawHex == m_storedRawSha256 ? QStringLiteral("verified") : QStringLiteral("mismatch");

  if (!m_storedReadingsSha256.isEmpty())
    m_readingsIntegrity = readingsHex == m_storedReadingsSha256 ? QStringLiteral("verified")
                                                                : QStringLiteral("mismatch");

  return true;
}

/**
 * @brief Re-hashes the archive's sample stream in insertion order, using whichever digest its
 *        storage was written with. A legacy archive keeps `hashReadingRow` over `readings` exactly
 *        as it was captured -- re-deriving it any other way would report every pre-0055 archive as
 *        tampered -- and a spec-0055 archive hashes its `blocks` rows with `hashBlockRow`.
 */
bool Sessions::Verifier::hashSampleStream(QCryptographicHash& hash)
{
  if (Sessions::sessionUsesBlocks(m_db, m_sessionId)) {
    QSqlQuery blocks(m_db);
    blocks.setForwardOnly(true);
    blocks.prepare(QStringLiteral("SELECT unique_id, t0_ns, dt_ns, frames, values_blob, "
                                  "raw_values, texts FROM blocks WHERE session_id = ? "
                                  "ORDER BY block_id"));
    blocks.bindValue(0, m_sessionId);
    if (!blocks.exec())
      return false;

    while (blocks.next())
      hashBlockRow(hash,
                   blocks.value(0).toLongLong(),
                   blocks.value(1).toLongLong(),
                   blocks.value(2).toLongLong(),
                   blocks.value(3).toLongLong(),
                   blocks.value(4).toByteArray(),
                   blocks.value(5).toByteArray(),
                   blocks.value(6).toByteArray());

    return true;
  }

  QSqlQuery readings(m_db);
  readings.setForwardOnly(true);
  readings.prepare(
    QStringLiteral("SELECT timestamp_ns, unique_id, raw_numeric_value, raw_string_value, "
                   "final_numeric_value, final_string_value, is_numeric FROM readings "
                   "WHERE session_id = ? ORDER BY reading_id"));
  readings.bindValue(0, m_sessionId);
  if (!readings.exec())
    return false;

  while (readings.next())
    hashReadingRow(hash,
                   readings.value(0).toLongLong(),
                   readings.value(1).toLongLong(),
                   SerialStudio::toDouble(readings.value(2)),
                   readings.value(3).toString(),
                   SerialStudio::toDouble(readings.value(4)),
                   readings.value(5).toString(),
                   readings.value(6).toInt() != 0);

  return true;
}

/**
 * @brief Reads the capture-time classification: a control script makes the whole session not
 *        mechanically verifiable (it can mutate interpretation state mid-session); transforms
 *        combined with project data tables make only the final values unverifiable.
 */
void Sessions::Verifier::classifySession()
{
  const auto doc = QJsonDocument::fromJson(m_reproClassJson.toUtf8());
  if (!doc.isObject())
    return;

  const auto obj           = doc.object();
  m_controlScriptSeen      = obj.value(QStringLiteral("controlScript")).toBool();
  const bool transformsFed = obj.value(QStringLiteral("transformsPresent")).toBool()
                          && obj.value(QStringLiteral("tablesPresent")).toBool();
  m_finalsVerifiable = !transformsFed;
}

//--------------------------------------------------------------------------------------------------
// Re-parse stage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads @p projectJson into the real pipeline and re-records the archived raw bytes
 *        into @p regenPath via the untouched Sessions::Export path; empty string on success,
 *        else the failure code for run()/runRegression(). Parse budget disabled around the
 *        feed (benchmark precedent): a headless flat-out replay must never skip frames.
 */
QString Sessions::Verifier::reparseSession(const QString& projectJson,
                                           const QString& regenPath,
                                           bool injectTimestamps)
{
  SS_ASSERT(!projectJson.isEmpty(), return QStringLiteral("stored-project-invalid"));
  SS_ASSERT(!regenPath.isEmpty(), return QStringLiteral("export-start-failed"));

  const auto doc = QJsonDocument::fromJson(projectJson.toUtf8());
  if (!doc.isObject())
    return QStringLiteral("stored-project-invalid");

  auto& appState = DataModel::pipelineModules().appState;
  appState.setOperationMode(SerialStudio::ProjectFile);

  auto& project = DataModel::pipelineModules().projectModel;
  project.setSuppressMessageBoxes(true);
  if (!project.loadFromJsonDocument(doc))
    return QStringLiteral("stored-project-rejected");

  auto& parser = DataModel::pipelineModules().frameParser;
  parser.setSuppressMessageBoxes(true);
  parser.readCode();

  auto& builder = DataModel::pipelineModules().frameBuilder;
  builder.syncFromProjectModel();
  builder.setParseBudgetEnabled(false);
  builder.resetFrameCounters();

  m_regenPath = regenPath;
  m_regenPaths.push_back(regenPath);
  QFile::remove(m_regenPath);
  DatabaseManager::setDbPathOverride(m_regenPath);

  if (!Core::License::activated())
    return QStringLiteral("export-not-licensed");

  static auto& exporter = Sessions::Export::instance();
  exporter.setSettingsPersistent(false);
  exporter.setRegressionBaselinePinned(injectTimestamps);
  exporter.setExportEnabled(true);
  if (!exporter.exportEnabled()) {
    exporter.setRegressionBaselinePinned(false);
    return QStringLiteral("export-start-failed");
  }

  const bool fed = feedArchivedBytes(injectTimestamps);

  builder.flushOpenBlocks();
  exporter.flushWorker();
  exporter.closeFile();
  exporter.setExportEnabled(false);
  exporter.setRegressionBaselinePinned(false);
  builder.setParseBudgetEnabled(true);
  DatabaseManager::setDbPathOverride(QString());
  return fed ? QString() : QStringLiteral("feed-failed");
}

/**
 * @brief Streams the archived raw chunks per device through FrameReader extraction into
 *        FrameBuilder, mirroring ConnectionManager::onFrameReady routing. Blocking worker
 *        flushes bound the export queue so the re-record never drops frames. Regression mode
 *        injects chunk-indexed timestamps so readings carry a deterministic provenance key.
 */
bool Sessions::Verifier::feedArchivedBytes(bool injectTimestamps)
{
  SS_ASSERT(m_db.isOpen(), return false);
  SS_ASSERT(m_sessionId >= 0, return false);

  QSqlQuery rows(m_db);
  rows.prepare(
    QStringLiteral("SELECT device_id, data FROM raw_bytes WHERE session_id = ? ORDER BY raw_id"));
  rows.bindValue(0, m_sessionId);
  if (!rows.exec())
    return false;

  auto& builder         = DataModel::pipelineModules().frameBuilder;
  static auto& exporter = Sessions::Export::instance();

  m_firstFrameChunk = -1;
  m_lastFeedChunks  = 0;
  m_lastFeedFrames  = 0;

  qint64 chunkIndex  = -1;
  quint64 sinceFlush = 0;
  IO::CapturedDataPtr drained;
  while (rows.next()) {
    ++chunkIndex;
    const int deviceId = rows.value(0).toInt();
    if (m_feedExcludedDevices.contains(deviceId))
      continue;

    if (!m_readers.contains(deviceId)
        && m_readers.size() >= static_cast<size_t>(kMaxArchiveDevices))
      return false;

    auto& reader = readerForDevice(deviceId);
    if (injectTimestamps)
      reader.processData(IO::makeCapturedData(
        rows.value(1).toByteArray(),
        kInjectionEpoch + std::chrono::nanoseconds(chunkIndex * kChunkStepNs)));
    else
      reader.processData(IO::makeCapturedData(rows.value(1).toByteArray()));

    ++m_lastFeedChunks;

    qint64 chunkFrames = 0;
    auto& queue        = reader.queue();
    while (queue.try_dequeue(drained)) {
      builder.hotpathRxSourceFrame(deviceId, drained);
      ++sinceFlush;
      ++chunkFrames;
    }

    m_lastFeedFrames += chunkFrames;

    if (m_firstFrameChunk < 0 && chunkFrames > 0)
      m_firstFrameChunk = chunkIndex;

    if (chunkFrames > kMaxFramesPerChunk)
      m_chunkBudgetExceeded = true;

    if (sinceFlush >= kFlushEveryFrames) {
      builder.flushOpenBlocks();
      exporter.flushWorker();
      QCoreApplication::processEvents();
      sinceFlush = 0;
    }
  }

  QCoreApplication::processEvents();
  return true;
}

/**
 * @brief Returns the FrameReader for @p deviceId, creating it from the production FrameConfig
 *        on first use.
 */
IO::FrameReader& Sessions::Verifier::readerForDevice(int deviceId)
{
  auto it = m_readers.find(deviceId);
  if (it != m_readers.end())
    return *it->second;

  const auto project = m_bus.latest<Core::Bus::ProjectStructureSnapshot>();
  SS_ASSERT_LOG(project != nullptr);
  const Core::Bus::ProjectStructureSnapshot empty{};
  auto& appState    = DataModel::pipelineModules().appState;
  const auto config = IO::FrameConfigBuilder::build(
    project ? *project : empty, appState.operationMode(), appState.frameConfig(), deviceId);

  auto reader = std::make_unique<IO::FrameReader>();
  reader->setOperationMode(config.operationMode);
  reader->setFrameDetectionMode(config.frameDetection);
  reader->setStartSequences(config.startSequences);
  reader->setFinishSequences(config.finishSequences);
  reader->setChecksum(config.checksumAlgorithm);

  auto& slot = m_readers[deviceId];
  slot       = std::move(reader);
  return *slot;
}

//--------------------------------------------------------------------------------------------------
// Diff stage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates the regenerated database (exactly one session, non-empty when the archive
 *        has readings) so an infrastructure failure reports as an error, never as divergence.
 */
static bool regenSanityCheck(QSqlDatabase& regen,
                             QSqlDatabase& archive,
                             int archiveSessionId,
                             int& regenSessionId)
{
  QSqlQuery sessions(regen);
  if (!sessions.exec(QStringLiteral("SELECT COUNT(*), MAX(session_id) FROM sessions"))
      || !sessions.next() || sessions.value(0).toInt() != 1)
    return false;

  regenSessionId = sessions.value(1).toInt();

  const auto sampleCount = [](QSqlDatabase& db, int sessionId) -> qint64 {
    QSqlQuery q(db);
    q.prepare(Sessions::sessionUsesBlocks(db, sessionId)
                ? QStringLiteral("SELECT COALESCE(SUM(frames), 0) FROM blocks WHERE session_id = ?")
                : QStringLiteral("SELECT COUNT(*) FROM readings WHERE session_id = ?"));
    q.bindValue(0, sessionId);
    if (!q.exec() || !q.next())
      return -1;

    return q.value(0).toLongLong();
  };

  const qint64 archivedRows = sampleCount(archive, archiveSessionId);
  const qint64 regenRows    = sampleCount(regen, regenSessionId);
  if (archivedRows < 0 || regenRows < 0)
    return false;

  return archivedRows == 0 || regenRows > 0;
}

/**
 * @brief Sequence-diffs the regenerated readings against the archived ones, one dataset at a
 *        time. Virtual (table-fed) datasets are classified, never compared.
 */
bool Sessions::Verifier::diffReadings()
{
  SS_ASSERT(!m_regenPath.isEmpty(), return false);

  const QString connName = QStringLiteral("ss_verify_regen");
  bool ok                = false;
  {
    auto regen = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    regen.setDatabaseName(m_regenPath);
    regen.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
    ok = regen.open();

    int regenSessionId = -1;
    if (ok)
      ok = regenSanityCheck(regen, m_db, m_sessionId, regenSessionId);

    if (ok) {
      QSqlQuery uids(m_db);
      uids.prepare(QStringLiteral("SELECT DISTINCT unique_id FROM columns WHERE session_id = ?"));
      uids.bindValue(0, m_sessionId);
      ok = uids.exec();

      while (ok && uids.next()) {
        const qint64 uid = uids.value(0).toLongLong();
        if (m_virtualDatasets.contains(uid)) {
          QJsonObject skipped;
          skipped.insert(Keys::UniqueId, uid);
          skipped.insert(QStringLiteral("skipped"), QStringLiteral("virtual-dataset"));
          m_datasetReports.append(skipped);
          continue;
        }

        m_datasetReports.append(diffDataset(regen, regenSessionId, uid, m_finalsVerifiable));
      }

      regen.close();
    }
  }

  QSqlDatabase::removeDatabase(connName);
  return ok;
}

/**
 * @brief Walks archived and regenerated reading sequences for one dataset in lockstep. Raw
 *        values attribute a mismatch to the parse stage, final values to the transform stage.
 *        Ordering rides (timestamp_ns, reading_id): prefix-covered by the existing per-uid
 *        index on legacy archives, and identical to insertion order within one dataset.
 */
QJsonObject Sessions::Verifier::diffDataset(QSqlDatabase& regen,
                                            int regenSessionId,
                                            qint64 uniqueId,
                                            bool compareFinals)
{
  SS_ASSERT(regen.isOpen(), return QJsonObject());
  SS_ASSERT(m_sessionId >= 0, return QJsonObject());

  Sessions::ReadingCursor archived;
  Sessions::ReadingCursor regenerated;

  QJsonObject result;
  result.insert(Keys::UniqueId, uniqueId);
  result.insert(QStringLiteral("finalsCompared"), compareFinals);
  if (!archived.open(m_db, m_sessionId, uniqueId)
      || !regenerated.open(regen, regenSessionId, uniqueId)) {
    result.insert(QStringLiteral("error"), QStringLiteral("query failed"));
    return result;
  }

  qint64 recordedRows = 0, regenRows = 0, mismatches = 0, row = 0;
  QJsonObject firstMismatch;

  Sessions::ReadingRow a;
  Sessions::ReadingRow b;
  // code-verify off
  // Bounded: the cursor walks a finite result set and yields each row once.
  while (archived.next(a)) {
    ++recordedRows;
    if (!regenerated.next(b))
      continue;

    ++regenRows;
    ++row;

    const bool rawMatch = bitEqual(a.rawNumeric, b.rawNumeric) && a.rawString == b.rawString;
    const bool finalMatch =
      !compareFinals
      || (bitEqual(a.finalNumeric, b.finalNumeric) && a.finalString == b.finalString);

    if (rawMatch && finalMatch)
      continue;

    ++mismatches;
    if (!firstMismatch.isEmpty())
      continue;

    const bool parseStage = !rawMatch;
    firstMismatch         = QJsonObject{
              {                    QStringLiteral("row"),                                                                row},
              {    QStringLiteral("recordedTimestampNs"),                                                      a.timestampNs},
              {                  QStringLiteral("stage"), parseStage ? QStringLiteral("parse") : QStringLiteral("transform")},
              {            QStringLiteral("recordedRaw"),                                                        a.rawString},
              {         QStringLiteral("regeneratedRaw"),                                                        b.rawString},
              {     QStringLiteral("recordedRawNumeric"),                                                       a.rawNumeric},
              {  QStringLiteral("regeneratedRawNumeric"),                                                       b.rawNumeric},
              {          QStringLiteral("recordedFinal"),                                                      a.finalString},
              {       QStringLiteral("regeneratedFinal"),                                                      b.finalString},
              {   QStringLiteral("recordedFinalNumeric"),                                                     a.finalNumeric},
              {QStringLiteral("regeneratedFinalNumeric"),                                                     b.finalNumeric}
    };
  }

  while (regenerated.next(b))
    ++regenRows;
  // code-verify on

  if (archived.damaged() || regenerated.damaged()) {
    result.insert(QStringLiteral("error"), QStringLiteral("archive damaged"));
    return result;
  }

  result.insert(QStringLiteral("recordedRows"), recordedRows);
  result.insert(QStringLiteral("regeneratedRows"), regenRows);
  result.insert(QStringLiteral("mismatches"), mismatches);
  if (!firstMismatch.isEmpty())
    result.insert(QStringLiteral("firstMismatch"), firstMismatch);

  if (recordedRows != regenRows)
    result.insert(QStringLiteral("countMismatch"), true);

  return result;
}

//--------------------------------------------------------------------------------------------------
// Verdict, record & cleanup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps the stage outcomes to the verdict string, notes, and exit code. Guard-clause
 *        priority: control script beats everything, divergence beats classification.
 */
int Sessions::Verifier::decideVerdict(bool diverged, bool anySkipped, bool consoleOnly)
{
  if (m_controlScriptSeen && !diverged) {
    m_verdict = QStringLiteral("not_verifiable");
    m_notes.append(QStringLiteral("This session used a control script while recording. Its "
                                  "results can change between runs, so they cannot be "
                                  "checked mechanically."));
    return kExitNotVerifiable;
  }

  if (diverged) {
    m_verdict = QStringLiteral("diverged");
    return kExitDiverged;
  }

  if (consoleOnly) {
    const bool verified = m_rawIntegrity == QStringLiteral("verified");
    m_verdict = verified ? QStringLiteral("reproduced") : QStringLiteral("not_verifiable");
    m_notes.append(QStringLiteral("This session contains raw console data only, so there is "
                                  "nothing to re-interpret. Only the stored data itself was "
                                  "checked."));
    return verified ? kExitReproduced : kExitNotVerifiable;
  }

  if (anySkipped) {
    m_verdict = QStringLiteral("partial");
    m_notes.append(QStringLiteral("Verified, except for values that depend on data that is "
                                  "not stored in the session."));
    return kExitNotVerifiable;
  }

  m_verdict = QStringLiteral("reproduced");
  return kExitReproduced;
}

/**
 * @brief Rolls the stage results up into the verdict and builds the final report. Honesty rules:
 *        classification beats a green checkmark, and a count mismatch is divergence annotated
 *        with the capture-time loss counters, never silently realigned.
 */
int Sessions::Verifier::settleVerdict()
{
  bool diverged = m_rawIntegrity == QStringLiteral("mismatch")
               || m_readingsIntegrity == QStringLiteral("mismatch");

  if (m_rawIntegrity == QStringLiteral("mismatch"))
    m_notes.append(QStringLiteral("The recorded data no longer matches the way it was "
                                  "originally saved. The session file was changed after "
                                  "recording."));

  if (m_readingsIntegrity == QStringLiteral("mismatch"))
    m_notes.append(QStringLiteral("The stored values no longer match the way they were "
                                  "originally saved. The session file was changed after "
                                  "recording.%1")
                     .arg(m_rawIntegrity == QStringLiteral("verified")
                            ? QStringLiteral(" The raw recorded data is intact.")
                            : QString()));

  bool anyError   = false;
  bool anySkipped = false;
  for (const auto& entryRef : std::as_const(m_datasetReports)) {
    const auto entry = entryRef.toObject();
    if (entry.contains(QStringLiteral("error")))
      anyError = true;

    if (entry.value(QStringLiteral("mismatches")).toInteger() > 0
        || entry.value(QStringLiteral("countMismatch")).toBool())
      diverged = true;

    if (entry.contains(QStringLiteral("skipped"))
        || !entry.value(QStringLiteral("finalsCompared")).toBool())
      anySkipped = true;
  }

  int code                      = decideVerdict(diverged, anySkipped, sessionIsConsoleOnly());
  const bool datasetQueryFailed = anyError && !diverged;
  if (datasetQueryFailed) {
    m_verdict = QStringLiteral("error");
    m_notes.append(QStringLiteral("Some values could not be checked, so no conclusion was "
                                  "reached about the data."));
    code = kExitError;
  }

  if (m_legacyCapture)
    m_notes.append(QStringLiteral("This session was recorded by an older version of Serial "
                                  "Studio, so the check is best-effort."));

  QJsonObject integrity;
  integrity.insert(QStringLiteral("rawBytes"), m_rawIntegrity);
  integrity.insert(QStringLiteral("readings"), m_readingsIntegrity);

  m_report = QJsonObject();
  m_report.insert(QStringLiteral("verdict"), m_verdict);
  m_report.insert(QStringLiteral("archive"), m_options.dbPath);
  m_report.insert(QStringLiteral("sessionId"), m_sessionId);
  m_report.insert(QStringLiteral("verifyingAppVersion"), QStringLiteral(APP_VERSION));
  m_report.insert(QStringLiteral("captureAppVersion"), m_captureAppVersion);
  m_report.insert(QStringLiteral("legacyCapture"), m_legacyCapture);
  m_report.insert(QStringLiteral("integrity"), integrity);
  m_report.insert(QStringLiteral("classification"),
                  QJsonDocument::fromJson(m_reproClassJson.toUtf8()).object());
  m_report.insert(QStringLiteral("captureFramesDropped"), m_framesDropped);
  m_report.insert(QStringLiteral("captureOverflowBytes"), m_overflowBytes);
  m_report.insert(QStringLiteral("datasets"), m_datasetReports);
  m_report.insert(QStringLiteral("notes"), m_notes);

  if (datasetQueryFailed) {
    m_report.insert(QStringLiteral("errorCode"), QStringLiteral("dataset-query-failed"));
    m_report.insert(QStringLiteral("stage"), QStringLiteral("diff"));
    m_report.insert(QStringLiteral("hint"),
                    QStringLiteral("Try again. If this keeps happening, the session file "
                                   "may be damaged."));
  }

  if (m_options.keepRegenerated && !m_regenPath.isEmpty())
    m_report.insert(QStringLiteral("regeneratedDb"), m_regenPath);

  return code;
}

/**
 * @brief Appends one verifications row to the archive: the only write verification ever makes.
 *        Legacy archives get the verifications table created, nothing else is migrated.
 *        Regression mode (spec 0047) is ephemeral and never writes to the archive at all.
 */
void Sessions::Verifier::appendVerificationRecord()
{
  if (m_options.mode == Mode::Regress)
    return;

  if (m_sessionId < 0)
    return;

  const QString connName = QStringLiteral("ss_verify_record");
  {
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    db.setDatabaseName(m_options.dbPath);
    if (db.open()) {
      QSqlQuery q(db);
      q.exec(QStringLiteral("PRAGMA busy_timeout=5000"));
      DatabaseManager::createSchemaVerifications(q);

      q.prepare(
        QStringLiteral("INSERT INTO verifications (session_id, verified_at, app_version, verdict, "
                       "detail_json) VALUES (?, ?, ?, ?, ?)"));
      q.bindValue(0, m_sessionId);
      q.bindValue(1, QDateTime::currentDateTime().toString(Qt::ISODate));
      q.bindValue(2, QStringLiteral(APP_VERSION));
      q.bindValue(3, m_verdict);
      q.bindValue(4, QString::fromUtf8(QJsonDocument(m_report).toJson(QJsonDocument::Compact)));
      if (!q.exec())
        qWarning() << "[Verifier] verification record append failed:" << q.lastError().text();

      db.close();
    } else {
      qWarning() << "[Verifier] cannot open archive for record append:" << db.lastError().text();
    }
  }
  QSqlDatabase::removeDatabase(connName);
}

/**
 * @brief Clears the DB-path override on every terminal path, then removes every temporary
 *        regenerated database unless --verify-keep-regen / --regress-keep-regen was given.
 */
void Sessions::Verifier::cleanupRegenerated()
{
  DatabaseManager::setDbPathOverride(QString());

  if (m_options.keepRegenerated)
    return;

  for (const auto& path : m_regenPaths) {
    QFile::remove(path);
    QFile::remove(path + QStringLiteral("-wal"));
    QFile::remove(path + QStringLiteral("-shm"));
  }

  m_regenPaths.clear();
  m_regenPath.clear();
}

#endif  // BUILD_COMMERCIAL
