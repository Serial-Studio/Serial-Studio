/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <cstddef>
#  include <vector>

#  include "Sessions/Player/ReplayClock.h"
#  include "Sessions/Player/SessionDbReader.h"

class AppState;

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace DataModel {
class FrameBuilder;
}  // namespace DataModel

namespace Sessions {

/**
 * @brief Turns a recorded instant into the frames the live pipeline ingests: stored cells read
 *        through the session reader, stamped with the replay clock, handed to FrameBuilder's
 *        replay lane so a replayed frame is indistinguishable downstream (spec 0020/0055). Both
 *        lanes carry the recording's own timestamps: the source owns time, replay included.
 */
class ReplaySynthesis {
public:
  explicit ReplaySynthesis(SessionDbReader& reader,
                           const ReplayLayout& layout,
                           AppState& appState,
                           DataModel::FrameBuilder& frameBuilder,
                           IO::ConnectionManager& connectionManager);

  ReplaySynthesis(ReplaySynthesis&&)                 = delete;
  ReplaySynthesis(const ReplaySynthesis&)            = delete;
  ReplaySynthesis& operator=(ReplaySynthesis&&)      = delete;
  ReplaySynthesis& operator=(const ReplaySynthesis&) = delete;

  [[nodiscard]] bool injecting() const noexcept;
  [[nodiscard]] ReplayRowValues buildFrameAt(qint64 timestampNs);

  void clear();
  void replayFrameAt(qint64 timestampNs);
  void anchorSteadyBase(double rowSeconds);
  void setStreamBlocks(std::vector<PlayerStreamBlockIndex> blocks);
  void injectFrame(const ReplayRowValues& frame, qint64 timestampNs);

private:
  void injectStreamBlocksAt(qint64 timestampNs);
  void replayStreamGroup(int sourceId, std::size_t first, std::size_t last);

private:
  bool m_injecting;
  ReplayClock m_clock;
  SessionDbReader& m_reader;
  const ReplayLayout& m_layout;
  AppState& m_appState;
  DataModel::FrameBuilder& m_frameBuilder;
  IO::ConnectionManager& m_connectionManager;
  std::vector<PlayerStreamBlockIndex> m_streamBlocks;
  std::vector<std::vector<double>> m_streamChannelBuf;
};

}  // namespace Sessions

#endif
