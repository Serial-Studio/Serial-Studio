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

#include <memory>
#include <QJSValue>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QTimer;
class QJSEngine;

namespace DataModel {

class JsWatchdog;

/**
 * @brief GUI-thread sink that runs an apiCall command and returns the wrapped result.
 */
class ControlApiMarshaller : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit ControlApiMarshaller(QObject* parent = nullptr);

  Q_INVOKABLE [[nodiscard]] QVariantMap dispatch(const QString& method, const QVariantMap& params);
  Q_INVOKABLE [[nodiscard]] qint64 writeAndArm(int sourceId, const QByteArray& data);
  Q_INVOKABLE [[nodiscard]] QByteArray pollReply(int sourceId);

public slots:
  void disarmReply(int sourceId);
};

/**
 * @brief JS-facing bridge backing apiCall() and delay(), running on the worker thread.
 */
class ControlApiBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit ControlApiBridge(ControlApiMarshaller* marshaller, QObject* parent = nullptr);

  void setWatchdog(DataModel::JsWatchdog* watchdog);

  Q_INVOKABLE [[nodiscard]] QVariantMap call(const QString& method, const QVariantMap& params);
  Q_INVOKABLE [[nodiscard]] QVariantList listCommands();
  Q_INVOKABLE [[nodiscard]] QVariantMap writeAndWait(const QJSValue& data,
                                                     int timeoutMs,
                                                     const QJSValue& until,
                                                     int sourceId);

public slots:
  void delay(int milliseconds);

private:
  ControlApiMarshaller* m_marshaller;
  DataModel::JsWatchdog* m_watchdog;
};

/**
 * @brief Worker that runs the user control script's setup()/loop() on its own thread.
 */
class ControlScriptWorker : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void scriptError(const QString& message);

public:
  explicit ControlScriptWorker(ControlApiMarshaller* marshaller, QObject* parent = nullptr);
  ~ControlScriptWorker() override;

  ControlScriptWorker(ControlScriptWorker&&)                 = delete;
  ControlScriptWorker(const ControlScriptWorker&)            = delete;
  ControlScriptWorker& operator=(ControlScriptWorker&&)      = delete;
  ControlScriptWorker& operator=(const ControlScriptWorker&) = delete;

  static void requestShutdown() noexcept;

public slots:
  void start(const QString& source);
  void stop();

private slots:
  void runLoopTick();

private:
  [[nodiscard]] bool compile(const QString& source, QString& errorOut);
  void releaseEngine();
  void runSetup();

private:
  bool m_loaded;
  QJSValue m_setupFn;
  QJSValue m_loopFn;
  QTimer* m_loopTimer;
  ControlApiBridge* m_bridge;
  ControlApiMarshaller* m_marshaller;
  std::unique_ptr<QJSEngine> m_engine;
  std::unique_ptr<DataModel::JsWatchdog> m_watchdog;
};

}  // namespace DataModel
