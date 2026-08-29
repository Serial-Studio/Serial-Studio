/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "DataModel/FrameBuilder/TableSnapshotChannel.h"

#include <QCoreApplication>
#include <QThread>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the mirror with every pool slot materialised up front: a publish must never
 *        allocate, and the pool outsizes the ring so the builder always has a slot to fill while
 *        the GUI still holds the ones in flight.
 */
DataModel::TableSnapshotChannel::TableSnapshotChannel(const QObject& owner,
                                                      DataModel::DataTableStore& store)
  : m_owner(owner)
  , m_store(store)
  , m_publishedGeneration(-1)
  , m_publishedClock(0)
  , m_poolHint(0)
  , m_guiUsers(false)
  , m_publishRequested(false)
  , m_mirrorRing(kMirrorSlots)
{
  m_pool.reserve(kPoolSlots);
  for (std::size_t i = 0; i < kPoolSlots; ++i)
    m_pool.emplace_back(std::make_shared<DataModel::DataTableSnapshot>());
}

//--------------------------------------------------------------------------------------------------
// GUI thread
//--------------------------------------------------------------------------------------------------

/**
 * @brief GUI-side half of the mirror: adopts the newest snapshot the builder published and reports
 *        whether the caller still owes the builder a publish request. Runs once per display tick
 *        before updated() reaches painter and output-widget scripts, so their tableGet/datasetGet
 *        calls read a GUI-local copy instead of parking the tick behind the pipeline thread.
 */
bool DataModel::TableSnapshotChannel::drainForGui()
{
  SS_ASSERT(qApp != nullptr, return false);
  SS_ASSERT(QThread::currentThread() == qApp->thread(), return false);

  if (!m_guiUsers.load(std::memory_order_relaxed)) [[likely]]
    return false;

  DataModel::DataTableSnapshotPtr snapshot;
  // code-verify off
  // Ring drain: bounded by the mirror ring capacity (4), provably finite per tick.
  while (m_mirrorRing.try_dequeue(snapshot))
    if (snapshot)
      m_guiSnapshot = snapshot;
  // code-verify on

  snapshot.reset();

  return !m_publishRequested.exchange(true, std::memory_order_acq_rel);
}

/**
 * @brief Arms the mirror when a script engine is wired up on the GUI thread. Engines injected on
 *        the pipeline thread (the parser and every dataset transform) read the live store
 *        directly, so they must not make the builder pay for a snapshot nobody reads.
 */
void DataModel::TableSnapshotChannel::noteGuiUser()
{
  if (qApp && QThread::currentThread() == qApp->thread())
    m_guiUsers.store(true, std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Builder thread
//--------------------------------------------------------------------------------------------------

/**
 * @brief Claims a free pooled snapshot slot, or null when every slot is in flight (the caller
 *        skips the publish and the next display-tick request retries the same state). The
 *        use_count probe is an atomic read and the acquire fence pairs with the GUI's release
 *        of its previously adopted snapshot, so slot reuse happens-after every consumer read.
 */
std::shared_ptr<DataModel::DataTableSnapshot> DataModel::TableSnapshotChannel::claimSlot()
{
  SS_ASSERT(!m_pool.empty(), return nullptr);

  const std::size_t n = m_pool.size();
  for (std::size_t k = 0; k < n; ++k) {
    const std::size_t idx = (m_poolHint + k) % n;
    if (m_pool[idx].use_count() != 1)
      continue;

    std::atomic_thread_fence(std::memory_order_acquire);
    m_poolHint = (idx + 1) % n;
    return m_pool[idx];
  }

  return nullptr;
}

/**
 * @brief Builder-thread half of the mirror: fills a reused pool slot from the store when its
 *        layout generation or write clock moved since the last publish, so the steady state
 *        allocates nothing. Runs on request at display-tick rate, never per frame. Pool
 *        exhaustion or a full ring leaves the bookkeeping untouched so the next request retries.
 */
void DataModel::TableSnapshotChannel::publish()
{
  SS_ASSERT(QThread::currentThread() == m_owner.thread(), return);
  SS_ASSERT(!m_pool.empty(), return);

  m_publishRequested.store(false, std::memory_order_release);

  const int generation = m_store.isInitialized() ? m_store.generation() : -1;
  const quint64 clock  = m_store.writeClock();
  if (generation == m_publishedGeneration && clock == m_publishedClock)
    return;

  const auto slot = claimSlot();
  if (!slot) [[unlikely]]
    return;

  m_store.snapshotInto(*slot);
  if (!m_mirrorRing.try_enqueue(DataModel::DataTableSnapshotPtr(slot, slot.get()))) [[unlikely]]
    return;

  m_publishedGeneration = generation;
  m_publishedClock      = clock;
}
