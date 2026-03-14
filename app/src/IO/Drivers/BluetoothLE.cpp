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

#include "IO/Drivers/BluetoothLE.h"

#include <QJsonObject>
#include <QOperatingSystemVersion>

#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Static shared discovery state
//--------------------------------------------------------------------------------------------------

bool IO::Drivers::BluetoothLE::s_initialized                               = false;
bool IO::Drivers::BluetoothLE::s_adapterAvailable                          = false;
QBluetoothLocalDevice* IO::Drivers::BluetoothLE::s_localDevice             = nullptr;
QBluetoothDeviceDiscoveryAgent* IO::Drivers::BluetoothLE::s_discoveryAgent = nullptr;
QStringList IO::Drivers::BluetoothLE::s_deviceNames;
QList<QBluetoothDeviceInfo> IO::Drivers::BluetoothLE::s_devices;
QList<IO::Drivers::BluetoothLE*> IO::Drivers::BluetoothLE::s_instances;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a BluetoothLE driver instance.
 *
 * All instances share a single discovery agent and device list via static state.
 * Each instance maintains its own connection (controller, service, characteristics).
 */
IO::Drivers::BluetoothLE::BluetoothLE()
  : m_deviceIndex(-1)
  , m_deviceConnected(false)
  , m_selectedCharacteristic(-1)
  , m_pendingServiceIndex(-1)
  , m_pendingCharacteristicIndex(-1)
  , m_service(nullptr)
  , m_controller(nullptr)
{
  // Register this instance for shared discovery notifications
  s_instances.append(this);

  // Forward configuration-affecting signals
  connect(this,
          &IO::Drivers::BluetoothLE::deviceIndexChanged,
          this,
          &IO::Drivers::BluetoothLE::configurationChanged);
  connect(this,
          &IO::Drivers::BluetoothLE::characteristicIndexChanged,
          this,
          &IO::Drivers::BluetoothLE::configurationChanged);

  connect(this, &IO::Drivers::BluetoothLE::error, this, [=](const QString& message) {
    Misc::Utilities::showMessageBox(tr("BLE I/O Module Error"), message, QMessageBox::Critical);
  });
}

/**
 * @brief Destructor — unregisters from shared instance list.
 */
IO::Drivers::BluetoothLE::~BluetoothLE()
{
  close();
  s_instances.removeAll(this);
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the Bluetooth LE connection.
 *
 * Clears per-instance connection state (controller, service, characteristics).
 * Does not affect shared discovery state.
 */
void IO::Drivers::BluetoothLE::close()
{
  const bool wasConnected = m_deviceConnected;
  m_deviceConnected       = false;

  // Reset all service/characteristic state
  m_serviceNames.clear();
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  // Clean up service object
  if (m_service) {
    disconnect(m_service);
    delete m_service;
    m_service = nullptr;
  }

  // Clean up controller
  if (m_controller) {
    disconnect(m_controller);
    m_controller->disconnectFromDevice();

    delete m_controller;
    m_controller = nullptr;
  }

  // Clear service/characteristic lists on other instances with the same device
  if (wasConnected) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      inst->m_serviceNames.clear();
      inst->m_characteristicNames.clear();
      inst->m_selectedCharacteristic = -1;
      Q_EMIT inst->servicesChanged();
      Q_EMIT inst->characteristicsChanged();
      Q_EMIT inst->characteristicIndexChanged();
    }
  }

  Q_EMIT servicesChanged();
  Q_EMIT characteristicsChanged();
  Q_EMIT characteristicIndexChanged();
  Q_EMIT deviceConnectedChanged();
}

/**
 * @brief Checks if the Bluetooth LE device is open (connected or connecting).
 */
bool IO::Drivers::BluetoothLE::isOpen() const noexcept
{
  return m_deviceConnected || m_controller;
}

/**
 * @brief Checks if the Bluetooth LE device is readable.
 */
bool IO::Drivers::BluetoothLE::isReadable() const noexcept
{
  return true;
}

/**
 * @brief Checks if the Bluetooth LE device is writable.
 */
bool IO::Drivers::BluetoothLE::isWritable() const noexcept
{
  return true;
}

/**
 * @brief Verifies if the Bluetooth LE device configuration is valid.
 */
bool IO::Drivers::BluetoothLE::configurationOk() const noexcept
{
  return operatingSystemSupported() && adapterAvailable() && deviceIndex() >= 0;
}

