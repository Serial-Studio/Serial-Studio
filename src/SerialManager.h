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

#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QtSerialPort>
#include <QTextCursor>
#include <QQuickTextDocument>

class SerialManager : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QString portName
               READ portName
               NOTIFY portChanged)
    Q_PROPERTY(bool readOnly
               READ readOnly
               NOTIFY connectedChanged)
    Q_PROPERTY(bool readWrite
               READ readWrite
               NOTIFY connectedChanged)
    Q_PROPERTY(bool connected
               READ connected
               NOTIFY connectedChanged)
    Q_PROPERTY(QString receivedBytes
               READ receivedBytes
               NOTIFY receivedBytesChanged)
    Q_PROPERTY(int maxBufferSize
               READ maxBufferSize
               WRITE setMaxBufferSize
               NOTIFY maxBufferSizeChanged)
    Q_PROPERTY(bool sendHexData
               READ sendHexData
               WRITE setSendHexData
               NOTIFY sendHexChanged)
    Q_PROPERTY(QString inputMask
               READ inputMask
               NOTIFY sendHexChanged)
    Q_PROPERTY(bool writeEnabled
               READ writeEnabled
               WRITE setWriteEnabled
               NOTIFY writeEnabledChanged)
    Q_PROPERTY(QString startSequence
               READ startSequence
               WRITE setStartSequence
               NOTIFY startSequenceChanged)
    Q_PROPERTY(QString finishSequence
               READ finishSequence
               WRITE setFinishSequence
               NOTIFY finishSequenceChanged)
    Q_PROPERTY(quint8 portIndex
               READ portIndex
               WRITE setPort
               NOTIFY portChanged)
    Q_PROPERTY(quint8 parityIndex
               READ parityIndex
               WRITE setParity
               NOTIFY parityChanged)
    Q_PROPERTY(quint8 displayMode
               READ displayMode
               WRITE setDisplayMode
               NOTIFY displayModeChanged)
    Q_PROPERTY(quint8 baudRateIndex
               READ baudRateIndex
               WRITE setBaudRateIndex
               NOTIFY baudRateIndexChanged)
    Q_PROPERTY(quint8 dataBitsIndex
               READ dataBitsIndex
               WRITE setDataBits
               NOTIFY dataBitsChanged)
    Q_PROPERTY(quint8 stopBitsIndex
               READ stopBitsIndex
               WRITE setStopBits
               NOTIFY stopBitsChanged)
    Q_PROPERTY(quint8 flowControlIndex
               READ flowControlIndex
               WRITE setFlowControl
               NOTIFY flowControlChanged)
    Q_PROPERTY(qint32 baudRate
               READ baudRate
               WRITE setBaudRate
               NOTIFY baudRateChanged)
    Q_PROPERTY(QStringList portList
               READ portList
               NOTIFY availablePortsChanged)
    Q_PROPERTY(QStringList parityList
               READ parityList
               CONSTANT)
    Q_PROPERTY(QStringList baudRateList
               READ baudRateList
               CONSTANT)
    Q_PROPERTY(QStringList dataBitsList
               READ dataBitsList
               CONSTANT)
    Q_PROPERTY(QStringList stopBitsList
               READ stopBitsList
               CONSTANT)
    Q_PROPERTY(QStringList flowControlList
               READ flowControlList
               CONSTANT)
    Q_PROPERTY(QStringList consoleDisplayModes
               READ consoleDisplayModes
               CONSTANT)
    // clang-format on

signals:
    void portChanged();
    void parityChanged();
    void sendHexChanged();
    void baudRateChanged();
    void dataBitsChanged();
    void stopBitsChanged();
    void connectedChanged();
    void displayModeChanged();
    void flowControlChanged();
    void writeEnabledChanged();
    void textDocumentChanged();
    void baudRateIndexChanged();
    void maxBufferSizeChanged();
    void startSequenceChanged();
    void receivedBytesChanged();
    void finishSequenceChanged();
    void availablePortsChanged();
    void rx(const QString &rxData);
    void tx(const QString &txData);
    void connectionError(const QString &name);
    void packetReceived(const QByteArray &packet);

public:
    static SerialManager *getInstance();

    QSerialPort *port() const;

    bool readOnly() const;
    bool readWrite() const;
    bool connected() const;
    bool sendHexData() const;
    QString portName() const;
    int maxBufferSize() const;
    bool writeEnabled() const;
    QString inputMask() const;
    QString receivedBytes() const;
    QString startSequence() const;
    QString finishSequence() const;

    quint8 portIndex() const;
    quint8 parityIndex() const;
    quint8 displayMode() const;
    quint8 baudRateIndex() const;
    quint8 dataBitsIndex() const;
    quint8 stopBitsIndex() const;
    quint8 flowControlIndex() const;

    QStringList portList() const;
    QStringList parityList() const;
    QStringList baudRateList() const;
    QStringList dataBitsList() const;
    QStringList stopBitsList() const;
    QStringList flowControlList() const;
    QStringList consoleDisplayModes() const;

    qint32 baudRate() const;
    QSerialPort::Parity parity() const;
    QSerialPort::DataBits dataBits() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::FlowControl flowControl() const;

    Q_INVOKABLE void configureTextDocument(QQuickTextDocument *doc);

public slots:
    void clearTempBuffer();
    void disconnectDevice();
    void sendData(const QString &data);
    void setBaudRate(const qint32 rate);
    void setPort(const quint8 portIndex);
    void setSendHexData(const bool &sendHex);
    void setWriteEnabled(const bool enabled);
    void setParity(const quint8 parityIndex);
    void setBaudRateIndex(const quint8 index);
    void setDataBits(const quint8 dataBitsIndex);
    void setStopBits(const quint8 stopBitsIndex);
    void setDisplayMode(const quint8 displayMode);
    void setMaxBufferSize(const int maxBufferSize);
    void setStartSequence(const QString &sequence);
    void setFinishSequence(const QString &sequence);
    void setFlowControl(const quint8 flowControlIndex);

private slots:
    void readFrames();
    void onDataReceived();
    void refreshSerialDevices();
    void handleError(QSerialPort::SerialPortError error);

private:
    SerialManager();
    ~SerialManager();

    QList<QSerialPortInfo> validPorts() const;

private:
    QSerialPort *m_port;

    qint32 m_baudRate;
    QSerialPort::Parity m_parity;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::FlowControl m_flowControl;

    quint8 m_portIndex;
    quint8 m_displayMode;
    quint8 m_parityIndex;
    quint8 m_baudRateIndex;
    quint8 m_dataBitsIndex;
    quint8 m_stopBitsIndex;
    quint64 m_receivedBytes;
    quint8 m_flowControlIndex;

    bool m_writeEnabled;
    int m_maxBufferSize;
    bool m_sendHexData;

    QString m_startSeq;
    QString m_finishSeq;
    QByteArray m_tempBuffer;
    QStringList m_portList;
};

#endif
