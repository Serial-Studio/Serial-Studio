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

#include <QAbstractSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QIODevice>
#include <QList>
#include <QPointer>
#include <QString>
#include <QTcpSocket>
#include <QTimer>

namespace IO {

/**
 * @brief Sequences one TCP dial off the blocking path: async resolution, a paced refusal probe on
 *        throwaway sockets, then one connectToHost() on the caller's socket. finished(ok, reason)
 *        lands exactly once per start() under a single deadline and cancel() ends an attempt with
 *        no verdict; retry churn stays on the probes, never on the caller's socket.
 */
class AsyncTcpDial : public QObject {
  Q_OBJECT

signals:
  void finished(bool ok, const QString& reason);

public:
  explicit AsyncTcpDial(QObject* parent = nullptr);
  AsyncTcpDial(AsyncTcpDial&&)                 = delete;
  AsyncTcpDial(const AsyncTcpDial&)            = delete;
  AsyncTcpDial& operator=(AsyncTcpDial&&)      = delete;
  AsyncTcpDial& operator=(const AsyncTcpDial&) = delete;

  ~AsyncTcpDial() override;

  [[nodiscard]] bool active() const noexcept;
  [[nodiscard]] int deadline() const noexcept;
  [[nodiscard]] QHostAddress resolvedAddress() const noexcept;

public slots:
  void cancel();
  void setDeadline(int milliseconds);
  void setProbeEnabled(bool enabled);
  void startProbe(const QString& host, quint16 port);
  void startResolve(const QString& host, quint16 port);
  void start(const QString& host,
             quint16 port,
             QTcpSocket* socket,
             QIODevice::OpenMode mode = QIODevice::ReadWrite);

private slots:
  void onDeadlineReached();
  void onPaceElapsed();
  void onProbeConnected();
  void onSocketConnected();
  void onLookupFinished(const QHostInfo& info);
  void onProbeError(QAbstractSocket::SocketError error);
  void onSocketError(QAbstractSocket::SocketError error);

private:
  void dropProbe();
  void dialSocket();
  void probeNextAddress();
  void releaseSocket();
  void beginAttempt(const QString& host, quint16 port);
  void report(bool ok, const QString& reason);
  void applyResolution(const QList<QHostAddress>& addresses);

private:
  bool m_active;
  bool m_probeEnabled;
  bool m_refusedRound;
  int m_lookupId;
  int m_deadlineMs;
  int m_addressIndex;
  quint16 m_port;
  QString m_host;
  QIODevice::OpenMode m_mode;
  QHostAddress m_target;
  QList<QHostAddress> m_addresses;
  QTcpSocket* m_probe;
  QPointer<QTcpSocket> m_socket;
  QTimer m_deadlineTimer;
  QTimer m_paceTimer;
  QMetaObject::Connection m_socketConnected;
  QMetaObject::Connection m_socketFailed;
};

}  // namespace IO
