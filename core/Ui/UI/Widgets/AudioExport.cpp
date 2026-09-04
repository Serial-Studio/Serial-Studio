/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/AudioExport.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "Core/SSAssert.h"
#include "CSV/Player.h"
#include "IO/ConnectionManager.h"
#include "Licensing/LemonSqueezy.h"
#include "MDF4/Player.h"
#include "Misc/WorkspaceManager.h"
#include "Sessions/Player.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kAudibleFloorHz       = 8000;
constexpr int kWavHeaderBytes       = 44;
constexpr int kBytesPerSample       = 4;
constexpr qint64 kRescaleChunk      = 65536;
constexpr float kHeadroomFullScale  = 0.891250938f;
constexpr qint64 kRiffSizeOffset    = 4;
constexpr qint64 kDataSizeOffset    = 40;
constexpr qint64 kRiffChunkOverhead = kWavHeaderBytes - 8;

//--------------------------------------------------------------------------------------------------
// Little-endian byte helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a little-endian 16-bit integer to a byte buffer.
 */
static void appendLE16(QByteArray& buffer, quint16 value)
{
  buffer.append(static_cast<char>(value & 0xFF));
  buffer.append(static_cast<char>((value >> 8) & 0xFF));
}

/**
 * @brief Appends a little-endian 32-bit integer to a byte buffer.
 */
static void appendLE32(QByteArray& buffer, quint32 value)
{
  buffer.append(static_cast<char>(value & 0xFF));
  buffer.append(static_cast<char>((value >> 8) & 0xFF));
  buffer.append(static_cast<char>((value >> 16) & 0xFF));
  buffer.append(static_cast<char>((value >> 24) & 0xFF));
}

/**
 * @brief Appends one IEEE-754 float as little-endian bytes to a byte buffer.
 */
static void appendFloatLE(QByteArray& buffer, float value)
{
  quint32 bits = 0;
  std::memcpy(&bits, &value, sizeof(bits));
  appendLE32(buffer, bits);
}

/**
 * @brief Maps a dashboard widget kind to a filename slug so co-titled taps get distinct files.
 */
static QString audioKindSlug(SerialStudio::DashboardWidget kind)
{
  SS_ASSERT(kind != SerialStudio::DashboardNoWidget, return QStringLiteral("widget"));
  switch (kind) {
    case SerialStudio::DashboardFFT:
      return QStringLiteral("fft");
    case SerialStudio::DashboardWaterfall:
      return QStringLiteral("waterfall");
    default:
      return QStringLiteral("widget");
  }
}

//--------------------------------------------------------------------------------------------------
// AudioExportWorker: WAV writer, drain loop, finalize
//--------------------------------------------------------------------------------------------------

/**
 * @brief Finalises every still-open session so an abrupt teardown leaves valid files.
 */
Widgets::AudioExportWorker::~AudioExportWorker()
{
  for (auto& [key, session] : m_sessions)
    finalizeSession(session);

  m_sessions.clear();
}

/**
 * @brief Returns @c true while any recording session holds an open file.
 */
bool Widgets::AudioExportWorker::isResourceOpen() const
{
  return !m_sessions.empty();
}

/**
 * @brief Finalises and forgets every session (drain-then-close path).
 */
void Widgets::AudioExportWorker::closeResources()
{
  for (auto& [key, session] : m_sessions)
    finalizeSession(session);

  m_sessions.clear();
}

/**
 * @brief Opens a new recording session for @p key, replacing any existing one first.
 */