/**
 * @brief Writes data to the Bluetooth LE device.
 */
qint64 IO::Drivers::BluetoothLE::write(const QByteArray& data)
{
  if (m_service && m_selectedCharacteristic >= 0
      && m_selectedCharacteristic < m_characteristics.count()) {
    const auto& characteristic = m_characteristics.at(m_selectedCharacteristic);
    if (characteristic.isValid()) {
      m_service->writeCharacteristic(characteristic, data, QLowEnergyService::WriteWithResponse);
      return data.length();
    }

    else {
      qWarning() << "Failed to write to BLE device: invalid characteristic";
      return 0;
    }
  }

  qWarning() << "Failed to write data to BLE device: ensure that a characteristic is selected";
  return 0;
}

/**
 * @brief Opens a connection to a Bluetooth LE device.
 *
 * Uses the shared discovery device list. Each instance creates its own
 * QLowEnergyController so multiple BLE connections can coexist.
 */
bool IO::Drivers::BluetoothLE::open(const QIODevice::OpenMode mode)
{
  (void)mode;

  // Unsupported OS, abort
  if (!operatingSystemSupported())
    return false;

  // Validate device index against shared device list
  if (m_deviceIndex < 0 || m_deviceIndex >= s_devices.count())
    return false;

  // Copy pending service/characteristic from the UI driver if not already set
  // (needed for live drivers created by ConnectionManager)
  if (m_pendingServiceUuid.isEmpty() && m_pendingServiceIndex < 0) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      // Copy service UUID from an instance that has a connected service
      if (inst->m_service) {
        m_pendingServiceUuid         = inst->m_service->serviceUuid().toString();
        m_pendingCharacteristicIndex = inst->m_selectedCharacteristic + 1;
        break;
      }

      // Or copy the pending UUID if it was set via project restore
      if (!inst->m_pendingServiceUuid.isEmpty()) {
        m_pendingServiceUuid         = inst->m_pendingServiceUuid;
        m_pendingCharacteristicIndex = inst->m_pendingCharacteristicIndex;
        break;
      }
    }
  }

  // Close any previous connection
  close();

  // Create BLE controller for the selected device
  auto device  = s_devices.at(m_deviceIndex);
  m_controller = QLowEnergyController::createCentral(device, this);

  connect(m_controller,
          &QLowEnergyController::discoveryFinished,
          this,
          &IO::Drivers::BluetoothLE::onServiceDiscoveryFinished);

  connect(m_controller, &QLowEnergyController::connected, this, [this]() {
    m_deviceConnected = true;
    m_controller->discoverServices();
    Q_EMIT deviceConnectedChanged();
  });

  connect(
    m_controller, &QLowEnergyController::disconnected, this, &IO::Drivers::BluetoothLE::close);

  m_controller->connectToDevice();
  return true;
}

/**
 * @brief Returns @c false on macOS Monterey (Qt < 6.0 only).
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
 * @brief Returns true if a Bluetooth adapter is available on the system.
 */
bool IO::Drivers::BluetoothLE::adapterAvailable() const
{
  return s_adapterAvailable;
}

//--------------------------------------------------------------------------------------------------
// Driver specifics
//--------------------------------------------------------------------------------------------------

/**
 * @return The total number of discovered devices (shared across all instances).
 */
int IO::Drivers::BluetoothLE::deviceCount() const
{
  return s_devices.count();
}

/**
 * @return The index of the BLE device selected by this instance.
 */
int IO::Drivers::BluetoothLE::deviceIndex() const
{
  return m_deviceIndex;
}

/**
 * @return The index of the characteristic selected by this instance.
 */
int IO::Drivers::BluetoothLE::characteristicIndex() const
{
  if (m_selectedCharacteristic >= 0)
    return m_selectedCharacteristic + 1;

  // Fall back to the live driver's selection
  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_selectedCharacteristic >= 0)
      return inst->m_selectedCharacteristic + 1;

  return 0;
}

/**
 * @return A list with the discovered BLE devices (shared, with placeholder).
 */
QStringList IO::Drivers::BluetoothLE::deviceNames() const
{
  QStringList list;
  list.append(tr("Select Device"));
  list.append(s_deviceNames);
  return list;
}

