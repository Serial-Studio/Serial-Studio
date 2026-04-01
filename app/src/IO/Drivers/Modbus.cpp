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

#include "IO/Drivers/Modbus.h"

#include <QJsonObject>
#include <QModbusDataUnit>
#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructor function
 *
 * Initializes the Modbus driver with default settings.
 */
IO::Drivers::Modbus::Modbus()
  : m_pollTimer(new QTimer(this))
  , m_device(nullptr)
  , m_lastReply(nullptr)
  , m_port(5020)
  , m_host("127.0.0.1")
  , m_baudRate(9600)
  , m_parityIndex(0)
  , m_slaveAddress(1)
  , m_pollInterval(100)
  , m_dataBitsIndex(3)
  , m_stopBitsIndex(0)
  , m_protocolIndex(1)
  , m_currentGroupIndex(0)
  , m_serialPortIndex(0)
{
  // Restore persisted settings
  m_slaveAddress  = m_settings.value("ModbusDriver/slaveAddress", 1).toUInt();
  m_protocolIndex = m_settings.value("ModbusDriver/protocolIndex", 1).toUInt();
  m_pollInterval  = m_settings.value("ModbusDriver/pollInterval", 100).toUInt();

  m_port = m_settings.value("ModbusDriver/port", 5020).toUInt();
  m_host = m_settings.value("ModbusDriver/host", "127.0.0.1").toString();
  // clang-format off
  m_baudRate = m_settings.value("ModbusDriver/baudRate", 9600).toInt();
  m_parityIndex = m_settings.value("ModbusDriver/parityIndex", 0).toUInt();
  m_dataBitsIndex = m_settings.value("ModbusDriver/dataBitsIndex", 3).toUInt();
  m_stopBitsIndex = m_settings.value("ModbusDriver/stopBitsIndex", 0).toUInt();
  m_serialPortIndex = m_settings.value("ModbusDriver/serialPortIndex", 0).toUInt();
  // clang-format on

  // Restore register groups
  // clang-format off
  const int groupCount = m_settings.beginReadArray("ModbusDriver/registerGroups");
  for (int i = 0; i < groupCount; ++i)
  {
    m_settings.setArrayIndex(i);
    ModbusRegisterGroup group;
    group.registerType = m_settings.value("type", 0).toUInt();
    group.startAddress = m_settings.value("start", 0).toUInt();
    group.count = m_settings.value("count", 0).toUInt();
    if (group.count > 0 && group.count <= 125)
      m_registerGroups.append(group);
  }
  m_settings.endArray();
  // clang-format on

  // Connect poll timer
  connect(m_pollTimer, &QTimer::timeout, this, &IO::Drivers::Modbus::pollRegisters);

  // Propagate configuration changes
  connect(this,
          &IO::Drivers::Modbus::protocolIndexChanged,
          this,
          &IO::Drivers::Modbus::configurationChanged);
  connect(this,
          &IO::Drivers::Modbus::serialPortIndexChanged,
          this,
          &IO::Drivers::Modbus::configurationChanged);
  connect(
    this, &IO::Drivers::Modbus::hostChanged, this, &IO::Drivers::Modbus::configurationChanged);
  connect(
    this, &IO::Drivers::Modbus::portChanged, this, &IO::Drivers::Modbus::configurationChanged);
  connect(this,
          &IO::Drivers::Modbus::slaveAddressChanged,
          this,
          &IO::Drivers::Modbus::configurationChanged);
  connect(this,
          &IO::Drivers::Modbus::pollIntervalChanged,
          this,
          &IO::Drivers::Modbus::configurationChanged);
  connect(
    this, &IO::Drivers::Modbus::baudRateChanged, this, &IO::Drivers::Modbus::configurationChanged);
  connect(this,
          &IO::Drivers::Modbus::parityIndexChanged,
          this,
          &IO::Drivers::Modbus::configurationChanged);
  connect(this,
          &IO::Drivers::Modbus::dataBitsIndexChanged,
          this,
          &IO::Drivers::Modbus::configurationChanged);
  connect(this,
          &IO::Drivers::Modbus::stopBitsIndexChanged,
          this,
          &IO::Drivers::Modbus::configurationChanged);
}

/**
 * @brief Destructor — releases all Modbus resources without invoking virtual functions.
 */
