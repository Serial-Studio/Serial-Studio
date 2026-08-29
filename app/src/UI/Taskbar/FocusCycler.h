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
#include <QVector>

class QQuickItem;

namespace UI {

/**
 * @brief Drives the focus ripple that runs across the tiles once a workspace finishes registering
 *        its windows: each tile is activated in turn on a timer whose interval is derived from a
 *        fixed total budget, so the ripple takes the same wall time whatever the tile count. The
 *        cycler decides when a tile takes focus; the taskbar decides what taking focus means.
 */
class FocusCycler : public QObject {
  Q_OBJECT

signals:
  void focusCleared();
  void focusRequested(QQuickItem* window);
  void focusRefreshRequested(QQuickItem* window);

public:
  explicit FocusCycler(QObject* parent = nullptr);
  FocusCycler(FocusCycler&&)                 = delete;
  FocusCycler(const FocusCycler&)            = delete;
  FocusCycler& operator=(FocusCycler&&)      = delete;
  FocusCycler& operator=(const FocusCycler&) = delete;

  [[nodiscard]] bool running() const;

public slots:
  void stop();
  void remove(QQuickItem* window);
  void start(const QVector<QQuickItem*>& windows);

private slots:
  void onTick();

private:
  QTimer m_timer;
  QVector<QQuickItem*> m_queue;
};

}  // namespace UI
