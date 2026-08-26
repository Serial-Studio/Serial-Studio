/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/BluetoothLE.h"

#include <QBluetoothUuid>
#include <QJsonObject>
#include <QOperatingSystemVersion>

#include "IO/ConnectionManager.h"
#include "IO/Drivers/BluetoothLE/BleUuids.h"
#include "Misc/Utilities.h"

using namespace IO::Drivers::BleDetail;

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
 * @brief Constructs a BLE driver instance and registers it for shared discovery.
 */
IO::Drivers::BluetoothLE::BluetoothLE()
  : m_deviceIndex(-1)
  , m_deviceConnected(false)
  , m_gattReady(false)
  , m_selectedCharacteristic(-1)
  , m_pendingServiceIndex(-1)
  , m_pendingCharacteristicIndex(-1)
  , m_probeServiceIndex(-1)
  , m_service(nullptr)
  , m_controller(nullptr)
{
  s_instances.append(this);

  connect(this,
          &IO::Drivers::BluetoothLE::deviceIndexChanged,
          this,
          &IO::Drivers::BluetoothLE::configurationChanged);
  connect(this,
          &IO::Drivers::BluetoothLE::characteristicIndexChanged,
          this,
          &IO::Drivers::BluetoothLE::configurationChanged);
  connect(this,
          &IO::Drivers::BluetoothLE::adapterAvailabilityChanged,
          this,
          &IO::Drivers::BluetoothLE::configurationChanged);

  connect(this, &IO::Drivers::BluetoothLE::error, this, [](const QString& message) {
    logDriverError(tr("BLE I/O Module Error"), message);
  });
}

/**
 * @brief Destructor; closes any active connection and unregisters this instance.
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
 * @brief Closes the Bluetooth LE connection. The resolved service + notify UUIDs are
 *        re-staged as pendings so the GATT configuration survives a disconnect. Controller
 *        and service go through deleteLater(): close() can run inside the controller's own
 *        disconnected emission, and deleting the sender mid-emission crashes the backend.
 */
void IO::Drivers::BluetoothLE::close()
{
  const bool wasConnected = m_deviceConnected;
  m_deviceConnected       = false;
  m_gattReady             = false;
  m_probeServiceIndex     = -1;

  const QString liveServiceUuid = m_service ? m_service->serviceUuid().toString() : QString();
  QString liveNotifyUuid;
  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count())
    liveNotifyUuid = m_characteristics.at(m_selectedCharacteristic).uuid().toString();

  if (!liveServiceUuid.isEmpty())
    m_pendingServiceUuid = liveServiceUuid;

  if (!liveNotifyUuid.isEmpty())
    m_pendingNotifyUuid = liveNotifyUuid;

  m_serviceNames.clear();
  m_serviceUuids.clear();
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  if (m_service) {
    m_service->disconnect(this);
    m_service->deleteLater();
    m_service = nullptr;
  }

  if (m_controller) {
    m_controller->disconnect(this);
    m_controller->disconnectFromDevice();
    m_controller->deleteLater();
    m_controller = nullptr;
  }

  if (wasConnected) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      inst->m_serviceNames.clear();
      inst->m_serviceUuids.clear();
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
 * @brief Open once the link is up and GATT (service + notify characteristic) is wired.
 */
bool IO::Drivers::BluetoothLE::isOpen() const noexcept
{
  return m_deviceConnected && m_gattReady;
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
      const auto props = characteristic.properties();
      const auto mode  = (props & QLowEnergyCharacteristic::WriteNoResponse)
                         ? QLowEnergyService::WriteWithoutResponse
                         : QLowEnergyService::WriteWithResponse;

      m_service->writeCharacteristic(characteristic, data, mode);
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
 * @brief Writes to a UUID-resolved characteristic for split read/write devices.
 */
qint64 IO::Drivers::BluetoothLE::writeCharacteristic(const QString& uuid, const QByteArray& data)
{
  const QBluetoothUuid target = bleUuidFromString(uuid);
  if (target.isNull()) {
    qWarning() << "BLE writeCharacteristic: invalid UUID" << uuid;
    return 0;
  }

  QLowEnergyService* service = m_service;
  if (!service) {
    for (auto* inst : std::as_const(s_instances))
      if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_service) {
        service = inst->m_service;
        break;
      }
  }

  if (!service) {
    qWarning() << "BLE writeCharacteristic: no active service for the selected device";
    return 0;
  }

  const auto characteristic = service->characteristic(target);
  if (!characteristic.isValid()) {
    qWarning() << "BLE writeCharacteristic: characteristic" << uuid << "not found in service";
    return 0;
  }

  const auto props = characteristic.properties();
  const auto mode  = (props & QLowEnergyCharacteristic::WriteNoResponse)
                     ? QLowEnergyService::WriteWithoutResponse
                     : QLowEnergyService::WriteWithResponse;

  service->writeCharacteristic(characteristic, data, mode);
  return data.length();
}