IO::Drivers::Modbus::~Modbus()
{
  doClose();
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the current Modbus connection.
 *
 * Delegates resource release to doClose() and emits state-change signals.
 */
void IO::Drivers::Modbus::close()
{
  doClose();

  Q_EMIT configurationChanged();
  Q_EMIT availableSerialPortsChanged();
}

/**
 * @brief Non-virtual cleanup implementation shared by close() and ~Modbus().
 *
 * Stops the poll timer, cancels any pending reply, and disconnects/deletes
 * the device.  Safe to call when all pointers are null.
 */
void IO::Drivers::Modbus::doClose()
{
  if (m_pollTimer)
    m_pollTimer->stop();

  if (m_lastReply) {
    disconnect(m_lastReply, nullptr, this, nullptr);
    m_lastReply->deleteLater();
    m_lastReply = nullptr;
  }

  if (m_device) {
    disconnect(m_device, nullptr, this, nullptr);

    if (m_device->state() != QModbusDevice::UnconnectedState)
      m_device->disconnectDevice();

    m_device->deleteLater();
    m_device = nullptr;
  }
}

/**
 * @brief Returns true if a Modbus connection is currently open
 */
bool IO::Drivers::Modbus::isOpen() const noexcept
{
  if (m_device)
    return m_device->state() == QModbusDevice::ConnectedState;

  return false;
}

/**
 * @brief Returns true if the current Modbus device is readable
 */
bool IO::Drivers::Modbus::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true if the current Modbus device is writable
 */
bool IO::Drivers::Modbus::isWritable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true if the user has selected appropriate controls & options
 *        to connect to a Modbus device
 */
bool IO::Drivers::Modbus::configurationOk() const noexcept
{
  if (m_protocolIndex == 0)
    return m_serialPortIndex > 0;

  else if (m_protocolIndex == 1)
    return !m_host.isEmpty() && m_port > 0;

  return false;
}

/**
 * @brief Writes data to the Modbus device
 *
 * The input data is expected to be in Modbus format.
 * This implementation supports writing single registers (function code 0x06).
 *
 * @param data The data to write
 * @return Number of bytes successfully written
 */
qint64 IO::Drivers::Modbus::write(const QByteArray& data)
{
  if (!isWritable() || data.length() < 4)
    return 0;

  quint16 register_address = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
  quint16 register_value   = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);

  QModbusDataUnit write_unit(QModbusDataUnit::HoldingRegisters, register_address, 1);
  write_unit.setValue(0, register_value);

  if (auto* reply = m_device->sendWriteRequest(write_unit, m_slaveAddress)) {
    if (!reply->isFinished())
      connect(reply, &QModbusReply::finished, reply, &QModbusReply::deleteLater);
    else
      reply->deleteLater();

    Q_EMIT dataSent(data);
    return data.length();
  }

  return 0;
}

/**
 * @brief Opens the Modbus device with the given mode
 *
 * Establishes connection to Modbus device (RTU or TCP) with comprehensive
 * error handling and validation:
 *
 * For Modbus RTU:
 * - Validates serial port selection and availability
 * - Configures serial parameters (baud, parity, data/stop bits)
 * - Creates QModbusRtuSerialClient
 *
 * For Modbus TCP:
 * - Validates host address and port
 * - Creates QModbusTcpClient
 * - Initiates asynchronous connection
 *
 * Safety features:
 * - Closes existing connection before opening new one
 * - Validates all parameters before creating device
 * - Sets timeout and retry parameters
 * - Connects state change and error signals
 * - Starts polling timer only after successful connection
 * - Emits configurationChanged() to update UI
 *
 * @param mode The open mode (ReadOnly or ReadWrite) - currently unused
 * @return True if connection initiated successfully, false on error
 *
 * @note For TCP connections, true return means connection initiated,
 *       not necessarily completed. Use onStateChanged() to detect
 *       when connection actually completes.
 */
