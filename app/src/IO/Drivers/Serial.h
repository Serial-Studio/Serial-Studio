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

#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QByteArray>
#include <QtSerialPort>
#include <QTextCursor>
#include <QQuickTextDocument>

#include "IO/HAL_Driver.h"

namespace IO
{
namespace Drivers
{
/**
 * @brief The Serial class
 * Serial Studio "driver" class to interact with serial port devices.
 */
class Serial : public HAL_Driver
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
  Q_PROPERTY(bool dtrEnabled
             READ dtrEnabled
             WRITE setDtrEnabled
             NOTIFY dtrEnabledChanged)
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
  Q_PROPERTY(QStringList portList
             READ portList
             NOTIFY availablePortsChanged)
  Q_PROPERTY(QStringList parityList
             READ parityList
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList baudRateList
             READ baudRateList
             NOTIFY baudRateListChanged)
  Q_PROPERTY(QStringList dataBitsList
             READ dataBitsList
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList stopBitsList
             READ stopBitsList
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList flowControlList
             READ flowControlList
             NOTIFY languageChanged)
  // clang-format on

signals:
  void portChanged();
  void parityChanged();
  void languageChanged();
  void baudRateChanged();
  void dataBitsChanged();
  void stopBitsChanged();
  void portIndexChanged();
  void dtrEnabledChanged();
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

  void close() override;

  [[nodiscard]] bool isOpen() const override;
  [[nodiscard]] bool isReadable() const override;
  [[nodiscard]] bool isWritable() const override;
  [[nodiscard]] bool configurationOk() const override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] QString portName() const;
  [[nodiscard]] QSerialPort *port() const;
  [[nodiscard]] bool autoReconnect() const;

  [[nodiscard]] bool dtrEnabled() const;
  [[nodiscard]] quint8 portIndex() const;
  [[nodiscard]] quint8 parityIndex() const;
  [[nodiscard]] quint8 displayMode() const;
  [[nodiscard]] quint8 dataBitsIndex() const;
  [[nodiscard]] quint8 stopBitsIndex() const;
  [[nodiscard]] quint8 flowControlIndex() const;

  [[nodiscard]] QStringList portList() const;
  [[nodiscard]] const QStringList &baudRateList() const;

  [[nodiscard]] QStringList parityList() const;
  [[nodiscard]] QStringList dataBitsList() const;
  [[nodiscard]] QStringList stopBitsList() const;
  [[nodiscard]] QStringList flowControlList() const;

  [[nodiscard]] qint32 baudRate() const;
  [[nodiscard]] QSerialPort::Parity parity() const;
  [[nodiscard]] QSerialPort::DataBits dataBits() const;
  [[nodiscard]] QSerialPort::StopBits stopBits() const;
  [[nodiscard]] QSerialPort::FlowControl flowControl() const;

public slots:
  void disconnectDevice();
  void setupExternalConnections();
  void setBaudRate(const qint32 rate);
  void setDtrEnabled(const bool enabled);
  void setParity(const quint8 parityIndex);
  void setPortIndex(const quint8 portIndex);
  void appendBaudRate(const QString &baudRate);
  void setDataBits(const quint8 dataBitsIndex);
  void setStopBits(const quint8 stopBitsIndex);
  void setAutoReconnect(const bool autoreconnect);
  void setFlowControl(const quint8 flowControlIndex);

private slots:
  void onReadyRead();
  void readSettings();
  void writeSettings();
  void refreshSerialDevices();
  void handleError(QSerialPort::SerialPortError error);

private:
  QVector<QSerialPortInfo> validPorts() const;

private:
  QSerialPort *m_port;

  bool m_dtrEnabled;
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

  QStringList m_portList;
  QStringList m_baudRateList;
};
} // namespace Drivers
} // namespace IO
