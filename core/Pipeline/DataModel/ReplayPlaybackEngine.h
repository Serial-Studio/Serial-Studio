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

#include <chrono>
#include <functional>
#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QTimer>

namespace DataModel {

/**
 * @brief The playback mechanics the CSV, MDF4 and Sessions players share (spec 0075, B11): the
 *        scrub timer chain, the playback epoch that retires a previous play()'s chain, the
 *        steady-clock anchor that makes the RECORDING own replay time, the catch-up fill gate and
 *        the trailing seek-window walk. Composed, not inherited: the players differ in storage.
 */
class ReplayPlaybackEngine : public QObject {
  Q_OBJECT

signals:
  void seekTick();
  void seekSettle();

public:
  static constexpr int kSeekTickMs         = 33;
  static constexpr int kSeekSettleMs       = 250;
  static constexpr int kMaxSeekWindowRows  = 262144;
  static constexpr int kCatchUpScanMax     = 262144;
  static constexpr int kCatchUpFillMs      = 250;
  static constexpr qint64 kCatchUpBudgetMs = 20;

  [[nodiscard]] static bool playbackKeyIsClaimed(int key);

  explicit ReplayPlaybackEngine(QObject* parent = nullptr);

  ReplayPlaybackEngine(ReplayPlaybackEngine&&)                 = delete;
  ReplayPlaybackEngine(const ReplayPlaybackEngine&)            = delete;
  ReplayPlaybackEngine& operator=(ReplayPlaybackEngine&&)      = delete;
  ReplayPlaybackEngine& operator=(const ReplayPlaybackEngine&) = delete;

  void armSeek();
  void stopSeek();
  void resetCatchUpFill();
  void anchorSteadyBase(double rowSeconds);

  [[nodiscard]] quint64 nextEpoch() noexcept;
  [[nodiscard]] quint64 epoch() const noexcept;
  [[nodiscard]] bool isCurrentEpoch(quint64 epoch) const noexcept;
  [[nodiscard]] bool catchUpFillDue();

  [[nodiscard]] std::chrono::steady_clock::time_point steadyBase() const noexcept;
  [[nodiscard]] std::chrono::steady_clock::time_point steadyTimestampFor(double rowSeconds) const;

  [[nodiscard]] static QString formatTimestamp(double seconds);
  [[nodiscard]] static int seekWindowStartRow(int target,
                                              int points,
                                              double range,
                                              const std::function<double(int)>& secondsAt);

private:
  quint64 m_epoch;
  double m_steadyBaseRowSeconds;
  QTimer m_seekTimer;
  QTimer m_settleTimer;
  QElapsedTimer m_catchUpFill;
  std::chrono::steady_clock::time_point m_steadyBase;
};

}  // namespace DataModel