bool IO::Drivers::Modbus::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  close();

  if (!configurationOk())
    return false;

  // Track the target device/host for error messages
  QString connectionTarget;

  if (m_protocolIndex == 0) {
    // Ensure the port list is populated (live drivers don't run the 1 Hz timer)
    if (m_serialPortNames.isEmpty())
      refreshSerialPorts();

    auto ports  = serialPortList();
    auto portId = serialPortIndex();
    if (portId < 1 || portId >= ports.count())
      return false;

    if (portId >= m_serialPortLocations.count()) {
      Misc::Utilities::showMessageBox(tr("Invalid Serial Port"),
                                      tr("The selected serial port \"%1\" is no longer available. "
                                         "Please refresh the port list and try again.")
                                        .arg(ports.value(portId)),
                                      QMessageBox::Critical);
      return false;
    }

    const auto portName = m_serialPortLocations.at(portId);

    m_device = new QModbusRtuSerialClient(this);

    QSerialPort::Parity parity = QSerialPort::NoParity;
    if (m_parityIndex == 1)
      parity = QSerialPort::EvenParity;
    else if (m_parityIndex == 2)
      parity = QSerialPort::OddParity;
    else if (m_parityIndex == 3)
      parity = QSerialPort::SpaceParity;
    else if (m_parityIndex == 4)
      parity = QSerialPort::MarkParity;

    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    if (m_dataBitsIndex == 0)
      dataBits = QSerialPort::Data5;
    else if (m_dataBitsIndex == 1)
      dataBits = QSerialPort::Data6;
    else if (m_dataBitsIndex == 2)
      dataBits = QSerialPort::Data7;
    else if (m_dataBitsIndex == 3)
      dataBits = QSerialPort::Data8;

    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    if (m_stopBitsIndex == 1)
      stopBits = QSerialPort::OneAndHalfStop;
    else if (m_stopBitsIndex == 2)
      stopBits = QSerialPort::TwoStop;

    m_device->setConnectionParameter(QModbusDevice::SerialPortNameParameter, portName);
    m_device->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baudRate);
    m_device->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
    m_device->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
    m_device->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);

    connectionTarget = ports.value(portId);
  }

  else if (m_protocolIndex == 1) {
    auto* tcp_device = new QModbusTcpClient(this);
    tcp_device->setConnectionParameter(QModbusDevice::NetworkAddressParameter, m_host);
    tcp_device->setConnectionParameter(QModbusDevice::NetworkPortParameter, m_port);
    m_device = tcp_device;

    connectionTarget = QStringLiteral("%1:%2").arg(m_host).arg(m_port);
  }

  if (!m_device) {
    Misc::Utilities::showMessageBox(
      tr("Modbus Initialization Failed"),
      tr("Unable to create Modbus device. Please check your system configuration and try again."),
      QMessageBox::Critical);
    return false;
  }

  m_device->setTimeout(1000);
  m_device->setNumberOfRetries(3);

  connect(m_device,
          &QModbusClient::stateChanged,
          this,
          &IO::Drivers::Modbus::onStateChanged,
          Qt::UniqueConnection);
  connect(m_device,
          &QModbusClient::errorOccurred,
          this,
          &IO::Drivers::Modbus::onErrorOccurred,
          Qt::UniqueConnection);

  if (!m_device->connectDevice()) {
    QString error = m_device->errorString();
    m_device->deleteLater();
    m_device = nullptr;

    Misc::Utilities::showMessageBox(
      tr("Modbus Connection Failed"),
      error.isEmpty() ? tr("Unable to connect to \"%1\". Please check your connection settings.")
                          .arg(connectionTarget)
                      : tr("\"%1\": %2").arg(connectionTarget, error),
      QMessageBox::Critical);
    return false;
  }

  if (m_device->state() == QModbusDevice::ConnectedState)
    m_pollTimer->start(m_pollInterval);

  Q_EMIT configurationChanged();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current protocol index (0 = RTU, 1 = TCP)
 */
quint8 IO::Drivers::Modbus::protocolIndex() const
{
  return m_protocolIndex;
}

/**
 * @brief Returns the current slave address
 */
quint8 IO::Drivers::Modbus::slaveAddress() const
{
  return m_slaveAddress;
}

/**
 * @brief Returns the polling interval in milliseconds
 */
quint16 IO::Drivers::Modbus::pollInterval() const
{
  return m_pollInterval;
}

/**
 * @brief Returns the TCP port (for Modbus TCP)
 */
quint16 IO::Drivers::Modbus::port() const
{
  return m_port;
}

/**
 * @brief Returns the TCP host address (for Modbus TCP)
 */
QString IO::Drivers::Modbus::host() const
{
  return m_host;
}

/**
 * @brief Returns the list of available Modbus protocols
 */
QStringList IO::Drivers::Modbus::protocolList() const
{
  QStringList list;
  list << "Modbus RTU";
  list << "Modbus TCP";
  return list;
}

/**
 * @brief Returns the baud rate for Modbus RTU
 */
qint32 IO::Drivers::Modbus::baudRate() const
{
  return m_baudRate;
}

/**
 * @brief Returns the serial port index
 */
quint8 IO::Drivers::Modbus::serialPortIndex() const
{
  return m_serialPortIndex;
}

/**
 * @brief Returns the parity index
 */
quint8 IO::Drivers::Modbus::parityIndex() const
{
  return m_parityIndex;
}

/**
 * @brief Returns the data bits index
 */
quint8 IO::Drivers::Modbus::dataBitsIndex() const
{
  return m_dataBitsIndex;
}

/**
 * @brief Returns the stop bits index
 */
quint8 IO::Drivers::Modbus::stopBitsIndex() const
{
  return m_stopBitsIndex;
}

/**
 * @brief Returns the list of available serial ports
 */
QStringList IO::Drivers::Modbus::serialPortList() const
{
  return m_serialPortNames;
}

/**
 * @brief Returns the list of parity options
 */
QStringList IO::Drivers::Modbus::parityList() const
{
  QStringList list;
  list << tr("None");
  list << tr("Even");
  list << tr("Odd");
  list << tr("Space");
  list << tr("Mark");
  return list;
}

/**
 * @brief Returns the list of data bits options
 */
QStringList IO::Drivers::Modbus::dataBitsList() const
{
  QStringList list;
  list << "5";
  list << "6";
  list << "7";
  list << "8";
  return list;
}

/**
 * @brief Returns the list of stop bits options
 */
QStringList IO::Drivers::Modbus::stopBitsList() const
{
  QStringList list;
  list << "1";
  list << "1.5";
  list << "2";
  return list;
}

/**
 * @brief Returns the list of standard baud rates
 */
