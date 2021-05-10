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

#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <QTimer>
#include <QObject>
#include <QIODevice>

namespace IO
{
class Manager : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(bool readOnly
               READ readOnly
               NOTIFY connectedChanged)
    Q_PROPERTY(bool readWrite
               READ readWrite
               NOTIFY connectedChanged)
    Q_PROPERTY(bool connected
               READ connected
               NOTIFY connectedChanged)
    Q_PROPERTY(bool deviceAvailable
               READ deviceAvailable
               NOTIFY deviceChanged)
    Q_PROPERTY(IO::Manager::DataSource dataSource
               READ dataSource
               WRITE setDataSource
               NOTIFY dataSourceChanged)
    Q_PROPERTY(QString receivedDataLength
               READ receivedDataLength
               NOTIFY receivedBytesChanged)
    Q_PROPERTY(QString startSequence
               READ startSequence
               WRITE setStartSequence
               NOTIFY startSequenceChanged)
    Q_PROPERTY(QString finishSequence
               READ finishSequence
               WRITE setFinishSequence
               NOTIFY finishSequenceChanged)
    Q_PROPERTY(bool configurationOk
               READ configurationOk
               NOTIFY configurationChanged)
    // clang-format on

signals:
    void tx();
    void rx();
    void deviceChanged();
    void connectedChanged();
    void watchdogTriggered();
    void dataSourceChanged();
    void writeEnabledChanged();
    void configurationChanged();
    void receivedBytesChanged();
    void maxBufferSizeChanged();
    void startSequenceChanged();
    void finishSequenceChanged();
    void watchdogIntervalChanged();
    void frameValidationRegexChanged();
    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void frameReceived(const QByteArray &frame);

public:
    enum class DataSource
    {
        Serial,
        Network
    };
    Q_ENUM(DataSource)

    static Manager *getInstance();

    bool readOnly();
    bool readWrite();
    bool connected();
    bool deviceAvailable();
    bool configurationOk() const;

    int maxBufferSize() const;
    int watchdogInterval() const;

    QIODevice *device();
    DataSource dataSource() const;

    QString startSequence() const;
    QString finishSequence() const;
    QString receivedDataLength() const;

    Q_INVOKABLE QStringList dataSourcesList() const;
    Q_INVOKABLE qint64 writeData(const QByteArray &data);

public slots:
    void connectDevice();
    void toggleConnection();
    void disconnectDevice();
    void setWriteEnabled(const bool enabled);
    void setDataSource(const DataSource source);
    void processPayload(const QByteArray &payload);
    void setMaxBufferSize(const int maxBufferSize);
    void setStartSequence(const QString &sequence);
    void setFinishSequence(const QString &sequence);
    void setWatchdogInterval(const int interval = 15);

private slots:
    void readFrames();
    void feedWatchdog();
    void onDataReceived();
    void clearTempBuffer();
    void onWatchdogTriggered();
    void setDevice(QIODevice *device);

private:
    Manager();
    ~Manager();

private:
    QTimer m_watchdog;
    bool m_writeEnabled;
    int m_maxBuzzerSize;
    QIODevice *m_device;
    DataSource m_dataSource;
    QByteArray m_dataBuffer;
    quint64 m_receivedBytes;
    QString m_startSequence;
    QString m_finishSequence;
};
}

#endif
