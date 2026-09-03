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

#include "DataModel/ReplayPlaybackEngine.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QInputMethod>
#include <QInputMethodQueryEvent>
#include <QtGlobal>

#include "SSAssert.h"

/**
 * @brief Whether another widget owns @p key, so a player must not steal it: a modal is up, or the
 *        focused item accepts text input. Media keys are never claimed, being global by
 *        definition. Shared by all three players, which install the same app-wide event filter.
 */
bool DataModel::ReplayPlaybackEngine::playbackKeyIsClaimed(int key)
{
  const bool mediaKey = key == Qt::Key_MediaPlay || key == Qt::Key_MediaPause
                     || key == Qt::Key_MediaTogglePlayPause || key == Qt::Key_MediaPrevious
                     || key == Qt::Key_MediaNext;
  if (mediaKey)
    return false;

  if (QGuiApplication::modalWindow())
    return true;

  auto* focus = QGuiApplication::focusObject();
  if (!focus)
    return false;

  QInputMethodQueryEvent query(Qt::ImEnabled);
  QCoreApplication::sendEvent(focus, &query);
  return query.value(Qt::ImEnabled).toBool();
}

/**
 * @brief Builds the timer chain: a coalescing scrub tick at ~30 Hz and a settle pass that only
 *        fires once the slider rests. Both are single-shot, so a drag re-arms them instead of
 *        queueing a rebuild per slider sample.
 */
DataModel::ReplayPlaybackEngine::ReplayPlaybackEngine(QObject* parent)
  : QObject(parent)
  , m_epoch(0)
  , m_steadyBaseRowSeconds(0.0)
  , m_steadyBase(std::chrono::steady_clock::now())
{
  m_seekTimer.setSingleShot(true);
  m_seekTimer.setInterval(kSeekTickMs);
  m_settleTimer.setSingleShot(true);
  m_settleTimer.setInterval(kSeekSettleMs);

  connect(&m_seekTimer, &QTimer::timeout, this, &ReplayPlaybackEngine::seekTick);
  connect(&m_settleTimer, &QTimer::timeout, this, &ReplayPlaybackEngine::seekSettle);
}

/**
 * @brief Arms one scrub pass: the tick timer only starts when idle so a fast drag coalesces, the
 *        settle timer restarts on every sample so the exact rebuild waits for the slider to rest.
 */
void DataModel::ReplayPlaybackEngine::armSeek()
{
  if (!m_seekTimer.isActive())
    m_seekTimer.start();

  m_settleTimer.start();
}

/**
 * @brief Retires a pending scrub; a close or a play() must not land a settle pass on the state
 *        that replaced the one the drag was scrubbing.
 */
void DataModel::ReplayPlaybackEngine::stopSeek()
{
  m_seekTimer.stop();
  m_settleTimer.stop();
}

/**
 * @brief Invalidates the catch-up fill gate so the next catch-up pass fills immediately.
 */
void DataModel::ReplayPlaybackEngine::resetCatchUpFill()
{
  m_catchUpFill.invalidate();
}

/**
 * @brief Anchors the steady-clock base used to stamp replayed rows with recorded deltas: the
 *        recording, not the wall clock, owns replay time.
 */
void DataModel::ReplayPlaybackEngine::anchorSteadyBase(double rowSeconds)
{
  m_steadyBase           = std::chrono::steady_clock::now();
  m_steadyBaseRowSeconds = (rowSeconds >= 0.0) ? rowSeconds : 0.0;
}

/**
 * @brief Opens a new playback epoch and returns it. A pause/play cycle would otherwise run two
 *        timer chains at once, each injecting rows.
 */
quint64 DataModel::ReplayPlaybackEngine::nextEpoch() noexcept
{
  return ++m_epoch;
}

/**
 * @brief The epoch a timer chain armed now belongs to.
 */
quint64 DataModel::ReplayPlaybackEngine::epoch() const noexcept
{
  return m_epoch;
}

/**
 * @brief True while @p epoch is still the live one; a chain that fails this must retire silently.
 */
bool DataModel::ReplayPlaybackEngine::isCurrentEpoch(quint64 epoch) const noexcept
{
  return epoch == m_epoch;
}

/**
 * @brief True when the catch-up path may spend a plot fill again, and restarts the gate. The fill
 *        is what makes a long scrub-forward look continuous; running it per injected row would
 *        cost more than the injection.
 */
bool DataModel::ReplayPlaybackEngine::catchUpFillDue()
{
  if (m_catchUpFill.isValid() && m_catchUpFill.elapsed() < kCatchUpFillMs)
    return false;

  m_catchUpFill.restart();
  return true;
}

/**
 * @brief The anchored steady base itself; the fallback stamp for a row with no usable time.
 */
std::chrono::steady_clock::time_point DataModel::ReplayPlaybackEngine::steadyBase() const noexcept
{
  return m_steadyBase;
}

/**
 * @brief Steady timestamp for a row at @p rowSeconds: the anchored base advanced by the recorded
 *        delta, so the gap between two replayed rows is the gap the recording captured.
 */
std::chrono::steady_clock::time_point DataModel::ReplayPlaybackEngine::steadyTimestampFor(
  double rowSeconds) const
{
  const auto delta = std::chrono::duration<double>(rowSeconds - m_steadyBaseRowSeconds);
  return m_steadyBase + std::chrono::duration_cast<std::chrono::steady_clock::duration>(delta);
}

/**
 * @brief Renders elapsed seconds as HH:MM:SS.mmm for the transport label; a negative input clamps
 *        to zero rather than printing a minus sign the tape can never reach.
 */
QString DataModel::ReplayPlaybackEngine::formatTimestamp(double seconds)
{
  constexpr double kInvHour = 1.0 / 3600.0;
  constexpr double kInvMin  = 1.0 / 60.0;

  const int hours   = static_cast<int>(seconds * kInvHour);
  const int minutes = static_cast<int>((seconds - hours * 3600.0) * kInvMin);
  const double secs = seconds - hours * 3600.0 - minutes * 60.0;

  return QStringLiteral("%1:%2:%3")
    .arg(qMax(hours, 0), 2, 10, QChar('0'))
    .arg(qMax(minutes, 0), 2, 10, QChar('0'))
    .arg(qMax(secs, 0.0), 6, 'f', 3, QChar('0'));
}

/**
 * @brief First row of the scrub window ending at @p target: walks back until @p range seconds are
 *        covered, never fewer than @p points rows, capped at kMaxSeekWindowRows so a dense
 *        recording bounds the per-tick cost. A row whose time is unknown (negative) ends the walk.
 */
int DataModel::ReplayPlaybackEngine::seekWindowStartRow(int target,
                                                        int points,
                                                        double range,
                                                        const std::function<double(int)>& secondsAt)
{
  SS_ASSERT(target >= 0, return 0);
  SS_ASSERT(static_cast<bool>(secondsAt), return target);

  const double targetSec = secondsAt(target);
  const int minStart     = qMax(0, target - qMax(1, points) + 1);
  const int capStart     = qMax(0, target - kMaxSeekWindowRows + 1);

  int start = minStart;
  for (int i = 0; i < kMaxSeekWindowRows && targetSec >= 0.0 && start > capStart; ++i) {
    const double sec = secondsAt(start - 1);
    if (sec < 0.0 || targetSec - sec > range)
      break;

    --start;
  }

  return start;
}
