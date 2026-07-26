/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QAbstractSocket>
#include <QIODevice>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

#include "Async/AsyncClock.h"
#include "Async/RetryPolicy.h"
#include "Async/TaskTree.h"
#include "IO/HAL_Driver.h"

namespace IO {
/**
 * @brief One open attempt expressed as a task-tree step: starts the driver's open and finishes
 *        on the outcome the driver reports, so a failure carries the driver's own reason rather
 *        than a bare timeout.
 */
class DriverOpenTask final : public Async::Task {
  Q_OBJECT

public:
  DriverOpenTask(HAL_Driver* driver, QIODevice::OpenMode mode, QObject* parent = nullptr);
  ~DriverOpenTask() override;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void releaseConnection();
  void onOpenFinished(bool ok, const QString& reason);

private:
  QIODevice::OpenMode m_mode;
  QPointer<HAL_Driver> m_driver;
  QMetaObject::Connection m_connection;
};

/**
 * @brief One socket connect attempt expressed as a task-tree step. The dial is issued from the
 *        step's own start, after its outcome signals are connected, so a connect that completes
 *        inside connectToHost() cannot be missed the way a separate dial-then-wait pair would.
 */
class SocketConnectTask final : public Async::Task {
  Q_OBJECT

public:
  SocketConnectTask(QAbstractSocket* socket, QString host, quint16 port, QObject* parent = nullptr);
  ~SocketConnectTask() override;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void onConnected();
  void onSocketError();
  void releaseConnections();
  void beginDial(int generation);
  [[nodiscard]] bool hasLiveConnection() const;

private:
  quint16 m_port;
  int m_generation;
  QString m_host;
  QPointer<QAbstractSocket> m_socket;
  QMetaObject::Connection m_okConnection;
  QMetaObject::Connection m_errorConnection;
};

/**
 * @brief Keeps a link up: runs the open flow, then watches the driver for an unsolicited drop
 *        and re-runs it. Attempts and drops emit nothing from here, so a recovering link cannot
 *        amplify connection-state churn.
 */
class SupervisorTask final : public Async::Task {
  Q_OBJECT

public:
  SupervisorTask(HAL_Driver* driver,
                 Async::Task* child,
                 const Async::RetryPolicy& recovery,
                 QObject* parent = nullptr);
  ~SupervisorTask() override;

  [[nodiscard]] int attempt() const;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void onLinkDropped();
  void releaseConnection();
  void onChildFinished(Async::Outcome outcome, const Async::StepError& error);

private:
  Async::Task* m_child;
  QPointer<HAL_Driver> m_driver;
  Async::RetryPolicy m_recovery;
  QMetaObject::Connection m_connection;
};

namespace Flows {
[[nodiscard]] Async::Task* makeOpenFlow(HAL_Driver* driver,
                                        QIODevice::OpenMode mode,
                                        Async::AsyncClock& clock);

[[nodiscard]] Async::Task* makeSupervised(HAL_Driver* driver,
                                          Async::Task* flow,
                                          const Async::RetryPolicy& policy,
                                          const Async::RetryPolicy& recovery,
                                          Async::AsyncClock& clock);

[[nodiscard]] Async::Task* makeSocketConnect(QAbstractSocket* socket,
                                             const QString& host,
                                             quint16 port,
                                             int timeout_msec,
                                             Async::AsyncClock& clock);
}  // namespace Flows
}  // namespace IO
