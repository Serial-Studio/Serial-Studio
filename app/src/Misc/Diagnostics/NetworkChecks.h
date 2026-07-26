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
#include <QList>
#include <QMetaObject>
#include <QString>
#include <QTcpSocket>

#include "Async/AsyncClock.h"
#include "Async/TaskTree.h"
#include "Misc/Diagnostics/DiagnosticsShared.h"

class QHostInfo;

namespace Misc::Diagnostics::NetworkChecks {

/**
 * @brief Step name a name-resolution failure is attributed to; the reachability reporter reads
 *        it instead of matching on an error string.
 */
inline constexpr auto kHostLookupStep = "host-lookup";

/**
 * @brief Step name a connect failure is attributed to.
 */
inline constexpr auto kTcpProbeStep = "tcp-probe";

/**
 * @brief Resolves one host name asynchronously. A literal address short-circuits to Success, and
 *        cancellation aborts the pending lookup instead of waiting it out.
 */
class HostLookupTask final : public Async::Task {
  Q_OBJECT

public:
  explicit HostLookupTask(QString host, QObject* parent = nullptr);
  HostLookupTask(HostLookupTask&&)                 = delete;
  HostLookupTask(const HostLookupTask&)            = delete;
  HostLookupTask& operator=(HostLookupTask&&)      = delete;
  HostLookupTask& operator=(const HostLookupTask&) = delete;
  ~HostLookupTask() override;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void abortLookup();
  void onLookupFinished(const QHostInfo& info);

private:
  int m_lookupId;
  QString m_host;
};

/**
 * @brief Opens and immediately aborts one TCP connection. No application byte is ever written or
 *        read and no protocol handshake is performed: reachability is the entire question.
 */
class TcpProbeTask final : public Async::Task {
  Q_OBJECT

public:
  TcpProbeTask(QString host, quint16 port, QObject* parent = nullptr);
  TcpProbeTask(TcpProbeTask&&)                 = delete;
  TcpProbeTask(const TcpProbeTask&)            = delete;
  TcpProbeTask& operator=(TcpProbeTask&&)      = delete;
  TcpProbeTask& operator=(const TcpProbeTask&) = delete;
  ~TcpProbeTask() override;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void onConnected();
  void releaseConnections();
  void onErrorOccurred(QAbstractSocket::SocketError error);

private:
  quint16 m_port;
  QString m_host;
  QTcpSocket m_socket;
  QMetaObject::Connection m_errorConnection;
  QMetaObject::Connection m_connectedConnection;
};

/**
 * @brief Reads the endpoint the user configured for @p bus, returning false when the bus carries
 *        no reachable endpoint (UDP, or an absent commercial driver).
 */
[[nodiscard]] bool endpoint(Bus bus, QString& host, quint16& port);

/**
 * @brief Returns the declared worst case of one reachability probe, in milliseconds.
 */
[[nodiscard]] int probeBudgetMsec();

/**
 * @brief Appends the instant configuration verdicts for @p bus: no host, or no port.
 */
void collectInstant(Bus bus, QList<Result>& out);

/**
 * @brief Builds the reachability flow for one endpoint: a bounded lookup followed by a bounded
 *        connect, each captured by value so the flow holds no driver pointer.
 */
[[nodiscard]] Async::Task* makeReachabilityFlow(const QString& host,
                                                quint16 port,
                                                Async::AsyncClock& clock);

/**
 * @brief Maps one flow outcome onto the verdict it means, switching on the failing step so the
 *        three reachability failures keep their three distinct remedies.
 */
[[nodiscard]] Result reachabilityResult(Bus bus,
                                        const QString& host,
                                        quint16 port,
                                        Async::Outcome outcome,
                                        const Async::StepError& error);

}  // namespace Misc::Diagnostics::NetworkChecks