/**
 * @return A list with the discovered BLE services for this instance's connection.
 */
QStringList IO::Drivers::BluetoothLE::serviceNames() const
{
  QStringList list;
  list.append(tr("Select Service"));
  list.append(m_serviceNames);
  return list;
}

/**
 * @return A list with the discovered BLE characteristics for this instance.
 */
QStringList IO::Drivers::BluetoothLE::characteristicNames() const
{
  QStringList list;
  list.append(tr("Select Characteristic"));
  list.append(m_characteristicNames);
  return list;
}

/**
 * @brief Returns the UUID of the currently selected service, or an empty string.
 */
QString IO::Drivers::BluetoothLE::selectedServiceUuid() const
{
  // Check this instance first
  if (m_service)
    return m_service->serviceUuid().toString();

  // Fall back to other instances with the same device (live driver may have it)
  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_service)
      return inst->m_service->serviceUuid().toString();

  // Return any pending UUID that was set via project restore
  return m_pendingServiceUuid;
}

/**
 * @brief Starts the shared BLE device discovery process.
 *
 * Initializes the Bluetooth adapter (if needed) and starts scanning. All
 * registered instances are notified when devices are found.
 */
void IO::Drivers::BluetoothLE::startDiscovery()
{
  if (!operatingSystemSupported())
    return;

  // Lazily initialize shared Bluetooth adapter
  initializeSharedState();

  if (!s_adapterAvailable)
    return;

  // Skip if discovery is already running
  if (s_discoveryAgent && s_discoveryAgent->isActive())
    return;

  // Clean up previous discovery agent
  if (s_discoveryAgent) {
    QObject::disconnect(s_discoveryAgent, nullptr, nullptr, nullptr);
    s_discoveryAgent->deleteLater();
    s_discoveryAgent = nullptr;
  }

  // Create and start a new discovery agent
  s_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
  QObject::connect(s_discoveryAgent,
                   &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                   s_discoveryAgent,
                   [](const QBluetoothDeviceInfo& d) { onDeviceDiscovered(d); });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QObject::connect(
    s_discoveryAgent,
    static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>(
      &QBluetoothDeviceDiscoveryAgent::error),
    s_discoveryAgent,
    [](QBluetoothDeviceDiscoveryAgent::Error e) { onDiscoveryError(e); });
#else
  QObject::connect(s_discoveryAgent,
                   &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                   s_discoveryAgent,
                   [](QBluetoothDeviceDiscoveryAgent::Error e) { onDiscoveryError(e); });
#endif

  s_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

/**
 * @brief Changes the index of the device selected by the user.
 *
 * Compensates for the added "Select device" placeholder element.
 */
void IO::Drivers::BluetoothLE::selectDevice(const int index)
{
  if (!operatingSystemSupported())
    return;

  // Ignore index resets when a device is already selected or connected
  // (QML combobox rebuilds on devicesChanged can trigger selectDevice(0))
  if (index <= 0 && (m_deviceConnected || m_deviceIndex >= 0))
    return;

  close();

  // Update device index (compensate for placeholder entry)
  m_deviceIndex = index - 1;
  Q_EMIT deviceIndexChanged();
}

/**
 * @brief Changes the index of the service selected by the user.
 *
 * Compensates for the added "Select service" placeholder element.
 */
void IO::Drivers::BluetoothLE::selectService(const int index)
{
  if (!operatingSystemSupported())
    return;

  // If this instance has no controller, forward to the instance that does
  if (!m_controller) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst != this && inst->m_controller && inst->m_deviceIndex == m_deviceIndex) {
        inst->selectService(index);
        return;
      }
    }

    return;
  }

  // Clean up previous service
  if (m_service) {
    disconnect(m_service);
    m_service->deleteLater();
    m_service = nullptr;
  }

  // Reset characteristics
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  // Create service object for the selected index
  if (index >= 1 && index <= m_serviceNames.count()) {
    // Bounds check against controller's service list
    if (index - 1 >= m_controller->services().count())
      return;

    auto serviceUuid = m_controller->services().at(index - 1);
    m_service        = m_controller->createServiceObject(serviceUuid, this);
    if (m_service) {
      connect(m_service,
              &QLowEnergyService::characteristicChanged,
              this,
              &IO::Drivers::BluetoothLE::onCharacteristicChanged);
      connect(m_service,
              &QLowEnergyService::characteristicRead,
              this,
              &IO::Drivers::BluetoothLE::onCharacteristicChanged);
      connect(m_service,
              &QLowEnergyService::stateChanged,
              this,
              &IO::Drivers::BluetoothLE::onServiceStateChanged);
      connect(m_service,
              &QLowEnergyService::errorOccurred,
              this,
              &IO::Drivers::BluetoothLE::onServiceError);

      if (m_service->state() == QLowEnergyService::RemoteService)
        m_service->discoverDetails();
      else
        configureCharacteristics();
    }

    if (!m_service)
      Q_EMIT error(tr("Error while configuring BLE service"));
  }

  Q_EMIT characteristicsChanged();
}

