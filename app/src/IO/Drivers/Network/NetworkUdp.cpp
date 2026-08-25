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
 * @brief Requests a large kernel receive buffer on the bound UDP socket so a bursty high-rate
 *        sender is not dropped before the event loop drains it. Windows' default UDP SO_RCVBUF
 *        is only tens of KB and overflows under loopback floods (Linux/macOS default much
 *        higher); the OS caps the request to its allowed maximum.
 */
void IO::Drivers::Network::enlargeUdpReceiveBuffer()
{
  constexpr int kUdpReceiveBufferBytes = 8 * 1024 * 1024;
  m_udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,
                               kUdpReceiveBufferBytes);
}

/**
 * @brief Binds the UDP socket and joins the multicast group when one is configured. The order is
 *        load-bearing: bind, enlarge the kernel buffer, join, then open the device.
 */
bool IO::Drivers::Network::openUdp(const QIODevice::OpenMode mode)
{
  if (!m_address.isEmpty() && m_resolvedAddress.isNull() && !m_lookupActive)
    lookup(m_address);

  if (!m_udpSocket->bind(udpLocalPort(),
                         QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
    qWarning() << "UDP bind failed on port" << udpLocalPort() << ":" << m_udpSocket->errorString();
    close();
    return false;
  }

  enlargeUdpReceiveBuffer();

  if (udpMulticast()) {
    const QHostAddress literal(m_address);
    const QHostAddress group = literal.isNull() ? m_resolvedAddress : literal;
    if (group.isNull() || !m_udpSocket->joinMulticastGroup(group)) {
      qWarning() << "UDP multicast join failed for" << m_address << ":"
                 << m_udpSocket->errorString();
      close();
      return false;
    }
  }

  if (m_udpSocket->open(mode)) {
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &IO::Drivers::Network::onUdpReadyRead);
    return true;
  }

  close();
  return false;
}

/**
 * @brief Retires the UDP socket and its read handler. Safe on an unbound socket.
 */
void IO::Drivers::Network::closeUdp()
{
  disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &IO::Drivers::Network::onUdpReadyRead);

  m_udpSocket->abort();
  m_udpSocket->close();
  m_udpSocket->disconnectFromHost();
}

/**
 * @brief Sends @p data as a datagram to the configured peer, preferring the literal address the
 *        user typed over a resolved one.
 */
qint64 IO::Drivers::Network::writeUdp(const QByteArray& data)
{
  const QHostAddress dest =
    m_resolvedAddress.isNull() ? QHostAddress(m_address) : m_resolvedAddress;
  if (dest.isNull())
    return -1;

  return m_udpSocket->writeDatagram(data, dest, udpRemotePort());
}

/**
 * @brief Returns true for an open socket that is bound or connected.
 */
bool IO::Drivers::Network::udpOpen() const
{
  const auto state = m_udpSocket->state();
  return m_udpSocket->isOpen()
      && (state == QUdpSocket::ConnectedState || state == QUdpSocket::BoundState);
}

/**
 * @brief Returns true when the UDP socket can be read.
 */
bool IO::Drivers::Network::udpReadable() const
{
  return m_udpSocket->isReadable();
}

/**
 * @brief Returns true when the UDP socket can be written.
 */
bool IO::Drivers::Network::udpWritable() const
{
  return m_udpSocket->isWritable();
}

/**
 * @brief Returns true when the UDP remote port is usable.
 */
bool IO::Drivers::Network::udpConfigured() const
{
  return udpRemotePort() > 0;
}

/**
 * @brief Drains pending datagrams, publishing each separately so the message boundaries UDP
 *        preserves survive into the frame reader. The per-pass cap keeps a flood from starving
 *        the event loop.
 */
void IO::Drivers::Network::onUdpReadyRead()
{
  constexpr int kMaxDatagramsPerRead = 256;
  for (int n = 0; n < kMaxDatagramsPerRead && m_udpSocket->hasPendingDatagrams(); ++n) {
    const qint64 size = m_udpSocket->pendingDatagramSize();
    m_udpBuffer.resize(size);
    m_udpSocket->readDatagram(m_udpBuffer.data(), m_udpBuffer.size());
    publishReceivedData(m_udpBuffer);
  }
}

/**
 * @brief Reports a UDP socket error. A refusal is ignored: a connectionless socket sees one
 *        whenever an ICMP port-unreachable comes back for a datagram nobody is listening to.
 */
void IO::Drivers::Network::onUdpError(const QAbstractSocket::SocketError socketError)
{
  if (socketError == QAbstractSocket::ConnectionRefusedError) [[unlikely]]
    return;

  reportLinkError(m_udpSocket->errorString());
}

/**
 * @brief Appends the UDP rows of the driver property model.
 */
void IO::Drivers::Network::appendUdpProperties(QList<IO::DriverProperty>& props) const
{
  appendAddressProperty(props);

  IO::DriverProperty udpLocal;
  udpLocal.key   = QStringLiteral("udpLocalPort");
  udpLocal.label = tr("UDP Local Port");
  udpLocal.type  = IO::DriverProperty::IntField;
  udpLocal.value = m_udpLocalPort;
  udpLocal.min   = 0;
  udpLocal.max   = 65535;
  props.append(udpLocal);

  IO::DriverProperty udpRemote;
  udpRemote.key   = QStringLiteral("udpRemotePort");
  udpRemote.label = tr("UDP Remote Port");
  udpRemote.type  = IO::DriverProperty::IntField;
  udpRemote.value = m_udpRemotePort;
  udpRemote.min   = 1;
  udpRemote.max   = 65535;
  props.append(udpRemote);

  IO::DriverProperty multicast;
  multicast.key   = QStringLiteral("udpMulticast");
  multicast.label = tr("UDP Multicast");
  multicast.type  = IO::DriverProperty::CheckBox;
  multicast.value = m_udpMulticast;
  props.append(multicast);
}

/**
 * @brief Applies a UDP property by key, reporting whether it was consumed.
 */
bool IO::Drivers::Network::applyUdpProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("udpLocalPort")) {
    setUdpLocalPort(static_cast<quint16>(value.toInt()));
    return true;
  }

  if (key == QLatin1String("udpRemotePort")) {
    setUdpRemotePort(static_cast<quint16>(value.toInt()));
    return true;
  }

  if (key == QLatin1String("udpMulticast")) {
    setUdpMulticast(value.toBool());
    return true;
  }

  return false;
}
