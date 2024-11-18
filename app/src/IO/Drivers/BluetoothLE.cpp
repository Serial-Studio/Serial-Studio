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

#include <QOperatingSystemVersion>

#include "Misc/Utilities.h"
#include "IO/Drivers/BluetoothLE.h"

//------------------------------------------------------------------------------
// Constructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * Constructor function, configures the signals/slots of the BLE module
 */
IO::Drivers::BluetoothLE::BluetoothLE()
  : m_deviceIndex(-1)
  , m_deviceConnected(false)
  , m_service(nullptr)
  , m_controller(nullptr)
  , m_discoveryAgent(nullptr)
{
  connect(this, &IO::Drivers::BluetoothLE::deviceIndexChanged, this,
          &IO::Drivers::BluetoothLE::configurationChanged);

  connect(this, &IO::Drivers::BluetoothLE::error, this,
          [=](const QString &message) {
            Misc::Utilities::showMessageBox(tr("BLE I/O Module Error"),
                                            message);
          });
}

/**
 * Returns the only instance of the class
 */
IO::Drivers::BluetoothLE &IO::Drivers::BluetoothLE::instance()
{
  static BluetoothLE singleton;
  return singleton;
}

//------------------------------------------------------------------------------
// HAL driver implementation
//------------------------------------------------------------------------------

/**
 * @brief Closes the Bluetooth LE connection.
 *
 * This function clears all internal states, deletes existing services
 * and controllers, and resets the connection flags.
 */
void IO::Drivers::BluetoothLE::close()
{
  // Clear connected flag
  m_deviceConnected = false;

  // Restore initial conditions
  m_serviceNames.clear();
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  // Delete previous service
  if (m_service)
  {
    disconnect(m_service);
    delete m_service;
    m_service = nullptr;
  }

  // Delete previous controller
  if (m_controller)
  {
    disconnect(m_controller);
    m_controller->disconnectFromDevice();

    delete m_controller;
    m_controller = nullptr;
  }

  // Update UI
  Q_EMIT servicesChanged();
  Q_EMIT characteristicsChanged();
  Q_EMIT deviceConnectedChanged();
}

/**
 * @brief Checks if the Bluetooth LE device is open (connected).
 * @return True if the device is connected, false otherwise.
 */
bool IO::Drivers::BluetoothLE::isOpen() const
{
  return m_deviceConnected;
}

/**
 * @brief Checks if the Bluetooth LE device is readable.
 * @return Always true, as the device supports reading.
 */
bool IO::Drivers::BluetoothLE::isReadable() const
{
  return true;
}

/**
 * @brief Checks if the Bluetooth LE device is writable.
 * @return Always true, as the device supports writing.
 */
bool IO::Drivers::BluetoothLE::isWritable() const
{
  return true;
}

/**
 * @brief Verifies if the Bluetooth LE device configuration is valid.
 *
 * This function checks if the operating system is supported and if
 * the device index is valid.
 *
 * @return True if the configuration is valid, false otherwise.
 */
bool IO::Drivers::BluetoothLE::configurationOk() const
{
  return operatingSystemSupported() && deviceIndex() >= 0;
}

/**
 * @brief Writes data to the Bluetooth LE device.
 *
 * This function sends data to the currently selected characteristic of the
 * BLE device.
 *
 * @param data The data to be written to the BLE device.
 * @return The number of bytes written, or -1 if an error occurs.
 */
quint64 IO::Drivers::BluetoothLE::write(const QByteArray &data)
{
  if (m_service && m_selectedCharacteristic >= 0)
  {
    const auto &characteristic = m_characteristics.at(m_selectedCharacteristic);
    if (characteristic.isValid())
    {
      m_service->writeCharacteristic(characteristic, data,
                                     QLowEnergyService::WriteWithResponse);
      return data.length();
    }

    else
    {
      qWarning()
          << "Failed to write data to BLE device: invalid characteristic";
      return -1;
    }
  }

  qWarning() << "Failed to write data to BLE device: ensure that a "
                "characteristic is selected";

  return -1;
}