/**
 * @brief Selects a characteristic and enables notifications if possible.
 */
void IO::Drivers::BluetoothLE::setCharacteristicIndex(const int index)
{
  if (!operatingSystemSupported())
    return;

  // If this instance has no service, forward to the instance that does
  if (!m_service) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst != this && inst->m_service && inst->m_deviceIndex == m_deviceIndex) {
        inst->setCharacteristicIndex(index);
        return;
      }
    }

    return;
  }

  // Update selected characteristic index
  if (index >= 0 && index <= m_characteristics.count())
    m_selectedCharacteristic = index - 1;
  else
    m_selectedCharacteristic = -1;

  // Enable notifications and emit initial value
  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count()) {
    const auto& c = m_characteristics.at(m_selectedCharacteristic);

    // Enable CCCD notifications
    const auto& cccd = c.clientCharacteristicConfiguration();
    if (cccd.isValid())
      m_service->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDEnableNotification);

    if (!c.value().isEmpty())
      Q_EMIT dataReceived(makeByteArray(c.value()));
  }

  Q_EMIT characteristicIndexChanged();
}

//--------------------------------------------------------------------------------------------------
// Per-instance private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Queries and registers available characteristics for the selected service.
 */
void IO::Drivers::BluetoothLE::configureCharacteristics()
{
  if (!m_service)
    return;

  // Reset and rebuild characteristics list
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;
  foreach (const QLowEnergyCharacteristic& c, m_service->characteristics()) {
    if (!c.isValid())
      continue;

    m_characteristics.append(c);
    if (!c.name().simplified().isEmpty())
      m_characteristicNames.append(c.name());
    else
      m_characteristicNames.append(c.uuid().toString());
  }

  Q_EMIT characteristicsChanged();
  Q_EMIT characteristicIndexChanged();

  // Propagate characteristic list to other instances so QML can display them
  for (auto* inst : std::as_const(s_instances)) {
    if (inst != this && inst->m_deviceIndex == m_deviceIndex) {
      inst->m_characteristicNames = m_characteristicNames;
      Q_EMIT inst->characteristicsChanged();
    }
  }

  // Auto-select characteristic for live drivers
  if (m_pendingCharacteristicIndex > 0) {
    setCharacteristicIndex(m_pendingCharacteristicIndex);
    m_pendingCharacteristicIndex = -1;
  }
}

/**
 * @brief Builds the service UUID list after service discovery completes.
 */
void IO::Drivers::BluetoothLE::onServiceDiscoveryFinished()
{
  if (!m_controller)
    return;

  // Build service UUID list
  m_serviceNames.clear();
  for (const auto& service : m_controller->services())
    m_serviceNames.append(service.toString());

  Q_EMIT servicesChanged();

  // Propagate service list to other instances so QML can display them
  for (auto* inst : std::as_const(s_instances)) {
    if (inst != this && inst->m_deviceIndex == m_deviceIndex) {
      inst->m_serviceNames = m_serviceNames;
      Q_EMIT inst->servicesChanged();
    }
  }

  // Auto-select service by pending index or UUID
  if (m_pendingServiceIndex > 0) {
    selectService(m_pendingServiceIndex);
    m_pendingServiceIndex = -1;
  }

  else if (!m_pendingServiceUuid.isEmpty()) {
    int idx = m_serviceNames.indexOf(m_pendingServiceUuid);
    if (idx >= 0) {
      selectService(idx + 1);
      m_pendingServiceUuid.clear();
    }
  }

  // Also check pending UUIDs on other instances (project restore)
  for (auto* inst : std::as_const(s_instances)) {
    if (inst == this || inst->m_deviceIndex != m_deviceIndex)
      continue;

    if (!inst->m_pendingServiceUuid.isEmpty()) {
      int idx = m_serviceNames.indexOf(inst->m_pendingServiceUuid);
      if (idx >= 0) {
        selectService(idx + 1);
        inst->m_pendingServiceUuid.clear();
      }
    }
  }
}

