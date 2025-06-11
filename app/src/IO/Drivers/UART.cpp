/*
 * Serial Studio - https://serial-studio.github.io/
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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "IO/Manager.h"
#include "IO/Drivers/UART.h"

#include "Misc/Utilities.h"
#include "Misc/Translator.h"
#include "Misc/TimerEvents.h"

//------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//------------------------------------------------------------------------------

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
  m_baudRate = m_settings.value("IO_Serial_Baud_Rate", 9600).toInt();

  // Populate error list
  populateErrors();

  // Init serial port configuration variables
  setParity(parityList().indexOf(tr("None")));
  setFlowControl(flowControlList().indexOf(tr("None")));
  setDataBits(dataBitsList().indexOf(QStringLiteral("8")));
  setStopBits(stopBitsList().indexOf(QStringLiteral("1")));

  // Update connect button status when user selects a serial device
  connect(this, &IO::Drivers::UART::portIndexChanged, this,
          &IO::Drivers::UART::configurationChanged);

  // Update error list when language is changed
  connect(this, &IO::Drivers::UART::languageChanged, this,
          &IO::Drivers::UART::populateErrors);
}

/**
 * Destructor function, closes the serial port before exiting the application
 * and saves the user's baud rate list settings.
 */
IO::Drivers::UART::~UART()
{
  if (port())
  {
    if (port()->isOpen())
      port()->close();

    port()->deleteLater();
  }
}

/**
 * Returns the only instance of the class
 */
IO::Drivers::UART &IO::Drivers::UART::instance()
{
  static UART singleton;
  return singleton;
}

//------------------------------------------------------------------------------
// HAL-driver implementation
//------------------------------------------------------------------------------

/**
 * Closes the current serial port connection
 */
void IO::Drivers::UART::close()
{
  // Check if serial port pointer is valid
  if (port() != nullptr)
  {
    // Disconnect signals/slots
    disconnect(port(), &QSerialPort::errorOccurred, this,
               &IO::Drivers::UART::handleError);
    disconnect(port(), &QIODevice::readyRead, this,
               &IO::Drivers::UART::onReadyRead);

    // Send DTR off signal
    if (dtrEnabled())
      port()->setDataTerminalReady(false);

    // Close & delete serial port handler
    port()->close();
    port()->deleteLater();
  }

  // Reset pointer & device status
  m_port = nullptr;
  m_usingCustomSerialPort = false;

  // Update user interface
  Q_EMIT portChanged();
  Q_EMIT availablePortsChanged();
}

/**
 * Returns @c true if a serial port connection is currently open
 */
bool IO::Drivers::UART::isOpen() const
{
  if (port())
    return port()->isOpen();

  return false;
}

/**
 * Returns @c true if the current serial device is readable
 */
bool IO::Drivers::UART::isReadable() const
{
  if (isOpen())
    return port()->isReadable();

  return false;
}

/**
 * Returns @c true if the current serial device is writable
 */
bool IO::Drivers::UART::isWritable() const
{
  if (isOpen())
    return port()->isWritable();

  return false;
}

/**
 * Returns @c true if the user selects the appropiate controls & options to be
 * able to connect to a serial device
 */
