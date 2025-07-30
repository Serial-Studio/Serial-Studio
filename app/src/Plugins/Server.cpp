/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "IO/Manager.h"
#include "Plugins/Server.h"
#include "Misc/Utilities.h"
#include "Misc/TimerEvents.h"

/**
 * @brief Constructs the plugin server.
 *
 * Initializes signal-slot connections for processing raw and structured
 * data frames, sets up socket handling, and prepares the TCP server
 * for later activation via setEnabled().
 */
Plugins::Server::Server()
  : m_enabled(false)
{
  // Send processed data at 1 Hz
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &Plugins::Server::sendProcessedData);

  // Configure TCP server
  connect(&m_server, &QTcpServer::newConnection, this,
          &Plugins::Server::acceptConnection);
}

/**
 * @brief Destroys the plugin server.
 *
 * Shuts down the TCP server and closes all existing connections.
 */
Plugins::Server::~Server()
{
  m_server.close();
}

/**
 * @brief Gets the singleton instance of the plugin server.
 *
 * @return Reference to the only instance of Plugins::Server.
 */
Plugins::Server &Plugins::Server::instance()
{
  static Server singleton;
  return singleton;
}

/**
 * @brief Checks whether the plugin server is currently enabled.
 *
 * @return true if the plugin subsystem is active and serving connections;
 *         false otherwise.
 */
bool Plugins::Server::enabled() const
{
  return m_enabled;
}

/**
 * @brief Disconnects a client socket from the server.
 *
 * Removes the socket from the active connections list and deletes it.
 * Triggered automatically when a client disconnects or encounters an error.
 */
void Plugins::Server::removeConnection()
{
  // Get caller socket
  auto socket = static_cast<QTcpSocket *>(QObject::sender());

  // Remove socket from registered sockets
  if (socket)
  {
    m_sockets.removeAll(socket);
    socket->deleteLater();
  }
}

/**
 * @brief Enables or disables the TCP plugin server.
 *
 * When enabling, starts listening for incoming TCP connections.
 * When disabling, stops the server, closes all sockets, and clears
 * frame buffers.
 *
 * @param enabled If true, activates the server. If false, deactivates it.
 */
void Plugins::Server::setEnabled(const bool enabled)
{
  // Change value
  m_enabled = enabled;
  Q_EMIT enabledChanged();

  // Enable the TCP plugin server
  if (enabled)
  {
    if (!m_server.isListening())
    {
      if (!m_server.listen(QHostAddress::Any, PLUGINS_TCP_PORT))
      {
        Misc::Utilities::showMessageBox(tr("Unable to start plugin TCP server"),
                                        m_server.errorString(),
                                        QMessageBox::Warning);
        m_server.close();
      }
    }
  }

  // Disable the TCP plugin server
  else
  {
    m_server.close();
    for (int i = 0; i < m_sockets.count(); ++i)
    {
      auto socket = m_sockets.at(i);
      if (socket)
      {
        socket->abort();
        socket->deleteLater();
      }
    }

    m_sockets.clear();
    JSON::Frame frame;
    while (m_pendingFrames.try_dequeue(frame))
    {
    }
  }
}

/**
 * @brief Sends raw binary data to all connected clients.
 *
 * The data is base64-encoded and wrapped in a JSON object before
 * being transmitted over each writable TCP socket.
 *
 * @param data Raw data bytes received from the I/O layer.
 */
void Plugins::Server::hotpathTxData(const QByteArray &data)
{
  // Stop if system is not enabled
  if (!enabled())
    return;

  // Stop if no sockets are available
  if (m_sockets.count() < 1)
    return;

  // Create JSON structure with incoming data encoded in Base-64
  QJsonObject object;
  object.insert(QStringLiteral("data"), QString::fromUtf8(data.toBase64()));

  // Get JSON string in compact format & send it over the TCP socket
  QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  // Send data to each plugin
  for (auto *socket : std::as_const(m_sockets))
  {
    if (!socket)
      continue;

    if (socket->isWritable())
      socket->write(json);
  }
}

/**
 * @brief Registers a new structured data frame.
 *
 * Appends the frame to an internal buffer that will be transmitted
 * to clients by sendProcessedData().
 *
 * @param frame JSON::Frame object to register.
 */
void Plugins::Server::hotpathTxFrame(const JSON::Frame &frame)
{
  if (enabled())
    m_pendingFrames.enqueue(frame);
}

/**
 * @brief Handles incoming data from a client socket.
 *
 * Forwards raw data from the client directly to the I/O manager
 * for processing or device transmission.
 */
void Plugins::Server::onDataReceived()
{
  // Get caller socket
  auto socket = static_cast<QTcpSocket *>(QObject::sender());

  // Write incoming data to manager
  if (enabled() && socket)
    IO::Manager::instance().writeData(socket->readAll());
}

/**
 * @brief Accepts new incoming TCP connections.
 *
 * Validates new client sockets and attaches relevant signal handlers.
 * Rejects connections if the server is not enabled.
 */
void Plugins::Server::acceptConnection()
{
  // Get & validate socket
  auto socket = m_server.nextPendingConnection();
  if (!socket && enabled())
  {
    Misc::Utilities::showMessageBox(tr("Plugin server"),
                                    tr("Invalid pending connection"),
                                    QMessageBox::Critical);
    return;
  }

  // Close connection if system is not enabled
  if (!enabled())
  {
    if (socket)
    {
      socket->close();
      socket->deleteLater();
    }

    return;
  }

  // Connect socket signals/slots
  connect(socket, &QTcpSocket::readyRead, this,
          &Plugins::Server::onDataReceived);
  connect(socket, &QTcpSocket::disconnected, this,
          &Plugins::Server::removeConnection);

  // React to socket errors
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(onErrorOccurred(QAbstractSocket::SocketError)));
#else
  connect(socket, &QTcpSocket::errorOccurred, this,
          &Plugins::Server::onErrorOccurred);
#endif

  // Add socket to sockets list
  m_sockets.append(socket);
}

/**
 * @brief Sends all collected structured data frames to connected clients.
 *
 * Frames are serialized into a compact JSON array and sent to each
 * writable socket. Called periodically (1 Hz) via timer events.
 */
void Plugins::Server::sendProcessedData()
{
  // Stop if system is not enabled
  if (!enabled())
    return;

  // Stop if no sockets are available
  if (m_sockets.count() < 1)
    return;

  // Create JSON array with frame data
  QJsonArray array;
  JSON::Frame frame;
  while (m_pendingFrames.try_dequeue(frame))
  {
  }
  {
    QJsonObject object;
    object.insert(QStringLiteral("data"), serialize(frame));
    array.append(object);
  }

  // Create JSON document with frame arrays
  if (array.count() > 0)
  {
    // Construct QByteArray with data
    QJsonObject object;
    object.insert(QStringLiteral("frames"), array);
    const QJsonDocument document(object);
    auto json = document.toJson(QJsonDocument::Compact) + "\n";

    // Send data to each plugin
    for (auto *socket : std::as_const(m_sockets))
    {
      if (!socket)
        continue;

      if (socket->isWritable())
        socket->write(json);
    }
  }
}

/**
 * @brief Handles socket-level errors from connected clients.
 *
 * Logs the error to debug output and attempts to cleanly disconnect
 * the faulty socket.
 *
 * @param socketError Error code provided by the QAbstractSocket.
 */
void Plugins::Server::onErrorOccurred(
    const QAbstractSocket::SocketError socketError)
{
  auto socket = static_cast<QTcpSocket *>(QObject::sender());
  if (socket)
    qDebug() << socket->errorString();
  else
    qDebug() << socketError;
}
