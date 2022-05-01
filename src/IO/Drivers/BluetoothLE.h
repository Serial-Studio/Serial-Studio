/*
 * Copyright (C) Secretaría de la Defensa Nacional
 *
 * La copia no autorizada de este archivo, a través de cualquier medio,
 * queda estrictamente prohibida. Propietario y confidencial.
 */

#pragma once

#include <QTimer>
#include <QObject>
#include <QLowEnergyController>
#include <QBluetoothDeviceDiscoveryAgent>

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
    Q_PROPERTY(QStringList deviceNames
               READ deviceNames
               NOTIFY devicesChanged)
    Q_PROPERTY(int deviceIndex
               READ deviceIndex
               WRITE selectDevice
               NOTIFY devicesChanged)
    Q_PROPERTY(bool deviceConnected
               READ deviceConnected
               NOTIFY deviceConnectedChanged)
    // clang-format on

Q_SIGNALS:
    void devicesChanged();
    void deviceIndexChanged();
    void deviceConnectedChanged();
    void error(const QString &message);
    void dataReceived(const QByteArray &data);

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
    QStringList deviceNames() const;

public Q_SLOTS:
    void startDiscovery();
    void selectDevice(const int index);

private Q_SLOTS:
    void configureCharacteristics();
    void onServiceDiscoveryFinished();
    void onServiceDiscovered(const QBluetoothUuid &gatt);
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onServiceError(QLowEnergyService::ServiceError serviceError);
    void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void onServiceStateChanged(QLowEnergyService::ServiceState serviceState);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &info,
                                 const QByteArray &value);

private:
    int m_deviceIndex;
    bool m_deviceConnected;
    bool m_uartServiceDiscovered;

    QLowEnergyService *m_service;
    QLowEnergyController *m_controller;

    QStringList m_deviceNames;
    QList<QBluetoothDeviceInfo> m_devices;
    QBluetoothDeviceDiscoveryAgent m_discoveryAgent;
};

}
}
