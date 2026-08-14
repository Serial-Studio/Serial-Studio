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

#pragma once

#include <cstdint>
#include <memory>
#include <QHash>
#include <QSet>
#include <QString>
#include <unordered_map>
#include <vector>

#include "DataModel/FrameConsumer.h"
#include "IO/StreamWorker.h"
#include "SerialStudio.h"

class QFile;

namespace Widgets {

/**
 * @brief Single time-domain sample enqueued for asynchronous WAV writing.
 */
struct AudioExportItem {
  float value;
  quint32 sessionKey;
};

/**
 * @brief Immutable per-session recording parameters resolved at session start.
 */
struct AudioSessionConfig {
  int sampleRate{0};
  int uniqueId{-1};
  bool useScale{false};
  double center{0.0};
  double halfRange{1.0};
  QString datasetTitle;
  QString projectTitle;
  QString outputPath;
};

/**
 * @brief Per-session WAV-writer state owned by @c AudioExportWorker.
 */
struct AudioSession {
  std::unique_ptr<QFile> file;
  AudioSessionConfig config;
  qint64 sampleCount{0};
  qint64 bytesOnDisk{0};
  float runningPeak{0.0f};
  qint64 droppedSamples{0};
};

/**
 * @brief Worker that writes per-widget time-domain streams to WAV files on a background thread.
 */
class AudioExportWorker : public DataModel::FrameConsumerWorker<AudioExportItem> {
  Q_OBJECT

signals:
  void sessionOpenFailed(quint32 key);

public:
  using DataModel::FrameConsumerWorker<AudioExportItem>::FrameConsumerWorker;

  ~AudioExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void openSession(quint32 key, AudioSessionConfig config);
  void closeSession(quint32 key);
  void closeAllSessions();

protected:
  void processItems(const std::vector<AudioExportItem>& items) override;

private:
  [[nodiscard]] bool openWavFile(AudioSession& session);
  void writeWavHeader(QFile& file, int sampleRate);
  void patchWavSizes(QFile& file, qint64 dataBytes);
  void rescaleDataChunk(AudioSession& session, float gain);
  void writeAudibleCompanion(AudioSession& session, int factor);
  void finalizeSession(AudioSession& session);

  std::unordered_map<quint32, AudioSession> m_sessions;
};

/**
 * @brief Singleton facade recording FFT/Waterfall input to WAV (Pro). Widgets own their frame-lane
 * Dashboard taps; stream-lane sources feed sessions by dataset uniqueId via @c ingestStreamBlock
 * (queued from blockReady, so the GUI stays the queue's single producer). External auto-stop calls
 * @c closeAllSessions(), which emits @c sessionsClosed() so widgets disarm (T6/T7 contract).
 */
class AudioExport : public DataModel::FrameConsumer<AudioExportItem> {
  Q_OBJECT

signals:
  void sessionsClosed();
  void sessionClosed(quint32 key);
  void activeSessionsChanged();

private:
  explicit AudioExport();
  AudioExport(AudioExport&&)                 = delete;
  AudioExport(const AudioExport&)            = delete;
  AudioExport& operator=(AudioExport&&)      = delete;
  AudioExport& operator=(const AudioExport&) = delete;

  ~AudioExport();

public:
  [[nodiscard]] static AudioExport& instance();
  [[nodiscard]] static quint32 sessionKey(SerialStudio::DashboardWidget kind, int index);

  [[nodiscard]] bool hasActiveSessions() const noexcept;
  [[nodiscard]] Q_INVOKABLE QString audioPath(const QString& datasetTitle,
                                              const QString& projectTitle) const;

  /**
   * @brief Lock-free hotpath entry: enqueues one sample for the worker; no branching, no alloc.
   */
  void enqueueSample(quint32 key, double value) { enqueueData({static_cast<float>(value), key}); }

public slots:
  void openSession(SerialStudio::DashboardWidget kind, int index, AudioSessionConfig config);
  void closeSession(SerialStudio::DashboardWidget kind, int index);
  void closeAllSessions();
  void ingestStreamBlock(const IO::StreamBlockItemPtr& block);
  void setupExternalConnections();

private slots:
  void onSessionOpenFailed(quint32 key);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private:
  QSet<quint32> m_activeSessions;
  QHash<quint32, int> m_sessionDatasets;
};

}  // namespace Widgets
