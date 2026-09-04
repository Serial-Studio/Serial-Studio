/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>
#include <QThread>

#include "CSV/PlayerLoaderWorker.h"

namespace CSV {

/**
 * @brief Owns the CSV player's background indexing run (spec 0022): the worker thread, the
 *        generation that fences stale results, and the cancel + join handshake that must
 *        complete before the mapping the worker reads may be unmapped. Batches and the finish
 *        verdict are forwarded; every playback decision stays with the owner.
 */
class FileIndexer : public QObject {
  Q_OBJECT

signals:
  void batchReady(const CSV::PlayerIndexBatchPtr& batch);
  void finished(bool ok, quint64 generation);

public:
  explicit FileIndexer(QObject* parent = nullptr);

  FileIndexer(FileIndexer&&)                 = delete;
  FileIndexer(const FileIndexer&)            = delete;
  FileIndexer& operator=(FileIndexer&&)      = delete;
  FileIndexer& operator=(const FileIndexer&) = delete;

  ~FileIndexer();

  [[nodiscard]] bool indexing() const;
  [[nodiscard]] double progress() const;
  [[nodiscard]] quint64 generation() const;

  void start(const PlayerIndexRequestPtr& request);
  [[nodiscard]] bool stop(QObject* mappingOwner);

private slots:
  void onWorkerBatch(const CSV::PlayerIndexBatchPtr& batch);
  void onWorkerFinished(bool ok, quint64 generation);

private:
  bool m_indexing;
  qint64 m_size;
  qint64 m_bytesIndexed;
  quint64 m_generation;

  QThread* m_thread;
  PlayerLoaderWorker* m_worker;
};

}  // namespace CSV