/**
 * @brief Notifies any service error that occurs.
 */
void IO::Drivers::BluetoothLE::onServiceError(QLowEnergyService::ServiceError serviceError)
{
  switch (serviceError) {
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
 * @brief Triggers characteristic configuration when service details are discovered.
 */
void IO::Drivers::BluetoothLE::onServiceStateChanged(QLowEnergyService::ServiceState serviceState)
{
  if (serviceState == QLowEnergyService::RemoteServiceDiscovered)
    configureCharacteristics();
}

/**
 * @brief Reads transmitted data from the BLE service.
 */
void IO::Drivers::BluetoothLE::onCharacteristicChanged(const QLowEnergyCharacteristic& info,
                                                       const QByteArray& value)
{
  if (m_selectedCharacteristic == -1) {
    Q_EMIT dataReceived(makeByteArray(value));
    return;
  }

  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count()
      && info == m_characteristics.at(m_selectedCharacteristic)) {
    Q_EMIT dataReceived(makeByteArray(value));
  }
}

//--------------------------------------------------------------------------------------------------
// Static shared discovery callbacks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the shared Bluetooth adapter detection (called once).
 */
void IO::Drivers::BluetoothLE::initializeSharedState()
{
  if (s_initialized)
    return;

  s_initialized = true;

  s_localDevice = new QBluetoothLocalDevice();

  // Read initial adapter state
  auto hostMode      = s_localDevice->hostMode();
  s_adapterAvailable = (hostMode != QBluetoothLocalDevice::HostPoweredOff);

  QObject::connect(s_localDevice,
                   &QBluetoothLocalDevice::hostModeStateChanged,
                   s_localDevice,
                   [](QBluetoothLocalDevice::HostMode m) { onHostModeStateChanged(m); });

  // Notify all instances of initial adapter state
  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->adapterAvailabilityChanged();
}

/**
 * @brief Static callback — registers a discovered BLE device in the shared list.
 *
 * Notifies all registered instances so their device lists update.
 */
void IO::Drivers::BluetoothLE::onDeviceDiscovered(const QBluetoothDeviceInfo& device)
{
  // Only register BLE-capable devices with valid names
  if (!(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
    return;

  if (!device.isValid() || device.name().isEmpty())
    return;

  // Deduplicate by device object and name
  if (s_devices.contains(device) || s_deviceNames.contains(device.name()))
    return;

  s_devices.append(device);
  s_deviceNames.append(device.name());

  // Notify instances that are not connected (QML combobox resets
  // the selection when the model changes, even if indices are stable)
  for (auto* inst : std::as_const(s_instances)) {
    if (inst->m_deviceConnected)
      continue;

    Q_EMIT inst->devicesChanged();

    // Check if this device matches a pending identifier
    if (!inst->m_pendingIdentifier.isEmpty()) {
      const auto savedAddr = inst->m_pendingIdentifier.value(QStringLiteral("address")).toString();
      const auto savedName = inst->m_pendingIdentifier.value(QStringLiteral("name")).toString();

      bool match = false;
      if (!savedAddr.isEmpty() && device.address().toString() == savedAddr)
        match = true;
      else if (!savedName.isEmpty() && device.name() == savedName)
        match = true;

      if (match) {
        inst->selectDevice(s_devices.count());
        inst->m_pendingIdentifier = {};
      }
    }
  }
}

/**
 * @brief Static callback — notifies all instances of a discovery error.
 */
void IO::Drivers::BluetoothLE::onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error e)
{
  QString message;
  switch (e) {
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
      message = QObject::tr("Invalid Bluetooth adapter!");
      break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
      message = QObject::tr("Unsuported platform or operating system");
      break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
      message = QObject::tr("Unsupported discovery method");
      break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
      message = QObject::tr("General I/O error");
      break;
    default:
      return;
  }

  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->error(message);
}

/**
 * @brief Static callback — handles Bluetooth adapter power state changes.
 */
void IO::Drivers::BluetoothLE::onHostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
  bool wasAvailable  = s_adapterAvailable;
  s_adapterAvailable = (state != QBluetoothLocalDevice::HostPoweredOff);

  if (wasAvailable == s_adapterAvailable)
    return;

  // Notify all instances
  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->adapterAvailabilityChanged();

  // Adapter lost — close all connections and clear shared device list
  if (!s_adapterAvailable) {
    if (s_discoveryAgent && s_discoveryAgent->isActive())
      s_discoveryAgent->stop();

    s_devices.clear();
    s_deviceNames.clear();
    for (auto* inst : std::as_const(s_instances)) {
      inst->close();
      inst->m_deviceIndex = -1;
      Q_EMIT inst->devicesChanged();
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the BLE address and name of the currently selected device.
 */
QJsonObject IO::Drivers::BluetoothLE::deviceIdentifier() const
{
  if (m_deviceIndex < 0 || m_deviceIndex >= s_devices.count())
    return {};

  const auto& device = s_devices.at(m_deviceIndex);
  QJsonObject id;

  const auto addr = device.address().toString();
  if (!addr.isEmpty() && addr != QStringLiteral("00:00:00:00:00:00"))
    id.insert(QStringLiteral("address"), addr);

  const auto name = device.name();
  if (!name.isEmpty())
    id.insert(QStringLiteral("name"), name);

  return id;
}

/**
 * @brief Tries to find a discovered BLE device matching a previously saved identifier.
 */
bool IO::Drivers::BluetoothLE::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  const auto savedAddr = id.value(QStringLiteral("address")).toString();
  const auto savedName = id.value(QStringLiteral("name")).toString();

  // Search shared device list
  int bestScore = 0;
  int bestIndex = -1;

  for (int i = 0; i < s_devices.count(); ++i) {
    const auto& device = s_devices.at(i);
    int score          = 0;

    if (!savedAddr.isEmpty() && device.address().toString() == savedAddr)
      score += 100;

    if (!savedName.isEmpty() && device.name() == savedName)
      score += 10;

    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  if (bestIndex >= 0) {
    selectDevice(bestIndex + 1);
    m_pendingIdentifier = {};
    return true;
  }

  // Device not found yet — save for deferred matching and start discovery
  m_pendingIdentifier = id;
  if (!s_discoveryAgent || !s_discoveryAgent->isActive())
    startDiscovery();

  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Bluetooth LE configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::BluetoothLE::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty dev;
  dev.key     = QStringLiteral("deviceIndex");
  dev.label   = tr("BLE Device");
  dev.type    = IO::DriverProperty::ComboBox;
  dev.value   = m_deviceIndex;
  dev.options = s_deviceNames;
  props.append(dev);

  IO::DriverProperty svc;
  svc.key   = QStringLiteral("serviceUuid");
  svc.label = tr("Service");
  svc.type  = IO::DriverProperty::Text;
  svc.value = selectedServiceUuid();
  props.append(svc);

  // Get characteristic index (fall back to live driver if needed)
  int charIdx = m_selectedCharacteristic;
  if (charIdx < 0) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst != this && inst->m_deviceIndex == m_deviceIndex
          && inst->m_selectedCharacteristic >= 0) {
        charIdx = inst->m_selectedCharacteristic;
        break;
      }
    }
  }

  IO::DriverProperty ch;
  ch.key     = QStringLiteral("characteristicIndex");
  ch.label   = tr("Characteristic");
  ch.type    = IO::DriverProperty::ComboBox;
  ch.value   = charIdx;
  ch.options = m_characteristicNames;
  props.append(ch);

  return props;
}

/**
 * @brief Applies a single Bluetooth LE configuration change by key.
 *
 * Sets the internal value directly (no placeholder compensation) since
 * driverProperties() returns the raw internal values.
 */
void IO::Drivers::BluetoothLE::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("deviceIndex")) {
    m_deviceIndex = value.toInt();
    Q_EMIT deviceIndexChanged();
  }

  else if (key == QLatin1String("serviceUuid")) {
    m_pendingServiceUuid = value.toString();
  }

  else if (key == QLatin1String("characteristicIndex")) {
    m_selectedCharacteristic     = value.toInt();
    m_pendingCharacteristicIndex = value.toInt() + 1;
    Q_EMIT characteristicIndexChanged();
  }
}
