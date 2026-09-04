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

#include <QStringConverter>

#include "IO/Drivers/Network.h"

/**
 * @brief Whether @p data decodes as UTF-8, which is what the Auto send format uses to pick a text
 *        message over a binary one. The choice is visible to the peer: a browser client receives
 *        a string from one and a Blob from the other.
 */
[[nodiscard]] static bool isUtf8Text(const QByteArray& data)
{
  QStringDecoder decoder(QStringDecoder::Utf8, QStringDecoder::Flag::Stateless);
  const QString decoded = decoder(data);
  Q_UNUSED(decoded);
  return !decoder.hasError();
}

/**
 * @brief Starts the WebSocket handshake and reports the dial as in flight. connected() settles it
 *        as success; errorOccurred() and a close arriving mid-handshake settle it as failure. The
 *        sslErrors hop must stay direct, or ignoreSslErrors() lands too late to matter, and a dial
 *        that already settled returns false so a pending verdict is never faked.
 */
bool IO::Drivers::Network::openWebSocket(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode);

  QUrl url;
  QString reason;
  if (!urlForCurrentMode(url, reason)) {
    logDriverError(tr("WebSocket error"), reason);
    return false;
  }

  connect(m_webSocket,
          &QWebSocket::connected,
          this,
          &IO::Drivers::Network::onWebSocketConnected,
          Qt::UniqueConnection);
  connect(m_webSocket,
          &QWebSocket::disconnected,
          this,
          &IO::Drivers::Network::onWebSocketDisconnected,
          Qt::UniqueConnection);
  connect(m_webSocket,
          &QWebSocket::errorOccurred,
          this,
          &IO::Drivers::Network::onWebSocketError,
          Qt::UniqueConnection);
  connect(m_webSocket,
          &QWebSocket::sslErrors,
          this,
          &IO::Drivers::Network::onWebSocketSslErrors,
          static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection));
  connect(m_webSocket,
          &QWebSocket::textMessageReceived,
          this,
          &IO::Drivers::Network::onWebSocketTextMessage,
          Qt::UniqueConnection);
  connect(m_webSocket,
          &QWebSocket::binaryMessageReceived,
          this,
          &IO::Drivers::Network::onWebSocketBinaryMessage,
          Qt::UniqueConnection);

  m_dialPending = true;
  m_webSocket->open(url);

  return m_dialPending || webSocketOpen();
}

/**
 * @brief Retires the WebSocket and its handlers. The handlers go first so the abort() below
 *        cannot re-enter as a disconnected() or errorOccurred() verdict for an attempt the user
 *        already cancelled. Safe on an idle socket, which is what lets close() tear down every
 *        transport unconditionally.
 */
void IO::Drivers::Network::closeWebSocket()
{
  // clang-format off
  disconnect(m_webSocket, &QWebSocket::connected,
             this, &IO::Drivers::Network::onWebSocketConnected);
  disconnect(m_webSocket, &QWebSocket::disconnected,
             this, &IO::Drivers::Network::onWebSocketDisconnected);
  disconnect(m_webSocket, &QWebSocket::errorOccurred,
             this, &IO::Drivers::Network::onWebSocketError);
  disconnect(m_webSocket, &QWebSocket::sslErrors,
             this, &IO::Drivers::Network::onWebSocketSslErrors);
  disconnect(m_webSocket, &QWebSocket::textMessageReceived,
             this, &IO::Drivers::Network::onWebSocketTextMessage);
  disconnect(m_webSocket, &QWebSocket::binaryMessageReceived,
             this, &IO::Drivers::Network::onWebSocketBinaryMessage);
  // clang-format on

  m_webSocket->abort();
}

/**
 * @brief Sends @p data to the peer as a single WebSocket message.
 */
qint64 IO::Drivers::Network::writeWebSocket(const QByteArray& data)
{
  const bool asText = m_webSocketFormat == 1 || (m_webSocketFormat == 0 && isUtf8Text(data));

  const qint64 sent = asText ? m_webSocket->sendTextMessage(QString::fromUtf8(data))
                             : m_webSocket->sendBinaryMessage(data);

  (void)m_webSocket->flush();
  return sent;
}

/**
 * @brief Returns true when the handshake completed and the socket can carry messages.
 */
bool IO::Drivers::Network::webSocketOpen() const
{
  return m_webSocket->isValid();
}

