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

#include <IO/Drivers/BluetoothLE.h>

/**
 * @brief UUID del servicio de UART provisto por el transmisor BLE
 */
const QBluetoothUuid UART_SERVICE(QString("{6e400001-b5a3-f393-e0a9-e50e24dcca9e}"));

//----------------------------------------------------------------------------------------
// Constructor & singleton access functions
//----------------------------------------------------------------------------------------

/**
 * Constructor de la clase, configura los conectores de señales/slots entre el modulo de
 * busqueda de dispositivos y la clase
 */
IO::Drivers::BluetoothLE::BluetoothLE()
    : m_deviceIndex(-1)
    , m_deviceConnected(false)
    , m_uartServiceDiscovered(false)
    , m_service(Q_NULLPTR)
    , m_controller(Q_NULLPTR)
{
    // clang-format off

    // Update connect button status when a BLE device is selected by the user
    connect(this, &IO::Drivers::BluetoothLE::deviceIndexChanged,
            this, &IO::Drivers::BluetoothLE::configurationChanged);

    // Register discovered devices
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &IO::Drivers::BluetoothLE::onDeviceDiscovered);

    // Report BLE discovery errors
    connect(&m_discoveryAgent,
            static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(
                QBluetoothDeviceDiscoveryAgent::Error)>(
                &QBluetoothDeviceDiscoveryAgent::error),
            this, &IO::Drivers::BluetoothLE::onDiscoveryError);

    // clang-format on
}

/**
 * Returns the only instance of the class
 */
IO::Drivers::BluetoothLE &IO::Drivers::BluetoothLE::instance()
{
    static BluetoothLE singleton;
    return singleton;
}

//----------------------------------------------------------------------------------------
// HAL driver implementation
//----------------------------------------------------------------------------------------

void IO::Drivers::BluetoothLE::close()
{
    if (m_controller)
        m_controller->disconnectFromDevice();
}

bool IO::Drivers::BluetoothLE::isOpen() const
{
    return false;
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
    return deviceIndex() >= 0;
}

quint64 IO::Drivers::BluetoothLE::write(const QByteArray &data)
{
    return -1;
}

bool IO::Drivers::BluetoothLE::open(const QIODevice::OpenMode mode)
{
    // Validar el indice del dispositivo
    if (m_deviceIndex < 0 || m_deviceIndex >= m_devices.count())
        return false;

    // Eliminar servicio anterior
    if (m_service)
    {
        disconnect(m_service);
        m_service->deleteLater();
        m_service = Q_NULLPTR;
    }

    // Eliminar controlador anterior
    if (m_controller)
    {
        disconnect(m_controller);
        m_controller->deleteLater();
        m_controller = Q_NULLPTR;
    }

    // Inicializar un controlador BLE para el dispositivo seleccionado
    auto device = m_devices.at(m_deviceIndex);
    m_controller = QLowEnergyController::createCentral(device, this);

    // Configurar señales/slots del controlador BLE
    connect(m_controller, &QLowEnergyController::serviceDiscovered, this,
            &IO::Drivers::BluetoothLE::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this,
            &IO::Drivers::BluetoothLE::onServiceDiscoveryFinished);

    // Reaccionar al evento de conexión con el dispositivo BLE
    connect(m_controller, &QLowEnergyController::connected, this, [this]() {
        m_deviceConnected = true;
        m_controller->discoverServices();
        Q_EMIT deviceConnectedChanged();
    });

    // Reaccionar al evento de desconexión del dispositivo BLE
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
        Q_EMIT error(tr("El dispositivo BLE ha sido desconectado"));
    });

    // Emparejarse con el dispositivo BLE
    m_controller->connectToDevice();
    return true;
}

//----------------------------------------------------------------------------------------
// Driver specifics
//----------------------------------------------------------------------------------------

/**
 * @return El número de dispositivos BLE encontrados.
 */
int IO::Drivers::BluetoothLE::deviceCount() const
{
    return m_devices.count();
}

/**
 * @return El índice del dispositivo BLE seleccionado por el usuario.
 */
int IO::Drivers::BluetoothLE::deviceIndex() const
{
    return m_deviceIndex;
}

/**
 * @return @c true si la aplicación esta conectada a un dispositivo BLE.
 */
bool IO::Drivers::BluetoothLE::deviceConnected() const
{
    return m_deviceConnected;
}

/**
 * @return Una lista con los dispositivos BLE encontrados.
 */
QStringList IO::Drivers::BluetoothLE::deviceNames() const
{
    QStringList list;
    list.append(tr("Select device"));
    list.append(m_deviceNames);
    return list;
}

/**
 * Comienza el proceso de busqueda de dispositivos BLE.
 */
