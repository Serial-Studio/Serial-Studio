/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "FrameConsumer.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

DataModel::FrameConsumerWorkerBase::FrameConsumerWorkerBase(QObject* parent)
  : QObject(parent)
  , m_lastFrameNs(-1)
{
}

DataModel::FrameConsumerWorkerBase::~FrameConsumerWorkerBase() = default;

//--------------------------------------------------------------------------------------------------
// Monotonic clock tracker — shared by every export worker
//--------------------------------------------------------------------------------------------------

qint64 DataModel::FrameConsumerWorkerBase::monotonicFrameNs(
  std::chrono::steady_clock::time_point now, std::chrono::steady_clock::time_point baseline)
{
  qint64 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - baseline).count();
  if (ns <= m_lastFrameNs)
    ns = m_lastFrameNs + 1;

  m_lastFrameNs = ns;
  return ns;
}

void DataModel::FrameConsumerWorkerBase::resetMonotonicClock()
{
  m_lastFrameNs = -1;
}