QStringList IO::Drivers::Modbus::baudRateList() const
{
  QStringList list;
  list << "1200";
  list << "2400";
  list << "4800";
  list << "9600";
  list << "19200";
  list << "38400";
  list << "57600";
  list << "115200";
  return list;
}

/**
 * @brief Returns the list of supported Modbus register types
 */
QStringList IO::Drivers::Modbus::registerTypeList() const
{
  QStringList list;
  list << tr("Holding Registers (0x03)");
  list << tr("Input Registers (0x04)");
  list << tr("Coils (0x01)");
  list << tr("Discrete Inputs (0x02)");
  return list;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the TCP host address
 */
void IO::Drivers::Modbus::setHost(const QString& host)
{
  if (m_host == host)
    return;

  m_host = host;
  m_settings.setValue("ModbusDriver/host", host);
  Q_EMIT hostChanged();
}

/**
 * @brief Sets the TCP port
 */
void IO::Drivers::Modbus::setPort(const quint16 port)
{
  if (m_port == port)
    return;

  m_port = port;
  m_settings.setValue("ModbusDriver/port", port);
  Q_EMIT portChanged();
}

/**
 * @brief Sets the protocol index (0 = RTU, 1 = TCP)
 */
void IO::Drivers::Modbus::setProtocolIndex(const quint8 index)
{
  if (index < 2) {
    m_protocolIndex = index;
    m_settings.setValue("ModbusDriver/protocolIndex", index);
    Q_EMIT protocolIndexChanged();
  }
}

/**
 * @brief Sets the slave address
 */
void IO::Drivers::Modbus::setSlaveAddress(const quint8 address)
{
  if (m_slaveAddress == address)
    return;

  m_slaveAddress = address;
  m_settings.setValue("ModbusDriver/slaveAddress", address);
  Q_EMIT slaveAddressChanged();
}

/**
 * @brief Sets the polling interval in milliseconds
 */
void IO::Drivers::Modbus::setPollInterval(const quint16 interval)
{
  if (m_pollInterval == interval)
    return;

  m_pollInterval = interval;
  m_settings.setValue("ModbusDriver/pollInterval", interval);

  if (m_pollTimer->isActive()) {
    m_pollTimer->stop();
    m_pollTimer->start(m_pollInterval);
  }

  Q_EMIT pollIntervalChanged();
}

/**
 * @brief Adds a register group to poll
 */
void IO::Drivers::Modbus::addRegisterGroup(const quint8 type,
                                           const quint16 start,
                                           const quint16 count)
{
  if (count > 0 && count <= 125) {
    for (const auto& group : std::as_const(m_registerGroups))
      if (group.registerType == type && group.startAddress == start && group.count == count)
        return;

    m_registerGroups.append(ModbusRegisterGroup(type, start, count));

    m_settings.beginWriteArray("ModbusDriver/registerGroups");
    for (int i = 0; i < m_registerGroups.size(); ++i) {
      m_settings.setArrayIndex(i);
      m_settings.setValue("type", m_registerGroups[i].registerType);
      m_settings.setValue("start", m_registerGroups[i].startAddress);
      m_settings.setValue("count", m_registerGroups[i].count);
    }
    m_settings.endArray();

    Q_EMIT registerGroupsChanged();
  }
}

/**
 * @brief Removes a register group at the specified index
 */
void IO::Drivers::Modbus::removeRegisterGroup(const int index)
{
  if (index >= 0 && index < m_registerGroups.count()) {
    m_registerGroups.removeAt(index);
    if (m_currentGroupIndex >= m_registerGroups.count())
      m_currentGroupIndex = 0;

    m_settings.beginWriteArray("ModbusDriver/registerGroups");
    for (int i = 0; i < m_registerGroups.size(); ++i) {
      m_settings.setArrayIndex(i);
      m_settings.setValue("type", m_registerGroups[i].registerType);
      m_settings.setValue("start", m_registerGroups[i].startAddress);
      m_settings.setValue("count", m_registerGroups[i].count);
    }
    m_settings.endArray();

    Q_EMIT registerGroupsChanged();
  }
}

/**
 * @brief Clears all register groups
 */
void IO::Drivers::Modbus::clearRegisterGroups()
{
  m_registerGroups.clear();
  m_currentGroupIndex = 0;

  m_settings.beginWriteArray("ModbusDriver/registerGroups");
  m_settings.endArray();

  Q_EMIT registerGroupsChanged();
}

/**
 * @brief Returns the number of configured register groups
 */
int IO::Drivers::Modbus::registerGroupCount() const
{
  return m_registerGroups.count();
}

/**
 * @brief Returns information about a register group
 */
QString IO::Drivers::Modbus::registerGroupInfo(const int index) const
{
  if (index < 0 || index >= m_registerGroups.count())
    return QString();

  // clang-format off
  const auto &group = m_registerGroups[index];
  const QStringList types = registerTypeList();
  const QString typeName = (group.registerType < types.count()) ? types[group.registerType] : "";
  // clang-format on

  return QString("%1: %2 @ %3 (count: %4)")
    .arg(index + 1)
    .arg(typeName)
    .arg(group.startAddress)
    .arg(group.count);
}

/**
 * @brief Sets the baud rate for Modbus RTU
 */
void IO::Drivers::Modbus::setBaudRate(const qint32 rate)
{
  if (m_baudRate != rate && rate > 0) {
    m_baudRate = rate;
    m_settings.setValue("ModbusDriver/baudRate", rate);
    Q_EMIT baudRateChanged();
  }
}

/**
 * @brief Sets the serial port index
 */
void IO::Drivers::Modbus::setSerialPortIndex(const quint8 index)
{
  // Ensure port list is populated so the clamp range is correct
  if (m_serialPortNames.isEmpty())
    refreshSerialPorts();

  if (index < m_serialPortNames.count()) {
    m_serialPortIndex = index;
    m_settings.setValue("ModbusDriver/serialPortIndex", index);
    Q_EMIT serialPortIndexChanged();
  }
}

/**
 * @brief Sets the parity index
 */
void IO::Drivers::Modbus::setParityIndex(const quint8 index)
{
  if (index < 5) {
    m_parityIndex = index;
    m_settings.setValue("ModbusDriver/parityIndex", index);
    Q_EMIT parityIndexChanged();
  }
}

/**
 * @brief Sets the data bits index
 */
void IO::Drivers::Modbus::setDataBitsIndex(const quint8 index)
{
  if (index < 4) {
    m_dataBitsIndex = index;
    m_settings.setValue("ModbusDriver/dataBitsIndex", index);
    Q_EMIT dataBitsIndexChanged();
  }
}

/**
 * @brief Sets the stop bits index
 */
void IO::Drivers::Modbus::setStopBitsIndex(const quint8 index)
{
  if (index < 3) {
    m_stopBitsIndex = index;
    m_settings.setValue("ModbusDriver/stopBitsIndex", index);
    Q_EMIT stopBitsIndexChanged();
  }
}

/**
 * @brief Sets up external connections for timer events and translations
 */
void IO::Drivers::Modbus::setupExternalConnections()
{
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &IO::Drivers::Modbus::refreshSerialPorts);

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::Drivers::Modbus::languageChanged);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Polls Modbus registers at the configured interval
 *
 * Reads holding registers and emits the data as a byte array compatible
 * with Serial Studio's frame parser.
 *
 * This function implements robust error handling:
 * - Checks if device is connected before attempting read
 * - Prevents overlapping requests by tracking pending replies
 * - Uses Qt::UniqueConnection to prevent duplicate signal connections
 * - Handles both synchronous and asynchronous reply completion
 *
 * @note This is called periodically by m_pollTimer
 */
