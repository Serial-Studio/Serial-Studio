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

#pragma once

#include <QObject>
#include <QLowEnergyController>
#include <QBluetoothDeviceDiscoveryAgent>

#include "IO/HAL_Driver.h"

namespace IO
{
namespace Drivers
{
/**
 * @brief The BluetoothLE class
 * Serial Studio driver class to interact with Bluetooth Low Energy devices.
 */
class BluetoothLE : public HAL_Driver
{
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
             NOTIFY devicesChanged)
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY deviceConnectedChanged)
  Q_PROPERTY(bool operatingSystemSupported
             READ operatingSystemSupported
             CONSTANT)
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
  void error(const QString &message);

private:
  explicit BluetoothLE();
  BluetoothLE(BluetoothLE &&) = delete;
  BluetoothLE(const BluetoothLE &) = delete;
  BluetoothLE &operator=(BluetoothLE &&) = delete;
  BluetoothLE &operator=(const BluetoothLE &) = delete;

public:
  static BluetoothLE &instance();

  void close() override;

  [[nodiscard]] bool isOpen() const override;
  [[nodiscard]] bool isReadable() const override;
  [[nodiscard]] bool isWritable() const override;
  [[nodiscard]] bool configurationOk() const override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] bool ignoreDataDelimeters() const;
  [[nodiscard]] bool operatingSystemSupported() const;

  [[nodiscard]] int deviceCount() const;
  [[nodiscard]] int deviceIndex() const;
  [[nodiscard]] int characteristicIndex() const;

  [[nodiscard]] QStringList deviceNames() const;
  [[nodiscard]] QStringList serviceNames() const;
  [[nodiscard]] QStringList characteristicNames() const;

public slots:
  void startDiscovery();
  void selectDevice(const int index);
  void selectService(const int index);
  void setCharacteristicIndex(const int index);

private slots:
  void configureCharacteristics();
  void onServiceDiscoveryFinished();
  void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
  void onServiceError(QLowEnergyService::ServiceError serviceError);
  void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
  void onServiceStateChanged(QLowEnergyService::ServiceState serviceState);
  void onCharacteristicChanged(const QLowEnergyCharacteristic &info,
                               const QByteArray &value);

private:
  int m_deviceIndex;
  bool m_deviceConnected;
  int m_selectedCharacteristic;

  QLowEnergyService *m_service;
  QLowEnergyController *m_controller;
  QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;

  QStringList m_deviceNames;
  QStringList m_serviceNames;
  QStringList m_characteristicNames;

  QList<QBluetoothDeviceInfo> m_devices;
  QList<QLowEnergyCharacteristic> m_characteristics;
};
} // namespace Drivers
} // namespace IO
