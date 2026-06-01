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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QMutex>
#include <QObject>
#include <vector>

class QThread;
class QTimer;

namespace DataModel {

class JsWatchdog;

/**
 * @brief Timer-driven worker that interrupts expired watchdogs off the blocked thread.
 */
class JsWatchdogWorker : public QObject {
  Q_OBJECT

public:
  explicit JsWatchdogWorker(QObject* parent = nullptr);

public slots:
  void begin();
  void finish();

private slots:
  void onTick();

private:
  QTimer* m_timer;
};

/**
 * @brief Singleton background thread that aborts runaway QJSEngine executions.
 */
class JsWatchdogThread {
public:
  [[nodiscard]] static JsWatchdogThread& instance();

  JsWatchdogThread(const JsWatchdogThread&)            = delete;
  JsWatchdogThread& operator=(const JsWatchdogThread&) = delete;

  void registerWatchdog(JsWatchdog* watchdog);
  void unregisterWatchdog(JsWatchdog* watchdog);

  void interruptExpired();

  void shutdown();

private:
  JsWatchdogThread();
  ~JsWatchdogThread();

  void ensureStarted();

private:
  QThread* m_thread;
  JsWatchdogWorker* m_worker;
  QMutex m_mutex;
  std::vector<JsWatchdog*> m_watchdogs;
  bool m_started;
  bool m_stopped;
};

}  // namespace DataModel
