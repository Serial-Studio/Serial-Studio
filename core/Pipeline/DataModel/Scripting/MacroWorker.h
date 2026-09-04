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

#include <atomic>
#include <memory>
#include <QJSValue>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QJSEngine;

namespace API {
class CommandRegistry;
}  // namespace API

namespace DataModel {

class JsWatchdog;
class ControlApiMarshaller;

/**
 * @brief JS-facing bridge backing apiCall()/delay()/log() for macros; stop-aware variant of
 *        ControlApiBridge so the Stop button can break a sleeping or device-waiting macro.
 */
class MacroApiBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void logMessage(const QString& message);

public:
  explicit MacroApiBridge(ControlApiMarshaller* marshaller,
                          const std::atomic<bool>* stopFlag,
                          QObject* parent = nullptr);

  void setWatchdog(DataModel::JsWatchdog* watchdog);

  Q_INVOKABLE [[nodiscard]] QVariantMap call(const QString& method, const QVariantMap& params);
  Q_INVOKABLE [[nodiscard]] QVariantList listCommands();
  Q_INVOKABLE [[nodiscard]] QVariantMap writeAndWait(const QJSValue& data,
                                                     int timeoutMs,
                                                     const QJSValue& until,
                                                     int sourceId);

public slots:
  void log(const QString& message);
  void delay(int milliseconds);

private:
  [[nodiscard]] bool stopRequested() const noexcept;

private:
  ControlApiMarshaller* m_marshaller;
  DataModel::JsWatchdog* m_watchdog;
  const std::atomic<bool>* m_stop;
  API::CommandRegistry& m_registry;
};

/**
 * @brief Worker that evaluates one macro on its own thread in a fresh engine per run.
 */
class MacroWorker : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void finished(const QString& result);
  void scriptError(const QString& message);
  void logMessage(const QString& message);

public:
  explicit MacroWorker(ControlApiMarshaller* marshaller, QObject* parent = nullptr);
  ~MacroWorker() override;

  MacroWorker(MacroWorker&&)                 = delete;
  MacroWorker(const MacroWorker&)            = delete;
  MacroWorker& operator=(MacroWorker&&)      = delete;
  MacroWorker& operator=(const MacroWorker&) = delete;

  void resetStop() noexcept;
  void requestStop() noexcept;
  void requestTeardown() noexcept;

public slots:
  void run(const QString& source);

private:
  void releaseEngine();

private:
  std::atomic<bool> m_teardown;
  MacroApiBridge* m_bridge;
  std::atomic<bool> m_stopRequested;
  ControlApiMarshaller* m_marshaller;
  std::unique_ptr<QJSEngine> m_engine;
  std::unique_ptr<DataModel::JsWatchdog> m_watchdog;
};

}  // namespace DataModel