/**
 * @brief Opens a connection to a Bluetooth LE device.
 */
bool IO::Drivers::BluetoothLE::open(const QIODevice::OpenMode mode)
{
  (void)mode;

  if (!operatingSystemSupported())
    return false;

  if (m_deviceIndex < 0 || m_deviceIndex >= s_devices.count())
    return false;

  if (m_pendingServiceUuid.isEmpty() && m_pendingServiceIndex < 0) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      if (inst->m_service) {
        m_pendingServiceUuid         = inst->m_service->serviceUuid().toString();
        m_pendingCharacteristicIndex = inst->m_selectedCharacteristic + 1;
        break;
      }

      if (!inst->m_pendingServiceUuid.isEmpty()) {
        m_pendingServiceUuid         = inst->m_pendingServiceUuid;
        m_pendingCharacteristicIndex = inst->m_pendingCharacteristicIndex;
        break;
      }
    }
  }

  close();

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

  connect(m_controller,
          &QLowEnergyController::disconnected,
          this,
          &IO::Drivers::BluetoothLE::onControllerDisconnected);

  connect(m_controller,
          &QLowEnergyController::errorOccurred,
          this,
          &IO::Drivers::BluetoothLE::onControllerError);

  m_controller->connectToDevice();
  return true;
}

/**
 * @brief Returns true while a dial started by open() has neither reached GATT-ready nor failed.
 */
bool IO::Drivers::BluetoothLE::isConnecting() const noexcept
{
  return m_controller != nullptr && !isOpen();
}

/**
 * @brief Handles the controller's disconnected signal. A drop while the dial is still pending
 *        reports the failure verdict BEFORE close() severs the controller's signals (backend
 *        ordering of disconnected vs errorOccurred is platform-dependent); an established-link
 *        drop keeps the historical close-only behavior.
 */
void IO::Drivers::BluetoothLE::onControllerDisconnected()
{
  reportOpenFinished(false, tr("The device disconnected before the connection completed"));
  close();
}

/**
 * @brief Reports a controller-level failure. A pending dial reports its verdict through
 *        openFinished; an ESTABLISHED link that errors goes through the driver-drop path
 *        instead, because the verdict latch is already spent and close() alone never
 *        reaches the connection manager.
 */
void IO::Drivers::BluetoothLE::onControllerError(QLowEnergyController::Error controllerError)
{
  if (controllerError == QLowEnergyController::NoError)
    return;

  const QString reason = m_controller ? m_controller->errorString() : tr("Unknown error");
  Q_EMIT error(tr("BLE connection error: %1").arg(reason));

  const bool dialing = openReportArmed();
  close();

  if (dialing) {
    reportOpenFinished(false, reason);
    return;
  }

  static auto& connectionManager = ConnectionManager::instance();
  connectionManager.disconnectDevice(this);
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

/**
 * @brief Returns whether a powered-on adapter exists, initializing the shared local-device state
 *        first so a caller that runs before any discovery does not read a stale false. The shared
 *        discovery agent is untouched: this path never starts, stops, or duplicates a scan.
 */
bool IO::Drivers::BluetoothLE::adapterPoweredOn()
{
  initializeSharedState();
  return s_adapterAvailable;
}

//--------------------------------------------------------------------------------------------------
// Driver specifics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of discovered devices (shared across instances).
 */
int IO::Drivers::BluetoothLE::deviceCount() const
{
  return s_devices.count();
}

/**
 * @brief Returns the index of the BLE device selected by this instance.
 */
int IO::Drivers::BluetoothLE::deviceIndex() const
{
  return m_deviceIndex;
}

/**
 * @brief Returns the index of the characteristic selected by this instance.
 */
int IO::Drivers::BluetoothLE::characteristicIndex() const
{
  if (m_selectedCharacteristic >= 0)
    return m_selectedCharacteristic + 1;

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_selectedCharacteristic >= 0)
      return inst->m_selectedCharacteristic + 1;

  return 0;
}

/**
 * @brief Returns the discovered BLE devices with a leading placeholder entry.
 */
QStringList IO::Drivers::BluetoothLE::deviceNames() const
{
  QStringList list;
  list.append(tr("Select Device"));
  list.append(s_deviceNames);
  return list;
}

