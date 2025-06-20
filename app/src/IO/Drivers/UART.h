/*
 * UART Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QByteArray>
#include <QtSerialPort>

#include "IO/HAL_Driver.h"

namespace IO
{
namespace Drivers
{
/**
 * @brief The UART class
 * Serial Studio "driver" class to interact with serial port devices.
 */
class UART : public HAL_Driver
{
  // clang-format off
  Q_OBJECT
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
             NOTIFY baudRateChanged)
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
  explicit UART();
  UART(UART &&) = delete;
  UART(const UART &) = delete;
  UART &operator=(UART &&) = delete;
  UART &operator=(const UART &) = delete;

  ~UART();

public:
  static UART &instance();

  void close() override;

  [[nodiscard]] bool isOpen() const override;
  [[nodiscard]] bool isReadable() const override;
  [[nodiscard]] bool isWritable() const override;
  [[nodiscard]] bool configurationOk() const override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

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
  [[nodiscard]] QStringList baudRateList() const;

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
  void setupExternalConnections();
  void setBaudRate(const qint32 rate);
  void setDtrEnabled(const bool enabled);
  void setParity(const quint8 parityIndex);
  void setPortIndex(const quint8 portIndex);
  void registerDevice(const QString &device);
  void setDataBits(const quint8 dataBitsIndex);
  void setStopBits(const quint8 stopBitsIndex);
  void setAutoReconnect(const bool autoreconnect);
  void setFlowControl(const quint8 flowControlIndex);

private slots:
  void onReadyRead();
  void populateErrors();
  void refreshSerialDevices();
  void handleError(QSerialPort::SerialPortError error);

private:
  QVector<QSerialPortInfo> validPorts() const;

private:
  QSerialPort *m_port;

  bool m_dtrEnabled;
  bool m_autoReconnect;
  bool m_usingCustomSerialPort;

  int m_lastSerialDeviceIndex;

  qint32 m_baudRate;
  QSerialPort::Parity m_parity;
  QSerialPort::DataBits m_dataBits;
  QSerialPort::StopBits m_stopBits;
  QSerialPort::FlowControl m_flowControl;

  quint8 m_portIndex;
  quint8 m_parityIndex;
  quint8 m_dataBitsIndex;
  quint8 m_stopBitsIndex;
  quint8 m_flowControlIndex;

  QSettings m_settings;
  QStringList m_deviceNames;
  QStringList m_customDevices;
  QStringList m_deviceLocations;

  QMutex m_errorHandlerMutex;
  QMap<QSerialPort::SerialPortError, QString> m_errorDescriptions;
};
} // namespace Drivers
} // namespace IO
