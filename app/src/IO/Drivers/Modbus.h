/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QTimer>
#include <QObject>
#include <QString>
#include <QSettings>
#include <QByteArray>
#include <QModbusReply>
#include <QModbusClient>
#include <QModbusDevice>

#include "IO/HAL_Driver.h"

namespace IO
{
namespace Drivers
{
/**
 * @struct ModbusRegisterGroup
 * @brief Defines a group of registers to poll
 */
struct ModbusRegisterGroup
{
  quint8 registerType;
  quint16 startAddress;
  quint16 count;

  ModbusRegisterGroup()
    : registerType(0)
    , startAddress(0)
    , count(0)
  {
  }

  ModbusRegisterGroup(quint8 type, quint16 start, quint16 cnt)
    : registerType(type)
    , startAddress(start)
    , count(cnt)
  {
  }
};

/**
 * @class Modbus
 * @brief Serial Studio driver for Modbus RTU and Modbus TCP communication
 *
 * This class provides a complete implementation of the Modbus protocol driver
 * for Serial Studio, supporting both Modbus RTU (over serial ports) and
 * Modbus TCP (over network) connections.
 *
 * ## Supported Protocols
 * - **Modbus RTU**: Serial communication using RS-232/RS-485
 * - **Modbus TCP**: Network communication using TCP/IP
 *
 * ## Connection Lifecycle
 * The driver implements an asynchronous connection model:
 * 1. User selects protocol and configures parameters
 * 2. `open()` initiates connection (returns true if successful)
 * 3. For TCP: connection state changes from Connecting → Connected
 * 4. `onStateChanged()` emits `configurationChanged()` signal
 * 5. IO::Manager queries `isOpen()` to update UI
 *
 * ## Data Flow
 * - Uses timer-based polling to read Modbus registers periodically
 * - Converts register data to byte arrays for Serial Studio's frame parser
 * - Supports reading holding registers (function code 0x03)
 * - Emits `dataReceived()` signal with formatted register data
 *
 * ## Signal Integration
 * - `configurationChanged()`: Emitted when connection state or config changes
 * - `dataReceived()`: Emitted when register data is successfully read
 * - `connectionError()`: Emitted when connection or read errors occur
 * - `availableSerialPortsChanged()`: Emitted when serial port list updates
 *
 * @see HAL_Driver
 * @see IO::Manager
 */
class Modbus : public HAL_Driver
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(quint8 protocolIndex
             READ protocolIndex
             WRITE setProtocolIndex
             NOTIFY protocolIndexChanged)
  Q_PROPERTY(quint8 slaveAddress
             READ slaveAddress
             WRITE setSlaveAddress
             NOTIFY slaveAddressChanged)
  Q_PROPERTY(quint16 pollInterval
             READ pollInterval
             WRITE setPollInterval
             NOTIFY pollIntervalChanged)
  Q_PROPERTY(QString host
             READ host
             WRITE setHost
             NOTIFY hostChanged)
  Q_PROPERTY(quint16 port
             READ port
             WRITE setPort
             NOTIFY portChanged)
  Q_PROPERTY(quint8 serialPortIndex
             READ serialPortIndex
             WRITE setSerialPortIndex
             NOTIFY serialPortIndexChanged)
  Q_PROPERTY(quint8 parityIndex
             READ parityIndex
             WRITE setParityIndex
             NOTIFY parityIndexChanged)
  Q_PROPERTY(quint8 dataBitsIndex
             READ dataBitsIndex
             WRITE setDataBitsIndex
             NOTIFY dataBitsIndexChanged)
  Q_PROPERTY(quint8 stopBitsIndex
             READ stopBitsIndex
             WRITE setStopBitsIndex
             NOTIFY stopBitsIndexChanged)
  Q_PROPERTY(qint32 baudRate
             READ baudRate
             WRITE setBaudRate
             NOTIFY baudRateChanged)
  Q_PROPERTY(QStringList protocolList
             READ protocolList
             CONSTANT)
  Q_PROPERTY(QStringList serialPortList
             READ serialPortList
             NOTIFY availableSerialPortsChanged)
  Q_PROPERTY(QStringList parityList
             READ parityList
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList dataBitsList
             READ dataBitsList
             CONSTANT)
  Q_PROPERTY(QStringList stopBitsList
             READ stopBitsList
             CONSTANT)
  Q_PROPERTY(QStringList baudRateList
             READ baudRateList
             CONSTANT)
  Q_PROPERTY(QStringList registerTypeList
             READ registerTypeList
             NOTIFY languageChanged)
  Q_PROPERTY(int registerGroupCount
             READ registerGroupCount
             NOTIFY registerGroupsChanged)
  // clang-format on

signals:
  void portChanged();
  void hostChanged();
  void baudRateChanged();
  void parityIndexChanged();
  void dataBitsIndexChanged();
  void stopBitsIndexChanged();
  void protocolIndexChanged();
  void slaveAddressChanged();
  void pollIntervalChanged();
  void serialPortIndexChanged();
  void availableSerialPortsChanged();
  void registerGroupsChanged();
  void languageChanged();
  void connectionError(const QString &error);

private:
  explicit Modbus();
  Modbus(Modbus &&) = delete;
  Modbus(const Modbus &) = delete;
  Modbus &operator=(Modbus &&) = delete;
  Modbus &operator=(const Modbus &) = delete;

