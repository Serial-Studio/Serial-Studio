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

#include "Bridge.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <IO/Manager.h>
#include <Misc/Utilities.h>
#include <Misc/TimerEvents.h>
#include <JSON/Generator.h>

using namespace Plugins;

/*
 * Singleton pointer
 */
static Bridge *INSTANCE = nullptr;

/**
 * Constructor function
 */
Bridge::Bridge()
{
    // Set socket to NULL
    m_socket = nullptr;

    // Send processed data at 42 Hz
    auto ge = JSON::Generator::getInstance();
    auto te = Misc::TimerEvents::getInstance();
    connect(ge, &JSON::Generator::jsonChanged, this, &Bridge::registerFrame);
    connect(te, &Misc::TimerEvents::timeout42Hz, this, &Bridge::sendProcessedData);

    // Send I/O "raw" data directly
    auto io = IO::Manager::getInstance();
    connect(io, &IO::Manager::dataReceived, this, &Bridge::sendRawData);

    // Configure TCP server
    connect(&m_server, &QTcpServer::newConnection, this, &Bridge::acceptConnection);
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
Bridge::~Bridge()
{
    stop();
}

/**
 * Returns a pointer to the only instance of the class
 */
Bridge *Bridge::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Bridge;

    return INSTANCE;
}

/**
 * Disconnects the socket used for communicating with plugins.
 */
void Bridge::stop()
{
    m_socket->deleteLater();
    m_socket = nullptr;
}

/**
 * Process incoming data and writes it directly to the connected I/O device
 */
void Bridge::onDataReceived()
{
    // Stop if socket is NULL
    if (!m_socket)
        return;

    // Read incoming data
    auto data = m_socket->readAll();

    // Todo validate incoming JSON data & process it
}

void Bridge::acceptConnection()
{
    // Get & validate socket
    m_socket = m_server.nextPendingConnection();
    if (!m_socket)
    {
        Misc::Utilities::showMessageBox(tr("Plugin server"),
                                        tr("Invalid pending connection"));
        return;
    }

    // Connect socket signals/slots
    connect(m_socket, &QTcpSocket::readyRead, this, &Bridge::onDataReceived);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Bridge::onErrorOccurred);
    connect(m_socket, &QTcpSocket::disconnected, this, &Bridge::stop);

    // Close TCP server
    m_server.close();
}

/**
 * Sends an array of frames with the following information:
 * - Frame ID number
 * - RX timestamp
 * - Frame JSON data
 */
void Bridge::sendProcessedData()
{
    // Stop if frame list is empty
    if (m_frames.count() <= 0)
        return;

    // Stop if socket is NULL
    if (!m_socket)
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
        QJsonObject object;
        object.insert("frames", array);
        QJsonDocument document(object);
        if (m_socket->isWritable())
            m_socket->write(document.toJson(QJsonDocument::Compact) + "\n");
    }

    // Clear frame list
    m_frames.clear();
}

/**
 * Encodes the given @a data in Base64 and sends it through the TCP socket connected
 * to the localhost.
 */
void Bridge::sendRawData(const QByteArray &data)
{
    // Stop if socket is NULL
    if (!m_socket)
        return;

    // Create JSON structure with incoming data encoded in Base-64
    QJsonObject object;
    object.insert("data", QString::fromUtf8(data.toBase64()));

    // Get JSON string in compact format & send it over the TCP socket
    QJsonDocument document(object);
    if (m_socket->isWritable())
        m_socket->write(document.toJson(QJsonDocument::Compact) + "\n");
}

/**
 * Obtains the latest JSON dataframe & appends it to the JSON list, which is later read
 * and sent by the @c sendProcessedData() function.
 */
void Bridge::registerFrame(const JFI_Object &frameInfo)
{
    m_frames.append(frameInfo);
}

/**
 * This function is called whenever a socket error occurs, it disconnects the socket
 * from the host and displays the error in a message box.
 */
void Bridge::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
    (void)socketError;
    Misc::Utilities::showMessageBox(tr("TCP plugin socket error"),
                                    m_socket->errorString());
}
