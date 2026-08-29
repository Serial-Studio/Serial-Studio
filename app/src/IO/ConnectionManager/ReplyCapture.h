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

#include <atomic>
#include <QByteArray>
#include <QHash>
#include <QMutex>

namespace IO {

/**
 * @brief The transmit-reply tap behind a control script's deviceWriteAndWait(): one buffer per
 *        armed device, filled from ConnectionManager's raw-data tap and polled until the script
 *        disarms. GUI-thread only today, yet the mutex moved here as one unit with the buffers it
 *        guards; armed() gates the tap down to one inlined atomic read per chunk when it is idle.
 */
class ReplyCapture {
public:
  ReplyCapture();
  ReplyCapture(ReplyCapture&&)                 = delete;
  ReplyCapture(const ReplyCapture&)            = delete;
  ReplyCapture& operator=(ReplyCapture&&)      = delete;
  ReplyCapture& operator=(const ReplyCapture&) = delete;

  void arm(int deviceId);
  void disarm(int deviceId);
  void record(int deviceId, const QByteArray& bytes);

  [[nodiscard]] QByteArray poll(int deviceId) const;

  /**
   * @brief True while at least one device has an armed buffer. Read once per raw chunk before
   *        anything is locked, so an idle session pays a single atomic load.
   */
  [[nodiscard]] inline bool armed() const noexcept
  {
    return m_armed.load(std::memory_order_acquire);
  }

private:
  std::atomic<bool> m_armed;
  mutable QMutex m_mutex;
  QHash<int, QByteArray> m_buffers;
};

}  // namespace IO
