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

#include "CSV/Player/FileIndexer.h"

#include <algorithm>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction & queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an idle indexer; no thread exists until start().
 */
CSV::FileIndexer::FileIndexer(QObject* parent)
  : QObject(parent)
  , m_indexing(false)
  , m_size(0)
  , m_bytesIndexed(0)
  , m_generation(0)
  , m_thread(nullptr)
  , m_worker(nullptr)
{}

/**
 * @brief Destructor - joins any in-flight run so the worker never outlives this object.
 */
CSV::FileIndexer::~FileIndexer()
{
  (void)stop(nullptr);
}

/**
 * @brief Returns whether a background run is still scanning.
 */
bool CSV::FileIndexer::indexing() const
{
  return m_indexing;
}

/**
 * @brief Returns scan progress in the range 0.0 to 1.0; an idle indexer reports complete.
 */
double CSV::FileIndexer::progress() const
{
  if (m_size <= 0)
    return 1.0;

  return std::clamp(static_cast<double>(m_bytesIndexed) / m_size, 0.0, 1.0);
}

/**
 * @brief Returns the generation of the current run; results carrying any other generation
 *        belong to a superseded file and are dropped.
 */
quint64 CSV::FileIndexer::generation() const
{
  return m_generation;
}

//--------------------------------------------------------------------------------------------------
// Run lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts the background indexer for @p request on a fresh worker thread, stamping it
 *        with the new generation. The caller guarantees the mapped bytes outlive the run.
 */
void CSV::FileIndexer::start(const PlayerIndexRequestPtr& request)
{
  SS_ASSERT(request != nullptr, return);
  SS_ASSERT(m_thread == nullptr, return);

  ++m_generation;
  request->generation = m_generation;

  m_thread = new QThread(this);
  m_thread->setObjectName(QStringLiteral("CSV::PlayerLoader"));
  m_worker = new PlayerLoaderWorker();
  m_worker->moveToThread(m_thread);

  connect(m_worker,
          &PlayerLoaderWorker::batchReady,
          this,
          &CSV::FileIndexer::onWorkerBatch,
          Qt::QueuedConnection);
  connect(m_worker,
          &PlayerLoaderWorker::finished,
          this,
          &CSV::FileIndexer::onWorkerFinished,
          Qt::QueuedConnection);

  m_thread->start();

  m_indexing     = true;
  m_size         = request->size;
  m_bytesIndexed = request->dataOffset;

  auto* loader = m_worker;
  QMetaObject::invokeMethod(
    loader, [loader, request]() { loader->indexFile(request); }, Qt::QueuedConnection);
}

/**
 * @brief Cancels and joins the indexer thread; true when the join succeeded, and only then
 *        may the caller unmap. A timed-out join detaches the thread for self-cleanup and
 *        hands it @p mappingOwner (if given) to delete, because destroying that owner here
 *        would unmap memory under a live reader: the caller releases its own ownership.
 */
bool CSV::FileIndexer::stop(QObject* mappingOwner)
{
  if (!m_thread)
    return true;

  constexpr int kJoinTimeoutMs = 5000;

  if (m_worker)
    m_worker->requestCancel();

  m_thread->quit();
  const bool joined = m_thread->wait(kJoinTimeoutMs);
  if (joined) {
    delete m_worker;
    delete m_thread;
  } else {
    qWarning() << "[CSV::FileIndexer] Indexer thread did not stop in time; detaching it.";
    disconnect(m_worker, nullptr, this, nullptr);
    m_thread->setParent(nullptr);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    if (mappingOwner)
      connect(m_thread, &QThread::finished, mappingOwner, &QObject::deleteLater);
  }

  m_worker       = nullptr;
  m_thread       = nullptr;
  m_indexing     = false;
  m_size         = 0;
  m_bytesIndexed = 0;
  return joined;
}

//--------------------------------------------------------------------------------------------------
// Worker results
//--------------------------------------------------------------------------------------------------

/**
 * @brief Advances the scanned-bytes counter from a live batch and forwards it to the owner.
 */
void CSV::FileIndexer::onWorkerBatch(const CSV::PlayerIndexBatchPtr& batch)
{
  SS_ASSERT(batch != nullptr, return);
  SS_ASSERT_LOG(m_generation > 0);

  if (batch->generation == m_generation)
    m_bytesIndexed = batch->bytesIndexed;

  Q_EMIT batchReady(batch);
}

/**
 * @brief Marks the run finished (a completed scan counts as fully read) and forwards the
 *        verdict; the thread stays alive until the owner's stop() joins it.
 */
void CSV::FileIndexer::onWorkerFinished(bool ok, quint64 generation)
{
  SS_ASSERT_LOG(m_generation > 0);

  if (generation == m_generation) {
    m_indexing = false;
    if (ok)
      m_bytesIndexed = m_size;
  }

  Q_EMIT finished(ok, generation);
}
