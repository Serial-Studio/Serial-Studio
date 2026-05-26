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

#include <atomic>
#include <QJSEngine>
#include <QJSValue>
#include <QString>

namespace DataModel {

/**
 * @brief Runtime watchdog for QJSEngine calls, interrupted from a separate thread.
 */
class JsWatchdog {
public:
  explicit JsWatchdog(QJSEngine* engine, int budgetMs = 1000, QString tag = QString());
  ~JsWatchdog();

  JsWatchdog(const JsWatchdog&)            = delete;
  JsWatchdog& operator=(const JsWatchdog&) = delete;

  void arm() noexcept;
  void disarm() noexcept;

  [[nodiscard]] QJSValue call(QJSValue& fn, const QJSValueList& args);
  [[nodiscard]] QJSValue call(QJSValue& fn, QJSValue thisObj, const QJSValueList& args);

  [[nodiscard]] int budgetMs() const noexcept;
  void setBudgetMs(int ms) noexcept;

  [[nodiscard]] bool lastCallTimedOut() const noexcept;

  [[nodiscard]] QJSEngine* engine() const noexcept;
  [[nodiscard]] qint64 deadlineNs() const noexcept;

private:
  QJSEngine* m_engine;
  std::atomic<qint64> m_deadlineNs;
  int m_budgetMs;
  QString m_tag;
  bool m_lastTimedOut;
};

}  // namespace DataModel
