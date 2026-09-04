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

#include "IO/Drivers/Network.h"

/**
 * @brief Returns true for an established TCP link. The state alone is trustworthy because the
 *        socket connects exactly once per open() and is never aborted and redialed (the 2026-07
 *        phantom-connect trigger was an abort and a dial racing in the same event-loop turn with
 *        the outcome read from signals).
 */
bool IO::Drivers::Network::tcpLinkUp() const
{
  return m_tcpSocket->state() == QAbstractSocket::ConnectedState;
}

/**
 * @brief Starts the asynchronous TCP dial. The helper absorbs resolution and the refusal retries
 *        on throwaway sockets and then connects this driver's socket exactly once; the verdict
 *        arrives at onTcpDialFinished(), so open() returns "attempt started" with isConnecting()
 *        true rather than the final outcome.
 */
bool IO::Drivers::Network::dialTcpAsync(const QString& host, const QIODevice::OpenMode mode)
{
  if (tcpPort() == 0) {
    logDriverError(tr("Network socket error"), tr("Enter a TCP port first"));
    return false;
  }

  m_dialPending = true;
  m_tcpDial.start(host, tcpPort(), m_tcpSocket, mode);

  return m_dialPending || tcpOpen();
}

/**
 * @brief Settles the TCP dial exactly once. The readyRead and errorOccurred handlers are wired
 *        only on success, so a dial failure can never be reported twice: failDial() reports and
 *        ConnectionManager owns the teardown.
 */
void IO::Drivers::Network::onTcpDialFinished(bool ok, const QString& reason)
{
  if (!ok) {
    const QString host = remoteAddress().isEmpty() ? defaultAddress() : remoteAddress();
    m_tcpPendingWrites.clear();
    failDial(tr("Cannot connect to %1:%2 (%3)").arg(host, QString::number(tcpPort()), reason));
    return;
  }

  connect(m_tcpSocket,
          &QTcpSocket::readyRead,
          this,
          &IO::Drivers::Network::onTcpReadyRead,
          Qt::UniqueConnection);
  connect(m_tcpSocket,
          &QTcpSocket::errorOccurred,
          this,
          &IO::Drivers::Network::onTcpError,
          Qt::UniqueConnection);

  if (!m_tcpPendingWrites.isEmpty()) {
    (void)m_tcpSocket->write(m_tcpPendingWrites);
    m_tcpPendingWrites.clear();
  }

  succeedDial();
}

/**
 * @brief Forwards every TCP state transition so ConnectionManager can refresh connected state.
 */
void IO::Drivers::Network::onTcpStateChanged()
{
  Q_EMIT configurationChanged();
}

/**
 * @brief Dials the configured TCP endpoint, falling back to the default address when the user
 *        left the field empty. The dial runs asynchronously and reports through openFinished().
 */
bool IO::Drivers::Network::openTcp(const QIODevice::OpenMode mode)
{
  auto hostAddr = remoteAddress();
  if (hostAddr.isEmpty())
    hostAddr = defaultAddress();

  return dialTcpAsync(hostAddr, mode);
}

/**
 * @brief Retires the TCP link and its handlers, cancelling a dial still in flight without a
 *        verdict: a cancel is not an open failure. Safe on an idle socket, which is what lets
 *        close() tear down every transport unconditionally.
 */
void IO::Drivers::Network::closeTcp()
{
  m_tcpDial.cancel();
  m_tcpPendingWrites.clear();

  disconnect(m_tcpSocket, &QTcpSocket::readyRead, this, &IO::Drivers::Network::onTcpReadyRead);
  disconnect(m_tcpSocket, &QTcpSocket::errorOccurred, this, &IO::Drivers::Network::onTcpError);

  m_tcpSocket->abort();
  m_tcpSocket->close();
  m_tcpSocket->disconnectFromHost();
}

/**
 * @brief Writes @p data to the TCP stream.
 */
qint64 IO::Drivers::Network::writeTcp(const QByteArray& data)
{
  return m_tcpSocket->write(data);
}

/**
 * @brief Holds @p data until the dial settles, standing in for the buffering QTcpSocket used to
 *        do when connectToHost() ran before the write. The queue is capped: a script writing into
 *        an endpoint that never answers must not grow memory without bound.
 */
qint64 IO::Drivers::Network::queueTcpWrite(const QByteArray& data)
{
  constexpr qsizetype kMaxPendingWriteBytes = 1024 * 1024;
  if (m_tcpPendingWrites.size() + data.size() > kMaxPendingWriteBytes)
    return 0;

  m_tcpPendingWrites.append(data);
  return data.size();
}

/**
 * @brief Returns true for an open socket carrying an established link.
 */
bool IO::Drivers::Network::tcpOpen() const
{
  return m_tcpSocket->isOpen() && tcpLinkUp();
}

/**
 * @brief Returns true when the TCP socket can be read.
 */
bool IO::Drivers::Network::tcpReadable() const
{
  return m_tcpSocket->isReadable();
}

/**
 * @brief Returns true when the TCP socket can be written.
 */
bool IO::Drivers::Network::tcpWritable() const
{
  return m_tcpSocket->isWritable();
}

/**
 * @brief Returns true when the TCP port is usable. Host validity is deliberately not checked
 *        here: open() falls back to the default address and connectToHost() resolves names.
 */
bool IO::Drivers::Network::tcpConfigured() const
{
  return tcpPort() > 0;
}

/**
 * @brief Publishes everything buffered on the TCP stream.
 */
void IO::Drivers::Network::onTcpReadyRead()
{
  publishReceivedData(m_tcpSocket->readAll());
}

/**
 * @brief Reports an error on an established TCP link.
 */
void IO::Drivers::Network::onTcpError()
{
  reportLinkError(m_tcpSocket->errorString());
}

/**
 * @brief Appends the TCP rows of the driver property model. The shared address row is appended by
 *        the facade, which emits every transport's rows in one list.
 */
void IO::Drivers::Network::appendTcpProperties(QList<IO::DriverProperty>& props) const
{
  IO::DriverProperty tcp;
  tcp.key   = QStringLiteral("tcpPort");
  tcp.label = tr("TCP Port");
  tcp.type  = IO::DriverProperty::IntField;
  tcp.value = m_tcpPort;
  tcp.min   = 1;
  tcp.max   = 65535;
  props.append(tcp);
}

/**
 * @brief Applies a TCP property by key, reporting whether it was consumed.
 */
bool IO::Drivers::Network::applyTcpProperty(const QString& key, const QVariant& value)
{
  if (key != QLatin1String("tcpPort"))
    return false;

  setTcpPort(static_cast<quint16>(value.toInt()));
  return true;
}
