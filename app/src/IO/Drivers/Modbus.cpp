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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QModbusDataUnit>
#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStandardPaths>
#include <QTimer>

#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

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

IO::Drivers::Modbus::~Modbus()
{
  doClose();
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the current Modbus connection.
 */
void IO::Drivers::Modbus::close()
{
  doClose();

  Q_EMIT configurationChanged();
  Q_EMIT availableSerialPortsChanged();
}

/**
 * @brief Non-virtual cleanup implementation shared by close() and ~Modbus().
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

bool IO::Drivers::Modbus::isOpen() const noexcept
{
  if (m_device)
    return m_device->state() == QModbusDevice::ConnectedState;

  return false;
}

bool IO::Drivers::Modbus::isReadable() const noexcept
{
  return isOpen();
}

bool IO::Drivers::Modbus::isWritable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true if the user has selected appropriate controls & options to connect to a
 * Modbus device.
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
 * @brief Writes a single- or double-register holding write to the Modbus device.
 */
qint64 IO::Drivers::Modbus::write(const QByteArray& data)
{
  if (!isWritable() || data.length() < 4)
    return 0;

  // Parse register address and determine count from payload length
  quint16 address    = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
  int register_count = (data.length() >= 6) ? 2 : 1;

  // Build the write unit with one or two 16-bit values
  QModbusDataUnit write_unit(QModbusDataUnit::HoldingRegisters, address, register_count);
  write_unit.setValue(0, (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]));

  if (register_count == 2)
    write_unit.setValue(1, (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]));

  // Send the write request and track the reply
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
 * @brief Opens the Modbus device (RTU or TCP) with the given mode.
 */