void IO::Drivers::BluetoothLE::startDiscovery()
{
    // Un dispositivo BLE esta conectado, cancelar proceso de escaneo
    if (deviceConnected())
        return;

    // Restaurar la clase y sus miembros al estado incial
    m_devices.clear();
    m_deviceIndex = -1;
    m_deviceNames.clear();
    m_uartServiceDiscovered = false;

    // Eliminar servicios anteriores
    if (m_service)
    {
        disconnect(m_service);
        delete m_service;
    }

    // Eliminar controladores anteriores
    if (m_controller)
    {
        disconnect(m_controller);
        delete m_controller;
    }

    // Notificar cambios
    Q_EMIT devicesChanged();
    Q_EMIT deviceIndexChanged();

    // Comenzar el proceso de escaneo
    m_discoveryAgent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

/**
 * Cambia el índice del dispositivo seleccionado por el usuario.
 *
 * @note Esta función compensa el elemento de "Seleccionar dispositivo" agregado
 *       a la lista de dispositivos en @c deviceNames() para que el índice
 *       corresponda a los elementos de @c m_devices.
 */
void IO::Drivers::BluetoothLE::selectDevice(const int index)
{
    close();
    m_deviceIndex = index - 1;
    Q_EMIT deviceIndexChanged();
}

/**
 * Establece las opciones de interacción entre cada característica del dispositivo BLE y
 * la aplicación.
 */
void IO::Drivers::BluetoothLE::configureCharacteristics()
{
    // Validar servicio BLE
    if (!m_service)
        return;

    // Probar todas las características del dispositivo
    foreach (QLowEnergyCharacteristic c, m_service->characteristics())
    {
        // Validar que la característica sea valida
        if (!c.isValid())
            continue;

        // Establecer propiedades de la conexión cliente/descriptor
        auto descriptor = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (descriptor.isValid())
            m_service->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
    }
}

/**
 * Configura las conexiónes de señal/slot para poder leer los datos transmitidos mediante
 * el servicio de UART.
 */
void IO::Drivers::BluetoothLE::onServiceDiscoveryFinished()
{
    // Eliminar servicio anterior
    if (m_service)
        delete m_service;

    // Intentar hacer uso del servicio de UART del dispostivo BLE
    if (m_uartServiceDiscovered)
    {
        m_service = m_controller->createServiceObject(UART_SERVICE, this);
        if (m_service)
        {
            // clang-format off
            connect(m_service, &QLowEnergyService::characteristicChanged, this,
                    &IO::Drivers::BluetoothLE::onCharacteristicChanged);
            connect(m_service, &QLowEnergyService::characteristicRead, this,
                    &IO::Drivers::BluetoothLE::onCharacteristicChanged);
            connect(m_service, &QLowEnergyService::stateChanged, this,
                    &IO::Drivers::BluetoothLE::onServiceStateChanged);
            connect(m_service,
                    static_cast<void (QLowEnergyService::*)
                    (QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                    this, &IO::Drivers::BluetoothLE::onServiceError);
            // clang-format on

            if (m_service->state() == QLowEnergyService::DiscoveryRequired)
                m_service->discoverDetails();
            else
                configureCharacteristics();
        }
    }

    // Error al registrar el servicio
    if (!m_service)
        Q_EMIT error(tr("El dispositivo no cuenta con un servicio de UART"));
}

/**
 * Activa la bandera que indica que el dispositivo BLE suporta el servicio de UART.
 * Esto se realiza al comparar el UUID del servicio encontrado y el UUID que corresponde
 * al servicio de UART.
 */
void IO::Drivers::BluetoothLE::onServiceDiscovered(const QBluetoothUuid &uuid)
{
    // Activar bandera de servicio encontrado si el GATT corresponde al servicio de UART
    if (uuid == UART_SERVICE)
        m_uartServiceDiscovered = true;
}

/**
 * Registra el dispositivo (@c device) encontrado en la lista de dispositivos BLE.
 */
void IO::Drivers::BluetoothLE::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    // Solo registrar dispositivos BLE
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        // Validar nombre del dispositivo (no queremos mostrar dispositivos ocultos...)
        if (!device.isValid() || device.name().isEmpty())
            return;

        // Agregar el dispositivo a la lista de dispositivos encontrados
        if (!m_devices.contains(device))
        {
            m_devices.append(device);
            m_deviceNames.append(device.name());

            Q_EMIT devicesChanged();
        }
    }
}

/**
 * Imprime cualquier error que ocurra con el servicio de UART.
 */
void IO::Drivers::BluetoothLE::onServiceError(
    QLowEnergyService::ServiceError serviceError)
{
    qDebug() << __func__ << "ERROR" << serviceError;
}

/**
 * Notifica al usuario de cualquier error detectado en el adaptador Bluetooth de la
 * computadora/tablet/telefono.
 */
void IO::Drivers::BluetoothLE::onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error e)
{
    switch (e)
    {
        case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
            Q_EMIT error(tr("El adaptador Bluetooth esta apagado!"));
            break;
        case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
            Q_EMIT error(tr("Adaptador de Bluetooth inválido!"));
            break;
        case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
            Q_EMIT error(tr("Este sistema operativo no está soportado!"));
            break;
        case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
            Q_EMIT error(tr("Modo de escaneo no soportado!"));
            break;
        case QBluetoothDeviceDiscoveryAgent::InputOutputError:
            Q_EMIT error(tr("Error general de I/O"));
            break;
        default:
            break;
    }
}

/**
 * Configura las características de los servicios encontrados en el dispositivo BLE
 * emparejado con la aplicación.
 */
void IO::Drivers::BluetoothLE::onServiceStateChanged(
    QLowEnergyService::ServiceState serviceState)
{
    if (serviceState == QLowEnergyService::ServiceDiscovered)
        configureCharacteristics();
}

/**
 * Lee los datos transmitidos (o característica RX) del servicio UART.
 */
void IO::Drivers::BluetoothLE::onCharacteristicChanged(
    const QLowEnergyCharacteristic &info, const QByteArray &value)
{
    (void)info;

    if (!value.isEmpty())
        Q_EMIT dataReceived(value);
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_BluetoothLE.cpp"
#endif