bool IO::Drivers::UART::configurationOk() const
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
quint64 IO::Drivers::UART::write(const QByteArray &data)
{
  if (isWritable())
    return port()->write(data);

  return -1;
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
  // Ignore the first item of the list (Select Port)
  auto ports = portList();
  auto portId = portIndex();
  if (portId >= 1 && portId < ports.count())
  {
    // Update port index variable & disconnect from current serial port
    close();
    m_portIndex = portId;
    m_lastSerialDeviceIndex = m_portIndex;
    Q_EMIT portIndexChanged();

    // Get port name from device list
    const auto name = ports.at(portId);

    // Create new serial port handler for native serial ports
    if (m_deviceNames.contains(name))
    {
      m_usingCustomSerialPort = false;
      m_port = new QSerialPort(validPorts().at(portId - 1));
    }

    // Create a new serial port handler for user-specified serial ports
    else if (m_customDevices.contains(name))
    {
      m_usingCustomSerialPort = true;
      m_port = new QSerialPort(name);
    }

    // Configure serial port
    port()->setParity(parity());
    port()->setBaudRate(baudRate());
    port()->setDataBits(dataBits());
    port()->setStopBits(stopBits());
    port()->setFlowControl(flowControl());

    // Connect signals/slots
    connect(port(), &QSerialPort::errorOccurred, this,
            &IO::Drivers::UART::handleError);

    // Open device
    if (port()->open(mode))
    {
      connect(port(), &QIODevice::readyRead, this,
              &IO::Drivers::UART::onReadyRead);
      port()->setDataTerminalReady(dtrEnabled());
      return true;
    }

    // Display error
    else
    {
      Misc::Utilities::showMessageBox(
          tr("Failed to connect to serial port device"), port()->errorString(),
          QMessageBox::Critical);
    }
  }

  // Disconnect serial port
  close();
  return false;
}

//------------------------------------------------------------------------------
// Driver specifics
//------------------------------------------------------------------------------

/**
 * Returns the pointer to the current serial port handler
 */
QSerialPort *IO::Drivers::UART::port() const
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
  QSet<qint32> baudSet
      = {110,   150,   300,    1200,   2400,   4800,   9600,   19200,
         38400, 57600, 115200, 230400, 256000, 460800, 576000, 921600};

  baudSet.insert(m_baudRate);
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
  // Build serial devices list and refresh it every second
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &IO::Drivers::UART::refreshSerialDevices);

  // Update lists when language changes
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &IO::Drivers::UART::languageChanged);
}

/**
 * Changes the baud @a rate of the serial port
 */