/**
 * @brief Opens a connection to a Bluetooth LE device.
 *
 * This function initializes the BLE controller, sets up signals/slots,
 * and attempts to connect to the specified device.
 *
 * @param mode Not used. Maintained for compatibility with QIODevice interface.
 * @return True if the connection is successful, false otherwise.
 */
bool IO::Drivers::BluetoothLE::open(const QIODevice::OpenMode mode)
{
  // I/O mode not used
  (void)mode;

  // Operating system not supported, abort process
  if (!operatingSystemSupported())
    return false;

  // Validar el indice del dispositivo
  if (m_deviceIndex < 0 || m_deviceIndex >= m_devices.count())
    return false;

  // Close previous device
  close();

  // Initialize a BLE controller for the current deveice
  auto device = m_devices.at(m_deviceIndex);
  m_controller = QLowEnergyController::createCentral(device, this);

  // Configure controller signals/slots
  connect(m_controller, &QLowEnergyController::discoveryFinished, this,
          &IO::Drivers::BluetoothLE::onServiceDiscoveryFinished);

  // React to connection event with BLE device
  connect(m_controller, &QLowEnergyController::connected, this, [this]() {
    m_deviceConnected = true;
    m_controller->discoverServices();
    Q_EMIT deviceConnectedChanged();
  });

  // React to disconnection event with BLE device
  connect(m_controller, &QLowEnergyController::disconnected, this,
          &IO::Drivers::BluetoothLE::close);

  // Pair with the BLE device
  m_controller->connectToDevice();
  return true;
}

/**
 * Returns @c false on macOS Monterey, for more info, please check this link:
 * https://forum.qt.io/topic/132285/mac-os-sdk12-not-working-with-qt-bluetooth
 */
bool IO::Drivers::BluetoothLE::operatingSystemSupported() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#  if defined(Q_OS_MAC)
  if (QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSBigSur)
    return false;
#  endif
#endif

  return true;
}

//------------------------------------------------------------------------------
// Driver specifics
//------------------------------------------------------------------------------

/**
 * @return The total number of discovered devices.
 */
int IO::Drivers::BluetoothLE::deviceCount() const
{
  return m_devices.count();
}

/**
 * @return The index of the BLE device selected by the user.
 */
int IO::Drivers::BluetoothLE::deviceIndex() const
{
  return m_deviceIndex;
}

/**
 * @return The index of the characteristic selected by the user.
 */
int IO::Drivers::BluetoothLE::characteristicIndex() const
{
  return m_selectedCharacteristic + 1;
}

/**
 * @return A list with the discovered BLE devices.
 */
QStringList IO::Drivers::BluetoothLE::deviceNames() const
{
  QStringList list;
  list.append(tr("Select Device"));
  list.append(m_deviceNames);
  return list;
}

/**
 * @return A list with the discovered BLE services for the current device.
 */
QStringList IO::Drivers::BluetoothLE::serviceNames() const
{
  QStringList list;
  list.append(tr("Select Service"));
  list.append(m_serviceNames);
  return list;
}

/**
 * @return A list with the discovered BLE characteristics for the current
 *         service.
 */
QStringList IO::Drivers::BluetoothLE::characteristicNames() const
{
  QStringList list;
  list.append(tr("Select Characteristic"));
  list.append(m_characteristicNames);
  return list;
}

/**
 * Starts the BLE device discovery process.
 */
void IO::Drivers::BluetoothLE::startDiscovery()
{
  // Operating system not supported, abort process
  if (!operatingSystemSupported())
    return;

  // Close previous device
  close();

  // Clear devices
  m_devices.clear();
  m_deviceIndex = -1;
  m_deviceNames.clear();
  Q_EMIT devicesChanged();

  // Delete previous discovery agent
  if (m_discoveryAgent)
  {
    disconnect(m_discoveryAgent);
    m_discoveryAgent->deleteLater();
    m_discoveryAgent = nullptr;
  }

  // Initialize a discovery agent
  m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

  // Register discovered devices
  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
          this, &IO::Drivers::BluetoothLE::onDeviceDiscovered);

  // Report BLE discovery errors
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  connect(&m_discoveryAgent,
          static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(
              QBluetoothDeviceDiscoveryAgent::Error)>(
              &QBluetoothDeviceDiscoveryAgent::error),
          this, &IO::Drivers::BluetoothLE::onDiscoveryError);
