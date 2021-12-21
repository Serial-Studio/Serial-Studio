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

#include <DataTypes.h>

#include <QObject>
#include <QString>
#include <QSettings>
#include <QByteArray>
#include <QtSerialPort>
#include <QTextCursor>
#include <QQuickTextDocument>

namespace IO
{
namespace DataSources
{
/**
 * @brief The Network class
 *
 * Serial Studio "driver" class to interact with serial port devices.
 */
class Serial : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QString portName
               READ portName
               NOTIFY portChanged)
    Q_PROPERTY(bool autoReconnect
               READ autoReconnect
               WRITE setAutoReconnect
               NOTIFY autoReconnectChanged)
    Q_PROPERTY(quint8 portIndex
               READ portIndex
               WRITE setPortIndex
               NOTIFY portIndexChanged)
    Q_PROPERTY(quint8 parityIndex
               READ parityIndex
               WRITE setParity
               NOTIFY parityChanged)
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
    Q_PROPERTY(StringList portList
               READ portList
               NOTIFY availablePortsChanged)
    Q_PROPERTY(StringList parityList
               READ parityList
               CONSTANT)
    Q_PROPERTY(StringList baudRateList
               READ baudRateList
               NOTIFY baudRateListChanged)
    Q_PROPERTY(StringList dataBitsList
               READ dataBitsList
               CONSTANT)
    Q_PROPERTY(StringList stopBitsList
               READ stopBitsList
               CONSTANT)
    Q_PROPERTY(StringList flowControlList
               READ flowControlList
               CONSTANT)
    // clang-format on

Q_SIGNALS:
    void portChanged();
    void parityChanged();
    void baudRateChanged();
    void dataBitsChanged();
    void stopBitsChanged();
    void portIndexChanged();
    void flowControlChanged();
    void baudRateListChanged();
    void autoReconnectChanged();
    void baudRateIndexChanged();
    void availablePortsChanged();
    void connectionError(const QString &name);

private:
    explicit Serial();
    Serial(Serial &&) = delete;
    Serial(const Serial &) = delete;
    Serial &operator=(Serial &&) = delete;
    Serial &operator=(const Serial &) = delete;

    ~Serial();

public:
    static Serial &instance();

    QString portName() const;
    QSerialPort *port() const;
    bool autoReconnect() const;
    bool configurationOk() const;

    quint8 portIndex() const;
    quint8 parityIndex() const;
    quint8 displayMode() const;
    quint8 dataBitsIndex() const;
    quint8 stopBitsIndex() const;
    quint8 flowControlIndex() const;

    StringList portList() const;
    StringList parityList() const;
    StringList baudRateList() const;
    StringList dataBitsList() const;
    StringList stopBitsList() const;
    StringList flowControlList() const;

    qint32 baudRate() const;
    QSerialPort::Parity parity() const;
    QSerialPort::DataBits dataBits() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::FlowControl flowControl() const;

    QSerialPort *openSerialPort();

public Q_SLOTS:
    void disconnectDevice();
    void setBaudRate(const qint32 rate);
    void setParity(const quint8 parityIndex);
    void setPortIndex(const quint8 portIndex);
    void appendBaudRate(const QString &baudRate);
    void setDataBits(const quint8 dataBitsIndex);
    void setStopBits(const quint8 stopBitsIndex);
    void setAutoReconnect(const bool autoreconnect);
    void setFlowControl(const quint8 flowControlIndex);

private Q_SLOTS:
    void readSettings();
    void writeSettings();
    void refreshSerialDevices();
    void handleError(QSerialPort::SerialPortError error);

private:
    QVector<QSerialPortInfo> validPorts() const;

private:
    QSerialPort *m_port;

    bool m_autoReconnect;
    int m_lastSerialDeviceIndex;

    qint32 m_baudRate;
    QSettings m_settings;
    QSerialPort::Parity m_parity;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::FlowControl m_flowControl;

    quint8 m_portIndex;
    quint8 m_parityIndex;
    quint8 m_dataBitsIndex;
    quint8 m_stopBitsIndex;
    quint8 m_flowControlIndex;

    StringList m_portList;
    StringList m_baudRateList;
};
}
}
