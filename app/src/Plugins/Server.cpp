/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "IO/Manager.h"
#include "JSON/FrameBuilder.h"
#include "Misc/Utilities.h"
#include "Plugins/Server.h"
#include "Misc/TimerEvents.h"

/**
 * Constructor function
 */
Plugins::Server::Server()
  : m_enabled(false)
{

  // Send processed data at 1 Hz
  connect(&JSON::FrameBuilder::instance(), &JSON::FrameBuilder::frameChanged,
          this, &Plugins::Server::registerFrame, Qt::QueuedConnection);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &Plugins::Server::sendProcessedData);

  // Send I/O "raw" data directly
  connect(&IO::Manager::instance(), &IO::Manager::dataReceived, this,
          &Plugins::Server::sendRawData, Qt::QueuedConnection);

  // Configure TCP server
  connect(&m_server, &QTcpServer::newConnection, this,
          &Plugins::Server::acceptConnection);

  // Begin listening on TCP port
  if (!m_server.listen(QHostAddress::Any, PLUGINS_TCP_PORT))
  {
    Misc::Utilities::showMessageBox(tr("Unable to start plugin TCP server"),
                                    m_server.errorString());
    m_server.close();
  }
}

/**
 * Destructor function
 */
Plugins::Server::~Server()
{
  m_server.close();
}

/**
 * Returns a pointer to the only instance of the class
 */
Plugins::Server &Plugins::Server::instance()
{
  static Server singleton;
  return singleton;
}

/**
 * Returns @c true if the plugin sub-system is enabled
 */
bool Plugins::Server::enabled() const
{
  return m_enabled;
}

/**
 * Disconnects the socket used for communicating with plugins.
 */
void Plugins::Server::removeConnection()
{
  // Get caller socket
  auto socket = static_cast<QTcpSocket *>(QObject::sender());

  // Remove socket from registered sockets
  if (socket)
  {
    for (int i = 0; i < m_sockets.count(); ++i)
    {
      if (m_sockets.at(i) == socket)
      {
        m_sockets.removeAt(i);
        i = 0;
      }
    }

    // Delete socket handler
    socket->deleteLater();
  }
}

/**
 * Enables/disables the plugin subsystem
 */
void Plugins::Server::setEnabled(const bool enabled)
{
  // Change value
  m_enabled = enabled;
  Q_EMIT enabledChanged();

  // If not enabled, remove all connections
  if (!enabled)
  {
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
  }

  // Clear frames array to avoid memory leaks
  m_frames.clear();
  m_frames.squeeze();
}

/**
 * Process incoming data and writes it directly to the connected I/O device
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
 * Configures incoming connection requests
 */
void Plugins::Server::acceptConnection()
{
  // Get & validate socket
  auto socket = m_server.nextPendingConnection();
  if (!socket && enabled())
  {
    Misc::Utilities::showMessageBox(tr("Plugin server"),
                                    tr("Invalid pending connection"));
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
 * Sends an array of frames with the following information:
 * - Frame ID number
 * - RX timestamp
 * - Frame JSON data
 */
void Plugins::Server::sendProcessedData()
{
  // Stop if system is not enabled
  if (!enabled())
    return;

  // Stop if frame list is empty
  if (m_frames.count() <= 0)
    return;

  // Stop if no sockets are available
  if (m_sockets.count() < 1)
    return;

  // Create JSON array with frame data
  QJsonArray array;
  for (int i = 0; i < m_frames.count(); ++i)
  {
    QJsonObject object;
    auto frame = m_frames.at(i);
    object.insert(QStringLiteral("data"), frame.serialize());
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
    Q_FOREACH (auto socket, m_sockets)
    {
      if (!socket)
        continue;

      if (socket->isWritable())
        socket->write(json);
    }
  }

  // Clear frame list
  m_frames.clear();
  m_frames.squeeze();
}

/**
 * Encodes the given @a data in Base64 and sends it through the TCP socket
 * connected to the localhost.
 */
void Plugins::Server::sendRawData(const QByteArray &data)
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
  Q_FOREACH (auto socket, m_sockets)
  {
    if (!socket)
      continue;

    if (socket->isWritable())
      socket->write(json);
  }
}

/**
 * Obtains the latest JSON dataframe & appends it to the JSON list, which is
 * later read and sent by the @c sendProcessedData() function.
 */
void Plugins::Server::registerFrame(const JSON::Frame &frame)
{
  if (enabled())
    m_frames.append(frame);
}

/**
 * This function is called whenever a socket error occurs, it disconnects the
 * socket from the host and displays the error in a message box.
 */
void Plugins::Server::onErrorOccurred(
    const QAbstractSocket::SocketError socketError)
{
  // Get caller socket
  auto socket = static_cast<QTcpSocket *>(QObject::sender());

  // Print error
  if (socket)
    qDebug() << socket->errorString();
  else
    qDebug() << socketError;
}