/**
 * @brief Returns the discovered BLE services with a leading placeholder entry.
 */
QStringList IO::Drivers::BluetoothLE::serviceNames() const
{
  QStringList list;
  list.append(tr("Select Service"));
  list.append(m_serviceNames);
  return list;
}

/**
 * @brief Returns the discovered BLE characteristics with a leading placeholder.
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
  if (m_service)
    return m_service->serviceUuid().toString();

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_service)
      return inst->m_service->serviceUuid().toString();

  if (!m_pendingServiceUuid.isEmpty())
    return m_pendingServiceUuid;

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex
        && !inst->m_pendingServiceUuid.isEmpty())
      return inst->m_pendingServiceUuid;

  return {};
}

/**
 * @brief Returns the UUID of the subscribed notify characteristic, or the pending one.
 */
QString IO::Drivers::BluetoothLE::selectedNotifyCharacteristicUuid() const
{
  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count())
    return m_characteristics.at(m_selectedCharacteristic).uuid().toString();

  for (auto* inst : std::as_const(s_instances)) {
    if (inst == this || inst->m_deviceIndex != m_deviceIndex)
      continue;

    const int idx = inst->m_selectedCharacteristic;
    if (idx >= 0 && idx < inst->m_characteristics.count())
      return inst->m_characteristics.at(idx).uuid().toString();
  }

  if (!m_pendingNotifyUuid.isEmpty())
    return m_pendingNotifyUuid;

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex
        && !inst->m_pendingNotifyUuid.isEmpty())
      return inst->m_pendingNotifyUuid;

  return {};
}

/**
 * @brief Marks GATT ready once per connection and announces it on both wiring paths.
 */
void IO::Drivers::BluetoothLE::announceGattReady()
{
  if (m_gattReady)
    return;

  m_gattReady = true;
  reportOpenFinished(true);
  Q_EMIT gattReady();
  Q_EMIT configurationChanged();
}

/**
 * @brief Starts the shared BLE device discovery process.
 */
void IO::Drivers::BluetoothLE::startDiscovery()
{
  if (!operatingSystemSupported())
    return;

  initializeSharedState();

  if (!s_adapterAvailable)
    return;

  if (s_discoveryAgent && s_discoveryAgent->isActive())
    return;

  if (s_discoveryAgent) {
    QObject::disconnect(s_discoveryAgent, nullptr, nullptr, nullptr);
    s_discoveryAgent->deleteLater();
    s_discoveryAgent = nullptr;
  }

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
 * @brief Changes the index of the device selected by the user. Selecting the placeholder entry
 * (index 0) clears the selection so the connect button reflects the real configuration state.
 */
void IO::Drivers::BluetoothLE::selectDevice(const int index)
{
  if (!operatingSystemSupported())
    return;

  if (index <= 0) {
    if (m_deviceIndex < 0 && !m_deviceConnected)
      return;

    close();
    m_deviceIndex = -1;
    Q_EMIT deviceIndexChanged();
    return;
  }

  if (index - 1 >= s_devices.count())
    return;

  close();

  m_deviceIndex = index - 1;
  Q_EMIT deviceIndexChanged();
}

/**
 * @brief Changes the index of the service selected by the user.
 */
void IO::Drivers::BluetoothLE::selectService(const int index)
{
  if (!operatingSystemSupported())
    return;

  static bool s_forwarding = false;

  if (!m_controller) {
    if (s_forwarding)
      return;

    s_forwarding = true;
    for (auto* inst : std::as_const(s_instances)) {
      if (inst != this && inst->m_controller && inst->m_deviceIndex == m_deviceIndex) {
        inst->selectService(index);
        s_forwarding = false;
        return;
      }
    }

    s_forwarding = false;
    return;
  }

  if (m_service) {
    m_service->disconnect(this);
    m_service->deleteLater();
    m_service = nullptr;
  }

  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  if (index >= 1 && index <= m_serviceNames.count()) {
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

  static bool s_charForwarding = false;

  if (!m_service) {
    if (s_charForwarding)
      return;

    s_charForwarding = true;
    for (auto* inst : std::as_const(s_instances)) {
      if (inst != this && inst->m_service && inst->m_deviceIndex == m_deviceIndex) {
        inst->setCharacteristicIndex(index);
        s_charForwarding = false;
        return;
      }
    }

    s_charForwarding = false;
    return;
  }

  if (index >= 0 && index <= m_characteristics.count())
    m_selectedCharacteristic = index - 1;
  else
    m_selectedCharacteristic = -1;

  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count()) {
    const auto& c = m_characteristics.at(m_selectedCharacteristic);

    const auto& cccd = c.clientCharacteristicConfiguration();
    if (cccd.isValid())
      m_service->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDEnableNotification);

    if (!c.value().isEmpty())
      publishReceivedData(c.value());
  }

  Q_EMIT characteristicIndexChanged();
}

