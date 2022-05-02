/*
 * Copyright (c) 2020-2022 Alex Spataru <https://github.com/alex-spataru>
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

#pragma once

#include <QTimer>
#include <QObject>
#include <QLowEnergyController>
#include <QBluetoothDeviceDiscoveryAgent>

#include <DataTypes.h>
#include <IO/HAL_Driver.h>

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
    Q_PROPERTY(StringList deviceNames
               READ deviceNames
               NOTIFY devicesChanged)
    Q_PROPERTY(StringList serviceNames
               READ serviceNames
               NOTIFY servicesChanged)
    Q_PROPERTY(int deviceIndex
               READ deviceIndex
               WRITE selectDevice
               NOTIFY devicesChanged)
    Q_PROPERTY(bool deviceConnected
               READ deviceConnected
               NOTIFY deviceConnectedChanged)
    Q_PROPERTY(bool operatingSystemSupported
               READ operatingSystemSupported
               CONSTANT)
    // clang-format on

Q_SIGNALS:
    void devicesChanged();
    void servicesChanged();
    void deviceIndexChanged();
    void deviceConnectedChanged();
    void error(const QString &message);

private:
    explicit BluetoothLE();
    BluetoothLE(BluetoothLE &&) = delete;
    BluetoothLE(const BluetoothLE &) = delete;
    BluetoothLE &operator=(BluetoothLE &&) = delete;
    BluetoothLE &operator=(const BluetoothLE &) = delete;

public:
    static BluetoothLE &instance();

    //
    // HAL functions
    //
    void close() override;
    bool isOpen() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool configurationOk() const override;
    quint64 write(const QByteArray &data) override;
    bool open(const QIODevice::OpenMode mode) override;

    int deviceCount() const;
    int deviceIndex() const;
    bool deviceConnected() const;
    StringList deviceNames() const;
    StringList serviceNames() const;
    bool operatingSystemSupported() const;

public Q_SLOTS:
    void startDiscovery();
    void selectDevice(const int index);
    void selectService(const int index);

private Q_SLOTS:
    void configureCharacteristics();
    void onServiceDiscoveryFinished();
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void onServiceStateChanged(QLowEnergyService::ServiceState serviceState);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &info,
                                 const QByteArray &value);

private:
    int m_deviceIndex;
    bool m_deviceConnected;

    QLowEnergyService *m_service;
    QLowEnergyController *m_controller;

    StringList m_deviceNames;
    StringList m_serviceNames;
    QList<QBluetoothDeviceInfo> m_devices;
    QBluetoothDeviceDiscoveryAgent m_discoveryAgent;
};
}
}
