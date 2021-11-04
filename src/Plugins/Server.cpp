/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#include "Server.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <IO/Manager.h>
#include <JSON/Generator.h>
#include <Misc/Utilities.h>
#include <Misc/TimerEvents.h>

using namespace Plugins;

/*
 * Singleton pointer
 */
static Server *INSTANCE = nullptr;

/**
 * Constructor function
 */
Server::Server()
{
    // Set internal variables
    m_enabled = false;

    // Send processed data at 1 Hz
    auto ge = JSON::Generator::getInstance();
    auto te = Misc::TimerEvents::getInstance();
    connect(ge, &JSON::Generator::jsonChanged, this, &Server::registerFrame);
    connect(te, &Misc::TimerEvents::highFreqTimeout, this, &Server::sendProcessedData);

    // Send I/O "raw" data directly
    auto io = IO::Manager::getInstance();
    connect(io, &IO::Manager::dataReceived, this, &Server::sendRawData);

    // Configure TCP server
    connect(&m_server, &QTcpServer::newConnection, this, &Server::acceptConnection);
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
Server::~Server()
{
    m_server.close();
}

/**
 * Returns a pointer to the only instance of the class
 */
Server *Server::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Server;

    return INSTANCE;
}

/**
 * Returns @c true if the plugin sub-system is enabled
 */
bool Server::enabled() const
{
    return m_enabled;
}

/**
 * Disconnects the socket used for communicating with plugins.
 */
void Server::removeConnection()
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
void Server::setEnabled(const bool enabled)
{
    // Change value
    m_enabled = enabled;
    emit enabledChanged();

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
}

/**
 * Process incoming data and writes it directly to the connected I/O device
 */
void Server::onDataReceived()
{
    // Get caller socket
    auto socket = static_cast<QTcpSocket *>(QObject::sender());

    // Write incoming data to manager
    if (enabled() && socket)
        IO::Manager::getInstance()->writeData(socket->readAll());
}

/**
 * Configures incoming connection requests
 */
void Server::acceptConnection()
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
    connect(socket, &QTcpSocket::readyRead, this, &Server::onDataReceived);
    connect(socket, &QTcpSocket::errorOccurred, this, &Server::onErrorOccurred);
    connect(socket, &QTcpSocket::disconnected, this, &Server::removeConnection);

    // Add socket to sockets list
    m_sockets.append(socket);
}

/**
 * Sends an array of frames with the following information:
 * - Frame ID number
 * - RX timestamp
 * - Frame JSON data
 */
void Server::sendProcessedData()
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
        object.insert("id", QString::number(frame.frameNumber));
        object.insert("timestamp", frame.rxDateTime.toString());
        object.insert("data", frame.jsonDocument.object());
        array.append(object);
    }

    // Create JSON document with frame arrays
    if (array.count() > 0)
    {
        // Construct QByteArray with data
        QJsonObject object;
        object.insert("frames", array);
        QJsonDocument document(object);
        auto json = document.toJson(QJsonDocument::Compact) + "\n";

        // Send data to each plugin
        foreach (auto socket, m_sockets)
        {
            if (!socket)
                continue;

            if (socket->isWritable())
                socket->write(json);
        }
    }

    // Clear frame list
    m_frames.clear();
}

/**
 * Encodes the given @a data in Base64 and sends it through the TCP socket connected
 * to the localhost.
 */
void Server::sendRawData(const QByteArray &data)
{
    // Stop if system is not enabled
    if (!enabled())
        return;

    // Stop if no sockets are available
    if (m_sockets.count() < 1)
        return;

    // Create JSON structure with incoming data encoded in Base-64
    QJsonObject object;
    object.insert("data", QString::fromUtf8(data.toBase64()));

    // Get JSON string in compact format & send it over the TCP socket
    QJsonDocument document(object);
    auto json = document.toJson(QJsonDocument::Compact) + "\n";

    // Send data to each plugin
    foreach (auto socket, m_sockets)
    {
        if (!socket)
            continue;

        if (socket->isWritable())
            socket->write(json);
    }
}

/**
 * Obtains the latest JSON dataframe & appends it to the JSON list, which is later read
 * and sent by the @c sendProcessedData() function.
 */
void Server::registerFrame(const JFI_Object &frameInfo)
{
    m_frames.append(frameInfo);
}

/**
 * This function is called whenever a socket error occurs, it disconnects the socket
 * from the host and displays the error in a message box.
 */
void Server::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
    // Get caller socket
    auto socket = static_cast<QTcpSocket *>(QObject::sender());

    // Print error
    if (socket)
        qDebug() << socket->errorString();
    else
        qDebug() << socketError;
}

#if SERIAL_STUDIO_MOC_INCLUDE
#    include "moc_Server.cpp"
#endif
