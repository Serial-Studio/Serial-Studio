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

#include "DataModel/FrameBuilder/LatestFrameTap.h"

#include <QCoreApplication>
#include <QThread>
#include <utility>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Default-constructs an empty latest-frame snapshot (no chunk, sequence 0).
 */
DataModel::LatestFrameInfo::LatestFrameInfo()
  : sourceId(-1), sequence(0), timestampMs(0), channelsSequence(0)
{}

/**
 * @brief Binds the tap to the builder-owned capture state it mirrors. Every dependency arrives by
 *        reference: the capture map, its newest-source and sequence counters, and the parse-load
 *        governor all keep living with the builder, which is the only thread that writes them.
 */
DataModel::LatestFrameTap::LatestFrameTap(const QObject& owner,
                                          const QHash<int, LatestFrameInfo>& captures,
                                          const int& newestSourceId,
                                          const quint64& captureSequence,
                                          const DataModel::ParseBudget& budget)
  : m_owner(owner)
  , m_captures(captures)
  , m_newestSourceId(newestSourceId)
  , m_captureSequence(captureSequence)
  , m_budget(budget)
  , m_publishedSequence(0)
  , m_guiUsers(false)
  , m_publishRequested(false)
  , m_mirrorRing(kMirrorSlots)
  , m_parseLoadRing(kParseLoadSlots)
{}

//--------------------------------------------------------------------------------------------------
// GUI thread
//--------------------------------------------------------------------------------------------------

/**
 * @brief GUI-side half of the latest-frame mirror: adopts the newest published copy and reports
 *        whether the caller still owes the builder a publish request. Gated on a GUI-thread reader
 *        having asked at least once, so a session without a polling script or API client never
 *        makes the builder pay for the copy.
 */
bool DataModel::LatestFrameTap::drainForGui()
{
  SS_ASSERT(qApp != nullptr, return false);
  SS_ASSERT(QThread::currentThread() == qApp->thread(), return false);

  if (!m_guiUsers.load(std::memory_order_relaxed)) [[likely]]
    return false;

  LatestFrameMirrorPtr mirror;
  // code-verify off
  // Ring drain: bounded by the mirror ring capacity (4), provably finite per tick.
  while (m_mirrorRing.try_dequeue(mirror))
    if (mirror)
      m_guiMirror = mirror;
  // code-verify on

  mirror.reset();

  return !m_publishRequested.exchange(true, std::memory_order_acq_rel);
}

/**
 * @brief GUI-thread read of the latest capture: serves the mirror the builder publishes on the
 *        display tick and arms it on first use. Marshaling here would spin a nested event loop
 *        inside the API dispatch and park the GUI behind the pipeline (spec 0051 M5 rule), which
 *        is what made a polling control script freeze the window's OS event handling.
 */
DataModel::LatestFrameInfo DataModel::LatestFrameTap::guiLatestFrame(int sourceId)
{
  m_guiUsers.store(true, std::memory_order_relaxed);

  const auto mirror = m_guiMirror;
  if (!mirror)
    return LatestFrameInfo();

  const int key = (sourceId >= 0) ? sourceId : mirror->newestSourceId;
  if (key < 0)
    return LatestFrameInfo();

  const auto it = mirror->frames.constFind(key);
  return (it != mirror->frames.constEnd()) ? it.value() : LatestFrameInfo();
}

/**
 * @brief GUI-thread read of the parse loads: adopts the newest published sample and serves it.
 *        One tick of staleness is inherent to a 1 Hz diagnostic.
 */
std::vector<DataModel::LatestFrameTap::ParseLoad> DataModel::LatestFrameTap::guiParseLoads()
{
  ParseLoadsPtr sample;
  // code-verify off
  // Ring drain: bounded by the mirror ring capacity (4), provably finite per call.
  while (m_parseLoadRing.try_dequeue(sample))
    if (sample)
      m_guiParseLoads = sample;
  // code-verify on

  sample.reset();
  return m_guiParseLoads ? *m_guiParseLoads : std::vector<ParseLoad>();
}

//--------------------------------------------------------------------------------------------------
// Builder thread
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builder-thread half of the latest-frame mirror: copies the capture map for the GUI when
 *        a new frame landed since the last publish. Runs at display-tick rate, never per frame.
 */
void DataModel::LatestFrameTap::publish()
{
  SS_ASSERT(QThread::currentThread() == m_owner.thread(), return);
  SS_ASSERT_LOG(m_captureSequence >= m_publishedSequence);

  m_publishRequested.store(false, std::memory_order_release);

  if (m_captureSequence == m_publishedSequence)
    return;

  auto mirror            = std::make_shared<LatestFrameMirror>();
  mirror->newestSourceId = m_newestSourceId;
  mirror->frames         = m_captures;

  if (!m_mirrorRing.try_enqueue(LatestFrameMirrorPtr(std::move(mirror)))) [[unlikely]]
    return;

  m_publishedSequence = m_captureSequence;
}

/**
 * @brief Builder-thread half of the parse-load mirror, published on the same 1 Hz tick the
 *        diagnostics sample at, so the GUI reader never marshals.
 */
void DataModel::LatestFrameTap::publishParseLoads()
{
  SS_ASSERT(QThread::currentThread() == m_owner.thread(), return);

  auto sample = std::make_shared<const std::vector<ParseLoad>>(m_budget.snapshot());
  (void)m_parseLoadRing.try_enqueue(std::move(sample));
}
