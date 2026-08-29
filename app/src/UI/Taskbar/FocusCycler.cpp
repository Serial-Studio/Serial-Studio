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

#include "UI/Taskbar/FocusCycler.h"

#include <QQuickItem>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Total ripple wall time and the per-tile interval clamps, so any tile count reads as one gesture
constexpr int kBudgetMs = 200;
constexpr int kMinMs    = 10;
constexpr int kMaxMs    = 40;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an idle cycler whose repeating timer drives one tile per tick.
 */
UI::FocusCycler::FocusCycler(QObject* parent) : QObject(parent)
{
  m_timer.setSingleShot(false);
  connect(&m_timer, &QTimer::timeout, this, &UI::FocusCycler::onTick);
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a ripple is currently in flight.
 */
bool UI::FocusCycler::running() const
{
  return m_timer.isActive();
}

//--------------------------------------------------------------------------------------------------
// Control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cancels any ripple in flight and drops its queue.
 */
void UI::FocusCycler::stop()
{
  m_timer.stop();
  m_queue.clear();
}

/**
 * @brief Drops a window from a ripple in flight, for a tile that is being unregistered.
 */
void UI::FocusCycler::remove(QQuickItem* window)
{
  m_queue.removeAll(window);
}

/**
 * @brief Starts a ripple over @a windows, in visual order. Nothing to ripple clears the focus, a
 *        single tile takes it directly, and anything longer revisits the first tile at the end so
 *        the ripple lands where it started.
 */
void UI::FocusCycler::start(const QVector<QQuickItem*>& windows)
{
  stop();

  m_queue = windows;
  if (m_queue.isEmpty()) {
    Q_EMIT focusCleared();
    return;
  }

  if (m_queue.size() == 1) {
    QQuickItem* only = m_queue.first();
    m_queue.clear();
    Q_EMIT focusRequested(only);
    return;
  }

  m_queue.append(m_queue.first());
  m_timer.setInterval(qBound(kMinMs, kBudgetMs / m_queue.size(), kMaxMs));
  m_timer.start();
}

/**
 * @brief Advances the ripple by one tile, stopping when the queue drains.
 */
void UI::FocusCycler::onTick()
{
  if (m_queue.isEmpty()) {
    m_timer.stop();
    return;
  }

  QQuickItem* window = m_queue.takeFirst();
  if (window)
    Q_EMIT focusRefreshRequested(window);

  if (m_queue.isEmpty())
    m_timer.stop();
}
