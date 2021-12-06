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

#pragma once

#include <QTimer>
#include <QObject>
#include <QIODevice>
#include <DataTypes.h>

namespace IO
{
/**
 * @brief The Manager class
 *
 * The I/O Manager class provides an abstraction layer between the physical
 * device (e.g. a serial or network port) and the rest of the application.
 *
 * Additionaly, all frame parsing, checksum verification and data processing
 * is done directly by this class. In this way, programmers can focus on
 * implementing the code to read and write data to a device, while the manager
 * class deals with the processing of the received data and generates frames.
 *
 * A "frame" is equivalent to a packet of data. To identify frames, the manager
 * class needs to know three things:
 * - The start sequence or header of the frame
 * - The end sequence or footer of the frame
 * - The data separator sequence, which allows Serial Studio to differentiate
 *   between a dataset value and another dataset value.
 *
 * Example of a frame:
 *
 *   $A,B,C,D,E,F,G;
 *
 * In this case, the start sequence of the frame is "$", while the end sequence
 * is ";". The data separator sequence is ",". Knowing this, Serial Studio
 * can process the frame and deduce that A,B,C,D,E,F and G are individual
 * dataset values.
 */
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
    Q_PROPERTY(QString startSequence
               READ startSequence
               WRITE setStartSequence
               NOTIFY startSequenceChanged)
    Q_PROPERTY(QString finishSequence
               READ finishSequence
               WRITE setFinishSequence
               NOTIFY finishSequenceChanged)
    Q_PROPERTY(QString separatorSequence
               READ separatorSequence
               WRITE setSeparatorSequence
               NOTIFY separatorSequenceChanged)
    Q_PROPERTY(bool configurationOk
               READ configurationOk
               NOTIFY configurationChanged)
    // clang-format on

signals:
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
    void separatorSequenceChanged();
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

    enum class ValidationStatus
    {
        FrameOk,
        ChecksumError,
        ChecksumIncomplete
    };
    Q_ENUM(ValidationStatus)

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
    QString separatorSequence() const;

    Q_INVOKABLE StringList dataSourcesList() const;
    Q_INVOKABLE qint64 writeData(const QByteArray &data);

public slots:
    void connectDevice();
    void toggleConnection();
    void disconnectDevice();
    void setWriteEnabled(const bool enabled);
    void processPayload(const QByteArray &payload);
    void setMaxBufferSize(const int maxBufferSize);
    void setStartSequence(const QString &sequence);
    void setFinishSequence(const QString &sequence);
    void setSeparatorSequence(const QString &sequence);
    void setWatchdogInterval(const int interval = 15);
    void setDataSource(const IO::Manager::DataSource &source);

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

    ValidationStatus integrityChecks(const QByteArray &frame,
                                     const QByteArray &masterBuffer, int *bytesToChop);

private:
    QTimer m_watchdog;
    bool m_enableCrc;
    bool m_writeEnabled;
    int m_maxBufferSize;
    QIODevice *m_device;
    DataSource m_dataSource;
    QByteArray m_dataBuffer;
    quint64 m_receivedBytes;
    QString m_startSequence;
    QString m_finishSequence;
    QString m_separatorSequence;
};
}
