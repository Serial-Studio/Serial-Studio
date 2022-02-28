/*
 * Copyright (c) 2022 Alex Spataru <https://github.com/alex-spataru>
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

#include <DataTypes.h>

#include <QObject>
#include <QBluetoothUuid>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>

namespace IO
{
namespace DataSources
{
/**
 * @brief The Bluetooth class
 *
 * Serial Studio "driver" class to interact with Bluetooth devices.
 */
class Bluetooth : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QStringList devices
               READ devices
               NOTIFY devicesChanged)
    Q_PROPERTY(QStringList services
               READ services
               NOTIFY servicesChanged)
    Q_PROPERTY(int currentDevice
               READ currentDevice
               WRITE setCurrentDevice
               NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString rssi
               READ rssi
               NOTIFY rssiChanged)
    Q_PROPERTY(bool supported
               READ supported
               NOTIFY supportedChanged)
    Q_PROPERTY(bool isScanning
               READ isScanning
               NOTIFY scanningChanged)
    // clang-format on

Q_SIGNALS:
    void rssiChanged();
    void devicesChanged();
    void scanningChanged();
    void servicesChanged();
    void supportedChanged();
    void currentDeviceChanged();

private:
    explicit Bluetooth();
    Bluetooth(Bluetooth &&) = delete;
    Bluetooth(const Bluetooth &) = delete;
    Bluetooth &operator=(Bluetooth &&) = delete;
    Bluetooth &operator=(const Bluetooth &) = delete;

    ~Bluetooth();

public:
    static Bluetooth &instance();

    QString rssi() const;
    bool supported() const;
    bool isScanning() const;
    int currentDevice() const;
    QStringList devices() const;
    QStringList services() const;

public Q_SLOTS:
    void beginScanning();
    void setCurrentDevice(const int index);

private Q_SLOTS:
    void refreshDevices();
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void deviceUpdated(const QBluetoothDeviceInfo &info,
                       QBluetoothDeviceInfo::Fields updatedFields);
    void onErrorOcurred(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    QString m_rssi;
    bool m_supported;
    int m_currentDevice;
    QStringList m_names;
    QStringList m_services;
    QList<QBluetoothDeviceInfo> m_devices;
    QBluetoothDeviceDiscoveryAgent m_discovery;
};
}
}
