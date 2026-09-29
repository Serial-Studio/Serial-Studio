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

#include <array>
#include <atomic>
#include <QtGlobal>
#include <vector>

namespace DataModel {

struct Frame;

/**
 * @brief Per-source short-frame watch (spec 0088 R5): the highest enabled non-computed frame
 *        index each source expects vs the highest token count its frames delivered, so the 1 Hz
 *        link check can name starved datasets. Single pipeline-thread writer, fixed storage,
 *        count published last: the GUI's pull reads torn-benign ints and never races a rebuild.
 */
class ShortFrameWatch {
public:
  /**
   * @brief One source's watch entry; watermark < maxIndex means datasets above the watermark
   *        are starved (session-monotonic, reset on connect and mode change).
   */
  struct Stat {
    int sourceId;
    int maxIndex;
    int watermark;
  };

  static constexpr int kCap = 16;

  ShortFrameWatch();
  ShortFrameWatch(ShortFrameWatch&&)                 = delete;
  ShortFrameWatch(const ShortFrameWatch&)            = delete;
  ShortFrameWatch& operator=(ShortFrameWatch&&)      = delete;
  ShortFrameWatch& operator=(const ShortFrameWatch&) = delete;

  void rebuild(const Frame& frame);
  void resetWatermarks() noexcept;
  void note(int sourceId, qsizetype count) noexcept;
  [[nodiscard]] std::vector<Stat> stats() const;

private:
  std::array<Stat, kCap> m_entries;
  std::array<qint8, kCap> m_directMap;
  std::atomic<int> m_count;
};

}  // namespace DataModel
