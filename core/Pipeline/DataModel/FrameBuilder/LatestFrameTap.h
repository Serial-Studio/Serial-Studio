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

#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <QHash>
#include <QObject>
#include <QStringList>
#include <vector>

#include "Core/ParseBudget.h"
#include "IO/HAL_Driver.h"
#include "ThirdParty/readerwriterqueue.h"

namespace DataModel {

/**
 * @brief Latest received frame snapshot for script/API consumers: the raw chunk (a retained
 *        FrameReader pool reference), the parser's channel tokens, and capture sequence
 *        numbers. Channel tokens are valid only when channelsSequence == sequence.
 */
struct LatestFrameInfo {
  LatestFrameInfo();
  int sourceId;
  quint64 sequence;
  qint64 timestampMs;
  quint64 channelsSequence;
  QStringList channels;
  IO::CapturedDataPtr chunk;
};

/**
 * @brief GUI-ward mirrors of the frame builder's two pulled diagnostics: the latest capture
 *        behind io.getLatestFrame, and the 1 Hz parse-load rows. The publish calls are
 *        builder-thread, the drain and gui readers GUI-thread, so a GUI reader never marshals
 *        into the pipeline: that nested loop is re-entrant on macOS and drops resize steps.
 */
class LatestFrameTap {
public:
  using ParseLoad = DataModel::ParseBudget::Load;

  LatestFrameTap(const QObject& owner,
                 const QHash<int, LatestFrameInfo>& captures,
                 const int& newestSourceId,
                 const quint64& captureSequence,
                 const DataModel::ParseBudget& budget);
  LatestFrameTap(LatestFrameTap&&)                 = delete;
  LatestFrameTap(const LatestFrameTap&)            = delete;
  LatestFrameTap& operator=(LatestFrameTap&&)      = delete;
  LatestFrameTap& operator=(const LatestFrameTap&) = delete;

  [[nodiscard]] bool drainForGui();
  [[nodiscard]] LatestFrameInfo guiLatestFrame(int sourceId);
  [[nodiscard]] std::vector<ParseLoad> guiParseLoads();

  void publish();
  void publishParseLoads();

private:
  /**
   * @brief GUI-side copy of every source's latest capture, published by the builder thread at
   *        display-tick rate so an API handler serving a script never marshals into the pipeline.
   */
  struct LatestFrameMirror {
    int newestSourceId;
    QHash<int, LatestFrameInfo> frames;
  };

  using ParseLoadsPtr        = std::shared_ptr<const std::vector<ParseLoad>>;
  using LatestFrameMirrorPtr = std::shared_ptr<const LatestFrameMirror>;

  static constexpr std::size_t kMirrorSlots    = 4;
  static constexpr std::size_t kParseLoadSlots = 4;

  const QObject& m_owner;
  const QHash<int, LatestFrameInfo>& m_captures;
  const int& m_newestSourceId;
  const quint64& m_captureSequence;
  const DataModel::ParseBudget& m_budget;

  quint64 m_publishedSequence;

  // code-verify off
  // One is armed once by the first GUI-thread reader, the other toggles once per display tick.
  // No steady-state cross-core write traffic, so sharing a cache line is harmless.
  std::atomic<bool> m_guiUsers;
  std::atomic<bool> m_publishRequested;
  // code-verify on

  LatestFrameMirrorPtr m_guiMirror;
  moodycamel::ReaderWriterQueue<LatestFrameMirrorPtr> m_mirrorRing;

  ParseLoadsPtr m_guiParseLoads;
  moodycamel::ReaderWriterQueue<ParseLoadsPtr> m_parseLoadRing;
};

}  // namespace DataModel
