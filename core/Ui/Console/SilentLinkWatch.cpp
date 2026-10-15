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

#include "Console/SilentLinkWatch.h"

#include "AppState.h"
#include "Console/WelcomeText.h"
#include "Core/SSAssert.h"
#include "DataModel/PipelineModules.h"
#include "IO/ConnectionManager.h"

/**
 * @brief How long a fresh connection may stay frameless before the console hints at a cause.
 */
static constexpr int kSilenceMs = 10000;

/**
 * @brief Returns @p current minus @p baseline, or @p current when a recreated frame reader
 *        restarted the counter below the baseline.
 */
[[nodiscard]] static quint64 counterDelta(const quint64 current, const quint64 baseline)
{
  return current >= baseline ? current - baseline : current;
}

/**
 * @brief Arms the watch on every connect and disarms it on every disconnect.
 */
Console::SilentLinkWatch::SilentLinkWatch(IO::ConnectionManager& connectionManager, QObject* parent)
  : QObject(parent), m_connectionManager(connectionManager), m_baseline{}
{
  m_timer.setSingleShot(true);
  m_timer.setInterval(kSilenceMs);
  connect(&m_timer, &QTimer::timeout, this, &Console::SilentLinkWatch::onTimeout);
  connect(&m_connectionManager,
          &IO::ConnectionManager::connectedChanged,
          this,
          &Console::SilentLinkWatch::onConnectedChanged);
}

/**
 * @brief Snapshots the link counters and starts the silence timer on connect; stops it otherwise.
 */
void Console::SilentLinkWatch::onConnectedChanged()
{
  m_timer.stop();
  if (!m_connectionManager.isConnected())
    return;

  m_baseline = m_connectionManager.linkStats();
  m_timer.start();
}

/**
 * @brief Emits the hint when the connection is still up, the mode parses frames, and no frame
 *        was extracted since connecting; bytes-or-not picks which diagnosis the hint gives.
 */
void Console::SilentLinkWatch::onTimeout()
{
  SS_ASSERT(m_timer.interval() == kSilenceMs, return);
  if (!m_connectionManager.isConnected())
    return;

  if (DataModel::pipelineModules().appState.operationMode() == SerialStudio::ConsoleOnly)
    return;

  const auto now = m_connectionManager.linkStats();
  if (counterDelta(now.framesExtracted, m_baseline.framesExtracted) > 0)
    return;

  Q_EMIT hint(Console::noDataHint(counterDelta(now.bytesIn, m_baseline.bytesIn) > 0));
}
