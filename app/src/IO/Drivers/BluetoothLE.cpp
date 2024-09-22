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

#include "IO/Manager.h"
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
  , m_service(Q_NULLPTR)
  , m_controller(Q_NULLPTR)
  , m_discoveryAgent(Q_NULLPTR)
{
  // Update connect button status when a BLE device is selected by the user
  connect(this, &IO::Drivers::BluetoothLE::deviceIndexChanged, this,
          &IO::Drivers::BluetoothLE::configurationChanged);
  connect(this, &IO::Drivers::BluetoothLE::deviceConnectedChanged,
          &IO::Manager::instance(), &IO::Manager::connectedChanged);
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

void IO::Drivers::BluetoothLE::close()
{
  // Clear services list & reset flags
  m_serviceNames.clear();
  m_deviceConnected = false;

  // Delete previous service
  if (m_service)
  {
    disconnect(m_service);
    m_service->deleteLater();
    m_service = Q_NULLPTR;
  }

  // Delete previous controller
  if (m_controller)
  {
    disconnect(m_controller);
    m_controller->disconnectFromDevice();
    m_controller->deleteLater();
    m_controller = Q_NULLPTR;
  }

  // Update UI
  Q_EMIT devicesChanged();
  Q_EMIT servicesChanged();
  Q_EMIT deviceConnectedChanged();
}

bool IO::Drivers::BluetoothLE::isOpen() const
{
  return m_deviceConnected;
}

bool IO::Drivers::BluetoothLE::isReadable() const
{
  return false;
}

bool IO::Drivers::BluetoothLE::isWritable() const
{
  return false;
}

bool IO::Drivers::BluetoothLE::configurationOk() const
{
  return operatingSystemSupported() && deviceIndex() >= 0;
}

quint64 IO::Drivers::BluetoothLE::write(const QByteArray &data)
{
  // TODO
  (void)data;
  return -1;
}

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
  connect(m_controller, &QLowEnergyController::disconnected, this, [this]() {
    if (m_service)
    {
      disconnect(m_service);
      m_service->deleteLater();
      m_service = Q_NULLPTR;
    }

    if (m_controller)
    {
      disconnect(m_controller);
      m_controller->deleteLater();
      m_controller = Q_NULLPTR;
    }

    m_deviceConnected = false;

    Q_EMIT deviceConnectedChanged();
    Q_EMIT error(tr("The BLE device has been disconnected"));
  });

  // Pair with the BLE device
  m_controller->connectToDevice();
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

/**
 * Starts the BLE device discovery process.
 */
void IO::Drivers::BluetoothLE::startDiscovery()
{
  // Operating system not supported, abort process
  if (!operatingSystemSupported())
    return;

  // Restore initial conditions
  m_devices.clear();
  m_deviceIndex = -1;
  m_deviceNames.clear();

  // Close previous device
  close();

  // Update UI
  Q_EMIT devicesChanged();
  Q_EMIT deviceIndexChanged();

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
    m_service = Q_NULLPTR;
  }

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
}

/**
 * Sets the interaction options between each characteristic of the BLE device
 * and the BLE module.
 */
void IO::Drivers::BluetoothLE::configureCharacteristics()
{
  // Validate service pointer
  if (!m_service)
    return;

  // Test & validate all service characteristics
  foreach (QLowEnergyCharacteristic c, m_service->characteristics())
  {
    // Validate characteristic
    if (!c.isValid())
      continue;

    // Set client/descriptor connection properties
    auto descriptor = c.clientCharacteristicConfiguration();
    if (descriptor.isValid())
      m_service->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
  }
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
  for (auto i = 0; i < m_controller->services().count(); ++i)
    m_serviceNames.append(m_controller->services().at(i).toString());

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
  (void)info;
  Q_EMIT dataReceived(value);
}
