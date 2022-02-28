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

#include "Bluetooth.h"

/**
 * Constructor function
 */
IO::DataSources::Bluetooth::Bluetooth()
{
    // Configure signals/slots
    connect(&m_discovery, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this,
            &IO::DataSources::Bluetooth::deviceUpdated);
    connect(&m_discovery, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this,
            &IO::DataSources::Bluetooth::deviceDiscovered);
    connect(&m_discovery, &QBluetoothDeviceDiscoveryAgent::canceled, this,
            &IO::DataSources::Bluetooth::scanningChanged);
    connect(&m_discovery, &QBluetoothDeviceDiscoveryAgent::finished, this,
            &IO::DataSources::Bluetooth::scanningChanged);
    connect(&m_discovery, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this,
            SLOT(onErrorOcurred(QBluetoothDeviceDiscoveryAgent::Error)));

    // Start the device discovery process
    QTimer::singleShot(1000, this, &IO::DataSources::Bluetooth::beginScanning);
}

/**
 * Destructor function
 */
IO::DataSources::Bluetooth::~Bluetooth() { }

/**
 * Returns a reference to the only instance of the class
 */
IO::DataSources::Bluetooth &IO::DataSources::Bluetooth::instance()
{
    static Bluetooth instance;
    return instance;
}

/**
 * Returns the RSSI of the current device
 */
QString IO::DataSources::Bluetooth::rssi() const
{
    return m_rssi;
}

/**
 * Returns @c true if current device is a BLE device.
 */
bool IO::DataSources::Bluetooth::supported() const
{
    return m_supported;
}

/**
 * Returns @c true if the bluetooth discovery service is searching
 * for devices
 */
bool IO::DataSources::Bluetooth::isScanning() const
{
    return m_discovery.isActive();
}

/**
 * Returns the index of the currently selected device
 */
int IO::DataSources::Bluetooth::currentDevice() const
{
    return m_currentDevice;
}

/**
 * Returns a list with the names of all the devices discovered so far
 */
QStringList IO::DataSources::Bluetooth::devices() const
{
    return m_names;
}

/**
 * Returns a list with the names of the services associated with the
 * current device.
 */
QStringList IO::DataSources::Bluetooth::services() const
{
    return m_services;
}

/**
 * Starts the device discovery process for both classic and low energy
 * bluetooth devices.
 */
void IO::DataSources::Bluetooth::beginScanning()
{
    // Reset device & service lists
    m_names.clear();
    m_devices.clear();
    m_services.clear();

    // Reset parameters for current device
    m_rssi = tr("N/A");
    m_supported = false;
    m_currentDevice = -1;
    m_services.append(tr("N/A"));
    m_names.append(tr("Searching..."));

    // Update UI
    Q_EMIT rssiChanged();
    Q_EMIT devicesChanged();
    Q_EMIT scanningChanged();
    Q_EMIT servicesChanged();
    Q_EMIT supportedChanged();
    Q_EMIT currentDeviceChanged();

    // Re-start device discovery
    m_discovery.stop();
    m_discovery.start();
}

/**
 * Changes the current device index & obtains the services associated
 * for the given device.
 */
void IO::DataSources::Bluetooth::setCurrentDevice(const int index)
{
    // Reset device parameters
    m_services.clear();
    m_currentDevice = index - 1;

    // Get device info
    if (m_currentDevice >= 0)
    {
        // Get device info object
        auto device = m_devices.at(m_currentDevice);

        // Update device information
        // clang-format off
        m_services.append(tr("No services found"));
        m_rssi = QString::number(device.rssi()) + " dBm";
        m_supported = device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
        // clang-format on
    }

    // Not a valid device, reset data
    else
    {
        m_rssi = tr("N/A");
        m_supported = false;
        m_services.append(tr("N/A"));
    }

    // Service list is empty, add dummy message for the user
    if (m_services.isEmpty())
    {
        if (currentDevice() >= 0)
            m_services.append(tr("Requesting services..."));
        else
            m_services.append(tr("Waiting for device selection..."));
    }

    // Update UI
    Q_EMIT rssiChanged();
    Q_EMIT servicesChanged();
    Q_EMIT supportedChanged();
    Q_EMIT currentDeviceChanged();
}

/**
 * Refreshes the bluetooth device list
 */
void IO::DataSources::Bluetooth::refreshDevices()
{
    // Clear devices
    m_names.clear();
    m_devices.clear();
    m_services.clear();
    m_names.append(tr("Select device"));

    // Add all discovered devices to list
    for (int i = 0; i < m_discovery.discoveredDevices().count(); ++i)
    {
        auto device = m_discovery.discoveredDevices().at(i);
        if (device.isValid())
        {
            auto name = device.name();
            auto addr = device.address();
            auto uuid = device.deviceUuid();

            if (!name.isEmpty())
                m_names.append(name);
            else if (!addr.isNull())
                m_names.append(addr.toString());
            else
                m_names.append(uuid.toString());

            m_devices.append(device);
        }
    }

    // Update UI
    setCurrentDevice(currentDevice() + 1);
}

/**
 * Re-generate device list uppon device discovery
 */
void IO::DataSources::Bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    (void)info;
    refreshDevices();
}

/**
 * Re-generate device list uppon device update
 */
void IO::DataSources::Bluetooth::deviceUpdated(const QBluetoothDeviceInfo &info,
                                               QBluetoothDeviceInfo::Fields updatedFields)
{
    (void)info;
    (void)updatedFields;
    refreshDevices();
}

/**
 * Prints the device discovery error that have occurred
 */
void IO::DataSources::Bluetooth::onErrorOcurred(
    QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << error << m_discovery.errorString();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Bluetooth.cpp"
#endif