#else
  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
          this, &IO::Drivers::BluetoothLE::onDiscoveryError);
#endif

  // Start device discovery process
  m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

/**
 * Changes the index of the device selected by the user.
 *
 * @note This function compensates for the added "Select device" element to the
 * list of devices in @c deviceNames() so that the index corresponds to the
 * elements of
 *       @c m_deviceNames.
 */
void IO::Drivers::BluetoothLE::selectDevice(const int index)
{
  // Operating system not supported, abort process
  if (!operatingSystemSupported())
    return;

  // Close current device
  close();

  // Set new device index
  m_deviceIndex = index - 1;
  Q_EMIT deviceIndexChanged();
}

/**
 * Changes the index of the service selected by the user.
 *
 * @note This function compensates for the added "Select service" element to the
 * list of devices in @c serviceNames() so that the index corresponds to the
 * elements of
 *       @c m_serviceNames.
 */
void IO::Drivers::BluetoothLE::selectService(const int index)
{
  // Operating system not supported, abort process
  if (!operatingSystemSupported())
    return;

  // Delete previous service
  if (m_service)
  {
    disconnect(m_service);
    m_service->deleteLater();
    m_service = nullptr;
  }

  // Forget characteristics from previous service
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  // Ensure that index is valid
  if (index >= 1 && index <= m_serviceNames.count())
  {
    // Generate service handler & connect signals/slots
    auto serviceUuid = m_controller->services().at(index - 1);
    m_service = m_controller->createServiceObject(serviceUuid, this);
    if (m_service)
    {
      connect(m_service, &QLowEnergyService::characteristicChanged, this,
              &IO::Drivers::BluetoothLE::onCharacteristicChanged);
      connect(m_service, &QLowEnergyService::characteristicRead, this,
              &IO::Drivers::BluetoothLE::onCharacteristicChanged);
      connect(m_service, &QLowEnergyService::stateChanged, this,
              &IO::Drivers::BluetoothLE::onServiceStateChanged);
      connect(m_service, &QLowEnergyService::errorOccurred, this,
              &IO::Drivers::BluetoothLE::onServiceError);

      if (m_service->state() == QLowEnergyService::RemoteService)
        m_service->discoverDetails();
      else
        configureCharacteristics();
    }

    // Service handler error
    if (!m_service)
      Q_EMIT error(tr("Error while configuring BLE service"));
  }

  // Update UI
  Q_EMIT characteristicsChanged();
}

/**
 * Selects a characteristic from the @a characteristicNames() list and enables
 * notifications if possible.
 */
void IO::Drivers::BluetoothLE::setCharacteristicIndex(const int index)
{
  // Operating system not supported, abort process
  if (!operatingSystemSupported())
    return;

  // No service selected, abort process
  if (!m_service)
    return;

  // Update characteristic index
  if (index >= 0 && index <= m_characteristics.count())
    m_selectedCharacteristic = index - 1;
  else
    m_selectedCharacteristic = -1;

  // Query characteristic for information
  if (m_selectedCharacteristic >= 0)
  {
    // Obtain the characteristic
    const auto &c = m_characteristics.at(m_selectedCharacteristic);

    // Enable notifications for the CCCD
    const auto &cccd = c.clientCharacteristicConfiguration();
    if (cccd.isValid())
      m_service->writeDescriptor(
          cccd, QLowEnergyCharacteristic::CCCDEnableNotification);

    // Display current value
    if (!c.value().isEmpty())
      processData(c.value());
  }

  // Update UI
  Q_EMIT characteristicIndexChanged();
}

/**
 * Queries and registers the available characteristics for the currently
 * selected service.
 */