void IO::Drivers::Modbus::pollRegisters()
{
  if (!m_device)
    return;

  if (!isOpen())
    return;

  if (m_lastReply && !m_lastReply->isFinished())
    return;

  if (m_registerGroups.isEmpty())
    return;

  m_currentGroupIndex = 0;
  pollNextGroup();
}

/**
 * @brief Polls the next register group in multi-group mode
 */
void IO::Drivers::Modbus::pollNextGroup()
{
  if (!m_device || !isOpen())
    return;

  if (m_lastReply && !m_lastReply->isFinished())
    return;

  if (m_currentGroupIndex >= m_registerGroups.count())
    return;

  const auto& group = m_registerGroups[m_currentGroupIndex];

  QModbusDataUnit::RegisterType registerType;
  switch (group.registerType) {
    case 0:
      registerType = QModbusDataUnit::HoldingRegisters;
      break;
    case 1:
      registerType = QModbusDataUnit::InputRegisters;
      break;
    case 2:
      registerType = QModbusDataUnit::Coils;
      break;
    case 3:
      registerType = QModbusDataUnit::DiscreteInputs;
      break;
    default:
      registerType = QModbusDataUnit::HoldingRegisters;
      break;
  }

  QModbusDataUnit read_unit(registerType, group.startAddress, group.count);

  auto* reply = m_device->sendReadRequest(read_unit, m_slaveAddress);
  if (!reply)
    return;

  m_lastReply = reply;

  connect(
    reply, &QModbusReply::finished, this, &IO::Drivers::Modbus::onReadReady, Qt::UniqueConnection);
}

/**
 * @brief Handles completed Modbus read operations
 *
 * Converts Modbus register values to a byte array format compatible
 * with Serial Studio's JavaScript frame parser.
 *
 * Data format emitted:
 * - Byte 0: Slave address
 * - Byte 1: Function code (0x03 for holding registers)
 * - Byte 2: Byte count (register_count * 2)
 * - Bytes 3+: Register values (16-bit big-endian)
 *
 * Error handling:
 * - Validates reply pointer before use
 * - Checks for Modbus protocol errors
 * - Validates data unit before accessing values
 * - Safely deletes reply object
 * - Clears m_lastReply reference
 *
 * @note This slot is connected to QModbusReply::finished signal
 */