  ~Modbus();

public:
  static Modbus &instance();

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] quint8 protocolIndex() const;
  [[nodiscard]] quint8 slaveAddress() const;
  [[nodiscard]] quint16 pollInterval() const;
  [[nodiscard]] quint16 port() const;
  [[nodiscard]] QString host() const;

  [[nodiscard]] qint32 baudRate() const;
  [[nodiscard]] quint8 serialPortIndex() const;
  [[nodiscard]] quint8 parityIndex() const;
  [[nodiscard]] quint8 dataBitsIndex() const;
  [[nodiscard]] quint8 stopBitsIndex() const;

  [[nodiscard]] QStringList protocolList() const;
  [[nodiscard]] QStringList serialPortList() const;
  [[nodiscard]] QStringList parityList() const;
  [[nodiscard]] QStringList dataBitsList() const;
  [[nodiscard]] QStringList stopBitsList() const;
  [[nodiscard]] QStringList baudRateList() const;
  [[nodiscard]] QStringList registerTypeList() const;
  [[nodiscard]] int registerGroupCount() const;

  Q_INVOKABLE QString registerGroupInfo(const int index) const;

public slots:
  void clearRegisterGroups();
  void setupExternalConnections();
  void setPort(const quint16 port);
  void setHost(const QString &host);
  void setBaudRate(const qint32 rate);
  void setParityIndex(const quint8 index);
  void setProtocolIndex(const quint8 index);
  void removeRegisterGroup(const int index);
  void setDataBitsIndex(const quint8 index);
  void setStopBitsIndex(const quint8 index);
  void setSlaveAddress(const quint8 address);
  void setSerialPortIndex(const quint8 index);
  void setPollInterval(const quint16 interval);
  void addRegisterGroup(const quint8 type, const quint16 start,
                        const quint16 count);

private slots:
  void onReadReady();
  void pollRegisters();
  void pollNextGroup();
  void refreshSerialPorts();
  void onStateChanged(QModbusDevice::State state);
  void onErrorOccurred(QModbusDevice::Error error);

private:
  QTimer *m_pollTimer;
  QModbusClient *m_device;
  QModbusReply *m_lastReply;

  quint16 m_port;
  QString m_host;
  qint32 m_baudRate;
  quint8 m_parityIndex;
  quint8 m_slaveAddress;
  quint16 m_pollInterval;
  quint8 m_dataBitsIndex;
  quint8 m_stopBitsIndex;
  quint8 m_protocolIndex;
  int m_currentGroupIndex;
  quint8 m_serialPortIndex;
  QStringList m_serialPortNames;
  QStringList m_serialPortLocations;
  QVector<ModbusRegisterGroup> m_registerGroups;

  QSettings m_settings;
};
} // namespace Drivers
} // namespace IO