/**
 * @brief Returns true when the peer can be written to.
 */
bool IO::Drivers::Network::webSocketWritable() const
{
  return m_webSocket->isValid();
}

/**
 * @brief Returns true when the configured URL is one this transport can dial. Validation runs
 *        through the shared resolver so the Connect button and open() can never disagree.
 */
bool IO::Drivers::Network::webSocketConfigured() const
{
  QUrl url;
  QString reason;
  return urlForCurrentMode(url, reason);
}

/**
 * @brief Settles a successful handshake.
 */
void IO::Drivers::Network::onWebSocketConnected()
{
  succeedDial();
}

/**
 * @brief Handles the peer going away. During the handshake this is the dial verdict; on an
 *        established link it is a drop, which reports once and stays down.
 */
void IO::Drivers::Network::onWebSocketDisconnected()
{
  if (m_dialPending) {
    const QString reason = m_webSocket->closeReason();
    failDial(reason.isEmpty() ? tr("The connection closed during the handshake") : reason);
    return;
  }

  reportLinkError(tr("The remote peer closed the connection"));
}

/**
 * @brief Routes a socket error to the dial verdict or to the established-link teardown. A refused
 *        upgrade, a TLS failure and an unreachable host all arrive here.
 */
void IO::Drivers::Network::onWebSocketError(const QAbstractSocket::SocketError socketError)
{
  Q_UNUSED(socketError);

  const QString reason = m_webSocket->errorString();
  if (m_dialPending) {
    failDial(reason);
    return;
  }

  reportLinkError(reason);
}

/**
 * @brief Continues a handshake that failed certificate verification, but only when the user asked
 *        for it, and never silently: the bypass is stated on the console every time. Doing
 *        nothing here drops the connection, which is the default and the safe answer.
 */
void IO::Drivers::Network::onWebSocketSslErrors(const QList<QSslError>& errors)
{
  if (!m_ignoreTlsErrors)
    return;

  QStringList descriptions;
  descriptions.reserve(errors.count());
  for (const auto& error : errors)
    descriptions.append(error.errorString());

  logDriverError(
    tr("TLS verification bypassed"),
    tr("Continuing to %1 despite: %2")
      .arg(m_webSocket->requestUrl().toString(), descriptions.join(QStringLiteral("; "))));

  m_webSocket->ignoreSslErrors();
}

/**
 * @brief Publishes a text message as one chunk. The timestamp is taken here, at the capture
 *        boundary, because the source owns time and nothing downstream may re-stamp it.
 */
void IO::Drivers::Network::onWebSocketTextMessage(const QString& message)
{
  const auto timestamp = IO::CapturedData::SteadyClock::now();
  publishReceivedData(message.toUtf8(), timestamp);
}

/**
 * @brief Publishes a binary message as one chunk, stamped at the capture boundary.
 */
void IO::Drivers::Network::onWebSocketBinaryMessage(const QByteArray& message)
{
  const auto timestamp = IO::CapturedData::SteadyClock::now();
  publishReceivedData(message, timestamp);
}

/**
 * @brief Appends the WebSocket rows of the driver property model. The shared TLS row is appended
 *        by the facade, which emits every transport's rows in one list.
 */
void IO::Drivers::Network::appendWebSocketProperties(QList<IO::DriverProperty>& props) const
{
  IO::DriverProperty url;
  url.key         = QStringLiteral("webSocketUrl");
  url.label       = tr("URL");
  url.description = tr("WebSocket endpoint, for example %1").arg(defaultWebSocketUrl());
  url.type        = IO::DriverProperty::Text;
  url.value       = m_webSocketUrl;
  props.append(url);

  IO::DriverProperty format;
  format.key     = QStringLiteral("webSocketFormatIndex");
  format.label   = tr("Send Format");
  format.type    = IO::DriverProperty::ComboBox;
  format.value   = m_webSocketFormat;
  format.options = webSocketFormats();
  props.append(format);
}

/**
 * @brief Applies a WebSocket property by key, reporting whether it was consumed.
 */
bool IO::Drivers::Network::applyWebSocketProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("webSocketUrl")) {
    setWebSocketUrl(value.toString());
    return true;
  }

  if (key == QLatin1String("webSocketFormatIndex")) {
    setWebSocketFormatIndex(value.toInt());
    return true;
  }

  return false;
}
