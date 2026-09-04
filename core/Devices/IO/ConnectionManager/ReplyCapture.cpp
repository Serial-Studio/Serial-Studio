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

#include "IO/ConnectionManager/ReplyCapture.h"

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a disarmed capture: no buffers, so the raw-data tap skips it entirely.
 */
IO::ReplyCapture::ReplyCapture() : m_armed(false) {}

//--------------------------------------------------------------------------------------------------
// Arming & draining
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens an empty capture buffer for @p deviceId and publishes the armed flag. The caller
 *        writes to the device only after this returns, so the tap is live before the request
 *        leaves and no part of the reply can be missed. Reached from a script worker's
 *        BlockingQueued marshal, so it runs on the GUI thread like every other entry point here.
 */
void IO::ReplyCapture::arm(int deviceId)
{
  SS_ASSERT(deviceId >= 0, return);

  {
    QMutexLocker locker(&m_mutex);
    m_buffers.insert(deviceId, QByteArray());
  }
  m_armed.store(true, std::memory_order_release);
}

/**
 * @brief Drops the capture buffer for @p deviceId and disarms the tap once no buffers remain, so
 *        the steady-state raw-data path pays only a single atomic read.
 */
void IO::ReplyCapture::disarm(int deviceId)
{
  SS_ASSERT(deviceId >= 0, return);

  QMutexLocker locker(&m_mutex);
  m_buffers.remove(deviceId);
  if (m_buffers.isEmpty())
    m_armed.store(false, std::memory_order_release);
}

/**
 * @brief Accumulates @p bytes into @p deviceId's buffer when one is armed, called from the GUI
 *        thread's raw-data tap. Bytes for an unarmed device are dropped on purpose: the capture
 *        exists only for the device a script is waiting on.
 */
void IO::ReplyCapture::record(int deviceId, const QByteArray& bytes)
{
  SS_ASSERT_LOG(deviceId >= 0);
  SS_ASSERT_LOG(!bytes.isEmpty());

  QMutexLocker locker(&m_mutex);
  auto it = m_buffers.find(deviceId);
  if (it != m_buffers.end())
    it->append(bytes);
}

/**
 * @brief Returns a copy of the bytes captured for @p deviceId since the last arm. The buffer is
 *        not cleared: a script polls until the reply satisfies it, then disarms.
 */
QByteArray IO::ReplyCapture::poll(int deviceId) const
{
  SS_ASSERT(deviceId >= 0, return {});

  QMutexLocker locker(&m_mutex);
  return m_buffers.value(deviceId);
}
