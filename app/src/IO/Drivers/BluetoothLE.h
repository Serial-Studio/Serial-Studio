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

#pragma once

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QLowEnergyService>

#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief HAL driver for Bluetooth Low Energy peripherals.
 */
class BluetoothLE : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int deviceCount
             READ deviceCount
             NOTIFY devicesChanged)
  Q_PROPERTY(QStringList deviceNames
             READ deviceNames
             NOTIFY devicesChanged)
  Q_PROPERTY(QStringList serviceNames
             READ serviceNames
             NOTIFY servicesChanged)
  Q_PROPERTY(QStringList characteristicNames
             READ characteristicNames
             NOTIFY characteristicsChanged)
  Q_PROPERTY(int deviceIndex
             READ deviceIndex
             WRITE selectDevice
             NOTIFY deviceIndexChanged)
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY deviceConnectedChanged)
  Q_PROPERTY(bool operatingSystemSupported
             READ operatingSystemSupported
             CONSTANT)
  Q_PROPERTY(bool adapterAvailable
             READ adapterAvailable
             NOTIFY adapterAvailabilityChanged)
  Q_PROPERTY(int characteristicIndex
             READ characteristicIndex
             WRITE setCharacteristicIndex
             NOTIFY characteristicIndexChanged)
  // clang-format on

signals:
  void devicesChanged();
  void servicesChanged();
  void deviceIndexChanged();
  void characteristicsChanged();
  void deviceConnectedChanged();
  void characteristicIndexChanged();
  void adapterAvailabilityChanged();
  void error(const QString& message);

public:
  explicit BluetoothLE();
  ~BluetoothLE() override;

  BluetoothLE(BluetoothLE&&)                 = delete;
  BluetoothLE(const BluetoothLE&)            = delete;
  BluetoothLE& operator=(BluetoothLE&&)      = delete;
  BluetoothLE& operator=(const BluetoothLE&) = delete;

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QJsonObject deviceIdentifier() const override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;
  bool selectByIdentifier(const QJsonObject& id) override;

  [[nodiscard]] bool ignoreDataDelimeters() const;
  [[nodiscard]] bool operatingSystemSupported() const;
  [[nodiscard]] bool adapterAvailable() const;

  [[nodiscard]] int deviceCount() const;
  [[nodiscard]] int deviceIndex() const;
  [[nodiscard]] int characteristicIndex() const;

  [[nodiscard]] QStringList deviceNames() const;
  [[nodiscard]] QStringList serviceNames() const;
  [[nodiscard]] QStringList characteristicNames() const;

  [[nodiscard]] QString selectedServiceUuid() const;

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void startDiscovery();
  void selectDevice(const int index);
  void selectService(const int index);
  void setCharacteristicIndex(const int index);

private slots:
  void configureCharacteristics();
  void onServiceDiscoveryFinished();
  void onServiceError(QLowEnergyService::ServiceError serviceError);
  void onServiceStateChanged(QLowEnergyService::ServiceState serviceState);
  void onCharacteristicChanged(const QLowEnergyCharacteristic& info, const QByteArray& value);

private:
  static void initializeSharedState();
  static void onDeviceDiscovered(const QBluetoothDeviceInfo& device);
  static void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
  static void onHostModeStateChanged(QBluetoothLocalDevice::HostMode state);

private:
  int m_deviceIndex;
  bool m_deviceConnected;
  int m_selectedCharacteristic;
  int m_pendingServiceIndex;
  int m_pendingCharacteristicIndex;
  QString m_pendingServiceUuid;
  QJsonObject m_pendingIdentifier;

  QLowEnergyService* m_service;
  QLowEnergyController* m_controller;

  QStringList m_serviceNames;
  QStringList m_characteristicNames;

  QList<QLowEnergyCharacteristic> m_characteristics;

private:
  static bool s_initialized;
  static bool s_adapterAvailable;
  static QBluetoothLocalDevice* s_localDevice;
  static QBluetoothDeviceDiscoveryAgent* s_discoveryAgent;
  static QStringList s_deviceNames;
  static QList<QBluetoothDeviceInfo> s_devices;
  static QList<BluetoothLE*> s_instances;
};
}  // namespace Drivers
}  // namespace IO
