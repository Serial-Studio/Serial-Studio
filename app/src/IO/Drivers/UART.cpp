/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/UART.h"

#include <QJsonObject>

#include "IO/ConnectionManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Static utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Calculates an ideal read buffer size for a serial port.
 *
 * The buffer is sized to hold approximately 200 milliseconds worth of data
 * based on the given baud rate. This helps balance memory use and latency,
 * avoiding overflows without introducing unnecessary delay.
 *
 * The result is clamped between 256 bytes and 16 kilobytes, and aligned to
 * the nearest 256-byte boundary.
 *
 * @param baud The serial port baud rate in bits per second.
 * @return Ideal buffer size in bytes.
 */
static size_t idealSerialBufferSize(const qint32 baud)
{
  size_t bytes = static_cast<size_t>(baud * 0.02);
  bytes        = std::max<size_t>(256, bytes);
  bytes        = std::min<size_t>(16384, bytes);

  constexpr size_t granularity = 256;
  bytes                        = ((bytes + granularity - 1) / granularity) * granularity;

  return bytes;
}

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * Constructor function
 */
IO::Drivers::UART::UART()
  : m_port(nullptr)
  , m_dtrEnabled(true)
  , m_autoReconnect(false)
  , m_usingCustomSerialPort(false)
  , m_lastSerialDeviceIndex(0)
  , m_portIndex(0)
{
  // Initialize error descriptions
  populateErrors();

  // Restore saved baud rate
  m_baudRate = m_settings.value("IO_Serial_Baud_Rate", 9600).toInt();

  // Determine default parameter indexes
  int defParity   = parityList().indexOf(tr("None"));
  int defFlow     = flowControlList().indexOf(tr("None"));
  int defDataBits = dataBitsList().indexOf(QStringLiteral("8"));
  int defStopBits = stopBitsList().indexOf(QStringLiteral("1"));

  // Restore persisted settings
  setDtrEnabled(m_settings.value("UartDriver/dtr", 1).toBool());
  setParity(m_settings.value("UartDriver/parity", defParity).toInt());
  setDataBits(m_settings.value("UartDriver/dataBits", defDataBits).toInt());
  setStopBits(m_settings.value("UartDriver/stopBits", defStopBits).toInt());
  setAutoReconnect(m_settings.value("UartDriver/autoReconnect", 0).toBool());
  setFlowControl(m_settings.value("UartDriver/flowControl", defFlow).toInt());

  // Propagate configuration changes
  connect(
    this, &IO::Drivers::UART::portIndexChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::baudRateChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(this, &IO::Drivers::UART::parityChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::dataBitsChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::stopBitsChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::flowControlChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::dtrEnabledChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::autoReconnectChanged, this, &IO::Drivers::UART::configurationChanged);

  // Retranslate error descriptions on language change
  connect(this, &IO::Drivers::UART::languageChanged, this, &IO::Drivers::UART::populateErrors);
}

/**
 * Destructor function, closes the serial port before exiting the application
 * and saves the user's baud rate list settings.
 */
IO::Drivers::UART::~UART()
{
  if (port()) {
    if (port()->isOpen())
      port()->close();

    port()->deleteLater();
  }
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * Closes the current serial port connection
 */
void IO::Drivers::UART::close()
{
  // Disconnect and close the serial port
  if (port() != nullptr) {
    disconnect(port(), &QSerialPort::errorOccurred, this, &IO::Drivers::UART::handleError);
    disconnect(port(), &QIODevice::readyRead, this, &IO::Drivers::UART::onReadyRead);

    if (dtrEnabled())
      port()->setDataTerminalReady(false);

    port()->close();
    port()->deleteLater();
  }

  // Reset internal state
  m_port                  = nullptr;
  m_usingCustomSerialPort = false;

  // Notify the UI
  Q_EMIT portChanged();
  Q_EMIT availablePortsChanged();
}

/**
 * Returns @c true if a serial port connection is currently open
 */
bool IO::Drivers::UART::isOpen() const noexcept
{
  if (port())
    return port()->isOpen();

  return false;
}

/**
 * Returns @c true if the current serial device is readable
 */
bool IO::Drivers::UART::isReadable() const noexcept
{
  if (isOpen())
    return port()->isReadable();

  return false;
}

/**
 * Returns @c true if the current serial device is writable
 */
bool IO::Drivers::UART::isWritable() const noexcept
{
  if (isOpen())
    return port()->isWritable();

  return false;
}

/**
 * Returns @c true if the user selects the appropiate controls & options to be
 * able to connect to a serial device
 */
bool IO::Drivers::UART::configurationOk() const noexcept
{
  return portIndex() > 0;
}

/**
 * @brief Writes data to the serial port.
 *
 * Sends the provided data to the serial port if it is writable.
 *
 * @param data The data to be written to the port.
 * @return The number of bytes written on success, or `-1` if the port is not
 *         writable.
 */
qint64 IO::Drivers::UART::write(const QByteArray& data)
{
  if (isWritable())
    return port()->write(data);

  return 0;
}

/**
 * @brief Opens the currently selected serial port with the specified mode.
 *
 * This function initializes and configures a serial port based on the current
 * settings and attempts to open it. If successful, it connects the necessary
 * signals for data handling and error reporting.
 *
 * @param mode The mode in which to open the serial port (e.g., read/write).
 * @return `true` if the port is successfully opened, `false` otherwise.
 */
bool IO::Drivers::UART::open(const QIODevice::OpenMode mode)
{
  // Ensure device list is populated (live drivers don't run the 1 Hz timer)
  if (m_deviceNames.isEmpty())
    refreshSerialDevices();

  // Skip the placeholder entry at index 0
  auto ports  = portList();
  auto portId = portIndex();
  if (portId >= 1 && portId < ports.count()) {
    // Close previous connection and update index
    close();
    m_portIndex             = portId;
    m_lastSerialDeviceIndex = m_portIndex;
    Q_EMIT portIndexChanged();

    const auto name = ports.at(portId);

    // Create serial port handler for native ports
    if (m_deviceNames.contains(name)) {
      m_usingCustomSerialPort = false;
      m_port                  = new QSerialPort(validPorts().at(portId - 1));
    }

    // Create serial port handler for custom device paths
    else if (m_customDevices.contains(name)) {
      m_usingCustomSerialPort = true;
      m_port                  = new QSerialPort(name);
    }

    // Apply serial port parameters
    port()->setParity(parity());
    port()->setBaudRate(baudRate());
    port()->setDataBits(dataBits());
    port()->setStopBits(stopBits());
    port()->setFlowControl(flowControl());
    port()->setReadBufferSize(idealSerialBufferSize(baudRate()));

    connect(port(), &QSerialPort::errorOccurred, this, &IO::Drivers::UART::handleError);

    // Attempt to open the port
    if (port()->open(mode)) {
      connect(port(), &QIODevice::readyRead, this, &IO::Drivers::UART::onReadyRead);
      port()->setDataTerminalReady(dtrEnabled());
      return true;
    }

    else {
      Misc::Utilities::showMessageBox(tr("Failed to connect to serial port \"%1\"").arg(name),
                                      port()->errorString(),
                                      QMessageBox::Critical);
    }
  }

  // Invalid port index, clean up
  close();
  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver specifics
//--------------------------------------------------------------------------------------------------

/**
 * Returns the pointer to the current serial port handler
 */
QSerialPort* IO::Drivers::UART::port() const
{
  return m_port;
}

/**
 * Returns @c true if auto-reconnect is enabled
 */
bool IO::Drivers::UART::autoReconnect() const
{
  return m_autoReconnect;
}

/**
 * Returns @c true if the module shall manually send the DTR
 * (Data Terminal Ready) signal to the connected device upon opening the
 * serial port connection.
 */
bool IO::Drivers::UART::dtrEnabled() const
{
  return m_dtrEnabled;
}

/**
 * Returns the index of the current serial device selected by the program.
 */
quint8 IO::Drivers::UART::portIndex() const
{
  return m_portIndex;
}

/**
 * Returns the correspoding index of the parity configuration in relation
 * to the @c QStringList returned by the @c parityList() function.
 */
quint8 IO::Drivers::UART::parityIndex() const
{
  return m_parityIndex;
}

/**
 * Returns the correspoding index of the data bits configuration in relation
 * to the @c QStringList returned by the @c dataBitsList() function.
 */
quint8 IO::Drivers::UART::dataBitsIndex() const
{
  return m_dataBitsIndex;
}

/**
 * Returns the correspoding index of the stop bits configuration in relation
 * to the @c QStringList returned by the @c stopBitsList() function.
 */
quint8 IO::Drivers::UART::stopBitsIndex() const
{
  return m_stopBitsIndex;
}

/**
 * Returns the correspoding index of the flow control config. in relation
 * to the @c QStringList returned by the @c flowControlList() function.
 */
quint8 IO::Drivers::UART::flowControlIndex() const
{
  return m_flowControlIndex;
}

/**
 * Returns a list with the available serial devices/ports to use.
 * This function can be used with a combo box to build nice UIs.
 *
 * @note The first item of the list will be invalid, since it's value will
 *       be "Select UART Device". This is inteded to make the user interface
 *       a little more friendly.
 */
QStringList IO::Drivers::UART::portList() const
{
  if (m_deviceNames.count() > 0)
    return m_deviceNames + m_customDevices;

  else
    return QStringList{tr("Select Port")};
}

/**
 * Returns a list with the available baud rate configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList IO::Drivers::UART::baudRateList() const
{
  // Build a sorted set of standard baud rates
  QSet<qint32> baudSet = {110,
                          150,
                          300,
                          1200,
                          2400,
                          4800,
                          9600,
                          19200,
                          38400,
                          57600,
                          115200,
                          230400,
                          256000,
                          460800,
                          576000,
                          921600};

  QList<qint32> sortedList = baudSet.values();
  std::sort(sortedList.begin(), sortedList.end());

  QStringList result;
  result.reserve(sortedList.size());
  for (qint32 rate : std::as_const(sortedList))
    result.append(QString::number(rate));

  return result;
}

/**
 * Returns a list with the available parity configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList IO::Drivers::UART::parityList() const
{
  QStringList list;
  list.append(tr("None"));
  list.append(tr("Even"));
  list.append(tr("Odd"));
  list.append(tr("Space"));
  list.append(tr("Mark"));
  return list;
}

/**
 * Returns a list with the available data bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList IO::Drivers::UART::dataBitsList() const
{
  QStringList list;
  list.append(QStringLiteral("5"));
  list.append(QStringLiteral("6"));
  list.append(QStringLiteral("7"));
  list.append(QStringLiteral("8"));
  return list;
}

/**
 * Returns a list with the available stop bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList IO::Drivers::UART::stopBitsList() const
{
  QStringList list;
  list.append(QStringLiteral("1"));
  list.append(QStringLiteral("1.5"));
  list.append(QStringLiteral("2"));
  return list;
}

/**
 * Returns a list with the available flow control configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList IO::Drivers::UART::flowControlList() const
{
  QStringList list;
  list.append(tr("None"));
  list.append(tr("RTS/CTS"));
  list.append(tr("XON/XOFF"));
  return list;
}

/**
 * Returns the current parity configuration used by the serial port
 * handler object.
 */
QSerialPort::Parity IO::Drivers::UART::parity() const
{
  return m_parity;
}

/**
 * Returns the current baud rate configuration used by the serial port
 * handler object.
 */
qint32 IO::Drivers::UART::baudRate() const
{
  return m_baudRate;
}

/**
 * Returns the current data bits configuration used by the serial port
 * handler object.
 */
QSerialPort::DataBits IO::Drivers::UART::dataBits() const
{
  return m_dataBits;
}

/**
 * Returns the current stop bits configuration used by the serial port
 * handler object.
 */
QSerialPort::StopBits IO::Drivers::UART::stopBits() const
{
  return m_stopBits;
}

/**
 * Returns the current flow control configuration used by the serial
 * port handler object.
 */
QSerialPort::FlowControl IO::Drivers::UART::flowControl() const
{
  return m_flowControl;
}

/**
 * Configures the signal/slot connections with the rest of the modules of the
 * application.
 */
void IO::Drivers::UART::setupExternalConnections()
{
  // Refresh serial device list every second
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &IO::Drivers::UART::refreshSerialDevices);

  // Retranslate on language change
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::Drivers::UART::languageChanged);
}

/**
 * Changes the baud @a rate of the serial port
 */
void IO::Drivers::UART::setBaudRate(const qint32 rate)
{
  if (m_baudRate != rate && rate > 0) {
    m_baudRate = rate;
    m_settings.setValue("IO_Serial_Baud_Rate", rate);

    if (port())
      port()->setBaudRate(baudRate());

    Q_EMIT baudRateChanged();
  }
}

/**
 * Sets the Data Terminal Ready (DTR) signal state.
 *
 * This function is called when the DTR checkbox state is changed. It updates
 * the internal state to reflect whether DTR is enabled or disabled and
 * communicates this change to the serial port if it is open.
 *
 * If the serial port is currently open, the DTR signal is set accordingly.
 *
 * This change is also emitted as a signal to notify any connected slots of the
 * change.
 */
void IO::Drivers::UART::setDtrEnabled(const bool enabled)
{
  if (m_dtrEnabled == enabled)
    return;

  m_dtrEnabled = enabled;
  m_settings.setValue("UartDriver/dtr", enabled);

  if (port() && port()->isOpen())
    port()->setDataTerminalReady(enabled);

  Q_EMIT dtrEnabledChanged();
}

/**
 * Changes the port index value, this value is later used by the @c
 * openSerialPort() function.
 */
void IO::Drivers::UART::setPortIndex(const quint8 portIndex)
{
  // Populate device list and clamp to valid range
  if (m_deviceNames.isEmpty())
    refreshSerialDevices();

  // Clamp to valid range
  if (portIndex < portList().count())
    m_portIndex = portIndex;
  else
    m_portIndex = 0;

  // Persist selection
  const auto name = portList().at(m_portIndex);
  if (!name.isEmpty() && m_portIndex > 0)
    m_settings.setValue("IO_Serial_SelectedDevice", name);

  Q_EMIT portIndexChanged();
}

/**
 * @brief Registers a custom serial device.
 *
 * This function allows the registration of a custom serial device by verifying
 * the validity of the provided path and ensuring it is not already registered.
 * If the path is valid and not yet registered, it is added to the list of
 * custom devices.
 *
 * @param device The file path of the device to register.
 */
void IO::Drivers::UART::registerDevice(const QString& device)
{
  // Validate and register the custom device path
  const auto trimmedPath = device.simplified();

  QFile path(trimmedPath);
  if (path.exists()) {
    if (!m_customDevices.contains(trimmedPath)) {
      m_customDevices.append(trimmedPath);
      Q_EMIT availablePortsChanged();
    }
  }

  else
    Misc::Utilities::showMessageBox(
      tr("\"%1\" is not a valid path").arg(trimmedPath),
      tr("Please type another path to register a custom serial device"),
      QMessageBox::Warning);
}

/**
 * @brief IO::Drivers::UART::setParity
 * @param parityIndex
 */
void IO::Drivers::UART::setParity(const quint8 parityIndex)
{
  // Persist the index and update internal parity enum
  Q_ASSERT(parityIndex < parityList().count());

  m_parityIndex = parityIndex;
  m_settings.setValue("UartDriver/parity", parityIndex);

  // Map index to QSerialPort parity enum
  switch (parityIndex) {
    case 0:
      m_parity = QSerialPort::NoParity;
      break;
    case 1:
      m_parity = QSerialPort::EvenParity;
      break;
    case 2:
      m_parity = QSerialPort::OddParity;
      break;
    case 3:
      m_parity = QSerialPort::SpaceParity;
      break;
    case 4:
      m_parity = QSerialPort::MarkParity;
      break;
  }

  // Apply to active port if connected
  if (port())
    port()->setParity(parity());

  Q_EMIT parityChanged();
}

/**
 * Changes the data bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void IO::Drivers::UART::setDataBits(const quint8 dataBitsIndex)
{
  // Persist the index and update internal data bits enum
  Q_ASSERT(dataBitsIndex < dataBitsList().count());

  m_dataBitsIndex = dataBitsIndex;
  m_settings.setValue("UartDriver/dataBits", dataBitsIndex);

  // Map index to QSerialPort data bits enum
  switch (dataBitsIndex) {
    case 0:
      m_dataBits = QSerialPort::Data5;
      break;
    case 1:
      m_dataBits = QSerialPort::Data6;
      break;
    case 2:
      m_dataBits = QSerialPort::Data7;
      break;
    case 3:
      m_dataBits = QSerialPort::Data8;
      break;
  }

  // Apply to active port if connected
  if (port())
    port()->setDataBits(dataBits());

  Q_EMIT dataBitsChanged();
}

/**
 * Changes the stop bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void IO::Drivers::UART::setStopBits(const quint8 stopBitsIndex)
{
  // Persist the index and update internal stop bits enum
  Q_ASSERT(stopBitsIndex < stopBitsList().count());

  m_stopBitsIndex = stopBitsIndex;
  m_settings.setValue("UartDriver/stopBits", stopBitsIndex);

  // Map index to QSerialPort stop bits enum
  switch (stopBitsIndex) {
    case 0:
      m_stopBits = QSerialPort::OneStop;
      break;
    case 1:
      m_stopBits = QSerialPort::OneAndHalfStop;
      break;
    case 2:
      m_stopBits = QSerialPort::TwoStop;
      break;
  }

  // Apply to active port if connected
  if (port())
    port()->setStopBits(stopBits());

  Q_EMIT stopBitsChanged();
}

/**
 * Enables or disables the auto-reconnect feature
 */
void IO::Drivers::UART::setAutoReconnect(const bool autoreconnect)
{
  if (m_autoReconnect == autoreconnect)
    return;

  m_autoReconnect = autoreconnect;
  m_settings.setValue("UartDriver/autoReconnect", autoreconnect);
  Q_EMIT autoReconnectChanged();
}

/**
 * Changes the flow control option of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void IO::Drivers::UART::setFlowControl(const quint8 flowControlIndex)
{
  // Persist the index and update internal flow control enum
  Q_ASSERT(flowControlIndex < flowControlList().count());

  m_flowControlIndex = flowControlIndex;
  m_settings.setValue("UartDriver/flowControl", flowControlIndex);

  // Map index to QSerialPort flow control enum
  switch (flowControlIndex) {
    case 0:
      m_flowControl = QSerialPort::NoFlowControl;
      break;
    case 1:
      m_flowControl = QSerialPort::HardwareControl;
      break;
    case 2:
      m_flowControl = QSerialPort::SoftwareControl;
      break;
  }

  // Apply to active port if connected
  if (port())
    port()->setFlowControl(flowControl());

  Q_EMIT flowControlChanged();
}

/**
 * Scans for new serial ports available & generates a QStringList with current
 * serial ports.
 */
void IO::Drivers::UART::refreshSerialDevices()
{
  // Build device list with placeholder header
  QStringList names;
  QStringList locations;
  locations.append("/dev/null");
  names.append(tr("Select Port"));

  // Enumerate available ports
  auto validPortList = validPorts();
  for (const auto& info : std::as_const(validPortList)) {
    if (!info.isNull()) {
#ifdef Q_OS_WIN
      names.append(info.portName() + "  " + info.description());
#else
      names.append(info.portName());
#endif

      locations.append(info.systemLocation());
    }
  }

  // Only update if the list has changed
  if (m_deviceNames != names) {
    m_deviceNames     = names;
    m_deviceLocations = locations;

    // Re-find the currently open port in the new list
    bool indexChanged = false;
    if (port()) {
      auto name = port()->portName();
      for (int i = 0; i < validPortList.count(); ++i) {
        auto info = validPortList.at(i);
        if (info.portName() == name) {
          indexChanged = true;
          m_portIndex  = i + 1;
          break;
        }
      }
    }

    // Attempt auto-reconnect if enabled
    if (ConnectionManager::instance().busType() == SerialStudio::BusType::UART) {
      if (autoReconnect() && m_lastSerialDeviceIndex > 0
          && m_lastSerialDeviceIndex < portList().count()) {
        setPortIndex(m_lastSerialDeviceIndex);
        ConnectionManager::instance().connectDevice();
      }
    }

    Q_EMIT availablePortsChanged();

    // Notify index change after list update
    if (indexChanged)
      Q_EMIT portIndexChanged();
  }

  // Restore last selected device from settings
  if (m_portIndex == 0) {
    const auto ports = portList();
    auto lastPort    = m_settings.value("IO_Serial_SelectedDevice", "").toString();
    if (!lastPort.isEmpty() && ports.contains(lastPort))
      setPortIndex(ports.indexOf(lastPort));
  }
}

/**
 * @brief IO::Drivers::UART::handleError
 * @param error
 */
void IO::Drivers::UART::handleError(QSerialPort::SerialPortError error)
{
  // Serialize concurrent error callbacks
  QMutexLocker locker(&m_errorHandlerMutex);

  // Ignore if port is not open
  auto serialPort = port();
  if (serialPort && !serialPort->isOpen())
    return;

  // Already disconnected from a prior error
  if (!ConnectionManager::instance().isConnected())
    return;

  if (error != QSerialPort::NoError) {
    // Ignore resource lock errors on virtual serial ports
    if (m_usingCustomSerialPort) {
      if (error == QSerialPort::UnsupportedOperationError || error == QSerialPort::ResourceError)
        return;
    }

    ConnectionManager::instance().disconnectDevice();

    if (!m_autoReconnect || error != QSerialPort::ResourceError) {
      const auto name = serialPort ? serialPort->portName() : tr("Unknown");
      Misc::Utilities::showMessageBox(tr("Critical error on serial port \"%1\"").arg(name),
                                      m_errorDescriptions.value(error, tr("Unknown error")),
                                      QMessageBox::Critical);
    }
  }
}

/**
 * Reads all the data from the serial port.
 */
void IO::Drivers::UART::onReadyRead()
{
  if (isOpen())
    Q_EMIT dataReceived(makeByteArray(port()->readAll()));
}

/**
 * @brief Populates the error descriptions for the serial port driver.
 *
 * This function initializes a mapping of QSerialPort error codes to their
 * corresponding human-readable descriptions. These descriptions provide
 * detailed context and, where applicable, suggestions for resolving the error.
 */
void IO::Drivers::UART::populateErrors()
{
  // clang-format off
  m_errorDescriptions.clear();
  m_errorDescriptions.insert(QSerialPort::NoError, tr("No error occurred."));
  m_errorDescriptions.insert(QSerialPort::DeviceNotFoundError, tr("The specified device could not be found. Please check the connection and try again."));
  m_errorDescriptions.insert(QSerialPort::PermissionError, tr("Permission denied. Ensure the application has the necessary access rights to the device."));
  m_errorDescriptions.insert(QSerialPort::OpenError, tr("Failed to open the device. It may already be in use or unavailable."));
  m_errorDescriptions.insert(QSerialPort::WriteError, tr("An error occurred while writing data to the device."));
  m_errorDescriptions.insert(QSerialPort::ReadError, tr("An error occurred while reading data from the device."));
  m_errorDescriptions.insert(QSerialPort::ResourceError, tr("A critical resource error occurred. The device may have been disconnected or is no longer accessible."));
  m_errorDescriptions.insert(QSerialPort::UnsupportedOperationError, tr("The requested operation is not supported on this device."));
  m_errorDescriptions.insert(QSerialPort::UnknownError, tr("An unknown error occurred. Please check the device and try again."));
  m_errorDescriptions.insert(QSerialPort::TimeoutError, tr("The operation timed out. The device may not be responding."));
  m_errorDescriptions.insert(QSerialPort::NotOpenError, tr("The device is not open. Please open the device before attempting this operation."));
  // clang-format on
}

/**
 * Returns a list with all the valid serial port objects
 */
QVector<QSerialPortInfo> IO::Drivers::UART::validPorts() const
{
  QVector<QSerialPortInfo> ports;
  for (const auto& info : QSerialPortInfo::availablePorts()) {
    if (!info.isNull()) {
      // Filter out tty.* devices on macOS, only use cu.*
#ifdef Q_OS_MACOS
      if (info.portName().toLower().startsWith("tty."))
        continue;
#endif
      ports.append(info);
    }
  }

  return ports;
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns cross-platform hardware identifiers for the currently selected serial port.
 *
 * Uses QSerialPortInfo to extract VID, PID, serial number, port name, and description.
 * These identifiers allow the same project file to find the correct serial port on any OS.
 */
QJsonObject IO::Drivers::UART::deviceIdentifier() const
{
  // Validate port index and retrieve port info
  if (m_portIndex < 1)
    return {};

  const auto ports = validPorts();
  const int idx    = m_portIndex - 1;
  if (idx < 0 || idx >= ports.count())
    return {};

  const auto& info = ports.at(idx);
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
 * Scores each available port by VID+PID+serial (best), VID+PID only, description match,
 * then port name match (weakest). Selects the highest-scoring port. If no match is found,
 * the current selection is left unchanged.
 */
bool IO::Drivers::UART::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  // Ensure device list is populated so setPortIndex() can clamp correctly
  if (m_deviceNames.isEmpty())
    refreshSerialDevices();

  const auto ports     = validPorts();
  const auto savedVid  = id.value(QStringLiteral("vid")).toString();
  const auto savedPid  = id.value(QStringLiteral("pid")).toString();
  const auto savedSer  = id.value(QStringLiteral("serial")).toString();
  const auto savedName = id.value(QStringLiteral("portName")).toString();
  const auto savedDesc = id.value(QStringLiteral("description")).toString();

  int bestScore = 0;
  int bestIndex = -1;

  for (int i = 0; i < ports.count(); ++i) {
    const auto& info = ports.at(i);
    int score        = 0;

    // VID + PID match
    if (!savedVid.isEmpty() && info.hasVendorIdentifier()) {
      const auto vid =
        QString::number(info.vendorIdentifier(), 16).rightJustified(4, '0').toUpper();
      const auto pid =
        QString::number(info.productIdentifier(), 16).rightJustified(4, '0').toUpper();
      if (vid == savedVid && pid == savedPid) {
        score += 100;

        // Serial number match on top of VID+PID
        if (!savedSer.isEmpty() && info.serialNumber() == savedSer)
          score += 50;
      }
    }

    // Description match (cross-platform fallback for non-USB serial ports)
    if (!savedDesc.isEmpty() && info.description() == savedDesc)
      score += 10;

    // Port name match (weakest — differs per OS)
    if (!savedName.isEmpty() && info.portName() == savedName)
      score += 5;

    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  if (bestIndex >= 0) {
    setPortIndex(static_cast<quint8>(bestIndex + 1));
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the UART configuration as a flat list of editable properties.
 * @return List of DriverProperty descriptors with current values.
 */
QList<IO::DriverProperty> IO::Drivers::UART::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty port;
  port.key     = QStringLiteral("portIndex");
  port.label   = tr("Serial Port");
  port.type    = IO::DriverProperty::ComboBox;
  port.value   = m_portIndex;
  port.options = portList();
  props.append(port);

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

  IO::DriverProperty flow;
  flow.key     = QStringLiteral("flowControlIndex");
  flow.label   = tr("Flow Control");
  flow.type    = IO::DriverProperty::ComboBox;
  flow.value   = m_flowControlIndex;
  flow.options = flowControlList();
  props.append(flow);

  IO::DriverProperty dtr;
  dtr.key   = QStringLiteral("dtr");
  dtr.label = tr("DTR");
  dtr.type  = IO::DriverProperty::CheckBox;
  dtr.value = m_dtrEnabled;
  props.append(dtr);

  IO::DriverProperty reconnect;
  reconnect.key   = QStringLiteral("autoReconnect");
  reconnect.label = tr("Auto-Reconnect");
  reconnect.type  = IO::DriverProperty::CheckBox;
  reconnect.value = m_autoReconnect;
  props.append(reconnect);

  return props;
}

/**
 * @brief Applies a single UART configuration change by key.
 * @param key   The DriverProperty::key that was edited.
 * @param value The new value chosen by the user.
 */
void IO::Drivers::UART::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("portIndex"))
    setPortIndex(static_cast<quint8>(value.toInt()));

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

  else if (key == QLatin1String("parityIndex"))
    setParity(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("dataBitsIndex"))
    setDataBits(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("stopBitsIndex"))
    setStopBits(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("flowControlIndex"))
    setFlowControl(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("dtr"))
    setDtrEnabled(value.toBool());

  else if (key == QLatin1String("autoReconnect"))
    setAutoReconnect(value.toBool());
}
