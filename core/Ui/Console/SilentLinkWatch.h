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

#include <QObject>
#include <QTimer>

#include "Core/IO/LinkStats.h"

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace Console {

/**
 * @brief Watches each new connection for silence (spec 0100 R5): 10 s after connecting, if the
 *        link has produced no frame in a parsing mode, emits one console hint naming the likely
 *        cause. Connection-rate only; it reads the polled link counters, never the frame path.
 */
class SilentLinkWatch : public QObject {
  Q_OBJECT

signals:
  void hint(const QString& text);

public:
  explicit SilentLinkWatch(IO::ConnectionManager& connectionManager, QObject* parent = nullptr);

private slots:
  void onConnectedChanged();
  void onTimeout();

private:
  IO::ConnectionManager& m_connectionManager;
  QTimer m_timer;
  IO::LinkStats m_baseline;
};

}  // namespace Console