void Widgets::AudioExportWorker::openSession(quint32 key, AudioSessionConfig config)
{
  SS_ASSERT(config.sampleRate > 0, {
    Q_EMIT sessionOpenFailed(key);
    return;
  });
  SS_ASSERT(!config.outputPath.isEmpty(), {
    Q_EMIT sessionOpenFailed(key);
    return;
  });

  auto existing = m_sessions.find(key);
  if (existing != m_sessions.end()) {
    finalizeSession(existing->second);
    m_sessions.erase(existing);
  }

  AudioSession session;
  session.config = std::move(config);
  if (!openWavFile(session)) {
    Q_EMIT sessionOpenFailed(key);
    return;
  }

  m_sessions.emplace(key, std::move(session));
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Drains pending samples, then finalises and removes the session for @p key.
 */
void Widgets::AudioExportWorker::closeSession(quint32 key)
{
  processData();

  auto it = m_sessions.find(key);
  if (it == m_sessions.end())
    return;

  finalizeSession(it->second);
  m_sessions.erase(it);
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Drains pending samples, then finalises and removes every session.
 */
void Widgets::AudioExportWorker::closeAllSessions()
{
  processData();

  const bool wasOpen = !m_sessions.empty();
  for (auto& [key, session] : m_sessions)
    finalizeSession(session);

  m_sessions.clear();
  if (wasOpen)
    Q_EMIT resourceOpenChanged();
}

/**
 * @brief Maps each queued sample to float32 and appends it to its session's WAV data chunk.
 */
void Widgets::AudioExportWorker::processItems(const std::vector<AudioExportItem>& items)
{
  static_assert(kBytesPerSample == 4);
  if (m_sessions.empty())
    return;

  std::unordered_map<quint32, QByteArray> pending;
  size_t count = items.size();
  SS_ASSERT(count <= 10000, count = 10000);
  for (size_t i = 0; i < count; ++i) {
    const auto& item = items[i];
    auto it          = m_sessions.find(item.sessionKey);
    if (it == m_sessions.end())
      continue;

    auto& session = it->second;
    float mapped  = item.value;
    if (session.config.useScale) {
      const double norm =
        (static_cast<double>(item.value) - session.config.center) / session.config.halfRange;
      mapped = static_cast<float>(std::clamp(norm, -1.0, 1.0));
    } else {
      const float magnitude = std::fabs(mapped);
      if (magnitude > session.runningPeak)
        session.runningPeak = magnitude;
    }

    appendFloatLE(pending[item.sessionKey], mapped);
    ++session.sampleCount;
  }

  for (auto& [key, buffer] : pending) {
    auto it = m_sessions.find(key);
    if (it == m_sessions.end() || !it->second.file)
      continue;

    auto& session      = it->second;
    const qint64 wrote = session.file->write(buffer);
    const qint64 good  = std::max<qint64>(0, wrote);
    SS_ASSERT_LOG(session.bytesOnDisk >= 0);
    if (session.bytesOnDisk < 0)
      continue;

    session.bytesOnDisk += good;
    if (wrote != buffer.size())
      session.droppedSamples += (buffer.size() - good) / kBytesPerSample;

    patchWavSizes(*session.file, session.bytesOnDisk);
  }
}

/**
 * @brief Creates the output directory + file and writes the placeholder WAV header.
 */
bool Widgets::AudioExportWorker::openWavFile(AudioSession& session)
{
  SS_ASSERT(session.config.sampleRate > 0, return false);
  SS_ASSERT(!session.config.outputPath.isEmpty(), return false);

  const QFileInfo info(session.config.outputPath);
  if (!QDir().mkpath(info.absolutePath())) {
    qWarning() << "AudioExport: failed to create directory" << info.absolutePath();
    return false;
  }

  session.file = std::make_unique<QFile>(session.config.outputPath);
  if (!session.file->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
    qWarning() << "AudioExport: failed to open" << session.config.outputPath;
    session.file.reset();
    return false;
  }

  writeWavHeader(*session.file, session.config.sampleRate);
  return true;
}

/**
 * @brief Writes a 44-byte float32 mono WAV header with placeholder chunk sizes.
 */
void Widgets::AudioExportWorker::writeWavHeader(QFile& file, int sampleRate)
{
  SS_ASSERT(file.isOpen(), return);
  SS_ASSERT(sampleRate > 0, return);

  const quint16 channels   = 1;
  const quint16 bits       = 32;
  const quint16 blockAlign = static_cast<quint16>(channels * bits / 8);
  const quint32 byteRate   = static_cast<quint32>(sampleRate) * blockAlign;

  QByteArray header;
  header.reserve(kWavHeaderBytes);
  header.append("RIFF", 4);
  appendLE32(header, 0);
  header.append("WAVE", 4);
  header.append("fmt ", 4);
  appendLE32(header, 16);
  appendLE16(header, 3);
  appendLE16(header, channels);
  appendLE32(header, static_cast<quint32>(sampleRate));
  appendLE32(header, byteRate);
  appendLE16(header, blockAlign);
  appendLE16(header, bits);
  header.append("data", 4);
  appendLE32(header, 0);

  const qint64 wrote = file.write(header);
  SS_ASSERT_LOG(wrote == kWavHeaderBytes);
}

/**
 * @brief Patches the RIFF and data chunk sizes, then seeks back to the data-chunk end.
 */
void Widgets::AudioExportWorker::patchWavSizes(QFile& file, qint64 dataBytes)
{
  SS_ASSERT(file.isOpen(), return);
  SS_ASSERT(dataBytes >= 0, return);

  QByteArray riffSize;
  QByteArray dataSize;
  appendLE32(riffSize, static_cast<quint32>(kRiffChunkOverhead + dataBytes));
  appendLE32(dataSize, static_cast<quint32>(dataBytes));

  bool ok = file.seek(kRiffSizeOffset) && file.write(riffSize) == riffSize.size();
  ok      = ok && file.seek(kDataSizeOffset) && file.write(dataSize) == dataSize.size();
  ok      = ok && file.seek(kWavHeaderBytes + dataBytes);
  if (!ok)
    qWarning() << "AudioExport: failed to patch WAV sizes for" << file.fileName();
}

/**
 * @brief Rewrites the data chunk in bounded passes, multiplying every sample by @p gain.
 */
void Widgets::AudioExportWorker::rescaleDataChunk(AudioSession& session, float gain)
{
  SS_ASSERT(session.file != nullptr, return);
  SS_ASSERT(gain > 0.0f, return);

  const qint64 dataBytes = session.bytesOnDisk;
  const qint64 chunks    = (dataBytes + kRescaleChunk - 1) / kRescaleChunk;
  for (qint64 c = 0; c < chunks; ++c) {
    const qint64 offset = kWavHeaderBytes + c * kRescaleChunk;
    const qint64 length = std::min(kRescaleChunk, dataBytes - c * kRescaleChunk);
    if (!session.file->seek(offset)) {
      qWarning() << "AudioExport: seek failed while rescaling" << session.config.outputPath;
      break;
    }

    QByteArray buffer = session.file->read(length);
    if (buffer.size() != length) {
      qWarning() << "AudioExport: short read while rescaling" << session.config.outputPath;
      break;
    }

    const qint64 samples = length / kBytesPerSample;
    for (qint64 s = 0; s < samples; ++s) {
      float value = 0.0f;
      std::memcpy(&value, buffer.constData() + s * kBytesPerSample, sizeof(value));
      value *= gain;
      std::memcpy(buffer.data() + s * kBytesPerSample, &value, sizeof(value));
    }

    if (!session.file->seek(offset) || session.file->write(buffer) != buffer.size()) {
      qWarning() << "AudioExport: short write while rescaling" << session.config.outputPath;
      break;
    }
  }
}

/**
 * @brief Writes the sped-up "-audible-<N>x.wav" companion sharing the finalised data chunk.
 */
void Widgets::AudioExportWorker::writeAudibleCompanion(AudioSession& session, int factor)
{
  SS_ASSERT(session.file != nullptr, return);
  SS_ASSERT(factor >= 1, factor = 1);

  QString base = session.config.outputPath;
  if (base.endsWith(QStringLiteral(".wav"), Qt::CaseInsensitive))
    base.chop(4);

  const QString path = QStringLiteral("%1-audible-%2x.wav").arg(base, QString::number(factor));
  QFile companion(path);
  if (!companion.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "AudioExport: failed to open companion" << path;
    return;
  }

  writeWavHeader(companion, session.config.sampleRate * factor);

  const qint64 dataBytes = session.bytesOnDisk;
  const qint64 chunks    = (dataBytes + kRescaleChunk - 1) / kRescaleChunk;
  for (qint64 c = 0; c < chunks; ++c) {
    const qint64 offset = kWavHeaderBytes + c * kRescaleChunk;
    const qint64 length = std::min(kRescaleChunk, dataBytes - c * kRescaleChunk);
    if (!session.file->seek(offset)) {
      qWarning() << "AudioExport: seek failed for companion" << path;
      break;
    }

    const QByteArray buffer = session.file->read(length);
    if (buffer.size() != length) {
      qWarning() << "AudioExport: short read for companion" << path;
      break;
    }

    if (companion.write(buffer) != buffer.size()) {
      qWarning() << "AudioExport: short write for companion" << path;
      break;
    }
  }

  patchWavSizes(companion, dataBytes);
  companion.close();
}

/**
 * @brief Patches sizes, rescales when peak-normalised, emits the companion, drops empty files.
 */
void Widgets::AudioExportWorker::finalizeSession(AudioSession& session)
{
  SS_ASSERT(session.config.sampleRate > 0, return);
  if (!session.file)
    return;

  if (session.sampleCount == 0) {
    session.file->close();
    QFile::remove(session.config.outputPath);
    session.file.reset();
    return;
  }

  SS_ASSERT(session.runningPeak >= 0.0f, session.runningPeak = 0.0f);
  if (!session.config.useScale && session.runningPeak > 0.0f)
    rescaleDataChunk(session, kHeadroomFullScale / session.runningPeak);

  patchWavSizes(*session.file, session.bytesOnDisk);

  if (session.config.sampleRate < kAudibleFloorHz) {
    const int factor =
      static_cast<int>(std::ceil(static_cast<double>(kAudibleFloorHz) / session.config.sampleRate));
    writeAudibleCompanion(session, factor);
  }

  if (session.droppedSamples > 0)
    qWarning() << "AudioExport: dropped" << session.droppedSamples << "samples for"
               << session.config.outputPath;

  session.file->close();
  session.file.reset();
}

//--------------------------------------------------------------------------------------------------
// AudioExport: singleton facade
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the singleton and starts the WAV-writer worker thread.
 */
Widgets::AudioExport::AudioExport()
  : DataModel::FrameConsumer<AudioExportItem>(
      {.queueCapacity = 65536, .flushThreshold = 4096, .timerIntervalMs = 33})
{
  initializeWorker();

  auto* worker = static_cast<AudioExportWorker*>(m_worker);
  connect(worker, &AudioExportWorker::sessionOpenFailed, this, &AudioExport::onSessionOpenFailed);

  setConsumerEnabled(true);
}

/**
 * @brief Default destructor.
 */
Widgets::AudioExport::~AudioExport() = default;

/**
 * @brief Returns the application-wide @c AudioExport singleton.
 */
Widgets::AudioExport& Widgets::AudioExport::instance()
{
  static AudioExport singleton;
  return singleton;
}

/**
 * @brief Packs a widget kind and index into a stable 32-bit session key.
 */
quint32 Widgets::AudioExport::sessionKey(SerialStudio::DashboardWidget kind, int index)
{
  SS_ASSERT(index >= 0, index = 0);
  return (static_cast<quint32>(kind) << 16) | (static_cast<quint32>(index) & 0xFFFFu);
}

/**
 * @brief Factory called by @c FrameConsumer to create the WAV-writer worker.
 */
DataModel::FrameConsumerWorkerBase* Widgets::AudioExport::createWorker()
{
  return new AudioExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

/**
 * @brief Returns true while any recording session is open; ConnectionManager reads this so the
 *        stream workers build export payloads while only WAV recording consumes them.
 */
bool Widgets::AudioExport::hasActiveSessions() const noexcept
{
  return !m_activeSessions.isEmpty();
}

/**
 * @brief Feeds one published block into the recording sessions matching its dataset uniqueIds.
 *        Runs on the pipeline thread, which is the single producer for every sink (spec 0055 D8),
 *        so this facade's queue keeps its single-producer invariant.
 */
void Widgets::AudioExport::ingestBlock(const DataModel::DataBlockPtr& block)
{
  SS_ASSERT(block != nullptr, return);

  const QMutexLocker locker(&m_sessionDatasetsMutex);
  if (m_sessionDatasets.isEmpty())
    return;

  const auto used = static_cast<std::size_t>(block->samples);
  for (const auto& column : block->columns) {
    if (column.uniqueId < 0)
      continue;

    for (auto it = m_sessionDatasets.cbegin(); it != m_sessionDatasets.cend(); ++it) {
      if (it.value() != column.uniqueId)
        continue;

      for (std::size_t i = 0; i < used; ++i)
        enqueueSample(it.key(), column.values[i]);
    }
  }
}

/**
 * @brief Returns the workspace directory that holds a dataset's audio recordings.
 */
QString Widgets::AudioExport::audioPath(const QString& datasetTitle,
                                        const QString& projectTitle) const
{
  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  const auto base               = workspaceManager.path(QStringLiteral("Audio Recordings"));
  return QStringLiteral("%1/%2/%3")
    .arg(base,
         Misc::WorkspaceManager::sanitizeName(projectTitle),
         Misc::WorkspaceManager::sanitizeName(datasetTitle));
}

/**
 * @brief Resolves the timestamped output path (main thread) and opens the session on the worker.
 */
void Widgets::AudioExport::openSession(SerialStudio::DashboardWidget kind,
                                       int index,
                                       AudioSessionConfig config)
{
  SS_ASSERT(m_worker != nullptr, return);
  SS_ASSERT(config.sampleRate > 0, return);

  const quint32 key = sessionKey(kind, index);
  const auto dir    = audioPath(config.datasetTitle, config.projectTitle);
  const auto stamp =
    QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss-zzz"));
  const auto slug = audioKindSlug(kind);
  config.outputPath =
    QStringLiteral("%1/%2-%3%4.wav").arg(dir, stamp, slug, QString::number(index));
  m_activeSessions.insert(key);
  {
    const QMutexLocker locker(&m_sessionDatasetsMutex);
    m_sessionDatasets.insert(key, config.uniqueId);
  }

  auto* worker = static_cast<AudioExportWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker, [worker, key, config] { worker->openSession(key, config); }, Qt::QueuedConnection);
  Q_EMIT activeSessionsChanged();
}

/**
 * @brief Finalises the session for one widget on the worker thread.
 */
void Widgets::AudioExport::closeSession(SerialStudio::DashboardWidget kind, int index)
{
  SS_ASSERT(m_worker != nullptr, return);
  const quint32 key = sessionKey(kind, index);
  m_activeSessions.remove(key);
  {
    const QMutexLocker locker(&m_sessionDatasetsMutex);
    m_sessionDatasets.remove(key);
  }

  auto* worker = static_cast<AudioExportWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker, [worker, key] { worker->closeSession(key); }, Qt::QueuedConnection);
  Q_EMIT activeSessionsChanged();
}

/**
 * @brief Finalises every session and notifies widgets to disarm their taps.
 */
void Widgets::AudioExport::closeAllSessions()
{
  SS_ASSERT(m_worker != nullptr, return);
  m_activeSessions.clear();
  {
    const QMutexLocker locker(&m_sessionDatasetsMutex);
    m_sessionDatasets.clear();
  }

  auto* worker = static_cast<AudioExportWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, [worker] { worker->closeAllSessions(); }, Qt::QueuedConnection);

  Q_EMIT sessionsClosed();
  Q_EMIT activeSessionsChanged();
}