void IO::Drivers::Modbus::onReadReady()
{
  auto* reply = qobject_cast<QModbusReply*>(sender());
  if (!reply) {
    m_lastReply = nullptr;
    return;
  }

  if (reply != m_lastReply) {
    reply->deleteLater();
    return;
  }

  m_lastReply = nullptr;

  if (reply->error() != QModbusDevice::NoError) {
    reply->deleteLater();
    return;
  }

  const QModbusDataUnit unit = reply->result();
  if (!unit.isValid() || unit.valueCount() == 0) {
    reply->deleteLater();
    return;
  }

  try {
    const QModbusDataUnit::RegisterType registerType = unit.registerType();

    quint8 functionCode;
    bool isRegisterType = false;

    switch (registerType) {
      case QModbusDataUnit::HoldingRegisters:
        functionCode   = 0x03;
        isRegisterType = true;
        break;
      case QModbusDataUnit::InputRegisters:
        functionCode   = 0x04;
        isRegisterType = true;
        break;
      case QModbusDataUnit::Coils:
        functionCode   = 0x01;
        isRegisterType = false;
        break;
      case QModbusDataUnit::DiscreteInputs:
        functionCode   = 0x02;
        isRegisterType = false;
        break;
      default:
        functionCode   = 0x03;
        isRegisterType = true;
        break;
    }

    QByteArray data;
    if (isRegisterType) {
      data.reserve(3 + unit.valueCount() * 2);
      data.append(static_cast<char>(m_slaveAddress));
      data.append(static_cast<char>(functionCode));
      data.append(static_cast<char>(unit.valueCount() * 2));

      for (int i = 0; i < unit.valueCount(); ++i) {
        quint16 value = unit.value(i);
        data.append(static_cast<char>((value >> 8) & 0xFF));
        data.append(static_cast<char>(value & 0xFF));
      }
    }

    else {
      const int byteCount = (unit.valueCount() + 7) / 8;
      data.reserve(3 + byteCount);
      data.append(static_cast<char>(m_slaveAddress));
      data.append(static_cast<char>(functionCode));
      data.append(static_cast<char>(byteCount));

      for (int i = 0; i < byteCount; ++i) {
        quint8 byte = 0;
        for (int bit = 0; bit < 8 && (i * 8 + bit) < unit.valueCount(); ++bit)
          if (unit.value(i * 8 + bit))
            byte |= (1 << bit);
        data.append(static_cast<char>(byte));
      }
    }

    Q_EMIT dataReceived(makeByteArray(std::move(data)));
  }

  catch (...) {
  }

  reply->deleteLater();

  ++m_currentGroupIndex;
  if (m_currentGroupIndex < m_registerGroups.count())
    pollNextGroup();
}

/**
 * @brief Handles Modbus device state changes
 *
 * Called when the Modbus device transitions between states:
 * - UnconnectedState: Device is not connected
 * - ConnectingState: Connection attempt in progress (TCP)
 * - ConnectedState: Device successfully connected
 * - ClosingState: Connection being closed
 *
 * ## Critical for Async Connections
 * For Modbus TCP, the connection is asynchronous:
 * 1. open() calls connectDevice() - returns true immediately
 * 2. Device state is Connecting
 * 3. Eventually transitions to Connected
 * 4. This handler is called with state=Connected
 * 5. We start the poll timer NOW (when actually connected)
 * 6. Emit configurationChanged() to update UI
 *
 * For Modbus RTU, connection is synchronous so the poll timer
 * is already started in open().
 *
 * @param state New device state
 *
 * @note Emits configurationChanged() which triggers Manager to
 *       re-query isOpen() and update the connected status in UI
 * @note Starts poll timer when connection completes (TCP only)
 */
void IO::Drivers::Modbus::onStateChanged(QModbusDevice::State state)
{
  if (!m_device)
    return;

  if (state == QModbusDevice::ConnectedState) {
    if (m_pollTimer && !m_pollTimer->isActive())
      m_pollTimer->start(m_pollInterval);
  }

  else if (state == QModbusDevice::UnconnectedState) {
    if (m_pollTimer)
      m_pollTimer->stop();
  }

  Q_EMIT configurationChanged();
}

/**
 * @brief Handles Modbus device errors
 *
 * Called when Modbus protocol or communication errors occur:
 * - Connection failures
 * - Read/write timeouts
 * - Protocol violations
 * - Device disconnections
 *
 * Shows a message box with descriptive error message to inform user
 * of the problem.
 *
 * @param error The error code from QModbusDevice::Error enum
 *
 * @note Does not automatically disconnect - device may recover
 *       from transient errors automatically
 */