bool IO::Drivers::Modbus::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  // Close any existing connection
  close();

  if (!configurationOk())
    return false;

  QString connectionTarget;

  // Configure RTU serial client
  if (m_protocolIndex == 0) {
    if (m_serialPortNames.isEmpty())
      refreshSerialPorts();

    auto ports  = serialPortList();
    auto portId = serialPortIndex();
    if (portId < 1 || portId >= ports.count())
      return false;

    // Validate the serial port still exists
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

    // Map parity index to Qt enum
    QSerialPort::Parity parity = QSerialPort::NoParity;
    if (m_parityIndex == 1)
      parity = QSerialPort::EvenParity;
    else if (m_parityIndex == 2)
      parity = QSerialPort::OddParity;
    else if (m_parityIndex == 3)
      parity = QSerialPort::SpaceParity;
    else if (m_parityIndex == 4)
      parity = QSerialPort::MarkParity;

    // Map data bits index to Qt enum
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    if (m_dataBitsIndex == 0)
      dataBits = QSerialPort::Data5;
    else if (m_dataBitsIndex == 1)
      dataBits = QSerialPort::Data6;
    else if (m_dataBitsIndex == 2)
      dataBits = QSerialPort::Data7;
    else if (m_dataBitsIndex == 3)
      dataBits = QSerialPort::Data8;

    // Map stop bits index to Qt enum
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    if (m_stopBitsIndex == 1)
      stopBits = QSerialPort::OneAndHalfStop;
    else if (m_stopBitsIndex == 2)
      stopBits = QSerialPort::TwoStop;

    // Apply serial parameters to the RTU client
    m_device->setConnectionParameter(QModbusDevice::SerialPortNameParameter, portName);
    m_device->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baudRate);
    m_device->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
    m_device->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
    m_device->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);

    connectionTarget = ports.value(portId);
  }

  // Configure TCP client
  else if (m_protocolIndex == 1) {
    auto* tcp_device = new QModbusTcpClient(this);
    tcp_device->setConnectionParameter(QModbusDevice::NetworkAddressParameter, m_host);
    tcp_device->setConnectionParameter(QModbusDevice::NetworkPortParameter, m_port);
    m_device = tcp_device;

    connectionTarget = QStringLiteral("%1:%2").arg(m_host).arg(m_port);
  }

  // Abort if device creation failed
  if (!m_device) {
    Misc::Utilities::showMessageBox(
      tr("Modbus Initialization Failed"),
      tr("Unable to create Modbus device. Please check your system configuration and try again."),
      QMessageBox::Critical);
    return false;
  }

  // Set timeout and retry parameters
  m_device->setTimeout(1000);
  m_device->setNumberOfRetries(3);

  // Connect state change and error signals
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

  // Attempt connection
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

  // Start polling if already connected (synchronous RTU case)
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
  if (index < 2 && m_protocolIndex != index) {
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
  // Validate count range
  if (count > 0 && count <= 125) {
    // Skip duplicate entries
    for (const auto& group : std::as_const(m_registerGroups))
      if (group.registerType == type && group.startAddress == start && group.count == count)
        return;

    // Append and persist to settings
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

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates a Serial Studio project from the configured register groups.
 */
void IO::Drivers::Modbus::generateProject()
{
  // Abort if no register groups are configured
  if (m_registerGroups.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("No register groups configured"),
      tr("Add at least one register group before generating a project."),
      QMessageBox::Warning,
      tr("Modbus Project Generator"));
    return;
  }

  // Build the project JSON
  const auto project   = buildProject();
  const auto temp_dir  = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  const auto temp_path = temp_dir + "/modbus_project_temp.ssproj";

  // Write to temporary file
  QFile file(temp_path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(tr("Failed to create temporary project file"),
                                    tr("Check write permissions to the temporary directory."),
                                    QMessageBox::Critical,
                                    tr("Modbus Project Generator"));
    return;
  }

  QJsonDocument doc(project);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  // Load and prompt save
  DataModel::ProjectModel::instance().openJsonFile(temp_path);
  if (DataModel::ProjectModel::instance().saveJsonFile(true)) {
    QFile::remove(temp_path);

    // Count total datasets for the success message
    int total_datasets = 0;
    for (const auto& g : m_registerGroups)
      total_datasets += g.count;

    Misc::Utilities::showMessageBox(
      tr("Successfully generated project with %1 groups and %2 datasets.")
        .arg(m_registerGroups.count())
        .arg(total_datasets),
      tr("The project editor is now open for customization."),
      QMessageBox::Information,
      tr("Modbus Project Generator"));
  } else {
    QFile::remove(temp_path);
  }
}

/**
 * @brief Assembles the complete project JSON object.
 */
QJsonObject IO::Drivers::Modbus::buildProject() const
{
  // Project root with title and empty actions
  QJsonObject project;
  project[QStringLiteral("title")]   = tr("Modbus Project");
  project[QStringLiteral("actions")] = QJsonArray();

  // Build the single Modbus source
  QJsonObject source;
  source[QStringLiteral("sourceId")]              = 0;
  source[QStringLiteral("title")]                 = tr("Modbus");
  source[QStringLiteral("busType")]               = static_cast<int>(SerialStudio::BusType::ModBus);
  source[QStringLiteral("frameStart")]            = QString();
  source[QStringLiteral("frameEnd")]              = QString();
  source[QStringLiteral("checksum")]              = QString();
  source[QStringLiteral("frameDetection")]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[QStringLiteral("decoder")]               = static_cast<int>(SerialStudio::Binary);
  source[QStringLiteral("hexadecimalDelimiters")] = false;
  source[QStringLiteral("frameParserCode")]       = buildFrameParser();
  source[QStringLiteral("frameParserLanguage")]   = static_cast<int>(SerialStudio::Lua);

  // Embed current driver settings in the source
  QJsonObject conn_settings;
  for (const auto& prop : driverProperties())
    conn_settings.insert(prop.key, QJsonValue::fromVariant(prop.value));
  source[QStringLiteral("connection")] = conn_settings;

  project[QStringLiteral("sources")] = QJsonArray{source};

  // Create one group per register configuration
  static const QStringList type_names = {
    tr("Holding Registers"),
    tr("Input Registers"),
    tr("Coils"),
    tr("Discrete Inputs"),
  };

  QJsonArray group_array;
  int group_id      = 0;
  int dataset_index = 1;

  for (const auto& reg_group : m_registerGroups) {
    DataModel::Group group;
    group.groupId = group_id;
    group.widget  = QStringLiteral("datagrid");

    // Derive group title from register type and address
    const QString type_name = (reg_group.registerType < type_names.count())
                              ? type_names[reg_group.registerType]
                              : tr("Unknown");
    group.title             = QStringLiteral("%1 @ %2").arg(type_name).arg(reg_group.startAddress);

    // Register types (16-bit) vs bit types (coil/discrete)
    const bool is_reg = (reg_group.registerType <= 1);

    // Create one dataset per register or coil
    for (quint16 i = 0; i < reg_group.count; ++i) {
      DataModel::Dataset dataset;
      dataset.index = dataset_index++;
      dataset.log   = true;

      const quint16 addr = reg_group.startAddress + i;

      if (is_reg) {
        dataset.title  = tr("Register %1").arg(addr);
        dataset.plt    = true;
        dataset.wgtMin = 0;
        dataset.wgtMax = 65535;
        dataset.pltMin = 0;
        dataset.pltMax = 65535;
      } else {
        dataset.title =
          (reg_group.registerType == 2) ? tr("Coil %1").arg(addr) : tr("Discrete %1").arg(addr);
        dataset.led     = true;
        dataset.ledHigh = 1;
        dataset.wgtMin  = 0;
        dataset.wgtMax  = 1;
      }

      group.datasets.push_back(dataset);
    }

    group_array.append(DataModel::serialize(group));
    ++group_id;
  }

  project[QStringLiteral("groups")] = group_array;
  return project;
}

/**
 * @brief Generates a JavaScript frame parser for the configured register groups.
 */
QString IO::Drivers::Modbus::buildFrameParser() const
{
  static const QStringList type_names = {
    QStringLiteral("Holding Registers"),
    QStringLiteral("Input Registers"),
    QStringLiteral("Coils"),
    QStringLiteral("Discrete Inputs"),
  };

  // Compute totals for the header comment
  int total_datasets = 0;
  for (const auto& g : m_registerGroups)
    total_datasets += g.count;

  const int group_count = m_registerGroups.count();

  QString code;

  // Emit the Lua header comment
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Modbus Register Frame Parser\n");
  code += QStringLiteral("-- Auto-generated by Serial Studio\n");
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Total groups: %1\n").arg(group_count);
  code += QStringLiteral("-- Total datasets: %1\n").arg(total_datasets);
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Frame format: {slaveAddr, funcCode, byteCount, ...data}\n");
  code += QStringLiteral("-- Groups are polled sequentially; this parser tracks the cycle.\n");
  code += QStringLiteral("--\n\n");

  // Emit global state initialization
  code += QStringLiteral("local values = {}\n");
  code += QStringLiteral("for i = 1, %1 do values[i] = 0 end\n").arg(total_datasets);
  code += QStringLiteral("local currentGroup = 0\n\n");

  // Emit parse() function with per-group dispatch
  code += QStringLiteral("function parse(frame)\n");
  code += QStringLiteral("  if #frame < 3 then return values end\n\n");
  code += QStringLiteral("  -- Extract data payload (skip slave addr, func code, byte count)\n");
  code += QStringLiteral("  local data = {}\n");
  code += QStringLiteral("  for i = 4, #frame do data[#data + 1] = frame[i] end\n\n");

  // Emit if-elseif chain dispatching by group index
  int dataset_offset = 0;
  for (int g = 0; g < group_count; ++g) {
    const auto& reg_group = m_registerGroups[g];
    const bool is_reg     = (reg_group.registerType <= 1);

    const QString type_name = (reg_group.registerType < type_names.count())
                              ? type_names[reg_group.registerType]
                              : QStringLiteral("Unknown");

    if (g == 0)
      code += QStringLiteral("  if currentGroup == %1 then -- %2 @ %3, count=%4\n")
                .arg(g)
                .arg(type_name)
                .arg(reg_group.startAddress)
                .arg(reg_group.count);
    else
      code += QStringLiteral("  elseif currentGroup == %1 then -- %2 @ %3, count=%4\n")
                .arg(g)
                .arg(type_name)
                .arg(reg_group.startAddress)
                .arg(reg_group.count);

    if (is_reg) {
      for (quint16 i = 0; i < reg_group.count; ++i) {
        const int byte_off = i * 2 + 1;
        code += QStringLiteral("    values[%1] = (data[%2] << 8) | data[%3]\n")
                  .arg(dataset_offset + i + 1)
                  .arg(byte_off)
                  .arg(byte_off + 1);
      }
    } else {
      for (quint16 i = 0; i < reg_group.count; ++i) {
        const int byte_idx = i / 8 + 1;
        const int bit_idx  = i % 8;
        code += QStringLiteral("    values[%1] = (data[%2] >> %3) & 1\n")
                  .arg(dataset_offset + i + 1)
                  .arg(byte_idx)
                  .arg(bit_idx);
      }
    }

    dataset_offset += reg_group.count;
  }

  if (group_count > 0)
    code += QStringLiteral("  end\n\n");

  code += QStringLiteral("  currentGroup = (currentGroup + 1) %% %1\n").arg(group_count);
  code += QStringLiteral("  return values\n");
  code += QStringLiteral("end\n");

  return code;
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

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
  // Ensure port list is populated so the index range is valid
  if (m_serialPortNames.isEmpty())
    refreshSerialPorts();

  if (index < m_serialPortNames.count() && m_serialPortIndex != index) {
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
  if (index < 5 && m_parityIndex != index) {
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
  if (index < 4 && m_dataBitsIndex != index) {
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
  if (index < 3 && m_stopBitsIndex != index) {
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
 * @brief Polls Modbus registers at the configured interval.
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
 * @brief Polls the next register group in multi-group mode.
 */
void IO::Drivers::Modbus::pollNextGroup()
{
  // Guard: device must be open with no pending reply
  if (!m_device || !isOpen())
    return;

  if (m_lastReply && !m_lastReply->isFinished())
    return;

  if (m_currentGroupIndex >= m_registerGroups.count())
    return;

  // Map group register type to Qt enum
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

  // Send the read request for this group
  QModbusDataUnit read_unit(registerType, group.startAddress, group.count);

  auto* reply = m_device->sendReadRequest(read_unit, m_slaveAddress);
  if (!reply)
    return;

  m_lastReply = reply;

  connect(
    reply, &QModbusReply::finished, this, &IO::Drivers::Modbus::onReadReady, Qt::UniqueConnection);
}

/**
 * @brief Handles completed Modbus read operations and publishes the result as RTU-format bytes.
 */
void IO::Drivers::Modbus::onReadReady()
{
  // Validate reply sender and ownership
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

  // Abort on Modbus protocol errors
  if (reply->error() != QModbusDevice::NoError) {
    reply->deleteLater();
    return;
  }

  // Validate the data unit
  const QModbusDataUnit unit = reply->result();
  if (!unit.isValid() || unit.valueCount() == 0) {
    reply->deleteLater();
    return;
  }

  try {
    // Determine function code and register type
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

    // Build the output byte array in Modbus RTU format
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

    // Bit-packed format for coils/discrete inputs
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

    publishReceivedData(std::move(data));
  }

  catch (...) {
  }

  reply->deleteLater();

  // Continue polling the next group in the round-robin
  ++m_currentGroupIndex;
  if (m_currentGroupIndex < m_registerGroups.count())
    pollNextGroup();
}

/**
 * @brief Handles Modbus device state changes by starting/stopping the poll timer.
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
 * @brief Handles Modbus device errors by showing a message box.
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
 * @brief Refreshes the list of available serial ports.
 */
void IO::Drivers::Modbus::refreshSerialPorts()
{
  // Build port lists with placeholder at index 0
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

  // Update only if the list changed
  if (m_serialPortNames != names) {
    m_serialPortNames     = names;
    m_serialPortLocations = locations;
    Q_EMIT availableSerialPortsChanged();

    // Clamp index if the selected port was removed
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
 */
QJsonObject IO::Drivers::Modbus::deviceIdentifier() const
{
  // Only meaningful for RTU mode
  if (m_protocolIndex != 0 || m_serialPortIndex < 1)
    return {};

  // Build a filtered port list matching our display order
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

  // Extract hardware identifiers from the port info
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
 */
bool IO::Drivers::Modbus::selectByIdentifier(const QJsonObject& id)
{
  // Only meaningful for RTU mode
  if (id.isEmpty() || m_protocolIndex != 0)
    return false;

  // Ensure port list is populated for index clamping
  if (m_serialPortNames.isEmpty())
    refreshSerialPorts();

  // Build a filtered port list matching our display order
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

  // Extract saved identifiers
  const auto savedVid  = id.value(QStringLiteral("vid")).toString();
  const auto savedPid  = id.value(QStringLiteral("pid")).toString();
  const auto savedSer  = id.value(QStringLiteral("serial")).toString();
  const auto savedName = id.value(QStringLiteral("portName")).toString();
  const auto savedDesc = id.value(QStringLiteral("description")).toString();

  // Score each port: VID+PID 100, serial 50, description 10, name 5
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
    baud.key   = QStringLiteral("baudRate");
    baud.label = tr("Baud Rate");
    baud.type  = IO::DriverProperty::IntField;
    baud.value = m_baudRate;
    baud.min   = 1;
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

  // Serialize register groups for project persistence
  if (!m_registerGroups.isEmpty()) {
    QJsonArray groups_array;
    for (const auto& g : m_registerGroups) {
      QJsonObject obj;
      obj[QStringLiteral("type")]  = g.registerType;
      obj[QStringLiteral("start")] = g.startAddress;
      obj[QStringLiteral("count")] = g.count;
      groups_array.append(obj);
    }

    IO::DriverProperty groups;
    groups.key   = QStringLiteral("registerGroups");
    groups.type  = IO::DriverProperty::Text;
    groups.value = QVariant::fromValue(groups_array);
    props.append(groups);
  }

  return props;
}

/**
 * @brief Applies a single Modbus configuration change by key.
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
    const int v = value.toInt();
    if (v >= 110)
      setBaudRate(v);
    else {
      const auto list = baudRateList();
      if (v >= 0 && v < list.size())
        setBaudRate(list.at(v).toInt());
    }
  }

  else if (key == QLatin1String("registerGroups")) {
    QJsonArray array;
    if (value.canConvert<QJsonArray>())
      array = value.toJsonArray();
    else if (value.typeId() == QMetaType::QVariantList) {
      const auto list = value.toList();
      for (const auto& item : list)
        array.append(QJsonValue::fromVariant(item));
    }

    if (!array.isEmpty()) {
      clearRegisterGroups();
      for (const auto& item : array) {
        const auto obj = item.toObject();
        addRegisterGroup(static_cast<quint8>(obj.value(QStringLiteral("type")).toInt()),
                         static_cast<quint16>(obj.value(QStringLiteral("start")).toInt()),
                         static_cast<quint16>(obj.value(QStringLiteral("count")).toInt()));
      }
    }
  }
}