/**
 * @brief Drops a session whose worker-side file open failed and notifies its widget to disarm.
 */
void Widgets::AudioExport::onSessionOpenFailed(quint32 key)
{
  SS_ASSERT(m_worker != nullptr, return);
  SS_ASSERT_LOG(thread() == QThread::currentThread());

  m_activeSessions.remove(key);
  {
    const QMutexLocker locker(&m_sessionDatasetsMutex);
    m_sessionDatasets.remove(key);
  }
  Q_EMIT sessionClosed(key);
  Q_EMIT activeSessionsChanged();
}

/**
 * @brief Wires the auto-stop sources: disconnect, pause, replay open, and license loss.
 */
void Widgets::AudioExport::setupExternalConnections()
{
  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (!IO::ConnectionManager::instance().isConnected())
        closeAllSessions();
    });

  connect(&IO::ConnectionManager::instance(), &IO::ConnectionManager::pausedChanged, this, [this] {
    if (IO::ConnectionManager::instance().paused())
      closeAllSessions();
  });

  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [this] {
    if (CSV::Player::instance().isOpen())
      closeAllSessions();
  });

  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, [this] {
    if (MDF4::Player::instance().isOpen())
      closeAllSessions();
  });

  connect(&Sessions::Player::instance(), &Sessions::Player::openChanged, this, [this] {
    if (Sessions::Player::instance().isOpen())
      closeAllSessions();
  });

  connect(
    &Licensing::LemonSqueezy::instance(), &Licensing::LemonSqueezy::activatedChanged, this, [this] {
      if (!SerialStudio::activated())
        closeAllSessions();
    });
}