void IO::Drivers::UART::setBaudRate(const qint32 rate)
{
  if (m_baudRate != rate && rate > 0)
  {
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
  m_dtrEnabled = enabled;

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
  // Validate that the port index selected by the user is valid
  if (portIndex < portList().count())
    m_portIndex = portIndex;
  else
    m_portIndex = 0;

  // Save settings
  const auto &name = portList().at(m_portIndex);
  if (!name.isEmpty() && m_portIndex > 0)
    m_settings.setValue("IO_Serial_SelectedDevice", name);

  // Update user interface
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
void IO::Drivers::UART::registerDevice(const QString &device)
{
  const auto trimmedPath = device.simplified();

  QFile path(trimmedPath);
  if (path.exists())
  {
    if (!m_customDevices.contains(trimmedPath))
    {
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
  // Argument verification
  Q_ASSERT(parityIndex < parityList().count());

  // Update current index
  m_parityIndex = parityIndex;

  // Set parity based on current index
  switch (parityIndex)
  {
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

  // Update serial port config.
  if (port())
    port()->setParity(parity());

  // Notify user interface
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
  // Argument verification
  Q_ASSERT(dataBitsIndex < dataBitsList().count());

  // Update current index
  m_dataBitsIndex = dataBitsIndex;

  // Obtain data bits value from current index
  switch (dataBitsIndex)
  {
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

  // Update serial port configuration
  if (port())
    port()->setDataBits(dataBits());

  // Update user interface
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
  // Argument verification
  Q_ASSERT(stopBitsIndex < stopBitsList().count());

  // Update current index
  m_stopBitsIndex = stopBitsIndex;

  // Obtain stop bits value from current index
  switch (stopBitsIndex)
  {
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

  // Update serial port configuration
  if (port())
    port()->setStopBits(stopBits());

  // Update user interface
  Q_EMIT stopBitsChanged();
}

/**
 * Enables or disables the auto-reconnect feature
 */
void IO::Drivers::UART::setAutoReconnect(const bool autoreconnect)
{
  m_autoReconnect = autoreconnect;
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
  // Argument verification
  Q_ASSERT(flowControlIndex < flowControlList().count());

  // Update current index
  m_flowControlIndex = flowControlIndex;

  // Obtain flow control value from current index
  switch (flowControlIndex)
  {
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

  // Update serial port configuration
  if (port())
    port()->setFlowControl(flowControl());

  // Update user interface
  Q_EMIT flowControlChanged();
}

/**
 * Scans for new serial ports available & generates a QStringList with current
 * serial ports.
 */
void IO::Drivers::UART::refreshSerialDevices()
{
  // Create device list, starting with dummy header
  // (for a more friendly UI when no devices are attached)
  QStringList names;
  QStringList locations;
  locations.append("/dev/null");
  names.append(tr("Select Port"));

  // Search for available ports and add them to the lsit
  auto validPortList = validPorts();
  Q_FOREACH (QSerialPortInfo info, validPortList)
  {
    if (!info.isNull())
    {
#ifdef Q_OS_WIN
      names.append(info.portName() + "  " + info.description());
#else
      names.append(info.portName());
#endif

      locations.append(info.systemLocation());
    }
  }

  // Update list only if necessary
  if (m_deviceNames != names)
  {
    // Update list
    m_deviceNames = names;
    m_deviceLocations = locations;

    // Update current port index
    bool indexChanged = false;
    if (port())
    {
      auto name = port()->portName();
      for (int i = 0; i < validPortList.count(); ++i)
      {
        auto info = validPortList.at(i);
        if (info.portName() == name)
        {
          indexChanged = true;
          m_portIndex = i + 1;
          break;
        }
      }
    }

    // Auto reconnect
    if (Manager::instance().busType() == SerialStudio::BusType::UART)
    {
      if (autoReconnect() && m_lastSerialDeviceIndex > 0)
      {
        if (m_lastSerialDeviceIndex < portList().count())
        {
          setPortIndex(m_lastSerialDeviceIndex);
          Manager::instance().connectDevice();
        }
      }
    }

    // Update UI
    Q_EMIT availablePortsChanged();

    // Update serial port index after the port list has been updated
    if (indexChanged)
      Q_EMIT portIndexChanged();
  }

  // Select last device
  if (m_portIndex == 0)
  {
    const auto ports = portList();
    auto lastPort = m_settings.value("IO_Serial_SelectedDevice", "").toString();
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
  // Ignore if port is not open
  if (port())
  {
    if (!port()->isOpen())
      return;
  }

  // Log error
  if (error != QSerialPort::NoError)
  {
    // Ingore resource lock errors on virtual serial ports
    if (m_usingCustomSerialPort)
    {
      if (error == QSerialPort::UnsupportedOperationError
          || error == QSerialPort::ResourceError)
        return;
    }

    // Fail silently on resource errors if auto-reconnect is enabled
    if (m_autoReconnect && error == QSerialPort::ResourceError)
    {
      Manager::instance().disconnectDevice();
      return;
    }

    // Display error
    Misc::Utilities::showMessageBox(tr("Critical serial port error"),
                                    m_errorDescriptions[error],
                                    QMessageBox::Critical);

    // Disconnect from device
    Manager::instance().disconnectDevice();
  }
}

/**
 * Reads all the data from the serial port.
 */
void IO::Drivers::UART::onReadyRead()
{
  if (isOpen())
    processData(port()->readAll());
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
  // Search for available ports and add them to the list
  QVector<QSerialPortInfo> ports;
  Q_FOREACH (QSerialPortInfo info, QSerialPortInfo::availablePorts())
  {
    if (!info.isNull())
    {
      // Only accept *.cu devices on macOS (remove *.tty)
      // https://stackoverflow.com/a/37688347
#ifdef Q_OS_MACOS
      if (info.portName().toLower().startsWith("tty."))
        continue;
#endif
      // Append port to list
      ports.append(info);
    }
  }

  // Return list
  return ports;
}