void IO::Drivers::Modbus::onErrorOccurred(QModbusDevice::Error error)
{
  if (!m_device)
    return;

  if (error == QModbusDevice::NoError)
    return;

  QString errorString = m_device->errorString();
  if (errorString.isEmpty())
    errorString = tr("Error code: %1").arg(static_cast<int>(error));

  Misc::Utilities::showMessageBox(
    tr("Modbus Communication Error"), errorString, QMessageBox::Warning);
}

/**
 * @brief Refreshes the list of available serial ports
 *
 * Scans for available serial ports and updates the internal lists.
 * Uses the same filtering logic as UART driver:
 * - On macOS, filters out tty.* devices (only accepts cu.* devices)
 * - On Windows, shows port name with description
 * - On other platforms, shows port name only
 *
 * This is called periodically to detect newly connected or disconnected ports.
 *
 * @note The first item is always "Select Port" pointing to /dev/null
 * @note macOS filtering: https://stackoverflow.com/a/37688347
 */
void IO::Drivers::Modbus::refreshSerialPorts()
{
  QStringList names;
  QStringList locations;

  locations.append("/dev/null");
  names.append(tr("Select Port"));

  const auto ports = QSerialPortInfo::availablePorts();
  for (const auto& info : ports) {
    if (!info.isNull()) {
#ifdef Q_OS_MACOS
      if (info.portName().toLower().startsWith("tty."))
        continue;
#endif

#ifdef Q_OS_WIN
      names.append(info.portName() + "  " + info.description());
#else
      names.append(info.portName());
#endif
      locations.append(info.systemLocation());
    }
  }

  if (m_serialPortNames != names) {
    m_serialPortNames     = names;
    m_serialPortLocations = locations;
    Q_EMIT availableSerialPortsChanged();

    if (m_serialPortIndex >= m_serialPortNames.count()) {
      m_serialPortIndex = 0;
      m_settings.setValue("ModbusDriver/serialPortIndex", 0);
      Q_EMIT serialPortIndexChanged();
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns cross-platform hardware identifiers for the currently selected serial port.
 *
 * Only active in RTU mode (protocolIndex == 0). For TCP mode, host/port strings are already
 * cross-platform and stored via driverProperties().
 */
QJsonObject IO::Drivers::Modbus::deviceIdentifier() const
{
  if (m_protocolIndex != 0 || m_serialPortIndex < 1)
    return {};

  const auto ports = QSerialPortInfo::availablePorts();
  QVector<QSerialPortInfo> filtered;
  for (const auto& info : ports) {
    if (!info.isNull()) {
#ifdef Q_OS_MACOS
      if (info.portName().toLower().startsWith("tty."))
        continue;
#endif
      filtered.append(info);
    }
  }

  const int idx = m_serialPortIndex - 1;
  if (idx < 0 || idx >= filtered.count())
    return {};

  const auto& info = filtered.at(idx);
  QJsonObject id;

  if (info.hasVendorIdentifier())
    id.insert(QStringLiteral("vid"),
              QString::number(info.vendorIdentifier(), 16).rightJustified(4, '0').toUpper());

  if (info.hasProductIdentifier())
    id.insert(QStringLiteral("pid"),
              QString::number(info.productIdentifier(), 16).rightJustified(4, '0').toUpper());

  const auto serial = info.serialNumber();
  if (!serial.isEmpty())
    id.insert(QStringLiteral("serial"), serial);

  id.insert(QStringLiteral("portName"), info.portName());

  const auto desc = info.description();
  if (!desc.isEmpty())
    id.insert(QStringLiteral("description"), desc);

  return id;
}

/**
 * @brief Tries to find and select a serial port matching a previously saved identifier.
 *
 * Only active in RTU mode. Uses VID+PID+serial scoring (identical to UART).
 */
bool IO::Drivers::Modbus::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty() || m_protocolIndex != 0)
    return false;

  // Ensure port list is populated so setSerialPortIndex() can clamp correctly
  if (m_serialPortNames.isEmpty())
    refreshSerialPorts();

  const auto ports = QSerialPortInfo::availablePorts();
  QVector<QSerialPortInfo> filtered;
  for (const auto& info : ports) {
    if (!info.isNull()) {
#ifdef Q_OS_MACOS
      if (info.portName().toLower().startsWith("tty."))
        continue;
#endif
      filtered.append(info);
    }
  }

  const auto savedVid  = id.value(QStringLiteral("vid")).toString();
  const auto savedPid  = id.value(QStringLiteral("pid")).toString();
  const auto savedSer  = id.value(QStringLiteral("serial")).toString();
  const auto savedName = id.value(QStringLiteral("portName")).toString();
  const auto savedDesc = id.value(QStringLiteral("description")).toString();

  int bestScore = 0;
  int bestIndex = -1;

  for (int i = 0; i < filtered.count(); ++i) {
    const auto& info = filtered.at(i);
    int score        = 0;

    if (!savedVid.isEmpty() && info.hasVendorIdentifier()) {
      const auto vid =
        QString::number(info.vendorIdentifier(), 16).rightJustified(4, '0').toUpper();
      const auto pid =
        QString::number(info.productIdentifier(), 16).rightJustified(4, '0').toUpper();
      if (vid == savedVid && pid == savedPid) {
        score += 100;

        if (!savedSer.isEmpty() && info.serialNumber() == savedSer)
          score += 50;
      }
    }

    if (!savedDesc.isEmpty() && info.description() == savedDesc)
      score += 10;

    if (!savedName.isEmpty() && info.portName() == savedName)
      score += 5;

    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  if (bestIndex >= 0) {
    setSerialPortIndex(static_cast<quint8>(bestIndex + 1));
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Modbus configuration as a flat list of editable properties.
 *
 * Only the properties relevant to the current protocol are included:
 *   - RTU (index 0): serial port, baud rate, parity, data bits, stop bits.
 *   - TCP (index 1): host address, port.
 * Slave address and poll interval are common to both protocols.
 *
 * @return List of DriverProperty descriptors with current values.
 */
QList<IO::DriverProperty> IO::Drivers::Modbus::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty proto;
  proto.key     = QStringLiteral("protocolIndex");
  proto.label   = tr("Protocol");
  proto.type    = IO::DriverProperty::ComboBox;
  proto.value   = m_protocolIndex;
  proto.options = protocolList();
  props.append(proto);

  IO::DriverProperty slave;
  slave.key   = QStringLiteral("slaveAddress");
  slave.label = tr("Slave Address");
  slave.type  = IO::DriverProperty::IntField;
  slave.value = m_slaveAddress;
  slave.min   = 1;
  slave.max   = 247;
  props.append(slave);

  IO::DriverProperty poll;
  poll.key   = QStringLiteral("pollInterval");
  poll.label = tr("Poll Interval (ms)");
  poll.type  = IO::DriverProperty::IntField;
  poll.value = m_pollInterval;
  poll.min   = 10;
  poll.max   = 60000;
  props.append(poll);

  if (m_protocolIndex == 1) {
    IO::DriverProperty host;
    host.key   = QStringLiteral("host");
    host.label = tr("Host / IP");
    host.type  = IO::DriverProperty::Text;
    host.value = m_host;
    props.append(host);

    IO::DriverProperty port;
    port.key   = QStringLiteral("port");
    port.label = tr("Port");
    port.type  = IO::DriverProperty::IntField;
    port.value = m_port;
    port.min   = 1;
    port.max   = 65535;
    props.append(port);
  } else {
    IO::DriverProperty serial;
    serial.key     = QStringLiteral("serialPortIndex");
    serial.label   = tr("Serial Port");
    serial.type    = IO::DriverProperty::ComboBox;
    serial.value   = m_serialPortIndex;
    serial.options = serialPortList();
    props.append(serial);

    IO::DriverProperty baud;
    baud.key     = QStringLiteral("baudRate");
    baud.label   = tr("Baud Rate");
    baud.type    = IO::DriverProperty::ComboBox;
    baud.value   = baudRateList().indexOf(QString::number(m_baudRate));
    baud.options = baudRateList();
    props.append(baud);

    IO::DriverProperty parity;
    parity.key     = QStringLiteral("parityIndex");
    parity.label   = tr("Parity");
    parity.type    = IO::DriverProperty::ComboBox;
    parity.value   = m_parityIndex;
    parity.options = parityList();
    props.append(parity);

    IO::DriverProperty data;
    data.key     = QStringLiteral("dataBitsIndex");
    data.label   = tr("Data Bits");
    data.type    = IO::DriverProperty::ComboBox;
    data.value   = m_dataBitsIndex;
    data.options = dataBitsList();
    props.append(data);

    IO::DriverProperty stop;
    stop.key     = QStringLiteral("stopBitsIndex");
    stop.label   = tr("Stop Bits");
    stop.type    = IO::DriverProperty::ComboBox;
    stop.value   = m_stopBitsIndex;
    stop.options = stopBitsList();
    props.append(stop);
  }

  return props;
}

/**
 * @brief Applies a single Modbus configuration change by key.
 * @param key   The DriverProperty::key that was edited.
 * @param value The new value chosen by the user.
 */
void IO::Drivers::Modbus::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("protocolIndex"))
    setProtocolIndex(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("slaveAddress"))
    setSlaveAddress(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("pollInterval"))
    setPollInterval(static_cast<quint16>(value.toInt()));

  else if (key == QLatin1String("host"))
    setHost(value.toString());

  else if (key == QLatin1String("port"))
    setPort(static_cast<quint16>(value.toInt()));

  else if (key == QLatin1String("serialPortIndex"))
    setSerialPortIndex(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("parityIndex"))
    setParityIndex(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("dataBitsIndex"))
    setDataBitsIndex(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("stopBitsIndex"))
    setStopBitsIndex(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("baudRate")) {
    const auto list = baudRateList();
    const int idx   = value.toInt();
    if (idx >= 0 && idx < list.size())
      setBaudRate(list.at(idx).toInt());
  }
}