/**
 * @brief Selects the discovered service whose UUID matches, by delegating to selectService.
 */
bool IO::Drivers::BluetoothLE::selectServiceByUuid(const QString& uuid)
{
  const QBluetoothUuid target = bleUuidFromString(uuid);
  if (target.isNull())
    return false;

  for (int i = 0; i < m_serviceUuids.count(); ++i) {
    if (QBluetoothUuid(m_serviceUuids.at(i)) == target) {
      selectService(i + 1);
      return true;
    }
  }

  return false;
}

/**
 * @brief Subscribes to the notify characteristic with the given UUID, by index delegation.
 */
bool IO::Drivers::BluetoothLE::setNotifyCharacteristicByUuid(const QString& uuid)
{
  const QBluetoothUuid target = bleUuidFromString(uuid);
  if (target.isNull())
    return false;

  for (int i = 0; i < m_characteristics.count(); ++i) {
    if (m_characteristics.at(i).uuid() == target) {
      setCharacteristicIndex(i + 1);
      return true;
    }
  }

  return false;
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

  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic   = -1;
  const auto characteristics = m_service->characteristics();
  for (const auto& c : characteristics) {
    if (!c.isValid())
      continue;

    m_characteristics.append(c);
    m_characteristicNames.append(bleCharacteristicName(c));
  }

  Q_EMIT characteristicsChanged();
  Q_EMIT characteristicIndexChanged();

  for (auto* inst : std::as_const(s_instances)) {
    if (inst != this && inst->m_deviceIndex == m_deviceIndex) {
      inst->m_characteristicNames = m_characteristicNames;
      Q_EMIT inst->characteristicsChanged();
    }
  }

  if (!m_pendingNotifyUuid.isEmpty()) {
    const bool found = setNotifyCharacteristicByUuid(m_pendingNotifyUuid);
    if (!found && m_probeServiceIndex >= 0 && m_probeServiceIndex + 1 < m_serviceUuids.count()) {
      ++m_probeServiceIndex;
      selectService(m_probeServiceIndex + 1);
      return;
    }

    if (found) {
      m_pendingNotifyUuid.clear();
      m_pendingCharacteristicIndex = -1;
    }

    m_probeServiceIndex = -1;
  }

  else if (m_pendingCharacteristicIndex > 0) {
    setCharacteristicIndex(m_pendingCharacteristicIndex);
    m_pendingCharacteristicIndex = -1;
  }

  announceGattReady();
}

/**
 * @brief Builds the service UUID list after service discovery completes.
 */
void IO::Drivers::BluetoothLE::onServiceDiscoveryFinished()
{
  if (!m_controller)
    return;

  m_serviceNames.clear();
  m_serviceUuids.clear();
  for (const auto& service : m_controller->services()) {
    m_serviceUuids.append(service.toString());
    m_serviceNames.append(bleServiceName(service));
  }

  Q_EMIT servicesChanged();

  for (auto* inst : std::as_const(s_instances)) {
    if (inst != this && inst->m_deviceIndex == m_deviceIndex) {
      inst->m_serviceNames = m_serviceNames;
      inst->m_serviceUuids = m_serviceUuids;
      Q_EMIT inst->servicesChanged();
    }
  }

  bool serviceSelected = false;
  if (m_pendingServiceIndex > 0) {
    selectService(m_pendingServiceIndex);
    m_pendingServiceIndex = -1;
    serviceSelected       = true;
  }

  else if (!m_pendingServiceUuid.isEmpty() && selectServiceByUuid(m_pendingServiceUuid)) {
    m_pendingServiceUuid.clear();
    serviceSelected = true;
  }

  if (!serviceSelected) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      if (inst->m_pendingServiceUuid.isEmpty())
        continue;

      if (selectServiceByUuid(inst->m_pendingServiceUuid)) {
        inst->m_pendingServiceUuid.clear();
        serviceSelected = true;
        break;
      }
    }
  }

  if (!serviceSelected && !m_pendingNotifyUuid.isEmpty() && !m_serviceUuids.isEmpty()) {
    m_probeServiceIndex = 0;
    selectService(1);
    serviceSelected = true;
  }

  if (!serviceSelected)
    announceGattReady();
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
    case QLowEnergyService::NoError:
      return;
    case QLowEnergyService::CharacteristicReadError:
      Q_EMIT error(tr("Characteristic read error"));
      break;
    case QLowEnergyService::DescriptorReadError:
      Q_EMIT error(tr("Descriptor read error"));
      break;
    default:
      break;
  }

  if (!m_gattReady && openReportArmed()) {
    close();
    reportOpenFinished(false, tr("BLE service error during connect"));
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
    publishReceivedData(value);
    return;
  }

  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count()
      && info == m_characteristics.at(m_selectedCharacteristic)) {
    publishReceivedData(value);
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

  auto hostMode      = s_localDevice->hostMode();
  s_adapterAvailable = (hostMode != QBluetoothLocalDevice::HostPoweredOff);

  QObject::connect(s_localDevice,
                   &QBluetoothLocalDevice::hostModeStateChanged,
                   s_localDevice,
                   [](QBluetoothLocalDevice::HostMode m) { onHostModeStateChanged(m); });

  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->adapterAvailabilityChanged();
}