void IO::Drivers::BluetoothLE::configureCharacteristics()
{
  // Validate service pointer
  if (!m_service)
    return;

  // Forget characteristics from previous service
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  // Test & validate all service characteristics
  foreach (const QLowEnergyCharacteristic &c, m_service->characteristics())
  {
    // Validate characteristic
    if (!c.isValid())
      continue;

    // Register characteristic
    m_characteristics.append(c);
    if (!c.name().simplified().isEmpty())
      m_characteristicNames.append(c.name());
    else
      m_characteristicNames.append(c.uuid().toString());
  }

  // Update UI
  Q_EMIT characteristicsChanged();
  Q_EMIT characteristicIndexChanged();
}

/**
 * Generates a list with the UUIDs of the services available for the current BLE
 * device.
 */
void IO::Drivers::BluetoothLE::onServiceDiscoveryFinished()
{
  // Validate BLE controller
  if (!m_controller)
    return;

  // Generate services list
  m_serviceNames.clear();
  for (const auto &service : m_controller->services())
    m_serviceNames.append(service.toString());

  // Update UI
  Q_EMIT servicesChanged();
}

/**
 * Registers the device (@c device) found in the list of BLE devices.
 */
void IO::Drivers::BluetoothLE::onDeviceDiscovered(
    const QBluetoothDeviceInfo &device)
{
  // Stop if BLE controller is already connected or user selected a device
  if (m_controller || m_deviceIndex > 0)
    return;

  // Only register BLE devices
  if (device.coreConfigurations()
      & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
  {
    // Validate device name (we don't want to show hidden devices...)
    if (!device.isValid() || device.name().isEmpty())
      return;

    // Add the device to the list of found devices
    if (!m_devices.contains(device) && !m_deviceNames.contains(device.name()))
    {
      m_devices.append(device);
      m_deviceNames.append(device.name());
      Q_EMIT devicesChanged();
    }
  }
}

/**
 * Notifies any service error that occurs
 */
void IO::Drivers::BluetoothLE::onServiceError(
    QLowEnergyService::ServiceError serviceError)
{
  switch (serviceError)
  {
    case QLowEnergyService::OperationError:
      Q_EMIT error(tr("Operation error"));
      break;
    case QLowEnergyService::CharacteristicWriteError:
      Q_EMIT error(tr("Characteristic write error"));
      break;
    case QLowEnergyService::DescriptorWriteError:
      Q_EMIT error(tr("Descriptor write error"));
      break;
    case QLowEnergyService::UnknownError:
      Q_EMIT error(tr("Unknown error"));
      break;
    case QLowEnergyService::CharacteristicReadError:
      Q_EMIT error(tr("Characteristic read error"));
      break;
    case QLowEnergyService::DescriptorReadError:
      Q_EMIT error(tr("Descriptor read error"));
      break;
    default:
      break;
  }
}

/**
 * Notifies the user of any errors detected in the Bluetooth adapter of the
 * computer.
 */
void IO::Drivers::BluetoothLE::onDiscoveryError(
    QBluetoothDeviceDiscoveryAgent::Error e)
{
  switch (e)
  {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
      Q_EMIT error(tr("Bluetooth adapter is off!"));
      break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
      Q_EMIT error(tr("Invalid Bluetooth adapter!"));
      break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
      Q_EMIT error(tr("Unsuported platform or operating system"));
      break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
      Q_EMIT error(tr("Unsupported discovery method"));
      break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
      Q_EMIT error(tr("General I/O error"));
      break;
    default:
      break;
  }
}

/**
 * Configures the characteristics of the services found on the paired BLE
 * device.
 */
void IO::Drivers::BluetoothLE::onServiceStateChanged(
    QLowEnergyService::ServiceState serviceState)
{
  if (serviceState == QLowEnergyService::RemoteServiceDiscovered)
    configureCharacteristics();
}

/**
 * Reads the transmitted data from the BLE service.
 */
void IO::Drivers::BluetoothLE::onCharacteristicChanged(
    const QLowEnergyCharacteristic &info, const QByteArray &value)
{
  const bool anyCharacteristic = (m_selectedCharacteristic == -1);
  const bool current = (info == m_characteristics.at(m_selectedCharacteristic));
  if (anyCharacteristic || current)
    processData(value);
}
