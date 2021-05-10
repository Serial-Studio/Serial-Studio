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

#ifndef PLUGINS_BRIDGE_H
#define PLUGINS_BRIDGE_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QHostAddress>

#include <JSON/Frame.h>
#include <JSON/Dataset.h>
#include <JSON/FrameInfo.h>

#define PLUGINS_TCP_PORT 7777

namespace Plugins
{
class Bridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

signals:
    void enabledChanged();

public:
    static Bridge *getInstance();
    bool enabled() const;

public slots:
    void removeConnection();
    void setEnabled(const bool enabled);

private slots:
    void onDataReceived();
    void acceptConnection();
    void sendProcessedData();
    void sendRawData(const QByteArray &data);
    void registerFrame(const JFI_Object &frameInfo);
    void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
    Bridge();
    ~Bridge();

private:
    bool m_enabled;
    QTcpServer m_server;
    QList<JFI_Object> m_frames;
    QList<QTcpSocket *> m_sockets;
};
}

#endif