/**
 * @brief Static callback -- registers a discovered BLE device in the shared list.
 */
void IO::Drivers::BluetoothLE::onDeviceDiscovered(const QBluetoothDeviceInfo& device)
{
  if (!(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
    return;

  if (!device.isValid() || device.name().isEmpty())
    return;

  if (s_devices.contains(device) || s_deviceNames.contains(device.name()))
    return;

  s_devices.append(device);
  s_deviceNames.append(device.name());

  for (auto* inst : std::as_const(s_instances)) {
    if (inst->m_deviceConnected)
      continue;

    Q_EMIT inst->devicesChanged();

    if (!inst->m_pendingIdentifier.isEmpty()) {
      const auto savedAddr = inst->m_pendingIdentifier.value(QStringLiteral("address")).toString();
      const auto savedName = inst->m_pendingIdentifier.value(QStringLiteral("name")).toString();

      const bool addrMatch = !savedAddr.isEmpty() && device.address().toString() == savedAddr;
      const bool nameMatch = !savedName.isEmpty() && device.name() == savedName;
      if (addrMatch || nameMatch) {
        inst->selectDevice(s_devices.count());
        inst->m_pendingIdentifier = {};
      }
    }
  }
}

/**
 * @brief Static callback -- notifies all instances of a discovery error.
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
 * @brief Static callback -- handles Bluetooth adapter power state changes.
 */
void IO::Drivers::BluetoothLE::onHostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
  bool wasAvailable  = s_adapterAvailable;
  s_adapterAvailable = (state != QBluetoothLocalDevice::HostPoweredOff);

  if (wasAvailable == s_adapterAvailable)
    return;

  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->adapterAvailabilityChanged();

  if (!s_adapterAvailable) {
    if (s_discoveryAgent && s_discoveryAgent->isActive())
      s_discoveryAgent->stop();

    s_devices.clear();
    s_deviceNames.clear();
    for (auto* inst : std::as_const(s_instances)) {
      inst->close();
      inst->m_deviceIndex = -1;
      Q_EMIT inst->devicesChanged();
      Q_EMIT inst->deviceIndexChanged();
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

  IO::DriverProperty notifyUuid;
  notifyUuid.key   = QStringLiteral("notifyCharacteristicUuid");
  notifyUuid.label = tr("Notify Characteristic");
  notifyUuid.type  = IO::DriverProperty::Text;
  notifyUuid.value = selectedNotifyCharacteristicUuid();
  props.append(notifyUuid);

  IO::DriverProperty ch;
  ch.key     = QStringLiteral("characteristicIndex");
  ch.label   = tr("Characteristic");
  ch.type    = IO::DriverProperty::ComboBox;
  ch.value   = characteristicIndex() - 1;
  ch.options = m_characteristicNames;
  props.append(ch);

  return props;
}

/**
 * @brief Applies a single Bluetooth LE configuration change by key.
 */
void IO::Drivers::BluetoothLE::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("deviceIndex")) {
    m_deviceIndex = value.toInt();
    Q_EMIT deviceIndexChanged();
    return;
  }

  if (key == QLatin1String("serviceUuid")) {
    m_pendingServiceUuid = value.toString();
    return;
  }

  if (key == QLatin1String("notifyCharacteristicUuid")) {
    m_pendingNotifyUuid = value.toString();
    return;
  }

  if (key == QLatin1String("characteristicIndex")) {
    m_selectedCharacteristic     = value.toInt();
    m_pendingCharacteristicIndex = value.toInt() + 1;
    Q_EMIT characteristicIndexChanged();
  }
}
